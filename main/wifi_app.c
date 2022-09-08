#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"

#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_softap.h>

#include "nvs_app.h"
#include "tasks_common.h"
#include "wifi_app.h"

// Tag used for ESP serial console messages
static const char TAG[] = "wifi_app";

// WiFi application callback
static wifi_connected_event_callback_t wifi_connected_event_cb;

// Used for returning the WiFi configuration
wifi_config_t *wifi_config = NULL;

// Used to track the number for retries when a connection attempt fails
static int g_retry_number;

/**
 * Wifi application event group handle and status bits
 */
static EventGroupHandle_t wifi_app_event_group;
const int WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT = BIT0;
const int WIFI_APP_STA_CONNECTED_GOT_IP_BIT = BIT1;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t wifi_app_queue_handle;

// netif objects for the station and access point
esp_netif_t *esp_netif_sta = NULL;
esp_netif_t *esp_netif_ap = NULL;

/**
 * WiFi application event handler
 * @param arg data, aside from event data, that is passed to the handler when it is called
 * @param event_base the base id of the event to register the handler for
 * @param event_id the id fo the event to register the handler for
 * @param event_data event data
 */
static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    static int retries;
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
            break;

        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
            break;

        case WIFI_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
            break;

        case WIFI_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
            break;

        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
            break;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");

            wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = (wifi_event_sta_disconnected_t *)malloc(sizeof(wifi_event_sta_disconnected_t));
            *wifi_event_sta_disconnected = *((wifi_event_sta_disconnected_t *)event_data);
            printf("WIFI_EVENT_STA_DISCONNECTED, reason code %d\n", wifi_event_sta_disconnected->reason);

            if (g_retry_number < MAX_CONNECTION_RETRIES)
            {
                esp_wifi_connect();
                g_retry_number++;
            }
            else
            {
                wifi_app_send_message(WIFI_APP_MSG_STA_DISCONNECTED);
            }

            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");

            wifi_app_send_message(WIFI_APP_MSG_STA_CONNECTED_GOT_IP);

            break;
        }
    }
    else if (event_base == WIFI_PROV_EVENT)
    {
        switch (event_id)
        {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "Provisioning started");
            break;
        case WIFI_PROV_CRED_RECV:
        {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "Received Wi-Fi credentials"
                          "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);

            break;
        }
        case WIFI_PROV_CRED_FAIL:
        {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                          "\n\tPlease reset to factory and retry provisioning",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
            retries++;
            if (retries >= MAX_CONNECTION_RETRIES)
            {
                ESP_LOGI(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
                wifi_prov_mgr_reset_sm_state_on_failure();
                retries = 0;
            }
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning successful");
            retries = 0;
            esp_wifi_get_config(ESP_IF_WIFI_AP, wifi_config);

            nvs_app_save_sta_creds();
            break;
        case WIFI_PROV_END:
            /* De-initialize manager once provisioning is finished */
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
        }
    }
}

/**
 * Initializes the WiFi application event handler for WiFi and IP events.
 */
static void wifi_app_event_handler_init(void)
{
    // Event loop for the WiFi driver
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Event handler for the connection
    esp_event_handler_instance_t instance_wifi_event;
    esp_event_handler_instance_t instance_ip_event;
    esp_event_handler_instance_t instance_Provising_event;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_Provising_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_ip_event));
}

/**
 * Initializes the TCP stack and default WiFi configuration.
 */
static void wifi_app_default_wifi_init(void)
{
    // Initialize the TCP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Default WiFi config - operations must be in this order!
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    esp_netif_sta = esp_netif_create_default_wifi_sta();
    esp_netif_ap = esp_netif_create_default_wifi_ap();
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/* Handler for the optional provisioning endpoint registered by the application.
 * The data format can be chosen by applications. Here, we are using plain ascii text.
 * Applications can choose to use other formats like protobuf, JSON, XML, etc.
 */
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf)
    {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL)
    {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

static void wifi_prov_print_qr(const char *name, const char *pop, const char *transport)
{
    if (!name || !transport)
    {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    if (pop)
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, pop, transport);
    }
    else
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, transport);
    }
    ESP_LOGI(TAG, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, payload);
}

/**
 * Connects the ESP32 to an external AP using the updated station configuration
 */
static void wifi_app_connect_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_app_get_wifi_config()));
    ESP_ERROR_CHECK(esp_wifi_connect());
}

/**
 * Main task for the WiFi application
 * @param pvParameters parameter which can be passed to the task
 */
