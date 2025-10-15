#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/rtc_io.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_sleep.h"
#include "nvs.h"
#include "driver/gpio.h"

#include "deep_sleep_app.h"
#include "tasks_common.h"

static void deep_sleep_task(void *args)
{

    while (true)
    {

        if (!gpio_get_level(switch_gpio))
        {

#if CONFIG_IDF_TARGET_ESP32
            // Isolate GPIO12 pin from external circuits. This is needed for modules
            // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
            // to minimize current consumption.
            rtc_gpio_isolate(GPIO_NUM_12);
#endif

            esp_deep_sleep_start();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void config_deep_sleep()
{
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(switch_gpio, 1));

    // Configure pullup/downs via RTCIO to tie wakeup pins to inactive level during deepsleep.
    // EXT0 resides in the same power domain (RTC_PERIPH) as the RTC IO pullup/downs.
    // No need to keep that power domain explicitly, unlike EXT1.
    ESP_ERROR_CHECK(rtc_gpio_pullup_dis(switch_gpio));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_dis(switch_gpio));

    // Configura o pino como GPIO
    esp_rom_gpio_pad_select_gpio(switch_gpio);
    // Configura o GPIO como saída INPUT
    gpio_set_direction(switch_gpio, GPIO_MODE_INPUT);

    xTaskCreatePinnedToCore(deep_sleep_task, "deep_sleep_task", DEEP_SLEEP_APP_TASK_STACK_SIZE, NULL, DEEP_SLEEP_APP_TASK_PRIORITY, NULL, DEEP_SLEEP_APP_TASK_CORE_ID);
}