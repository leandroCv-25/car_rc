#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "device_params.h"

#include "app_nvs.h"
#include "button_app.h"
#include "car_model.h"
#include "app_communication.h"

#include "servo.h"
#include "drive_motor.h"
#include "car_app.h"
#include "rgb_led.h"
#include "headlight.h"
#include "deep_sleep_app.h"

button_handle_t communicationButon;

uint8_t peerMac[6];
uint8_t newPeerMac[6];
uint8_t numberOfDevices;
bool isResquestedPairing = false;

static servo_config_t servoCfgLs = {
    .max_angle = 180,
    .min_width_us = 500,
    .max_width_us = 2500,
    .freq = 500,
    .gpio = GPIO_NUM_13,
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

void communication_button_cb();
void communication_cb(bool isBroadcast, uint8_t *srcAddress, const uint8_t *data, int len);

void app_main(void)
{

    ESP_LOGI("MAIN", "TESTE");
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

    rgb_led_config(GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_23, LEDC_TIMER_0, LEDC_CHANNEL_3, LEDC_CHANNEL_4, LEDC_CHANNEL_5);
    headlight_config(GPIO_NUM_25);

    communicationButon = button_app_init(GPIO_NUM_15, 0);

    resgister_event_callback(communicationButon, BUTTON_LONG_PRESS_START, 8000, &communication_button_cb, NULL);

    // start comunication
    configAppCommunication(&communication_cb);

    // start car
    car_init(&driveMotorConfig, &servoCfgLs);
}

void communication_button_cb()
{
    ESP_LOGI("MAIN", "Long press detected - Clear paired devices and restart");

    if (isResquestedPairing)
    {
        isResquestedPairing = false;
        char msg_str[16];
        sprintf(msg_str, "{\n\"event\": %01u\n}", CAR_EVENT_NEW_CONNECTION);
        appCommunicationEventWithData((uint8_t *)msg_str, newPeerMac);

        DelPeerAppCommunication(peerMac);

        memcpy(peerMac, newPeerMac, 6);
        AddPeerAppCommunication(peerMac);

        numberOfDevices = 1;
        app_nvs_save_number_devices(numberOfDevices);
        app_nvs_save_mac(numberOfDevices, peerMac);
    }
}

bool isSameMac(uint8_t *mac1, uint8_t *mac2)
{
    bool isSmMac = true;

    for (int i = 0; i < 6; i++)
    {
        isSmMac = isSmMac && (mac1[i] == mac2[i]);
    }

    return isSmMac;
}

void communication_cb(bool isBroadcast, uint8_t *srcAddress, const uint8_t *data, int len)
{
    if (isBroadcast)
    {
        ESP_LOGI("MAIN", "BROADCAST MSG");

        ESP_LOGI("MAIN", "SRC ADDRESS: %02X:%02X:%02X:%02X:%02X:%02X", srcAddress[0], srcAddress[1], srcAddress[2], srcAddress[3], srcAddress[4], srcAddress[5]);
        ESP_LOGI("MAIN", "DATA LEN: %d", len);
        ESP_LOGI("MAIN", "DATA: %s", (char *)data);

        car_event_e event;

        char msg[len];
        strcpy(msg, (char *)data);

        int ret = sscanf(msg, "{\n\"Event\": %01u\n}", &event);

        if (ret == 1 && event == CAR_EVENT_NEW_CONNECTION)
        {
            isResquestedPairing = true;
            memcpy(newPeerMac, srcAddress, 6);
            ESP_LOGI("MAIN", "New connection requested from %02X:%02X:%02X:%02X:%02X:%02X", newPeerMac[0], newPeerMac[1], newPeerMac[2], newPeerMac[3], newPeerMac[4], newPeerMac[5]);

            if (isSameMac(srcAddress, peerMac))
            {
                DelPeerAppCommunication(peerMac);
                numberOfDevices = 0;
                app_nvs_save_number_devices(numberOfDevices);
            }

            rgb_led_car_waiting();
        }
    }
    else
    {
        ESP_LOGI("MAIN", "UNICAST MSG");

        ESP_LOGI("MAIN", "SRC ADDRESS: %02X:%02X:%02X:%02X:%02X:%02X", srcAddress[0], srcAddress[1], srcAddress[2], srcAddress[3], srcAddress[4], srcAddress[5]);
        ESP_LOGI("MAIN", "DATA LEN: %d", len);
        ESP_LOGI("MAIN", "DATA: %s", (char *)data);

        car_event_e event;

        char msg[len];
        strcpy(msg, (char *)data);

        int32_t steering;
        int32_t speed;
        unsigned int headlightOn;

        int ret = sscanf(msg, "{\n\"Event\": %01u,\n\"Steering\":%5ld,\n\"Speed\":%5ld\n}", &event, &steering, &speed);

        if (ret == 1 && event == CAR_EVENT_DRIVING)
        {
            ESP_LOGI("MAIN", "Steering: %ld", steering);
            ESP_LOGI("MAIN", "Speed: %ld", speed);

            if (steering >= 0 && steering <= 180)
            {
                car_app_send_message(CAR_STERING, (float)steering);
            }

            if (speed > 0)
            {
                car_app_send_message(CAR_DRIVE, (float)speed / 100.00);
            }
            else if (speed < 0)
            {
                car_app_send_message(CAR_REVERSE, (float)speed / 100.00);
            }
            else
            {
                car_app_send_message(CAR_STOP, 0);
            }
        }
        else
        {

            ret = sscanf(msg, "{\n\"Event\": %01u,\n\"Headlight\":%01u\n}", &event, &headlightOn);

            if (ret == 1 && event == CAR_EVENT_HEADLIGHT)
            {
                ESP_LOGI("MAIN", "Headlight: %01u", headlightOn);
                if (headlightOn == 1)
                {
                    headlight_toggle(true);
                }
                else
                {
                    headlight_toggle(false);
                }
            }
        }
    }
}
