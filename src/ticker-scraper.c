#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include <csv.h>

#include "config.h"
#include "resources/finviz.h"
#include "ticker-scraper.h"

void explicit_bzero(void *s, size_t n);

size_t csv_field_index = 0;
struct csv_parser parser;
unsigned char csv_options = CSV_STRICT;

void csv_cb_end_of_field(void *s, size_t i, void *outfile) {
    csv_fwrite((FILE *)outfile, s, i);

    if (csv_field_index < 6) { /* Do not put separator after last field */
        fputc(CSV_COMMA, (FILE *)outfile);
    }

    csv_field_index++;
}

void csv_cb_end_of_row(int c, void *outfile) {
    fputc('\n', (FILE *)outfile);

    csv_field_index = 0;
}

char *marketplace_to_str(MarketPlace marketplace)
{
    switch (marketplace)
    {
        /*
           This is now called NYSE American, it's part of NYSE.
           Was called NYSE MKT prior, AMEX earlier.
        */
        case AMEX:
            return "NYSE";
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

char *escape_for_csv(const char *input)
{
    size_t input_len = strlen(input);
    size_t temp_len = input_len*2 + 2;
    char *temp = malloc(temp_len);

    explicit_bzero(temp, temp_len);

    csv_write(temp, temp_len, input, input_len);

    return temp;
}

int ticker_scraper_add(DataRow *dataRow)
{
    char *marketplace = marketplace_to_str(dataRow->marketplace);
    csv_parse(&parser, marketplace, strlen(marketplace), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, ",", 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *ticker = escape_for_csv(dataRow->ticker);
    csv_parse(&parser, ticker, strlen(ticker), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, ",", 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *company = escape_for_csv(dataRow->company);
    csv_parse(&parser, company, strlen(company), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, ",", 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *sector = escape_for_csv(dataRow->sector);
    csv_parse(&parser, sector, strlen(sector), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, ",", 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *industry = escape_for_csv(dataRow->industry);
    csv_parse(&parser, industry, strlen(industry), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, ",", 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *country = escape_for_csv(dataRow->country);
    csv_parse(&parser, country, strlen(country), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, ",", 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *marketcap = escape_for_csv(dataRow->marketcap);
    csv_parse(&parser, marketcap, strlen(marketcap), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_fini(&parser, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    free(ticker);
    free(company);
    free(sector);
    free(industry);
    free(country);
    free(marketcap);

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
            if (0 == strcmp(argv[i], "NASDAQ")) {
                scan_nasdaq = true;
            } else if (0 == strcmp(argv[i], "NYSE")) {
                scan_amex = true;
                scan_nyse = true;
            }
        }

        if (!scan_amex && !scan_nasdaq && !scan_nyse) {
            fprintf(stderr, "No correct marketplace parameters provided\n");
            exit(EXIT_FAILURE);
        }
    }

    if (csv_init(&parser, csv_options) != 0) {
        fprintf(stderr, "Error: Couldn’t initialize CSV parser");
        exit(EXIT_FAILURE);
    }
    csv_set_delim(&parser, CSV_COMMA);

    const char *csv_header = "marketplace,ticker,company,sector,industry,country,marketcap";
    csv_parse(&parser, csv_header, strlen(csv_header), csv_cb_end_of_field, csv_cb_end_of_row, stdout);
    csv_fini(&parser, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

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

    exit(EXIT_SUCCESS);
}
