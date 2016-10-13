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



		QtProfile(SHARED_PTR<casacore::ImageInterface<float> > img, const char *name = 0,
		          QWidget *parent = 0, std::string rcstr="prf");

		~QtProfile();
		casacore::MFrequency::Types determineRefFrame(SHARED_PTR<casacore::ImageInterface<float> > img, bool check_native_frame = false );

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
		void postStatus( casacore::String status );
		casacore::Vector<float> getXValues() const;
		casacore::Vector<float> getYValues() const;
		casacore::Vector<float> getZValues() const;
		casacore::Vector<double> getRegionXValues() const;
		casacore::Vector<double> getRegionYValues() const;
		QString getYUnit() const;
		QString getYUnitPrefix() const;
		casacore::String getXAxisUnit() const;
		QString getFileName() const;
		QString getImagePath() const;
		casacore::String getRegionShape() const;
		SHARED_PTR<const casacore::ImageInterface<float> > getImage( const QString& imageName="") const;
		virtual bool getBeamInfo( const QString& curveName, double& beamAngle, double& beamArea ) const;
		void getPixelBounds(casacore::Vector<double>& pixelX, casacore::Vector<double>& pixelY) const;
		void persist( const QString& key, const QString& value );
		QString read( const QString & key ) const;
		void imageCollapsed(casacore::String path, casacore::String dataType, casacore::String displayType,
				bool autoRegister, bool tmpData, SHARED_PTR<casacore::ImageInterface<float> > img);
		void setPosition( const QList<double>& xValues, const QList<double>& yValues );
		void processTrackRecord( const casacore::String& dataName, const casacore::String& positionInfo );
		virtual casacore::MFrequency::Types getReferenceFrame() const;
		QString getBrightnessUnit( SHARED_PTR<casacore::ImageInterface<float> > img ) const;
		typedef std::pair<QString, SHARED_PTR<casacore::ImageInterface<float> > > OverplotInterface;

		static const casacore::String SHAPE_ELLIPSE;
		static const casacore::String SHAPE_RECTANGLE;
		static const casacore::String SHAPE_POINT;
		static const casacore::String SHAPE_POLY;
		static const QString FREQUENCY;
		static const QString CHANNEL;

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
		void resetProfile(SHARED_PTR<casacore::ImageInterface<float> > img, const char *name = 0);
		void wcChanged( const casacore::String,
		                const casacore::Vector<double>, const casacore::Vector<double>,
		                const casacore::Vector<double>, const casacore::Vector<double>,
		                const ProfileType);
		void redraw( );
		void changePlotType(const QString &text);
		void changeErrorType(const QString &text);

		void changeAxis(casacore::String xa, casacore::String ya, casacore::String za, std::vector<int>);
		void changeSpectrum(casacore::String spcTypeUnit, casacore::String spcRval, casacore::String spcSys);

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
		bool isImageSupported(SHARED_PTR<casacore::ImageInterface<float> > img );

	signals:
		void hideProfile();
		void coordinateChange(const casacore::String&);
		void showCollapsedImg(casacore::String path, casacore::String dataType, casacore::String displayType,
				bool autoRegister, bool tmpData, SHARED_PTR<casacore::ImageInterface<float> > img);
		void channelSelect( int channelIndex );
		void adjustPosition( double tlcx, double tlcy, double brcx, double brcy );
		void movieChannel( int startChannel, int endChannel );
		void reloadImages();

	private:
		void stringToPlotType(const QString &text,  QtProfile::PlotType &pType);
		void stringToErrorType(const QString &text, QtProfile::ErrorType &eType);
		void fillPlotTypes(const SHARED_PTR<casacore::ImageInterface<float> > img);
		void getcoordTypeUnit(casacore::String &ctypeUnitStr, casacore::String &cTypeStr, casacore::String &unitStr);
		void printIt(QPrinter*);
		bool exportASCIISpectrum(QString &fn);
		bool exportFITSSpectrum(QString &fn);
		void messageFromProfile(QString &msg);
		void setUnitsText( casacore::String unitStr );
		void resetYUnits( const QString& units);
		void outputCurve( int k, QTextStream& ts, float scaleFactor );
		int getFreqProfileTabularIndex(SHARED_PTR<const casacore::ImageInterface<float> > img);
		casacore::SpectralCoordinate resetTabularConversion(SHARED_PTR< const casacore::ImageInterface<float> > imagePtr, bool& valid);
		bool isSpectralAxis() const;
		void initializeSpectralProperties();
		casacore::SpectralCoordinate getSpectralAxis( SHARED_PTR<const casacore::ImageInterface<float> > imagePtr, bool& valid );
		void resetXUnits( bool spectralAxis);
		void updateSpectralReferenceFrame();

		int computeCB( const casacore::String& xa, const casacore::String& ya, const casacore::String& za );
		void getBeamInfo( SHARED_PTR<const casacore::ImageInterface<float> > imagePtr, double& beamArea, double& beamSolidAngle) const;
		casacore::SpectralCoordinate getSpectralCoordinate( SHARED_PTR<const casacore::ImageInterface<float> > imagePtr, bool& valid );
		bool parseRestFrequency( const casacore::String& restStr, casacore::Quantity& result ) const;

		bool generateProfile( casacore::Vector<float>& resultXValues, casacore::Vector<float>& resultYValues,
					SHARED_PTR<casacore::ImageInterface<float> > imagePtr,
					const casacore::Vector<double>& regionX, const casacore::Vector<double>& regionY, casacore::String shape,
					QtProfile::ExtrType combineType, casacore::String& unit, const casacore::String& coordinateType,
					int qualityAxis, casacore::String restFreq="", const casacore::String& frame="");

		pair<casacore::Vector<float>, casacore::Vector<float> > convertIntensity( const casacore::Vector<float>& sourceXVals, const casacore::Vector<float>& sourceYVals,
							SHARED_PTR<casacore::ImageInterface<float> > imagePtr, const QString& xUnits,
							const QString& yUnitsOld, const QString& yUnitsNew );
		void saveAsPDF( const QString& fileName );
		/**
		 * Returns false if first vector value is greater than the last
		 * vector value; otherwise returns true.
		 */
		bool isAxisAscending(const casacore::Vector<float>& axisValues ) const;
		bool isVelocityUnit( const QString& unit ) const;
		bool isFrequencyUnit( const QString& unit ) const;
		bool isWavelengthUnit( const QString& unit ) const;
		void setTitle( const QString& shape );
		void setTitle(const ProfileType ptype);
		void copyToLastEvent( const casacore::String& c, const casacore::Vector<double> &px,
		                      const casacore::Vector<double> &py,
		                      const casacore::Vector<double> &wx,
		                      const casacore::Vector<double> &wy );
		void setPlotType( int wcArraySize );
		bool checkCube();
		void assignCoordinate(const casacore::String& c);
		void initializeCoordinateVectors(const casacore::Vector<double> &px, const casacore::Vector<double> &py,
		                                 const casacore::Vector<double> &wx, const casacore::Vector<double> &wy, casacore::Vector<double> &pxv,
		                                 casacore::Vector<double> &pyv, casacore::Vector<double> &wxv, casacore::Vector<double> &wyv) const;
		void setPositionStatus(const casacore::Vector<double> &pxv, const casacore::Vector<double> &pyv,
		                       const casacore::Vector<double> &wxv, const casacore::Vector<double> &wyv );
		/*bool getFrequencyProfileWrapper( SHARED_PTR<casacore::ImageInterface<float> >& img, const casacore::Vector<double> &wxv, const casacore::Vector<double> &wyv,
		                                 casacore::Vector<float> &z_xval, casacore::Vector<float> &z_yval,
		                                 const casacore::String& xytype, const casacore::String& specaxis,
		                                 const int& whichStokes, const int& whichTabular,
		                                 const int& whichLinear, casacore::String& xunits,
		                                 const casacore::String& specFrame, QtProfile::ExtrType combineType,
		                                 const int& whichQuality, const casacore::String& restValue,
		                                 const casacore::String& shape);*/
		bool assignFrequencyProfile( const casacore::Vector<double> &wxv, const casacore::Vector<double> &wyv,
		                             const casacore::String& coordinateType, casacore::String& xAxisUnit,
		                             casacore::Vector<float> &z_xval, casacore::Vector<float> &z_yval, const casacore::String& shape );
		bool setErrorPlotting( const casacore::Vector<double> &wxv, const casacore::Vector<double> &wyv);
		void storeCoordinates( const casacore::Vector<double> pxv, const casacore::Vector<double> pyv,
		                       const casacore::Vector<double> wxv, const casacore::Vector<double> wyv );
		void addCanvasMainCurve( const casacore::Vector<float>& xVals, const casacore::Vector<float>& yVals,
		                         const QString& label, double beamAngle, double beamArea,
		                         casacore::SpectralCoordinate coord);
		bool adjustTopAxisSettings();
		void initializeXAxisUnits();
		void setXAxisUnits();
		void initSmoothing();
		void setPixelCanvasYUnits( const QString& yUnitPrefix, const QString& yUnit );
		void toggleAction( QAction* action );
		int scaleAxis();
		void addImageAnalysisGraph( const casacore::Vector<double> &wxv, const casacore::Vector<double> &wyv, int ordersOfM );
		void initializeSolidAngle() const;
		//Conversion
		QString getRaDec(double x, double y);

		//Finds the channel index closest to the passed in value.
		int findNearestChannel( float xval ) const;
		void initPreferences();
		void updateAxisUnitCombo( const QString& textToMatch, QComboBox* axisUnitCombo );
		void setYUnitConversionVisibility( bool visible );
		SHARED_PTR<casacore::ImageInterface<float> > image;

		//For deciding whether or not it makes sense to show the top axis when
		//multiple images are loaded.
		bool isXUnitsMatch(const QString& matchUnits);
		bool isFrequencyMatch();
		bool isVelocityMatch();
		int getChannelCount( SHARED_PTR<casacore::ImageInterface<float> >& img);
		SHARED_PTR<casacore::ImageInterface<float> > findImageWithMaximumChannels();
		void restrictTopAxisOptions( bool restrictOptions, const QString& bottomUnits, bool allowFrequency = true,
				bool allowVelocity=true );
		double getUnitsPerChannel( SHARED_PTR<casacore::ImageInterface<float> > img , bool* ok, const QString& matchUnits );
		QString readTopAxis() const;
		void persistTopAxis( const QString& units );
		void assignProfileType( const casacore::String& shape, int regionPointCount );
		void addOverplotToCanvas( SHARED_PTR<casacore::ImageInterface<float> > imagePtr, const casacore::Vector<float>& xVals, const
					casacore::Vector<float>& yVals, const QString& ky );
		bool isProfileChanged( const casacore::String& c, const casacore::Vector<double> & px, const casacore::Vector<double>& py,
					const casacore::Vector<double>& wx, const casacore::Vector<double>& wy,
					const ProfileType pType );

		bool _generateProfile( casacore::Vector<float>& resultXValues, casacore::Vector<float>& resultYValues,
					SHARED_PTR<const casacore::ImageInterface<float> > imagePtr,
					const casacore::Vector<double>& regionX, const casacore::Vector<double>& regionY, casacore::String shape,
					QtProfile::ExtrType combineType, casacore::String& unit, const casacore::String& coordinateType,
					casacore::String restFreq, const casacore::String& frame);

		//Handle custom spectral reference frames such as REST and Undefined
		//for which conversions are not possible.
		bool customizeSpectralReferenceFrame( const QString& specialType );
		QList<OverplotInterface> *over;
		const casacore::String WORLD_COORDINATES;
		casacore::String coordinate;
		casacore::String coordinateType;
		casacore::String xaxisUnit;
		casacore::String ctypeUnit;
		casacore::String spcRefFrame;

		//Rest frequency quantity and unit.
		casacore::String cSysRval;

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

		casacore::Vector<float> z_xval;
		casacore::Vector<float> z_yval;
		casacore::Vector<double> lastPX, lastPY;
		casacore::Vector<double> lastWX, lastWY;
		casacore::Vector<float> z_eval;
		QString region;

		casacore::String last_event_cs;
		casacore::Vector<double> last_event_px, last_event_py;
		casacore::Vector<double> last_event_wx, last_event_wy;

		// connection to rc file
		casacore::Casarc &rc;
		// rc id for this panel type
		std::string rcid_;

		QtProfile::PlotType  itsPlotType;
		QtProfile::ErrorType itsErrorType;


		casacore::LogIO *itsLog;
		int ordersOfM_;
		bool newCollapseVals;
		bool showTopAxis;
		bool showSingleChannelImage;
		static bool topAxisDefaultSet;

		static const QString PLOT_TYPE_FLUX;
		static const QString PLOT_TYPE_MEAN;
		static const QString PLOT_TYPE_MEDIAN;
		static const QString PLOT_TYPE_SUM;

		static const QString VELOCITY;
		static const QString OPTICAL;
		static const QString AIR;
		static const QString FRAME_REST;
		static const QString FRAME_NONE;
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
		casacore::String lastShape;

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

