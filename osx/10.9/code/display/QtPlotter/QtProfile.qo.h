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
#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <display/QtPlotter/canvasMode/CanvasMode.h>
#include <display/DisplayEvents/MWCCrosshairTool.h>
#include <display/QtPlotter/QtMWCTools.qo.h>

#include <display/Display/PanelDisplay.h>
#include <display/Utilities/Lowlevel.h>

#include <images/Regions/ImageRegion.h>
#include <images/Images/ImageInterface.h>
#include <measures/Measures/Stokes.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/SpectralCollapser.h>
#include <imageanalysis/ImageAnalysis/ImageCollapserData.h>

#include <display/region/Region.qo.h>

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

#include <casa/cppconfig.h>

inline void initPlotterResource() {
	Q_INIT_RESOURCE(QtPlotter);
}


namespace casa {

	class QtProfilePrefs;
	class ColorSummaryWidget;
	class LegendPreferences;
	class SmoothPreferences;
	class QtCanvas;

//Note:  The purpose of the SpecFitMonitor interface is to provide
//a communications interface between the class doing spectral line
//fitting and this class.  By using an interface, we can decouple
//this class and the class doing the work of spectral fitting as
//much as possible.

	class QtProfile : public QMainWindow, Ui::QtProfileGUI, public ProfileTaskMonitor {
		Q_OBJECT

	public:
		enum ExtrType {
		    MEAN     =ImageCollapserData::MEAN,
		    MEDIAN   =ImageCollapserData::MEDIAN,
		    SUM      =ImageCollapserData::SUM,
		    MSE      =ImageCollapserData::VARIANCE,
		    RMSE     =ImageCollapserData::STDDEV,
		    SQRTSUM  =ImageCollapserData::SQRTSUM,
		    NSQRTSUM =ImageCollapserData::SQRTSUM_NPIX,
		    FLUX     =ImageCollapserData::FLUX,
		    EFLUX    =ImageCollapserData::SQRTSUM_NPIX_BEAM
		};

		enum PlotType {
		    PMEAN = MEAN,
		    PMEDIAN = MEDIAN,
		    PSUM = SUM,
		    //PVRMSE,
		    PFLUX=FLUX,
		};

		enum ErrorType {
		    PNOERROR,
		    PERMSE,
		    PPROPAG
		};



		QtProfile(shared_ptr<ImageInterface<Float> > img, const char *name = 0,
		          QWidget *parent = 0, std::string rcstr="prf");

		~QtProfile();
		MFrequency::Types determineRefFrame(shared_ptr<ImageInterface<Float> > img, bool check_native_frame = false );

		virtual std::string rcid( ) const {
			return rcid_;
		}
		void setPath( QString filePath ) {
			imagePath = filePath;
		}

		//Allows the profiler to come up specialized to do spectroscopy
		//or another task.
		void setPurpose( ProfileTaskMonitor::PURPOSE purpose );

		//These methods are from the ProfileTaskMoniter interface.
		//Their purpose is to provide the class doing the spectral
		//profile fitting with initial input values, to inform
		//this class of status and log messages indicated the result
		//of a spectral fit, and to perform moments/collapsing.
		void postStatus( String status );
		Vector<Float> getXValues() const;
		Vector<Float> getYValues() const;
		Vector<Float> getZValues() const;
		QString getYUnit() const;
		QString getYUnitPrefix() const;
		String getXAxisUnit() const;
		QString getFileName() const;
		QString getImagePath() const;
		shared_ptr<const ImageInterface<Float> > getImage( const QString& imageName="") const;
		virtual bool getBeamInfo( const QString& curveName, Double& beamAngle, Double& beamArea ) const;
		const void getPixelBounds(Vector<double>& pixelX, Vector<double>& pixelY) const;
		void persist( const QString& key, const QString& value );
		QString read( const QString & key ) const;
		void imageCollapsed(String path, String dataType, String displayType,
				Bool autoRegister, Bool tmpData, shared_ptr<ImageInterface<Float> > img);
		void setPosition( const QList<double>& xValues, const QList<double>& yValues );
		void processTrackRecord( const String& dataName, const String& positionInfo );
		virtual MFrequency::Types getReferenceFrame() const;

