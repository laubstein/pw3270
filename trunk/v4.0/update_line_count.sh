#!/bin/bash

OUTPUT="sources.txt"

rm -f $OUTPUT

TEMPFILE=`mktemp`
find . -name *.c | sort > $TEMPFILE

while read FILENAME
do
	NAME=$(echo $FILENAME | sed "s@./src/@@")
	LINES=$(wc -l $FILENAME | cut -d" " -f1)
	echo "Processing $FILENAME with $LINES lines of code"

	echo "---------------------------------------------------------------------------------" >> $OUTPUT
	echo " *** $NAME ***" >> $OUTPUT
	echo "---------------------------------------------------------------------------------" >> $OUTPUT

	sed -e "s#@@FILENAME@@#`basename $FILENAME`#;s#e possui .* linhas de código#e possui $LINES linhas de código#" $FILENAME >> $OUTPUT
	if [ "$?" != "0" ]; then
		echo "Erro ao ajustar contagem de linhas em $FILENAME"
		exit -1
	fi

	TEMPSOURCE=`mktemp`
	cp -f $FILENAME $TEMPSOURCE
	if [ "$?" != "0" ]; then
		echo "Erro ao criar temporario para $FILENAME"
		exit -1
	fi

	sed -e "s#@@FILENAME@@#`basename $FILENAME`#;s#e possui .* linhas de código#e possui $LINES linhas de código#" $TEMPSOURCE > $FILENAME
	if [ "$?" != "0" ]; then
		echo "Erro ao atualizar contagem de linhas em $FILENAME"
		exit -1
	fi


done < $TEMPFILE
rm -f $TEMPFILE
