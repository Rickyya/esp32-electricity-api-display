#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_netif_sntp.h"

#include "time_date.h"

esp_err_t initTime() {
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_err_t res = esp_netif_sntp_init(&config);
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
        printf("Failed to update system time within 10s timeout");
        return ESP_ERR_TIMEOUT;
    } else {
        return res;
    }
}

char *getCurrentTime() {
    static char time_str[64];
    time_t now;
    struct tm timeinfo;

    time(&now);
    setenv("TZ", "EET-2EEST,M3.5.0/03,M10.5.0/04", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return time_str;
}
/** from
 * https://stackoverflow.com/questions/283166/easy-way-to-convert-a-struct-tm-expressed-in-utc-to-time-t-type 
 * */
time_t myTimegm(struct tm *tm) {
    time_t ret;
    char *tz;

    tz = getenv("TZ");
    if (tz)
        tz = strdup(tz);
    setenv("TZ", "", 1);
    tzset();
    ret = mktime(tm);
    if (tz) {
        setenv("TZ", tz, 1);
        free(tz);
    } else {
        unsetenv("TZ");
    }
    tzset();
    return ret;
}

char *convertUtcToLocalIso(const char *utc_in) {
    static char local_time_str[64];
    struct tm tm_utc = {0};
    struct tm tm_local;
    time_t t;

    strptime(utc_in, "%Y-%m-%dT%H:%M:%SZ", &tm_utc);
    t = myTimegm(&tm_utc);

    localtime_r(&t, &tm_local);
    strftime(local_time_str, sizeof(local_time_str), "%Y-%m-%dT%H:%M:%S", &tm_local);

    return local_time_str;
}

bool isMatchingDateAndHour(const char *timestamp) {
    char *current_time_str = getCurrentTime();
    current_time_str[10] = 'T';  // to match api format
    // Compare the first 13 characters: "YYYY-MM-DDTHH"
    return strncmp(timestamp, current_time_str, 13) == 0;
}

const char *getDayLabelFromIso(const char *timestamp) {
    struct tm input_tm = {0};
    if (strptime(timestamp, "%Y-%m-%dT%H:%M:%S", &input_tm) == NULL)
        return "";

    // Normalize input date to midnight
    input_tm.tm_hour = input_tm.tm_min = input_tm.tm_sec = 0;
    time_t input_midnight = mktime(&input_tm);

    // Get current date at midnight
    time_t now = time(NULL);
    struct tm now_tm;
    localtime_r(&now, &now_tm);
    now_tm.tm_hour = now_tm.tm_min = now_tm.tm_sec = 0;
    time_t now_midnight = mktime(&now_tm);

    int day_diff = (int)difftime(input_midnight, now_midnight) / (60 * 60 * 24);

    if (day_diff == 0.0)
        return "today";
    else if (day_diff == 1.0)
        return "tmrw";
    else
        return "";
}

