#include <stdbool.h>

#include "headlight.h"

static gpio_num_t headlightGPIO;
bool headlight_init_handle = false;

void headlight_config(gpio_num_t led)
{
    headlightGPIO = led;
}

static void headLight_init(void)
{
    gpio_reset_pin(headlightGPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(headlightGPIO, GPIO_MODE_OUTPUT);

    headlight_init_handle = true;
}

void headlight_set(bool value)
{
    if (headlight_init_handle == false)
    {
        headLight_init();
    }

    gpio_set_level(headlightGPIO, value);
}
