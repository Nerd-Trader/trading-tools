PROG_NAME = ticker-scraper

CFLAGS = -std=c99 -s -pedantic -Wall -Wextra -Wfatal-errors -pedantic-errors -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L # -O3
CC     = gcc $(CFLAGS)

all: $(PROG_NAME)
.PHONY: all

prepare: bin
.PHONY: prepare

config: src/config.h
.PHONY: config

$(PROG_NAME): bin/$(PROG_NAME)
.PHONY: $(PROG_NAME)

clean:
	@rm -rf bin
.PHONY: clean

bin:
	@mkdir -p bin

src/config.h:
	@cp src/config.h.def src/config.h

bin/$(PROG_NAME): prepare config
	$(CC) src/curl.c src/$(PROG_NAME).c src/resources/finviz.c -o bin/$(PROG_NAME) -lcurl

run:
	@bin/$(PROG_NAME)
