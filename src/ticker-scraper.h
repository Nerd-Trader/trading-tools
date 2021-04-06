#pragma once

/* compat */
#ifndef REG_NOERROR
#define REG_NOERROR 0
#endif

#define true  1
#define false 0

typedef char bool;

typedef enum MarketPlaces
{
    AMEX   = (1 << 0),
    NYSE   = (1 << 1),
    NASDAQ = (1 << 2)
} MarketPlace;

typedef char Symbol[7];

int strpos(const char *source, const char *search);

char *mplace2str(MarketPlace marketplace);

int scrape_ticker_symbols(MarketPlace marketplace);
int ticker_scraper_add(MarketPlace marketplace, Symbol symbol, char *company_name);
