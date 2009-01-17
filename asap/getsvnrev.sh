#! /bin/bash

# Store info about which svn branch, what revision and at what date/time
# we executed make.

# if we don't see the .svn directory, just return
if test ! -d .svn; then
   exit 0;
fi


OUT=python/svninfo.txt

INFOTOP=`svn info | awk '/Last Changed Rev:/ { print $4} /URL:/ { print $2 }'`
SRCVER=`svn info src/.svn | awk '/Last Changed Rev:/ { print $4}'`
PYVER=`svn info python/.svn | awk '/Last Changed Rev:/ { print $4}'`

#HTTPS="https://svn.cv.nrao.edu/svn/casa"
HTTPS="https://svn.atnf.csiro.au/asap/branches/alma"
for i in $INFOTOP; do
   echo $i
   if test ${i:0:4} = "http"; then
      if test ${i:0:5} = "https"; then
         echo ${i#${HTTPS}} > $OUT
      fi
   else
      if test $SRCVER -gt $i; then
         echo $SRCVER >> $OUT
      elif test $PYVER -gt $i; then
         echo $PYVER >> $OUT
      else 
         #echo $i | tee -a $OUT
         echo $i >> $OUT
      fi
   fi
done

date "+%b %d %Y, %H:%M:%S" >> $OUT

exit 0

