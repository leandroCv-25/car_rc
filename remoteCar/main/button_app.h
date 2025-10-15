#ifndef BUTTON_APP_H_
#define BUTTON_APP_H_

#include "esp_check.h"

#include "driver/gpio.h"

#include "iot_button.h"

/**
 * @brief Registrando o callback
 * 
 * @param gpio_btn  //Qual botão?
 * @param button_event //Tipo de evento
 * @param time_pressed //Tempo que deve estar pressionado caso for para pressionar em longa duração
 * @param cb //Função de callback
 * @param usr_data dados de callback
 * @return esp_err_t 
 */
esp_err_t resgister_event_callback(button_handle_t gpio_btn, button_event_t button_event, uint16_t time_pressed, button_cb_t cb,void *usr_data);

/**
 * @brief Configura o botão
 *
 * @param gpio_btn
 * @param active_level
 * @return button_handle_t
 */
button_handle_t button_app_init(gpio_num_t gpio_btn, int active_level);

#endif