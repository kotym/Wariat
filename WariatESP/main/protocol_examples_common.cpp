#include "protocol_examples_common.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include "esp_netif.h"

static const char *TAG = "example_connect";

static esp_err_t example_wifi_init(void)
{
    esp_err_t ret = ESP_OK;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t example_connect(void)
{
    esp_err_t ret = ESP_OK;
    
    // Initialize WiFi
    ret = example_wifi_init();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Connect as station
    ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_connect failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "WiFi connection initiated");
    return ESP_OK;
}