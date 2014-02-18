AM_CPPFLAGS = -I$(top_srcdir)/src -pedantic -Wall -Wextra
AM_CPPFLAGS += -std=c++03 -Dnullptr=NULL
LDADD = $(top_builddir)/src/libnmcrpc.la -lcurl -ljsoncpp

AM_CPPFLAGS += $(LIBIDN_CFLAGS)
LDADD += $(LIBIDN_LIBS)

noinst_PROGRAMS = registerName
bin_PROGRAMS = nmreg nmupdate

registerName_SOURCES = registerName.cpp
nmreg_SOURCES = nmreg.cpp
nmupdate_SOURCES = nmupdate.cpp
