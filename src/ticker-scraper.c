#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "resources/finviz.h"
#include "ticker-scraper.h"

char *marketplace_to_str(MarketPlace marketplace)
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

int ticker_scraper_add(DataRow *dataRow)
{
    printf("\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
        marketplace_to_str(dataRow->marketplace),
        dataRow->ticker,
        dataRow->company,
        dataRow->sector,
        dataRow->industry,
        dataRow->country,
        dataRow->marketcap
    );

    return 1;
}

int scrape_ticker_symbols(MarketPlace marketplace)
{
    switch (marketplace)
    {
        case AMEX:
        case NYSE:
        case NASDAQ:
#if DEBUG
            fprintf(stderr, "Scraping %s FinViz ticker symbols…\n", marketplace_to_str(marketplace));
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

    puts("marketplace,ticker,company,sector,industry,country,marketcap");

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
