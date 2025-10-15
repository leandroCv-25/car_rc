#include "esp_log.h"
#include "esp_err.h"
#include "driver/ledc.h"

#include "servo.h"

static const char *TAG = "servo";

static uint32_t g_full_duty =0;
static servo_config_t g_cfg;

static uint32_t calculate_duty(float angle)
{
    // ESP_LOGI(TAG,"Angle: %f",angle);

    angle += 25.5f;
    float angle_us = angle / g_cfg.max_angle * (g_cfg.max_width_us - g_cfg.min_width_us) + g_cfg.min_width_us;
    ESP_LOGI(TAG, "angle us: %f", angle_us);
    uint32_t duty = g_full_duty- (uint32_t)((float)g_full_duty * (angle_us) * g_cfg.freq / (1000000.0f));
    ESP_LOGI(TAG, "duty: %lu ", duty);
    return duty;
}

esp_err_t servo_set_angle(float angle)
{
    if (angle < -18.5f) // 7
    {
       angle = -18.5f;
    }
    if (angle > 18.5f) //44
    {
        angle = 18.5f;
    }
    
    uint32_t duty = calculate_duty(angle);
    esp_err_t ret = ledc_set_duty(g_cfg.speed_mode, g_cfg.channel, duty);
    ret |= ledc_update_duty(g_cfg.speed_mode, g_cfg.channel);

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

    g_full_duty = (1 << g_cfg.resolution) - 1;

    ESP_LOGI(TAG, "Servo pwm frequency %lu",g_cfg.freq);

    ledc_timer_config_t ledc_timer = {
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = g_cfg.resolution, // resolution of PWM duty
        .freq_hz = g_cfg.freq,                 // frequency of PWM signal
        .speed_mode = g_cfg.speed_mode,           // timer mode
        .timer_num = g_cfg.timer                 // timer index
    };
    ret = ledc_timer_config(&ledc_timer);

    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "ledc timer configuration failed");
        return ESP_FAIL;
    }

    ledc_channel_config_t ledc_ch = {
        .intr_type = LEDC_INTR_DISABLE,
        .channel = g_cfg.channel,
        .duty = calculate_duty(0),
        .gpio_num = g_cfg.gpio,
        .speed_mode = g_cfg.speed_mode,
        .timer_sel = g_cfg.timer,
        .hpoint = 0};
    
    ret = ledc_channel_config(&ledc_ch);

    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "ledc channel configuration failed");
        return ESP_FAIL;
    }

    return ESP_OK;
}


