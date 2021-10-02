#pragma once

#include "common.h"

typedef struct DataRow {
    Symbol      ticker;
    MarketPlace marketplace;
    char        company[512];
    char        price[32];
    char        sector[512];
    char        industry[512];
    char        country[512];
    char        marketcap[512];
} DataRow;

char *escape_for_csv(const char *input);
char *marketplace_to_str(const MarketPlace marketplace);

int scrape_ticker_symbols(const MarketPlace marketplace);
int ticker_scraper_add(const DataRow *dataRow);
