#!/bin/bash
IAM=`basename -- "$0"`
VERBOSE=
WORKDIR=`pwd`

# Send a warning message to stderr.
function warning
{
   echo "[$IAM] *** WARNING: $@" 1>&2
}

 # Send an error message to stderr and exit.
function error
{
   echo "[$IAM] *** ERROR: $@" 1>&2
   exit -1
}

# Verbose a message to stderr.
function verbose
{
   [ -n "$VERBOSE" ] && echo "[$IAM] $@" 1>&2
}

[ -z "$CASAPATH" ] && error "CASAPATH environment variable not defined!"
CASAROOT=`echo $CASAPATH | awk '{print $1}'`
verbose "Casa root directory: $CASAROOT"
[ ! -d "$CASAROOT" ] && error "can't find Casa root directory $CASAROOT!"

CASAARCH=`echo $CASAPATH | awk '{printf "%s/%s", $1, $2}'`
verbose "Casa architecture: $CASAARCH"

VOCLIENTROOT="/Users/wyoung/voclient"

SAXONJAR="$CASAARCH/lib/saxon8.jar"
if [[ ! -e ${SAXONJAR} ]]; then
   SAXONJAR=${CASAROOT}/java/saxon8.jar
   if [[ ! -e ${SAXONJAR} ]] ; then
      SAXONJAR=${CASAROOT}/lib/saxon8.jar
   fi
fi
[ ! -e "$SAXONJAR" ] && error "can't find the Saxon processor $SAXONJAR!"
verbose "Saxon processor: $SAXONJAR"
SAXON="java -jar $SAXONJAR"
verbose "Saxon command: $SAXON"

if [[ `uname -s` = "Darwin" ]]; then
   INFOFILE=`mktemp bmt`
else
   INFOFILE=`mktemp`
fi
verbose "intermediate task file: $INFOFILE"
[ ! -e "$INFOFILE" ] && error "can't create an intermediate file $INFOFILE!"

MYPACK=$1
SWIGFILE=${MYPACK}.i
CASABUILDROOT=${CASAARCH}
PLUGROOT="../.."
CASAINCLUDE=${CASAROOT}"/bindings"
BOOSTROOT=/opt/casa/darwin11/include
EXTRAINCDIRS="\".\", \"${VOCLIENTROOT}/include\""
PYINCLUDE="/opt/casa/darwin11/Library/Frameworks/Python.framework/Headers"
EXTRALIBDIRS="\"${VOCLIENTROOT}/lib\""
EXTRALIBS="\"VO\", \"tools\""
${SAXON} ./${MYPACK}.xml ${CASAROOT}/bindings/install/casa2swigxml.xsl > ${INFOFILE}
sed -i -e "s/exmlns/xmlns/" ${INFOFILE} 
${SAXON} ${INFOFILE} ${CASAROOT}/bindings/install/casa2c++h.xsl > ${MYPACK}_cmpt.h
sed -i -e "s/<?xml version=.*//" ${MYPACK}_cmpt.h
rm ${INFOFILE}
#
# Write out the swig interface file
#
echo "%module ${MYPACK}" > ${SWIGFILE}
echo "%feature(\"kwargs\");" >> ${SWIGFILE}
echo "%feature(\"autodoc\");" >> ${SWIGFILE}
echo "%include <tools/casa_typemaps.i> " >> ${SWIGFILE} 
echo "%include \"${MYPACK}_cmpt.h\"" >> ${SWIGFILE} 
echo "%{" >> ${SWIGFILE} 
echo "#include <${MYPACK}_cmpt.h>" >> ${SWIGFILE} 
echo "%}" >> ${SWIGFILE} 
#
# Write out the setup.py file
#
echo > setup.py
echo '"""' >> setup.py
echo "setup.py file for SWIG ${MYPACK}" >> setup.py
echo '"""' >> setup.py
echo >> setup.py
echo "from distutils.core import setup, Extension" >> setup.py
echo >> setup.py
echo "${MYPACK}_module = Extension(\"_${MYPACK}\", sources=[\"${MYPACK}_wrap.cxx\", \"${MYPACK}_cmpt.cc\"]," >> setup.py
echo "                       include_dirs=[\"${CASABUILDROOT}/include/casacore\"," >> setup.py
echo "                                     \"${CASABUILDROOT}/include/casacode\"," >> setup.py
echo "                                     \"${CASAINCLUDE}\"," >> setup.py
echo "                                     \"${PLUGROOT}\"," >> setup.py
echo "                                     \"${BOOSTROOT}\"," >> setup.py
echo "                                     \".\"," >> setup.py
echo "                                     \"${PYINCLUDE}\"," >> setup.py
echo "                                     ${EXTRAINCDIRS}]," >> setup.py
echo "                       library_dirs=[${EXTRALIBDIRS}, \"${CASABUILDROOT}/lib\"]," >> setup.py
echo "                       libraries=[${EXTRALIBS}]" >> setup.py
echo "                                  )" >> setup.py
echo "" >> setup.py
echo "setup (name = \"${MYPACK}\"," >> setup.py
echo "       version = \"0.1\"," >> setup.py
echo "       author      = \"Wes Young\"," >> setup.py
echo "       description = \"\"\"Simple swig $MYPACK example\"\"\"," >> setup.py
echo "       ext_modules = [${MYPACK}_module]," >> setup.py
echo "       py_modules = [\"${MYPACK}\"]," >> setup.py
echo "       )" >> setup.py

swig -I${CASAROOT}/bindings -Wall -c++ -python ${MYPACK}.i
python setup.py build
python setup.py install
