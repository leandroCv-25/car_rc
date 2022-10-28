#include <stdbool.h>

#include "driver/ledc.h"
#include "rgb_led.h"

static gpio_num_t RGB_LED_BLUE_GPIO;
static gpio_num_t RGB_LED_GREEN_GPIO;
static gpio_num_t RGB_LED_RED_GPIO;
ledc_timer_t rgbTimer;
ledc_channel_t RED_Channel;
ledc_channel_t BLUE_Channel;
ledc_channel_t GREEN_Channel;
/*
    initializes the RGB LED settings per channel, including
    the GPIO for each color, mode and timer configuration.
*/

bool pwm_init_handle = false;

bool pwm_config_handle = false;

void rgb_led_config(gpio_num_t ledBlue, gpio_num_t ledGreen, gpio_num_t ledRed, ledc_timer_t timer, ledc_channel_t redChannel, ledc_channel_t blueChannel, ledc_channel_t greenChannel)
{
    RGB_LED_BLUE_GPIO = ledBlue;
    RGB_LED_GREEN_GPIO = ledGreen;
    RGB_LED_RED_GPIO = ledRed;

    rgbTimer = timer;
    RED_Channel = redChannel;
    BLUE_Channel = blueChannel;
    GREEN_Channel = greenChannel;

    pwm_config_handle = true;
}

static void rgb_led_pwm_init(void)
{
    if (pwm_config_handle)
    {
        int rgb_ch;

        // Red
        ledc_ch[0].channel = RED_Channel;
        ledc_ch[0].gpio = RGB_LED_RED_GPIO;
        ledc_ch[0].mode = LEDC_HIGH_SPEED_MODE;
        ledc_ch[0].timer_index = rgbTimer;

        // GREEN
        ledc_ch[1].channel = GREEN_Channel;
        ledc_ch[1].gpio = RGB_LED_GREEN_GPIO;
        ledc_ch[1].mode = LEDC_HIGH_SPEED_MODE;
        ledc_ch[1].timer_index = rgbTimer;

        // BLUE
        ledc_ch[2].channel = BLUE_Channel;
        ledc_ch[2].gpio = RGB_LED_BLUE_GPIO;
        ledc_ch[2].mode = LEDC_HIGH_SPEED_MODE;
        ledc_ch[2].timer_index = rgbTimer;

        // Configure timer
        ledc_timer_config_t ledc_timer = {
            .duty_resolution = LEDC_TIMER_8_BIT,
            .freq_hz = 100,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .timer_num = rgbTimer};

        ledc_timer_config(&ledc_timer);

        // config channels
        for (rgb_ch = 0; rgb_ch < RGB_LED_CHANNEL_NUM; rgb_ch++)
        {
            ledc_channel_config_t ledc_channel = {
                .channel = ledc_ch[rgb_ch].channel,
                .duty = 0,
                .hpoint = 0,
                .gpio_num = ledc_ch[rgb_ch].gpio,
                .intr_type = LEDC_INTR_DISABLE,
                .speed_mode = ledc_ch[rgb_ch].mode,
                .timer_sel = ledc_ch[rgb_ch].timer_index,
            };

            ledc_channel_config(&ledc_channel);
        }

        pwm_init_handle = true;
    }
}

// sets the RGB color
static void rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    // value should be 0 - 255 for 8 bit number
    ledc_set_duty(ledc_ch[0].mode, ledc_ch[0].channel, red);
    ledc_update_duty(ledc_ch[0].mode, ledc_ch[0].channel);

    ledc_set_duty(ledc_ch[1].mode, ledc_ch[1].channel, green);
    ledc_update_duty(ledc_ch[1].mode, ledc_ch[1].channel);

    ledc_set_duty(ledc_ch[2].mode, ledc_ch[2].channel, blue);
    ledc_update_duty(ledc_ch[2].mode, ledc_ch[2].channel);
}

// Color to indicate WiFi aplication has started
void rgb_led_remote_connected(void)
{
    if (pwm_init_handle == false)
    {
        rgb_led_pwm_init();
    }

    rgb_led_set_color(0, 255, 0);
}

void rgb_led_remote_error(void)
{
    if (pwm_init_handle == false)
    {
        rgb_led_pwm_init();
    }

    rgb_led_set_color(255, 0, 0);
}

// Color to indicate Wifi is connected
void rgb_led_remote_waiting(void)
{
    if (pwm_init_handle == false)
    {
        rgb_led_pwm_init();
    }

    rgb_led_set_color(0, 0, 255);
}