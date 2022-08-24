#ifndef SERVO_H_
#define SERVO_H_

#include "esp_err.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

/**
 * @brief servo motor's freq max and min 
 * 
 */
#define SERVO_FREQ_MIN       50
#define SERVO_FREQ_MAX       400

/**
 * @brief Configuration of servo motor
 * 
 */
typedef struct {
    uint16_t max_angle;        /**< Servo max angle */
    uint16_t min_width_us;     /**< Pulse width corresponding to minimum angle, which is usually 500us */
    uint16_t max_width_us;     /**< Pulse width corresponding to maximum angle, which is usually 2500us */
    uint32_t freq;             /**< PWM frequency */
    uint8_t gpio; 
    ledc_channel_t channel;
    ledc_mode_t speed_mode;
    ledc_timer_t timer;
    ledc_timer_bit_t resolution;
} servo_config_t;

/**
 * @brief Initialize ledc to control the servo
 * 
 * @param config Pointer of servo configure struct
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Configure ledc failed
 */
esp_err_t servo_init(const servo_config_t *config);


/**
 * @brief Set the servo motor to a certain angle
 * 
 * @param angle The angle to go
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t servo_set_angle(float angle);


#endif