#!/bin/bash
OUTFILE=casac.py
echo "import __casac__"
echo "class casac : " 
for file in $*
do
	pack=`basename $file .xml`
	echo "   $pack = __casac__.${pack}.${pack}"
done
