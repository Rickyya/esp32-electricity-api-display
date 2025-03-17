#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "iot_button.h"
#include "button_gpio.h"

#include "api_controller.h"
#include "display_controller.h"
#include "button_controller.h"

static const char *TAG = "BUTTON_CONTROLLER";

extern SemaphoreHandle_t xDataSemaphore;
extern int displayDataOffset;
extern DataArray data;
extern TimerHandle_t displayButtonResetTimer;

static void buttonNext(void *arg, void *usr_data) {
    ESP_LOGI(TAG, "buttonNext");
    if (xSemaphoreTake(xDataSemaphore, pdMS_TO_TICKS(100))) {
        PriceEntry* entry = getCurrentTimePriceEntry(&data);
        if (entry) {
            int leading_index = getIndexOfEntry(&data, entry);

            if (leading_index - displayDataOffset > 2) {
                displayDataOffset += 3;
                PriceEntry *new_entry = &data.prices[leading_index - displayDataOffset];
                displayThreePrices(new_entry);
                xTimerReset(displayButtonResetTimer, 0);
            }
        }
        xSemaphoreGive(xDataSemaphore);
    }
}

static void buttonPrevious(void *arg, void *usr_data) {
    ESP_LOGI(TAG, "buttonPrevious");
    if (xSemaphoreTake(xDataSemaphore, pdMS_TO_TICKS(100))) {
        if (displayDataOffset > 0) {
            displayDataOffset = displayDataOffset < 3 ? 0 : displayDataOffset - 3;
            PriceEntry* entry = getCurrentTimePriceEntry(&data);
            if (entry) {
                int leading_index = getIndexOfEntry(&data, entry);
                PriceEntry *new_entry = &data.prices[leading_index - displayDataOffset];
                displayThreePrices(new_entry);
                xTimerReset(displayButtonResetTimer, 0);
            }
        }
        xSemaphoreGive(xDataSemaphore);
    }
}

esp_err_t initButtons() {
    button_config_t btn_cfg = {0};
    button_gpio_config_t gpio_cfg = {
        .gpio_num = CONFIG_ELEC_BTN_NEXT,
        .active_level = 0,
        .enable_power_save = true,
        .disable_pull = false,
    };
    button_handle_t btn;
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn);
    assert(ret == ESP_OK);

    ret = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NULL, buttonNext, NULL);

    gpio_cfg.gpio_num = CONFIG_ELEC_BTN_PREV;
    button_handle_t btn2;
    ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn2);
    assert(ret == ESP_OK);
    ret |= iot_button_register_cb(btn2, BUTTON_SINGLE_CLICK, NULL, buttonPrevious, NULL);
    return ret;
}
