#ifndef REMOTE_APP_H_
#define REMOTE_APP_H_

#include "driver/gpio.h"
#include "driver/adc.h"

/**
 * @brief Configuration of adc
 * 
 */
typedef struct {
    adc_channel_t adcChannel;
    adc_bits_width_t adcWidth;
    adc_atten_t adcAtten;
    adc_unit_t adcUnit;
}adc_config_t;



/**
 * @brief Config and start the car task
 *
 * @param adc_config_t pointer to the config of the adc config
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Configure failed
 */
esp_err_t remote_init(adc_config_t *adc_config, gpio_num_t buttonGpio, gpio_num_t sclGpio, gpio_num_t sdaGpio,uint16_t  addr);

#endif