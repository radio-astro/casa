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

#ifndef QTCANVAS_H
#define QTCANVAS_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>

#include <display/QtPlotter/QtPlotSettings.h>
#include <display/QtPlotter/CanvasCurve.h>
#include <display/QtPlotter/WorldCanvasTranslator.h>
#include <display/QtPlotter/ProfileFitMarker.h>
#include <display/QtPlotter/MolecularLine.h>
#include <display/QtPlotter/canvasMode/CanvasMode.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <QToolButton>
#include <QDialog>
#include <QPixmap>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QLabel>
#include <QMenu>
#include <map>
#include <vector>
#include <graphics/X11/X_exit.h>

namespace casa {

	class Annotation;
	class CanvasModeFactory;

#define QT_DIAMOND_SIZE 5

	class GraphLabel {
	public:
		QString text;
		QString fontName;
		int fontSize;
		QColor color;
		GraphLabel() : text(""), fontName(""), fontSize(12), color(Qt::blue) {}
	};

	class QtCanvas : public QWidget, public WorldCanvasTranslator {
		Q_OBJECT

		friend class CanvasModeAnnotation;
		friend class CanvasModeRangeSelection;
		friend class CanvasModeChannel;
		friend class CanvasModeContextMenu;
		friend class CanvasModeZoom;

	public:
		QtCanvas(QWidget *parent = 0);

		void setPlotSettings(const QtPlotSettings &settings);
		void setTopAxisRange(const Vector<Float> &xValues, bool topAxisDescending );
		void setFrameMarker( float framePositionX );
		void setZoomRectColor( QColor color );
		//Returns the spectrum as it is currently displayed in the canvas (with
		//possible zooming).
		void getCanvasDomain( double* minValue, double* maxValue, QString& units);

		CurveData getCurveData(int);
		ErrorData getCurveError(int id);
		QString getCurveName(int id);
		void setCurveName(int id, const QString& name );
		void curveColorsChanged();
		QColor getCurveColor( int id );
		CanvasCurve getCurve( const QString& curveName );
		int getLineCount();
		void curveLabelsChanged();
		void clearCurve();
		void clearFitCurves();
		void clearMolecularLines( bool refresh = true );
		void setDataRange();
		void setImageMode(bool);
		QPixmap* graph();
		void drawBackBuffer(QPainter *);
		//Draws a vertical line indicating the current frame.
		void drawFrameMarker( QPainter* );
		QString getUnits( QtPlotSettings::AxisIndex axisIndex = QtPlotSettings::xBottom );
		QList<MolecularLine*> getMolecularLines() const;

		//Plotting curves
		void plotPolyLines(QString);
		template<class T> void plotPolyLine(const Vector<T>&, const Vector<T>&,
				double beamAngle, double beamArea, SpectralCoordinate coord);
		void plotPolyLine(const Vector<Float> &x, const Vector<Float> &y, const Vector<Float> &e,
		                  double beamAngle, double beamArea, SpectralCoordinate coord, const QString& lb="");
		enum ColorCategory { TITLE_COLOR, CURVE_COLOR, CURVE_COLOR_PRIMARY,
		                     CURVE_COLOR_SECONDARY, WARNING_COLOR, CURVE_TRADITIONAL};
		void addPolyLine(const Vector<Float> &x, const Vector<Float> &y,
				double beamAngle, double beamArea, SpectralCoordinate coord,
		                 const QString& lb="", ColorCategory colorCategory=CURVE_COLOR );
		void addMolecularLine(MolecularLine* molecularLine );
		QList<QString> getMolecularLineNames() const;

		template<class T> void plotPolyLine(const Matrix<T> &verts, double beamAngle,
				double beamArea, SpectralCoordinate coord, double restValue );

		QSize minimumSizeHint() const;
		QSize sizeHint() const;
		~QtCanvas();


		void setTitle(const QString &text, int fontSize = 13,const QString &font = FONT_NAME);
		QString getTitle() {
			return title.text;
		};

		bool isShowChannelLine();
		void setShowChannelLine( bool showLine );
		void setChannelLineColor( QColor color );

