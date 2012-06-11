#include "ForexConnectWrapper.h"
#include <string>
#include <iostream>
#include <fstream>
#include "ForexConnect.h"
#include "Listener.h"
#include "Session.h"

ForexConnectWrapper::ForexConnectWrapper(const std::string user, const std::string password, const std::string accountType, const std::string url) {

    session = CO2GTransport::createSession();
    listener = new Session(session);

    if (!listener->loginAndWait(user, password, url, accountType)) {
        throw "Failed to login";
    }

    connected = true;

    loginRules = session->getLoginRules();
    if (!loginRules->isTableLoadedByDefault(::Accounts)) {
        throw "Accounts table not loaded";
    }

    IO2GResponse *response = loginRules->getTableRefreshResponse(::Accounts);
    if(!response) {
        throw "No response to refresh accounts table request";
    }

    mResponseReaderFactory = session->getResponseReaderFactory();
    IO2GAccountsTableResponseReader *accountsTableResponseReader = mResponseReaderFactory->createAccountsTableReader(response);
    response->release();
    accountRow = accountsTableResponseReader->getRow(0);
    accountsTableResponseReader->release();
    sAccountID = accountRow->getAccountID();
    mRequestFactory = session->getRequestFactory();
}

ForexConnectWrapper::~ForexConnectWrapper() {
    mRequestFactory->release();
    accountRow->release();
    mResponseReaderFactory->release();

    listener->logoutAndWait();

    listener->release();
    session->release();
}

void ForexConnectWrapper::openMarket(const std::string symbol, const std::string direction, int amount) {
    if (direction != O2G2::Sell && direction != O2G2::Buy) {
        throw "Direction must be 'B' or 'S'";
    }

    IO2GOfferRow *offer = getTableRow<IO2GOfferRow, IO2GOffersTableResponseReader>(Offers, symbol, &findOfferRowBySymbol, &getOffersReader);
    std::string sOfferID = offer->getOfferID();
    offer->release();

    IO2GValueMap *valuemap = mRequestFactory->createValueMap();
    valuemap->setString(Command, O2G2::Commands::CreateOrder);
    valuemap->setString(OrderType, O2G2::Orders::TrueMarketOpen);
    valuemap->setString(AccountID, sAccountID.c_str());
    valuemap->setString(OfferID, sOfferID.c_str());
    valuemap->setString(BuySell, direction.c_str());
    valuemap->setInt(Amount, amount);

    IO2GRequest *orderRequest = mRequestFactory->createOrderRequest(valuemap);
    valuemap->release();


    Listener *ll = new Listener(session);
    ll->sendRequest(orderRequest);
    orderRequest->release();
    ll->release();
}

double ForexConnectWrapper::getBid(const std::string symbol) {
    IO2GOfferRow *offer = getTableRow<IO2GOfferRow, IO2GOffersTableResponseReader>(Offers, symbol, &findOfferRowBySymbol, &getOffersReader);
    double rv = offer->getBid();
    offer->release();

    return rv;
}

double ForexConnectWrapper::getAsk(const std::string symbol) {
    IO2GOfferRow *offer = getTableRow<IO2GOfferRow, IO2GOffersTableResponseReader>(Offers, symbol, &findOfferRowBySymbol, &getOffersReader);
    double rv = offer->getAsk();
    offer->release();

    return rv;
}

void ForexConnectWrapper::closeMarket(const std::string tradeID, int amount) {
    IO2GTradeRow *trade = getTableRow<IO2GTradeRow, IO2GTradesTableResponseReader>(Trades, tradeID, &findTradeRowByTradeId, &getTradesReader);

    IO2GValueMap *valuemap = mRequestFactory->createValueMap();
    valuemap->setString(Command, O2G2::Commands::CreateOrder);
    valuemap->setString(OrderType, O2G2::Orders::TrueMarketClose);
    valuemap->setString(AccountID, sAccountID.c_str());
    valuemap->setString(OfferID, trade->getOfferID());
    valuemap->setString(TradeID, tradeID.c_str());
    valuemap->setString(BuySell, ( strncmp(trade->getBuySell(), "B", 1) == 0 ? O2G2::Sell : O2G2::Buy ) );
    trade->release();
    valuemap->setInt(Amount, amount);
    //valuemap->setString(CustomID, "Custom");

    IO2GRequest *request = mRequestFactory->createOrderRequest(valuemap);
    valuemap->release();

    Listener *ll = new Listener(session);
    ll->sendRequest(request);
    request->release();
    ll->release();
}

std::string ForexConnectWrapper::getTrades() {
    std::string rv;
    IO2GRequest *request = mRequestFactory->createRefreshTableRequestByAccount(Trades, sAccountID.c_str());

    Listener *ll = new Listener(session);
    IO2GResponse *response = ll->sendRequest(request);
    request->release();
    ll->release();
    if (!response) {
        throw "No response to Trades table refresh request";
    }

    IO2GTradesTableResponseReader *reader = mResponseReaderFactory->createTradesTableReader(response);
    response->release();

    IO2GTradeRow *row = NULL;
    for ( int i=0; i < reader->size(); ++i ) {
        row = reader->getRow(i);
        IO2GOfferRow *offer = getTableRow<IO2GOfferRow, IO2GOffersTableResponseReader>(Offers, row->getOfferID(), &findOfferRowByOfferId, &getOffersReader);
        bool isLong = (strncmp(row->getBuySell(), "B", 1) == 0);
        double baseCurrencyPL = (isLong ? offer->getBid() - row->getOpenRate() : row->getOpenRate() - offer->getAsk()) * row->getAmount();
        std::string baseCurrency = offer->getInstrument();
        baseCurrency = baseCurrency.substr(4, -1);

        if (baseCurrency != "GBP") {
            std::string s = "GBP/";
            s.append(baseCurrency);
            double conversionFactor = getAsk(s);
            baseCurrencyPL = baseCurrencyPL / conversionFactor;
        }

        double d = row->getOpenTime();
        std::string openDateTime;
        formatDate(d, openDateTime);

        rv.append("- symbol: ").append(offer->getInstrument()).append("\n");
        rv.append("  id: ").append(row->getTradeID()).append("\n");
        rv.append("  direction: ").append(isLong ? "long" : "short").append("\n");
        rv.append("  openPrice: ").append(double2str(row->getOpenRate())).append("\n");
        rv.append("  size: ").append(int2str(row->getAmount())).append("\n");
        rv.append("  openDate: ").append(openDateTime).append("\n");
        rv.append("  pl: ").append(double2str(baseCurrencyPL)).append("\n");

        offer->release();
        row->release();
    }
    reader->release();

    return rv;
}

