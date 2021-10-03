# trading-tools

Set of CLI tools for working with stock market data.

Some tools require access to gatewayed APIs. Credentials should be added to `inc/config.h` after running `make config` (and inspecting the source code of this whole codebase, of course).


------------------------------------------------------------------------------


# Tool 1: ticker-scraper

This CLI tool is capable of scraping general stock market ticker data from various web resources.  It generates CSV files that can later be used by other tools in the suite.


## Currently supported markets:

 - `NYSE`, `NASDAQ` (via [finviz.com](https://finviz.com))
 - `OTCQB`, `OTCQX`, `Pink sheets` (via [otcmarkets.com](https://otcmarkets.com))


## Dependencies

`libcsv`, `libcurl`, `libtidy`


## How to build

```console
make -j ticker-scraper
```


## How to run

```console
bin/ticker-scraper NASDAQ > nasdaq-and-pink.csv
bin/ticker-scraper --no-csv-header Pink >> nasdaq-and-pink.csv
```
or

```console
bin/ticker-scraper US OTC > us-and-otc-stocks.csv
```


## Sample output

```
"marketplace","ticker","company","price","sector","industry","country","marketcap"
"NASDAQ","A","Agilent Technologies, Inc.","173.94","Healthcare","Diagnostics & Research","USA","52.46B"
"NASDAQ","AA","Alcoa Corporation","48.83","Basic Materials","Aluminum","USA","9.23B"
"NASDAQ","AAAU","Goldman Sachs Physical Gold ETF","17.42","Financial","Exchange Traded Fund","USA","-"
"NASDAQ","AAC","Ares Acquisition Corporation","9.74","Financial","Shell Companies","USA","1.22B"
```


------------------------------------------------------------------------------


# Tool 2: historical-data-scraper

Historical stock market data scraper.  Obtains OHLC records and saves them to disk as JSON files.

The input is CSV files in the same format as the one that gets produced by ticker-scraper (Tool 1).


## Currently supported data sources:

 - TD Ameritrade [developer.tdameritrade.com](https://developer.tdameritrade.com)


## Dependencies

`libcsv`, `libcurl`


## How to build

```console
make config
```
put your TD Ameritrade API key into `inc/config.h`

```console
make -j historical-data-scraper
```


## How to run

```console
bin/historical-data-scraper --min-price=0.1 --max-price=7.5 -o data/historical/ input.csv
```


## Sample output

```json

```


------------------------------------------------------------------------------


# Tool 3: chart-generator

CLI tool for creating chart images out of historical stock market data generated by historical-data-scraper (Tool 2).


## Dependencies

`libcairo2`, `libjson-c`


## How to build

```console
make -j chart-generator
```


## How to use

```console
bin/chart-generator -o data/charts data/historical/*
```
