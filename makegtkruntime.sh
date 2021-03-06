#!/bin/bash
. scripts.conf

TARGET_PATH=".bin/gtkruntime"

# Clean target folder
rm -fr "$TARGET_PATH/*"
mkdir -p "$TARGET_PATH"

# Build DLL list
TEMPFILE="temp1.tmp"
cat > $TEMPFILE << EOF
intl.dll
libatk-1.0-0.dll
libcairo-2.dll
libgailutil-18.dll
libgdk_pixbuf-2.0-0.dll
libgdk-win32-2.0-0.dll
libgettextlib-0-*.dll
libgettextpo*.dll
libgettextsrc*.dll
libgio-2.0-0.dll
libglib-2.0-0.dll
libgmodule-2.0-0.dll
libgobject-2.0-0.dll
libgthread-2.0-0.dll
libgtk-win32-2.0-0.dll
libpango-1.0-0.dll
libpangocairo-1.0-0.dll
libpangoft2-1.0-0.dll
libpangowin32-1.0-0.dll
zlib1.dll
libpng*.dll
libfontconfig*.dll
libasprintf*.dll
libgettextlib*.dll
libexpat*.dll
freetype*.dll
gspawn-win32-helper-console.exe
gspawn-win32-helper.exe
gdk-pixbuf-query-loaders.exe
EOF

# jpeg62.dll
# libtiff*.dll

while read FILE
do
	FILEPATH=$(find "$GTK_RUNTIME_PATH/bin" -name "$FILE")
	if [ ! -z $FILEPATH ]; then
		echo "$FILEPATH ..."
		cp $FILEPATH "$TARGET_PATH"
		if [ "$?" != "0" ]; then
			echo "Can´t copy $FILEPATH"
			exit -1
		fi
	else
		echo "Can´t find $FILE"
	fi
done < $TEMPFILE
rm -f $TEMPFILE

# Build locale dirs
cat > $TEMPFILE << EOF
atk10.mo
gdk-pixbuf.mo
gettext-runtime.mo
gettext-tools.mo
glib20.mo
gtk20-properties.mo
gtk20.mo
libiconv.mo
EOF

rm -fr $TARGET_PATH/share/locale/pt_BR/LC_MESSAGES
mkdir -p $TARGET_PATH/share/locale/pt_BR/LC_MESSAGES

while read FILE
do
	echo "$GTK_RUNTIME_PATH/share/locale/pt_BR/LC_MESSAGES/$FILE ..."

	if [ -e "$GTK_RUNTIME_PATH/share/locale/pt_BR/LC_MESSAGES/$FILE" ]; then
		cp "$GTK_RUNTIME_PATH/share/locale/pt_BR/LC_MESSAGES/$FILE" "$TARGET_PATH/share/locale/pt_BR/LC_MESSAGES"
		if [ "$?" != "0" ]; then
			echo "Can´t copy $FILE"
			exit -1
		fi
	fi

done < $TEMPFILE
rm -f $TEMPFILE

# Copy default theme
THEME_PATH="themes/MS-Windows/gtk-2.0"
rm -fr "$TARGET_PATH/$THEME_PATH"
mkdir -p "$TARGET_PATH/share/$THEME_PATH"

echo "$GTK_RUNTIME_PATH/share/$THEME_PATH/gtkrc ..."
cp "$GTK_RUNTIME_PATH/share/$THEME_PATH/gtkrc" "$TARGET_PATH/share/$THEME_PATH/gtkrc"
if [ "$?" != "0" ]; then
	echo "Can´t copy default theme"
	exit -1
fi

cat > $TEMPFILE << EOF
engines
loaders
EOF

rm -fr $TARGET_PATH/lib/gtk-2.0/$GTK_RUNTIME_VERSION/
mkdir -p $TARGET_PATH/lib/gtk-2.0/$GTK_RUNTIME_VERSION/
while read DIRNAME
do
	if [ -d "$GTK_RUNTIME_PATH/lib/gtk-2.0/$GTK_RUNTIME_VERSION/$DIRNAME" ]; then
		echo "$GTK_RUNTIME_PATH/lib/gtk-2.0/$GTK_RUNTIME_VERSION/$DIRNAME ..."
		mkdir -p "$TARGET_PATH/lib/gtk-2.0/$GTK_RUNTIME_VERSION/$DIRNAME"
		cp -r "$GTK_RUNTIME_PATH/lib/gtk-2.0/$GTK_RUNTIME_VERSION/$DIRNAME" "$TARGET_PATH/lib/gtk-2.0/$GTK_RUNTIME_VERSION"
		if [ "$?" != "0" ]; then
			echo "Can´t copy $DIRNAME"
			exit -1
		fi
	fi
done < $TEMPFILE
rm -f $TEMPFILE

mkdir -p $TARGET_PATH/etc/gtk-2.0/
echo "gtk-theme-name = \"MS-Windows\"" >  $TARGET_PATH/etc/gtk-2.0/gtkrc
if [ "$?" != "0" ]; then
	echo "Can´t set theme name"
	exit -1
fi

