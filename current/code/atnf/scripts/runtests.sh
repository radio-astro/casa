#!/usr/local/gnu/bin/bash

d=`date +%Y-%m-%d-%H-%M`
filename=`echo $d'-results'`
cat /dev/null > $filename
echo ATCA test results `date` >> $filename
glish runAssay.g >> $filename
grep ATCA $filename

grep "##" $filename > $filename.log
