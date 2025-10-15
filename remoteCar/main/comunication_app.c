#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"

#include "espnow.h"
#include "espnow_ctrl.h"

#include "device_params.h"
#include "comunication_app.h"
#include "rgb_led.h"

// Tag used for ESP serial console messages
static const char TAG[] = "comunication_app";

// control status
static espnow_ctrl_status_t s_espnow_ctrl_status;

static void wifi_init(void)
{
	// Initialize the TCP stack
	ESP_ERROR_CHECK(esp_netif_init());

	// Event loop for the WiFi driver
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Default WiFi config - operations must be in this order!
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	ESP_ERROR_CHECK(esp_wifi_start());
}

static void comunication_bind(){
	ESP_LOGI(TAG, "initiator bind press");
    if (s_espnow_ctrl_status == ESPNOW_CTRL_INIT) {
        espnow_ctrl_initiator_bind(ESPNOW_ATTRIBUTE_KEY_1, true);
        s_espnow_ctrl_status = ESPNOW_CTRL_BOUND;
		rgb_led_remote_connected();
    }
}

static void comunication_unbind(){
	ESP_LOGI(TAG, "initiator bind press");
    if (s_espnow_ctrl_status == ESPNOW_CTRL_BOUND) {
        espnow_ctrl_initiator_bind(ESPNOW_ATTRIBUTE_KEY_1, false);
        s_espnow_ctrl_status = ESPNOW_CTRL_INIT;
		rgb_led_remote_waiting();
    }
}

void comunication_send(kind_control_data_t kindControlData, uint32_t data){
	if(s_espnow_ctrl_status==ESPNOW_CTRL_BOUND){
		espnow_ctrl_initiator_send(ESPNOW_ATTRIBUTE_KEY_1, kindControlData, data);
	}
}

void comunication_action_pairing(void){
	if(s_espnow_ctrl_status==ESPNOW_CTRL_INIT){
		comunication_bind();
	} else {
		comunication_unbind();
	}
}

void comunication_app_start(void)
{
	ESP_LOGI(TAG, "STARTING");

	// Start WiFi started LED
	s_espnow_ctrl_status = ESPNOW_CTRL_INIT;
	rgb_led_remote_waiting();

	// Disable default WiFi logging messages
	esp_log_level_set("wifi", ESP_LOG_NONE);

	wifi_init();

	espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_init(&espnow_config);
}