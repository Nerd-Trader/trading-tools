#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>
typedef unsigned int uint;
#if __APPLE__
#include <tidybuffio.h>
#else
typedef unsigned long ulong;
#include <tidy/tidybuffio.h>
#endif

#include "config.h"
#include "curl.h"
#include "data-sources/finviz.h"
#include "ticker-scraper.h"

#define FINVIZ_PP  20 /* How many results per page finviz.com displays */
#define FINVIZ_URL "https://finviz.com/screener.ashx?v=110" /* Base URL */

typedef enum FINVIZ_MarketPlaces {
    FINVIZ_MPLACE_AMEX   = 1 << 0, // Legacy
    FINVIZ_MPLACE_NASDAQ = 1 << 1,
    FINVIZ_MPLACE_NYSE   = 1 << 2,
} FINVIZ_MarketPlace;

const char *html_entities[][2] = {
   { "&amp;",  "&" },
   { "&apos;", "'" },
   { "&gt;",   ">" },
   { "&lt;",   "<" },
   { "&quot;", "\"" },
   { "&copy;", "©" }
};

CURL *curl_handle;
unsigned int m;
char *cursor;
int page, total, new;

char *str_replace(const char *input, const char *pattern, const char *replacement)
{
    size_t numhits, pattern_len, replacement_len, output_len;
    const char *input_at = NULL;
    const char *input_prev = NULL;
    char *output = NULL;
    char *output_at = NULL;

    pattern_len = strlen(pattern);
    replacement_len = strlen(replacement);
    numhits = 0;
    input_at = input;

    while ((input_at = strstr(input_at, pattern))) {
        input_at += pattern_len;
        numhits++;
    }

    output_len = strlen(input) - pattern_len * numhits + replacement_len * numhits;
    output = (char *)malloc(output_len + 1);

    output_at = output;
    input_at = input_prev = input;

    while ((input_at = strstr(input_at, pattern))) {
        memcpy(output_at, input_prev, input_at - input_prev);
        output_at += input_at - input_prev;
        memcpy(output_at, replacement, replacement_len);
        output_at += replacement_len;
        input_at += pattern_len;
        input_prev = input_at;
    }

    strcpy(output_at, input_prev);

    return output;
}

static void sanitize_entities(const char *data, char *str, const size_t len)
{
    char sanitized_data[256];

    strncpy(sanitized_data, data, sizeof(sanitized_data) - 1);

    for (size_t i = 0; i < sizeof(html_entities) / sizeof(html_entities[0]); i++) {
        const char *find_string = html_entities[i][0];
        const char *replace_string = html_entities[i][1];

        if (strstr(sanitized_data, find_string) != 0) {
            char *tmp = str_replace(sanitized_data, find_string, replace_string);

            strncpy(sanitized_data, tmp, sizeof(sanitized_data) - 1);

            free(tmp);
        }
   }

   strncpy(str, sanitized_data, len);
}

void extract_text(const TidyDoc doc, const TidyNode tnod, char *destination, const ulong max_size)
{
    TidyBuffer buf;
    tidyBufInit(&buf);
    tidyNodeGetText(doc, tnod, &buf);
    sanitize_entities((char *)buf.bp, destination, max_size - 1);

    if (strlen(destination) < max_size) {
        destination[strlen(destination) - 1] = '\0';
    } else {
        destination[max_size - 1] = '\0';
    }

    tidyBufFree(&buf);
}

