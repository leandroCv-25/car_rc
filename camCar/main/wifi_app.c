#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"

#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_softap.h>

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

// Queue handle used to manipulate the main queue of events
static QueueHandle_t wifi_app_queue_handle;

// netif objects for the station and access point
esp_netif_t *esp_netif_sta = NULL;

/**
 * WiFi application event handler
 * @param arg data, aside from event data, that is passed to the handler when it is called
 * @param event_base the base id of the event to register the handler for
 * @param event_id the id fo the event to register the handler for
 * @param event_data event data
 */
static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {

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
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
            wifi_app_send_message(WIFI_APP_MSG_STA_CONNECTED_GOT_IP);

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

    // Initialize the event handler
    wifi_app_event_handler_init();

    // Initialize the TCP/IP stack and WiFi config
    wifi_app_default_wifi_init();

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    // Send first event message
    wifi_app_send_message(WIFI_APP_MSG_SET_CREDENTIALS);

    while (true)
    {
        if (xQueueReceive(wifi_app_queue_handle, &msg, portMAX_DELAY))
        {
            switch (msg.msgID)
            {

            case WIFI_APP_MSG_SET_CREDENTIALS:
                ESP_LOGI(TAG, "WIFI_APP_MSG_SET_CREDENTIALS");

                ESP_LOGI(TAG, "Loaded station configuration");
                wifi_app_connect_sta();

                break;

            case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP");

                // Check for connection callback
                if (wifi_connected_event_cb)
                {
                    wifi_app_call_callback();
                }

                break;

            case WIFI_APP_MSG_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED");

                ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: ATTEMPT FAILED, CHECK WIFI ACCESS POINT AVAILABILITY");
                vTaskDelay(30000 / portTICK_RATE_MS);
                wifi_app_send_message(WIFI_APP_MSG_SET_CREDENTIALS);
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

void wifi_app_start(char *str_ssid_wifi, char *str_pass_wifi)
{
    ESP_LOGI(TAG, "STARTING WIFI APPLICATION");

    // Start WiFi started LED
    // rgb_led_wifi_app_started();

    // Disable default WiFi logging messages
    esp_log_level_set("wifi", ESP_LOG_NONE);

    // Allocate memory for the wifi configuration
    wifi_config = (wifi_config_t *)malloc(sizeof(wifi_config_t));
    memset(wifi_config, 0x00, sizeof(wifi_config_t));

    // Create message queue
    wifi_app_queue_handle = xQueueCreate(3, sizeof(wifi_app_queue_message_t));

    memcpy(wifi_config->sta.ssid, str_ssid_wifi, strlen(str_ssid_wifi));
    memcpy(wifi_config->sta.password, str_pass_wifi, strlen(str_pass_wifi));

    printf("app_nvs_load_sta_creds: SSID: %s Password: %s\n", wifi_config->sta.ssid, wifi_config->sta.password);

    // Start the WiFi application task
    xTaskCreatePinnedToCore(&wifi_app_task, "wifi_app_task", WIFI_APP_TASK_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL, WIFI_APP_TASK_CORE_ID);
}