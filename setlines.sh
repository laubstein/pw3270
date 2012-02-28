#!/bin/bash

TEMPFILE=`mktemp`
find . -name *.c > $TEMPFILE

while read FILENAME
do
	LINES=$(wc -l $FILENAME | cut -d" " -f1)
	echo "Processing $FILENAME with $LINES lines of code"

	CONVERTED=`mktemp`

	sed -e "s#@@FILENAME@@#`basename $FILENAME`#;s#e possui .* linhas de código#e possui $LINES linhas de código#" $FILENAME > $CONVERTED
	if [ "$?" != "0" ]; then
		echo "Erro ao ajustar contagem de linhas em $FILENAME"
		exit -1
	fi

	mv -f $CONVERTED $FILENAME
	if [ "$?" != "0" ]; then
		echo "Erro ao copiar arquivo convertido para $FILENAME"
		exit -1
	fi

done < $TEMPFILE
rm -f $TEMPFILE


