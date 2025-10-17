#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "car_model.h"
#include "device_params.h"

#include "app_communication.h"
#include "rgb_led.h"
#include "app_nvs.h"
#include "remote_app.h"
#include "button_app.h"
#include "deep_sleep_app.h"

button_handle_t communicationButon;
button_handle_t headlightButon;
button_handle_t sensorButon;

gpio_num_t sclGpio = GPIO_NUM_19;
gpio_num_t sdaGpio = GPIO_NUM_18;

uint8_t peerMac[6];
uint8_t numberOfDevices;
bool isPairing = false;

TaskHandle_t paringDisplayTaskHandle;

bool isHeadLightOn = false;

void communication_button_cb();
void headlight_cb();
void paring_display_task(void *args);
void remote_cb(int32_t *angle, int32_t *speed, remote_msg_e *msg);

void communication_cb(bool isBroadcast, uint8_t *srcAddress, const uint8_t *data, int len);

void app_main(void)
{

    ESP_LOGI("MAIN", "TESTE");

    uint16_t addrSensor = 0x68u;

    app_nvs_init();

    if (app_nvs_load_number_devices(&numberOfDevices) != ESP_OK && numberOfDevices == 1)
    {
        if (app_nvs_load_mac(1, peerMac) == ESP_OK)
        {

            AddPeerAppCommunication(peerMac);

            ESP_LOGI("MAIN", "MAC of PEER: %X:%X:%X:%X:%X:%X", peerMac[0], peerMac[1], peerMac[2], peerMac[3], peerMac[4], peerMac[5]);
        }
    }

    config_deep_sleep();

    rgb_led_config(GPIO_NUM_23, GPIO_NUM_22, GPIO_NUM_21, LEDC_TIMER_0, LEDC_CHANNEL_3, LEDC_CHANNEL_4, LEDC_CHANNEL_5);

    // start comunication
    // comunication_app_start();

    communicationButon = button_app_init(GPIO_NUM_5, 0);
    headlightButon = button_app_init(GPIO_NUM_13, 0);
    // sensorButon = button_app_init(GPIO_NUM_27, 0);

    resgister_event_callback(communicationButon, BUTTON_LONG_PRESS_START, 8000, &communication_button_cb, NULL);
    resgister_event_callback(headlightButon, BUTTON_SINGLE_CLICK, 1000, &headlight_cb, NULL);
    // resgister_event_callback(sensorButon, BUTTON_SINGLE_CLICK, 1000, &set_sensor_on, NULL);

    remote_init(sclGpio, sdaGpio, addrSensor, &remote_cb);

    configAppCommunication(&communication_cb);
}

void communication_button_cb()
{
    ESP_LOGI("MAIN", "COMMUNICATION");

    DelPeerAppCommunication(peerMac);

    numberOfDevices = 0;
    app_nvs_save_number_devices(numberOfDevices);

    isPairing = true;

    char msg_str[16];
    sprintf(msg_str, "{\n\"Event\": %01u\n}", CAR_EVENT_NEW_CONNECTION);

    appCommunicationBroadcastEventWithData((uint8_t *)msg_str);
    xTaskCreatePinnedToCore(&paring_display_task, "paring_display_task", PAIRING_DISPLAY_STACK, NULL, PAIRING_DISPLAY_PRIORITY, &paringDisplayTaskHandle, PAIRING_DISPLAY_CORE_ID);
}

void paring_display_task(void *args)
{
    rgb_led_remote_connected();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    rgb_led_remote_waiting();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void headlight_cb()
{
    isHeadLightOn = !isHeadLightOn;
    ESP_LOGI("MAIN", "HEADLIGHT");

    char msg_str[250];
    sprintf(msg_str, "{\n\"Event\": %01u,\n\"Headlight\":%01u\n}", CAR_EVENT_HEADLIGHT, isHeadLightOn);

    appCommunicationEventWithData((uint8_t *)msg_str, peerMac);
}

void remote_cb(int32_t *angle, int32_t *speed, remote_msg_e *msg)
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

    char msg_str[250];
    sprintf(msg_str, "{\n\"Event\": %01u,\n\"Steering\":%5ld,\n\"Speed\":%5ld\n}", CAR_EVENT_DRIVING, *angle, *speed);

    appCommunicationEventWithData((uint8_t *)msg_str, peerMac);

    ESP_LOGI("MAIN", "ANGLE: %lu - SPEED: %lu", *angle, *speed);
}

void communication_cb(bool isBroadcast, uint8_t *srcAddress, const uint8_t *data, int len)
{
    if (!isBroadcast)
    {
        ESP_LOGI("MAIN", "UNICAST MSG");

        ESP_LOGI("MAIN", "SRC ADDRESS: %02X:%02X:%02X:%02X:%02X:%02X", srcAddress[0], srcAddress[1], srcAddress[2], srcAddress[3], srcAddress[4], srcAddress[5]);
        ESP_LOGI("MAIN", "DATA LEN: %d", len);
        ESP_LOGI("MAIN", "DATA: %s", (char *)data);

        if (isPairing == true)
        {
            memcpy(peerMac, srcAddress, 6);
            AddPeerAppCommunication(peerMac);

            numberOfDevices++;
            app_nvs_save_number_devices(numberOfDevices);
            app_nvs_save_mac(numberOfDevices, peerMac);

            isPairing = false;

            if (paringDisplayTaskHandle != NULL)
            {
                vTaskDelete(paringDisplayTaskHandle);
                paringDisplayTaskHandle = NULL;
            }

            rgb_led_remote_connected();
        }
    }
}