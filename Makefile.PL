use ExtUtils::MakeMaker;

##{ $share_dir_code{preamble} || '' ##}

die("
    Please set environment variable FXCONNECT_HOME to point at directory
    location of the FXCONNECT library.  The library can be downloaded from:
    http://forexforums.dailyfx.com/forexconnect/392705-forexconnect-api-subscribe-updates.html")
    if (!$ENV{FXCONNECT_HOME});

WriteMakefile(
    NAME                => 'Finance::FXCM::Simple',
    VERSION_FROM        => 'lib/Finance/FXCM/Simple.pm',
    LIBS                => ["-L$ENV{FXCONNECT_HOME}/lib -lForexConnect -lfxmsg"],
    DEFINE              => '',
    INC                 => "-I. -I$ENV{FXCONNECT_HOME}/include",
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
    ( $] >= 5.005 ? 
        (   ABSTRACT_FROM       => 'lib/Finance/FXCM/Simple.pm',
            AUTHOR              => 'Joao Costa <joaocosta@zonalivre.org>',
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
