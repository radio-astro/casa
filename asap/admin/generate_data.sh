#!/bin/sh

if [ "$AIPSPATH" != "" ]; then
    a_root=`/bin/echo $AIPSPATH | awk '{print $1}'`
else
    echo "No aips++ installation found"
    exit 1
fi
    
target="`pwd`/data.tar.gz"
cd $a_root
echo "Generating ASAP data archive ${target} from aips++ installation."
tar cfz ${target} data/ephemerides data/geodetic
echo "Done."
