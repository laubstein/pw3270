#!/bin/sh

svn update

NAME=SisBB
ICON=sisbb/sisbb98.ico
LOGO=sisbb/sisbb98.jpg

#
# Build windows version
#
ln -sf ~/win32/GTK-Runtime .
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

cp locale/pt_BR/LC_MESSAGES/$NAME.mo /usr/share/locale/pt_BR/LC_MESSAGES/g3270.mo
if [ "$?" != "0" ]; then
	exit -1
fi

wine  ~/.wine/drive_c/Arquivos\ de\ programas/NSIS/makensis.exe g3270.nsi
if [ "$?" != "0" ]; then
	echo "*** ERRO AO GERAR O PACOTE DE INSTALACAO"
	exit -1
fi

scp $NAME-*.exe perry@os2team:public_html/g3270
mv $NAME-*.exe ~/Desktop/

make clean

#
# Build Linux version
#
autoconf
./configure

RPMDIR=`rpm --eval="%{u2p:%{_rpmdir}}"`
RPMARCH=`rpm --eval="%{u2p:%{_target_cpu}}"`
VENDOR=`rpm --eval="%{u2p:%{_vendor}}"`
RELEASE=`grep Release g3270.spec | sed 's/ //g' |cut -d: -f2 |cut -d. -f1`

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

echo "Enviando arquivo source para o servidor..."
scp `rpm --eval="%{u2p:%{_srcrpmdir}}"`/g3270*.src.rpm $USER@os2team.df.intrabb.bb.com.br:public_html/g3270
if [ "$?" != "0" ]; then
	echo "Erro ao copiar o pacote fonte"
	exit -1
fi

echo "Enviando arquivo binario para o servidor..."
scp $RPMDIR/$RPMARCH/g3270*.rpm $USER@os2team.df.intrabb.bb.com.br:public_html/g3270
if [ "$?" != "0" ]; then
	echo "Erro ao copiar o pacote binario"
	exit -1
fi

#if [ "$1" != "--debug" ]; then
#   echo Gerando copia do repositorio para a tag $PACKET_VERSION

#   if [ "$VENDOR" = "suse" ]; then
#   	svn rm --force -m "Empacotamento $PACKET_VERSION" http://$USER@suportelinux.df.bb.com.br/svn/$PACKAGE/tags/$PACKET_VERSION
#   	svn cp -m "Empacotamento $PACKET_VERSION" http://$USER@suportelinux.df.bb.com.br/svn/$PACKAGE/trunk http://$USER@suportelinux.df.bb.com.br/svn/$PACKAGE/tags/$PACKET_VERSION
#   fi

#  echo "Enviando arquivo source para o servidor..."
#   scp `rpm --eval="%{u2p:%{_srcrpmdir}}"`/$PACKAGE*.src.rpm $USER@suportelinux.df.bb.com.br:/dados/src/$VENDOR/$PACKAGE-latest.src.rpm
#   if [ "$?" != "0" ]; then
#      echo "Erro ao copiar o pacote fonte"
#      exit -1
#   fi

#   echo "Enviando arquivo binario o servidor..."
#   scp $RPMDIR/$RPMARCH/$PACKAGE*.rpm $USER@os2team.df.intrabb.bb.com.br:/home/matriz/pacotes/$VENDOR/bb/$RPMARCH
#   if [ "$?" != "0" ]; then
#      echo "Erro ao copiar o pacote binario"
#   fi

#fi

./configure --enable-plugins --prefix=/home/perry/bin/g3270

make install
if [ "$?" != "0" ]; then
	exit -1
fi

make bin/Release/plugins/rx3270.so
mkdir -p /home/perry/bin/g3270/lib/g3270/plugins
cp bin/Release/plugins/*.so /home/perry/bin/g3270/lib/g3270/plugins
cp ui/rexx.xml /home/perry/bin/g3270/share/g3270/ui

cat > ~/bin/g3270.sh << EOF 
#!/bin/bash
cd /home/perry/bin/g3270/bin
LD_LIBRARY_PATH=../lib ./g3270 
EOF
chmod +x ~/bin/g3270.sh
make clean

echo "Pacotes g3270 gerados"


