#include <stdio.h>
#include <stdlib.h>
#include "curl/curl.h"

int main(void) {
    CURL *curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "init failed\n");
        return EXIT_FAILURE;
    }
    printf("curl worked\n");
    curl_easy_cleanup(curl);

    return EXIT_SUCCESS;
}