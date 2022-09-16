#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_app.h"
#include "servo.h"
#include "drive_motor.h"
#include "esp_log.h"
#include "wifi_reset_button.h"
#include "http_server.h"
#include "camera.h"

static const char TAG[] = "main";

void wifi_application_connected_events(void)
{
	ESP_LOGI(TAG, "WiFi Application Connected!!");
    http_server_start();
}

void app_main(void)
{
    //PIN button
    gpio_num_t WIFI_RESET_BUTTON = GPIO_NUM_2;

    // INITIALISE NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // start Wi_FI
    wifi_app_start();

    // Configure Wifi reset button
    wifi_reset_button_config(WIFI_RESET_BUTTON);

    wifi_app_set_callback(&wifi_application_connected_events);

    camera_init();

    // servo_config_t servo_cfg_ls = {
    //     .max_angle = 180,
    //     .min_width_us = 500,
    //     .max_width_us = 2500,
    //     .freq = 50,
    //     .gpio = 25,
    //     .channel = LEDC_CHANNEL_0,
    //     .speed_mode = LEDC_HIGH_SPEED_MODE,
    //     .timer = LEDC_TIMER_0,
    //     .resolution = LEDC_TIMER_10_BIT,
    // };
    // servo_init(&servo_cfg_ls);

    // drive_motor_config_t driveMotorConfig = {
    //     .gpio_forward = 4,
    //     .channel_forward = LEDC_CHANNEL_1,
    //     .gpio_reverse = 2,
    //     .channel_reverse = LEDC_CHANNEL_2,
    //     .speed_mode = LEDC_HIGH_SPEED_MODE,
    //     .timer = LEDC_TIMER_1,
    //     .resolution = LEDC_TIMER_10_BIT,
    // };
    // drive_motor_init(&driveMotorConfig);
}