#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/dac.h>

#include "servo.h"
#include "esp_log.h"

void app_main(void)
{
    servo_config_t servo_cfg_ls = {
        .max_angle = 180,
        .min_width_us = 500,
        .max_width_us = 2500,
        .freq = 50
    };
    servo_init(&servo_cfg_ls);


    while (true)
    {
        
            ESP_LOGI("MAIN", "angle: 7");
            servo_set_angle(7);
            vTaskDelay(3000 / portTICK_RATE_MS);
            ESP_LOGI("MAIN", "angle: 25");
            servo_set_angle(25);
            vTaskDelay(3000 / portTICK_RATE_MS);
            ESP_LOGI("MAIN", "angle: 43");
            servo_set_angle(43);
            vTaskDelay(3000 / portTICK_RATE_MS);
        
        
    }
}