#include <stdbool.h>

#include "headlight.h"

static gpio_num_t headlightGPIO;
bool headlight_init_handle = false;
bool headlight_config_handle = false;

void headlight_config(gpio_num_t led)
{
    headlightGPIO = led;
    headlight_config_handle = true;
}

static void headLight_init(void)
{
    if (headlight_config_handle)
    {
        gpio_reset_pin(headlightGPIO);
        /* Set the GPIO as a push/pull output */
        gpio_set_direction(headlightGPIO, GPIO_MODE_OUTPUT);

        headlight_init_handle = true;
    }
}

void headlight_set(bool value)
{
    if (headlight_init_handle == false)
    {
        headLight_init();
    }

    gpio_set_level(headlightGPIO, value);
}
