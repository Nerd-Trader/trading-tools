# ticker-scraper

CLI tool for scraping stock market ticker data

#### Currently supported markets:
 - `NYSE`, `NASDAQ` (via [finviz.com](https://finviz.com))
 - `OTCQB`, `OTCQX`, `Pink sheets` (via [otcmarkets.com](https://otcmarkets.com))


## How to build

```console
make -j
```


## How to run

```console
bin/ticker-scraper --no-csv-header > us-stocks.csv
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