/* Traverse the document tree */
int process_node(TidyDoc doc, TidyNode tnod, const MarketPlace marketplace)
{
    TidyNode child;
    int tt = -1;

    /* Loop through children of tnod, looking for div#screener-content */
    for (child = tidyGetChild(tnod); child; child = tidyGetNext(child)) {
        ctmbstr name = tidyNodeGetName(child);

        if (name && strcmp(name, "div") == 0) {
            TidyAttr attr;
            for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
                if (strcmp(tidyAttrName(attr), "id") == 0 && strcmp(tidyAttrValue(attr), "screener-content") == 0) {
                    /* Found <div id="screener-content" */
                    const TidyNode table = tidyGetChild(child);

                    /* Dig into the DOM and find "Total:" value */
                    {
                        TidyNode tr = tidyGetChild(table);
                        tr = tidyGetNext(tr);
                        tr = tidyGetNext(tr);
                        TidyNode td = tidyGetChild(tr);
                        TidyNode table = tidyGetChild(td);
                        tr = tidyGetChild(table);
                        td = tidyGetChild(tr);
                        TidyNode b = tidyGetChild(td);
                        TidyNode text = tidyGetNext(b);
                        /* Parse total items number */
                        TidyBuffer buf;
                        tidyBufInit(&buf);
                        tidyNodeGetText(doc, text, &buf);
                        int t, n;
                        n = sscanf((char *)buf.bp, " %d #", &t);
                        tt = (n == 1) ? t : 0;
                        tidyBufFree(&buf);
                    }

                    /* Go back up to main table and parse all rows */
                    {
                        TidyNode tr = tidyGetChild(table);
                        tr = tidyGetNext(tr);
                        tr = tidyGetNext(tr);
                        tr = tidyGetNext(tr);
                        TidyNode td = tidyGetChild(tr);
                        TidyNode table = tidyGetChild(td);
                        tr = tidyGetChild(table);
                        tr = tidyGetNext(tr);
                        for (; tr; tr = tidyGetNext(tr)) {
                            DataRow dataRow;
                            dataRow.marketplace = marketplace;

                            TidyNode td = tidyGetChild(tr);
                            // Skip first column
                            td = tidyGetNext(td);
                            // Parse Ticker
                            {
                                TidyNode a = tidyGetChild(td);
                                TidyNode text = tidyGetChild(a);
                                extract_text(doc, text, dataRow.ticker, sizeof(dataRow.ticker));
                            }
                            // Go to the next column
                            td = tidyGetNext(td);
                            // Parse Company
                            {
                                TidyNode a = tidyGetChild(td);
                                TidyNode text = tidyGetChild(a);
                                extract_text(doc, text, dataRow.company, sizeof(dataRow.company));
                            }
                            // Go to the next column
                            td = tidyGetNext(td);
                            // Parse Sector
                            {
                                TidyNode a = tidyGetChild(td);
                                TidyNode text = tidyGetChild(a);
                                extract_text(doc, text, dataRow.sector, sizeof(dataRow.sector));
                            }
                            // Go to the next column
                            td = tidyGetNext(td);
                            // Parse Industry
                            {
                                TidyNode a = tidyGetChild(td);
                                TidyNode text = tidyGetChild(a);
                                extract_text(doc, text, dataRow.industry, sizeof(dataRow.industry));
                            }
                            // Go to the next column
                            td = tidyGetNext(td);
                            // Parse Country
                            {
                                TidyNode a = tidyGetChild(td);
                                TidyNode text = tidyGetChild(a);
                                extract_text(doc, text, dataRow.country, sizeof(dataRow.country));
                            }
                            // Go to the next column
                            td = tidyGetNext(td);
                            // Parse Market Cap
                            {
                                TidyNode a = tidyGetChild(td);
                                TidyNode text = tidyGetChild(a);
                                extract_text(doc, text, dataRow.marketcap, sizeof(dataRow.marketcap));
                                // Replace dash with empty string
                                if (0 == strcmp(dataRow.marketcap, "-")) {
                                    strcpy(dataRow.marketcap, "");
                                }
                            }
                            // Go to the next column
                            td = tidyGetNext(td);
                            // Go to the next column
                            td = tidyGetNext(td);
                            // Parse Price
                            {
                                TidyNode a = tidyGetChild(td);
                                TidyNode span = tidyGetChild(a);
                                TidyNode text = tidyGetChild(span);
                                if (text != NULL) {
                                    extract_text(doc, text, dataRow.price, sizeof(dataRow.price));
                                } else {
                                    extract_text(doc, span, dataRow.price, sizeof(dataRow.price));
                                }
                            }

                            new += ticker_scraper_add(&dataRow);
                            total++;
                        }
                    }
                }
            }
        }

        /* Dig deeper if couldn't find #screener-content above */
        if (tt <= 0) {
            tt = process_node(doc, child, marketplace);
        }
    }

    return tt;
}

