#!/bin/sh
#
# Install script for SeaMo Version 0.1
# Create an install space under /usr/local/SeaMo and install
# the binaries and config files
# <seamo@ece.iisc.ernet.in>
#
INSTALLDIR=/usr/local/seamo
CONFFILE=$INSTALLDIR/conf/seamo.conf
INIT=/etc/init.d/seamod

#Check for root access

if [ "$(id -u)" != "0" ]; then
  echo "SeaMo requires root user permissions"
  exit 1
fi


cd src/seamo_prehandoff
make clean

cd -

cd src/seamo_vho_core
make clean

cd -

[ -d files ] || mkdir files

[ -f $INIT ] && mv $INIT files
[ -f $CONFFILE ] && mv $CONFFILE files

rm -rf $INSTALLDIR
