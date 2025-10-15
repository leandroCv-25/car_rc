#include "esp_log.h"
#include "esp_err.h"
#include "driver/ledc.h"

#include "drive_motor.h"

static const char *TAG = "drive motor";

static uint32_t g_full_duty = 0;
static drive_motor_config_t g_cfg;

static uint32_t calculate_duty(float percentage)
{
    uint32_t duty = (uint32_t)((float)g_full_duty * percentage / (100.0f));
    return duty;
}

static esp_err_t set_drive_motor_duty(uint32_t duty, ledc_channel_t channel)
{
    esp_err_t ret = ledc_set_duty(g_cfg.speed_mode, channel, duty);
    ret |= ledc_update_duty(g_cfg.speed_mode, channel);
    ESP_LOGI(TAG, "set duty %lu on channel %i", duty, channel);
    return ret;
}

static void validate_speed_percentage(float *percentage)
{
    if (*percentage < 0.0f)
    {
        *percentage= 0.0f;
    }
    if (*percentage > 100.0f)
    {
        *percentage = 100.0f;
    }
}

esp_err_t drive_motor_forward(float percentage)
{
    validate_speed_percentage(&percentage);

    uint32_t duty = calculate_duty(percentage);
    esp_err_t ret = set_drive_motor_duty(0, g_cfg.channel_reverse);
    ret |= set_drive_motor_duty(duty, g_cfg.channel_forward);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "forward is failing");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t drive_motor_reverse(float percentage)
{
    validate_speed_percentage(&percentage);

    uint32_t duty = calculate_duty(percentage);
    esp_err_t ret = set_drive_motor_duty(0, g_cfg.channel_forward);
    ret |= set_drive_motor_duty(duty, g_cfg.channel_reverse);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Reverse is failing");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t drive_motor_break()
{
    esp_err_t ret = set_drive_motor_duty(0, g_cfg.channel_forward);
    ret |= set_drive_motor_duty(0, g_cfg.channel_reverse);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "break is failing");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t drive_motor_init(const drive_motor_config_t *config)
{
    esp_err_t ret;

    if (config == NULL)
    {
        ESP_LOGE(TAG, "Pointer of config is invalid");
        return ESP_ERR_INVALID_ARG;
    }

    g_cfg = *config;

    g_full_duty = (1 << g_cfg.resolution) - 1;

    ledc_timer_config_t ledc_timer = {
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = g_cfg.resolution, // resolution of PWM duty
        .freq_hz = 5000,                     // frequency of PWM signal
        .speed_mode = g_cfg.speed_mode,      // timer mode
        .timer_num = g_cfg.timer             // timer index
    };
    ret = ledc_timer_config(&ledc_timer);

    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "ledc timer configuration failed");
        return ESP_FAIL;
    }

    ledc_channel_config_t ledc_chForward = {
        .intr_type = LEDC_INTR_DISABLE,
        .channel = g_cfg.channel_forward,
        .duty = 0,
        .gpio_num = g_cfg.gpio_forward,
        .speed_mode = g_cfg.speed_mode,
        .timer_sel = g_cfg.timer,
        .hpoint = 0};

    ret = ledc_channel_config(&ledc_chForward);

    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "ledc channel configuration failed");
        return ESP_FAIL;
    }

    ledc_channel_config_t ledc_chReverse = {
        .intr_type = LEDC_INTR_DISABLE,
        .channel = g_cfg.channel_reverse,
        .duty = 0,
        .gpio_num = g_cfg.gpio_reverse,
        .speed_mode = g_cfg.speed_mode,
        .timer_sel = g_cfg.timer,
        .hpoint = 0};

    ret = ledc_channel_config(&ledc_chReverse);

    if (ESP_OK != ret)
    {
        ESP_LOGE(TAG, "ledc channel configuration failed");
        return ESP_FAIL;
    }

    return ESP_OK;
}
