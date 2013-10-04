#!/bin/bash
OUTFILE=__init__.py
echo "__name__ = '__casac__'"
echo '__all__ = ['
for file in $*
do
	pack=`basename $file .xml`
	echo "     '$pack',"
done
echo ']'
for file in $*
do
	pack=`basename $file .xml`
	echo "import $pack"
done
