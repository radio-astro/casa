#include <QtGui/QApplication>
#include "dDBus.qo.h"

int main ( int argc, char *argv[ ] ) {

    QApplication app( argc, argv );
    casa::EditlineGui elgui;
    elgui.show( );
    return app.exec( );
}
