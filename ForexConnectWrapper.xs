#include "ForexConnectWrapper/ForexConnectWrapper.h"


#ifdef __cplusplus
extern "C" {
#endif


#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#ifdef __cplusplus
}
#endif



MODULE = Finance::FXCM::Simple		PACKAGE = Finance::FXCM::Simple		

ForexConnectWrapper *
ForexConnectWrapper::new(const char *user, const char *password, const char *accountType, const char *url);

void
ForexConnectWrapper::DESTROY()

double
ForexConnectWrapper::getAsk(const char *symbol)

double
ForexConnectWrapper::getBid(const char *symbol)

void
ForexConnectWrapper::openMarket(const char *symbol, const char *direction, int amount)

void
ForexConnectWrapper::closeMarket(const char *tradeID, int amount)

const char *
ForexConnectWrapper::getTrades()

double
ForexConnectWrapper::getNav()

int
ForexConnectWrapper::getBaseUnitSize(const char *symbol)

void
ForexConnectWrapper::saveHistoricalDataToFile(const char *filename, const char *symbol, const char * tf, int totalItemsToDownload)
