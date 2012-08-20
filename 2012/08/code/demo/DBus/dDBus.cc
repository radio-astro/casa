#include <string>
#include <QtGui/QApplication>
#include "dDBus.qo.h"

int main ( int argc, char *argv[ ] ) {
    std::string dbus_name;
    std::string launch_flag("--dbusname");
    for (int x=0; x < argc; ++x ) {
	if ( launch_flag == argv[x] && (x + 1 < argc) )
	    dbus_name = argv[++x];
    }

    QApplication app( argc, argv );
    casa::EditlineGui elgui(dbus_name);
    elgui.show( );
    return app.exec( );
}
