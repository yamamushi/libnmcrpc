lib_LTLIBRARIES = libnmcrpc.la

libnmcrpc_la_CXXFLAGS = -pedantic -Wall -Wextra -fPIC
libnmcrpc_la_CXXFLAGS += -std=c++03 -Dnullptr=NULL
libnmcrpc_la_CXXFLAGS += $(LIBIDN_CFLAGS)
libnmcrpc_la_LIBADD = $(LIBIDN_LIBS)
libnmcrpc_la_SOURCES = \
  CoinInterface.cpp \
  JsonRpc.cpp \
  IdnTool.cpp \
  NameInterface.cpp \
  NameRegistration.cpp \
  RpcSettings.cpp

pkgincludedir = $(includedir)/nmcrpc
pkginclude_HEADERS = \
  CoinInterface.hpp \
  JsonRpc.hpp JsonRpc.tpp \
  IdnTool.hpp \
  NameInterface.hpp NameInterface.tpp \
  NameRegistration.hpp \
  RpcSettings.hpp