		typedef std::pair<QString, shared_ptr<ImageInterface<float> > > OverplotInterface;
		//typedef pair<QString,ImageAnalysis*> OverplotAnalysis;

		static const String SHAPE_ELLIPSE;
		static const String SHAPE_RECTANGLE;
		static const String SHAPE_POINT;
		static const String SHAPE_POLY;
		static const QString FREQUENCY;

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
		void frameChanged( int );
		void setPreferences(bool stateAutoX, bool stateAutoY, int showGrid,
		                    int stateMProf, int stateRel, bool showToolTips, bool showTopAxis,
		                    bool displayStepFunction, bool opticalFitter,
		                    bool showChannelLine, bool singleChannelImage );
		void curveColorPreferences();
		void legendPreferences();
		void togglePalette( int modeIndex );

		void setPlotError(int);
		void changeCoordinate(const QString &text);
		void changeFrame(const QString &text);

		void changeCoordinateType( const QString & text );

		void changeTopAxisCoordinateType( const QString & text );
		virtual void closeEvent ( QCloseEvent *);
		void resetProfile(shared_ptr<ImageInterface<Float> > img, const char *name = 0);
		void wcChanged( const String,
		                const Vector<Double>, const Vector<Double>,
		                const Vector<Double>, const Vector<Double>,
		                const ProfileType);
		void redraw( );
		void changePlotType(const QString &text);
		void changeErrorType(const QString &text);

		void changeAxis(String xa, String ya, String za, std::vector<int>);
		void changeSpectrum(String spcTypeUnit, String spcRval, String spcSys);

		void plotMainCurve();
		void clearPlots();
		void setCollapseRange(double xmin, double xmax);

		void overplot(QList<OverplotInterface>);

		void newRegion( int, const QString &shape, const QString &name,
		                const QList<double> &world_x, const QList<double> &world_y,
		                const QList<int> &pixel_x, const QList<int> &pixel_y,
		                const QString &linecolor, const QString &text, const QString &font, int fontsize, int fontstyle );

		void updateRegion( int, viewer::region::RegionChanges,
		                   const QList<double> &world_x, const QList<double> &world_y,
		                   const QList<int> &pixel_x, const QList<int> &pixel_y );
		void pixelsChanged(int, int );
		void clearPaletteModes();

		/**
		 * Returns whether or not the image can be profiled.
		 */
		bool isImageSupported(shared_ptr<ImageInterface<float> > img );

	signals:
		void hideProfile();
		void coordinateChange(const String&);
		void showCollapsedImg(String path, String dataType, String displayType,
				Bool autoRegister, Bool tmpData, shared_ptr<ImageInterface<Float> > img);
		void channelSelect( int channelIndex );
		void adjustPosition( double tlcx, double tlcy, double brcx, double brcy );
		void movieChannel( int startChannel, int endChannel );
		void reloadImages();

	private:
		void stringToPlotType(const QString &text,  QtProfile::PlotType &pType);
		void stringToErrorType(const QString &text, QtProfile::ErrorType &eType);
		void fillPlotTypes(const shared_ptr<ImageInterface<Float> > img);
		void getcoordTypeUnit(String &ctypeUnitStr, String &cTypeStr, String &unitStr);
		void printIt(QPrinter*);
		bool exportASCIISpectrum(QString &fn);
		bool exportFITSSpectrum(QString &fn);
		void messageFromProfile(QString &msg);
		void setUnitsText( String unitStr );
		void resetYUnits( const QString& units);
		void outputCurve( int k, QTextStream& ts, Float scaleFactor );
		int getFreqProfileTabularIndex(shared_ptr<const ImageInterface<Float> > img);
		SpectralCoordinate resetTabularConversion(shared_ptr< const ImageInterface<Float> > imagePtr, Bool& valid);
		bool isSpectralAxis() const;
		void initializeSpectralProperties();
		SpectralCoordinate getSpectralAxis( shared_ptr<const ImageInterface<Float> > imagePtr, Bool& valid );
		void resetXUnits( bool spectralAxis);
		void updateSpectralReferenceFrame();
		String getRegionShape();
		int computeCB( const String& xa, const String& ya, const String& za );
		void getBeamInfo( shared_ptr<const ImageInterface<Float> > imagePtr, Double& beamArea, Double& beamSolidAngle) const;
		SpectralCoordinate getSpectralCoordinate( shared_ptr<const ImageInterface<Float> > imagePtr, Bool& valid );
		bool parseRestFrequency( const String& restStr, Quantity& result ) const;

