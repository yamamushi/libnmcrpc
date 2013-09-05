AM_CPPFLAGS = -I$(top_srcdir)/src -std=c++11 -pedantic -Wall -Wextra
LDADD = -L$(top_builddir)/src -lnmcrpc -lcurl -ljsoncpp

check_PROGRAMS = jsonrpc basicInfo
TESTS = $(check_PROGRAMS)

basicInfo_SOURCES = basicInfo.cpp
jsonrpc_SOURCES = jsonrpc.cpp
