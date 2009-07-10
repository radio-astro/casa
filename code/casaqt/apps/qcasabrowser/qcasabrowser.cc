
#include <xercesc/util/PlatformUtils.hpp>

#include <QApplication>
#include <casaqt/QtBrowser/TBMain.qo.h>
//#include <QtGui>

#include <casa/namespace.h>

int main(int argc, char** argv) {
    xercesc::XMLPlatformUtils::Initialize();
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(QtBrowser);

    TBMain* main = new TBMain();
    
    for(int i = 1; i < argc; i++)
      main->openTable(argv[i]);

    int r = app.exec();
    delete main;
    xercesc::XMLPlatformUtils::Terminate();
    return r;
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
