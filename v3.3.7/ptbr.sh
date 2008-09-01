#!/bin/bash
make g3270.pot
cp g3270.pot po/pt_BR.po
gtranslator --auto-translate=po/pt_BR.po
gtranslator po/pt_BR.po
rm -f ~/.gtranslator/umtf/personal-learn-buffer.xml
printf "po/pt_BR.po\ny" | gtranslator-build-learn-buffer
