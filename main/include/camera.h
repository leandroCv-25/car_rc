#ifndef CAMERA_H_
#define CAMERA_H_

#include "esp_camera.h"


/**
 * @brief Initialize ledc to control the drive motor
 * 
 * @param config Pointer of drive motor configure struct
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Configure camera failed
 */
esp_err_t camera_init(camera_config_t* camera_config);


#endif