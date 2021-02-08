#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "curl/curl.h"

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#define BWHT    "\e[1;37m"
#define CLEAR   "\e[1;1H\e[2J"

#define BASEURL     "https://finnhub.io/api/v1/"
#define BASEURL_LEN 27
#define MAX_TICKERS 5

typedef struct {
  char *ptr;
  size_t len;
} CURLdata;

typedef struct {
    float curr;
    float high;
    float low;
    float open;
} quote_res;

typedef char URL[256];

typedef struct {
    CURLSH *share;
    int quote_num;
    char *symbol;
    URL url;
    char *api_key;
    int api_len;
    quote_res q_data;
} quote_t;

static pthread_mutex_t lock_array[MAX_TICKERS];

/* initialize the struct needed to receive the GET request */
void init_CURLdata(CURLdata *data) {
  data->len = 0;
  data->ptr = malloc(data->len+1);
  if (data->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  data->ptr[0] = '\0';
}

// void init_quote_res(quote_res *quote_data) {
//     quote_data->curr = (char *)malloc(sizeof(char) * 1);
//     if (quote_data->curr == NULL) {
//         fprintf(stderr, "malloc() failed\n");
//         exit(EXIT_FAILURE);
//     }
//     quote_data->high = (char *)malloc(sizeof(char) * 1);
//     if (quote_data->high == NULL) {
//         fprintf(stderr, "malloc() failed\n");
//         exit(EXIT_FAILURE);
//     }
//     quote_data->low = (char *)malloc(sizeof(char) * 1);
//     if (quote_data->low == NULL) {
//         fprintf(stderr, "malloc() failed\n");
//         exit(EXIT_FAILURE);
//     }
//     quote_data->open = (char *)malloc(sizeof(char) * 1);
//     if (quote_data->open == NULL) {
//         fprintf(stderr, "malloc() failed\n");
//         exit(EXIT_FAILURE);
//     }
//     quote_data->curr[0] = '\0';
//     quote_data->open[0] = '\0';
//     quote_data->low[0] = '\0';
//     quote_data->open[0] = '\0';
// }

/* CURL provides format for write functions in their documentation */
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

    printf("build URL\n");
    strncat(quote->url, "quote?symbol=", 14);
    strncat(quote->url, quote->symbol, 10);
    strncat(quote->url, "&token=", 8);
    strncat(quote->url, quote->api_key, quote->api_len);
    printf("%s\n", quote->url);
}

/* use CURL library to get quote data and save in CURLdata struct */
void getQuoteData(URL *url, CURLdata *data) {
    CURL *curl = curl_easy_init();
    CURLcode res;

    curl_easy_setopt(curl, CURLOPT_URL, *url);
    printf("1\n");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    printf("2\n");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    printf("3\n");
    res = curl_easy_perform(curl);
    printf("get quote data\n");

    curl_easy_cleanup(curl);
}

/* API returns JSON formatted data. parseQuote splits on commas and
   deletes the leading chars to get just the price for each field */
void parseQuote(quote_res *quote, CURLdata *data) {
    char *curr, *high, *low, *open;
    printf("parse quote\n");
    printf("%s\n", data->ptr);
    curr = strtok(data->ptr, ",");
    high = strtok(NULL, ",");
    low = strtok(NULL, ",");
    open = strtok(NULL, ",");

    delchar(curr, 5);
    delchar(high, 4);
    delchar(low, 4);
    delchar(open, 4);
    printf("%s\n", curr);
    printf("%s\n", high);
    printf("%s\n", low);
    printf("%s\n", open);

    // memcpy(quote->curr, curr, strlen(curr));
    // memcpy(quote->high, high, strlen(high));
    // memcpy(quote->low, low, strlen(low));
    // memcpy(quote->open, open, strlen(open));
    // printf("after memcpy\n");
    quote->curr = atof(curr);
    quote->high = atof(high);
    quote->low = atof(low);
    quote->open = atof(open);
}

