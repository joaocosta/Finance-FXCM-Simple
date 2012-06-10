use ExtUtils::MakeMaker;

##{ $share_dir_code{preamble} || '' ##}

WriteMakefile(
    NAME                => 'Finance::FXCM::Simple',
    LIBS                => ["-L$ENV{FXCONNECT_HOME}/lib -lForexConnect -lfxmsg"],
    DEFINE              => '',
    INC                 => "-I. -I$ENV{FXCONNECT_HOME}/include",
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