		void setXLabel(const QString &text, int fontSize = 10,
		               const QString &font = FONT_NAME, QtPlotSettings::AxisIndex axisIndex=QtPlotSettings::xBottom);
		void setWelcome(const QString &text, int fontSize = 14,
		                const QString &font = FONT_NAME);
		void setAutoScaleX(bool autoScale);
		void setAutoScaleY(bool autoScale);
		void setShowGrid(int a)   {
			showGrid = a;
			refreshPixmap();
		}
		bool getAutoScaleX( ) {
			return autoScaleX;
		}
		bool getAutoScaleY( ) {
			return autoScaleY;
		}
		int getShowGrid( )   {
			return showGrid;
		}
		void setPlotError(int a)  {
			plotError = a;
			setDataRange();
		}

		//Whether or not to show a top axis on the plot.
		void setShowTopAxis( bool showAxis );
		/*bool getShowTopAxis() const {
			return showTopAxis;
		}*/

		//Even if the user wants to show a top axis, we won't do it
		//when there are multiple curves which are not compatible in their
		//x-axis units.
		void setTopAxisCompatible( bool compatible );

		//Whether or not to show tool tips with the x,y coordinates
		//of the points on the plot.
		void setShowToolTips( bool toolTipsVisible ) {
			showToolTips = toolTipsVisible;
		}
		bool getShowToolTips() const {
			return showToolTips;
		}

		//Sets the coordinates for the x and y points that are displayed
		//as tooltips.
		void setToolTipXUnit( const QString& xUnit ) {
			toolTipXUnit = xUnit;
		}

		//Stores the location of a (center,peak) point that represents an initial
		//Gaussian estimate for spectral line fitting.
		void setProfileFitMarkerCenterPeak( int index, double center, double peak);

		//Stores the fwhm and the y value of the fwhm for an initial Gaussian estimate
		//for spectral line fitting.
		void setProfileFitMarkerFWHM( int index, double fwhm, double fwhmHeight);

		//Whether or not to plot a line graph or instead plot the points as a
		//step function.
		bool isDisplayStepFunction() const;
		void setDisplayStepFunction( bool displayAsStepFunction );

		//Used for customizing the curve colors on the canvas.
		void setTraditionalCurveColors( const QList<QString>& colors );
		void setMainCurveColors( const QList<QString>& colors );
		void setFitCurveColors( const QList<QString>& colors );
		void setSummaryCurveColors( const QList<QString>& colors );

		//If this flag is set, only 'traditional' colors will be used.
		void setTraditionalColors( bool traditionalColors );
		static const QString FONT_NAME;

		//Customization of the curve legend.
		void setShowLegend( bool visible );
		bool isShowLegend() const;
		void setLegendPosition( int position );
		int getLegendPosition() const;

		//Set the yaxis units the image is using ( units the data we are getting is in).
		void setImageYUnits( const QString& imageUnits );


		enum TaskMode {/*UNKNOWN_MODE,*/ SPECTRAL_LINE_MODE, LINE_OVERLAY_MODE, MODE_COUNT };


		//Units we will be displaying the y-axis in which may be different
		//from the intrinsic image units.
		QString getDisplayYUnits();
		void setDisplayYUnits( const QString& displayUnits );

		int getCurZoom();
		int getZoomStackSize();

	public slots:
		void zoomIn();
		void zoomOut();
		void zoomNeutral();
		void changeTaskMode( int mode );
		void createAnnotationText();
		void rangeSelectionMode();
		void channelPositioningMode();

	signals:
		void xRangeChanged(double xmin, double xmax);
		void channelSelect(float xvalue);
		void specFitEstimateSpecified( double xValue, double yValue, bool centerPeak );
		void findRedshiftAt( double center, double peak );
		void channelRangeSelect( float startVal, float endVal );
		void curvesChanged();
		void clearPaletteModes();
		void togglePalette( int modeIndex );

