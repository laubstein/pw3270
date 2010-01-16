#!/bin/bash

PACKAGE_VERSION=4.1
PACKAGE_RELEASE=0

svn update
if [ "$?" == "0" ]; then

	REV_FILE=./revision.m4

	if svn --xml info >/dev/null 2>&1; then
		REV=`svn --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*revision="\([0-9]*\)".*<\/commit>.*/\1/'`
		LCD=`svn --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*<date>\([0-9\-]*\)\T\([0-9\:]*\)\..*<\/date>.*<\/commit>.*/\1 \2/'`
	elif svn --version --quiet >/dev/null 2>&1; then
		REV=`svn info | grep "^Revision:" | cut -d" " -f2`
		LCD=`svn info | grep "^Last Changed Date:" | cut -d" " -f4,5`
	else
		REV=`date +%y%m%d%H%M`
		LCD=""
	fi

	echo "m4_define([SVN_REV], $REV)" > $REV_FILE
	echo "m4_define([SVN_DATE], $LCD)" >> $REV_FILE
	echo "m4_define([SVN_RELEASE], $PACKAGE_RELEASE)" >> $REV_FILE

fi

if [ -e "debian/control.in" ]; then
	sed "s/@PACKAGE_VERSION@/$PACKAGE_VERSION/g;s/@PACKAGE_RELEASE@/$PACKAGE_RELEASE/g" "debian/control.in" > "debian/control"
fi

aclocal
if [ "$?" != "0" ]; then
	exit -1
fi

autoconf
if [ "$?" != "0" ]; then
	exit -1
fi

echo "pw3270 $PACKAGE_VERSION-$PACKAGE_RELEASE"