void printQuote(quote_t *quote) {
    static bool init = true;
    // static char prev[256] = "0";
    // size_t curr_len = strlen(quote->q_data.curr);
    // float open_f = atof(quote->q_data.open);
    // float curr_f = atof(quote->q_data.curr);
    float diff = quote->q_data.curr - quote->q_data.open;
    float percent = (diff / quote->q_data.open) * 100;

    if (init) {
        printf(CLEAR);
        init = false;
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

void getQuote(quote_t *quote) {
    // URL_params *query = (URL_params *)arg;
    
    CURLdata data;
    memcpy(quote->url, BASEURL, BASEURL_LEN);

    printf("get quote\n");
    init_CURLdata(&data);
    buildURL(quote);
    getQuoteData(&quote->url, &data);
    parseQuote(&quote->q_data, &data);
    // printQuote(quote);
    
    free(data.ptr);
}

static void *pull_one_url(void *arg) {
    getQuote((quote_t *)arg);
    return NULL;
}

static void lock_cb(CURL *handle, 
                    curl_lock_data data, 
                    curl_lock_access access, 
                    void *userptr) 
{
  pthread_mutex_lock(&lock_array[data]); /* uses a global lock array */
}

static void unlock_cb(CURL *handle, curl_lock_data data,void *userptr) {
  pthread_mutex_unlock(&lock_array[data]); /* uses a global lock array */
}

static void init_locks(int max) {
  int i;

  for(i = 0; i< MAX_TICKERS; i++)
    pthread_mutex_init(&lock_array[i], NULL);
}

static void kill_locks(int max) {
  int i;

  for(i = 0; i < max; i++)
    pthread_mutex_destroy(&(lock_array[i]));
}

int main(int argc, char **argv) {
    pthread_t tid[MAX_TICKERS];
    quote_t quotes[argc - 1];
    // CURL *curl = curl_easy_init();
    char *API_KEY = getenv("FINNHUB_API_KEY");
    int API_LEN = strlen(API_KEY);
    char *symbols[argc - 1];
    int i, loop_max, error;
    CURLSH *share;

    // URL_params tparams;

    // tparams.api_key = getenv("FINNHUB_API_KEY");
    // tparams.api_len = strlen(tparams.api_key);
    

    curl_global_init(CURL_GLOBAL_ALL);
    share = curl_share_init();
    curl_share_setopt(share, CURLSHOPT_LOCKFUNC, lock_cb);
    curl_share_setopt(share, CURLSHOPT_UNLOCKFUNC, unlock_cb);
    curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);



    /* check if user entered everything properly */
    if (argc < 2) {
        fprintf(stderr, "I need a stock symbol\n");
        return EXIT_FAILURE;
    } else if (!API_KEY) {
        fprintf(stderr, "Set your FINNHUB_API_KEY env var");
        return EXIT_FAILURE;
    } 
    
    /* make sure curl initializes properly */
    // if (!curl) {
    //     fprintf(stderr, "init failed\n");
    //     return EXIT_FAILURE;
    // }

    /* read in ticker symbols and get the quotes */
    // if (strcmp(argv[1], "-p") == 0) {
    //     loop_max = (argc - 2) < MAX_TICKERS ? argc - 2 : MAX_TICKERS;
        
    //     for (i = 0; i < loop_max; i++) {
    //         symbol[i] = argv[i+2];
    //     }

    //     while (1) {
    //         for (i = 0; i < loop_max; i++) {
    //             getQuote(curl, symbol[i], API_KEY, API_LEN, i == 0);
    //         }
    //         sleep(10);
    //     }
    // } else {
    //     loop_max = (argc - 1) < MAX_TICKERS ? argc - 1 : MAX_TICKERS;

    //     for (i = 0; i < loop_max; i++) {
    //         symbol[i] = argv[i+1];
    //         getQuote(curl, symbol[i], API_KEY, API_LEN, i== 0);
    //     }
    // }

    loop_max = (argc - 1) < MAX_TICKERS ? argc - 1 : MAX_TICKERS;

    init_locks(loop_max);

        for (i = 0; i < loop_max; i++) {
            symbols[i] = argv[i+1];
            quotes[i].symbol = argv[i+1];
            quotes[i].api_key = API_KEY;
            quotes[i].api_len = API_LEN;
            quotes[i].quote_num = i;
            // init_quote_res(&quotes[i].q_data);
            int error = pthread_create(&tid[i],
                                       NULL,
                                       pull_one_url,
                                       (void *)&quotes[i]);

            if (0 != error)
                fprintf(stderr, "Couldn't run thread number %d\n", i);
            else 
                fprintf(stderr, "Thread %d, gets %s\n", i, symbols[i]);
        }

        printf("after threads\n");

        for (i = 0; i < loop_max; i++) {
            pthread_join(tid[i], NULL);
            fprintf(stderr, "Thread %d terminated\n", i);
        }

        for (i = 0; i < loop_max; i++) {
            printQuote(&quotes[i]);
        }

        kill_locks(loop_max);

        curl_share_cleanup(share);

        curl_global_cleanup();

    // curl_easy_cleanup(curl);

    return EXIT_SUCCESS;
}