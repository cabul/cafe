CC = gcc
CFLAGS = -Wall -pedantic -std=gnu99

.PHONY: all clean

all:

build/%: src/%.c test/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -Itest -Isrc -o $@ $^

test/%: build/% test/cafe.h
	@$<


clean:
	rm -rf build

