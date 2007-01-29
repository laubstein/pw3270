#!/bin/bash

OLDDIR=$PWD
PACKAGE=`basename $OLDDIR`

RPMDIR=`rpm --eval="%{u2p:%{_rpmdir}}"`
RPMARCH=`rpm --eval="%{u2p:%{_target_cpu}}"`
VENDOR=`rpm --eval="%{u2p:%{_vendor}}"`

RELEASE=`grep Release g3270.spec | sed 's/ //g' |cut -d: -f2 |cut -d. -f1`

mv *~ /tmp

make -C src clean

cd ..
tar -zcvf `rpm --eval="%{u2p:%{_sourcedir}}"`/$PACKAGE.tar.gz --exclude=*.svn* $PACKAGE

if [ "$?" != "0" ]; then
   exit -1
fi

cd $OLDDIR

rm -f $RPMDIR/$RPMARCH/$PACKAGE*.rpm
rm -f $RPMDIR/noarch/$PACKAGE*.rpm

rpmbuild -ba $PACKAGE.spec
if [ "$?" != "0" ]; then
   exit -1
fi

echo "Enviando arquivo source para o servidor..."
scp `rpm --eval="%{u2p:%{_srcrpmdir}}"`/$PACKAGE*.src.rpm $USER@suportelinux.df.bb.com.br:/dados/src/$VENDOR
if [ "$?" != "0" ]; then
   echo "Erro ao copiar o pacote fonte"
   exit -1
fi
mv -f `rpm --eval="%{u2p:%{_srcrpmdir}}"`/$PACKAGE*.src.rpm /tmp

echo "Enviando arquivo binario o servidor..."
scp $RPMDIR/$RPMARCH/$PACKAGE*.rpm $USER@os2team.df.intrabb.bb.com.br:/home/html/$VENDOR
if [ "$?" = "0" ]; then
   mv -f $RPMDIR/$RPMARCH/$PACKAGE*.rpm /tmp
   echo "Binarios movidos para /tmp"
fi

