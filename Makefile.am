AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = moba-display
moba_display_SOURCES = src/main.cpp src/msgloop.cpp
moba_display_LDADD = -lMoBaCommon -lssl -lcrypto -lboost_regex -lboost_system -lboost_filesystem
LDFLAGS += -L/usr/local/lib -L/usr/lib/i386-linux-gnu/ -pthread

AM_CPPFLAGS = -std=c++11
