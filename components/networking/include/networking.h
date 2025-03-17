#ifndef COMPONENTS_NETWORKING_INCLUDE_NETWORKING_H_
#define COMPONENTS_NETWORKING_INCLUDE_NETWORKING_H_

#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

typedef struct {
    EventGroupHandle_t wifi_event_group;
    int retry_num;
    bool is_connected;
    char ssid[32];
    char password[64];
} WifiManager_t;

esp_err_t wifiManagerInit(WifiManager_t *manager);
esp_err_t wifiManagerConnect(WifiManager_t *manager);
bool wifi_manager_is_connected(WifiManager_t *manager);
void wifi_manager_deinit(WifiManager_t *manager);
#ifdef __cplusplus
}
#endif
#endif  // COMPONENTS_NETWORKING_INCLUDE_NETWORKING_H_
