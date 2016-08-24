CC = gcc
CFLAGS = -Wall -pedantic -std=gnu99

.PHONY: all clean

all: test/hello

test/%: src/%.c test/%.c
	$(CC) $(CFLAGS) $^ -o runtest
	./runtest $(ARGS)


clean:
	rm -rf runtest

