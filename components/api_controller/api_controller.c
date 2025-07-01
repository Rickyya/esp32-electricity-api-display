#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "cJSON.h"

#include "http_rest_json_client.h"
#include "time_utils.h"
#include "api_controller.h"

extern SemaphoreHandle_t xDataSemaphore;
extern DataArray data;

static const char *TAG_API = "API";

void xFetchPricesTask() {
    char *url = CONFIG_ELEC_API_URL;
    while (1) {
        if (xSemaphoreTake(xDataSemaphore, portMAX_DELAY)) {
            http_rest_recv_json_t response = {0};

            esp_err_t err = http_rest_client_get_json(url, &response);
            if (err == ESP_OK && response.status_code == 200) {
                cJSON *prices = cJSON_GetObjectItem(response.json, "prices");
                if (cJSON_IsArray(prices)) {
                    int count = cJSON_GetArraySize(prices);
                    data.size = count > MAX_PRICE_ENTRIES ? MAX_PRICE_ENTRIES : count;
                    for (int i = 0; i < data.size; i++) {
                        cJSON *entry = cJSON_GetArrayItem(prices, i);
                        if (!entry) break;
                        cJSON *price_item = cJSON_GetObjectItem(entry, "price");
                        cJSON *date_item = cJSON_GetObjectItem(entry, "startDate");

                        if (cJSON_IsNumber(price_item) && cJSON_IsString(date_item)) {
                            data.prices[i].price = price_item->valuedouble;
                            strncpy(data.prices[i].startDate, convertUtcToLocalIso(date_item->valuestring), sizeof(data.prices[i].startDate) - 1);
                            data.prices[i].startDate[sizeof(data.prices[i].startDate) - 1] = '\0';
                        }
                    }
                    ESP_LOGI(TAG_API, "Stored %d parsed prices", data.size);
                } else {
                    ESP_LOGE(TAG_API, "API request succesful but Json object is not an array");
                    data.size = 0;
                }
            } else {
                ESP_LOGE(TAG_API, "HTTP GET Request Failed with status: %d", response.status_code);
                data.size = 0;
            }
            xSemaphoreGive(xDataSemaphore);
        }
        TickType_t prev = xTaskGetTickCount();
        TickType_t delayTicks = pdMS_TO_TICKS(1000ULL*60*60* CONFIG_ELEC_FETCH_PRICES_PERIODICALLY_HOURS);
        ESP_LOGI(TAG_API, "Sleeping for %lu hours", (delayTicks / configTICK_RATE_HZ) / 3600);
        vTaskDelayUntil(&prev, delayTicks);
    }
    vTaskDelete(NULL);
}

PriceEntry* getCurrentTimePriceEntry(DataArray *data) {
    if (data->size == 0) {
        ESP_LOGE(TAG_API, "No prices available");
        return NULL;
    }

    for (int i = 0; i < data->size; i++) {
        if (isMatchingDateAndHour(data->prices[i].startDate)) {
            ESP_LOGD(TAG_API, "Matching price entry found: index=%d, time=%s", i, data->prices[i].startDate);
            return &data->prices[i];
        }
    }

    ESP_LOGE(TAG_API, "No matching price entry found for the current hour.");
    return NULL;
}

int getIndexOfEntry(DataArray* data, PriceEntry* entry) {
    for (int i = 0; i < data->size; i++) {
        if (&(data->prices[i]) == entry) {
            return i;
        }
    }
    return -1;
}

