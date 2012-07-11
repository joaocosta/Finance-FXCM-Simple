use strict;
use warnings;

use Test::More tests => 5;
use Scalar::Util qw(looks_like_number);
use Data::Dumper;

BEGIN { use_ok('Finance::FXCM::Simple') };

eval {
    my $ff = Finance::FXCM::Simple->new("GBD118836001", "5358", "Demo", "http://www.fxcorporate.com/Hosts.jsp");
    ok(looks_like_number($ff->getAsk("EUR/USD")), "getAsk returns a number");
    ok(looks_like_number($ff->getBid("EUR/USD")), "getBid returns a number");
    $ff->openMarket("EUR/USD", "B", 5000);
    my $trades = $ff->getTrades();
    is(@$trades, 1, "1 trade opened");
    foreach my $trade(@$trades) {
        print Dumper(\$trade);
        $ff->closeMarket($trade->{id}, $trade->{size});
    }
    $trades = $ff->getTrades();
    is(@$trades, 0, "All trades closed");

};

if ($@) {
    print "Error: " . $@;
}
