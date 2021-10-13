#pragma once

#define true  1
#define false 0

typedef char bool;

// Ticker type
typedef char Symbol[32];

typedef enum MarketPlaces {
    MPLACE_UNKNOWN = 1 << 0,
    // US
    MPLACE_NASDAQ  = 1 << 1,
    MPLACE_NYSE    = 1 << 2,
    // OTC
    MPLACE_OTCQB   = 1 << 3,
    MPLACE_OTCQX   = 1 << 4,
    MPLACE_PINK    = 1 << 5,
} MarketPlace;

#define MPLACE_UNKNOWN_STR "Unknown"
#define MPLACE_NASDAQ_STR  "NASDAQ"
#define MPLACE_NYSE_STR    "NYSE"
#define MPLACE_OTCQB_STR   "OTCQB"
#define MPLACE_OTCQX_STR   "OTCQX"
#define MPLACE_PINK_STR    "Pink"

#define bzero(b, len) (memset(b, '\0', len), (void)0)

char *marketplace_to_str(const MarketPlace marketplace);
long parse_marketcap_str(const char *mcap_str);
MarketPlace str_to_marketplace(const char *mplace_str);
