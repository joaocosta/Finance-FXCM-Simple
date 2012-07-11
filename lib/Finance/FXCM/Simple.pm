#ABSTRACT: Simple interface for trading FXCM accounts
use strict;
use warnings;
package Finance::FXCM::Simple;

require XSLoader;
XSLoader::load(
    'Finance::FXCM::Simple',
    $Finance::FXCM::Simple::{VERSION} ? ${ $Finance::FXCM::Simple::{VERSION} } : ()
);

use YAML::Syck;

sub getTrades {
    my ($self) = @_;
    my $trades = YAML::Syck::Load($self->getTradesAsYAML());
    return $trades;
}
1;
