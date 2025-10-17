#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "espnow.h"

#include "driver/i2c.h"
#include "mpu6050.h"

#include "remote_app.h"
#include "device_params.h"

static const char *TAG = "Remote App";

bool isSensorOn = true;

mpu6050_handle_t mpu6050Handle;

app_remote_callback_t _remoteCb;

static void remote_app_task(void *pvParameters)
{
    mpu6050_acce_value_t acce_value;
    int32_t aceleration;
    int32_t angle;

    while (true)
    {
        remote_msg_e msg = REMOTE_MSG_OK;

        if (isSensorOn)
        {
            if (mpu6050_wake_up(mpu6050Handle) == ESP_OK)
            {
                if (mpu6050_get_acce(mpu6050Handle, &acce_value) == ESP_OK)
                {
                    mpu6050_sleep(mpu6050Handle);
                    ESP_LOGI(TAG, "acce x: %f\tacce y: %f\tacce z:%f\n", acce_value.acce_x, acce_value.acce_y, acce_value.acce_z);

                    angle = (int32_t)(atan(acce_value.acce_y / acce_value.acce_x));

                    ESP_LOGI(TAG, "angle: %ld", angle);

                    aceleration = (int32_t)(atan(acce_value.acce_z / acce_value.acce_x));

                    ESP_LOGI(TAG, "aceleration: %ld", aceleration);

                    _remoteCb(&angle, &aceleration, &msg);
                }
                else
                {
                    ESP_LOGE(TAG, "Error get acce");
                    msg = REMOTE_MSG_ERROR_ACCE;
                    _remoteCb(&angle, &aceleration, &msg);
                }
            }
            else
            {
                ESP_LOGE(TAG, "Error wake up");
                angle = 0;
                aceleration = 3500;
                msg = REMOTE_MSG_ERROR_WAKEUP;
                _remoteCb(&angle, &aceleration, &msg);

                vTaskDelay(5000 / portTICK_PERIOD_MS);

                angle = 3000;
                aceleration = 500;
                _remoteCb(&angle, &aceleration, &msg);

                vTaskDelay(5000 / portTICK_PERIOD_MS);

                angle = 10000;
                aceleration = 1505;
                _remoteCb(&angle, &aceleration, &msg);
            }
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void set_sensor_on()
{
    isSensorOn = !isSensorOn;
}

esp_err_t remote_init(gpio_num_t sclGpio, gpio_num_t sdaGpio, uint16_t addr, app_remote_callback_t remoteCb)
{
    ESP_LOGI(TAG, "STARTING");

    _remoteCb = remoteCb;

    int i2c_master_port = 0;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sdaGpio,
        .scl_io_num = sclGpio,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };

    i2c_param_config(i2c_master_port, &conf);

    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);

    // ESP_LOGI(TAG, "I2C initialized successfully");

    mpu6050Handle = mpu6050_create(i2c_master_port, addr);
    mpu6050_config(mpu6050Handle, ACCE_FS_2G, GYRO_FS_250DPS);

    // Start the Driving application task
    xTaskCreatePinnedToCore(&remote_app_task, "remote_app_task", REMOTE_APP_TASK_STACK_SIZE, NULL, REMOTE_APP_TASK_PRIORITY, NULL, REMOTE_APP_TASK_CORE_ID);
    return ESP_OK;
}
