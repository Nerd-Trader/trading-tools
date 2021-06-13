#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "config.h"
#include "curl.h"
#include "ticker-scraper.h"

size_t nerd_trader_curl_write_memory_callback(void *contents, size_t size,
                                     size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
#if DEBUG
        fprintf(stderr, "Out of memory!\n");
#endif
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

CURL *nerd_trader_curl_init(MemoryStruct *chunk)
{
    CURL *curl_handle;

    curl_global_init(CURL_GLOBAL_ALL);
    /* Init the curl session */
    curl_handle = curl_easy_init();
    /* Send all data to this function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION,
                        nerd_trader_curl_write_memory_callback);
    /* We pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)chunk);
    /* Some servers don't like requests that are made without a user-agent
        field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USER_AGENT);
    /* Enable all supported built-in compressions */
    curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);

    return curl_handle;
}

void nerd_trader_curl_cleanup(CURL *curl_handle)
{
    /* Clean-up curl stuff */
    curl_easy_cleanup(curl_handle);

    /* We're done with libcurl, clean it up */
    curl_global_cleanup();
}
