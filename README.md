# Stonks CLI

## Description

With all the recent buzz on the stock market, I was getting tired of constantly checking specific stocks online. This inspired me to create this little commandline tool to get stock quotes instantly. All you need is an API key to the [Finnhub](https://finnhub.io/) financial API and you are on your way! I set the executable in the `PATH` environment variable so that wherever I am I can type `stonks <ticker>` and get a real time quote for where the price is at. This has only been tested on Linux, but should work on Mac. Feel free to take this and make it your own!

![](./stonks_CLI.gif)

## Installation

- Clone repo

- Navigate to base directory

- Run `make`

- Set `FINNHUB_API_KEY` environment variable to your Finnhub API key

- Run `stonks <ticker>` where the ticker is any valid stock ticker symbol

## Dependancies

- CURL C library
- C compiler (set in *Makefile*)
- [Finnhub](https://finnhub.io/) API key (free for developers)

## License

Copyright 2021 Rance Campbell

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.