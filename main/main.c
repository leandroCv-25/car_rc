#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_app.h"
#include "servo.h"
#include "drive_motor.h"
#include "car_app.h"
#include "wifi_reset_button.h"
#include "http_server.h"
#include "camera.h"

static const char TAG[] = "main";

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 10000000, // EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,   // QQVGA-QXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 40,                 // 0-63 lower number means higher quality
    .fb_count = 2,                      // if more than one, i2s runs in continuous mode. Use only with JPEG
    .grab_mode = CAMERA_GRAB_LATEST, // CAMERA_GRAB_LATEST. Sets when buffers should be filled
    .fb_location = CAMERA_FB_IN_PSRAM
};

static servo_config_t servoCfgLs = {
    .max_angle = 180,
    .min_width_us = 500,
    .max_width_us = 2500,
    .freq = 50,
    .gpio = GPIO_NUM_12,
    .channel = LEDC_CHANNEL_3,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .timer = LEDC_TIMER_2,
    .resolution = LEDC_TIMER_10_BIT,
};

static drive_motor_config_t driveMotorConfig = {
    .gpio_forward = GPIO_NUM_4,
    .channel_forward = LEDC_CHANNEL_1,
    .gpio_reverse = GPIO_NUM_2,
    .channel_reverse = LEDC_CHANNEL_2,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .timer = LEDC_TIMER_1,
    .resolution = LEDC_TIMER_10_BIT,
};

void wifi_application_connected_events(void)
{
    ESP_LOGI(TAG, "WiFi Application Connected!!");
    //camera init
     camera_init(&camera_config);
    car_init(&driveMotorConfig, &servoCfgLs);
    http_server_start();
}

void app_main(void)
{
    // PIN button
    // gpio_num_t WIFI_RESET_BUTTON = GPIO_NUM_13;

    // INITIALISE NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //start Wi_FI
    wifi_app_start();

    // Configure Wifi reset button
    // wifi_reset_button_config(WIFI_RESET_BUTTON);

    wifi_app_set_callback(&wifi_application_connected_events);
}