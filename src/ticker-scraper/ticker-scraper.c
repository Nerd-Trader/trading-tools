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

void explicit_bzero(void *s, size_t n);

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

static bool is_allowed_to_output_csv_header = true;
static size_t csv_field_index = 0;
static struct csv_parser parser;

void csv_cb_end_of_field(void *s, size_t i, void *outfile) {
    csv_fwrite((FILE *)outfile, s, i);

    if (csv_field_index + 1 < csv_output_column_count) { /* Do not put separator after last field */
        fputc(csv_get_delim(&parser), (FILE *)outfile);
    }

    csv_field_index++;
}

void csv_cb_end_of_row(int c, void *outfile) {
    (void)(c); // Suppress "unused parameter" compiler warning

    fputc('\n', (FILE *)outfile);

    csv_field_index = 0;
}

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

char *escape_for_csv(const char *input)
{
    size_t input_len = strlen(input);
    size_t temp_len = input_len*2 + 2;
    char *temp = malloc(temp_len);

    explicit_bzero(temp, temp_len);

    csv_write(temp, temp_len, input, input_len);

    return temp;
}

int ticker_scraper_add(const DataRow *data_row)
{
    const char delimeter[2] = { csv_get_delim(&parser), 0 };

    char *marketplace = marketplace_to_str(data_row->marketplace);
    csv_parse(&parser, marketplace, strlen(marketplace), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *ticker = escape_for_csv(data_row->ticker);
    csv_parse(&parser, ticker, strlen(ticker), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *company = escape_for_csv(data_row->company);
    csv_parse(&parser, company, strlen(company), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *price = escape_for_csv(data_row->price);
    csv_parse(&parser, price, strlen(price), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *sector = escape_for_csv(data_row->sector);
    csv_parse(&parser, sector, strlen(sector), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *industry = escape_for_csv(data_row->industry);
    csv_parse(&parser, industry, strlen(industry), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *country = escape_for_csv(data_row->country);
    csv_parse(&parser, country, strlen(country), csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    csv_parse(&parser, delimeter, 1, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    char *marketcap = escape_for_csv(data_row->marketcap);
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

int scrape_ticker_symbols(const MarketPlace marketplace)
{
    int new = 0;

    switch (marketplace) {
        case MPLACE_NYSE:
        case MPLACE_NASDAQ:
            fprintf(stderr, "Scraping %s tickers from %s…\n", marketplace_to_str(marketplace), "FINVIZ");
            new = ticker_scraper_scrape_finviz(marketplace);
        break;

        case MPLACE_OTCQB:
        case MPLACE_OTCQX:
        case MPLACE_PINK:
            fprintf(stderr, "Scraping %s tickers from %s…\n", marketplace_to_str(marketplace), "OTC Markets");
            new = ticker_scraper_scrape_otcmarkets(marketplace);
        break;

        default:
        case MPLACE_UNKNOWN:
            fprintf(stderr, "Error: Unknown marketplace\n");
    }

    return new;
}

int main(const int argc, const char **argv)
{
    unsigned int new_symbols_retrieved = 0;
    bool marketplace_params_provided = false;
    bool scrape_nasdaq = false,
         scrape_nyse = false,
         scrape_otcqb = false,
         scrape_otcqx = false,
         scrape_pink = false;

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp(argv[i], "--no-csv-header")) {
            is_allowed_to_output_csv_header = false;
        } else if (0 == strcmp(argv[i], "NASDAQ")) {
            scrape_nasdaq = true;
            marketplace_params_provided = true;
        } else if (0 == strcmp(argv[i], "NYSE")) {
            scrape_nyse = true;
            marketplace_params_provided = true;
        } else if (0 == strcmp(argv[i], "OTCQB")) {
            scrape_otcqb = true;
            marketplace_params_provided = true;
        } else if (0 == strcmp(argv[i], "OTCQX")) {
            scrape_otcqx = true;
            marketplace_params_provided = true;
        } else if (0 == strcmp(argv[i], "Pink")) {
            scrape_pink = true;
            marketplace_params_provided = true;
        } else if (0 == strcmp(argv[i], "US")) {
            scrape_nasdaq = true;
            scrape_nyse = true;
            marketplace_params_provided = true;
        } else if (0 == strcmp(argv[i], "OTC")) {
            scrape_otcqb = true;
            scrape_otcqx = true;
            scrape_pink = true;
            marketplace_params_provided = true;
        } else {
            fprintf(stderr, "Error: Unrecognized parameter %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    if (!marketplace_params_provided) {
        // Scrape only US stocks by default
        scrape_nasdaq = true;
        scrape_nyse = true;
    }

    if (csv_init(&parser, CSV_STRICT) != 0) {
        fprintf(stderr, "Error: Couldn’t initialize CSV parser\n");
        exit(EXIT_FAILURE);
    }

    csv_set_delim(&parser, CSV_COMMA);

    if (is_allowed_to_output_csv_header) {
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
    }

    csv_fini(&parser, csv_cb_end_of_field, csv_cb_end_of_row, stdout);

    new_symbols_retrieved = 0;

    // US
    if (scrape_nasdaq) {
        new_symbols_retrieved += scrape_ticker_symbols(MPLACE_NASDAQ);
    }
    if (scrape_nyse) {
        new_symbols_retrieved += scrape_ticker_symbols(MPLACE_NYSE);
    }

    // OTC
    if (scrape_otcqb) {
        new_symbols_retrieved += scrape_ticker_symbols(MPLACE_OTCQB);
    }
    if (scrape_otcqx) {
        new_symbols_retrieved += scrape_ticker_symbols(MPLACE_OTCQX);
    }
    if (scrape_pink) {
        new_symbols_retrieved += scrape_ticker_symbols(MPLACE_PINK);
    }

    csv_free(&parser);

    exit(EXIT_SUCCESS);
}
