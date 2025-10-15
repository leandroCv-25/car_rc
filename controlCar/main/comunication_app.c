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
#include "esp_mac.h"


#include "espnow.h"
#include "espnow_ctrl.h"

#include "tasks_common.h"
#include "comunication_app.h"
#include "car_app.h"
#include "rgb_led.h"
#include "headlight.h"

// Tag used for ESP serial console messages
static const char TAG[] = "comunication_app";

// control status
static espnow_ctrl_status_t s_espnow_ctrl_status;

static void wifi_init(void)
{
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}


static void espnow_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
	if (base != ESP_EVENT_ESPNOW)
	{
		return;
	}

	switch (id)
	{
	case ESP_EVENT_ESPNOW_CTRL_BIND:
	{
		espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
		printf("bind, uuid: " MACSTR ", initiator_type: %u", MAC2STR(info->mac), info->initiator_attribute);

		rgb_led_car_connected();
		break;
	}

	case ESP_EVENT_ESPNOW_CTRL_UNBIND:
	{
		espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
		printf("unbind, uuid:  "MACSTR" , initiator_type: %u", MAC2STR(info->mac), info->initiator_attribute);

		rgb_led_car_waiting();
		break;
	}

	default:
		break;
	}
}

static void comunication_app_event_handler_init(void)
{
	// Event handler for the connection
	espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
	espnow_init(&espnow_config);

	esp_event_handler_register(ESP_EVENT_ESPNOW, ESP_EVENT_ANY_ID, espnow_event_handler, NULL);
}

static void espnow_ctrl_responder_data_cb(espnow_attribute_t initiator_attribute,
										  espnow_attribute_t responder_attribute,
										  uint32_t data)
{
	int d;
	ESP_LOGI(TAG, "espnow_ctrl_responder_recv, initiator_attribute: %d, responder_attribute: %d, value: %lu",
			 initiator_attribute, responder_attribute, data);
	switch (responder_attribute)
	{
	case 0:
		d = (int)data;
		car_app_send_message(CAR_STERING, (float)d / 100.00);
		break;
	case 1:
		ESP_LOGI(TAG, "Headlight: Toggle");
		headlight_toggle();
		break;
	case 2:
		ESP_LOGI(TAG, "Aceleration: %f", (float)data / 100.00);
		if ((float)data / 100.00 > 0)
		{
			car_app_send_message(CAR_DRIVE, data);
		}
		else if ((float)data / 100.00 < 0)
		{
			car_app_send_message(CAR_REVERSE, data);
		}
		else
		{
			car_app_send_message(CAR_STOP, 0);
		}
		break;

	default:
		ESP_LOGI(TAG, "comando %d dado %f", responder_attribute, (float)data / 100.00);
		break;
	}
}

void comunication_app_start(void)
{
	ESP_LOGI(TAG, "STARTING");

	// Start WiFi started LED
	rgb_led_car_waiting();
	s_espnow_ctrl_status = ESPNOW_CTRL_INIT;

	espnow_storage_init();

	// Disable default WiFi logging messages
	esp_log_level_set("wifi", ESP_LOG_NONE);
	wifi_init();



	comunication_app_event_handler_init();

	ESP_ERROR_CHECK(espnow_ctrl_responder_bind(30 * 1000, -55, NULL));
	espnow_ctrl_responder_data(espnow_ctrl_responder_data_cb);
}