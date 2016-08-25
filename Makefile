CC = gcc
CFLAGS = -Wall -pedantic -std=gnu99

.PHONY: all clean

all: test/calc

test/%: test/%.c src/cafe.c
	$(CC) $(CFLAGS) -o runtest $^
	./runtest $(ARGS)


clean:
	rm -rf runtest

