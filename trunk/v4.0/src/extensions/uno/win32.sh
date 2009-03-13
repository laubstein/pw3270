#!/bin/sh
export PREFIX=/usr/local/cross-tools

export TARGET=i386-mingw32

export CC=$PREFIX"/bin/i386-mingw32-gcc -mms-bitfields"
export CXX=$PREFIX"/bin/i386-mingw32-g++ -mms-bitfields"
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

OO_SDK_HOME="/usr/local/cross-tools/OpenOffice.org_3.0_SDK_win32/sdk"
NATIVE_OO_SDK_HOME="/usr/lib/ooo3/solver"
NATIVE_OFFICE_HOME="/usr/lib/ooo3"

# IDLC="LD_LIBRARY_PATH=/usr/lib/ooo3/ure/lib $NATIVE_OO_SDK_HOME/bin/idlc" \
# CPPUMAKER="LD_LIBRARY_PATH=/usr/lib/ooo3/ure/lib $NATIVE_OO_SDK_HOME/bin/cppumaker" \

autoconf
./configure	--cache-file="$cache" \
		--target=$TARGET \
		--host=$TARGET \
		--build=i686-linux \
		--with-office-sdk-home="$OO_SDK_HOME" \
		--with-idlc-program="LD_LIBRARY_PATH=$NATIVE_OFFICE_HOME/ure/lib/ $NATIVE_OO_SDK_HOME/bin/idlc" \
		--with-cppumaker-program="LD_LIBRARY_PATH=$NATIVE_OFFICE_HOME/ure/lib/ $NATIVE_OO_SDK_HOME/bin/cppumaker" \
		--with-regmerge-program="LD_LIBRARY_PATH=$NATIVE_OFFICE_HOME/lib/ $NATIVE_OFFICE_HOME/ure/bin/regmerge" \
		--with-regcomp-program="LD_LIBRARY_PATH=$NATIVE_OFFICE_HOME/ure/bin/regcomp" \
		--with-types.rdb="$NATIVE_OO_SDK_HOME/bin/types.rdb" \
		--with-ooincludepath="$OO_SDK_HOME/include"


status=$?
rm -f "$cache"
exit $status