	protected:
		void paintEvent(QPaintEvent *event);
		void resizeEvent(QResizeEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void keyReleaseEvent(QKeyEvent *event);
		void wheelEvent(QWheelEvent *event);

		void updateRubberBandRegion();
		void updatexRangeBandRegion();
		void drawGrid(QPainter *painter);
		void drawTicks(QPainter *painter);
		void drawLabels(QPainter *painter);
		void drawWelcome(QPainter *painter);
		void drawCurves(QPainter *painter);

	private slots:
		void deleteSelectedAnnotation();
		void editSelectedAnnotation();
		void gaussianCenterPeakSelected();
		void gaussianFWHMSelected();
		void findRedshift();

	private:

		//Zoom functionality
		void zoomYBasedOnX( QtPlotSettings& settings, double zoomFactor, bool zoomIn );
		pair<double,double> getRangeFor( double zoomFactor, bool zoomIn, double minX, double maxX );
		void increaseCurZoom();
		void defaultZoomIn();
		void defaultZoomOut();

		/**
		 * Stores the curve information for later plotting.
		 * @param id a unique identifier for the curve
		 * @param data a vector containing the (x,y) points that make up the curve.
		 * @param error a vector containing errors associated with the curve.
		 * @param lb a human readable identifier for the curve.
		 * @param colorCategory an indication of the type of curve this is (main curve,
		 * 		secondary spectral fitting curve, etc).
		 */
		void setCurveData(int id, const CurveData &data, double beamAngle, double beamArea,
				SpectralCoordinate coord, const ErrorData &error=ErrorData(),
		        const QString& lb="", ColorCategory colorCategory = CURVE_COLOR);
		void refreshPixmap();

		/**
		 * Attempts to display a tool tip indicated a data point corresponding
		 * to the mouse position, if there is such a point.
		 */
		void displayToolTip( QMouseEvent* event ) const;

		/**
		 * Adjusts the min and the max when they are a little too close
		 * together.
		 */
		void adjustExtremes( double* const min, double* const max ) const;

		/**
		 * Maps a y pixel coordinate to a world y value.
		 * @param pixelPosition the y-coordinate in pixels.
		 * @return double the corresponding world y value.
		 */
		double getDataY( int pixelPosition ) const;

		/**
		 * Maps an x pixel coordinate to a world x value.
		 * @param pixelPosition the x-coordinate in pixels.
		 * @return double the corresponding world x value.
		 */
		double getDataX( int pixelPosition ) const;

		//WorldCanvasTranslator interface methods.
		/**
		 * Maps a world x value to an x pixel coordinate.
		 * @param dataX an world X value.
		 * @return the pixel position of the passed in value.
		 */
		int getPixelX( double dataX ) const;

		/**
		 * Maps a world y value to a y pixel value.
		 * @param dataY a world y value.
		 * @return the pixel position of the world y value.
		 */
		int getPixelY ( double dataY ) const;

		/**
		 * Looks for a data point corresponding to the world
		 * cooordinates (x,y).  Returns an empty string if there
		 * is no close match.
		 * @param x an world x-coordinate.
		 * @param y a world y-coordinate.
		 * @return a String containing the curve data point that is a close
		 * 		match to the passed in coordinates or an empty string if there
		 * 		is no good match.
		 */
		QString findCoords( double x, double y ) const;

		/**
		 * Returns the height of the standard drawing rectangle.
		 */
		int getRectHeight() const;

		/**
		 * Returns the width of the standard drawing rectangle.
		 */
		int getRectWidth() const;

		/**
		 * Returns the bottom pixel coordinate for the standard drawing rectangle.
		 */
		int getRectBottom() const;

		/**
		 * Returns the standard left pixel coordinate for the standard drawing rectangle.
		 */
		int getRectLeft() const;

		/**
		 * Returns the appropriate x-Axis tick label corresponding to the tick
		 * identified by the given index and axis.
		 * @param index the index number of the tick withen a given axis.
		 * @param tickCount the number of ticks on the axis.
		 * @param axisIndex, an identifier for the axis (currently, top or bottom).
		 * @return the appropriate axis label for the tick.
		 */
		QString getXTickLabel( int index, int tickCount, QtPlotSettings::AxisIndex axisIndex ) const;

		/**
		 * Initializes the shift-right-click pop-up menu that
		 * the user can use to specify Gaussian estimates, edit annotations, search for redshift, etc,
		 * depending on context.
		 */
		void initContextMenu();
		void addDiamond( int x, int y, int diamondSize, QPainterPath& points ) const;
		bool storeClickPosition( QMouseEvent* event );
		int getIndependentCurveCount();
		int getLastAxis();
		void stripCurveTitleNumbers( QString& curveName ) const;
		int duplicateCurve( QString& targetLabel );
		void setYLabel(const QString &text, int fontSize = 10, const QString &font = FONT_NAME);

		//Annotation methods
		void selectAnnotation( QMouseEvent* event, bool select = true );
		bool isAnnotation( QMouseEvent* event ) const;
		Annotation* getSelectedAnnotation() const;

		//Mouse Events
		void storeActiveAnnotation( QMouseEvent* event );
		void selectChannel( QMouseEvent* event );
		void updateChannel( QMouseEvent* event );
		void startRangeX( QMouseEvent* event );
		void updateRangeX( QMouseEvent* event );
		void endRangeX( QMouseEvent* event );
		void startZoomRect( QMouseEvent* event );
		void updateZoomRect( QMouseEvent* event );
		void endZoomRect( QMouseEvent* event );
		void resetSelectedAnnotation( QMouseEvent* event );
		void drawMolecularLines( QPainter& painter );
		void drawAnnotations( QPainter& painter );

		//Plot Margins
		const int MARGIN_LEFT;
		const int MARGIN_BOTTOM;
		const int MARGIN_TOP;
		const int MARGIN_RIGHT;
		const int FRACZOOM;
		const double ZERO_LIMIT;

		GraphLabel title;
		GraphLabel xLabel[2];
		GraphLabel yLabel;
		GraphLabel welcome;

		std::map<int, CanvasCurve> curveMap;
		std::vector<QtPlotSettings> zoomStack;
		QList<MolecularLine*> molecularLineStack;
		std::pair<double,double> topAxisRange;

		int curZoom;
		int curMarker;
		bool rubberBandIsShown;
		bool xRangeIsShown;
		bool imageMode;
		bool xRangeMode;
		double xRangeStart;
		double xRangeEnd;
		QRect rubberBandRect;
		QRect xRangeRect;
		QPixmap pixmap;
		QPixmap backBuffer;
		Matrix<uInt> *pMask;

		QPoint currentCursorPosition;
		QColor xcursor;

		bool autoScaleX;
		bool autoScaleY;
		int plotError;
		int showGrid;

		int xRectStart;
		int xRectEnd;

		//Whether or not the user WANTS to see a top axis.
		bool showTopAxis;
		//We can only show the top axis in the case of multiple curves, when
		//all the curves have equally spaced units in their top axis.
		bool topAxisCompatible;
		bool showToolTips;
		bool showFrameMarker;
		bool displayStepFunction;
		QString toolTipXUnit;
		QString yUnitImage;
		QString yUnitDisplay;

		//Context Menu
		QMenu contextMenu;
		QAction* centerPeakAction;
		QAction* fwhmAction;
		QAction* findRedshiftAction;
		QAction* createAnnotationAction;
		QAction* editAnnotationAction;
		QAction* deleteAnnotationAction;
		void showContextMenu( QMouseEvent* event );

		//Profile Fitting Gaussian Estimation
		double gaussianEstimateX;
		double gaussianEstimateY;
		QList<ProfileFitMarker> profileFitMarkers;

		ProfileFitMarker getMarker( int index ) const;
		void setMarker( int index, ProfileFitMarker& marker );


		/**
		 * Returns the color to use for drawing a particular item on the canvas.
		 * The rational for the method is to always draw similar items in shades
		 * of the same color so that the user will learn to recognize those items
		 * by color.  Similar items should always have the same colorCategory.
		 * @param colorCategory the type of item that needs to be drawn.
		 */
		QColor getDiscreteColor(ColorCategory colorCategory, int id=0);
		QColor frameMarkerColor;
		QColor zoomColor;
		QList<QString> mainCurveColorList;
		QList<QString> fitCurveColorList;
		QList<QString> fitSummaryCurveColorList;
		QList<QString>  traditionalCurveColorList;

		int curveCount;
		int curveCountPrimary;
		int curveCountSecondary;

		//The optical spectral line fitting curve needs to use
		//the traditional color list rather than a customizable one.
		//That is the purpose of this flag.
		bool traditionalColors;

		//Whether the pixel canvas should show a legend with the curves.
		bool showLegend;

		//Where the curve legend should appear relative to this
		//canvas.
		int legendPosition;

		//The x location for the vertical line representing the
		//current frame position.
		float framePositionX;

		//Indicates the current work focus of the profiler
		TaskMode taskMode;

		float channelSelectValue;

		Annotation* selectedAnnotation;
		vector<Annotation*> annotations;

		//Returns true if the user is in the process of drawing an
		//annotation.
		bool isAnnotationActive() const;

		Annotation* getActiveAnnotation() const;
		CanvasMode* currentMode;
		CanvasModeFactory* modeFactory;
	};

}
#endif