static void wifi_app_task(void *pvParameters)
{
    wifi_app_queue_message_t msg;
    EventBits_t eventBits;

    // Initialize the event handler
    wifi_app_event_handler_init();

    // Initialize the TCP/IP stack and WiFi config
    wifi_app_default_wifi_init();

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    // Send first event message
    wifi_app_send_message(WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS);

    while (true)
    {
        if (xQueueReceive(wifi_app_queue_handle, &msg, portMAX_DELAY))
        {
            switch (msg.msgID)
            {

            case WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS:
                ESP_LOGI(TAG, "WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS");

                if (nvs_app_load_sta_creds())
                {
                    ESP_LOGI(TAG, "Loaded station configuration");
                    wifi_app_connect_sta();
                }
                else
                {
                    ESP_LOGI(TAG, "Unable to load station configuration");
                    wifi_app_send_message(WIFI_APP_MSG_START_PROVISING);
                }

                // Next, start the web server

                break;

            case WIFI_APP_MSG_START_PROVISING:

                ESP_LOGI(TAG, "Proving about to start");
                bool provisioned = false;

                wifi_prov_mgr_reset_provisioning();
                /* Let's find out if the device is provisioned */
                esp_err_t ret = wifi_prov_mgr_is_provisioned(&provisioned);
                if (ret == ESP_FAIL)
                {
                    ESP_LOGI(TAG, "ESP_FAIL");
                }
                else if (ret == ESP_ERR_INVALID_STATE)
                {
                    ESP_LOGI(TAG, "ESP_ERR_INVALID_STATE");
                }
                else if (ret == ESP_ERR_INVALID_ARG)
                {
                    ESP_LOGI(TAG, "ESP_ERR_INVALID_ARG");
                }
                else if (ret == ESP_OK)
                {
                    ESP_LOGI(TAG, "ESP_OK");
                    if (provisioned)
                    {
                        ESP_LOGI(TAG, "true");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "false");
                    }
                }
                if (!provisioned)
                {
                    ESP_LOGI(TAG, "Starting provisioning");

                    char service_name[12];
                    get_device_service_name(service_name, sizeof(service_name));

                    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
                    const char *pop = "abcd1234";
                    const char *service_key = NULL;

                    wifi_prov_mgr_endpoint_create("custom-data");
                    ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key));
                    wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);
                    wifi_prov_print_qr(service_name, pop, PROV_TRANSPORT_SOFTAP);
                }
                else
                {
                    ESP_LOGI(TAG, "Erro Provising");
                }
                break;

            case WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT:
                ESP_LOGI(TAG, "WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT");
                eventBits = xEventGroupGetBits(wifi_app_event_group);

				if (eventBits & WIFI_APP_STA_CONNECTED_GOT_IP_BIT)
				{
                    ESP_ERROR_CHECK(esp_wifi_disconnect());
                }
                nvs_app_clear_sta_creds();

                break;

            case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP");

                // rgb_led_wifi_connected();
                xEventGroupSetBits(wifi_app_event_group, WIFI_APP_STA_CONNECTED_GOT_IP_BIT);

                // Check for connection callback
                if (wifi_connected_event_cb)
                {
                    wifi_app_call_callback();
                }

                break;

            case WIFI_APP_MSG_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED");

                eventBits = xEventGroupGetBits(wifi_app_event_group);
                if (eventBits && WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT)
                {
                    ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: USER REQUESTED DISCONNECTION");
                    xEventGroupClearBits(wifi_app_event_group, WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT);
                    wifi_app_send_message(WIFI_APP_MSG_START_PROVISING);
                }
                else
                {
                    ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: ATTEMPT FAILED, CHECK WIFI ACCESS POINT AVAILABILITY");
                    vTaskDelay(30000 / portTICK_RATE_MS);
                    wifi_app_send_message(WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS);
                }

            default:
                break;
            }
        }
    }
}

BaseType_t wifi_app_send_message(wifi_app_message_e msgID)
{
    wifi_app_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSend(wifi_app_queue_handle, &msg, portMAX_DELAY);
}

wifi_config_t *wifi_app_get_wifi_config(void)
{
    return wifi_config;
}

void wifi_app_set_callback(wifi_connected_event_callback_t cb)
{
    wifi_connected_event_cb = cb;
}

void wifi_app_call_callback(void)
{
    wifi_connected_event_cb();
}

void wifi_app_start(void)
{
    ESP_LOGI(TAG, "STARTING WIFI APPLICATION");

    // Start WiFi started LED
    // rgb_led_wifi_app_started();

    ESP_LOGI(TAG, "Configuring Provising");
    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t configProvising = {
        /* What is the Provisioning Scheme that we want ?
         * wifi_prov_scheme_softap or wifi_prov_scheme_ble */
        .scheme = wifi_prov_scheme_softap,

        /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * WIFI_PROV_EVENT_HANDLER_NONE when using wifi_prov_scheme_softap*/
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE

    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(configProvising));

    // Disable default WiFi logging messages
    esp_log_level_set("wifi", ESP_LOG_NONE);

    // Allocate memory for the wifi configuration
    wifi_config = (wifi_config_t *)malloc(sizeof(wifi_config_t));
    memset(wifi_config, 0x00, sizeof(wifi_config_t));

    // Create message queue
    wifi_app_queue_handle = xQueueCreate(3, sizeof(wifi_app_queue_message_t));

    // Create Wifi application event group
    wifi_app_event_group = xEventGroupCreate();

    // Start the WiFi application task
    xTaskCreatePinnedToCore(&wifi_app_task, "wifi_app_task", WIFI_APP_TASK_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL, WIFI_APP_TASK_CORE_ID);
}