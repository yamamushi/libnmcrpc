AM_CPPFLAGS = -I$(top_srcdir)/src -std=c++11 -pedantic -Wall -Wextra
LDADD = $(top_builddir)/src/libnmcrpc.la -lcurl -ljsoncpp

AM_CPPFLAGS += $(LIBIDN_CFLAGS)
LDADD += $(LIBIDN_LIBS)

noinst_PROGRAMS = registerName
bin_PROGRAMS = nmreg nmupdate

registerName_SOURCES = registerName.cpp
nmreg_SOURCES = nmreg.cpp
nmupdate_SOURCES = nmupdate.cpp
