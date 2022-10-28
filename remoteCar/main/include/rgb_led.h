#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

#include "driver/ledc.h"
#include "driver/gpio.h"

//RGB LED Color mix channels
#define RGB_LED_CHANNEL_NUM 3

//RGB LED configuration
typedef struct
{
    ledc_channel_t channel;
    gpio_num_t gpio;
    int mode;
    ledc_timer_t timer_index;
}ledc_info_t;

ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM];

void rgb_led_config(gpio_num_t ledBlue, gpio_num_t ledGreen, gpio_num_t ledRed, ledc_timer_t timer, ledc_channel_t redChannel,ledc_channel_t blueChannel,ledc_channel_t greenChannel);

//Color to indicate car is connected
void rgb_led_remote_connected(void);

//Color to indicate car has a problem
void rgb_led_remote_error(void);

//Color to indicate car is waiting connection
void rgb_led_remote_waiting(void);


#endif