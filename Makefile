CC=gcc

stonks: main.c
	$(CC) -o stonks main.c -lcurl -lpthread