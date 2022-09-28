#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include "esp_camera.h"

/**
 * @brief async send  camera frame function
 * 
 * @param frame camera_fb_t
 */
 void ws_frame_async_send(camera_fb_t *frame);

/**
 * @brief Starts the HTTP server.
 */
void http_server_start(void);

#endif /* HTTP_SERVER_H_ */