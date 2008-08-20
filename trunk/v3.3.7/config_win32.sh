#!/bin/bash

PACKAGE_NAME=g3270
PACKAGE_VERSION=3.3.7p5
LOCALE=locale

PKG_CONFIG_PATH="/usr/i386-mingw32/lib/pkgconfig"
GTK_MODULES="glib-2.0 gtk+-2.0 gthread-2.0"
SSL_MODULES="libcrypto libssl openssl"

TEMPFILE=`mktemp`
echo "s&@PACKAGE_NAME@&$PACKAGE_NAME&g;" > $TEMPFILE
echo "s&@PACKAGE@&$PACKAGE_NAME&g;" >> $TEMPFILE
echo "s&@CC@&mingw32-gcc&g;" >> $TEMPFILE
echo "s&@PACKAGE_VERSION@&$PACKAGE_VERSION&g;" >> $TEMPFILE
echo "s&@GTK_CFLAGS@&`pkg-config --cflags $GTK_MODULES`&g;" >> $TEMPFILE
echo "s&@GTK_LIBS@&`pkg-config --libs $GTK_MODULES`&g;" >> $TEMPFILE
echo "s&@SSL_CFLAGS@&`pkg-config --cflags $SSL_MODULES`&g;" >> $TEMPFILE
echo "s&@SSL_LIBS@&`pkg-config --libs $SSL_MODULES`&g;" >> $TEMPFILE
echo "s&@NATIVECC@&gcc&g;" >> $TEMPFILE
echo "s&@WINDRES@&mingw32-windres&g;" >> $TEMPFILE
echo "s&@OS_LIBS@&-lws2_32&g;" >> $TEMPFILE
echo "s&@XCPPFLAGS@&-D_WIN32 -DWC3270 -D_WIN32_WINNT=0x0500&g;" >> $TEMPFILE
echo "s&@DLLEXT@&dll&g;" >> $TEMPFILE
echo "s&@OBJEXT@&o&g;" >> $TEMPFILE
echo "s&@BINEXT@&.exe&g;" >> $TEMPFILE
echo "s&@EXTRASRC@&resources.rc&g;" >> $TEMPFILE
echo "s&#undef PACKAGE_NAME&#define PACKAGE_NAME \"$PACKAGE_NAME\"&g;" >> $TEMPFILE
echo "s&#undef PACKAGE_VERSION&#define PACKAGE_VERSION \"$PACKAGE_VERSION\"&g;" >> $TEMPFILE
echo "s&#undef X3270_TN3270E&#define X3270_TN3270E 1&g;" >> $TEMPFILE
echo "s&#undef X3270_TRACE&#define X3270_TRACE 1&g;" >> $TEMPFILE
echo "s&#undef X3270_FT&#define X3270_FT 1&g;" >> $TEMPFILE
echo "s&#undef X3270_ANSI&#define X3270_ANSI 1&g;" >> $TEMPFILE
echo "s&#undef X3270_PRINTER&#define X3270_PRINTER 1&g;" >> $TEMPFILE
echo "s&#undef HAVE_LIBSSL&#define HAVE_LIBSSL 1&g;" >> $TEMPFILE
echo "s&#undef LOCALEDIR&#define LOCALEDIR \"$LOCALE\"&g;" >> $TEMPFILE

mv ~/Desktop/g3270_installer.exe ~/tmp > /dev/null 2>&1

sed --file=$TEMPFILE Makefile.in > Makefile
if [ "$?" != "0" ]; then
	exit -1
fi

make clean

sed --file=$TEMPFILE src/g3270/config.h.in > src/g3270/config.h
if [ "$?" != "0" ]; then
	exit -1
fi

sed --file=$TEMPFILE src/lib/conf.h.in > src/lib/conf.h
if [ "$?" != "0" ]; then
	exit -1
fi

sed --file=$TEMPFILE g3270.nsi.in > g3270.nsi
if [ "$?" != "0" ]; then
	exit -1
fi
