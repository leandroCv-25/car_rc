#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_timer.h"

#include "driver/gpio.h"

#include "espnow.h"

#include "device_params.h"
#include "app_communication.h"

// Tag used for ESP serial console messages
static const char TAG[] = "app_communication";

uint8_t newMac;

const uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

app_communication_callback_t _responseCb;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t app_communication_queue_handle;

bool isMyAddress(uint8_t *mac)
{
    uint8_t baseMac[6];
    bool isMyAdr = true;
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
    if (ret == ESP_OK)
    {
        for (int i = 0; i < 6; i++)
        {
            isMyAdr = isMyAdr && (baseMac[i] == mac[i]);
        }
    }
    else
    {
        return false;
    }

    return isMyAdr;
}

static void wifi_init(void)
{
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void comunication_app_event_data_recv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
{
    uint8_t *srcAdr = info->src_addr;
    uint8_t *desAdr = info->des_addr;

    if (memcmp( desAdr, broadcastAddress,6) == 0)
    {
        ESP_LOGI(TAG, "Received a Broadcast msg from %02X%02X%02X%02X%02X%02X: %s", srcAdr[0], srcAdr[1], srcAdr[2], srcAdr[3], srcAdr[4], srcAdr[5],(char*)incomingData);
        _responseCb(true, srcAdr, incomingData, sizeof(incomingData));
    }
    else if(isMyAddress(desAdr))
    {
        ESP_LOGI(TAG, "Received msg from %02X%02X%02X%02X%02X%02X: %s", srcAdr[0], srcAdr[1], srcAdr[2], srcAdr[3], srcAdr[4], srcAdr[5],(char*) incomingData);
        _responseCb(false, srcAdr, incomingData, sizeof(incomingData));
    }
}

// Callback when data is sent
void comunication_app_event_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS)
    {

        ESP_LOGI(TAG, "Last Packet Send Status:\tDelivery Success");
    }
    else
    {
        ESP_LOGI(TAG, "Last Packet Send Status:\tDelivery Fail");
    }
}

BaseType_t appCommunicationEvent(uint8_t *adrDst)
{
    communication_app_msg_t msg;
    memcpy(msg.destAddress, adrDst, 6);
    return xQueueSend(app_communication_queue_handle, &msg, portMAX_DELAY);
}

BaseType_t appCommunicationEventWithData(uint8_t *data, uint8_t *adrDst)
{
    communication_app_msg_t msg;
    memcpy(msg.destAddress, adrDst, 6);
    memcpy(msg.data, data, 250);

    return xQueueSend(app_communication_queue_handle, &msg, portMAX_DELAY);
}

void AddPeerAppCommunication(uint8_t *mac)
{
    ESP_LOGI(TAG, "MAC:%X:%X:%X:%X:%X:%X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    esp_now_peer_info_t newPeerInfo;
    newPeerInfo.ifidx = WIFI_IF_STA,
    memcpy(newPeerInfo.peer_addr, mac, 6);
    newPeerInfo.channel = 0;
    if (!esp_now_is_peer_exist(mac))
    {
        esp_now_add_peer(&newPeerInfo);
    }
}

void DelPeerAppCommunication(uint8_t *mac)
{
    if (esp_now_is_peer_exist(mac))
    {
        esp_now_del_peer(mac);
    }
}

static void app_event_communication_teacher_task(void *pvParameters)
{
    communication_app_msg_t msg;

    while (true)
    {
        if (xQueueReceive(app_communication_queue_handle, &msg, portMAX_DELAY))
        {
            if (memcmp((msg.destAddress),broadcastAddress,6) == 0)
            {
                ESP_LOGI(TAG, "Broadcast data with payload: %s", (char*) (msg.data));
            }
            else
            {
                ESP_LOGI(TAG, "SENDING DATA to MAC:%X:%X:%X:%X:%X:%X WITH PAYLOAD: %s", msg.destAddress[0], msg.destAddress[1], msg.destAddress[2], msg.destAddress[3], msg.destAddress[4], msg.destAddress[5],(char*)(msg.data));
            }

            uint8_t data[250];
            memcpy(data, msg.data, 250);
            esp_now_send(msg.destAddress, (uint8_t *)&data, 250);
        }
    }
}

void configAppCommunication(app_communication_callback_t responseCb)
{
    ESP_LOGI(TAG, "STARTING");

    _responseCb = responseCb;

    espnow_storage_init();

    // Disable default WiFi logging messages
    esp_log_level_set("wifi", ESP_LOG_NONE);
    wifi_init();

    // Event handler for the connection
    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_init(&espnow_config);

    esp_now_register_recv_cb(comunication_app_event_data_recv);
    esp_now_register_send_cb(comunication_app_event_data_sent);

    // Create message queue
    app_communication_queue_handle = xQueueCreate(3, sizeof(communication_app_msg_t));

    // Start the Communication application task
    xTaskCreatePinnedToCore(&app_event_communication_teacher_task, "app_event_communication_teacher_task", APP_COMUNICATION_STACK, NULL, APP_COMUNICATION_PRIORITY, NULL, APP_COMUNICATION_CORE_ID);
}