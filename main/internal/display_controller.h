#ifndef MAIN_INTERNAL_DISPLAY_CONTROLLER_H_
#define MAIN_INTERNAL_DISPLAY_CONTROLLER_H_
#include <stdio.h>
#include <string.h>

#include "api_controller.h"

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

#endif  // MAIN_INTERNAL_DISPLAY_CONTROLLER_H_
