#!/bin/sh
#
# This is a knock off of Darrell's casapyinfo
#
AIPSROOT=`echo $CASAPATH | awk '{print $1}'`
AIPSARCH=`echo $CASAPATH | awk '{print $2}'`
CASAINSTALL_DIR=$AIPSROOT/$AIPSARCH
INSTALL_BIN=$CASAINSTALL_DIR/bin
BUILD_CASAPATH=$CASAPATH
WHOBUILTME=`uname -a`
SVNROOT=`svn info | egrep '^URL' | perl -pe 's#^URL:\s+(\S+)/code.*\n?#$1#'`

cat > $INSTALL_BIN/casapyinfo<<EOF
#!/bin/sh

usage() {
    echo "casapyinfo [--help|--data|--keys|--version|--environ|--makedefs]"
    echo "    --help:        usage info"
    echo "    --data:        location of the data repository"
    echo "    --version:     casa version info"
    echo "    --svnroot      svn url for source"
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
                echo "$AIPSROOT/data"
                shift
                ;;
        --version)
                cat "$CASAINSTALL_DIR/VERSION"
                shift
                ;;
        --svnroot)
                echo "$SVNROOT"
                shift
                ;;
        --buildhost)
                echo "$WHOBUILTME"
                shift
                ;;
        --makedefs)
                cat "$CASAINSTALL_DIR/makedefs"
                shift
                ;;
        --environ)
                echo "LD_LIBRARY_PATH=$CASAINSTALL_DIR/lib:/usr/lib:$LD_LIBRARY_PATH:\$LD_LIBRARY_PATH"
                echo "CASAPATH=$BUILD_CASAPATH"
                echo "PATH=$CASAINSTALL_DIR/bin:/usr/bin:/usr/X11R6/bin:$PATH"
                if [ -f /usr/lib/pgplot/grfont.dat ]; then
                    echo "PGPLOT_FONT=/usr/lib/pgplot/grfont.dat"
                fi
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
	*)
                usage
	        shift
		;;
    esac
done

if [ "\$exec_str" ]; then
    export LD_LIBRARY_PATH=$CASAINSTALL_DIR/lib:/usr/lib:$LD_LIBRARY_PATH:\$LD_LIBRARY_PATH
    export CASAPATH="$BUILD_CASAPATH"
    export PATH=$CASA_INSTAL_DIR:/usr/bin:/usr/X11R6/bin:$PATH
    if [ -f /usr/lib/pgplot/grfont.dat ]; then
        export PGPLOT_FONT=/usr/lib/pgplot/grfont.dat
    fi
    exec /bin/sh -c "\$exec_str"
fi
EOF
chmod a+rx $INSTALL_BIN/casapyinfo
