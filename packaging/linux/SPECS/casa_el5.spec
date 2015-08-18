####
#### (rpmbuild -ba -D 'revision 30871' casa422.spec > log.casa422.110 2>&1 &)&
####
%define DISTRO %([ -f /etc/redhat-release ] && sed -e "s/\\(.\\+\\)\\( Linux release \\)\\(.\\+\\)\\( .\\+\\)/\\1 \\3/" /etc/redhat-release)
%define DISTRO_REL %([ -f /etc/redhat-release ] && sed -e "s/\\(.\\+ release \\)\\(.\\+\\)\\( .\\+\\)/\\2/" /etc/redhat-release)
%define REDHAT %(( [ -f /etc/redhat-release ] && head -1 /etc/redhat-release ) || ( [ -f /etc/SuSE-release ] && head -1 /etc/SuSE-release ) || ( [ -f /etc/mandrake-release ] && head -1 /etc/mandrake-release ))
%define finddata /usr/src/rpmbuild/build/SOURCES/casa%{?SVNTAG:-%{SVNTAG}}/find-data-version

%if %{?rel:1}0
%define relcount      %{rel}
%else
%define relcount      1
%endif

%define SVNROOT       https://svn.cv.nrao.edu/svn/casa/branches/release-4_5
###
### configure version and revision...
%define ver           4.5.0
%define rev           %(/usr/src/rpmbuild/build/SOURCES/%{name}/svn-revision branch=release-4_5)
%if %{?ver:1}0
%define CASAVER       %{ver}
%else
%define CASAVER       %(/usr/src/rpmbuild/build/SOURCES/%{name}/casa-version --ver)
%endif

%if %{?rev:1}0
%define CASAREV       %{rev}
%else
%define CASAREV       %(/usr/src/rpmbuild/build/SOURCES/%{name}/casa-version --rev)
%endif

%define pybasever %(rpm -q --queryformat '%{VERSION}' casa01-python | sed -e 's/.[0-9][0-9]*$//')
%define pythondep %{pybasever}.5-05%{?dist}
%define dataver %(%{finddata})

%define arch %(uname -i)

%define prefix /usr
%define topdir %{prefix}/%{_lib}/casa
%define rootdir %{topdir}/%{CASAVER}R
%define incdir %{rootdir}/include
%define libdir %{rootdir}/lib
%define bindir %{rootdir}/bin
%define plibdir %{rootdir}/lib
%define pylibdir %{plibdir}/python%{pybasever}
%define pgplotdir %{rootdir}/pgplot
%define thirdparty 01

Vendor:        NRAO
Name:          casa-prerelease
Release:       %{relcount}%{?dist}
Packager:      Darrell Schiebel <drs@nrao.edu>
Prefix:        %{_prefix}

License:       GNU LESSER GENERAL PUBLIC LICENSE (LGPL)
Group:         Applications
Version:       %{CASAVER}
Autoreqprov:   on
Summary:       The casa/aips++ public executables (built on RH%{DISTRO_REL}).
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Source1:       grexec.f
Source2:       asap-2_1.patch
Source7:       find-data-version
Source20:      casa-version
Source22:      replace-version
Source23:      svn-revision
Source50:      find-templates
Source60:      fixbrowser
Source85:      ccm-mods.tar.gz
Source90:      saxon8.jar
Source100:     casa

Patch1:        casapy.install.patch

BuildRequires: subversion
BuildRequires: tar
BuildRequires: casa01-boost-devel

Requires:      casa01-python >= %{pybasever}.5-05%{?dist}
Requires:      %{name}-bin = %{version}-%{release}
Conflicts:     casa, casa-stable

#Obsoletes:     casapy
#Obsoletes:     casapy-launchers
Conflicts:     casapy
Conflicts:     casapy-launchers


%description
CASA/AIPS++ user environment

%package bin
Summary:       The casa/aips++ executable and shared library binaries (built on RH%{DISTRO_REL}).
Group:         Applications
Version:       %{CASAVER}
Autoreqprov:   on
Requires:      %{name}-shared = %{version}-%{release}
Requires:      casa01-python >= %{pythondep}

