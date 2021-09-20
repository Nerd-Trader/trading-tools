#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <csv.h>

#include "config.h"
#include "data-sources/finviz.h"
#include "data-sources/otcmarkets.h"
#include "ticker-scraper.h"

static const char *csv_output_columns[] = {
    "marketplace",
    "ticker",
    "company",
    "price",
    "sector",
    "industry",
    "country",
    "marketcap",
};
static const int csv_output_column_count = sizeof(csv_output_columns) / sizeof(csv_output_columns[0]);

void explicit_bzero(void *s, size_t n);

size_t csv_field_index = 0;
struct csv_parser parser;
unsigned char csv_options = CSV_STRICT;

void csv_cb_end_of_field(void *s, size_t i, void *outfile) {
    csv_fwrite((FILE *)outfile, s, i);

    if (csv_field_index + 1 < csv_output_column_count) { /* Do not put separator after last field */
        fputc(CSV_COMMA, (FILE *)outfile);
    }

    csv_field_index++;
}

void csv_cb_end_of_row(int c, void *outfile) {
    (void)(c); // Suppress "unused parameter" compiler warning

    fputc('\n', (FILE *)outfile);

    csv_field_index = 0;
}

char *marketplace_to_str(MarketPlace marketplace)
{
    switch (marketplace)
    {
        case NASDAQ:
            return "NASDAQ";
        break;

        case NYSE:
        case AMEX:
            return "NYSE";
        break;

        case OTCQB:
            return "OTCQB";
        break;

        case OTCQX:
            return "OTCQX";
        break;

        case PINK:
            return "Pink";
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
    const char delimeter[2] = { csv_get_delim(&parser), 0 };

    char *marketplace = marketplace_to_str(dataRow->marketplace);
    csv_parse(&parser, marketplace, strlen(marketplace), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *ticker = escape_for_csv(dataRow->ticker);
    csv_parse(&parser, ticker, strlen(ticker), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *company = escape_for_csv(dataRow->company);
    csv_parse(&parser, company, strlen(company), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *price = escape_for_csv(dataRow->price);
    csv_parse(&parser, price, strlen(price), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *sector = escape_for_csv(dataRow->sector);
    csv_parse(&parser, sector, strlen(sector), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *industry = escape_for_csv(dataRow->industry);
    csv_parse(&parser, industry, strlen(industry), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *country = escape_for_csv(dataRow->country);
    csv_parse(&parser, country, strlen(country), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *marketcap = escape_for_csv(dataRow->marketcap);
    csv_parse(&parser, marketcap, strlen(marketcap), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_fini(&parser, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    free(ticker);
    free(company);
    free(price);
    free(sector);
    free(industry);
    free(country);
    free(marketcap);

    return 1;
}

int scrape_ticker_symbols(MarketPlace marketplace)
{
    int new = 0;

    switch (marketplace)
    {
        case AMEX:
            // Just a placeholder, no point in letting it being scraped separately from NYSE
            break;
        case NYSE:
        case NASDAQ:
#if DEBUG
            fprintf(stderr, "Scraping %s tickers…\n", marketplace_to_str(marketplace));
#endif
            new = ticker_scraper_scrape_finviz(marketplace);
            if (marketplace == NYSE) {
                // Finviz requires NYSE American to be scraped separately, as legacy "AMEX"
                new += ticker_scraper_scrape_finviz(AMEX);
            }
        break;

        case OTCQX:
        case OTCQB:
        case PINK:
#if DEBUG
            fprintf(stderr, "Scraping %s tickers…\n", marketplace_to_str(marketplace));
#endif
            new = ticker_scraper_scrape_otcmarkets(marketplace);
        break;
    }

    return new;
}

int main(int argc, char **argv)
{
    unsigned int new_symbols_retrieved = 0;
    bool no_params_provided = argc < 2;
    bool scan_nasdaq = false,
         scan_nyse = false,
         scan_otcqb = false,
         scan_otcqx = false,
         scan_pink = false;

    if (no_params_provided) {
        scan_nasdaq = true;
        scan_nyse = true;
    } else {
        for (int i = 1; i < argc; i++) {
            if (0 == strcmp(argv[i], "NASDAQ")) {
                scan_nasdaq = true;
            } else if (0 == strcmp(argv[i], "NYSE")) {
                scan_nyse = true;
            } else if (0 == strcmp(argv[i], "OTCQB")) {
                scan_otcqb = true;
            } else if (0 == strcmp(argv[i], "OTCQX")) {
                scan_otcqx = true;
            } else if (0 == strcmp(argv[i], "Pink")) {
                scan_pink = true;
            } else if (0 == strcmp(argv[i], "US")) {
                scan_nasdaq = true;
                scan_nyse = true;
            } else if (0 == strcmp(argv[i], "OTC")) {
                scan_otcqb = true;
                scan_otcqx = true;
                scan_pink = true;
            }
        }

        if (!scan_nasdaq && !scan_nyse && !scan_otcqb && !scan_otcqx && !scan_pink) {
            fprintf(stderr, "No correct marketplace parameters provided\n");
            exit(EXIT_FAILURE);
        }
    }

    if (csv_init(&parser, csv_options) != 0) {
        fprintf(stderr, "Error: Couldn’t initialize CSV parser\n");
        exit(EXIT_FAILURE);
    }
    csv_set_delim(&parser, CSV_COMMA);

    const int csv_header_mem_len = csv_output_column_count-1 + csv_output_column_count * sizeof(csv_output_columns[0]) + 1;
    char csv_header[csv_header_mem_len];
    explicit_bzero(csv_header, csv_header_mem_len);
    for(int i = 0; i < csv_output_column_count; i++) {
        const char delimeter[2] = { csv_get_delim(&parser), 0 };
        if (i > 0) {
            strcat(csv_header, delimeter);
        }
        strcat(csv_header, csv_output_columns[i]);
    }
    csv_parse(&parser, csv_header, strlen(csv_header), csv_cb_end_of_field, csv_cb_end_of_row, stdout);
    csv_fini(&parser, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    new_symbols_retrieved = 0;

    // US
    if (scan_nasdaq) {
        new_symbols_retrieved += scrape_ticker_symbols(NASDAQ);
    }
    if (scan_nyse) {
        new_symbols_retrieved += scrape_ticker_symbols(NYSE);
    }

    // OTC
    if (scan_otcqb) {
        new_symbols_retrieved += scrape_ticker_symbols(OTCQB);
    }
    if (scan_otcqx) {
        new_symbols_retrieved += scrape_ticker_symbols(OTCQX);
    }
    if (scan_pink) {
        new_symbols_retrieved += scrape_ticker_symbols(PINK);
    }

    csv_free(&parser);

    exit(EXIT_SUCCESS);
}
