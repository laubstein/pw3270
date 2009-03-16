#!/bin/sh
export PREFIX=/usr/local/cross-tools

export TARGET=i386-mingw32

export CC=$PREFIX"/bin/i386-mingw32-gcc"
export CXX=$PREFIX"/bin/i386-mingw32-g++"
export WINDRES=$PREFIX"/bin/i386-mingw32-windres"

export CFLAGS="-O2 -march=i586 -mms-bitfields"
export CXXFLAGS="-O2 -march=i586 -mms-bitfields"
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig
export PATH=$PREFIX/bin:$PREFIX/$TARGET/bin:/bin:/usr/bin
export LD_LIBRARY_PATH=$PREFIX/$TARGET/lib
export LDFLAGS=-L$PREFIX/$TARGET/lib
export OBJDUMP=$PREFIX/bin/mingw32-objdump
export HOST_CC=/usr/bin/gcc

export TARGET=i386-mingw32
export cache=win32.cache

autoconf
./configure	--cache-file="$cache" \
		--target=$TARGET \
		--host=$TARGET \
		--build=i686-linux \
		--without-gnome


status=$?
rm -f "$cache"
exit $status

