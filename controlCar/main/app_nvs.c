#include <stdio.h>
#include <string.h>

#include "esp_mac.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "app_nvs.h"

// Tag for logging to the monitor
static const char TAG[] = "nvs";

// NVS name space used for station mode credentials
const char app_nvs_edutec_namespace[] = "car";

void app_nvs_init()
{
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }
}

esp_err_t app_nvs_load_mac(uint8_t index, uint8_t *mac)
{
    nvs_handle handle;
    esp_err_t esp_err;

    ESP_LOGI(TAG, "Loading mac from flash");

    esp_err = nvs_open(app_nvs_edutec_namespace, NVS_READWRITE, &handle);

    if (esp_err == ESP_OK)
    {

        // Allocate buffer
        size_t size_str = 6;
        uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * size_str);
        memset(buffer, 0x00, sizeof(size_str));

        // Load device name
        char deviceMacLocation[12];
        sprintf(deviceMacLocation, "Device %u", index);

        // Load device name
        esp_err = nvs_get_blob(handle, deviceMacLocation, buffer, &size_str);
        if (esp_err != ESP_OK)
        {
            free(buffer);
            ESP_LOGW(TAG, "(%s) no peer device found in NVS\n", esp_err_to_name(esp_err));
            return esp_err;
        }
        memcpy(mac, buffer, size_str);

        free(buffer);
        nvs_close(handle);

        ESP_LOGI(TAG, "Device from flash %u: %X:%X:%X:%X:%X:%X", index, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        return esp_err;
    }
    else
    {
        return esp_err;
    }
}

esp_err_t app_nvs_save_mac(uint8_t index, uint8_t *mac)
{
    nvs_handle handle;
    esp_err_t esp_err;

    ESP_LOGI(TAG, "Saving mac from flash");

    esp_err = nvs_open(app_nvs_edutec_namespace, NVS_READWRITE, &handle);

    if (esp_err == ESP_OK)
    {

        // Allocate buffer
        size_t size_str = 6;

        // Load device name
        char deviceMacLocation[12];
        sprintf(deviceMacLocation, "Device %u", index);

        esp_err = nvs_set_blob(handle, deviceMacLocation, mac, size_str);
        if (esp_err != ESP_OK)
        {
            ESP_LOGW(TAG, "(%s) no peer device saved in NVS\n", esp_err_to_name(esp_err));
            return esp_err;
        }

        ESP_LOGI(TAG, "Saved in flash %u: %X:%X:%X:%X:%X:%X", index, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        if (esp_err == ESP_OK)
        {
            esp_err = nvs_commit(handle);
            nvs_close(handle);
            return esp_err;
        }
        else
        {
            return esp_err;
        }
    }
    else
    {
        return esp_err;
    }
}

esp_err_t app_nvs_load_number_devices(uint8_t *nDevices)
{

    nvs_handle handle;
    esp_err_t esp_err;

    ESP_LOGI(TAG, "Loading Number Devices from flash");

    esp_err = nvs_open(app_nvs_edutec_namespace, NVS_READWRITE, &handle);

    if (esp_err == ESP_OK)
    {
        ESP_LOGI(TAG, "Number Devices %u", *nDevices);
        esp_err = nvs_get_u8(handle, "numberDevices", nDevices);

        nvs_close(handle);
        return esp_err;
    }
    else
    {
        ESP_LOGI(TAG, "err: (%s) Loading Number Devices from flash", esp_err_to_name(esp_err));

        return esp_err;
    }
}

esp_err_t app_nvs_save_number_devices(uint8_t nDevices)
{
    nvs_handle handle;
    esp_err_t esp_err;

    ESP_LOGI(TAG, "Saving Number Devices from flash");

    esp_err = nvs_open(app_nvs_edutec_namespace, NVS_READWRITE, &handle);

    if (esp_err == ESP_OK)
    {
        ESP_LOGI(TAG, "Number Devices %u", nDevices);
        esp_err = nvs_set_u8(handle, "numberDevices", nDevices);
        if (esp_err == ESP_OK)
        {
            ESP_LOGI(TAG, "Number Devices %u", nDevices);
            esp_err = nvs_commit(handle);
            nvs_close(handle);
            return esp_err;
        }
        else
        {
            return esp_err;
        }
    }
    else
    {
        return esp_err;
    }
}