#Obsoletes:     casapy-bin
Conflicts:     casapy-bin

%description bin
CASA/AIPS++ development environment

%package shared
Summary:      The casa/aips++ shared libraries (built on RH%{DISTRO_REL}).
Group:        Applications
Version:      %{CASAVER}
Autoreqprov:  on
Requires:     casa01-swig
Requires:     casapy-data >= %{dataver}
Requires:     casa01-qt, casa01-qt-x11, casa01-qwt
##
## with 0.06-05 we switched to -laatm from -lATM...
##
Requires:     aatm >= 0.06-05%{?dist}

###
###  with 1.33.1-7 we rebuilt with 2 byte unicode for ACS
###  otherwise asap gets the following error:
###
###     CASA <2>: asap_init()
###     *** Loading ATNF ASAP Package...
###     ---------------------------------------------------------------------------
###     ImportError                               Traceback (most recent call last)
###
###     /usr/lib64/casapy/test/lib/python2.5/casapy.py in <module>()
###     ----> 1
###           2
###           3
###           4
###           5
###
###     /usr/lib64/casapy/test/lib/python2.5/asap_init.py in asap_init()
###          15     casapath=os.environ['CASAPATH']
###          16     print '*** Loading ATNF ASAP Package...'
###     ---> 17     import asap as sd
###          18     print '*** ... ASAP (%s rev#%s) import complete ***' % (sd.__version__,sd.__revision__)
###          19     os.environ['CASAPATH']=casapath
###
###     /usr/lib64/casapy/test/lib/python2.5/asap/__init__.py in <module>()
###          11 from asap.parameters import *
###          12 # third import
###     ---> 13 from asap.logging import *
###          14 from asap.utils import *
###          15 # explicitly import 'hidden' functions
###
###     /export/data/casa/dschieb/casa/testing/asap/logging.py in <module>()
###           6
###           7
###     ----> 8
###           9
###          10
###
###     ImportError: /usr/lib64/libboost_python.so.2: undefined symbol: PyUnicodeUCS4_FromEncodedObject
###
###     CASA <3>:
###

Requires:     boost

#Obsoletes:     casapy-shared
Conflicts:     casapy-shared

%description shared
CASA/AIPS++ development environment

%package devel
Summary:       The casa/aips++ shared libraries (built on RH%{DISTRO_REL}).
Group:         Applications
Version:       %{CASAVER}
Autoreqprov:   on
Requires:      %{name}-bin = %{version}-%{release}
Requires:      casa01-swig
Requires:      cfitsio-devel
Requires:      casa01-qt-devel, casa01-qwt-devel
Requires:      casa01-dbus-cpp, casa01-dbus-cpp-devel
Requires:      xerces-c28-devel, rpfits, aatm-devel, blas, lapack
Requires:      casa01-swig
Requires:      tix-devel, tk-devel
Requires:      casa01-python-devel >= %{pybasever}.5-05%{?dist}
Requires:      fftw3-devel
### this assumes that regular developers will not want to build their own pgplot
Requires:      pgplot-devel
###
Requires:      wcslib-devel >= 4.7-1%{?dist}
Requires:      casa01-boost-devel

#Obsoletes:     casapy-devel
Conflicts:     casapy-devel


%description devel
CASA/AIPS++ development environment

%prep
echo "revision:      %{CASAREV}"
echo "version:       %{CASAVER}"
echo "data version:  %{dataver}"
echo %{name} %{version} %{release}
echo %{SVNROOT}
echo '=========================================================================================='
date
echo '=========================================================================================='

rm -rf casa-*

%setup -c -T -n %{name}-%{version}
top=`pwd`

echo "update time:" `date -u`
(cd .. && svn co -r %{CASAREV} %{SVNROOT} %{name}-%{version})

%patch1
# TODO: SAR: Delete?
# %patch2

pushd code/graphics
svn co --username rpm.build https://svn.cv.nrao.edu/svn/pgplot
popd

