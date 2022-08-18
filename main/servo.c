#include "esp_log.h"
#include "esp_err.h"
#include "driver/ledc.h"

#include "servo.h"

static const char *TAG = "servo";

static uint32_t g_full_duty = (1 << SERVO_LEDC_INIT_BITS) - 1;;
static servo_config_t g_cfg;

static uint32_t calculate_duty(float angle)
{
    float angle_us = angle / g_cfg.max_angle * (g_cfg.max_width_us - g_cfg.min_width_us) + g_cfg.min_width_us;
    ESP_LOGI(TAG, "angle us: %f", angle_us);
    uint32_t duty = g_full_duty- (uint32_t)((float)g_full_duty * (angle_us) * g_cfg.freq / (1000000.0f));
    ESP_LOGI(TAG, "duty: %i ", duty);
    return duty;
}

esp_err_t servo_set_angle(float angle)
{
    if (angle < 7.0f)
    {
        ESP_LOGE(TAG, "Angle can't to be less than 7");
        return ESP_ERR_INVALID_ARG;
    }
    if (angle > 44.0f)
    {
        ESP_LOGE(TAG, "Angle can't to be more than 44");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t duty = calculate_duty(angle);
    esp_err_t ret = ledc_set_duty(SERVO_SPEEDMODE, SERVO_CHANNEL, duty);
    ret |= ledc_update_duty(SERVO_SPEEDMODE, SERVO_CHANNEL);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "set servo angle failed");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t servo_init(const servo_config_t *config)
{
    esp_err_t ret;

    if (config == NULL)
    {
        ESP_LOGE(TAG, "Pointer of config is invalid");
        return ESP_ERR_INVALID_ARG;
    }
    if (config->freq > SERVO_FREQ_MAX && config->freq < SERVO_FREQ_MIN)
    {
        ESP_LOGE(TAG, "Servo pwm frequency out the range");
        return ESP_ERR_INVALID_ARG;
    }

     g_cfg = *config;

    ledc_timer_config_t ledc_timer = {
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = SERVO_LEDC_INIT_BITS, // resolution of PWM duty
        .freq_hz = config->freq,                 // frequency of PWM signal
        .speed_mode = SERVO_SPEEDMODE,           // timer mode
        .timer_num = SERVO_TIMER                 // timer index
    };
    ret = ledc_timer_config(&ledc_timer);

    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "ledc timer configuration failed");
        return ESP_FAIL;
    }

    ledc_channel_config_t ledc_ch = {
        .intr_type = LEDC_INTR_DISABLE,
        .channel = SERVO_CHANNEL,
        .duty = calculate_duty(20),
        .gpio_num = SERVO_GPIO,
        .speed_mode = SERVO_SPEEDMODE,
        .timer_sel = SERVO_TIMER,
        .hpoint = 0};
    
    ret = ledc_channel_config(&ledc_ch);

    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "ledc channel configuration failed");
        return ESP_FAIL;
    }

    return ESP_OK;
}


