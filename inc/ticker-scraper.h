#pragma once

#define true  1
#define false 0

typedef char bool;

typedef char Symbol[7];

typedef enum MarketPlaces {
    UNKNOWN = 1 << 0,
    AMEX    = 1 << 1,
    NYSE    = 1 << 2,
    NASDAQ  = 1 << 3,
    OTCQX   = 1 << 4,
    OTCQB   = 1 << 5,
    PINK    = 1 << 6,
} MarketPlace;

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