echo "setting revision state: %{CASAVER} %{CASAREV} %{SVNROOT}"
%{SOURCE22} -v ver=%{CASAVER} rev=%{CASAREV} url=%{SVNROOT} .

#(cd asap && patch -p0 < %{SOURCE2})
#sed -i -e "s@__BUILD_ROOT__@`pwd`@g" asap-*/Makefile

##
## path to Boyd's assignment filter (hack) doesn't get set properly
##
sed -i -e "s#^fullpath=.*#fullpath='%{pylibdir}/assignmentFilter.py'#g" gcwrap/python/scripts/casapy.py

%build

top=`pwd`

##
## sourcing casainit.sh results in:
##
##	error: Bad exit status from /var/tmp/rpm-tmp.17501 (% build)
##
## part way through casainit...
##
export CASAPATH="`pwd` linux"
export PATH=`pwd`/linux/bin:%{bindir}:%{topdir}/%{thirdparty}/bin:/usr/lib64/casa/%{thirdparty}/bin:$PATH
export LD_LIBRARY_PATH=`pwd`/linux/lib:%{libdir}:%{topdir}/%{thirdparty}/lib:$LD_LIBRARY_PATH

echo "build time:" `date -u`

export SVNGEN_URL=%{SVNROOT}
export SVNGEN_REVISION=%{CASAREV}

#Replace casacore from github
#rm -rf $top/casacore
#cd $top
#git clone https://github.com/casacore/casacore

export COMPILERS="-DCXX11=1 -DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-2/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-2/root/usr/bin/gcc -DCMAKE_Fortran_COMPILER=/opt/rh/devtoolset-2/root/usr/bin/gfortran"

cd $top/casacore
mkdir build && cd build
#Changed -DBUILD_CASA=1 to -DCASA_BUILD=1
cmake $COMPILERS -DCASA_BUILD=1 -DBUILD_TESTING=OFF '-DCMAKE_INSTALL_PREFIX=../../linux' -DBUILD_PYTHON=1 -DPYTHON_INCLUDE_DIR=/usr/lib64/casa/01/include/python2.7 -DPYTHON_LIBRARY=/usr/lib64/casa/01/lib/libpython2.7.so -DBOOST_ROOT=/usr/lib64/casa/01 ..
gmake 'VERBOSE=1' install

cd $top/code
mkdir build
cd build
cmake $COMPILERS -DEXTRA_C_FLAGS="-DPG_PPU -I/usr/include/wcslib-4.3" -Darch=linux -DSKIP_PGPLOT="yes" -DPYTHONLIBD=%{pylibdir} -DPYTHONTASKD=%{pylibdir} ..
gmake 'VERBOSE=1'

cd $top/gcwrap
mkdir build && cd build
cmake $COMPILERS -Darch=linux -DPYTHONLIBD=%{pylibdir} ..
gmake 'VERBOSE=1'

cd $top/asap
mkdir build && cd build
cmake $COMPILERS -DEXTRA_C_FLAGS="-I/usr/include/wcslib-4.3" -Darch=linux -DPYTHONLIBD=%{pylibdir} -DPYTHONTASKD=%{pylibdir} ..
gmake 'VERBOSE=1'

cd $top
%{SOURCE60}

%install

top=`pwd`
##
## left over cruft
##
rm -f code/include/pgxwin.h
rm -f code/include/wcsconfig_f77.h

export AIPSPATH="`pwd` linux"
export CASAPATH="`pwd` linux"
export PATH=`pwd`/linux/bin:%{topdir}/%{thirdparty}/bin:/usr/lib64/casa/%{thirdparty}/bin:$PATH

if [ -e /usr/%{_lib}/qt-4.8.5/dbus/%{_lib} ]; then
    export LD_LIBRARY_PATH=`pwd`/linux/lib:/usr/%{_lib}/qt-4.8.5/dbus/%{_lib}:$LD_LIBRARY_PATH
else
    export LD_LIBRARY_PATH=`pwd`/linux/lib:$LD_LIBRARY_PATH
fi

##
## viewer apparently always needs the data...
##
svn co https://svn.cv.nrao.edu/svn/casa-data/distro data

