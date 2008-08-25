#!/bin/sh

#
# Build windows version
#
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

make clean

scp g3270_installer.exe perry@os2team:public_html/g3270
mv g3270_installer.exe ~/Desktop/

#
# Build Linux version
#
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

make Release
if [ "$?" != "0" ]; then
	exit -1
fi

rm -fr ~/bin/g3270
mkdir -p ~/bin/g3270
cp bin/Release/g3270		~/bin/g3270
cp bin/Release/lib3270.so	~/bin/g3270
cp src/g3270/actions.conf	~/bin/g3270
cp src/g3270/*.xml		~/bin/g3270
cp /home/perry/Project/g3270/conf/sisbb.jpg ~/bin/g3270/g3270.jpg

cat > ~/bin/g3270.sh << EOF 
#!/bin/bash
cd ~/bin/g3270
LD_LIBRARY_PATH=. ./g3270
EOF
chmod +x ~/bin/g3270.sh
make clean

echo "Pacotes g3270 gerados"

