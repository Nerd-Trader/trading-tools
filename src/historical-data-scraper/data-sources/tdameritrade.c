// Reference: https://developer.tdameritrade.com/price-history/apis/get/marketdata/%7Bsymbol%7D/pricehistory

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "curl.h"
#include "data-sources/tdameritrade.h"
#include "historical-data-scraper.h"

#define TD_AMERITRADE_API_URL "https://api.tdameritrade.com/v1/marketdata/"

CURL *curl_handle;

char *historical_data_scraper_get_tdameritrade_price_history(MemoryStruct *chunk, const Symbol *ticker)
{
    CURLcode res;

    chunk->memory = malloc(1); /* Will be grown as needed by realloc() in nerd_trader_curl_write_memory_callback */
    chunk->size = 0; /* No data at this point */

    char url[256] = TD_AMERITRADE_API_URL;

    strcat(url, *ticker);
    strcat(url, "/");

    strcat(url, "pricehistory");

    strcat(url, "?apikey=");
    strcat(url, TD_AMERITRADE_API_KEY);

    strcat(url, "&periodType=");
    strcat(url, "year");

    strcat(url, "&period=");
    strcat(url, "5");

    strcat(url, "&frequencyType=");
    strcat(url, "daily");

    strcat(url, "&frequency=");
    strcat(url, "1");

    strcat(url, "&needExtendedHoursData=");
    strcat(url, "true");

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

#if DEBUG
    // fprintf(stderr, "Attempting to obtain JSON data for  \"%s\" of marketplace %s: %s\n", industry[0], marketplace_to_str(marketplace), url);
#endif

    /* Get it! */
    res = curl_easy_perform(curl_handle);

    /* The API rate limit is 120 requests per minute, hence let's wait 500ms */
    {
        static int milliseconds = 500;
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
        nanosleep(&ts, NULL);
    }

    /* Check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return NULL;
    }

    return (char *)chunk->memory;
}

char *historical_data_scraper_scrape_tdameritrade(const DataRow *data_row)
{
    struct MemoryStruct chunk;

    /* Init curl */
    curl_handle = nerd_trader_curl_init(&chunk);

    char *result = historical_data_scraper_get_tdameritrade_price_history(&chunk, &data_row->ticker);

    nerd_trader_curl_cleanup(curl_handle);

    return result;
}
