#ifndef MAIN_INTERNAL_API_CONTROLLER_H_
#define MAIN_INTERNAL_API_CONTROLLER_H_

#include "ssd1306.h"
#include "cJSON.h"

#define MAX_PRICE_ENTRIES 48

typedef struct {
    float price;
    char startDate[20];
} PriceEntry;

typedef struct {
    PriceEntry prices[MAX_PRICE_ENTRIES];
    int size;
} DataArray;

/**
 * @brief Fetch api prices and populate the DataArray
 * @link https://api.porssisahko.net/v1/latest-prices.json
 */
void xFetchPricesTask();
PriceEntry* getCurrentTimePriceEntry(DataArray *data);
/**
 * @brief Get the Index Of Entry object in DataArray
 * 
 * @param data 
 * @param entry 
 * @return -1 if not found
 */
int getIndexOfEntry(DataArray* data, PriceEntry* entry);

#endif  // MAIN_INTERNAL_API_CONTROLLER_H_
