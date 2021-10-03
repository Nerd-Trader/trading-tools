#include <stdlib.h>
#include <string.h>

#include "common.h"

char *marketplace_to_str(const MarketPlace marketplace)
{
    switch (marketplace)
    {
        case MPLACE_NASDAQ:
            return MPLACE_NASDAQ_STR;
        break;

        case MPLACE_NYSE:
            return MPLACE_NYSE_STR;
        break;

        case MPLACE_OTCQB:
            return MPLACE_OTCQB_STR;
        break;

        case MPLACE_OTCQX:
            return MPLACE_OTCQX_STR;
        break;

        case MPLACE_PINK:
            return MPLACE_PINK_STR;
        break;

        default:
        case MPLACE_UNKNOWN:
            return MPLACE_UNKNOWN_STR;
    }
}

long parse_marketcap_str(const char *mcap_str)
{
    long mcap = 0l;
    long multiplier = 0l;

    if (strlen(mcap_str) > 0) {
        switch (mcap_str[strlen(mcap_str) - 1]) {
            case 'K':
                multiplier = 1000l;
                break;
            case 'M':
                multiplier = 1000l * 1000l;
                break;
            case 'B':
                multiplier = 1000l * 1000l * 1000l;
                break;
            case 'T':
                multiplier = 1000l * 1000l * 1000l * 1000l;
                break;
        }

        if (multiplier > 0l) {
            char float_str[strlen(mcap_str)];
            strcpy(float_str, mcap_str);
            float_str[strlen(mcap_str) - 1] = '\0';

            mcap = multiplier * atof(float_str);
        } else {
            mcap = atol(mcap_str);
        }
    }

    return mcap;
}

MarketPlace str_to_marketplace(const char *mplace_str)
{
    if (strcmp(mplace_str, MPLACE_NASDAQ_STR) == 0) {
        return MPLACE_NASDAQ;
    } else if (strcmp(mplace_str, MPLACE_NYSE_STR) == 0) {
        return MPLACE_NYSE;
    } else if (strcmp(mplace_str, MPLACE_OTCQB_STR) == 0) {
        return MPLACE_OTCQB;
    } else if (strcmp(mplace_str, MPLACE_OTCQX_STR) == 0) {
        return MPLACE_OTCQX;
    } else if (strcmp(mplace_str, MPLACE_PINK_STR) == 0) {
        return MPLACE_PINK;
    }

    return MPLACE_UNKNOWN;
}
