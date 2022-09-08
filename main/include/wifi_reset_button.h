#ifndef MAIN_WIFI_RESET_BUTTON_H_
#define MAIN_WIFI_RESET_BUTTON_H_

// Default interrupt flag
#define ESP_INTR_FLAG_DEFAULT	0



/**
 * Configures Wifi reset button and interrupt configuration
 */
void wifi_reset_button_config(gpio_num_t WIFI_RESET_BUTTON);

#endif /* MAIN_WIFI_RESET_BUTTON_H_ */
