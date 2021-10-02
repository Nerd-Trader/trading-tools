#pragma once

#include "curl.h"
#include "historical-data-scraper.h"

char *historical_data_scraper_get_tdameritrade_price_history(MemoryStruct *chunk, const Symbol *ticker);
char *historical_data_scraper_scrape_tdameritrade(const DataRow *dataRow);
