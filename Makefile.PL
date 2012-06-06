use ExtUtils::MakeMaker;

##{ $share_dir_code{preamble} || '' ##}

my $ForexConnectHome = "ForexConnectWrapper/ForexConnectAPI-1.1.3-Linux-x86_64";
WriteMakefile(
    NAME                => 'Finance::FXCM::Simple',
    LIBS                => ["-L$ForexConnectHome/lib -lForexConnect -lfxmsg"],
    DEFINE              => '',
    INC                 => "-I. -I$ForexConnectHome/include",
    OBJECT              => '$(O_FILES)', # link all the C files too
    'XSOPT'             => '-C++',
    'TYPEMAPS'          => ['perlobject.map'],
    'CC'                => 'g++',
    'LD'                => '$(CC)',
    'MYEXTLIB'          => 'ForexConnectWrapper/libForexConnectWrapper$(LIB_EXT)',
##{ $plugin->get_prereqs ##}
);

sub MY::postamble {
    '
    $(MYEXTLIB): ForexConnectWrapper/Makefile
		cd ForexConnectWrapper && $(MAKE) $(PASSTHRU)
    ';
}

##{ $share_dir_code{postamble} || '' ##}
