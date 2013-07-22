#!/bin/bash

PACKAGE_VERSION=4.2
PACKAGE_RELEASE=7
REV_FILE=./revision.m4
REV=`date +%y%m%d%H%M`

SVN=`which svn 2> /dev/null`

if test -x "$SVN" ; then

	echo "Updating sources..."
	"$SVN" update
	if [ "$?" != "0" ]; then

		echo "$SVN update failed!"

	else

		LANG="EN_US"

		if $SVN --xml info >/dev/null 2>&1; then
			REV=`$SVN --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*revision="\([0-9]*\)".*<\/commit>.*/\1/'`
			LCD=`$SVN --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*<date>\([0-9\-]*\)\T\([0-9\:]*\)\..*<\/date>.*<\/commit>.*/\1 \2/'`
		elif $SVN --version --quiet >/dev/null 2>&1; then
			REV=`$SVN info | grep "^Revision:" | cut -d" " -f2`
			LCD=`$SVN info | grep "^Last Changed Date:" | cut -d" " -f4,5`
		else
			LCD=""
		fi

		if [ "$LCD" == "" ]; then
			LCD=`date +%Y%m%d`
		fi

		URL=`svn info | grep "URL: " | sed -s "s@URL: @@g"`

		echo "m4_define([SVN_REV], $REV)" > $REV_FILE
		echo "m4_define([SVN_DATE], $LCD)" >> $REV_FILE
		echo "m4_define([SVN_URL], $URL)" >> $REV_FILE
		echo "m4_define([SVN_RELEASE], $PACKAGE_RELEASE)" >> $REV_FILE

	fi

	SVN2CL=`which svn2cl.sh 2> /dev/null`
	if [ ! -z $SVN2CL ]; then
		echo "Creating changelog ..."
		$SVN2CL
	fi
fi

if [ ! -f $REV_FILE ]; then
	echo "Can't create $REV_FILE is svn available?"
	exit -1
fi

case $MACHTYPE in

*-apple-*)

	if [ -z $JHBUILD_PREFIX ]; then
		echo "JHBUILD_PREFIX is undefined"
		exit -1
	fi

	;;

*)
	SVN_SCRIPT="s/@PACKAGE_VERSION@/$PACKAGE_VERSION/g;s/@PACKAGE_RELEASE@/$PACKAGE_RELEASE/g;s/@PACKAGE_REVISION@/$REV/g;s/@DATE_CHANGED@/`date --rfc-2822`/g"

	if [ -e "debian/control-base.in" ]; then
		sed "$SVN_SCRIPT" "debian/control-base.in" > "debian/control-base"
	fi

	if [ -e "debian/changelog.in" ]; then
		sed "$SVN_SCRIPT" "debian/changelog.in" > "debian/changelog"
	fi
	;;

esac

aclocal
if [ "$?" != "0" ]; then
	exit -1
fi

autoconf
if [ "$?" != "0" ]; then
	exit -1
fi

echo "pw3270 $PACKAGE_VERSION-$PACKAGE_RELEASE Ok"



