#ifndef COMPONENTS_DISPLAY_CONTROLLER_INCLUDE_DISPLAY_CONTROLLER_H_
#define COMPONENTS_DISPLAY_CONTROLLER_INCLUDE_DISPLAY_CONTROLLER_H_

#include "ssd1306.h"

esp_err_t initDisplay(SSD1306_t *display);

/**
 * @brief Callback function to reset display to current hours price.
 * 
 * @param xTimer 
 */
void resetDisplayCallback(TimerHandle_t xTimer);
/**
 * @brief Display prices and date_strs, formatted. Max 3 counts.
 * 
 * @param startingFrom PriceEntry + next 2 PriceEntries
 */
void displayThreePrices(PriceEntry *startingFrom);

#endif  // COMPONENTS_DISPLAY_CONTROLLER_INCLUDE_DISPLAY_CONTROLLER_H_
