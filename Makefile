CC=gcc

stocks: main.c
	$(CC) -o stonks main.c -lcurl