int finviz_parse_page_extract_symbols(struct MemoryStruct *chunk, const MarketPlace marketplace)
{
    m = 0;
    int t = 0;

    TidyDoc tdoc;
    TidyBuffer docbuf = {0};
    TidyBuffer tidy_errbuf = {0};
    int err;

    tdoc = tidyCreate();
    tidyOptSetBool(tdoc, TidyForceOutput, true); /* Try harder */
    tidyOptSetInt(tdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer(tdoc, &tidy_errbuf);
    tidyBufInit(&docbuf);
    tidyBufAttach(&docbuf, (unsigned char *)chunk->memory, strlen(chunk->memory));

    err = tidyParseBuffer(tdoc, &docbuf); /* Parse the input */
    if (err >= 0) {
        err = tidyCleanAndRepair(tdoc); /* Fix problems */
        if (err >= 0) {
            err = tidyRunDiagnostics(tdoc); /* Load tidy error buffer */
            if (err >= 0) {
                t = process_node(tdoc, tidyGetRoot(tdoc), marketplace); /* Walk the tree */
#if DEBUG
                // fprintf(stderr, "%s\n", tidy_errbuf.bp); /* Display errors */
#endif
            }
        }
    }

    tidyBufFree(&docbuf);
    tidyBufFree(&tidy_errbuf);
    tidyRelease(tdoc);

    return t;
}

void finviz_scrape_page(struct MemoryStruct *chunk, const FINVIZ_MarketPlace finviz_marketplace, const MarketPlace marketplace)
{
    CURLcode res;

    chunk->memory = malloc(1); /* Will be grown as needed by realloc() in nerd_trader_curl_write_memory_callback */
    chunk->size = 0; /* No data at this point */

    /* Specify the URL */
    char url[strlen(FINVIZ_URL) + 10 + 3 + 11 + 1];
    strncpy(url, FINVIZ_URL, sizeof(url) - 1);
    url[sizeof(url) - 1] = '\0';

    switch (finviz_marketplace) {
        case FINVIZ_MPLACE_AMEX:
            strncat(url, "&f=exch_amex", sizeof(url) - 1 - strlen(url));
        break;

        case FINVIZ_MPLACE_NYSE:
            strncat(url, "&f=exch_nyse", sizeof(url) - 1 - strlen(url));
        break;

        case FINVIZ_MPLACE_NASDAQ:
            strncat(url, "&f=exch_nasd", sizeof(url) - 1 - strlen(url));
        break;

        default:
            free(chunk->memory);
            return;
    }

    if (page > 1) {
        strncat(url, "&r=", sizeof(url) - 1 - strlen(url));
        char pageNum[11];
        sprintf(pageNum, "%d", (page - 1) * FINVIZ_PP + 1);
        strncat(url, pageNum,  sizeof(url) - 1 - strlen(url) - strlen(pageNum));
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    /* Get it! */
    res = curl_easy_perform(curl_handle);

    /* Check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        int t = finviz_parse_page_extract_symbols(chunk, marketplace);
        fprintf(stderr, "Scraped %s tickers from FINVIZ page %d [%s]\n", marketplace_to_str(marketplace), page, url);
        page = (page * FINVIZ_PP < t) ? page + 1 : -1;
    }

    // free(chunk->memory);
}

int ticker_scraper_scrape_finviz_internal(const FINVIZ_MarketPlace finviz_marketplace, const MarketPlace marketplace)
{
    struct MemoryStruct chunk;

    /* Init curl */
    curl_handle = nerd_trader_curl_init(&chunk);

    page = 1;
    total = new = 0;

    while (page > 0) {
        if (page > 1) {
            /* Delay to avoid the "Too many requests" error */
            sleep(1);
        }

        finviz_scrape_page(&chunk, finviz_marketplace, marketplace);
    }

    nerd_trader_curl_cleanup(curl_handle);

    return new;
}

int ticker_scraper_scrape_finviz(const MarketPlace marketplace)
{
    int item_count = 0;

    if (marketplace == MPLACE_NASDAQ) {
        item_count += ticker_scraper_scrape_finviz_internal(FINVIZ_MPLACE_NASDAQ, MPLACE_NASDAQ);
    } else if (marketplace == MPLACE_NYSE) {
        item_count += ticker_scraper_scrape_finviz_internal(FINVIZ_MPLACE_NYSE, MPLACE_NYSE);
        // Finviz requires NYSE American to be scraped separately, as legacy "AMEX"
        item_count += ticker_scraper_scrape_finviz_internal(FINVIZ_MPLACE_AMEX, MPLACE_NYSE);
    } else {
        fprintf(stderr, "FINVIZ does not provide data for given marketplace\n");
    }

    return item_count;
}
