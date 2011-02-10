#!/bin/bash -x
#
# Creates a disk image (dmg) on Mac OS X from the command line.
# usage:
#    mkdmg <volname> <vers> <srcdir>
#
# Where <volname> is the name to use for the mounted image, <vers> is the version
# number of the volume and <srcdir> is where the contents to put on the dmg are.
#
# The result will be a file called <volname>-<vers>.dmg
#
# Source: http://hints.macworld.com/article.php?story=20020311215452999
#

./bootstrap.sh
if [ "$?" != "0" ]; then
	exit -1
fi

./configure --with-macui
if [ "$?" != "0" ]; then
	exit -1
fi

make clean
make bin/pw3270.app
if [ "$?" != "0" ]; then
	exit -1
fi

VOL="pw3270"
VER="4.2"
FILES="bin/pw3270.app"

DMG="tmp-$VOL.dmg"

# create temporary disk image and format, ejecting when done
SIZE=$(du -sk ${FILES} | cut -f1)
SIZE=$((${SIZE}/1000+1))

hdiutil create "$DMG" -megabytes ${SIZE} -ov -type UDIF -fs HFS+ -volname "$VOL"
if [ ! "$?" == "0" ]; then
	exit -1
fi

hdiutil attach "$DMG"
if [ ! "$?" == "0" ]; then
	hdiutil detach "/Volumes/$VOL"
	exit -1
fi

cp -Rv "bin/pw3270.app" "/Volumes/$VOL"
if [ ! "$?" == "0" ]; then
	hdiutil detach "/Volumes/$VOL"
	exit -1
fi

hdiutil detach "/Volumes/$VOL"
if [ ! "$?" == "0" ]; then
	exit -1
fi

# convert to compressed image, delete temp image
rm -f "${VOL}-${VER}.dmg"
hdiutil convert "$DMG" -format UDZO -o "$HOME/Desktop/${VOL}-${VER}.dmg"
rm -f "$DMG"
ls -l "$HOME/Desktop/${VOL}-${VER}.dmg"