double ForexConnectWrapper::getBalance() {
    return accountRow->getBalance();
}

int ForexConnectWrapper::getBaseUnitSize(const std::string symbol) {
    IO2GTradingSettingsProvider *tradingSettingsProvider = loginRules->getTradingSettingsProvider();

    int rv = tradingSettingsProvider->getBaseUnitSize(symbol.c_str(), accountRow);
    tradingSettingsProvider->release();
    return rv;
}


template <class RowType, class ReaderType>
RowType* ForexConnectWrapper::getTableRow(O2GTable table, std::string key, bool (*finderFunc)(RowType *, std::string), ReaderType* (*readerCreateFunc)(IO2GResponseReaderFactory* , IO2GResponse *)) {

    IO2GResponse *response;

    if( !loginRules->isTableLoadedByDefault(table) ) {
        IO2GRequest *request = mRequestFactory->createRefreshTableRequestByAccount(Trades, sAccountID.c_str());

        Listener *ll = new Listener(session);
        response = ll->sendRequest(request);
        request->release();
        ll->release();
        if (!response) {
            throw "No response to manual table refresh request";
        }
    } else {
        response = loginRules->getTableRefreshResponse(table);
        if (!response) {
            throw "No response to automatic table refresh request";
        }
    }

    ReaderType *reader = readerCreateFunc(mResponseReaderFactory, response);
    response->release();

    RowType *row = NULL;

    for ( int i = 0; i < reader->size(); ++i ) {
        row = reader->getRow(i);
        if ( finderFunc(row, key) ) {
                break;
        }
        row->release();
        row = NULL;
    }
    reader->release();

    if (row == NULL) {
        throw "Could not find row";
    }

    return row;
}

void ForexConnectWrapper::saveHistoricalDataToFile(const std::string filename, const std::string symbol, const std::string tf, int totalItemsToDownload) {
        IO2GTimeframeCollection *timeframeCollection = mRequestFactory->getTimeFrameCollection();
        IO2GTimeframe *timeframeObject = NULL;
        int timeframeCollectionSize = timeframeCollection->size();

        for (int i = 0; i < timeframeCollectionSize; i++) {
            IO2GTimeframe *searchTimeframeObject = timeframeCollection->get(i);
            if ( tf == searchTimeframeObject->getID()) {
                timeframeObject = searchTimeframeObject;
                break;
            } else {
                searchTimeframeObject->release();
            }
        }
        timeframeCollection->release();

        if (!timeframeObject) {
            throw "Could not find timeframe";
        }


        double timeFrom = 0, timeTo = 0;

        time_t rawtime = time(NULL);
        struct tm timeinfo = *localtime(&rawtime);
        timeinfo.tm_year = 50;
        timeinfo.tm_mon = 0;
        timeinfo.tm_mday = 1;
        timeinfo.tm_hour = 0;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;

        CO2GDateUtils::UnixTimeToVariantTime(&timeinfo, &timeFrom);
        std::string dd;
        formatDate(timeFrom, dd);

        std::ofstream outputFile;
        outputFile.open(filename.c_str());
        Listener *ll = new Listener(session);
        while (totalItemsToDownload > 0) {
            IO2GRequest *marketDataRequest = mRequestFactory->createMarketDataSnapshotRequestInstrument(symbol.c_str(), timeframeObject, totalItemsToDownload > 300 ? 300 : totalItemsToDownload);
            mRequestFactory->fillMarketDataSnapshotRequestTime(marketDataRequest, timeFrom, timeTo, false);
            IO2GResponse *marketDataResponse = ll->sendRequest(marketDataRequest);

            IO2GMarketDataSnapshotResponseReader *marketSnapshotReader = mResponseReaderFactory->createMarketDataSnapshotReader(marketDataResponse);
            int snapshotSize = marketSnapshotReader->size();
            for (int i = snapshotSize - 1; i >= 0; i--) {
                double date = marketSnapshotReader->getDate(i);
                std::string dateTimeStr;
                formatDate(date, dateTimeStr);
                outputFile << dateTimeStr << "\t" << double2str(marketSnapshotReader->getAskOpen(i)) << "\t" << double2str(marketSnapshotReader->getAskHigh(i)) << "\t" << double2str(marketSnapshotReader->getAskLow(i)) << "\t" << double2str(marketSnapshotReader->getAskClose(i)) << "\t" << std::endl;

            }

            totalItemsToDownload -= snapshotSize;
            if (totalItemsToDownload > 0) {
                timeTo = marketSnapshotReader->getDate(0);
            }
            marketSnapshotReader->release();
        }
        ll->release();
        outputFile.close();
}
