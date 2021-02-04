#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl/curl.h"

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

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

int main(int argc, char *argv[]) {
    CURL *curl = curl_easy_init();
    CURLcode res;
    char URL[256] = "https://finnhub.io/api/v1/quote?symbol=";
    char *API = getenv("FINNHUB_API_KEY");
    int API_size = strlen(API);
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


    struct string s;
    init_string(&s);
    strncat(URL, symbol, 10);
    strncat(URL, "&token=", 8);
    strncat(URL, API, API_size);

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    res = curl_easy_perform(curl);

    memcpy(copy, s.ptr, s.len);
    curr = strtok(copy, ",");
    high = strtok(NULL, ",");
    low = strtok(NULL, ",");

    delchar(curr, 5);
    delchar(high, 4);
    delchar(low, 4);

    printf("-------------\n");
    printf("Quote for %s\n", symbol);
    printf("Current price: $%s\n", curr);
    printf("Daily High: $%s\n", high);
    printf("Daily Low: $%s\n", low);
    printf("-------------\n");

    free(s.ptr);

    curl_easy_cleanup(curl);

    return EXIT_SUCCESS;
}