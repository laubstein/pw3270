#!/bin/bash

IMAGE=image.w32
LOCALE=locale

rm -fr bin/Release
make Release

if [ "$?" != "0" ]; then
	exit -1
fi

rm -fr $IMAGE
mkdir $IMAGE
if [ "$?" != "0" ]; then
	exit -1
fi

mkdir -p $LOCALE/pt_BR/LC_MESSAGES
if [ "$?" != "0" ]; then
	exit -1
fi

msgfmt -c -v -o $LOCALE/pt_BR/LC_MESSAGES/g3270.mo po/pt_BR.po
if [ "$?" != "0" ]; then
	exit -1
fi

