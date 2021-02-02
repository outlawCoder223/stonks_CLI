#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl/curl.h"

int main(int argc, char *argv[]) {
    CURL *curl = curl_easy_init();
    char URL[256] = "https://finnhub.io/api/v1/quote?symbol=";
    char *API = getenv("FINNHUB_API_KEY");
    int API_size = strlen(API);

    if(argc < 2) {
        printf("I need a stock symbol\n");
        return EXIT_FAILURE;
    } else if (!API)
    {
        printf("Set your FINNHUB_API_KEY env var");
        return EXIT_FAILURE;
    }
    

    if (!curl) {
        fprintf(stderr, "init failed\n");
        return EXIT_FAILURE;
    }

    CURLcode res;
    strncat(URL, argv[1], 10);
    strncat(URL, "&token=", 8);
    strncat(URL, API, API_size);
    printf("%s\n", URL);
    curl_easy_setopt(curl, CURLOPT_URL, URL);
    res = curl_easy_perform(curl);
    
    curl_easy_cleanup(curl);
    printf("\n");
    return EXIT_SUCCESS;
}