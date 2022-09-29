#include "stdio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_http_server.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lwip/netdb.h"

#include "http_server.h"
#include "camera.h"
#include "tasks_common.h"

// Tag used for ESP serial console messages
static const char TAG[] = "http_server";

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// Queue handle used to manipulate the frame events
static QueueHandle_t queueFrameHandle;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u";

static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *frame = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];

    ESP_LOGI(TAG, "Stream requested");
    
    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
    {
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

    while (true)
    {
        if (xQueueReceive(queueFrameHandle, &frame, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Stream go1");

             if (frame->format == PIXFORMAT_JPEG) {
                _jpg_buf = frame->buf;
                _jpg_buf_len = frame->len;
            }
            ESP_LOGI(TAG, "Stream go2");

            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
            ESP_LOGI(TAG, "Stream go3");
            if (res == ESP_OK)
            {
                size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len);
                res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
                ESP_LOGI(TAG, "Stream go4");
            }
            if (res == ESP_OK)
            {
                res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
                ESP_LOGE(TAG,"%i", res);
                ESP_LOGI(TAG, "Stream go5");
            }

            ESP_LOGI(TAG, "Stream go6");

            esp_camera_fb_return(frame);

            if (res != ESP_OK)
            {
                ESP_LOGE(TAG, "Break stream handler");
                break;
            }
        }
    }
    return res;
}

/**
 * Sets up the default httpd server configuration.
 * @return http server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
    // Generate the default configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // The core that the HTTP server will run on
    config.core_id = HTTP_SERVER_TASK_CORE_ID;

    // Adjust the default priority to 1 less than the wifi application task
    config.task_priority = HTTP_SERVER_TASK_PRIORITY;

    // Bump up the stack size (default is 4096)
    config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

    // Increase uri handlers
    config.max_uri_handlers = 20;

    // Increase the timeout limits
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;

    // Create the message queue
    queueFrameHandle = xQueueCreate(2, sizeof(camera_fb_t *));

    ESP_LOGI(TAG,
             "http_server_configure: Starting server on port: '%d' with task priority: '%d'",
             config.server_port,
             config.task_priority);

    // Start the httpd server
    if (httpd_start(&http_server_handle, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "http_server_configure: Registering URI handlers");

        httpd_uri_t stream = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = stream_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(http_server_handle, &stream);

        return http_server_handle;
    }

    return NULL;
}

void http_server_start(void)
{
    if (http_server_handle == NULL)
    {
        http_server_handle = http_server_configure();
    }
}

BaseType_t frame_send(camera_fb_t *frame)
{
    return xQueueSend(queueFrameHandle, frame, portMAX_DELAY);
}