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
#include "rgb_led.h"

// Tag used for ESP serial console messages
static const char TAG[] = "wifi_app";

// Used for returning the WiFi configuration
wifi_config_t *wifi_config = NULL;

// netif objects for the station and access point
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
    default:
        break;
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
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
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
    esp_netif_ap = esp_netif_create_default_wifi_ap();
}

/**
 * Configures the WiFi access point settings and assigns the static IP to the SoftAP.
 */
static void wifi_app_soft_ap_config(wifi_config_t ap_config)
{

	// Configure DHCP for the AP
	esp_netif_ip_info_t ap_ip_info;
	memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));

	esp_netif_dhcps_stop(esp_netif_ap);					///> must call this first
	inet_pton(AF_INET, "192.168.0.1", &ap_ip_info.ip);		///> Assign access point's static IP, GW, and netmask
	inet_pton(AF_INET, "192.168.0.1", &ap_ip_info.gw);
	inet_pton(AF_INET, "255.255.255.0", &ap_ip_info.netmask);
	ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));			///> Statically configure the network interface
	ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));						///> Start the AP DHCP server (for connecting stations e.g. your mobile device)
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));						///> Setting the mode as Access Point / Station Mode
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));			///> Set our configuration
	ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20));		///> Our default bandwidth 20 MHz
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));						///> Power save set to "NONE"

}


void wifi_app_start(char *str_ssid_wifi, char *str_pass_wifi)
{
    ESP_LOGI(TAG, "STARTING WIFI APPLICATION");

    // Start WiFi started LED
    rgb_led_wifi_app_started();

    // Disable default WiFi logging messages
    esp_log_level_set("wifi", ESP_LOG_NONE);

    // SoftAP - WiFi access point configuration
	wifi_config_t ap_config =
		{
			.ap = {
				.ssid_len = strlen(str_ssid_wifi),
				.channel = 1,
				.ssid_hidden = 0,
				.authmode = WIFI_AUTH_WPA2_PSK,
				.max_connection = 5,
				.beacon_interval = 100,
			},
		};

    memcpy(ap_config.ap.ssid, str_ssid_wifi, strlen(str_ssid_wifi));
    memcpy(ap_config.ap.password, str_pass_wifi, strlen(str_pass_wifi));

    printf("Wifi : SSID: %s Password: %s\n", wifi_config->sta.ssid, wifi_config->sta.password);

     // Initialize the event handler
    wifi_app_event_handler_init();

    // Initialize the TCP/IP stack and WiFi config
    wifi_app_default_wifi_init();

    //config AP 
    wifi_app_soft_ap_config(ap_config);

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

}