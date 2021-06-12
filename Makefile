# Makefile for ticker-scraper

CFLAGS = -std=c99 -s -pedantic -Wall -Wextra -Wfatal-errors -pedantic-errors -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L -O3
CC     = gcc $(CFLAGS)

all: ticker-scraper
.PHONY: all

prepare: bin
.PHONY: prepare

config: src/config.h
.PHONY: config

ticker-scraper: bin/ticker-scraper
.PHONY: ticker-scraper

clean:
	@rm -rf bin
.PHONY: clean

bin:
	@mkdir -p bin

src/config.h:
	@cp src/config.h.def src/config.h

bin/ticker-scraper: prepare config
	$(CC) src/curl.c src/ticker-scraper.c src/resources/finviz.c -o bin/ticker-scraper -lcurl

run:
	@bin/ticker-scraper