if test -e linux/bin/casa && echo "" | (source ./casainit.sh; linux/bin/casa --debug-shell-script --nogui); then
    echo "casapy built ok:"
    echo "------------------------------------------------------------"
    pushd linux/bin
    echo `ls -l casa`
    popd
    echo "------------------------------------------------------------"
else
    if test -e linux/bin/casa; then
	echo "------------------------------------------------------------"
	echo "casapy built, but doesn't run"
	echo "------------------------------------------------------------"
    else
	echo "------------------------------------------------------------"
	echo "casapy failed to build"
	echo "------------------------------------------------------------"
    fi
    exit 1
fi

echo "install time:" `date -u`

rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{rootdir}
mkdir -p $RPM_BUILD_ROOT%{bindir}
mkdir -p $RPM_BUILD_ROOT%{plibdir}
mkdir -p $RPM_BUILD_ROOT%{pylibdir}
mkdir -p $RPM_BUILD_ROOT%{incdir}/casacode
mkdir -p $RPM_BUILD_ROOT%{pgplotdir}

cp %{SOURCE90} $RPM_BUILD_ROOT%{rootdir}/lib

cat > $RPM_BUILD_ROOT%{rootdir}/.revstate<<EOF
install="rpm"
version="%{version}"
revision="%{CASAREV}"
url="%{SVNROOT}"
EOF

