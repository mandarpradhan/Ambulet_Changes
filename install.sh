#!/bin/sh
# Check for root access
# Install script for SeaMo Version 0.1
# Create an install space under /usr/local/SeaMo and install
# the binaries and config files
# <seamo@ece.iisc.ernet.in>
# <deshpandesanvi@gmail.com>
#
INSTALLDIR=/usr/local/seamo
SBINDIR=$INSTALLDIR/sbin
CONFDIR=$INSTALLDIR/conf
CONFFILE=$CONFDIR/seamo.conf
INIT=/etc/init.d/seamod

#Check for root access

if [ "$(id -u)" != "0" ]; then
  echo "SeaMo requires root user permissions"
  exit 1
fi

[ -f $INIT ] || mv files/seamod $INIT

[ -d $INSTALLDIR ] || mkdir $INSTALLDIR
[ -d $SBINDIR ] || mkdir $SBINDIR
[ -d $CONFDIR ] || mkdir $CONFDIR

[ -f src/seamo_vho_core/seamo_vho ] || echo "SeaMo not compiled"
[ -f src/seamo_prehandoff/seamod ] || exit

cp src/seamo_vho_core/seamo_vho $SBINDIR
cp src/seamo_prehandoff/seamod $SBINDIR
cp src/seamo_vrms/seamo_vrms $SBINDIR

[ -f $CONFFILE ] || mv files/seamo.conf $CONFDIR
rm -rf files

echo ""
echo "**************************CONGRATULATIONS***********************"
echo "                      INSTALLED SUCCESSFULLY"
echo ""
