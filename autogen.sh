#!/bin/sh

libtoolize
aclocal
autoconf
autoheader
automake -a
