#!/bin/bash
#./autogen.sh

VERSION=$(grep AC_INIT configure.ac | cut -d[ -f3 | cut -d] -f1)
TIMESTAMP=$(LANG=en_US date)
. ./revision

rm -f debian/changelog
EDITOR=true dch --preserve -v $VERSION-$PACKAGE_LEVEL -u low --create --package pw3270
sed -i "s@UNRELEASED@unstable@;s@Initial release. (Closes: #XXXXXX)@SVN Revision $PACKAGE_REVISION@" debian/changelog

dpkg-buildpackage -rfakeroot -uc -us
