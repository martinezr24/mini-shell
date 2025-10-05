CC=gcc
CFLAGS=-g -Wall -Werror
INCLUDE=./include

all: ./*.c
	$(CC) $(CFLAGS) ./*.c -I$(INCLUDE) -o ./bin/shell
	./bin/shell

run:
	./bin/shell


# Removes the binary files automatically
clean:
	rm ./bin/shell
