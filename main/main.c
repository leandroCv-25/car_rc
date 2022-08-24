#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/dac.h>

#include "servo.h"
#include "drive_motor.h"
#include "esp_log.h"

void app_main(void)
{
    servo_config_t servo_cfg_ls = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = 50,
        .gpio = 25,
        .channel = LEDC_CHANNEL_0,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer = LEDC_TIMER_0,
        .resolution = LEDC_TIMER_10_BIT,
    };
    servo_init(&servo_cfg_ls);

    drive_motor_config_t driveMotorConfig = {
        .gpio_forward = 2,
        .channel_forward = LEDC_CHANNEL_1,
        .gpio_reverse = 4,
        .channel_reverse = LEDC_CHANNEL_2,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer = LEDC_TIMER_1,
        .resolution = LEDC_TIMER_10_BIT,
    };
    drive_motor_init(&driveMotorConfig);

    while (true)
    {

        for(int i=0; i <= 100;i++){
            ESP_LOGI("MAIN", "percentage speed: %i",i);
            drive_motor_forward(i);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
        
        
        drive_motor_break();
        vTaskDelay(30000 / portTICK_RATE_MS);
    }
}