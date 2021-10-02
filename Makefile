TOOL_NAME_1 = ticker-scraper

CFLAGS = -std=c99 -s -pedantic -Wall -Wextra -Wfatal-errors -pedantic-errors -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L -O3
CC     = cc $(CFLAGS)

all: $(TOOL_NAME_1)
.PHONY: all

bin:
	@mkdir -p bin

$(TOOL_NAME_1): bin/$(TOOL_NAME_1)
.PHONY: $(TOOL_NAME_1)

bin/$(TOOL_NAME_1): bin config
	$(CC) \
        -I inc \
        -I inc/$(TOOL_NAME_1) \
        src/curl.c \
        src/$(TOOL_NAME_1)/$(TOOL_NAME_1).c \
        src/$(TOOL_NAME_1)/data-sources/finviz.c \
        src/$(TOOL_NAME_1)/data-sources/otcmarkets.c \
        -lcsv \
        -lcurl \
        -ltidy \
        -o bin/$(TOOL_NAME_1)

clean:
	@rm -rf bin
.PHONY: clean

config: inc/config.h
.PHONY: config

data:
	@mkdir -p data

inc/config.h:
	@cp inc/config.h.def inc/config.h

run-$(TOOL_NAME_1): data
	bin/$(TOOL_NAME_1) US OTC > data/tickers.csv
.PHONY: run-$(TOOL_NAME_1)

run: run-$(TOOL_NAME_1)
.PHONY: run
