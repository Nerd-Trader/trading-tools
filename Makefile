PROG_NAME = ticker-scraper

CFLAGS = -std=c99 -s -pedantic -Wall -Wextra -Wfatal-errors -pedantic-errors -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L -O3
CC     = cc $(CFLAGS)

all: $(PROG_NAME)
.PHONY: all

prepare: bin
.PHONY: prepare

config: inc/config.h
.PHONY: config

$(PROG_NAME): bin/$(PROG_NAME)
.PHONY: $(PROG_NAME)

clean:
	@rm -rf bin
.PHONY: clean

bin:
	@mkdir -p bin

inc/config.h:
	@cp inc/config.h.def inc/config.h

bin/$(PROG_NAME): prepare config
	$(CC) \
                -I inc \
		src/curl.c \
		src/$(PROG_NAME).c \
		src/data-sources/finviz.c \
		src/data-sources/otcmarkets.c \
		-lcsv \
		-lcurl \
		-ltidy \
		-o bin/$(PROG_NAME)

run:
	@bin/$(PROG_NAME)
