CC = gcc
CFLAGS = -Wall -pedantic -std=gnu99

.PHONY: all clean

all: test/sort test/calc

test/%: test/%.c
	@$(CC) $(CFLAGS) -o runtest-$(notdir $@) $^
	@echo "+ running $^"
	@./runtest-$(notdir $@) $(ARGS)
	@rm runtest-$(notdir $@)

clean:
	rm -rf runtest-*
