#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <curl/curl.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#define BWHT    "\e[1;37m"
#define CLEAR   "\e[1;1H\e[2J"

#define BASEURL        "https://finnhub.io/api/v1/"
#define BASEURL_LEN    27
#define MAX_TICKERS    5
#define POLL_OFFSET    2
#define NONPOLL_OFFSET 1
#define SLEEP_TIME     10
 
typedef struct {
  char *ptr;
  size_t len;
} string;

/* data type modeling necessary information of the API response */
typedef struct {
    float curr;
    float high;
    float low;
    float open;
} quote_res;

typedef char URL[256];

/* data type modeling all information necessary to make API call and save response */
typedef struct {
    CURLSH *share;
    int quote_num;
    char *symbol;
    URL url;
    char *api_key;
    int api_len;
    quote_res q_data;
} quote_t;

/* initialize the struct needed to receive the GET request */
void init_string(string *data) {
  data->len = 0;
  data->ptr = malloc(data->len+1);
  if (data->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  data->ptr[0] = '\0';
}

/* CURL provides format for write functions in their documentation */
size_t writefunc(void *ptr, size_t size, size_t nmemb, string *data)
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

/* removes chars from beginning of string. */
void delchar(char *str, int num_delete) {
    int len = strlen(str) - num_delete;
    if ((num_delete) <= strlen(str)) {
        memmove(&str[0], &str[num_delete], len);
        str[len] = '\0';
    }
}

/* construct necessary URL for API GET quote request. */
void buildURL(quote_t *quote) {

    strncat(quote->url, "quote?symbol=", 14);
    strncat(quote->url, quote->symbol, 10);
    strncat(quote->url, "&token=", 8);
    strncat(quote->url, quote->api_key, quote->api_len);
}

/* use CURL library to get quote data and save in string struct */
void getQuoteData(URL *url, string *data) {
    CURL *curl = curl_easy_init();
    CURLcode res;

    curl_easy_setopt(curl, CURLOPT_URL, *url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
}

/* API returns JSON formatted data. parseQuote splits on commas and
   deletes the leading chars to get just the price for each field */
void parseQuote(quote_res *quote, string *data) {
    char *curr, *high, *low, *open;

    curr = strtok(data->ptr, ",");
    high = strtok(NULL, ",");
    low = strtok(NULL, ",");
    open = strtok(NULL, ",");

    delchar(curr, 5);
    delchar(high, 4);
    delchar(low, 4);
    delchar(open, 4);

    quote->curr = atof(curr);
    quote->high = atof(high);
    quote->low = atof(low);
    quote->open = atof(open);
}

/* print the parsed quote data nicely to console */
void printQuote(quote_t *quote, bool clear) {
    // static bool init = true;
    float diff = quote->q_data.curr - quote->q_data.open;
    float percent = (diff / quote->q_data.open) * 100;

    if (clear) {
        printf(CLEAR);
        // init = false;
    }
    printf(BWHT "%s\n" RESET, quote->symbol);
    printf("Current price:\t");
    if (diff < 0.0) {
        printf(RED "$%.2f %.2f (%.2f%%)\n" RESET, quote->q_data.curr, diff, percent);
    } else {
        printf(GREEN "$%.2f +%.2f (+%.2f%%)\n" RESET, quote->q_data.curr, diff, percent);
    }
    printf("Opened At:\t");
    printf(CYAN "$%.2f\n" RESET, quote->q_data.open);
    printf("Daily High:\t");
    printf(YELLOW "$%.2f\n" RESET, quote->q_data.high);
    printf("Daily Low:\t");
    printf(RED "$%.2f\n\n" RESET, quote->q_data.low);
    
}

/* handling function for thread */
static void *getQuote(void *arg) {
    quote_t *quote = (quote_t *)arg;
    string data;
    memcpy(quote->url, BASEURL, BASEURL_LEN);

    init_string(&data);
    buildURL(quote);
    getQuoteData(&quote->url, &data);
    parseQuote(&quote->q_data, &data);
    
    free(data.ptr);
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t tid[argc];
    quote_t quotes[argc - 1];
    char *API_KEY = getenv("FINNHUB_API_KEY");
    int API_LEN = strlen(API_KEY);
    int i, offset, loop_max, error_create, error_join;
    bool poll_en = false;

    curl_global_init(CURL_GLOBAL_ALL);

    /* check if user entered everything properly */
    if (argc < 2) {
        fprintf(stderr, "I need a stock symbol\n");
        return EXIT_FAILURE;
    } else if (!API_KEY) {
        fprintf(stderr, "Set your FINNHUB_API_KEY env var");
        return EXIT_FAILURE;
    } 

    /* if in polling mode, offset for extra arg and only take MAX_TICKERS
       amount of tickers to avoid API lockout */
    if (strcmp(argv[1], "-p") == 0) {
        loop_max = (argc - 2) < MAX_TICKERS ? argc - 2 : MAX_TICKERS;
        offset = POLL_OFFSET;
        poll_en = true;
    } else {
        loop_max = (argc - 1) < MAX_TICKERS ? argc - 1 : MAX_TICKERS;
        offset = NONPOLL_OFFSET;
    }


    do {
        printf("Requesting quote data...\n");
        
        /* create threads */
        for (i = 0; i < loop_max; i++) {
            quotes[i].symbol = argv[i+offset];
            quotes[i].api_key = API_KEY;
            quotes[i].api_len = API_LEN;
            quotes[i].quote_num = i;
            int error_create = pthread_create(&tid[i],
                                        NULL,
                                        getQuote,
                                        (void *)&quotes[i]);

            if (0 != error_create)
                fprintf(stderr, "Couldn't run quote %s\n", quotes[i].symbol);
            
        }

        /* join threads */
        for (i = 0; i < loop_max; i++) {
            error_join = pthread_join(tid[i], NULL);
            
            if (error_join != 0) {
                fprintf(stderr, "Failed to join thread %d", i);
            }
        }

        /* print quote data */
        for (i = 0; i < loop_max; i++) {
            printQuote(&quotes[i], i == 0);
        }

        /* if polling enabled wait SLEEP_TIME seconds to avoid API lockout */
        if (poll_en)
            sleep(SLEEP_TIME);

    } while(poll_en);

    curl_global_cleanup();

    return EXIT_SUCCESS;
}