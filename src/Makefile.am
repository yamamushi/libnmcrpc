lib_LTLIBRARIES = libnmcrpc.la

libnmcrpc_la_CXXFLAGS = -pedantic -Wall -Wextra -fPIC
libnmcrpc_la_CXXFLAGS += -std=c++03 -DCXX_03 -Dnullptr=NULL
libnmcrpc_la_CXXFLAGS += $(LIBIDN_CFLAGS)
libnmcrpc_la_LIBADD = $(LIBIDN_LIBS)
libnmcrpc_la_SOURCES = \
  JsonRpc.cpp \
  IdnTool.cpp \
  NamecoinInterface.cpp \
  NameRegistration.cpp \
  RpcSettings.cpp

pkgincludedir = $(includedir)/nmcrpc
pkginclude_HEADERS = \
  JsonRpc.hpp JsonRpc.tpp \
  IdnTool.hpp \
  NamecoinInterface.hpp NamecoinInterface.tpp \
  NameRegistration.hpp \
  RpcSettings.hpp
