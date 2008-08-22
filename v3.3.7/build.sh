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

mv g3270_installer.exe ~/Desktop/

#
# Build Local Linux version
#
./configure
make Release
if [ "$?" != "0" ]; then
	exit -1
fi

rm -fr ~/bin/g3270
mkdir -p ~/bin/g3270
cp bin/Release/g3270		~/bin/g3270
cp bin/Release/lib3270.so	~/bin/g3270
cp src/g3270/*.conf		~/bin/g3270
cp src/g3270/*.xml		~/bin/g3270
cp /home/perry/Project/g3270/conf/sisbb.jpg ~/bin/g3270/g3270.jpg

cat > ~/bin/g3270.sh << EOF 
#!/bin/bash
cd ~/bin/g3270
LD_LIBRARY_PATH=. ./g3270
EOF
chmod +x ~/bin/g3270.sh
make clean
