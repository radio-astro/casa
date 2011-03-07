#include <QtCore>
#include <QApplication>
#include <QFileDialog>
#include <display/QtPlotter/QtPlotter.qo.h>


#include <casa/namespace.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    char* fn = 0;
    if (argc > 1)
        fn = argv[1]; 
    QtPlotter plotter(0, fn);
    plotter.show();
 	
    return a.exec();
}


