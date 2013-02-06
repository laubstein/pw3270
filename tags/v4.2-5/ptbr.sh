#!/bin/bash

svn update

rm -f ~/.gtranslator/umtf/personal-learn-buffer.xml
printf "po/pt_BR.po\ny" | /usr/local/share/gtranslator/scripts/build-gtranslator-learn-buffer.sh

make .bin/pot/pw3270.pot
cp .bin/pot/pw3270.pot po/pt_BR.po

gtranslator --auto-translate=po/pt_BR.po > /dev/null 2>&1
gtranslator po/pt_BR.po > /dev/null 2>&1

TEMPFILE=$(mktemp)

msgfmt -c -v -o $TEMPFILE po/pt_BR.po
if [ "$?" != "0" ]; then
	exit -1
fi

cp -f $TEMPFILE /usr/local/share/locale/pt_BR/LC_MESSAGES/pw3270.mo
if [ "$?" != "0" ]; then
	exit -1
fi

rm -f $TEMPFILE

