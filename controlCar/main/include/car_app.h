#ifndef DRIVING_APP_H_
#define DRIVING_APP_H_

#include "servo.h"
#include "drive_motor.h"

/**
 * Message IDs for the car application task
 */
typedef enum car_app_message
{
    CAR_STERING,
    CAR_STOP,
    CAR_DRIVE,
    CAR_REVERSE,
} car_app_message_e;

/**
 * Structure for the message queue
 * @note Expand this based on application requirements e.g. add another type and parameter as required
 */
typedef struct car_app_queue_message
{
    car_app_message_e msgID;
    float data;
} car_app_queue_message_t;

/**
 * Sends a message to the queue
 * @param msgID message ID from the car_app_message_e enum.
 * @param data double data drive car.
 * @return pdTRUE if an item was successfully sent to the queue, otherwise pdFALSE.
 * @note Expand the parameter list based on your requirements e.g. how you've expanded the wifi_app_queue_message_t.
 */
BaseType_t car_app_send_message(car_app_message_e msgID, float data);

/**
 * @brief Config and start the car task
 *
 * @param driveMotorConfig pointer to the config of the drive Motor
 * @param servoCfgLs pointer to the config of the servo Motor ("Wheels")
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Configure failed
 */
esp_err_t car_init(drive_motor_config_t *driveMotorConfig, servo_config_t *servoCfgLs);

#endif