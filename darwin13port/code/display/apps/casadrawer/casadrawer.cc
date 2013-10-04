//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#


#include <QtCore>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <display/QtPlotter/QtDrawer.qo.h>
#include <display/QtPlotter/QtDrawCanvas.qo.h>

#include <Python.h>
#include <casa/namespace.h>

int main( int argc, char **argv ) {
	QApplication a( argc, argv );

	//----------------plot a line with 2 vectors
	QtDrawer *plotter2 = new QtDrawer(0, "CASA Plotter");
	plotter2->show();

	Vector<Float> vecX(100);
	Vector<Float> vecY(100);
	for (uInt i = 0; i < 100; i++) {
		double x = i * 0.0628;
		vecX[i] = x;
		vecY[i] = sin(x);
	}


	QMessageBox::information(0, "Plotter Example",
	                         "Next: draw a line");
	plotter2->canvas()->setTitle("sin(x) - this is title", 18, 7);
	plotter2->canvas()->setXLabel("rad - this is xlable");
	plotter2->canvas()->setYLabel("this is ylabel");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 0, 9);

	for (uInt i = 0; i < 100; i++) {
		vecY[i] = cos(vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: draw a new chart");
	plotter2->canvas()->setTitle("Plot Example");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 0, 2);

	for (uInt i = 0; i < 100; i++) {
		vecY[i] = sin(2 * vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: add new line 8");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 8);

	for (uInt i = 0; i < 100; i++) {
		vecY[i] = sin(3 * vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: add new line 3");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 3);

	vecY.resize(10);
	for (uInt i = 0; i < 10; i++) {
		vecX[i] += 6.283;
		vecY[i] = sin(4 * vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: append data to line 3");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 3);

	for (uInt i = 0; i < 10; i++) {
		vecY[i] = sin(vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: append data to line 8");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 8);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: remove line 3");
	plotter2->canvas()->plotPolyLine(vecX, vecY, -3);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: remove line 8");
	plotter2->canvas()->plotPolyLine(vecX, vecY, -8);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: remove line 1");
	plotter2->canvas()->plotPolyLine(vecX, vecY, -1);

	vecY.resize(100);
	for (uInt i = 0; i < 100; i++) {
		vecX[i] = i * 0.0628;
		vecY[i] = cos(vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: add line 5 as cross=9");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 5, 9);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: add line 0 as cross=9");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 0, 10);


	for (uInt i = 0; i < 100; i++) {
		vecY[i] = sin(vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: add line 4 as circle=3");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 4, 3);

	for (uInt i = 0; i < 100; i++) {
		vecY[i] = sin(2 * vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: add line 7 as point=1");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 7, 1);

	for (uInt i = 0; i < 100; i++) {
		vecY[i] = cos(2 * vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: add line 17 as rect=7");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 17, 7);

	for (uInt i = 0; i < 100; i++) {
		vecY[i] = sin(3 * vecX[i]);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: add line 17 as plus=11");
	plotter2->canvas()->plotPolyLine(vecX, vecY, 17, 11);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: new line as hist=HIST");
	plotter2->canvas()->plotPolyLine(vecX, vecY,
	                                 0, QtDrawCanvas::HIST);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: new line as hist=HISTF");
	plotter2->canvas()->plotPolyLine(vecX, vecY,
	                                 0, QtDrawCanvas::FHIST);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: draw available symbols one at a time");

	vecX.resize(50);
	vecY.resize(50);
	for (uInt i = 0; i < 50; i++) {
		vecX[i] = i * 2 * 0.0628;
		vecY[i] = cos(vecX[i]);
	}
	for (int i = -QtDrawCanvas::FHIST - 3;
	        i < QtDrawCanvas::FHIST + 3; i++) {
		QMessageBox::information(0, "Plotter Example",
		                         QString("Next: draw symbol ")
		                         .append(QString().setNum(i)));
		plotter2->canvas()->plotPolyLine(vecX, vecY, 0, i);
	}

	QMessageBox::information(0, "Plotter Example",
	                         "Next: draw symbols with different color");
	for (int i = 1; i <= QtDrawCanvas::FHIST; i++) {
		QMessageBox::information(0, "Plotter Example",
		                         QString("Next: draw with color ")
		                         .append(QString().setNum(i)));
		plotter2->canvas()->plotPolyLine(vecX, vecY, 0, 9, i);
	}

	QMessageBox::information(0, "Plotter Example",
	                         "Next: draw symbols with symbol + line");
	for (int i = QtDrawCanvas::FHIST - 4;
	        i < QtDrawCanvas::FHIST + 4; i++) {
		QMessageBox::information(0, "Plotter Example",
		                         QString("Next: draw with line + symbol ")
		                         .append(QString().setNum(i)));
		plotter2->canvas()->plotPolyLine(vecX, vecY, 0, -i, i);
	}

	Matrix<Float> mat(3, 100);
	for (uInt j = 0; j < 100; j++) {
		if (j % 2 == 0) {
			mat(0, j) =  j;
			mat(1, j) =  j;
			mat(2, j) =  j;
		} else {
			mat(0, j) =  sin(j * 0.0314);
			mat(1, j) =  cos(j * 0.0314);
			mat(2, j) =  sin(2 * j * 0.0314);
		}
	}
	plotter2->canvas()->setTitle("Draw 3 traces", 16, 8);
	plotter2->canvas()->plotPolyLine(mat);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: draw 3 traces as symbols");
	plotter2->canvas()->plotPolyLine(mat, 0, 5);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: draw 3 traces with same color");
	plotter2->canvas()->plotPolyLine(mat, 0, 5, 5);

	for (uInt j = 0; j < 100; j++) {
		if (j % 2 == 0) {
			mat(0, j) =  j + 100;
			mat(1, j) =  j + 100;
			mat(2, j) =  j + 100;
		} else {
			mat(0, j) =  sin(j * 0.0314);
			mat(1, j) =  cos(j * 0.0314);
			mat(2, j) =  sin(2 * j * 0.0314);
		}
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: append 3 traces start from 2");
	plotter2->canvas()->plotPolyLine(mat, 2, 3, 3);

	QMessageBox::information(0, "Plotter Example",
	                         "Next: remove 3 traces start from 1");
	plotter2->canvas()->plotPolyLine(mat, -1, 3, 5);


	mat.resize(64, 100);
	for (uInt i = 0; i < 64; i++) {
		for (uInt j = 0; j < 100; j++) {
			if (j % 2 == 0)
				mat(i, j) =  j;
			else
				mat(i, j) =  5 + drand48();
		}
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: draw 64 traces");
	plotter2->canvas()->plotPolyLine(mat, 0, -1, -1);

	for (uInt i = 0; i < 64; i++) {
		for (uInt j = 0; j < 100; j++) {
			if (j % 2 == 0)
				mat(i, j) =  j;
			else
				mat(i, j) =  5 + drand48();
		}
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: add 64 traces");
	plotter2->canvas()->plotPolyLine(mat, 64, -1, -1);

	vecX.resize(100);
	vecY.resize(100);
	for (uInt i = 0; i < 100; i++) {
		double x = i * 0.0628;
		vecX[i] = x;
		vecY[i] = sin(x) + cos(x);
	}
	QMessageBox::information(0, "Plotter Example",
	                         "Next: 4 panels");
	QtDrawer plotter1(0, "tqplotter", 2, 2);
	plotter1.show();
	plotter1.canvas()->setTitle("0-------0");
	plotter1.canvas()->plotPolyLine(mat, 0, -1, -1);
	plotter1.canvas(0, 1)->setTitle("0-------1");
	plotter1.canvas(0, 1)->plotPolyLine(vecX, vecY );
	plotter1.canvas(1, 0)->setTitle("1-------0");
	plotter1.canvas(1, 0)->plotPolyLine(vecX, vecY );
	plotter1.canvas(1, 1)->setTitle("1-------1");
	plotter1.canvas(1, 1)->plotPolyLine(vecX, vecY );



	QMessageBox::information(0, "Plotter Example",
	                         "load a plot data file");
	char* fn = 0;
	if (argc > 1)
		fn = argv[1];
	QtDrawer plotter(0, fn);
	plotter.show();

	return a.exec();
}



/*
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
*/

