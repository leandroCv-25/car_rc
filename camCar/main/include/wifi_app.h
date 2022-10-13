#ifndef WIFI_APP_H_
#define WIFI_APP_H_

#include "esp_netif.h"

// Callback typedef
typedef void (*wifi_connected_event_callback_t)(void);

// netif object for the Station
extern esp_netif_t *esp_netif_sta;

/**
 * Starts the WiFi RTOS task
 */
void wifi_app_start(char *str_ssid_wifi, char *str_pass_wifi);

#endif /* MAIN_TASKS_COMMON_H_ */