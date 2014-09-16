AM_CPPFLAGS = -I$(top_srcdir)/src -pedantic -Wall -Wextra
AM_CPPFLAGS += -std=c++03 -Dnullptr=NULL
LDADD = $(top_builddir)/src/libnmcrpc.la -lcurl -ljsoncpp

AM_CPPFLAGS += $(LIBIDN_CFLAGS)
LDADD += $(LIBIDN_LIBS)

check_PROGRAMS = jsonrpc basicInfo messageSigning idn
TESTS = $(check_PROGRAMS)

basicInfo_SOURCES = basicInfo.cpp
jsonrpc_SOURCES = jsonrpc.cpp
messageSigning_SOURCES = messageSigning.cpp
#nameList_SOURCES = nameList.cpp
idn_SOURCES = idn.cpp
