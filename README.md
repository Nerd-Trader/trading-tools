# ticker-scraper

CLI tool for scraping stock market ticker data.

Currently supported markets:
 - NYSE, NASDAQ (via [finviz.com](https://finviz.com/))
 - OTCQX, OTCQB, Pink sheets (via [otcmarkets.com](https://otcmarkets.com/))


## How to build

    make -j


## How to run

    bin/ticker-scraper > us-stocks.csv

or

    bin/ticker-scraper NYSE NASDAQ OTCQX OTCQX Pink > us-and-otc-stocks.csv

or

    bin/ticker-scraper US > us-stocks.csv

or

    bin/ticker-scraper OTC > otc-stocks.csv
