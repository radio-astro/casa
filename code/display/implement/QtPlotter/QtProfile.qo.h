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

#ifndef QtProfile_H
#define QtProfile_H

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
#include <display/QtPlotter/QtMWCTools.qo.h>
#include <display/Display/PanelDisplay.h>
#include <display/Utilities/Lowlevel.h>

#include <images/Regions/ImageRegion.h>
#include <images/Images/ImageInterface.h>
#include <measures/Measures/Stokes.h>
#include <images/Images/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/SpectralCollapser.h>
#include <msvis/MSVis/StokesVector.h>


#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QMainWindow>
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

#include <display/QtPlotter/QtProfileGUI.ui.h>

inline void initPlotterResource() { Q_INIT_RESOURCE(QtPlotter); }


namespace casa { 

class QtProfilePrefs;

class QtProfile : public QMainWindow, Ui::QtProfileGUI
{
	Q_OBJECT

public:
	enum ExtrType {
		MEAN     =0,
		MEDIAN   =1,
		SUM      =2,
		MSE      =3,
		RMSE     =4,
		SQRTSUM  =5,
		NSQRTSUM =6,
	};

	enum PlotType {
		PMEAN,
		PMEDIAN,
		PSUM,
		//PVRMSE,
	};

	enum ErrorType {
		PNOERROR,
		PERMSE,
		PPROPAG
	};

	QtProfile(ImageInterface<Float>* img, const char *name = 0,
			QWidget *parent = 0, std::string rcstr="prf");

	~QtProfile();
    MFrequency::Types determineRefFrame( ImageInterface<Float>* img, bool check_native_frame = false );

    virtual std::string rcid( ) const { return rcid_; }

public slots:
    void zoomIn();
    void zoomOut();
    void zoomNeutral();
    void print();
    void save();
    void printExp();
    void saveExp();
    void writeText();
    void up();
    void down();
    void left();
    void right();
    void preferences();
	void setPreferences(int stateAutoX, int stateAutoY, int showGrid, int stateMProf, int stateRel);

    void setPlotError(int);
    void changeCoordinate(const QString &text);
    void changeFrame(const QString &text);
    void changeCoordinateType(const QString &text);
    virtual void closeEvent ( QCloseEvent *);
    void resetProfile(ImageInterface<Float>* img, const char *name = 0);
    void wcChanged( const String,
		    const Vector<Double>, const Vector<Double>,
		    const Vector<Double>, const Vector<Double>,
		    const ProfileType);
    void redraw( );
    void changePlotType(const QString &text);
    void changeErrorType(const QString &text);
    void changeCollapseType(const QString &text);
    void changeCollapseError(const QString &text);

    void changeAxisOld(String xa, String ya, String za, std::vector<int>);
    void changeAxis(String xa, String ya, String za, std::vector<int>);
    void changeSpectrum(String spcTypeUnit, String spcRval, String spcSys);
    void doImgCollapse();
    void setCollapseRange(float xmin, float xmax);


    void overplot(QHash<QString, ImageInterface<float>*>);

    void newRegion( int, const QString &shape, const QString &name,
		    const QList<double> &world_x, const QList<double> &world_y,
		    const QList<int> &pixel_x, const QList<int> &pixel_y,
		    const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

    void updateRegion( int, const QList<double> &world_x, const QList<double> &world_y,
		       const QList<int> &pixel_x, const QList<int> &pixel_y );

signals:
   void hideProfile();
   void coordinateChange(const String&);
   void showCollapsedImg(String path, String dataType, String displayType, Bool autoRegister, Bool tmpData);

private:

	void stringToPlotType(const QString &text,  QtProfile::PlotType &pType);
    void stringToErrorType(const QString &text, QtProfile::ErrorType &eType);
    void fillPlotTypes();
    void getcoordTypeUnit(String &ctypeUnitStr, String &cTypeStr, String &unitStr);
    void printIt(QPrinter*);
    QString getRaDec(double x, double y);

	ImageAnalysis* analysis;
	ImageInterface<Float>* image;

	SpectralCollapser *collapser;

	QHash<QString, ImageAnalysis*> *over;
	String coordinate;
	String coordinateType;
	String xaxisUnit;
	String ctypeUnit;
	String spcRefFrame;
	String cSysRval;
	QString fileName;
	QString position;
	QString yUnit;
	QString yUnitPrefix;

	QString xpos;
	QString ypos;
	int cube;

	int npoints;
	int npoints_old;

	int stateMProf;
	int stateRel;

	Vector<Double> lastPX, lastPY;
	Vector<Double> lastWX, lastWY;
	Vector<Float> z_xval;
	Vector<Float> z_yval;
	Vector<Float> z_eval;
	QString region;

	String last_event_cs;
    Vector<Double> last_event_px, last_event_py;
    Vector<Double> last_event_wx, last_event_wy;

    // connection to rc file
    Casarc &rc;
    // rc id for this panel type
    std::string rcid_;

    QtProfile::PlotType  itsPlotType;
    QtProfile::ErrorType itsErrorType;
    SpectralCollapser::CollapseType  itsCollapseType;
    SpectralCollapser::CollapseError itsCollapseError;
    LogIO *itsLog;

    class spectra_info {
	public:
	    spectra_info( ) { }
	    spectra_info( const QString &s ) : shape_(s) { }
	    spectra_info( const spectra_info &other ) : shape_(other.shape_) { }
	    const spectra_info &operator=( const spectra_info &other ) { shape_ = other.shape_; return *this; }
	    const QString &shape( ) { return shape_; }
	private:
	    QString shape_;
    };

    typedef std::map<int,spectra_info> SpectraInfoMap;
    SpectraInfoMap spectra_info_map;

};

}
#endif

