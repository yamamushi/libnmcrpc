lib_LIBRARIES = libnmcrpc.a

libnmcrpc_a_CXXFLAGS = -std=c++11 -pedantic -Wall -Wextra
libnmcrpc_a_SOURCES = \
  JsonRpc.cpp \
  NamecoinInterface.cpp \
  NameRegistration.cpp \
  RpcSettings.cpp

pkgincludedir = $(includedir)/nmcrpc
pkginclude_HEADERS = JsonRpc.hpp JsonRpc.tpp \
  NamecoinInterface.hpp \
  NameRegistration.hpp \
  RpcSettings.hpp
