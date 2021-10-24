TOOL_NAME_1 = ticker-scraper
TOOL_NAME_2 = historical-data-scraper
TOOL_NAME_3 = chart-generator

CFLAGS = -std=c99 -s -pedantic -Wall -Wextra -Wfatal-errors -pedantic-errors -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L -O3
CC     = cc $(CFLAGS)

all: $(TOOL_NAME_1) $(TOOL_NAME_2) $(TOOL_NAME_3)
.PHONY: all

bin:
	@mkdir -p bin

$(TOOL_NAME_1): bin/$(TOOL_NAME_1)
.PHONY: $(TOOL_NAME_1)

$(TOOL_NAME_2): bin/$(TOOL_NAME_2)
.PHONY: $(TOOL_NAME_2)

$(TOOL_NAME_3): bin/$(TOOL_NAME_3)
.PHONY: $(TOOL_NAME_3)

bin/$(TOOL_NAME_1): bin config
	$(CC) \
        -DPROG="\"$(TOOL_NAME_1)\"" \
        -I inc \
        -I inc/$(TOOL_NAME_1) \
        src/common.c \
        src/curl.c \
        src/$(TOOL_NAME_1)/$(TOOL_NAME_1).c \
        src/$(TOOL_NAME_1)/data-sources/finviz.c \
        src/$(TOOL_NAME_1)/data-sources/otcmarkets.c \
        -lcsv \
        -lcurl \
        -ltidy \
        -o bin/$(TOOL_NAME_1)

bin/$(TOOL_NAME_2): bin config
	$(CC) \
        -DPROG="\"$(TOOL_NAME_2)\"" \
        -I inc \
        -I inc/$(TOOL_NAME_2) \
        src/common.c \
        src/curl.c \
        src/$(TOOL_NAME_2)/$(TOOL_NAME_2).c \
        src/$(TOOL_NAME_2)/data-sources/tdameritrade.c \
        -lcsv \
        -lcurl \
        -ljson-c \
        -o bin/$(TOOL_NAME_2)

bin/$(TOOL_NAME_3): bin config
	$(CC) \
        -DPROG="\"$(TOOL_NAME_3)\"" \
        -I inc \
        -I inc/$(TOOL_NAME_3) \
        src/common.c \
        src/$(TOOL_NAME_3)/$(TOOL_NAME_3).c \
        `pkg-config --cflags --libs cairo` \
        -ljson-c \
        -o bin/$(TOOL_NAME_3)

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
	@bin/$(TOOL_NAME_1) NASDAQ OTC > data/tickers.csv
.PHONY: run-$(TOOL_NAME_1)

run-$(TOOL_NAME_2): data
	@bin/$(TOOL_NAME_2) -P 5 --include-missing-price -M 0.5B --include-missing-market-cap -o data/historical/ data/tickers.csv
.PHONY: run-$(TOOL_NAME_2)

run-$(TOOL_NAME_3): data
	@bin/$(TOOL_NAME_3) -o data/charts/ data/historical/*
.PHONY: run-$(TOOL_NAME_3)

run: run-$(TOOL_NAME_1) run-$(TOOL_NAME_2) run-$(TOOL_NAME_3)
.PHONY: run
