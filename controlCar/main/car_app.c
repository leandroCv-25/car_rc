#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "lwip/netdb.h"

#include "car_app.h"
#include "device_params.h"

static const char *TAG = "Car App";

// Queue handle used to manipulate the main queue of events
static QueueHandle_t car_app_queue_handle;

static void car_app_task(void *pvParameters)
{
    car_app_queue_message_t msg;

    // Send first event message
    car_app_send_message(CAR_STOP, 0.0);

    while (true)
    {
        if (xQueueReceive(car_app_queue_handle, &msg, portMAX_DELAY))
        {
            switch (msg.msgID)
            {
            case CAR_STOP:
                ESP_LOGI(TAG, "car stop");
                drive_motor_break();
                break;
            case CAR_DRIVE:
                ESP_LOGI(TAG, "car drive");

                drive_motor_forward(msg.data);

                break;
            case CAR_REVERSE:
                ESP_LOGI(TAG, "car reverse");
                drive_motor_reverse(msg.data);
                break;
            case CAR_STERING:
                ESP_LOGI(TAG, "car stering");
                ESP_LOGI(TAG,"Angle: %f",msg.data);
                servo_set_angle(msg.data);
            default:
                break;
            }
        }
    }
}

BaseType_t car_app_send_message(car_app_message_e msgID, float data)
{
    car_app_queue_message_t msg;
    msg.msgID = msgID;
    msg.data = data;
    return xQueueSend(car_app_queue_handle, &msg, portMAX_DELAY);
}

esp_err_t car_init(drive_motor_config_t *driveMotorConfig, servo_config_t *servoCfgLs)
{
    ESP_LOGI(TAG, "STARTING DRIVING APPLICATION");

    ESP_LOGI(TAG, "servo config");
    if (servo_init(servoCfgLs) == ESP_FAIL)
    {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "drive motor config");
    if (drive_motor_init(driveMotorConfig) == ESP_FAIL)
    {
        return ESP_FAIL;
    }

    // Create the message queue
	car_app_queue_handle = xQueueCreate(3, sizeof(car_app_queue_message_t));

    // Start the Driving application task
    xTaskCreatePinnedToCore(&car_app_task, "car_app_task", CAR_APP_TASK_STACK_SIZE, NULL, CAR_APP_TASK_PRIORITY, NULL, CAR_APP_TASK_CORE_ID);
    return ESP_OK;
}
