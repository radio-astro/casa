#!/bin/sh
#Routine for building a setup.py file
COMPONENT="paramgui"
AIPSROOT=`echo $CASAPATH | awk '{print $1}'`
ARCHLIB=`echo $CASAPATH | awk '{printf "%s/%s/lib", $1,$2}'`
ARCH=`echo $CASAPATH | awk '{print $2}'`
SITE=`echo $CASAPATH | awk '{print $3}'`
MAKEDEFS=$AIPSROOT/$ARCH/$SITE/makedefs
VARS="CPPSTD PYTHONLIBD PYTHONVER CCMTOOLSLIBD CCMTOOLSINCD CORELIBD COREINCD QT4LIBD QT4LIB QT4INCD CFITSIOLIBD CFITSIOINCD XTRNLIBS_rpath"
eval `gmake -f $AIPSROOT/$ARCH/makedefs VARS="$VARS" eval_vars`
DEFINES2=`for i in $CPPSTD; do echo $i | grep "\-D"; done`
DEFINES2="$DEFINES2 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE_64_SOURCE"
COMMA=""

CFITSIO_LIBPATH=""
if [ "$CFITSIOLIBD" != "" ]
then
    CFITSIO_LIBPATH=", '$CFITSIOLIBD'"
fi

# Load makedefs variables into the environment if necessary.
  echo "#$SETUPLIBS#$SETUPPYTHONLIB#SETUPEXTRALINK" | grep '##' > /dev/null 2>&1
  if [ "$?" = 0 ]
  then
     VARS="SETUPLIBS SETUPPYTHONLIB SETUPEXTRALINK"
     eval `gmake -f $AIPSROOT/$ARCH/makedefs VARS="$VARS" eval_vars`
  fi
#
# OK check again if they setup variables are not set then set them to a sensible default
#
  echo "#$SETUPLIBS#$SETUPEXTRALINK#" | grep '##' > /dev/null 2>&1
  if [ "$?" = 0 ]
  then
     if [ "$SETUPEXTRALINK" = "" ]; then
        if [ "$XTRNLIBS_rpath" = "" ]; then
            SETUPEXTRALINK="'-Xlinker', '-rpath', '-Xlinker', '$PYTHONLIBD', '$CCMTOOLSLIBD/libCCM_Local_HomeFinder.a', '$CCMTOOLSLIBD/libWX_Utils_types.a'"
        else
            SETUPEXTRALINK="'$XTRNLIBS_rpath', '$CCMTOOLSLIBD/libCCM_Local_HomeFinder.a', '$CCMTOOLSLIBD/libWX_Utils_types.a'"
        fi
     fi
  fi

# cd $AIPSROOT/gcode_$ARCH/Python_Converter
theFiles=`ls *.cc`
echo "from distutils.core import setup, Extension" > setup.py
echo "setup( name=\"$COMPONENT\", version=\"1.0\"," >> setup.py
echo "      ext_modules=[Extension(\"$COMPONENT\"," >> setup.py
echo "               [" >>setup.py
for afile in $theFiles
do
echo "            $COMMA'$afile'" >> setup.py
COMMA=", "
done
echo "               ]," >> setup.py
if [ "$SETUPLIBS" != "" ]; then
   if [ "$QT4LIBD" != "" ]; then
      echo "               library_dirs=['$ARCHLIB', $SETUPLIBS, '$PYTHONLIBD' $CFITSIO_LIBPATH, '$QT4LIBD']," >> setup.py
   else
      echo "               library_dirs=['$ARCHLIB', $SETUPLIBS, '$PYTHONLIBD' $CFITSIO_LIBPATH]," >> setup.py
   fi
else
   if [ "$QT4LIBD" != "" ]; then
      echo "               library_dirs=['$ARCHLIB', '$PYTHONLIBD' $CFITSIO_LIBPATH, '$QT4LIBD']," >> setup.py
   else
      echo "               library_dirs=['$ARCHLIB', '$PYTHONLIBD' $CFITSIO_LIBPATH]," >> setup.py
   fi
fi
echo "               libraries=[ 'nrao', 'xmlcasa', 'display', 'flagging', 'calibration', 'msvis', 'casa_msfits', 'casa_ms', 'casa_images', 'casa_components', 'casa_coordinates'," >> setup.py
echo "                           'casa_lattices', 'casa_fits', 'casa_measures', 'casa_tables', 'casa_scimath', 'synthesis', 'graphics', 'casa_casa','casaqt','qwt'," >> setup.py
#
#If it's a flavour of darwin the skip lapack and blas
tjunk=`echo "$ARCH" | grep "^darwin"`
if [ "$tjunk" ]; then
   echo "                           'casa_scimath_f', 'casa_mirlib', 'cfitsio', " >> setup.py
else
   echo "                           'casa_scimath_f', 'lapack', 'blas', 'casa_mirlib', 'cfitsio', " >> setup.py
   if [ "$QT4LIB" != "" ]; then
   qtlibs=''
   for i in `echo $QT4LIB | sed 's/-l//g'`; do
       qtlibs="$qtlibs '$i',";
   done
   echo "                          $qtlibs" >> setup.py
   fi
fi
if [ "$SETUPPYTHONLIB" != "" ]; then
echo "$SETUPPYTHONLIB," >> setup.py
fi
echo "                           'CCM_Python', 'CCM_Local_HomeFinder', 'WX_Utils_error', 'WX_Utils_code', 'WX_Utils_types', 'c', 'm' ]," >> setup.py
echo "               extra_compile_args = [" >> setup.py
for i in $DEFINES2
do
echo                                       \'$i\', >> setup.py
done
echo "                                     ]," >> setup.py
if [ "$SETUPEXTRALINK" != "" ]; then
echo "               extra_link_args = [$SETUPEXTRALINK])], " >> setup.py
fi

echo "       include_dirs = ['$AIPSROOT/code/include', '$AIPSROOT/code/casa', '$COREINCD', '..', '$CCMTOOLSINCD', '$CFITSIOINCD', '../impl'," >> setup.py
if [ "$QT4INCD" != "" ]; then
   for i in $QT4INCD
   do
      echo "                     '$i'," >> setup.py
   done
fi
echo "                       '.' ])" >> setup.py
