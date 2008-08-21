#!/bin/bash

./win32.sh
if [ "$?" != "0" ]; then
	exit -1
fi

make Release

if [ "$?" != "0" ]; then
	exit -1
fi

mkdir -p locale/pt_BR/LC_MESSAGES
if [ "$?" != "0" ]; then
	exit -1
fi

msgfmt -c -v -o locale/pt_BR/LC_MESSAGES/g3270.mo po/pt_BR.po
if [ "$?" != "0" ]; then
	exit -1
fi

wine  ~/.wine/drive_c/Arquivos\ de\ programas/NSIS/makensis.exe g3270.nsi
if [ "$?" != "0" ]; then
	echo "*** ERRO AO GERAR O PACOTE DE INSTALACAO"
	exit -1
fi

mv g3270_installer.exe ~/Desktop/
