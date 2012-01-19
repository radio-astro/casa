MYPACK=vo
SWIGFILE=${MYPACK}.i
java -jar /opt/casa/active/code/xmlcasa/install/saxon8.jar ./${MYPACK}.xml /opt/casa/active/code/xmlcasa/install/casa2swigxml.xsl > ./stuff.xml
sed -e "s/exmlns/xmlns/" ./stuff.xml > ./stuff2.xml
java -jar /opt/casa/active/code/xmlcasa/install/saxon8.jar ./stuff2.xml /opt/casa/active/code/xmlcasa/install/casa2c++h.xsl > ./stuff.h
sed -e "s/<?xml version=.*//" ./stuff.h > ${MYPACK}_cmpt.h
echo "%module ${MYPACK}" > ${SWIGFILE}
echo '%include "casa_typemaps.i" ' >> ${SWIGFILE} 
echo "%include \"${MYPACK}_cmpt.h\"" >> ${SWIGFILE} 
echo "%{" >> ${SWIGFILE} 
echo "#include <${MYPACK}_cmpt.h>" >> ${SWIGFILE} 
echo "%}" >> ${SWIGFILE} 
