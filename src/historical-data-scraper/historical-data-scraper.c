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
long min_market_cap = 0l;
long max_market_cap = 0l;
bool is_including_unset_price = false;
bool is_including_unset_market_cap = false;

DataRow csv_input_data_row;
int csv_input_field_index = 0;
int csv_input_row_index = 0;

void csv_cb_end_of_field(void *s, size_t l, void *outfile)
{
    (void)(outfile); // Suppress "unused parameter" compiler warning

    if (csv_input_row_index > 0) {
        char string[l + 1];
        string[l] = '\0';
        memcpy(&string, s, l);

        switch (csv_input_field_index) {
            case 0:
                csv_input_data_row.marketplace = str_to_marketplace(string);
                break;
            case 1:
                explicit_bzero(csv_input_data_row.ticker, sizeof(Symbol));
                memcpy(csv_input_data_row.ticker, string, l);
                break;
            case 2:
                explicit_bzero(csv_input_data_row.company, sizeof(csv_input_data_row.company));
                memcpy(csv_input_data_row.company, string, l);
                break;
            case 3:
                csv_input_data_row.price = atof(string);
                break;
            case 4:
                explicit_bzero(csv_input_data_row.sector, sizeof(csv_input_data_row.sector));
                memcpy(csv_input_data_row.sector, string, l);
                break;
            case 5:
                explicit_bzero(csv_input_data_row.industry, sizeof(csv_input_data_row.industry));
                memcpy(csv_input_data_row.industry, string, l);
                break;
            case 6:
                explicit_bzero(csv_input_data_row.country, sizeof(csv_input_data_row.country));
                memcpy(csv_input_data_row.country, string, l);
                break;
            case 7:
                csv_input_data_row.marketcap = parse_marketcap_str(string);
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
                (csv_input_data_row.price != 0.0f || is_including_unset_price) &&
                (
                    (min_price == 0.0f || csv_input_data_row.price >= min_price) &&
                    (max_price == 0.0f || csv_input_data_row.price <= max_price)
                )
            )
            &&
            (
                (csv_input_data_row.marketcap != 0l || is_including_unset_market_cap) &&
                (
                    (min_market_cap == 0l || csv_input_data_row.marketcap >= min_market_cap) &&
                    (max_market_cap == 0l || csv_input_data_row.marketcap <= max_market_cap)
                )
            )
        ) {
            fprintf(stderr, "%s\t", csv_input_data_row.ticker);
            fprintf(stderr, "data ");
            char *historical_data = historical_data_scraper_scrape_tdameritrade(&csv_input_data_row);
            if (historical_data != NULL) {
                fprintf(stderr, "✅");

                // Write historical JSON data into <output dir>/<ticker>.json
                FILE * fp;
                char fpath[2048];
                explicit_bzero(fpath, sizeof(fpath));
                strcat(fpath, output_path);
                strcat(fpath, "/");
                strcat(fpath, csv_input_data_row.ticker);
                strcat(fpath, ".json");

                fprintf(stderr, " → file ");
                fp = fopen(fpath, "w");
                if (fp != NULL) {
                    fputs(historical_data, fp);
                    fputs("\n", fp);
                    fprintf(stderr, "✅");
                    fclose(fp);
                } else {
                    fprintf(stderr, "❌");
                }
            } else {
                fprintf(stderr, "❌");
            }
            fprintf(stderr, "\n");
        }
    }

    csv_input_row_index++;
    csv_input_field_index = 0;
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
        { "output-dir",                 required_argument, NULL,                                  'o'   },
        { "min-price",                  required_argument, NULL,                                  'p'   },
        { "max-price",                  required_argument, NULL,                                  'P'   },
        { "min-market-cap",             required_argument, NULL,                                  'm'   },
        { "max-market-cap",             required_argument, NULL,                                  'M'   },
        { "include-missing-price",      no_argument,       (int *)&is_including_unset_price,      true  },
        { "include-missing-market-cap", no_argument,       (int *)&is_including_unset_market_cap, true  },
        { "help",                       no_argument,       NULL,                                  'h'   },
        { NULL,                         0,                 NULL,                                  false }
    };

    while (true) {
        opt = getopt_long(argc, (char**)argv, "o:p:P:m:M:h:", long_options, &option_index);

        /* Detect the end of the options. */
        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 0:
                break;

            case 'o':
                strcpy(output_path, optarg);
                break;

            case 'p':
            case 'P':
                {
                    char value_str[32];
                    explicit_bzero(value_str, sizeof(value_str));
                    memcpy(value_str, optarg, strlen(optarg));
                    float value = atof(value_str);
                    if (opt == 'p') {
                        min_price = value;
                    } else if (opt == 'P') {
                        max_price = value;
                    }
                }
                break;

            case 'm':
                min_market_cap = parse_marketcap_str(optarg);
                break;

            case 'M':
                max_market_cap = parse_marketcap_str(optarg);
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
