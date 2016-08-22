CC = gcc
CFLAGS = -Wall -pedantic -std=c11

.PHONY: all clean

all:

test: test.c cafe.h
	$(CC) $(CFLAGS) $< -o $@

build/%.o: src/%.c
	@echo "+ compiling $<"
	@mkdir -p $(@D)
	$(CC) $(CLFAGS) -c $< -o $@

clean:
	rm -rf build

