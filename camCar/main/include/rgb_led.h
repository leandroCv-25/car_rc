#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

#include "driver/ledc.h"
#include "driver/gpio.h"

//RGB LED Color mix channels
#define RGB_LED_CHANNEL_NUM 3

//RGB LED configuration
typedef struct
{
    int channel;
    int gpio;
    int mode;
    int timer_index;
}ledc_info_t;

ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM];

void rgb_led_config(gpio_num_t ledBlue, gpio_num_t ledGreen, gpio_num_t ledRed, ledc_timer_t timer, ledc_channel_t redChannel,ledc_channel_t blueChannel,ledc_channel_t greenChannel);

//Color to indicate WiFi aplication has started
void rgb_led_wifi_app_started(void);

//Color to indicate http server has started
void rgb_led_http_server_started(void);

//Color to indicate Wifi is connected
void rgb_led_wifi_connected(void);


#endif