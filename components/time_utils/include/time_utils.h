#ifndef COMPONENTS_TIME_UTILS_INCLUDE_TIME_UTILS_H_
#define COMPONENTS_TIME_UTILS_INCLUDE_TIME_UTILS_H_

esp_err_t initTime();
/**
 * @brief Get the Current Time in local time (finnish)
 * 
 * @return char* 
 */
char *getCurrentTime();
/**
 * @brief convert timestring (ending in Z aka UTC) to local time
 * 
 * @param utc_in 
 * @return char* 
 */
char *convertUtcToLocalIso(const char *utc_in);
/**
 * @brief Expects timestamp to be in format "YYYY-MM-DD HH:MM:SS"
 * 
 * @param timestamp 
 * @return true 
 * @return false 
 */
bool isMatchingDateAndHour(const char *timestamp);
/**
 * @brief returns "today", "tmrw" or ""
 * 
 * @param timestamp
 */
const char *getDayLabelFromIso(const char *timestamp);
#endif  // COMPONENTS_TIME_UTILS_INCLUDE_TIME_UTILS_H_
