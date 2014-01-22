AM_CPPFLAGS = -I$(top_srcdir)/src -std=c++11 -pedantic -Wall -Wextra
LDADD = $(top_builddir)/src/libnmcrpc.la -lcurl -ljsoncpp

check_PROGRAMS = jsonrpc basicInfo messageSigning nameList
TESTS = $(check_PROGRAMS)

basicInfo_SOURCES = basicInfo.cpp
jsonrpc_SOURCES = jsonrpc.cpp
messageSigning_SOURCES = messageSigning.cpp
nameList_SOURCES = nameList.cpp
