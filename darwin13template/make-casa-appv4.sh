#! /bin/bash


if [ -z $1 ]; then
	echo "usage: make-casa-appv2 BUILD_NAME"
	echo "   where BUILD_NAME is one of "
	find /opt/casa -maxdepth 1 -mindepth 1 -type d | sed 's,/opt/casa/,          ,' | grep -v data
	exit 1
fi

prefix=/opt/casa/$1
SVNVERSION=$2



tmpname=casa-tmp-$(date +%s)

cd $(dirname $0)
workdir=$(pwd)/${tmpname}-staging
cd -
echo $workdir

mkdir $workdir
mkdir ${workdir}/CASA.app

cd canonical
tar cpf - CASA.app |  (cd ${workdir}; tar xpf - )

# copy canonical symlink references
cd ${workdir}/CASA.app
for i in $(find Contents -type l); do
	j=$(stat -f %Y $i)
	echo $j | grep -q @@PREFIX@@ && {
		k=$(echo $j | sed -e "s,@@PREFIX@@,$prefix,");
		echo $i >> /tmp/${tmpname}-symlinks;
		rm $i
		if [ -e $k ]; then
	  	   echo ditto -V $k $i
	  	   ditto -V $k $i
	        else 
		   k=$(echo $j | sed -e "s,@@PREFIX@@/core,$prefix/$SVNVERSION,");
		   echo ditto -V $k $i
		   ditto -V $k $i
	        fi
	};
done

version=`cut -d '$' -f 1 Contents/Resources/VERSION | awk -F '.' '{printf "%2.1f.%d\n", $1/10, $2}'`
echo version=`cut -d '$' -f 1 Contents/Resources/VERSION | awk -F '.' '{printf "%2.1f.%d", $1/10, $2}'`
echo $version

# copy all CASA dynamic libs
cd ${workdir}/CASA.app/Contents/Frameworks

