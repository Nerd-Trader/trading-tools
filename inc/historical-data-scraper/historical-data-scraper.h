#pragma once

#include "common.h"

typedef struct DataRow {
    Symbol      ticker;
    MarketPlace marketplace;
    char        company[512];
    float       price;
    char        sector[512];
    char        industry[512];
    char        country[512];
    long        marketcap;
} DataRow;