		bool generateProfile( Vector<Float>& resultXValues, Vector<Float>& resultYValues,
					shared_ptr<casa::ImageInterface<Float> > imagePtr,
					const Vector<Double>& regionX, const Vector<Double>& regionY, String shape,
					QtProfile::ExtrType combineType, String& unit, const String& coordinateType,
					String restFreq="", const String& frame="");
		/**
		 * Returns false if first vector value is greater than the last
		 * vector value; otherwise returns true.
		 */
		bool isAxisAscending(const Vector<Float>& axisValues ) const;
		bool isVelocityUnit( const QString& unit ) const;
		bool isFrequencyUnit( const QString& unit ) const;
		bool isWavelengthUnit( const QString& unit ) const;
		void setTitle( const QString& shape );
		void setTitle(const ProfileType ptype);
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
		/*bool getFrequencyProfileWrapper( shared_ptr<ImageInterface<Float> >& img, const Vector<double> &wxv, const Vector<double> &wyv,
		                                 Vector<Float> &z_xval, Vector<Float> &z_yval,
		                                 const String& xytype, const String& specaxis,
		                                 const Int& whichStokes, const Int& whichTabular,
		                                 const Int& whichLinear, String& xunits,
		                                 const String& specFrame, QtProfile::ExtrType combineType,
		                                 const Int& whichQuality, const String& restValue,
		                                 const String& shape);*/
		bool assignFrequencyProfile( const Vector<double> &wxv, const Vector<double> &wyv,
		                             const String& coordinateType, String& xAxisUnit,
		                             Vector<Float> &z_xval, Vector<Float> &z_yval, const String& shape );
		bool setErrorPlotting( const Vector<double> &wxv, const Vector<double> &wyv);
		void storeCoordinates( const Vector<double> pxv, const Vector<double> pyv,
		                       const Vector<double> wxv, const Vector<double> wyv );
		void addCanvasMainCurve( const Vector<Float>& xVals, const Vector<Float>& yVals,
		                         const QString& label, double beamAngle, double beamArea,
		                         SpectralCoordinate coord);
		void adjustTopAxisSettings();
		void initializeXAxisUnits();
		void setXAxisUnits();
		void initSmoothing();
		void setPixelCanvasYUnits( const QString& yUnitPrefix, const QString& yUnit );
		void toggleAction( QAction* action );
		Int scaleAxis();
		void addImageAnalysisGraph( const Vector<double> &wxv, const Vector<double> &wyv, Int ordersOfM );
		void initializeSolidAngle() const;
		//Conversion
		QString getRaDec(double x, double y);

		//Finds the channel index closest to the passed in value.
		int findNearestChannel( float xval ) const;
		void initPreferences();
		void updateAxisUnitCombo( const QString& textToMatch, QComboBox* axisUnitCombo );
		void setYUnitConversionVisibility( bool visible );
		//ImageAnalysis* analysis;
		shared_ptr<ImageInterface<Float> > image;

