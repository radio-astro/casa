#include <QtCore>
#include <QApplication>
#include <display3d/3DViewer/3DWindow.qo.h>


#include <casa/namespace.h>


int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    char* fn = 0;
    if (argc > 1) {
        fn = argv[1]; 
    }
    else {
       qDebug() << "Usage: 3v <image_name>\n"
                   "(The image format can be CASA or FITS)";
    }

    Window3D window(fn);
    window.show();
    window.resize(window.size() + QSize(10, 10)); 	
    return app.exec();
}

