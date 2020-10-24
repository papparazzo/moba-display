bin_PROGRAMS = moba-display
moba_display_SOURCES = \
    main.cpp \
    msgloop.cpp \
    webserver.cpp

moba_display_LDADD = \
    -lmobacommon -lssl -lcrypto -lboost_regex -lboost_system -lboost_filesystem \
    $(srcdir)/../modules/lib-msghandling/src/moba/libmsghandling.a

LDFLAGS += -L/usr/local/lib -pthread -L/usr/lib/i386-linux-gnu/ -pthread

AM_CPPFLAGS = -std=c++11 -I$(srcdir)/../modules/lib-msghandling/src -g3 -ggdb -O3 -Wall

ACLOCAL_AMFLAGS = -I m4
