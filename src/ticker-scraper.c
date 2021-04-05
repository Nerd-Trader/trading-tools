#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "resources/finviz.h"
#include "ticker-scraper.h"

int strpos(const char *source, const char *search)
{
    char *found = strstr(source, search);

    if (found != NULL) {
        return found - source;
    }

    return -1;
}

char *mplace2str(MarketPlace marketplace)
{
    switch (marketplace)
    {
        case AMEX:
            return "AMEX";
        break;

        case NYSE:
            return "NYSE";
        break;

        case NASDAQ:
            return "NASDAQ";
        break;

        default:
            return "ERROR";
    }
}

int ticker_scraper_add(MarketPlace marketplace, Symbol symbol, char *company_name)
{
    printf("%s\n", symbol);

    return 1;
}

int scrape_ticker_symbols(MarketPlace marketplace)
{
    switch (marketplace)
    {
        case AMEX:
        case NYSE:
        case NASDAQ:
#if NERD_TRADER_DEBUG
            fprintf(stderr, "Scraping FinViz ticker symbols from marketplace %s…\n", mplace2str(marketplace));
#endif
            return ticker_scraper_scrape_finviz(marketplace);
        break;
    }

    return 0;
}

int main(int argc, char **argv)
{
    unsigned int new_symbols_retrieved = 0;
    bool no_params_provided = argc < 2;
    bool scan_amex = false,
         scan_nasdaq = false,
         scan_nyse = false;

    if (no_params_provided) {
        scan_amex = true;
        scan_nasdaq = true;
        scan_nyse = true;
    } else {
        for (int i = 1; i < argc; i++) {
            if (0 == strcmp(argv[i], "AMEX")) {
                scan_amex = true;
            } else if (0 == strcmp(argv[i], "NASDAQ")) {
                scan_nasdaq = true;
            } else if (0 == strcmp(argv[i], "NYSE")) {
                scan_nyse = true;
            }
        }

        if (!scan_amex && !scan_nasdaq && !scan_nyse) {
            fprintf(stderr, "No correct marketplace parameters provided\n");
            return EXIT_FAILURE;
        }
    }

    new_symbols_retrieved = 0;

    if (scan_nasdaq) {
        new_symbols_retrieved += scrape_ticker_symbols(NASDAQ);
    }

    if (scan_nyse) {
        new_symbols_retrieved += scrape_ticker_symbols(NYSE);
    }

    if (scan_amex) {
        new_symbols_retrieved += scrape_ticker_symbols(AMEX);
    }

    return (new_symbols_retrieved > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
