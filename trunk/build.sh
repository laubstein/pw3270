#!/bin/bash

OLDDIR=$PWD
PACKAGE=`basename $OLDDIR`

RPMDIR=`rpm --eval="%{u2p:%{_rpmdir}}"`
RPMARCH=`rpm --eval="%{u2p:%{_target_cpu}}"`
VENDOR=`rpm --eval="%{u2p:%{_vendor}}"`


mv *~ /tmp

make -C src clean

cd ..
tar -zcf `rpm --eval="%{u2p:%{_sourcedir}}"`/$PACKAGE.tar.gz --exclude=*CVS* $PACKAGE

if [ "$?" != "0" ]; then
   exit -1
fi

cd $OLDDIR

rm -f $RPMDIR/$RPMARCH/$PACKAGE*.rpm
rm -f $RPMDIR/noarch/$PACKAGE*.rpm

rpmbuild -ba $PACKAGE.spec

echo "Enviando arquivo source para o servidor..."
scp `rpm --eval="%{u2p:%{_srcrpmdir}}"`/$PACKAGE*.src.rpm $USER@os2team.df.intrabb.bb.com.br:/home/html/src/$VENDOR

echo "Enviando arquivo binario o servidor..."
scp $RPMDIR/$RPMARCH/$PACKAGE*.rpm $RPMDIR/noarch/$PACKAGE*.rpm $USER@os2team.df.intrabb.bb.com.br:/home/html/$VENDOR



