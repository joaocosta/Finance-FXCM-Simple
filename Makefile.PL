use ExtUtils::MakeMaker;

##{ $share_dir_code{preamble} || '' ##}

use Config;

WriteMakefile(
    NAME                => 'Finance::FXCM::Simple',
    VERSION_FROM        => 'lib/Finance/FXCM/Simple.pm',
    LIBS                => ["-lForexConnect -lfxmsg"],
    DEFINE              => '',
    INC                 => "-I.",
    OBJECT              => '$(O_FILES)', # link all the C files too
    'XSOPT'             => '-C++',
    'TYPEMAPS'          => ['perlobject.map'],
    'CC'                => 'g++',
    'LD'                => '$(CC)',
    'MYEXTLIB'          => 'ForexConnectWrapper/libForexConnectWrapper$(LIB_EXT)',
    'BUILD_REQUIRES'    => {
        'Test::More'    =>  0,
        'Scalar::Util'  =>  0,
    },
    'PREREQ_PM'         => {
        'YAML::Syck'    => 0,
    },
    LICENSE             => 'mit',
    ( $] >= 5.005 ? 
        (   ABSTRACT_FROM       => 'lib/Finance/FXCM/Simple.pm',
            AUTHOR              => 'João Costa <joaocosta@zonalivre.org>',
        ) : ()
    ),
##{ $plugin->get_prereqs ##}
);

sub MY::postamble {
    '
    $(MYEXTLIB): ForexConnectWrapper/Makefile
		cd ForexConnectWrapper && $(MAKE) $(PASSTHRU)
    ';
}

##{ $share_dir_code{postamble} || '' ##}
