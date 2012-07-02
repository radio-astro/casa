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

#ifndef QTPROFILE_H
#define QTPROFILE_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/DisplayEvents/MWCCrosshairTool.h>
#include <display/Display/PanelDisplay.h>

#include <images/Regions/ImageRegion.h>
#include <images/Images/ImageInterface.h>
#include <measures/Measures/Stokes.h>
#include <images/Images/ImageAnalysis.h>
#include <msvis/MSVis/StokesVector.h>


#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <QToolButton>
#include <QDialog>
#include <QCheckBox>
#include <QPixmap>
#include <QLineEdit>
#include <QComboBox>
#include <map>
#include <vector>
#include <QHash>
#include <QHashIterator>
#include <graphics/X11/X_exit.h>

inline void initPlotterResource() { Q_INIT_RESOURCE(QtPlotter); }


namespace casa { 

//class casa::Stokes;

class QtProfile : public QWidget//, public MWCCrosshairTool
{
    Q_OBJECT
public:
    QtProfile(ImageInterface<Float>* img, const char *name = 0, 
	      QWidget *parent = 0);
    ~QtProfile();
    MFrequency::Types determineRefFrame( ImageInterface<Float>* img, bool check_native_frame = false );
public slots:
    void zoomIn();
    void zoomOut();
    void print();
    void save();
    void printExp();
    void saveExp();
    void writeText();
    void up();
    void down();
    void left();
    void right();
    void setMultiProfile(int);
    void setRelativeProfile(int);
    void setAutoScale(int);
    void changeCoordinate(const QString &text); 
    void changeFrame(const QString &text);
    void changeCoordinateType(const QString &text); 
    void updateZoomer();
    //virtual void crosshairReady(const String& evtype);
    virtual void closeEvent ( QCloseEvent * event ); 
    void resetProfile(ImageInterface<Float>* img, const char *name = 0);
    void wcChanged(const String,
                     const Vector<Double>, const Vector<Double>);
    void changeAxis(String, String, String);

    void overplot(QHash<QString, ImageInterface<float>*>);
signals:
    void hideProfile();
    void coordinateChange(const String&);

private:

    void printIt(QPrinter*);
    QToolButton *zoomInButton;
    QToolButton *zoomOutButton;
    QToolButton *leftButton;
    QToolButton *rightButton;
    QToolButton *upButton;
    QToolButton *downButton;
    QCheckBox *multiProf;
    QCheckBox *relative;
    QCheckBox *autoScale;
    
    QToolButton *printButton;
    QToolButton *saveButton;
    //QToolButton *printExpButton;
    //QToolButton *saveExpButton;
    QToolButton *writeButton;
    
    QtCanvas *pc;
    QLineEdit *te;
    QComboBox *chk;
    QComboBox *ctype;
    QComboBox *frameButton_p;
    
    ImageAnalysis* analysis;
    QHash<QString, ImageAnalysis*> *over;
    String coordinate;
    String coordinateType;
    String frameType_p;
    QString fileName;
    QString position;
    QString yUnit;
    QString yUnitPrefix;

    QString xpos;
    QString ypos;
    int cube;

    Vector<Double> lastX, lastY;
    Vector<Float> z_xval;
    Vector<Float> z_yval;
    QString region;

    QString getRaDec(double x, double y);
};

}
#endif

