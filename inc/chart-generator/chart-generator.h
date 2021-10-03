#pragma once

#include "common.h"

typedef struct Candle {
    double open;
    double high;
    double low;
    double close;
    int    volume;
    int    datetime;
} Candle;
