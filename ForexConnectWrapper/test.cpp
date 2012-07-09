#include "ForexConnectWrapper.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    const char *user = "GBD118836001";
    const char *password = "5358";
    const char *url = "http://www.fxcorporate.com/Hosts.jsp";
    const char *connection = "Demo";
    ForexConnectWrapper *tradeStation = NULL;

    try {
        tradeStation = new ForexConnectWrapper(user, password, connection, url);

        tradeStation->closeMarket("12895305", 3000);
        tradeStation->openMarket("EUR/GBP", "B", 3000);

        std::string trades = tradeStation->getTrades();
        printf("%s\n", trades.c_str());
    }

    catch(std::string error) {
        printf("%s\n", error.c_str());
    }

    if (tradeStation) {
        delete tradeStation;
    }
}
