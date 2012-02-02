#!/usr/local/gnu/bin/bash

cd /u/mur339/aips++/code/atnf/scripts/

d=`date +%Y-%m-%d-%H-%M`
filename=`echo $d'-cron-results'`
cat /dev/null > $filename
echo ATCA test results `date` >> $filename
glish runAssay.g >> $filename
grep ATCA $filename

grep "##" $filename > $filename.log
