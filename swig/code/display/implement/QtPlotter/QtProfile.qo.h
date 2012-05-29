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
#include <display/QtPlotter/SpecFitMonitor.h>
#include <display/DisplayEvents/MWCCrosshairTool.h>
#include <display/QtPlotter/QtMWCTools.qo.h>
#include <display/Display/PanelDisplay.h>
#include <display/Utilities/Lowlevel.h>

#include <images/Regions/ImageRegion.h>
#include <images/Images/ImageInterface.h>
#include <measures/Measures/Stokes.h>
#include <images/Images/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/SpectralCollapser.h>
//#include <imageanalysis/ImageAnalysis/SpectralFitter.h>

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
class SpectralFitter;

//Note:  The purpose of the SpecFitMonitor interface is to provide
//a communications interface between the class doing spectral line
//fitting and this class.  By using an interface, we can decouple
//this class and the class doing the work of spectral fitting as
//much as possible.

class QtProfile : public QMainWindow, Ui::QtProfileGUI, public SpecFitMonitor
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
		FLUX     =7,
		EFLUX    =8,
	};

	enum PlotType {
		PMEAN,
		PMEDIAN,
		PSUM,
		//PVRMSE,
		PFLUX=7,
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

	//These methods are from the SpecFitMoniter interface.
	//Their purpose is to provide the class doing the spectral
	//profile fitting with initial input values, and to inform
	//this class of status and log messages indicated the result
	//of a spectral fit.
	void postStatus( String status );
	Vector<Float> getXValues() const;
	Vector<Float> getYValues() const;
	Vector<Float> getZValues() const;
	QString getYUnit() const;
	QString getYUnitPrefix() const;
	String getXAxisUnit() const;
	QString getFileName() const;
	const ImageInterface<Float>* getImage() const;
	const String getPixelBox() const;

public slots:
	void zoomIn();
	void zoomOut();
	void zoomNeutral();
	void print();
	void saveGraphic();
	void printExp();
	void saveExp();
	void exportProfile();
	void up();
	void down();
	void left();
	void right();
	void preferences();
	void setPreferences(int stateAutoX, int stateAutoY, int showGrid,
			int stateMProf, int stateRel, bool showToolTips, bool showTopAxis,
			bool displayStepFunction, bool opticalFitter);

	void setPlotError(int);
	void changeCoordinate(const QString &text);
	void changeFrame(const QString &text);

	void changeCoordinateType( const QString & text );

	void changeTopAxisCoordinateType( const QString & text );
	virtual void closeEvent ( QCloseEvent *);
	void resetProfile(ImageInterface<Float>* img, const char *name = 0);
	void wcChanged( const String,
			const Vector<Double>, const Vector<Double>,
			const Vector<Double>, const Vector<Double>,
			const ProfileType);
	void redraw( );
	void changePlotType(const QString &text);
	void changeErrorType(const QString &text);
	void changeCollapseType(QString text=QString(""));
	void changeCollapseError(QString text=QString(""));

	void changeAxisOld(String xa, String ya, String za, std::vector<int>);
	void changeAxis(String xa, String ya, String za, std::vector<int>);
	void changeSpectrum(String spcTypeUnit, String spcRval, String spcSys);
	void doImgCollapse();
	//void doLineFit();
	void plotMainCurve();
	void setCollapseRange(float xmin, float xmax);
	void emitChannelSelect(float xval);

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
   void channelSelect( const Vector<float> &zvec, float zval );

private:
   void stringToPlotType(const QString &text,  QtProfile::PlotType &pType);
   void stringToErrorType(const QString &text, QtProfile::ErrorType &eType);
   void fillPlotTypes(const ImageInterface<Float>* img);
   void getcoordTypeUnit(String &ctypeUnitStr, String &cTypeStr, String &unitStr);
   void printIt(QPrinter*);
   bool exportASCIISpectrum(QString &fn);
   bool exportFITSSpectrum(QString &fn);
   void messageFromProfile(QString &msg);
   void setCollapseVals(const Vector<Float> &spcVals);
   void setUnitsText( String unitStr );

   /**
    * Initializes the spectrum positioning tab.
    */
   void initSpectrumPosition();

   /**
    * Returns false if first vector value is greater than the last
    * vector value; otherwise returns true.
    */
   bool isAxisAscending(const Vector<Float>& axisValues ) const;

   void setTitle( const QString& shape );
   void copyToLastEvent( const String& c, const Vector<Double> &px,
   	    		const Vector<Double> &py,
   	    		const Vector<Double> &wx,
   	    		const Vector<Double> &wy );
   void setPlotType( int wcArraySize );
   bool checkCube();
   void assignCoordinate(const String& c);
   void initializeCoordinateVectors(const Vector<double> &px, const Vector<double> &py,
   			const Vector<double> &wx, const Vector<double> &wy, Vector<double> &pxv,
   			Vector<double> &pyv, Vector<double> &wxv, Vector<double> &wyv) const;
   void setPositionStatus(const Vector<double> &pxv, const Vector<double> &pyv,
   								const Vector<double> &wxv, const Vector<double> &wyv );
   bool assignFrequencyProfile( const Vector<double> &wxv, const Vector<double> &wyv,
		   const String& coordinateType, const String& xAxisUnit,
		   Vector<Float> &z_xval, Vector<Float> &z_yval);
   bool setErrorPlotting( const Vector<double> &wxv, const Vector<double> &wyv);
   void storeCoordinates( const Vector<double> pxv, const Vector<double> pyv,
									const Vector<double> wxv, const Vector<double> wyv );
   void pageUpdate( int selectionIndex, int unitIndex );
   bool populatePixels( QList<int> &pixelX, QList<int> &pixelY,
   			const QList<double> &worldX, const QList<double> &worldY ) const ;
   bool populateWorlds( const QList<int> &pixelX, const QList<int> &pixelY,
      			QList<double> &worldX, QList<double> &worldY );
   void fillPointWorld( QList<double> &worldX, QList<double> &worldY ) const;
   void fillPointPixel( QList<int> &pixelX, QList<int>&pixelY ) const;
   bool fillBoxPixel( QList<int> &pixelX, QList<int>&pixelY );
   bool fillBoxWorld( QList<double> &worldX, QList<double> & worldY );
   bool fillBasedOnBoxSpecification(  const double*  const firstXPix, const double * const firstYPix,
   		const double* const secondXPix, const double* const secondYPix,
   		double* const blcxPix, double* const blcyPix,
   		double* const trcxPix, double* const trcYPix );
   double spinToRadians( bool dec, QSpinBox *degSpinBox,
   		QSpinBox* minSpinBox, QLineEdit* secSpinBox) const;
   void switchBoxLabels( int index, int pageIndex, QLabel* const x1Label, QLabel* const y1Label,
   		QLabel* const x2Label, QLabel* const y2Label );
   void updateAxisUnitCombo( const QString& textToMatch, QComboBox* axisUnitCombo );
   /**
    *
    */
   void setPixelCanvasYUnits( const QString& yUnitPrefix, const QString& yUnit );

   Int scaleAxis();
   void addImageAnalysisGraph( const Vector<double> &wxv, const Vector<double> &wyv, Int ordersOfM );

   //Conversion
   QString getRaDec(double x, double y);

   ImageAnalysis* analysis;
   ImageInterface<Float>* image;

   SpectralCollapser *collapser;
   //SpectralFitter    *fitter;

   QHash<QString, ImageAnalysis*> *over;
   const String WORLD_COORDINATES;
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

   Vector<Float> z_xval;
   Vector<Float> z_yval;
   Vector<Double> lastPX, lastPY;
   Vector<Double> lastWX, lastWY;
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
   Int ordersOfM_;
   Bool newCollapseVals;

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

   //Used for spectrum positioning
   enum PositionTypeIndex { POINT, BOX, END_POSITION_TYPE };
   enum UnitIndex {RADIAN, PIXEL, END_UNIT };
   QIntValidator* pixelValidator;
   QDoubleValidator* secValidator;
   enum StackPages { POINT_PIXEL, POINT_RA_DEC, BOX_PIXEL, BOX_RA_DEC };
   enum BoxSpecificationIndex { TL_LENGTH_HEIGHT, CENTER_LENGTH_HEIGHT, TL_BR, BL_TR,
	   TL_LENGTH_HEIGHT_WORLD, CENTER_LENGTH_HEIGHT_WORLD, TL_BR_WORLD, BL_TR_WORLD, END_SPEC };
   QMap<BoxSpecificationIndex,QList<QString> > boxLabelMap;



   private slots:
   	    void setPosition();
   		void locationSelectionTypeChanged( int index );
   		void locationUnitsChanged( int index );
   		void boxSpecChanged( int index );
   		void changeTopAxis();
   		void updateXAxisLabel( const QString &text, QtPlotSettings::AxisIndex axisIndex );

};

}
#endif

