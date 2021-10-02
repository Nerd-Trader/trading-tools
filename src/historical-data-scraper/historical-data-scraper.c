#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <csv.h>

#include "config.h"
#include "data-sources/tdameritrade.h"
#include "historical-data-scraper.h"

void explicit_bzero(void *s, size_t n);

char output_path[2048] = ".";
float min_price = 0.0f;
float max_price = 0.0f;
float min_market_cap = 0.0f;
float max_market_cap = 0.0f;
bool is_including_unset_marketcap = false;
bool is_including_unset_price = false;

DataRow csv_input_data_row;
int csv_input_field_index = 0;
int csv_input_row_index = 0;

void csv_cb_end_of_field(void *s, size_t l, void *outfile)
{
    (void)(outfile); // Suppress "unused parameter" compiler warning

    if (csv_input_row_index > 0) {
        int field_value_len = l + 1;
        char string[field_value_len];
        explicit_bzero(string, sizeof(string));
        memcpy(&string, s, l);
        string[field_value_len] = '\0';

        switch (csv_input_field_index) {
            case 0:
                {
                    char mplace_str[field_value_len + 1];
                    explicit_bzero(mplace_str, sizeof(mplace_str));
                    memcpy(mplace_str, string, field_value_len);
                    csv_input_data_row.marketplace = str_to_marketplace(mplace_str);
                }
                break;
            case 1:
                memcpy(csv_input_data_row.ticker, string, field_value_len);
                break;
            case 2:
                memcpy(csv_input_data_row.company, string, field_value_len);
                break;
            case 3:
                {
                    char price_str[field_value_len + 1];
                    explicit_bzero(price_str, sizeof(price_str));
                    memcpy(price_str, string, field_value_len);
                    float price = atof(price_str);
                    csv_input_data_row.price = price;
                }
                break;
            case 4:
                memcpy(csv_input_data_row.sector, string, field_value_len);
                break;
            case 5:
                memcpy(csv_input_data_row.industry, string, field_value_len);
                break;
            case 6:
                memcpy(csv_input_data_row.country, string, field_value_len);
                break;
            case 7:
                memcpy(csv_input_data_row.marketcap, string, field_value_len);
                break;
        }
    }

    csv_input_field_index++;
}

void csv_cb_end_of_row(int c, void *outfile)
{
    (void)(c); // Suppress "unused parameter" compiler warning
    (void)(outfile); // Suppress "unused parameter" compiler warning

    if (csv_input_row_index > 0) {
        if (
                (
                    (min_price > 0.0f && csv_input_data_row.price >= min_price) &&
                    (max_price > 0.0f && csv_input_data_row.price <= max_price)
                )
        ) {
            char *historical_data = historical_data_scraper_scrape_tdameritrade(&csv_input_data_row);
            if (historical_data != NULL) {
                fprintf(stderr, "\nSuccessfully obtained historical data for ticker %s\n", csv_input_data_row.ticker);

                // Write historical JSON data into <output dir>/<ticker>.json
                FILE * fp;
                char fpath[2048];
                explicit_bzero(fpath, sizeof(fpath));
                strcat(fpath, output_path);
                strcat(fpath, "/");
                strcat(fpath, csv_input_data_row.ticker);
                strcat(fpath, ".json");
                fp = fopen(fpath, "w");
                fprintf(stderr, "Attempting to write historical data into %s\n", fpath);
                if (fp != NULL) {
                    fputs(historical_data, fp);
                    fputs("\n", fp);
                    fprintf(stderr, "Wrote obtained historical data into %s\n", fpath);
                    fclose(fp);
                } else {
                    fprintf(stderr, "Error: Unable to write obtained historical data into %s\n", fpath);
                }
            } else {
                fprintf(stderr, "Error: Unable to obtain historical data for ticker %s\n", csv_input_data_row.ticker);
            }
        }
    }

    csv_input_row_index++;
    csv_input_field_index = 0;
}

MarketPlace str_to_marketplace(const char *mplace_str)
{
    if (strcmp(mplace_str, "NASDAQ") == 0) {
        return NASDAQ;
    } else if (strcmp(mplace_str, "NYSE") == 0) {
        return NYSE;
    } else if (strcmp(mplace_str, "OTCQB") == 0) {
        return OTCQB;
    } else if (strcmp(mplace_str, "OTCQX") == 0) {
        return OTCQX;
    } else if (strcmp(mplace_str, "Pink") == 0) {
        return PINK;
    }

    return UNKNOWN;
}

