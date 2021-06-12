#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <curl/curl.h>

#include "../config.h"
#include "../curl.h"
#include "../ticker-scraper.h"
#include "finviz.h"

#define FINVIZ_PP   20 /* How many results per page finviz.com displays */
#define REG_PATTERN "class=\"screener-link-primary\">([^<]*)</a></td><td height=\"10\" align=\"left\" class=\"screener-body-table-nw\"><a href=\"[^\"]*\" class=\"screener-link\">([^<]*)</a></td>"
#define REG_GROUPS  3

ssize_t s;
CURL *curl_handle;
regex_t regex_compiled;
regmatch_t group_array[REG_GROUPS];
unsigned int m;
char *cursor;
int page, total, new;

int finviz_parse_page_get_total(struct MemoryStruct* chunk)
{
    int total, n;

    char *start = strstr(chunk->memory, "Total:");
    n = sscanf(start, "Total: </b>%d #", &total);

    return (n == 1) ? total : 0;
}

void finviz_parse_page_extract_symbols(struct MemoryStruct *chunk, MarketPlace marketplace)
{
    m = 0;
    unsigned int g;
    cursor = chunk->memory;
    unsigned int maxMatches = FINVIZ_PP;

    // Iterate over matches
    for (m = 0; m < maxMatches; m++) {
        if (regexec(&regex_compiled, cursor, REG_GROUPS, group_array, 0) == REG_NOMATCH) {
            /* No more matches */
            break;
        }

        unsigned int offset = 0;
        Symbol symbol;
        char company_name[256];

        // Iterate over matching groups within every match
        for (g = 0; g < REG_GROUPS; g++) {
            if (group_array[g].rm_so == -1) {
                /* No more groups */
                break;
            }

            char cursorCopy[strlen(cursor) + 1];

            switch (g) {
                case 0:
                    offset = group_array[g].rm_eo;
                break;

                case 1:
                    strncpy(cursorCopy, cursor, sizeof(cursorCopy) - 1);
                    cursorCopy[sizeof(cursorCopy) - 1] = '\0';
                    cursorCopy[group_array[g].rm_eo] = 0;
                    strncpy(symbol, cursorCopy + group_array[g].rm_so, sizeof(symbol) - 1);
                    symbol[sizeof(symbol) - 1] = '\0';
                break;

                case 2:
                    strncpy(company_name, cursorCopy + group_array[g].rm_so, sizeof(company_name) - 1);
                    company_name[group_array[g].rm_eo - group_array[g].rm_so] = '\0';

                    new += ticker_scraper_add(marketplace, symbol, company_name);
                    total++;
                break;
            }
        }

        cursor += offset;
    }
}

void finviz_scrape_page(struct MemoryStruct *chunk, MarketPlace marketplace)
{
    CURLcode res;

    chunk->memory = malloc(1); /* Will be grown as needed by realloc() in nerd_trader_curl_write_memory_callback */
    chunk->size = 0; /* No data at this point */

    /* Specify the URL to get */
    char url[strlen(RESOURCE_FINVIZ_SCAN_URL) + 10 + 5 + 1];
    strncpy(url, RESOURCE_FINVIZ_SCAN_URL, sizeof(url) - 1);
    url[sizeof(url)-1] = '\0';

    switch (marketplace) {
        case AMEX:
            strcat(url, ",exch_amex");
        break;

        case NYSE:
            strcat(url, ",exch_nyse");
        break;

        case NASDAQ:
            strcat(url, ",exch_nasd");
        break;

        default:
            free(chunk->memory);
            return;
    }

    if (page > 1) {
        strcat(url, "&r=");
        char pageNum[11];
        sprintf(pageNum, "%d", (page - 1) * FINVIZ_PP + 1);
        strcat(url, pageNum);
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /* Get it! */
    res = curl_easy_perform(curl_handle);

#if DEBUG
    fprintf(stderr, "Scraping page %d: %s\n", page, url);
#endif

    /* Check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
    } else {
        if (strlen(chunk->memory) > 1000) { // Protection against error pages
            finviz_parse_page_extract_symbols(chunk, marketplace);
            page = (page * FINVIZ_PP < finviz_parse_page_get_total(chunk)) ? page + 1 : 0;
        } else {
            fprintf(stderr, "Error scraping page %s\n", url);
#if DEBUG
            fprintf(stderr, "%s\n", chunk->memory);
#endif
        }
    }

    free(chunk->memory);
}

int ticker_scraper_scrape_finviz(MarketPlace marketplace)
{
    struct MemoryStruct chunk;

    if (regcomp(&regex_compiled, REG_PATTERN, REG_EXTENDED) != REG_NOERROR) {
        fprintf(stderr, "Failed to compile regex %s\n", REG_PATTERN);
        return EXIT_FAILURE;
    }

    /* Init the curl */
    curl_handle = nerd_trader_curl_init(&chunk);

    while (true) {
        page = 1;
        s = total = new = 0;

        while (page > 0) {
            finviz_scrape_page(&chunk, marketplace);
        }

        /* Exit from the infinite loop */
        break;
    }

    regfree(&regex_compiled);
    nerd_trader_curl_cleanup(curl_handle);

    return new;
}
