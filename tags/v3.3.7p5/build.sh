#!/bin/sh

svn update

NAME=SisBB
ICON=sisbb/sisbb98.ico
LOGO=sisbb/sisbb98.jpg
RELEASE=5

BuildWin() {
	#
	# Build windows version
	#
	echo -e "\e]2;Building Windows Installer\a"

	make clean

	ln -sf /usr/i386-mingw32/GTK-Runtime .
	./win32.sh --gtkroot="GTK-Runtime" --locale="locale" --release=$RELEASE --name=$NAME --icon=$ICON --logo=$LOGO
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
	./configure --with-release=$RELEASE

	RPMDIR=`rpm --eval="%{u2p:%{_rpmdir}}"`
	RPMARCH=`rpm --eval="%{u2p:%{_target_cpu}}"`
	VENDOR=`rpm --eval="%{u2p:%{_vendor}}"`

	mkdir -p $RPMDIR
	mkdir -p $RPMARCH
	mkdir -p `rpm --eval="%{u2p:%{_srcrpmdir}}"`
	mkdir -p `rpm --eval="%{u2p:%{_builddir}}"`

	make clean
	make tgz
	if [ "$?" != "0" ]; then
		exit -1
	fi

	cp *.tar.gz `rpm --eval="%{u2p:%{_sourcedir}}"`
	if [ "$?" != "0" ]; then
		exit -1
	fi

	mv -f `rpm --eval="%{u2p:%{_srcrpmdir}}"`/g3270*.src.rpm /tmp > /dev/null 2>&1
	rpmbuild -ba g3270.spec
	if [ "$?" != "0" ]; then
		exit -1
	fi

	if [ "$1" == "src" ]; then
		echo "Enviando arquivo source para o servidor..."
		echo scp `rpm --eval="%{u2p:%{_srcrpmdir}}"`/g3270*.src.rpm $USER@suportelinux.df.bb.com.br:src/suse/g3270-latest.src.rpm
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

	echo "Copiando pacotes para o $TARGET_FOLDER..."
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

	if svn --xml info >/dev/null 2>&1; then
		REV=`svn --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*revision="\([0-9]*\)".*<\/commit>.*/\1/'`
	elif svn --version --quiet >/dev/null 2>&1; then
		REV=`svn info | grep "^Revision:" | cut -d" " -f2`
	else
		REV="Debug"
	fi

	PREFIX=$HOME/bin/g3270

	rm -fr $PREFIX
	./configure --enable-plugins --with-release="svn$REV"

	make clean
	make Debug
	if [ "$?" != "0" ]; then
		exit -1
	fi

	make src/g3270/g3270.png

	rm -fr $PREFIX
	mkdir -p $PREFIX/plugins

	install --mode=755 bin/Debug/g3270 $PREFIX
	install --mode=755 bin/Debug/lib3270.so $PREFIX

	make po/pt_BR.po
	msgfmt -c -v -o /usr/share/locale/pt_BR/LC_MESSAGES/g3270.mo po/pt_BR.po
	if [ "$?" != "0" ]; then
		exit -1
	fi

#	mkdir -p $PREFIX/share/locale/pt_BR/LC_MESSAGES
#	msgfmt -c -v -o $PREFIX/share/locale/pt_BR/LC_MESSAGES/g3270.mo po/pt_BR.po

	make bin/Debug/plugins/rx3270.so

	mkdir -p $PREFIX/plugins
	cp bin/Debug/plugins/*.so $PREFIX/plugins

}

BuildRemote() {

	EXEC_CMD="ssh $USER@os2team.df.intrabb.bb.com.br"
	
	rm -f ./GTK-Runtime

	echo "Enviando arquivo fonte para o servidor de empacotamento..."
	$EXEC_CMD "rm -rf tmp/build.tmp"
	$EXEC_CMD "mkdir -p tmp/build.tmp"
	$EXEC_CMD 'mkdir --parents --mode=755 `rpm --eval="%{u2p:%{_builddir}}"`'
	$EXEC_CMD 'mkdir --parents --mode=755 `rpm --eval="%{u2p:%{_rpmdir}}"`/`rpm --eval="%{u2p:%{_target_cpu}}"`'
	$EXEC_CMD 'mkdir --parents --mode=755 `rpm --eval="%{u2p:%{_rpmdir}}"`/noarch'
	$EXEC_CMD 'mkdir --parents --mode=755 `rpm --eval="%{u2p:%{_specdir}}"`'
	$EXEC_CMD 'mkdir --parents --mode=755 `rpm --eval="%{u2p:%{_sourcedir}}"`'
	$EXEC_CMD 'mkdir --parents --mode=755 `rpm --eval="%{u2p:%{_srcrpmdir}}"`'
	scp -r . $USER@os2team.df.intrabb.bb.com.br:tmp/build.tmp/.
	if [ "$?" != "0" ]; then
	   echo "Erro ao copiar fontes para o servidor virtual"
	fi
	$EXEC_CMD "cd tmp/build.tmp; ./build.sh rpm"

	ln -sf /usr/i386-mingw32/GTK-Runtime ./GTK-Runtime

}

if [ -z "$TMPDIR" ]; then
	TMPDIR=/tmp
	export TMPDIR
fi

aclocal
autoconf

if [ -z "$1" ]; then

	BuildWin
	BuildRPM "src"
	BuildDebug
	BuildRemote

else

	until [ -z "$1" ]
	do
		if [ "$1" == "win" ]; then
			BuildWin
		elif [ "$1" == "rpm" ]; then
			BuildRPM
		elif [ "$1" == "debug" ]; then
			BuildDebug
		elif [ "$1" == "remote" ]; then
			BuildRemote
		else
			echo "Parametro invalido: $1"
			exit -1
		fi

		shift
	done
fi

echo "G3270 Build Ok!"

