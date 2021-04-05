#pragma once

#include <curl/curl.h>

typedef struct MemoryStruct {
    char   *memory;
    size_t size;
} MemoryStruct;

void nerd_trader_curl_cleanup(CURL *curl_handle);
CURL *nerd_trader_curl_init(MemoryStruct *chunk);
size_t nerd_trader_curl_write_memory_callback(void *contents, size_t size,
                                                size_t nmemb, void *userp);
