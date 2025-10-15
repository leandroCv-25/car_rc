#ifndef DRIVE_MOTOR_H_
#define DRIVE_MOTOR_H_

#include "esp_err.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

/**
 * @brief Configuration of drive motor
 * 
 */
typedef struct {
    uint8_t gpio_forward; 
    ledc_channel_t channel_forward;
    uint8_t gpio_reverse; 
    ledc_channel_t channel_reverse;
    ledc_mode_t speed_mode;
    ledc_timer_t timer;
    ledc_timer_bit_t resolution;
} drive_motor_config_t;

/**
 * @brief Initialize ledc to control the drive motor
 * 
 * @param config Pointer of drive motor configure struct
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Configure ledc failed
 */
esp_err_t drive_motor_init(const drive_motor_config_t *config);


/**
 * @brief Set the drive motor to move forward
 * 
 * @param speed The speed in percentage
 * 
 * @return
 *     - ESP_OK Success
 */
esp_err_t drive_motor_forward(float percentage);

/**
 * @brief Set the drive motor to move reverse
 * 
 * @param speed The speed in percentage
 * 
 * @return
 *     - ESP_OK Success
 */
esp_err_t drive_motor_reverse(float percentage);

/**
 * @brief Set the drive motor to stop
 * 
 * @return
 *     - ESP_OK Success
 */
esp_err_t drive_motor_break();


#endif