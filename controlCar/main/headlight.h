#ifndef HEADLIGHT_H_
#define HEADLIGHT_H_

#include "driver/gpio.h"

void headlight_config(gpio_num_t led);

void headlight_toggle(bool value);

#endif