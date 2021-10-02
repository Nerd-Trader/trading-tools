#pragma once

#define true  1
#define false 0

typedef char bool;

typedef char Symbol[7];

typedef enum MarketPlaces {
    UNKNOWN = 1 << 0,
    NASDAQ  = 1 << 1,
    NYSE    = 1 << 2,
    OTCQB   = 1 << 3,
    OTCQX   = 1 << 4,
    PINK    = 1 << 5,
} MarketPlace;

typedef struct DataRow {
    Symbol      ticker;
    MarketPlace marketplace;
    char        company[512];
    float       price;
    char        sector[512];
    char        industry[512];
    char        country[512];
    char        marketcap[512];
} DataRow;

MarketPlace str_to_marketplace(const char *mplace_str);
