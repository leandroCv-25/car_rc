#ifndef APP_NVS_H_
#define APP_NVS_H_

#include "stdint.h"

void app_nvs_init();

esp_err_t app_nvs_load_mac(uint8_t index, uint8_t *mac);

esp_err_t app_nvs_save_mac(uint8_t index, uint8_t *mac);

esp_err_t app_nvs_load_number_devices(uint8_t *nDevices);

esp_err_t app_nvs_save_number_devices(uint8_t nDevices);

#endif /* NVS_APP_H_ */