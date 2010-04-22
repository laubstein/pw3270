#!/bin/bash

PACKAGE_VERSION=4.1
PACKAGE_RELEASE=0

REV_FILE=./revision.m4

if [ -d .svn ]; then
	SVN=`which svn 2> /dev/null`
else
	SVN=""
fi

if [ ! -z $SVN ]; then

	echo "Updating sources..."
	$SVN update >/dev/null 2>&1
	if [ "$?" == "0" ]; then

		if $SVN --xml info >/dev/null 2>&1; then
			REV=`$SVN --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*revision="\([0-9]*\)".*<\/commit>.*/\1/'`
			LCD=`$SVN --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*<date>\([0-9\-]*\)\T\([0-9\:]*\)\..*<\/date>.*<\/commit>.*/\1 \2/'`
		elif $SVN --version --quiet >/dev/null 2>&1; then
			REV=`$SVN info | grep "^Revision:" | cut -d" " -f2`
			LCD=`$SVN info | grep "^Last Changed Date:" | cut -d" " -f4,5`
		else
			REV=`date +%y%m%d%H%M`
			LCD=""
		fi

		echo "m4_define([SVN_REV], $REV)" > $REV_FILE
		echo "m4_define([SVN_DATE], $LCD)" >> $REV_FILE
		echo "m4_define([SVN_RELEASE], $PACKAGE_RELEASE)" >> $REV_FILE

	fi

#	SVN2CL=`which svn2cl.sh 2> /dev/null`
#	if [ ! -z $SVN2CL ]; then
#		echo "Creating changelog ..."
#		$SVN2CL
#	fi

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

if [ ! -f $REV_FILE ]; then
	echo "Can't create $REV_FILE Is svn installed?"
	exit -1
fi

echo "pw3270 $PACKAGE_VERSION-$PACKAGE_RELEASE Ok"



