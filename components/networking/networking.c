#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "networking.h"

#define WIFI_MAXIMUM_RETRY 5

static const char *TAG = "WIFI";

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    WifiManager_t *manager = (WifiManager_t*)arg;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (manager->retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            manager->retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(manager->wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
        manager->is_connected = false;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        manager->retry_num = 0;
        xEventGroupSetBits(manager->wifi_event_group, WIFI_CONNECTED_BIT);
        manager->is_connected = true;
    }
}

esp_err_t wifiManagerInit(WifiManager_t *manager)
{
    manager->wifi_event_group = xEventGroupCreate();
    manager->retry_num = 0;
    manager->is_connected = false;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, manager, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, manager, NULL));

    return ESP_OK;
}

esp_err_t wifiManagerConnect(WifiManager_t *manager)
{
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    strncpy((char*)wifi_config.sta.ssid, manager->ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, manager->password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifiManagerConnect finished.");

    EventBits_t bits = xEventGroupWaitBits(manager->wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", manager->ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", manager->ssid);
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_ERR_INVALID_STATE;
    }
}

bool wifi_manager_is_connected(WifiManager_t *manager)
{
    return manager->is_connected;
}

void wifi_manager_deinit(WifiManager_t *manager)
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    vEventGroupDelete(manager->wifi_event_group);
    esp_wifi_stop();
    esp_wifi_deinit();
}