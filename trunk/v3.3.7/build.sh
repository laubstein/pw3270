#!/bin/sh

svn update

NAME=SisBB
ICON=sisbb/sisbb98.ico
LOGO=sisbb/sisbb98.jpg

BuildWin() {
	#
	# Build windows version
	#
	echo -e "\e]2;Building Windows Installer\a"

	make clean

	ln -sf /usr/i386-mingw32/GTK-Runtime .
	./win32.sh --gtkroot="GTK-Runtime" --locale="locale" --name=$NAME --icon=$ICON --logo=$LOGO
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

	msgfmt -c -v -o locale/pt_BR/LC_MESSAGES/$NAME.mo po/pt_BR.po
	if [ "$?" != "0" ]; then
		exit -1
	fi

	wine  ~/.wine/drive_c/Arquivos\ de\ programas/NSIS/makensis.exe g3270.nsi
	if [ "$?" != "0" ]; then
		echo "*** ERRO AO GERAR O PACOTE DE INSTALACAO"
		exit -1
	fi

	scp $NAME-*.exe perry@os2team:public_html/g3270
	mkdir -p ~/win32
	mv $NAME-*.exe ~/win32/

}

BuildRPM() {
	#
	# Build Linux version
	#
	echo -e "\e]2;Building Linux RPM\a"

	autoconf
	./configure

	RPMDIR=`rpm --eval="%{u2p:%{_rpmdir}}"`
	RPMARCH=`rpm --eval="%{u2p:%{_target_cpu}}"`
	VENDOR=`rpm --eval="%{u2p:%{_vendor}}"`
	RELEASE=`grep Release g3270.spec | sed 's/ //g' |cut -d: -f2 |cut -d. -f1`

	make clean
	make tgz
	if [ "$?" != "0" ]; then
		exit -1
	fi

	cp *.tar.gz `rpm --eval="%{u2p:%{_sourcedir}}"`
	if [ "$?" != "0" ]; then
		exit -1
	fi

	rpmbuild -ba g3270.spec
	if [ "$?" != "0" ]; then
		exit -1
	fi

	if [ "$1" != "--debug" ]; then
		echo "Enviando arquivo source para o servidor..."
		scp `rpm --eval="%{u2p:%{_srcrpmdir}}"`/g3270*.src.rpm $USER@suportelinux.df.bb.com.br:src/suse/g3270-latest.src.rpm
		if [ "$?" != "0" ]; then
			echo "Erro ao copiar o pacote fonte"
			exit -1
		fi
	fi

	echo "Enviando arquivo binario para o servidor..."
	scp $RPMDIR/$RPMARCH/g3270*.rpm $USER@os2team.df.intrabb.bb.com.br:public_html/g3270
	if [ "$?" != "0" ]; then
		echo "Erro ao copiar o pacote binario"
		exit -1
	fi

	TARGET_FOLDER=$USER@os2team.df.intrabb.bb.com.br:/home/matriz/pacotes/$VENDOR/bb/$RPMARCH

	if [ "$VENDOR" == "suse" ]; then
		TARGET_FOLDER="$USER@storage:/dados/suse/$(rpm --eval="%{suse_version}" | cut -b1-2)/bb/$RPMARCH"
	fi

	scp $RPMDIR/$RPMARCH/g3270*.rpm $TARGET_FOLDER
	if [ "$?" != "0" ]; then
		echo "Erro ao copiar o pacote binario"
		exit -1
	fi

	mv $RPMDIR/$RPMARCH/g3270*.rpm $TMPDIR
	mv `rpm --eval="%{u2p:%{_srcrpmdir}}"`/g3270*.src.rpm $TMPDIR

	make clean

}

BuildDebug() {
	#
	# Build Debug
	#
	echo -e "\e]2;Building Local Debug\a"

	PREFIX=$HOME/bin/g3270

	rm -fr $PREFIX
	./configure --enable-plugins --prefix=$PREFIX

	make clean
	make Debug
	if [ "$?" != "0" ]; then
		exit -1
	fi

	mkdir -p $PREFIX/bin/debug/plugins
	mkdir -p $PREFIX/lib
	mkdir -p $PREFIX/ui

	install --mode=755 bin/Debug/g3270 $PREFIX/bin/debug
	install --mode=755 bin/Debug/lib3270.so $PREFIX/lib
	install --mode=644 ui/g3270.xml $PREFIX/ui
	install --mode=644 ui/g3270.act $PREFIX/ui
	install --mode=644 src/g3270/g3270.jpg $PREFIX
	install --mode=644 src/g3270/colors.conf $PREFIX

	make po/pt_BR.po
	msgfmt -c -v -o /usr/share/locale/pt_BR/LC_MESSAGES/g3270.mo po/pt_BR.po
	if [ "$?" != "0" ]; then
		exit -1
	fi

	mkdir -p $PREFIX/share/locale/pt_BR/LC_MESSAGES
	msgfmt -c -v -o $PREFIX/share/locale/pt_BR/LC_MESSAGES/g3270.mo po/pt_BR.po

	make bin/Debug/plugins/rx3270.so

	mkdir -p $PREFIX/lib/plugins
	cp bin/Debug/plugins/*.so $PREFIX/bin/debug/plugins
	cp ui/rexx.xml $PREFIX/ui
	cp ui/debug.xml $PREFIX/ui

	start_script=$HOME/bin/g3270.sh

	rm -f $start_script
	echo #!/bin/bash > $start_script 
	echo "cd \`dirname \$0\`/g3270/bin/debug" >> $start_script
	echo LD_LIBRARY_PATH=../../lib ./g3270 >> $start_script 

	chmod +x $start_script

	make clean

}

if [ -z "$1" ]; then

	BuildWin
	BuildRPM
	BuildDebug

else

	until [ -z "$1" ]
	do
		if [ "$1" == "win" ]; then
			BuildWin
		elif [ "$1" == "rpm" ]; then
			BuildRPM
		elif [ "$1" == "debug" ]; then
			BuildDebug
		fi

		shift
	done
fi

echo "G3270 Build Ok!"


