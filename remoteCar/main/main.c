#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#include "comunication_app.h"
#include "rgb_led.h"
#include "remote_app.h"
#include "comunication_button.h"

adc_config_t adc_config = {
        .adcChannel = ADC_CHANNEL_6,
        .adcWidth = ADC_WIDTH_BIT_12,
        .adcAtten = ADC_ATTEN_DB_11,
        .adcUnit  = ADC_UNIT_1,
    };

void app_main(void)
{
    gpio_num_t comunicationButon = GPIO_NUM_5;

    gpio_num_t buttonGpio = GPIO_NUM_35; 
    gpio_num_t sclGpio = GPIO_NUM_18;
    gpio_num_t sdaGpio = GPIO_NUM_19;

    uint16_t addrSensor = 0x68u;

    // INITIALISE NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_timer_early_init();

    rgb_led_config(GPIO_NUM_23, GPIO_NUM_22, GPIO_NUM_21, LEDC_TIMER_0, LEDC_CHANNEL_3, LEDC_CHANNEL_4, LEDC_CHANNEL_5);

    // start comunication
    comunication_app_start();

    comunication_button_config(comunicationButon);

    remote_init(&adc_config, buttonGpio, sclGpio, sdaGpio, addrSensor);
}