#ABSTRACT: Simple interface for trading FXCM accounts
use strict;
use warnings;
package Finance::FXCM::Simple;

require XSLoader;
XSLoader::load(
    'ForexConnectWrapper',
    $Finance::FXCM::Simple::{VERSION} ? ${ $Finance::FXCM::Simple::{VERSION} } : ()
);

1;
