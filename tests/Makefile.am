AM_CPPFLAGS = -I$(top_srcdir)/src -std=c++11 -pedantic -Wall -Wextra
LDADD = -L$(top_builddir)/src -lnmcrpc -lcurl

check_PROGRAMS = jsonrpc
TESTS = $(check_PROGRAMS)

jsonrpc_SOURCES = jsonrpc.cpp
