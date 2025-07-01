#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "font8x8_basic.h"

#include "ssd1306.h"
#include "time_utils.h"
#include "api_controller.h"
#include "display_controller.h"

extern SSD1306_t display;
extern int displayDataOffset;
extern DataArray data;
extern SemaphoreHandle_t xDataSemaphore;

static const char *TAG_DISPLAY = "DISPLAY";

esp_err_t initDisplay(SSD1306_t *display) {
    #if CONFIG_I2C_INTERFACE
    i2c_master_init(display, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    #elif CONFIG_SPI_INTERFACE
    spi_master_init(display, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
    #endif

    #if CONFIG_SSD1306_128x64
    ssd1306_init(display, 128, 64);
    #elif CONFIG_SSD1306_128x32
    ssd1306_init(display, 128, 32);
    #endif
    if (!display) {
        return ESP_FAIL;
    }
    ssd1306_clear_screen(display, false);
    ssd1306_contrast(display, 0xff);

    return ESP_OK;
}

void displayThreePrices(PriceEntry *startingFrom) {
    int index = getIndexOfEntry(&data, startingFrom);
    assert(index != -1);

    int count = (index >= 2) ? 3 : (index + 1);
    float prices[3] = {0};
    const char *dates[3] = {0};
    for (int i = 0; i < count; i++) {
        prices[i] = data.prices[index - i].price;
        dates[i] = data.prices[index - i].startDate;
    }
    ssd1306_clear_screen(&display, true);
    if (count == 0) {
        ssd1306_display_text(&display, 3, "No prices", 16, true);
        return;
    }
    const char *label = getDayLabelFromIso(dates[0]);
    char header[17];
    snprintf(header, sizeof(header), "  Prices %-7.7s", label);
    ssd1306_display_text(&display, 0, header, 16, true);
    ssd1306_display_text(&display, 2, "   h      c/kWh ", 16, true);
    int max_display = count > 3 ? 3 : count;
    for (int i = 0; i < max_display; i++) {
        char hour[6] = {0};
        if (strlen(dates[i]) >= 16) {
            strncpy(hour, dates[i] + 11, 5);
        } else {
            strncpy(hour, "??:??", 5);
        }
        char line[17];
        snprintf(line, sizeof(line), " %s -> %.2f ", hour, prices[i]);
        ssd1306_display_text(&display, 3 + i, line, 16, true);
    }
    ssd1306_display_text(&display, 7, dates[0], 16, true);
}

void resetDisplayCallback(TimerHandle_t xTimer) {
    if (xSemaphoreTake(xDataSemaphore, pdMS_TO_TICKS(100))) {
        PriceEntry* entry = getCurrentTimePriceEntry(&data);
        if (entry) {
            displayDataOffset = 0;
            displayThreePrices(entry);
        } else {
            ESP_LOGE(TAG_DISPLAY, "No matching price entry for this hour found");
        }
        xSemaphoreGive(xDataSemaphore);
    }
}
