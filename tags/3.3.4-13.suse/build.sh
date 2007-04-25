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

if [ ! -d /usr/include/g3270 ]; then
   mkdir /usr/include/g3270
   ln -s $PWD/src/lib/*.h /usr/include/g3270

fi

rm -f $RPMDIR/$RPMARCH/$PACKAGE*.rpm
rm -f $RPMDIR/noarch/$PACKAGE*.rpm

rpmbuild -ba $PACKAGE.spec
if [ "$?" != "0" ]; then
   exit -1
fi

SOURCE_PACKET=`rpm --eval="%{u2p:%{_srcrpmdir}}"`/$PACKAGE*.src.rpm
PACKET_VERSION=`rpm -qp --qf '%{VERSION}-%{RELEASE}' $SOURCE_PACKET`

if [ "$1" != "--debug" ]; then
   echo Gerando copia do repositorio para a tag $PACKET_VERSION
   svn rm --force -m "Atualizando $PACKET_VERSION" http://$USER@suportelinux.df.bb.com.br/svn/$PACKAGE/tags/$PACKET_VERSION
   svn cp -m "Pacote $PACKET_VERSION" http://$USER@suportelinux.df.bb.com.br/svn/$PACKAGE/trunk http://$USER@suportelinux.df.bb.com.br/svn/$PACKAGE/tags/$PACKET_VERSION

   echo "Enviando arquivo source para o servidor..."
   scp `rpm --eval="%{u2p:%{_srcrpmdir}}"`/$PACKAGE*.src.rpm $USER@suportelinux.df.bb.com.br:/dados/src/$VENDOR
   if [ "$?" != "0" ]; then
      echo "Erro ao copiar o pacote fonte"
      exit -1
   fi
   echo "Enviando arquivo binario o servidor..."
   scp $RPMDIR/$RPMARCH/$PACKAGE*.rpm $USER@os2team.df.intrabb.bb.com.br:/home/matriz/pacotes/$VENDOR/bb
   if [ "$?" != "0" ]; then
      echo "Erro ao copiar o pacote binario"
   fi
   
fi   

mv -f `rpm --eval="%{u2p:%{_srcrpmdir}}"`/$PACKAGE*.src.rpm /tmp
mv -f $RPMDIR/$RPMARCH/$PACKAGE*.rpm /tmp
