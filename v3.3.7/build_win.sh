#!/bin/bash

IMAGE=image.w32
LOCALE=locale

mv ~/Desktop/g3270_installer.exe ~/tmp

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

wine  ~/.wine/drive_c/Arquivos\ de\ programas/NSIS/makensis.exe g3270.nsi
if [ "$?" != "0" ]; then
	echo "*** ERRO AO GERAR O PACOTE DE INSTALACAO"
	exit -1
fi

mv g3270_installer.exe ~/Desktop/
