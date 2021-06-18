#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>


#include <csv.h>

#include "config.h"
#include "resources/finviz.h"
#include "ticker-scraper.h"

size_t csv_col_idx = 0;
struct csv_parser parser;
unsigned char csv_options = CSV_STRICT | CSV_APPEND_NULL;

void csv_cb1(void *s, size_t i, void *outfile) {
    csv_fwrite((FILE *)outfile, s, i);

    if (csv_col_idx < 6) { /* Do not put separator after last column */
        fputc(CSV_COMMA, (FILE *)outfile);
    }

    csv_col_idx++;
}

void csv_cb2(int c, void *outfile) {
    fputc('\n', (FILE *)outfile);

    csv_col_idx = 0;
}

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
    char *marketplace = marketplace_to_str(dataRow->marketplace);
    csv_parse(&parser, marketplace, strlen(marketplace), csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, ",", 1, csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, dataRow->ticker, strlen(dataRow->ticker), csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, ",", 1, csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, dataRow->company, strlen(dataRow->company), csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, ",", 1, csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, dataRow->sector, strlen(dataRow->sector), csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, ",", 1, csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, dataRow->industry, strlen(dataRow->industry), csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, ",", 1, csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, dataRow->country, strlen(dataRow->country), csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, ",", 1, csv_cb1, csv_cb2, stdout);
    csv_parse(&parser, dataRow->marketcap, strlen(dataRow->marketcap), csv_cb1, csv_cb2, stdout);
    csv_fini(&parser, csv_cb1, csv_cb2, stdout);

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

    int ret = csv_init(&parser, csv_options);
    if (ret) {
        fprintf(stderr, "Error: Couldn’t initialize CSV parser.");
        return EXIT_FAILURE;
    }
    csv_set_delim(&parser, CSV_COMMA);

    const char *csv_header = "marketplace,ticker,company,sector,industry,country,marketcap";
    csv_parse(&parser, csv_header, strlen(csv_header), csv_cb1, csv_cb2, stdout);
    csv_fini(&parser, csv_cb1, csv_cb2, stdout);

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

    csv_free(&parser);

    return EXIT_SUCCESS;
}