		//For deciding whether or not it makes sense to show the top axis when
		//multiple images are loaded.
		bool isXUnitsMatch(const QString& matchUnits);
		bool isFrequencyMatch();
		bool isVelocityMatch();
		//int getChannelCount( ImageAnalysis* analysis );
		int getChannelCount( shared_ptr<ImageInterface<float> >& img);
		shared_ptr<ImageInterface<float> > findImageWithMaximumChannels();
		void restrictTopAxisOptions( bool restrictOptions, const QString& bottomUnits, bool allowFrequency = true,
				bool allowVelocity=true );
		double getUnitsPerChannel( shared_ptr<ImageInterface<Float> > img , bool* ok, const QString& matchUnits );
		QString readTopAxis() const;
		void persistTopAxis( const QString& units );
		void assignProfileType( const String& shape, int regionPointCount );
		void addOverplotToCanvas( shared_ptr<ImageInterface<Float> > imagePtr, const Vector<Float>& xVals, const
					Vector<Float>& yVals, const QString& ky );

		bool _generateProfile( Vector<Float>& resultXValues, Vector<Float>& resultYValues,
					shared_ptr<const casa::ImageInterface<Float> > imagePtr,
					const Vector<Double>& regionX, const Vector<Double>& regionY, String shape,
					QtProfile::ExtrType combineType, String& unit, const String& coordinateType,
					String restFreq, const String& frame);


		QList<OverplotInterface> *over;
		const String WORLD_COORDINATES;
		String coordinate;
		String coordinateType;
		String xaxisUnit;
		String ctypeUnit;
		String spcRefFrame;

		//Rest frequency quantity and unit.
		String cSysRval;

		QString fileName;
		QString imagePath;
		QString position;
		QString yUnit;
		QString yUnitPrefix;
		//Holds the available x-axis units
		QStringList xUnitsList;
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

		LogIO *itsLog;
		Int ordersOfM_;
		Bool newCollapseVals;
		bool showTopAxis;
		bool showSingleChannelImage;
		static bool topAxisDefaultSet;

		static const QString PLOT_TYPE_FLUX;
		static const QString PLOT_TYPE_MEAN;
		static const QString PLOT_TYPE_MEDIAN;
		static const QString PLOT_TYPE_SUM;

		static const QString RADIO_VELOCITY;
		static const QString VELOCITY;
		static const QString CHANNEL;
		static const QString OPTICAL;
		static const QString AIR;
		static const QString FRAME_REST;
		static const QString PERSIST_FREQUENCY_BOTTOM;
		static const QString PERSIST_FREQUENCY_TOP;
		static const QString IMAGE_MISSING_ERROR;
		static const QString MISSING_REGION_ERROR;
		static const QString NO_PROFILE_ERROR;
		static const QString REGION_ELLIPSE;
		static const QString REGION_RECTANGLE;
		static const QString REGION_POINT;
		static const QString REGION_POLY;


		class spectra_info {
		public:
			spectra_info( ) { }
			spectra_info( const QString &s ) : shape_(s) { }
			spectra_info( const spectra_info &other ) : shape_(other.shape_) { }
			const spectra_info &operator=( const spectra_info &other ) {
				shape_ = other.shape_;
				return *this;
			}
			const QString &shape( ) {
				return shape_;
			}
		private:
			QString shape_;
		};


		typedef std::map<int,spectra_info> SpectraInfoMap;
		const int NO_REGION_ID;
		int current_region_id;
		SpectraInfoMap spectra_info_map;
		ProfileType profileType;
		String lastShape;

		ColorSummaryWidget* colorSummaryWidget;
		LegendPreferences* legendPreferencesDialog;
		QtCanvas* pixelCanvas;
		QtProfilePrefs* profilePrefs;
		SmoothPreferences* smoothWidget;
		int frameIndex;
		bool newOverplots;
		pair<double,double> getMaximumTemperature();
		void postConversionWarning( QString unitStr);
		void adjustPlotUnits( );
	private slots:
		void changeTopAxis();
		void updateXAxisLabel( const QString &text, QtPlotSettings::AxisIndex axisIndex );
		void setDisplayYUnits( const QString& unitStr );
		void channelSelect(float xval);
		void channelRangeSelect( float channelStart, float channelEnd );
		void showSmoothingPreferences();
		void replotCurves();
	};

}
#endif

