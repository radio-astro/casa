#!/bin/bash
OUTFILE=casac.py
echo "import __casac__"
echo "class casac : " 
echo "   __file__ = __casac__.__file__"
for file in $*
do
	pack=`basename $file .xml`
	echo "   $pack = __casac__.${pack}.${pack}"
done
	echo "   Quantity = 'set to quantity'"
