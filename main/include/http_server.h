#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include "esp_camera.h"

/**
 * @brief send  camera frame to queue
 * 
 * @param frame camera_fb_t
 */
 BaseType_t frame_send(camera_fb_t *frame);

/**
 * @brief Starts the HTTP server.
 */
void http_server_start(void);

#endif /* HTTP_SERVER_H_ */