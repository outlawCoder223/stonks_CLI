CC=gcc
BINPATH=~/../../bin/stonks

all:
	$(CC) -o stonks main.c -lcurl -lpthread

install:
	$(CC) -o $(BINPATH) main.c -lcurl -lpthread
	rm test.txt stonks_CLI.gif .git .gitignore .vscode main.c README.md

clean:
	rm stonks