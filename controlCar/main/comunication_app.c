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

#include "tasks_common.h"
#include "comunication_app.h"
#include "car_app.h"
#include "rgb_led.h"
#include "headlight.h"

// Tag used for ESP serial console messages
static const char TAG[] = "comunication_app";

// control status
static espnow_ctrl_status_t s_espnow_ctrl_status = ESPNOW_CTRL_INIT;

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

static void comunication_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
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
		ESP_LOGI(TAG, "bind, uuid: " MACSTR ", initiator_type: %d", MAC2STR(info->mac), info->initiator_attribute);

		rgb_led_car_connected();
		break;
	}

	case ESP_EVENT_ESPNOW_CTRL_UNBIND:
	{
		espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
		ESP_LOGI(TAG, "unbind, uuid: " MACSTR ", initiator_type: %d", MAC2STR(info->mac), info->initiator_attribute);

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

	esp_event_handler_register(ESP_EVENT_ESPNOW, ESP_EVENT_ANY_ID, comunication_event_handler, NULL);
}

static esp_err_t receive_data_handle(uint8_t *src_addr, void *data,
									 size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl)
{
	ESP_PARAM_CHECK(src_addr);
	ESP_PARAM_CHECK(data);
	ESP_PARAM_CHECK(size);
	ESP_PARAM_CHECK(rx_ctrl);

	static uint32_t count = 0;

	ESP_LOGI(TAG, "espnow_recv, <%d> [" MACSTR "][%d][%d][%d]: %.*s",
			 count++, MAC2STR(src_addr), rx_ctrl->channel, rx_ctrl->rssi, size, size, (char *)data);

	double angle;
	double aceleration;
	unsigned int light;
	char gear;

	

	sscanf(data, "{\"angle\":%le,\"aceleration\":%le,\"light\":%u,\"gear\":%c}", &angle, &aceleration, &light, &gear);

	headlight_set(light);

	switch (gear)
	{
	case 'D':
		car_app_send_message(CAR_DRIVE, aceleration, angle);
		break;
	case 'R':
		car_app_send_message(CAR_REVERSE, aceleration, angle);
		break;
	default:
		car_app_send_message(CAR_STOP, aceleration, angle);
		break;
	}

	return ESP_OK;
}

/**
 * Main task for the Comunication application
 * @param pvParameters parameter which can be passed to the task
 */
static void comunication_app_task(void *pvParameters)
{
	// Initialize the event handler
	comunication_app_event_handler_init();

	// Initialize the TCP/IP stack and WiFi config
	wifi_init();

	espnow_set_type(ESPNOW_TYPE_DATA, 1, receive_data_handle);

	while (true)
	{
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void comunication_app_start(void)
{
	ESP_LOGI(TAG, "STARTING");

	// Start WiFi started LED
	rgb_led_car_waiting();

	// Disable default WiFi logging messages
	esp_log_level_set("wifi", ESP_LOG_NONE);

	// Start the WiFi application task
	xTaskCreatePinnedToCore(&comunication_app_task, "comunication_app_task", COMUNICATION_APP_TASK_STACK_SIZE, NULL, COMUNICATION_APP_TASK_PRIORITY, NULL, COMUNICATION_APP_TASK_CORE_ID);
}