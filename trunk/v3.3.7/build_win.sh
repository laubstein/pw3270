#!/bin/bash

PACKAGE_NAME=g3270
PACKAGE_VERSION=3.3.7p5
LOCALE=locale

mv ~/Desktop/g3270_installer.exe ~/tmp > /dev/null 2>&1

sed "s/@PACKAGE_NAME@/$PACKAGE_NAME/g;s/@PACKAGE_VERSION@/$PACKAGE_VERSION/g" g3270.nsi.in > g3270.nsi
if [ "$?" != "0" ]; then
	exit -1
fi

rm -fr bin/Release
make Release

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

wine  ~/.wine/drive_c/Arquivos\ de\ programas/NSIS/makensis.exe g3270.nsi
if [ "$?" != "0" ]; then
	echo "*** ERRO AO GERAR O PACOTE DE INSTALACAO"
	exit -1
fi

mv g3270_installer.exe ~/Desktop/
