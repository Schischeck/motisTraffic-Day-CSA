Compile Boost under Windows:

    b2 --with-test --with-random --with-program_options --with-date_time --with-chrono --with-system --with-thread --with-regex --with-iostreams --with-filesystem address-model=64  variant=release link=static threading=multi -s ZLIB_SOURCE=C:\zlib-1.2.8

Dependencies:

    boost, zlib, openssl