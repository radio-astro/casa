#!/bin/bash
MYPACK=vo
SWIGFILE=${MYPACK}.i
CASABUILDROOT="/opt/casa/active/darwinllvm"
CASAINCLUDE="/opt/casa/active/bindings/tools"
EXTRAINCDIRS="\".\", \"/iraf/iraf/vendor/voclient/include\""
PYINCLUDE="/opt/casa/darwin11/Library/Frameworks/Python.framework/Headers"
EXTRALIBDIRS="\"/iraf/iraf/vendor/voclient/lib\""
EXTRALIBS="\"VOCLient\", \"xmlcasa\""
java -jar /opt/casa/active/bindings/install/saxon8.jar ./${MYPACK}.xml /opt/casa/active/bindings//install/casa2swigxml.xsl > ./stuff.xml
sed -e "s/exmlns/xmlns/" ./stuff.xml > ./stuff2.xml
java -jar /opt/casa/active/bindings/install/saxon8.jar ./stuff2.xml /opt/casa/active/bindings/install/casa2c++h.xsl > ./stuff.h
sed -e "s/<?xml version=.*//" ./stuff.h > ${MYPACK}_cmpt.h
#
# Write out the swig interface file
#
echo "%module ${MYPACK}" > ${SWIGFILE}
echo "%include <casa_typemaps.i> " >> ${SWIGFILE} 
echo "%include \"${MYPACK}_cmpt.h\"" >> ${SWIGFILE} 
echo "%{" >> ${SWIGFILE} 
echo "#include <${MYPACK}_cmpt.h>" >> ${SWIGFILE} 
echo "%}" >> ${SWIGFILE} 
exit
#
# Write out the setup.py file
#
echo >> setup.py
echo '"""' >> setup.py
echo "setup.py file for SWIG ${MYPACK}" >> setup.py
echo '"""' >> setup.py
echo >> setup.py
echo "from distutils.core import setup, Extension" >> setup.py
echo >> setup.py
echo "${MYPACK}_module = Extension(\"_${MYPACK}\", sources=[\"${MYPACK}_wrap.cxx\", \"${MYPACK}_cmpt.cc\"]," >> setup.py
echo "                       include_dirs=[\"${CASABUILDROOT}/include/casacore\"," >> setup.py
echo "                                     \"${CASAINCLUDE}\"," >> setup.py
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

