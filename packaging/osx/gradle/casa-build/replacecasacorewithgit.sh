#!/bin/bash
echo "########################################################################"
date
if [ -z "$1" ]
then
 echo "Missing casacoredir."
exit 1
fi
if [ -z "$2" ]
then
 echo "Missing casacoredir."
exit 1
fi
casacoredir=$1
gitcheckoutdir=$2
echo "deleting casacoredir $casacoredir"
sleep 10
rm -rf $casacoredir
mkdir -p $gitcheckoutdir
cd $gitcheckoutdir
oldGitCasaCore=$gitcheckoutdir/casacore
rm -rf $oldGitCasaCore
git clone https://github.com/casacore/casacore
mv casacore $casacoredir