for i in ${prefix}/${SVNVERSION}/darwin/lib/*.*.dylib ${prefix}/${SVNVERSION}/darwin/lib/lib{air_casawvr,atnf,asap,pyrap,mir,wcs}.dylib ${prefix}/${SVNVERSION}/darwin/lib/*_f.dylib ${prefix}/${SVNVERSION}/darwin/lib/libaatm.0.dylib ${prefix}/lib/libqwt.5.dylib  ${prefix}/${SVNVERSION}/darwin/lib/libcasa_*.dylib /usr/lib/libxslt.1.dylib
do
	dylib=$(basename $i)
	echo $dylib >> /tmp/${tmpname}-casalibs
	ditto -V $i $dylib
done


find ${workdir}/CASA.app/Contents -name '*.pyc' -o -name '*.pyo' -print0 | xargs -0 rm


## compile the pythonw wrapper
# cd ${workdir}/..
# ${prefix}/3rd-party/bin/gcc pythonw.c -o ${workdir}/CASA.app/Contents/MacOS/pythonw


######################
# replace library absolute-path references with path-relative references
cd ${workdir}/CASA.app/Contents

tplibd=${prefix}/lib
cdlibd=/opt/casa/${SVNVERSION}/darwinllvm/lib
fortlibd=/usr/local/gfortran/lib
framelibd=${prefix}/Library

echo tplibd = $tplibd 
echo cdlibd = $cdlibd

for i in $(find * -type f ! -name '*.py?' -a ! -name '*.py' | xargs file | grep "x86_64" | grep -v "x86_64.egg" | cut -d ':' -f 1 | cut -d ' ' -f 1)
do
   echo file = $i
   chmod +w $i

   dylib=$(basename $i)
   dpath=$(dirname  $i)
   ddir=$(basename ${dpath})
  # echo dylib = $dylib
  echo dpath = $dpath
  # echo ddir = $ddir
   if   [[ ${ddir} == "gcc42" ]]; then
      # gcc42 subdir has some special cases
      echo "process gcc42 later, skipping..."
      echo
      continue
   elif [[ ${ddir} == "Frameworks" ]]
    then
      echo install_name_tool -id $(basename $i) $i
           #install_name_tool -id $(basename $i) $i
      #drel="@loader_path"
      echo drel="@loader_path/$(echo $dpath | sed -e 's,[^/]*,..,g')/Frameworks"
      drel="@loader_path/$(echo $dpath | sed -e 's,[^/]*,..,g')/Frameworks"
      install_name_tool -id $drel/$(basename $i) $i
    else
      echo drel="@loader_path/$(echo $dpath | sed -e 's,[^/]*,..,g')/Frameworks"
      drel="@loader_path/$(echo $dpath | sed -e 's,[^/]*,..,g')/Frameworks"
      echo install_name_tool -id $drel/$i $i
      install_name_tool -id $drel/$i $i
    fi
  
  #  echo drel = $drel
   for j in $( otool -L $i | grep -v ${dylib} | grep $tplibd | sed -e 's, (.*,,' -e "s,${tplibd},," )
    do
	    # echo $j
     echo install_name_tool -change ${tplibd}$j  ${drel}$j $i
          install_name_tool -change ${tplibd}$j  ${drel}$j $i
    done

   for j in $( otool -L $i | grep -v ${dylib} | grep $cdlibd | sed -e 's, (.*,,' -e "s,${cdlibd},," )
    do
	    # echo $j
     echo install_name_tool -change ${cdlibd}$j  ${drel}$j $i
          install_name_tool -change ${cdlibd}$j  ${drel}$j $i
    done

   for j in $( otool -L $i | grep -v ${dylib} | grep  gfortran | sed -e 's, (.*,,' -e "s,${fortlibd},," )
    do
     echo install_name_tool -change ${fortlibd}$j  ${drel}$j $i
          install_name_tool -change ${fortlibd}$j  ${drel}$j $i
    done

   for j in $( otool -L $i | grep -v ${dylib} | grep  "casa.*Versions" | sed -e 's, (.*,,' -e "s,${framelibd},," )
    do
     echo install_name_tool -change ${framelibd}$j  ${drel}$j $i
          install_name_tool -change ${framelibd}$j  ${drel}$j $i
    done

   for j in $( otool -L $i | grep -v ${dylib} | grep  "Qt.*Versions" | sed -e 's, (.*,,' -e "s,${framelibd},," )
    do
     echo install_name_tool -change $j  ${drel}/$j $i
          install_name_tool -change $j  ${drel}/$j $i
    done

   for j in $(cat /tmp/${tmpname}-casalibs); do
	   # echo j = $j
	   # echo dylib = $dylib
	   # echo tplibd = $tplibd
	   # echo cdlibd = $cdlibd
    for k in $( otool -L $i | grep -v ${dylib} | grep -v "@loader_path" | grep $j     | sed -e 's, (.*,,' -e "s,${tplibd},," -e "s,${cdlibd}/,," )
     do
	     # echo k = $k
      echo install_name_tool -change $k ${drel}/$k $i
           install_name_tool -change $k ${drel}/$k $i
     done
    done
done


cd ${workdir}/CASA.app/Contents/Frameworks
cp /usr/local/gfortran/lib/libgfortran*.dylib .
cp /usr/local/gfortran/lib/libquadmath*.dylib .
cp /usr/local/gfortran/lib/libgcc_s*.dylib .

install_name_tool -id @loader_pat/../Frameworks/libgfortran.3.dylib libgfortran.3.dylib
install_name_tool -change /usr/local/gfortran/lib/libquadmath.0.dylib @loader_path/../Frameworks/libquandmath.0.dylib libgfortrans.3.dylib
install_name_tool -change /usr/local/gfortran/lib/libgcc_s.1.dylib @loader_path/../Frameworks/libgcc_s.1.dylib libgfortrans.3.dylib

install_name_tool -id @loader_pat/../Frameworks/libquadmath.0.dylib libquandmath.0.dylib
install_name_tool -change /usr/local/gfortran/lib/libgcc_s.1.dylib @loader_path/../Frameworks/libgcc_s.1.dylib libquadmath.1.dylib

install_name_tool -id @loader_pat/../Frameworks/libgcc_s.1.dylib libgcc_s.1.dylib



cd ${workdir}/CASA.app/Contents/MacOS
sed -i .bak -e 's,share/xml,Resources/xml,g' *.sh
rm *.bak

#### THE CASA PYTHON DIRECTORY
cd ${workdir}/CASA.app/Contents/Resources/python

sed -i .bak -e 's,share/xml,Resources/xml,g' *.py
rm *.bak

patch << EOF
--- /opt/casa/merge/darwin/python/2.6/casapy.py	2009-06-05 22:05:38.000000000 -0600
+++ casapy.py	2009-06-06 15:56:17.000000000 -0600
@@ -1,4 +1,9 @@
 #
+try:
+   import IPython
+except ImportError, e: 
+   print 'Failed to load IPython: ', e
+   exit(1)
 
 try:
     import casac
 ###################
 #setup file catalog
EOF

#### CASA DATA REPOSITORY

cd ${workdir}/CASA.app/Contents/Resources
mkdir casa-data
cd casa-data
#svn export --force https://svn.cv.nrao.edu/svn/casa-data/distro .

###########################



echo "Changing ownership and permissions to nobody:staff"
set -v

cd ${workdir}
sudo chmod g-s CASA.app/Contents/Resources/xml
sudo chown -R nobody:staff .
for i in $(find . -type l)
	do sudo chown -h nobody:staff $i
done

#
# Clean out Subversion cruft in CASA.app
find CASA.app -name "\.svn" -exec rm -fr {} \;

pwd
echo "Moving CASA.app to ../CASA"
rm -rf ../CASA/CASA.app
mv CASA.app ../CASA
cd ..

sudo rmdir ${workdir}
echo "Now making the dmg file"
sudo rm -f c[1,2].dmg
sudo ./make-casa-dmg.sh
#sudo rm -rf CASA/CASA.app
#echo "move CASA.app into /opt/casa/${1}/template/CASA and make a disk image"
