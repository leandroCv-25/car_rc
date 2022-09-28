#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_camera.h"
#include <esp_log.h>

#include "tasks_common.h"
#include "camera.h"
#include "http_server.h"

// Tag used for ESP serial console messages
static const char TAG[] = "Camera";

camera_fb_t *frame;

static void camera_task(void *pvParameters)
{

    while (true)
    {
        frame = esp_camera_fb_get();
        if (frame)
        {
            if (frame->format == PIXFORMAT_JPEG)
            {
                ws_frame_async_send(frame);
            }
            else
            {
                ESP_LOGE(TAG, "JPEG compression failed");
            }
        }

        vTaskDelay(20 / portTICK_RATE_MS);
    }
}

esp_err_t camera_init(camera_config_t *camera_config)
{
    // initialize the camera
    esp_err_t err = esp_camera_init(camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Init Failed");
        return err;
    }

    ESP_LOGI(TAG, "Task init");

    xTaskCreatePinnedToCore(&camera_task, "camera_task", CAMERA_TASK_STACK_SIZE, NULL, CAMERA_TASK_PRIORITY, NULL, CAMERA_TASK_CORE_ID);

    ESP_LOGI(TAG, "Init Ok");
    return ESP_OK;
}