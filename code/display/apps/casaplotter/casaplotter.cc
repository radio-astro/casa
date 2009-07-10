#include <qmacdefines_mac.h>
#include <QApplication>
#include <display/QtPlotter/QtPlotter.qo.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    char* fn = 0;
    if (argc > 1)
        fn = argv[1]; 
    casa::QtPlotter plotter(0, fn);
    plotter.show();
 	
    return a.exec();
}


