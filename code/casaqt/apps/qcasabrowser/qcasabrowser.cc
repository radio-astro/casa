#include <casaqt/QtBrowser/TBMain.qo.h>

#include <xercesc/util/PlatformUtils.hpp>

#include <QApplication>
#include <QtGui>
#include <signal.h>
#include <casa/namespace.h>

static void preprocess_args( int argc, const char *argv[], int &numargs, char **&args );

int main(int argc, const char* argv[] ) {

    char **args;
    int numargs;
    preprocess_args( argc, argv, numargs, args );

    xercesc::XMLPlatformUtils::Initialize();
    QApplication app( numargs, args );

    Q_INIT_RESOURCE(QtBrowser);

    TBMain* main = new TBMain();
    
    for(int i = 1; i < numargs; i++)
      main->openTable(args[i]);

    int r = app.exec();
    delete main;
    xercesc::XMLPlatformUtils::Terminate();
    return r;
}


static void preprocess_args( int argc, const char *argv[], int &numargs, char **&args ) {
    args = (char**) malloc( sizeof(char*) * argc );
    numargs = 0;
    for ( int x = 0; x < argc; ++x ) {
	if ( ! strcmp( argv[x], "--casapy" ) ) {
	    signal(SIGINT,SIG_IGN);
	} else {
	    args[numargs++] = strdup(argv[x]);
	}
    }
}



/* casabrowser script

#!/bin/sh

aipspath=($CASAPATH)
CASAROOT=${aipspath[0]}

case $(uname) in
  Darwin)
     echo "Adjust this as it will cause problems for Boyd with the distro"
     $CASAROOT/darwin/apps/qcasabrowser.app/Contents/MacOS/qcasabrowser $@
  ;;
  Linux)
        qcasabrowser $@
  ;;
esac

*/
