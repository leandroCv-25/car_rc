#ifndef WIFI_APP_H_
#define WIFI_APP_H_

#include "esp_netif.h"

// Callback typedef
typedef void (*wifi_connected_event_callback_t)(void);

// netif object for the Station
extern esp_netif_t *esp_netif_sta;

#define MAX_CONNECTION_RETRIES 5

/**
 * Message IDs for the WiFi application task
 * @note Expand this based on your application requirements.
 */
typedef enum wifi_app_message
{
    WIFI_APP_MSG_STA_CONNECTED_GOT_IP,
    WIFI_APP_MSG_SET_CREDENTIALS,
    WIFI_APP_MSG_STA_DISCONNECTED,
} wifi_app_message_e;

/**
 * Structure for the message queue
 * @note Expand this based on application requirements e.g. add another type and parameter as required
 */
typedef struct wifi_app_queue_message
{
    wifi_app_message_e msgID;
} wifi_app_queue_message_t;

/**
 * Sends a message to the queue
 * @param msgID message ID from the wifi_app_message_e enum.
 * @return pdTRUE if an item was successfully sent to the queue, otherwise pdFALSE.
 * @note Expand the parameter list based on your requirements e.g. how you've expanded the wifi_app_queue_message_t.
 */
BaseType_t wifi_app_send_message(wifi_app_message_e msgID);

/**
 * Starts the WiFi RTOS task
 */
void wifi_app_start(char *str_ssid_wifi, char *str_pass_wifi);

/**
 * Gets the wifi configuration
 */
wifi_config_t *wifi_app_get_wifi_config(void);

/**
 * Sets the callback function.
 */
void wifi_app_set_callback(wifi_connected_event_callback_t cb);

/**
 * Calls the callback function.
 */
void wifi_app_call_callback(void);

#endif /* MAIN_TASKS_COMMON_H_ */