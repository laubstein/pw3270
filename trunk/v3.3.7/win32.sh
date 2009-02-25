#!/bin/bash

name=g3270
version=3.3.7p5
release=`date +%y%m%d`
locale="locale"
PLUGIN=plugins
gtkroot="GTK2-Runtime"
crossroot=/usr/local/cross-tools
rexxroot=/usr/local/cross-tools/ooRexx
icon="image/default.ico"
logo="image/default.jpg"
rexx=yes

until [ -z "$1" ]
do
   if [ ${1:0:2} = '--' ]; then
      tmp=${1:2}
      parameter=${tmp%%=*}
      value=${tmp##*=}
      eval "$parameter=$value" > /dev/null 2>&1
   fi
   shift
done

WINDRES="$crossroot/bin/i386-mingw32-windres"
CC="$crossroot/bin/i386-mingw32-gcc"
PKG_CONFIG_PATH="$crossroot/lib/pkgconfig"
GTK_MODULES="glib-2.0 gtk+-2.0 gthread-2.0 gmodule-2.0"
SSL_MODULES="libcrypto libssl openssl"

if [ "$rexx" == "yes" ]; then

	if [ -e "$rexxroot/api/rexx.h" ]; then
		SELECTED_PLUGINS="rx3270"
		REXX_CFLAGS="-I$rexxroot/api"
		REXX_LIBS="-L$rexxroot/api -lrexx -lrexxapi"
	fi

fi

TEMPFILE=`mktemp`
echo "s&@PACKAGE_NAME@&$name&g;" > $TEMPFILE
echo "s&@PACKAGE_RELEASE@&$release&g;" >> $TEMPFILE
echo "s&@PACKAGE@&$name&g;" >> $TEMPFILE
echo "s&@CC@&$CC&g;" >> $TEMPFILE
echo "s&@PACKAGE_VERSION@&$version&g;" >> $TEMPFILE
echo "s&@GTK_CFLAGS@&`PKG_CONFIG_PATH=$PKG_CONFIG_PATH pkg-config --cflags $GTK_MODULES`&g;" >> $TEMPFILE
echo "s&@GTK_LIBS@&`PKG_CONFIG_PATH=$PKG_CONFIG_PATH pkg-config --libs $GTK_MODULES`&g;" >> $TEMPFILE
echo "s&@LIBGNOME_CFLAGS@&&g;" >> $TEMPFILE
echo "s&@LIBGNOME_LIBS@&&g;" >> $TEMPFILE
echo "s&@LIBSSL_CFLAGS@&`PKG_CONFIG_PATH=$PKG_CONFIG_PATH pkg-config --cflags $SSL_MODULES`&g;" >> $TEMPFILE
echo "s&@LIBSSL_LIBS@&`PKG_CONFIG_PATH=$PKG_CONFIG_PATH pkg-config --libs $SSL_MODULES`&g;" >> $TEMPFILE
echo "s&@NATIVECC@&gcc&g;" >> $TEMPFILE
echo "s&@WINDRES@&$WINDRES&g;" >> $TEMPFILE
echo "s&@OS_LIBS@&-lws2_32&g;" >> $TEMPFILE
echo "s&@XCPPFLAGS@&-D_WIN32 -DWC3270 -D_WIN32_WINNT=0x0500&g;" >> $TEMPFILE
echo "s&@PROGRAM_FLAGS@&&g;" >> $TEMPFILE
echo "s&@PROGRAM_LIBS@&&g;" >> $TEMPFILE
echo "s&@DLLEXT@&dll&g;" >> $TEMPFILE
echo "s&@OBJEXT@&o&g;" >> $TEMPFILE
echo "s&@BINEXT@&.exe&g;" >> $TEMPFILE
echo "s&@CROSS@&CROSS=1&g;" >> $TEMPFILE
echo "s&@EXEOPT@&-mwindows&g;" >> $TEMPFILE
echo "s&@EXTRA_FLAGS@&-mno-cygwin&g;" >> $TEMPFILE
echo "s&@EXTRASRC@&resources.rc&g;" >> $TEMPFILE
echo "s&@EXTRA_TARGETS@&w3n46.dll&g;" >> $TEMPFILE
echo "s&@REXX_CFLAGS@&$REXX_CFLAGS&g;" >> $TEMPFILE
echo "s&@REXX_LIBS@&$REXX_LIBS&g;" >> $TEMPFILE
echo "s&@host_os@&windows&g;" >> $TEMPFILE
echo "s&@prefix@&.&g;" >> $TEMPFILE
echo "s&@exec_prefix@&.&g;" >> $TEMPFILE
echo "s&@bindir@&.&g;" >> $TEMPFILE
echo "s&@libdir@&.&g;" >> $TEMPFILE

echo "s&@localedir@&$locale&g;" >> $TEMPFILE
echo "s&#undef LOCALEDIR&#define LOCALEDIR \"$locale\"&g;" >> $TEMPFILE

echo "s&@GTKROOT@&$gtkroot&g;" >> $TEMPFILE
echo "s&@ICONFILE@&$icon&g;" >> $TEMPFILE
echo "s&@LOGOFILE@&$logo&g;" >> $TEMPFILE
echo "s&@LOGOEXT@&jpg&g;" >> $TEMPFILE
echo "s&@datarootdir@&.&g;" >> $TEMPFILE
echo "s&@SELECTED_PLUGINS@&$SELECTED_PLUGINS&g;" >> $TEMPFILE

echo "s&#undef PACKAGE_NAME&#define PACKAGE_NAME \"$name\"&g;" >> $TEMPFILE
echo "s&#undef PACKAGE_VERSION&#define PACKAGE_VERSION \"$version\"&g;" >> $TEMPFILE
echo "s&#undef X3270_TN3270E&#define X3270_TN3270E 1&g;" >> $TEMPFILE
echo "s&#undef X3270_TRACE&#define X3270_TRACE 1&g;" >> $TEMPFILE
echo "s&#undef X3270_FT&#define X3270_FT 1&g;" >> $TEMPFILE
echo "s&#undef X3270_ANSI&#define X3270_ANSI 1&g;" >> $TEMPFILE
echo "s&#undef X3270_PRINTER&#define X3270_PRINTER 1&g;" >> $TEMPFILE
echo "s&#undef HAVE_LIBSSL&#define HAVE_LIBSSL 1&g;" >> $TEMPFILE
echo "s&#undef PLUGINDIR&#define PLUGINDIR \"$PLUGIN\"&g;" >> $TEMPFILE
echo "s&#undef MOUSE_POINTER_CHANGE&#define MOUSE_POINTER_CHANGE 1&g;" >> $TEMPFILE
	


mv ~/Desktop/g3270_installer.exe ~/tmp > /dev/null 2>&1

sed --file=$TEMPFILE Makefile.in > Makefile
if [ "$?" != "0" ]; then
	exit -1
fi

# make clean

sed --file=$TEMPFILE src/include/lib3270/config.h.in > src/include/lib3270/config.h
if [ "$?" != "0" ]; then
	exit -1
fi

sed --file=$TEMPFILE g3270.nsi.in > g3270.nsi
if [ "$?" != "0" ]; then
	exit -1
fi

sed --file=$TEMPFILE src/g3270/resources.rc.in > src/g3270/resources.rc
if [ "$?" != "0" ]; then
	exit -1
fi

sed --file=$TEMPFILE src/plugins/rexx/Makefile.in > src/plugins/rexx/Makefile
if [ "$?" != "0" ]; then
	exit -1
fi


sed --file=$TEMPFILE src/g3270/version.c.in > src/g3270/version.c
if [ "$?" != "0" ]; then
	exit -1
fi

echo $name $version configured for win32
