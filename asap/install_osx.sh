#!/bin/sh

MPLTMP=/tmp/mpltmp
FTPURL="ftp://ftp.atnf.csiro.au/pub/software/asap"

trap 'rm -rf /tmp/mpltmp' EXIT

#test -n $1 && export http_proxy=$1

if [ `id -u` -ne 0 ]; then
  echo "$0: Must be executed by root (sudo)"
  exit 1
fi
ASAPVERS=4.1
OSXVERS=$(sw_vers |  grep -o '10\.[7-8]')
echo $OSXVERS
if [ -z ${OSXVERS} ]; then echo "Only OS X >= 10.7 supported"; exit 1; fi
if [ "$OSXVERS" == '10.8' ]
then
    ASAPEGG="${FTPURL}/${ASAPVERS}/asap-4.1.2-py2.7-macosx-${OSXVERS}-intel.egg"
else
    ASAPEGG="${FTPURL}/${ASAPVERS}/asap-4.1.0-py2.7-macosx-${OSXVERS}-intel.egg"
fi
MPLEGG="${FTPURL}/matplotlib/matplotlib-1.1.1-py2.7-macosx-${OSXVERS}-intel.egg"
MPL=$(/usr/bin/python -c 'import matplotlib' >/dev/null 2>&1)
# install matplotlib if not already there
if [ -n $? ]
then
    echo "Matplotlib not installed. Installing from CASS ftp server..."
    /usr/bin/easy_install-2.7 -U ${MPLEGG}
fi

# This should pull in IPython if required
/usr/bin/easy_install-2.7 -U ${ASAPEGG}

# make sure readline is installed
/usr/bin/easy_install-2.7 readline

# update measures data
mkdir ${MPLTMP}
MPLCONFIGDIR="${MPLTMP}" 
/usr/local/bin/asap_update_data
