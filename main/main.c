#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "freertos/queue.h"
#include "esp_sntp.h"

#include "http_rest_json_client.h"
#include "networking.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "internal/button_controller.h"
#include "internal/display_controller.h"
#include "internal/time_date.h"
#include "internal/api_controller.h"

static const char *TAG = "MAIN";
/** Wifi */
WifiManager_t wifiManager;
/** Display */
int displayDataOffset = 0;
SSD1306_t display;
TimerHandle_t displayButtonResetTimer;
/** DataArray struct with daily electricity prices and current display index */
DataArray data;
SemaphoreHandle_t xDataSemaphore;

void app_main(void) {
    ESP_LOGI(TAG, "Running on core %d", xPortGetCoreID());
    /** init nvs */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    /** connect to wifi */
    wifiManager = (WifiManager_t){
        .retry_num = CONFIG_ESP_MAXIMUM_RETRY,
        .ssid = CONFIG_ESP_WIFI_SSID,
        .password = CONFIG_ESP_WIFI_PASSWORD,
    };
    ESP_ERROR_CHECK(wifiManagerInit(&wifiManager));
    ESP_ERROR_CHECK(wifiManagerConnect(&wifiManager));

    /** init oled display */
    ESP_ERROR_CHECK(initDisplay(&display));
    /** displayButtonResetTimer <--> reset to 0 after a button press, after **time** seconds call resetDisplayCallback */
    displayButtonResetTimer = xTimerCreate("buttonInactivityDisplayReset", pdMS_TO_TICKS(CONFIG_ELEC_BUTTON_INACTIVITY_RESET_SEC * 1000), pdFALSE, NULL, resetDisplayCallback);
    /** displayPeriodicTimer <--> reset every **time** seconds (and auto reload) -> call resetDisplayCallback */
    TimerHandle_t displayPeriodicTimer = xTimerCreate("displayPeriodicTimer", pdMS_TO_TICKS(CONFIG_ELEC_RESET_DISPLAY_PERIODICALLY_SEC * 1000), pdTRUE, NULL, resetDisplayCallback);
    /** Init time */
    ESP_ERROR_CHECK(initTime());
    ESP_LOGI(TAG, "Current time and date: %s", getCurrentTime());
    /** Init data semaphore */
    xDataSemaphore = xSemaphoreCreateBinary();
    if (xDataSemaphore == NULL) {
        ESP_LOGE(TAG, "Failed to create semaphore!");
        return;
    }
    xSemaphoreGive(xDataSemaphore);
    /** buttons */
    ESP_ERROR_CHECK(initButtons());
    /** Start tasks */
    if (wifi_manager_is_connected(&wifiManager)) {
        xTimerStart(displayButtonResetTimer, 0);
        xTimerStart(displayPeriodicTimer, 0);
        xTaskCreatePinnedToCore(xFetchPricesTask, "FetchPricesTask", 8192, NULL, 5, NULL, 0);
    } else {
        ESP_LOGE(TAG, "Failed to connect to WiFi, did not start tasks");
    }
    vTaskDelete(NULL);
}
