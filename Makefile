CC=gcc
BINPATH=~/../../bin

all:
	$(CC) -o stonks main.c -lcurl -lpthread

install:
	sudo $(CC) -o $(BINPATH)/stonks main.c -lcurl -lpthread

clean:
	rm stonks