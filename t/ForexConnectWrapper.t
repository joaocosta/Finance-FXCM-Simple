use strict;
use warnings;

use Test::More tests => 1;
BEGIN { use_ok('Finance::FXCM::Simple') };

my $ff = Finance::FXCM::Simple->new("GBD118836001", "5358", "Demo", "http://www.fxcorporate.com/Hosts.jsp");
#print $ff->getAsk("EUR/USD"), "\n";
#$ff->saveHistoricalDataToFile("/tmp/test.tmp", "EUR/USD", "m5", 500);