int main(const int argc, const char **argv)
{
    struct csv_parser parser;
    int opt;
    int option_index = 0;
    int i;
    char b;
    FILE *fp;

    if (strlen(TD_AMERITRADE_API_KEY) == 0) {
        fprintf(stderr, "Error: Missing TD Ameritrade API key\n");
        exit(EXIT_FAILURE);
    }

    if (csv_init(&parser, CSV_STRICT) != 0) {
        fprintf(stderr, "Error: Couldn’t initialize CSV parser\n");
    }

    static const struct option long_options[] = {
        { "output-dir",                 required_argument, NULL,                                   'o'   },
        { "min-price",                  required_argument, NULL,                                   'p'   },
        { "max-price",                  required_argument, NULL,                                   'P'   },
        { "min-market-cap",             required_argument, NULL,                                   'm'   },
        { "max-market-cap",             required_argument, NULL,                                   'M'   },
        { "include-missing-price",      no_argument,       (int *)&is_including_unset_price,       true  },
        { "include-missing-market-cap", no_argument,       (int *)&is_including_unset_marketcap,   true  },
        { "help",                       no_argument,       NULL,                                   'h'   },
        { NULL,                         0,                 NULL,                                   false }
    };

    while (true) {
        opt = getopt_long(argc, (char**)argv, "o:p:P:m:M:h:", long_options, &option_index);

        /* Detect the end of the options. */
        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 'o':
                strcpy(output_path, optarg);
                break;

            case 'p':
            case 'P':
            case 'm':
            case 'M':
                {
                    char value_str[64];
                    explicit_bzero(value_str, sizeof(value_str));
                    memcpy(value_str, optarg, strlen(optarg));
                    float value = atof(value_str);
                    if (opt == 'p') {
                        min_price = value;
                    } else if (opt == 'P') {
                        max_price = value;
                    } else if (opt == 'm') {
                        min_market_cap = value;
                    } else if (opt == 'M') {
                        max_market_cap = value;
                    }
                }
                break;

            case 'h':
                printf("Usage: %s [OPTIONS] [TARGET] ...\n", argv[0]);
                puts("Options:");
                printf("  -o PATH, --output-dir=PATH       Specify output directory.\n");
                printf("  -p VALUE, --min-price=VALUE      Exclude items below minimum price per share.\n");
                printf("  -P VALUE, --max-price=VALUE      Exclude items above maximum price per share.\n");
                printf("  -m VALUE, --min-market-cap=VALUE Exclude items below minimum market cap.\n");
                printf("  -M VALUE, --max-market-cap=VALUE Exclude items above maximum market cap.\n");
                printf("  --include-missing-price          Omit price limits for items that miss price information.\n");
                printf("  --include-missing-market-cap     Omit market cap limits for items that miss market cap information.\n");
                printf("  -h, --help                       Print this help and exit.\n");
                puts("");
                exit(EXIT_SUCCESS);

            default:
                fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        // Ensure the output directory exists
        mkdir(output_path, S_IRWXU);

        while (optind < argc) {
            // Open and read input CSV file
            char file_path[strlen(argv[optind]) + 1];
            explicit_bzero(file_path, sizeof(file_path));
            strcpy(file_path, argv[optind++]);
            fprintf(stderr, "Attempting to open input file %s…\n", file_path);
            if ((fp = fopen(file_path, "r"))) {
                fprintf(stderr, "Successfully opened input file %s…\n", file_path);
                // Parse CSV
                while ((i = getc(fp)) != EOF) {
                    b = i;

                    if (csv_parse(&parser, &b, 1, csv_cb_end_of_field, csv_cb_end_of_row, NULL) != 1) {
                        fprintf(stderr, "Error: %s\n", csv_strerror(csv_error(&parser)));
                    }
                }

                csv_fini(&parser, csv_cb_end_of_field, csv_cb_end_of_row, NULL);
                csv_free(&parser);

                fclose(fp);
            } else {
                fprintf(stderr, "Error: Unable to open input file %s\n", file_path);
                continue;
            }
        }
    } else {
        fprintf(stderr, "Error: No input file(s) specified\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
