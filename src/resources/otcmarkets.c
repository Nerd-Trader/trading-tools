#include <string.h>

#include <curl/curl.h>

#include "../config.h"
#include "../curl.h"
#include "../ticker-scraper.h"
#include "otcmarkets.h"

const char markets_otcqx[4] = {
    6, // OTCQX International
    5, // OTCQX International Premier
    2, // UTCQX U.S.
    1  // UTCQX U.S. Premier
};

const char market_otcqb[1] = {
    10 // OTCQB
};

const char markets_pink[3] = {
    20, // Pink Current
    21, // Pink Limited
    22  // Pink No Information
};

int ticker_scraper_scrape_otcmarkets(MarketPlace marketplace)
{
}
