#ifndef REMOTE_APP_H_
#define REMOTE_APP_H_

#include "driver/gpio.h"

typedef enum
{
    REMOTE_MSG_OK = 0,
    REMOTE_MSG_ERROR_WAKEUP,
    REMOTE_MSG_ERROR_ACCE,
} remote_msg_e;

// Callback typedef
typedef void (*app_remote_callback_t)(int32_t *angle, int32_t *speed, remote_msg_e *msg);

void set_sensor_on();

esp_err_t remote_init(gpio_num_t sclGpio, gpio_num_t sdaGpio, uint16_t addr, app_remote_callback_t remoteCb);

#endif