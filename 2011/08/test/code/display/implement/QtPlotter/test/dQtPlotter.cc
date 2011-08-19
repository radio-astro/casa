#include <QtCore>
#include <QApplication>
#include <QFileDialog>
#include <display/QtPlotter/QtPlotter.qo.h>

#include <casa/namespace.h>


int main( int argc, char **argv )
{
    Q_INIT_RESOURCE(plotter);
    QApplication a( argc, argv );

    //----------------plot data from a file
    QtPlotter *plotter = new QtPlotter(0, "lines in a file, hold-on");
  
    QFileDialog *fdlg = new QFileDialog(0,
                    "Choose a file to open",
                    "/home/basho/hye/current/linux_rhe/bin",
                    "Plots (*.plt)");
    QStringList fileNames;
    if (fdlg->exec())
          fileNames = fdlg->selectedFiles();
    if (!fileNames.isEmpty()){
         plotter->canvas()->plotPolyLines(fileNames.at(0));
	 plotter->show();
   }

    //----------------plot a line with 2 vectors
    Vector<Float> vecX(100);
    Vector<Float> vecY(100);
    for (uInt i = 0; i < 100; i++)
    {
        double x = i * 0.0628;
        vecX[i] = x;
        vecY[i] = sin(x);
    }
    QtPlotter *plotter2 = new QtPlotter(0, "CASA Plotter");
    plotter2->canvas()->plotPolyLine(vecX, vecY);
    plotter2->canvas()->setTitle("sin(x) - this is title", 18, 7);
    plotter2->canvas()->setXLabel("rad - this is xlable");
    plotter2->canvas()->setYLabel("this is ylabel");
    plotter2->show();

    //----------------plot lines with matrix<Double>
    Matrix<Double> mat(100, 10); 
    for (int i = 0; i < 100; i++)
    {
        double x = i * 0.0628;
        mat(i, 0) = x;
        mat(i, 1) = sin(x);

        mat(i, 2) = x;
        mat(i, 3) = cos(x);

        mat(i, 4) = x;
        mat(i, 5) = sin(x / 2);

        mat(i, 6) = x;
        mat(i, 7) = sin(3 * x);

        mat(i, 8) = x;
        mat(i, 9) = sin(2 * x);
    }
    QtPlotter *plotter3 = new QtPlotter(0, "Sin(x*)");
    plotter3->canvas()->plotPolyLine(mat);
    plotter3->show();
  
    //----------------plot an image with matrix<Double>
    Matrix<uInt> img(300, 300);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            img(i, j) = i * j  ;
        }
    }
    
    QtPlotter *plotter5 = new QtPlotter(0, "'3C48'");
    plotter5->canvas()->drawImage(img);
    plotter5->show();
 	
    return a.exec();
}


