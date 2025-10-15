
#include "esp_check.h"

#include "driver/gpio.h"

#include "iot_button.h"
#include "button_gpio.h"

#include "button_app.h"

esp_err_t resgister_event_callback(button_handle_t gpio_btn, button_event_t button_event, uint16_t time_pressed, button_cb_t cb, void *usr_data)
{
    button_event_args_t cfg = {};
    cfg.long_press.press_time = time_pressed; // quanto tempo para considerar longo periodo e chamar o evento de smart config

    // Registrando evento com função criada
    return iot_button_register_cb(gpio_btn, button_event, &cfg, cb, usr_data);
}

button_handle_t button_app_init(gpio_num_t gpio_btn, int active_level)
{

    // criando o botão na GPIO gpio_btn e com tipo de digitais
    button_config_t gpio_btn_cfg = {0};

    button_gpio_config_t gpio_cfg = {
        .gpio_num = gpio_btn,
        .active_level = active_level,
        .enable_power_save = true,
    };

    // Variável de gerenciamento do botão sendo configurada
    button_handle_t gpio_btn_handle;
    iot_button_new_gpio_device(&gpio_btn_cfg, &gpio_cfg, &gpio_btn_handle);

    return gpio_btn_handle;
}