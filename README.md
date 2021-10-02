# trading-tools

A whole bunch of programs, for fun and profit.

Some tools require access to gatewayed APIs. Credentials should be added to `inc/config.h` after running `make config` (and inspecting the source code of this whole codebase, of course).


------------------------------------------------------------------------------

## Tool 1: ticker-scraper

This CLI tool is capable of scraping general stock market ticker data from various web resources.  It generates CSV files that can later be used by other tools in the suite.


### Currently supported markets:
 - `NYSE`, `NASDAQ` (via [finviz.com](https://finviz.com))
 - `OTCQB`, `OTCQX`, `Pink sheets` (via [otcmarkets.com](https://otcmarkets.com))


### Dependencies

`libcsv`, `libcurl`, `libtidy`


### How to build

```console
make -j ticker-scraper
```


### How to run

```console
bin/ticker-scraper NASDAQ > nasdaq-and-pink.csv
bin/ticker-scraper --no-csv-header Pink >> nasdaq-and-pink.csv
```

or

```console
bin/ticker-scraper US OTC > us-and-otc-stocks.csv
```


### Sample output

```
"marketplace","ticker","company","price","sector","industry","country","marketcap"
"NASDAQ","A","Agilent Technologies, Inc.","173.94","Healthcare","Diagnostics & Research","USA","52.46B"
"NASDAQ","AA","Alcoa Corporation","48.83","Basic Materials","Aluminum","USA","9.23B"
"NASDAQ","AAAU","Goldman Sachs Physical Gold ETF","17.42","Financial","Exchange Traded Fund","USA","-"
"NASDAQ","AAC","Ares Acquisition Corporation","9.74","Financial","Shell Companies","USA","1.22B"
```

------------------------------------------------------------------------------
