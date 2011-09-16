#!/bin/bash

OUTPUT="sources.txt"

rm -f $OUTPUT

TEMPFILE=`mktemp`
find . -name *.c  > $TEMPFILE
find . -name *.h  >> $TEMPFILE

while read FILENAME
do
	NAME=$(echo $FILENAME | sed "s@./src/@@")
	LINES=$(wc -l $FILENAME | cut -d" " -f1)
	echo "Processing $FILENAME with $LINES lines of code"

	echo "---------------------------------------------------------------------------------" >> $OUTPUT
	echo " *** $NAME ***" >> $OUTPUT
	echo "---------------------------------------------------------------------------------" >> $OUTPUT

	SED_COMMAND="s#aplicativos mainframe.#aplicativos mainframe. Registro no INPI sob o nome G3270.#1;s#@@FILENAME@@#`basename $FILENAME`#;s#e possui .* linhas de código#e possui $LINES linhas de código#"

	sed "$SED_COMMAND"  $FILENAME >> $OUTPUT
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

	sed "$SED_COMMAND" $TEMPSOURCE > $FILENAME
	if [ "$?" != "0" ]; then
		echo "Erro ao atualizar contagem de linhas em $FILENAME"
		exit -1
	fi


done < $TEMPFILE
rm -f $TEMPFILE