##
## parameter checking, XML files, which are read at runtime
##
mkdir -p $RPM_BUILD_ROOT%{rootdir}/xml
cp linux/xml/* $RPM_BUILD_ROOT%{rootdir}/xml
#
# Quick and dirty work around for CAS-5373.
mkdir $RPM_BUILD_ROOT%{rootdir}/linux
pushd $RPM_BUILD_ROOT%{rootdir}/linux
ln -s ../xml
popd

mkdir -p $RPM_BUILD_ROOT%{pylibdir}/asap/data
cp linux/share/asap/ipythonrc-asap $RPM_BUILD_ROOT%{pylibdir}/asap/data

cd $top/linux/bin
##
## install binaries & scripts
##
tar cf - . | (cd $RPM_BUILD_ROOT%{bindir}; tar xf -)
rm -f $RPM_BUILD_ROOT%{bindir}/casainfo
rm -f $RPM_BUILD_ROOT%{bindir}/casapyinfo

cd $top/linux/lib
tar cf - . | tar -C $RPM_BUILD_ROOT%{libdir} -xf -
cd ../pgplot
cp grfont.dat rgb.txt pgplot.inc cpgplot.h grpckg1.inc $RPM_BUILD_ROOT%{pgplotdir}

##
## install header files & template source files
##
cd $top
## package ALL header files
pushd linux/include
tar --ignore-failed-read -cf - `find . -follow -type f -name '*.h'` | (cd $RPM_BUILD_ROOT%{incdir}; tar xf -)
popd
## package up only templates *.cc files
pushd casacore
%{SOURCE50} > template-files.txt
tar --ignore-failed-read -cf - --files-from template-files.txt | (cd $RPM_BUILD_ROOT%{incdir}/casacore; tar xf -)
popd
pushd code
%{SOURCE50} > template-files.txt
mkdir -p $RPM_BUILD_ROOT%{incdir}/casacode
tar --ignore-failed-read -cf - --files-from template-files.txt | (cd $RPM_BUILD_ROOT%{incdir}/casacode; tar xf -)
popd

cd $top

## depends upon perl...
rm -f $RPM_BUILD_ROOT%{bindir}/gbtlogviewutil.pl
## depends upon glish...
rm -f $RPM_BUILD_ROOT%{libdir}/gDisplay.so
rm -f $RPM_BUILD_ROOT%{bindir}/tkViewer
rm -f $RPM_BUILD_ROOT%{bindir}/gencolormaptable
rm -f $RPM_BUILD_ROOT%{bindir}/gbtobsparser
rm -f $RPM_BUILD_ROOT%{bindir}/iramcalibrater
rm -f $RPM_BUILD_ROOT%{bindir}/pksmscaltoimage

(cd $RPM_BUILD_ROOT%{bindir}; chmod 755 `find . -type f`)
(cd $RPM_BUILD_ROOT%{plibdir}; chmod 755 *.so.* *.so)
(cd $RPM_BUILD_ROOT%{libdir}; chmod 755 *.so.*)

cp -p linux/makedefs $RPM_BUILD_ROOT%{rootdir}
cp -p code/VERSION $RPM_BUILD_ROOT%{rootdir}

##
## copy over the python code
##
(cd linux/python/%{pybasever} && tar --dereference -cf - `find . -type f -o -type l -name '*.py' | grep -v /asap/`) | tar -C $RPM_BUILD_ROOT%{pylibdir} -xf -
(cd linux/python/%{pybasever} && tar -cf - `find . -type f -name '*.so' | grep -v /asap/`) | tar -C $RPM_BUILD_ROOT%{pylibdir} -xf -
(cd linux/python/%{pybasever} && tar -cf - `find . -type f -name '*.pl' | grep -v /asap/`) | tar -C $RPM_BUILD_ROOT%{pylibdir} -xf -
(cd linux/python/%{pybasever} && tar -cf - `find . -type f -name '*.sh' | grep -v /asap/`) | tar -C $RPM_BUILD_ROOT%{pylibdir} -xf -
(cd linux/python/%{pybasever} && tar -cf - `find . -type f -name '*.txt' | grep -v /asap/`) | tar -C $RPM_BUILD_ROOT%{pylibdir} -xf -
tar -C linux/python/%{pybasever} -c -f - asap | tar -C $RPM_BUILD_ROOT%{pylibdir} -x -f -

##
## substitute the proper path(s)
##
(cd $RPM_BUILD_ROOT%{pylibdir} && sed -i -e 's|^task_directory.*|task_directory = "%{pylibdir}"|' casadef.py)
(cd $RPM_BUILD_ROOT%{pylibdir} && sed -i -e s,pathname=.*,pathname=\'file:///\'+os.environ.get\(\'CASAPATH\'\).split\(\)[0]+\'/xml/\',g *.py)

##
## byte compile the python code
##
(cd $RPM_BUILD_ROOT%{pylibdir} && python -c 'import compileall; compileall.compile_dir(".", force=1)')

##
## strip libraries
##
( cd $RPM_BUILD_ROOT%{libdir};
  for i in *.so.*; do
      strip $i
  done
)

##
## clean up wes' casabrowser crap
##
( cd $RPM_BUILD_ROOT%{bindir};
  if test -e qcasabrowser -a -e casabrowser; then
    /bin/rm -f casabrowser
    mv qcasabrowser casabrowser
  fi
)

##
## strip binaries
##
( cd $RPM_BUILD_ROOT%{bindir};
  for i in `find . -type f`; do
      if test '!' -h $i; then
          if file $i 2> /dev/null | grep ELF > /dev/null; then
              strip $i
          fi
      fi
  done
)

####################### casa wrappers #############
mkdir -p $RPM_BUILD_ROOT%{prefix}/bin
cp -p %{SOURCE100} $RPM_BUILD_ROOT%{prefix}/bin
pushd $RPM_BUILD_ROOT%{prefix}/bin
for i in asdm2MS casabrowser casa-config casafeather casalogger casaplotms casaviewer casapy; do
    ln -s casa $i
done
popd

####################### .aipsrc ###################
cat > $RPM_BUILD_ROOT%{rootdir}/.aipsrc<<EOF
system.aipscenter:      namerica
printer*default:
printer*paper:          letter
help.server:            http://aips2.nrao.edu/
help.directory:         /aips++/stable/
help.popup.type:        hoverlong
inhale.cvsroot:         :pserver:anonymous@cvs.cv.nrao.edu:/home/cvs
inhale.sneeze.*.rcmd:   ssh
inhale.base.*.preserve: false
sneeze.base.*.preserve: false
EOF

cat > $RPM_BUILD_ROOT%{bindir}/casapyinfo<<EOF
#!/bin/sh
# Generated by rpm from SPECS/casapy.spec.  Do not edit.

VERSION="%{CASAVER}"
SVNVERSION="%{CASAREV}"
HOST="%{REDHAT}"

usage() {
    echo "casapyinfo [--help|--data|--keys|--version|--rpmversion|--environ|--makedefs]"
    echo "    --help:        usage info"
    echo "    --data:        location of the data repository"
    echo "    --keys:        location of the glish keys"
    echo "    --version:     casa version info"
    echo "    --rpmversion:  casa version info"
    echo "    --svnroot:     svn url for source"
    echo "    --svnversion:  svn version number for source"
    echo "    --buildhost:   build host info"
    echo "    --environ:     show support environment (this is automatically"
    echo "                   set up for standard binaries)"
    echo "    --makedefs:    dump build makedefs"
    echo "    --exec STRING: export casa environment and then exec STRING"
}

if [ "\$#" -eq 0 ]; then
    usage
fi

exec_str=""

while [ "\$#" -gt 0 ]; do
    case \$1 in
	--data)
		echo "%{rootdir}/data"
		shift;
		;;
	--keys)
		echo "%{rootdir}/keys"
		shift;
		;;
	--version)
		cat "%{rootdir}/VERSION" | perl -pe 's|^([0-9])([0-9])\.([0-9]+).*|\$1.\$2.\$3|'
		shift
		;;
	--rpmversion)
		echo "\$VERSION"
		shift
		;;
	--svnroot)
		echo "\$SVNROOT"
		shift
		;;
	--svnversion)
		echo "\$SVNVERSION"
		shift
		;;
	--buildhost)
		echo "\$HOST"
		shift
		;;
	--makedefs)
		cat "%{rootdir}/makedefs"
		shift
		;;
	--environ)
		if [ -e /usr/gcc/3.3/lib/libgcc_s.so -o -e /usr/gcc/3.3/lib/libg2c.so -o -e /usr/gcc/3.3/lib/libstdc++.so ]; then
		    if [ -e /usr/%{_lib}/qt-4.8.5/dbus/%{_lib} ]; then
			echo "LD_LIBRARY_PATH=%{rootdir}/lib:%{topdir}/%{thirdparty}/lib:/usr/%{_lib}/qt-4.8.5/dbus/%{_lib}:/%{_lib}:/lib:/usr/lib/%{_lib}:/usr/lib:\\\$LD_LIBRARY_PATH:/usr/gcc/3.3/lib"
		    else
			echo "LD_LIBRARY_PATH=%{rootdir}/lib:%{topdir}/%{thirdparty}/lib:/%{_lib}:/lib:/usr/lib/%{_lib}:/usr/lib:\\\$LD_LIBRARY_PATH:/usr/gcc/3.3/lib"
		    fi
		else
		    if [ -e /usr/%{_lib}/qt-4.8.5/dbus/%{_lib} ]; then
			echo "LD_LIBRARY_PATH=%{rootdir}/lib:%{topdir}/%{thirdparty}/lib:/usr/%{_lib}/qt-4.8.5/dbus/%{_lib}:/%{_lib}:/lib:/usr/%{_lib}:/usr/lib:\\\$LD_LIBRARY_PATH"
		    else
			echo "LD_LIBRARY_PATH=%{rootdir}/lib:%{topdir}/%{thirdparty}/lib:/%{_lib}:/lib:/usr/%{_lib}:/usr/lib:\\\$LD_LIBRARY_PATH"
		    fi
		fi
		echo "CASAPATH=\"%{rootdir} linux\""
		echo "PATH=%{bindir}:%{topdir}/%{thirdparty}/bin:/usr/bin:/usr/X11R6/bin:\\\$PATH"
		echo "PGPLOT_FONT=%{pgplotdir}/grfont.dat"
		echo "PGPLOT_RGB=%{pgplotdir}/rgb.txt"
		shift
		;;
	--exec)
 		shift
		exec_str=\$1
		shift
		;;
	--help)
		usage
		shift
		;;
    esac
done

if [ "\$exec_str" ]; then
    if [ -e /usr/gcc/3.3/lib/libgcc_s.so -o -e /usr/gcc/3.3/lib/libg2c.so -o -e /usr/gcc/3.3/lib/libstdc++.so ]; then
        if [ -e /usr/%{_lib}/qt-4.8.5/dbus/%{_lib} ]; then
            export LD_LIBRARY_PATH=%{rootdir}/lib:%{topdir}/%{thirdparty}/lib:/usr/%{_lib}/qt-4.8.5/dbus/%{_lib}:/%{_lib}:/lib:/usr/%{_lib}:/usr/lib:\$LD_LIBRARY_PATH:/usr/gcc/3.3/lib
        else
            export LD_LIBRARY_PATH=%{rootdir}/lib:%{topdir}/%{thirdparty}/lib:/%{_lib}:/lib:/usr/%{_lib}:/usr/lib:\$LD_LIBRARY_PATH:/usr/gcc/3.3/lib
        fi
    else
        if [ -e /usr/%{_lib}/qt-4.8.5/dbus/%{_lib} ]; then
            export LD_LIBRARY_PATH=%{rootdir}/lib:%{topdir}/%{thirdparty}/lib:/usr/%{_lib}/qt-4.8.5/dbus/%{_lib}:/%{_lib}:/lib:/usr/%{_lib}:/usr/lib:\$LD_LIBRARY_PATH
        else
            export LD_LIBRARY_PATH=%{rootdir}/lib:%{topdir}/%{thirdparty}/lib:/%{_lib}:/lib:/usr/%{_lib}:/usr/lib:\$LD_LIBRARY_PATH
        fi
    fi
    export CASAPATH="%{rootdir} linux"
    export PATH=%{bindir}:%{topdir}/%{thirdparty}/bin:/usr/bin:/usr/X11R6/bin:\$PATH

    PGPLOT_FONT=%{pgplotdir}/grfont.dat
    PGPLOT_RGB=%{pgplotdir}/rgb.txt
    export PGPLOT_FONT PGPLOT_RGB

    exec /bin/sh -c "\$exec_str"
fi
EOF
chmod 755 $RPM_BUILD_ROOT%{bindir}/casapyinfo

cd $RPM_BUILD_ROOT%{rootdir}
ln -s ../../casapy/data

##
## previously, we removed all perl scripts since they were unnecessary, and
## they introduce a perl dependency unnecessarily. Now we have perl scripts
## which are required, but we continue to remove those which are not needed
## by end users... [those needed by end users should contain
## "Distribution: <possibly other text> END_USER_BINARY"].
##
cd $RPM_BUILD_ROOT%{bindir}
for i in `find . -type f`; do
    if file $i 2> /dev/null | grep 'perl script' > /dev/null; then
	if ! egrep 'Distribution:.*END_USER_BINARY' $i > /dev/null 2>&1; then
	    /bin/rm -f $i
	fi
    fi
done

##
## backup svn tree for possible future developer's rpm
##
cd $top
find code -type f | grep '/\.svn/' | tar --files-from - -zcf $RPM_BUILD_ROOT%{rootdir}/code-tree.tar.gz

##
## stamp out the java plague...
##
cd $RPM_BUILD_ROOT
for i in `find . -name '*casajni*'`; do
    rm -f $i
done

##
## and finally, do rpath substitution...
## ...chrpath is available from the casa repo (for RHEL5)...
##
cd $RPM_BUILD_ROOT
for i in `find . -type f`; do
  if file $i | grep 'ELF 64-bit' > /dev/null 2>&1; then
    if chrpath -l $i | grep 'no rpath' > /dev/null 2>&1; then
	echo WARNING $i has no runpath set...
    else
        chrpath -r "%{libdir}:%{topdir}/%{thirdparty}/lib" $i
    fi
  fi
done

%files
%defattr(-,root,root)
####################### casa wrappers #############
%{prefix}/bin/casa
%{prefix}/bin/casapy
%{prefix}/bin/asdm2MS
%{prefix}/bin/casabrowser
%{prefix}/bin/casa-config
%{prefix}/bin/casafeather
%{prefix}/bin/casalogger
%{prefix}/bin/casaplotms
%{prefix}/bin/casaviewer

%files bin
%defattr(-,root,root)
%dir %{pylibdir}
%dir %{rootdir}
%dir %{rootdir}/lib
%dir %{rootdir}/bin
%dir %{pgplotdir}

%dir %{rootdir}/xml
%{rootdir}/xml/*
%dir %{rootdir}/linux
%{rootdir}/linux/*

%{rootdir}/VERSION
%{rootdir}/.aipsrc
%{rootdir}/makedefs

################ binaries            ################
%{rootdir}/bin/*

################ pgplot init files   ################
%{pgplotdir}/grfont.dat
%{pgplotdir}/rgb.txt

################ python objects      ################
%{pylibdir}/*

################ loadable objects    ################
##
##   no longer any loadable object in lib & this results in
##   libwcs.so and libmir.so being included in both casapy and
##   casapy-shared
##
##%{rootdir}/lib/*.so

################ casa-data symlink   ################
%{rootdir}/data

################ jar file for xslt   ################
%{rootdir}/lib/saxon8.jar

%files shared
%defattr(-,root,root)
%dir %{libdir}
%dir %{rootdir}

%{rootdir}/.revstate

################ shared libraries    ################
%{libdir}/*.so.*
%{libdir}/libasap.so
%{libdir}/libasdmstman.so
%{libdir}/libatnf.so
%{libdir}/libcasa_*.so

%files devel
%defattr(-,root,root)
%dir %{rootdir}
%dir %{libdir}
%dir %{incdir}

################ source code         ################
%{rootdir}/code-tree.tar.gz

################ pgplot header files ################
%{pgplotdir}/pgplot.inc
%{pgplotdir}/cpgplot.h
#%{pgplotdir}/tkpgplot.h
#%{pgplotdir}/XmPgplot.h
%{pgplotdir}/grpckg1.inc

################ shared links        ################
%{libdir}/libalma.so
#%{libdir}/libatnf.so
#%{libdir}/libcalibration.so
%{libdir}/libcasaqt.so
%{libdir}/libdish.so
%{libdir}/libdisplay.so
%{libdir}/libflagging.so
%{libdir}/libgraphics.so
%{libdir}/libcasadbus.so
#%{libdir}/libionosphere_f.so
#%{libdir}/libionosphere.so
%{libdir}/libmsvis.so
%{libdir}/libnrao.so
#%{libdir}/libsimulators.so
%{libdir}/libsynthesis.so
#%{libdir}/libxmlcasa.so
#%{libdir}/libtableplot.so
%{libdir}/libplotms.so
%{libdir}/libspectrallines.so
%{libdir}/libimageanalysis.so
%{libdir}/libmiriad.so
%{libdir}/libair_casawvr.so
%{libdir}/libbnmin1.so
%{libdir}/libcalanalysis.so
%{libdir}/libstdcasa.so
%{libdir}/libtools.so
%{libdir}/libasdmstman.so
%{libdir}/libguitools.so
%{libdir}/libcomponents.so
%{libdir}/libmstransform.so
%{libdir}/libsingledish.so
%{libdir}/libatmosphere.so
%{libdir}/libparallel.so

################ static libraries    ################
#%{libdir}/*.a

################ headers & templates ################
%{incdir}/*

%clean

exit 0
################ run unit tests ################
#export CASAPATH="`pwd` linux"
#export PATH=`pwd`/linux/bin:$PATH
#if [ -e /usr/%{_lib}/qt-4.8.5/dbus/%{_lib} ]; then
#    export LD_LIBRARY_PATH=`pwd`/linux/lib:/usr/%{_lib}/qt-4.8.5/dbus/%{_lib}:$LD_LIBRARY_PATH
#else
#    export LD_LIBRARY_PATH=`pwd`/linux/lib:$LD_LIBRARY_PATH
#fi
#
#echo "runtests start time:" `date -u`
#gmake -C code runtests 2>&1 | su aips2mgr -c /root/casa/runtest_filter
#echo "runtests end time:" `date -u`

