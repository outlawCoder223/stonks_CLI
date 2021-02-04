#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl/curl.h"

typedef struct {
  char *ptr;
  size_t len;
} CURLdata;

typedef char URL[256];

void init_CURLdata(CURLdata *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, CURLdata *data)
{
  size_t new_len = data->len + size*nmemb;
  data->ptr = realloc(data->ptr, new_len+1);
  if (data->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(data->ptr + data->len, ptr, size*nmemb);
  data->ptr[new_len] = '\0';
  data->len = new_len;

  return size*nmemb;
}

void delchar(char *str, int num_delete)
{
    int len = strlen(str) - num_delete;
    if ((num_delete) <= strlen(str)) {
        memmove(&str[0], &str[num_delete], len);
        str[len] = '\0';
    }
}

void buildURL(URL *base_url, char *symbol, char *API, int API_len) {
    strncat(*base_url, "quote?symbol=", 14);
    strncat(*base_url, symbol, 10);
    strncat(*base_url, "&token=", 8);
    strncat(*base_url, API, API_len);
}

void getQuote(CURL *curl, CURLcode res, URL *url, CURLdata *data) {
    curl_easy_setopt(curl, CURLOPT_URL, *url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    res = curl_easy_perform(curl);
}

int main(int argc, char *argv[]) {
    CURL *curl = curl_easy_init();
    CURLcode res;
    CURLdata data;
    URL base_url = "https://finnhub.io/api/v1/";
    char *API = getenv("FINNHUB_API_KEY");
    int API_len = strlen(API);
    char *curr, *high, *low;
    char *symbol = argv[1];
    char copy[256];

    if (argc < 2) {
        printf("I need a stock symbol\n");
        return EXIT_FAILURE;
    } else if (!API) {
        printf("Set your FINNHUB_API_KEY env var");
        return EXIT_FAILURE;
    }

    if (!curl) {
        fprintf(stderr, "init failed\n");
        return EXIT_FAILURE;
    }

    init_CURLdata(&data);

    buildURL(&base_url, symbol, API, API_len);
    getQuote(curl, res, &base_url, &data);

    memcpy(copy, data.ptr, data.len);
    curr = strtok(copy, ",");
    high = strtok(NULL, ",");
    low = strtok(NULL, ",");

    delchar(curr, 5);
    delchar(high, 4);
    delchar(low, 4);

    printf("-------------\n");
    printf("Quote for %s\n", symbol);
    printf("Current price:\t$%s\n", curr);
    printf("Daily High:\t$%s\n", high);
    printf("Daily Low:\t$%s\n", low);
    printf("-------------\n");

    free(data.ptr);
    data.ptr = NULL;

    curl_easy_cleanup(curl);

    return EXIT_SUCCESS;
}