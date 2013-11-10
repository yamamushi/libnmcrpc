AM_CPPFLAGS = -I$(top_srcdir)/src -std=c++11 -pedantic -Wall -Wextra
LDADD = -L$(top_builddir)/src -lnmcrpc -lcurl -ljsoncpp

noinst_PROGRAMS = registerName
bin_PROGRAMS = nmreg nmupdate

registerName_SOURCES = registerName.cpp
nmreg_SOURCES = nmreg.cpp
nmupdate_SOURCES = nmupdate.cpp
