#pragma once

#define true  1
#define false 0

typedef char bool;

// Ticker type
typedef char Symbol[32];

typedef enum MarketPlaces {
    UNKNOWN = 1 << 0,
    // US
    NASDAQ  = 1 << 1,
    NYSE    = 1 << 2,
    // OTC
    OTCQB   = 1 << 3,
    OTCQX   = 1 << 4,
    PINK    = 1 << 5,
} MarketPlace;
