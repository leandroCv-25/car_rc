#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "servo.h"
#include "drive_motor.h"
#include "car_app.h"
#include "comunication_app.h"
#include "rgb_led.h"
#include "headlight.h"

static servo_config_t servoCfgLs = {
    .max_angle = 180,
    .min_width_us = 500,
    .max_width_us = 2500,
    .freq = 50,
    .gpio = GPIO_NUM_12,
    .channel = LEDC_CHANNEL_2,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .timer = LEDC_TIMER_2,
    .resolution = LEDC_TIMER_10_BIT,
};

static drive_motor_config_t driveMotorConfig = {
    .gpio_forward = GPIO_NUM_4,
    .channel_forward = LEDC_CHANNEL_0,
    .gpio_reverse = GPIO_NUM_2,
    .channel_reverse = LEDC_CHANNEL_1,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .timer = LEDC_TIMER_1,
    .resolution = LEDC_TIMER_10_BIT,
};

void app_main(void)
{
    // INITIALISE NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    rgb_led_config(GPIO_NUM_23, GPIO_NUM_22, GPIO_NUM_21, LEDC_TIMER_0, LEDC_CHANNEL_3, LEDC_CHANNEL_4, LEDC_CHANNEL_5);
    headlight_config(GPIO_NUM_25);
    // start comunication
    comunication_app_start();

    // start car
    car_init(&driveMotorConfig, &servoCfgLs);
}