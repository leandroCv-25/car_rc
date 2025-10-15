#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "comunication_app.h"
#include "rgb_led.h"
#include "remote_app.h"
#include "button_app.h"
#include "deep_sleep_app.h"

button_handle_t comunicationButon;
button_handle_t headlightButon;
button_handle_t sensorButon;

gpio_num_t buttonGpio = GPIO_NUM_32;
gpio_num_t sclGpio = GPIO_NUM_19;
gpio_num_t sdaGpio = GPIO_NUM_18;

void communication_cb();
void headlight_cb();

void remote_cb(uint32_t *angle, uint32_t *speed, remote_msg_e *msg);

void app_main(void)
{

    ESP_LOGI("MAIN", "TESTE");

    uint16_t addrSensor = 0x68u;

    // INITIALISE NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    config_deep_sleep();

    rgb_led_config(GPIO_NUM_23, GPIO_NUM_22, GPIO_NUM_21, LEDC_TIMER_0, LEDC_CHANNEL_3, LEDC_CHANNEL_4, LEDC_CHANNEL_5);

    // start comunication
    // comunication_app_start();

    comunicationButon = button_app_init(GPIO_NUM_5, 0);
    headlightButon = button_app_init(GPIO_NUM_13, 0);
    // sensorButon = button_app_init(GPIO_NUM_27, 0);

    resgister_event_callback(comunicationButon, BUTTON_LONG_PRESS_START, 8000, &communication_cb, NULL);
    resgister_event_callback(headlightButon, BUTTON_SINGLE_CLICK, 1000, &headlight_cb, NULL);
    // resgister_event_callback(sensorButon, BUTTON_SINGLE_CLICK, 1000, &set_sensor_on, NULL);

    remote_init(sclGpio, sdaGpio, addrSensor, &remote_cb);
}

void communication_cb()
{
    ESP_LOGI("MAIN", "COMMUNICATION");
}

void headlight_cb()
{
    ESP_LOGI("MAIN", "HEADLIGHT");
}

void remote_cb(uint32_t *angle, uint32_t *speed, remote_msg_e *msg)
{
    if (*msg == REMOTE_MSG_OK)
    {
        ESP_LOGE("MAIN", "REMOTE ERROR: %d", *msg);
        rgb_led_remote_connected();
    }
    else if (*msg == REMOTE_MSG_ERROR_WAKEUP)
    {
        ESP_LOGE("MAIN", "REMOTE ERROR: %d", *msg);
        rgb_led_remote_error_wakeup();
    }
    else if (*msg == REMOTE_MSG_ERROR_ACCE)
    {
        ESP_LOGE("MAIN", "REMOTE ERROR: %d", *msg);
        rgb_led_remote_error_acce();
    }
    ESP_LOGI("MAIN", "ANGLE: %lu - SPEED: %lu", *angle, *speed);
}