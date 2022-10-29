#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "espnow.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/i2c.h"
#include "mpu6050.h"

#include "remote_app.h"
#include "comunication_app.h"
#include "tasks_common.h"

static const char *TAG = "Remote App";

#define DEFAULT_VREF 1100
#define NO_OF_SAMPLES 64 // Multisampling

static esp_adc_cal_characteristics_t *adc_chars;

adc_config_t *config;
gpio_num_t headlightButton;

mpu6050_handle_t mpu6050Handle;

static void remote_app_task(void *pvParameters)
{
    mpu6050_acce_value_t acce_value;

    while (true)
    {

        if(gpio_get_level(headlightButton)){
            ESP_LOGI(TAG, "headlight: %d", 1);
            comunication_send(HEADLIGHT_CONTROL_DATA, 1);
        }

        mpu6050_wake_up(mpu6050Handle);
        mpu6050_get_acce(mpu6050Handle, &acce_value);
        mpu6050_sleep(mpu6050Handle);
        printf("acce x: %f\tacce y: %f\tacce z:%f\n", acce_value.acce_x, acce_value.acce_y, acce_value.acce_z);
        
        uint32_t angle = (uint32_t)(atan(acce_value.acce_y/acce_value.acce_x)*5727.273);

        comunication_send(ANGLE_CONTROL_DATA, angle);

        ESP_LOGI(TAG, "angle: %d", angle);

        uint32_t adc_reading = 0;

        // Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (config->adcUnit == ADC_UNIT_1)
            {
                adc_reading += adc1_get_raw(config->adcChannel);
            }
            else
            {
                int raw;
                adc2_get_raw(config->adcChannel, config->adcWidth, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;

        // Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        ESP_LOGI(TAG, "Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);

        if (voltage > 600 * config->adcAtten)
        {
            uint32_t aceleration = (voltage - 550 * config->adcAtten) / (5 * config->adcAtten);
            if (aceleration > 100)
                aceleration = 100.00;
            ESP_LOGI(TAG, "Aceleration: %d", aceleration);
            comunication_send(ACELERATION_CONTROL_DATA, aceleration);
        }
        else if (voltage < 500 * config->adcAtten)
        {
            uint32_t aceleration = (550 * config->adcAtten - voltage) / (5 * config->adcAtten);
            if (aceleration < -100)
                aceleration = -100;
            ESP_LOGI(TAG, "Aceleration: %d", aceleration);
            comunication_send(ACELERATION_CONTROL_DATA, aceleration);
        }
        else
        {
            ESP_LOGI(TAG, "Aceleration: 0");
            comunication_send(ACELERATION_CONTROL_DATA, 0);
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

esp_err_t remote_init(adc_config_t *adc_config, gpio_num_t buttonGpio, gpio_num_t sclGpio, gpio_num_t sdaGpio, uint16_t addr)
{
    ESP_LOGI(TAG, "STARTING");

    headlightButton = buttonGpio;

    // Configure the button and set the direction
    gpio_pad_select_gpio(headlightButton);
    gpio_set_direction(headlightButton, GPIO_MODE_INPUT);

    config = adc_config;

    // Configure ADC
    if (config->adcUnit == ADC_UNIT_1)
    {
        adc1_config_width(config->adcWidth);
        adc1_config_channel_atten(config->adcChannel, config->adcAtten);
    }
    else
    {
        adc2_config_channel_atten(config->adcChannel, config->adcAtten);
    }

    // Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(config->adcUnit, config->adcAtten, config->adcWidth, DEFAULT_VREF, adc_chars);

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
