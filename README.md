# ticker-scraper

CLI tool for scraping stock market ticker data.

Currently supported markets:
 - NYSE, NASDAQ (via [finviz.com](https://finviz.com/))
 - OTCQX, OTCQB, Pink sheets (via [otcmarkets.com](https://otcmarkets.com/))


## How to build

    make -j


## How to run

    bin/ticker-scraper > /tmp/tickers.csv

or

    bin/ticker-scraper NYSE NASDAQ OTCQX OTCQX Pink > /tmp/all-tickers.csv
