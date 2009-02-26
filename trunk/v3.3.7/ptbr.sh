#!/bin/bash
make g3270.pot
cp g3270.pot po/pt_BR.po
gtranslator --auto-translate=po/pt_BR.po > /dev/null 2>&1
gtranslator po/pt_BR.po > /dev/null 2>&1
rm -f ~/.gtranslator/umtf/personal-learn-buffer.xml
printf "po/pt_BR.po\ny" | /usr/share/gtranslator/scripts/build-gtranslator-learn-buffer.sh
mkdir -p locale/pt_BR/LC_MESSAGES
if [ "$?" != "0" ]; then
	exit -1
fi

msgfmt -c -v -o locale/pt_BR/LC_MESSAGES/$NAME.mo po/pt_BR.po
if [ "$?" != "0" ]; then
	exit -1
fi

cp locale/pt_BR/LC_MESSAGES/$NAME.mo /usr/share/locale/pt_BR/LC_MESSAGES/g3270.mo
if [ "$?" != "0" ]; then
	exit -1
fi
