lib_LTLIBRARIES = libnmcrpc.la

libnmcrpc_la_CXXFLAGS = -std=c++11 -pedantic -Wall -Wextra -fPIC
libnmcrpc_la_SOURCES = \
  JsonRpc.cpp \
  NamecoinInterface.cpp \
  NameRegistration.cpp \
  RpcSettings.cpp

pkgincludedir = $(includedir)/nmcrpc
pkginclude_HEADERS = JsonRpc.hpp JsonRpc.tpp \
  NamecoinInterface.hpp NamecoinInterface.tpp \
  NameRegistration.hpp \
  RpcSettings.hpp
