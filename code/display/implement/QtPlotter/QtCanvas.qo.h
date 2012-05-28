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
#include <display/QtPlotter/WorldCanvasTranslator.h>
#include <display/QtPlotter/ProfileFitMarker.h>

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
#include <QLabel>
#include <QMenu>
#include <map>
#include <vector>
#include <graphics/X11/X_exit.h>


namespace casa { 

typedef std::vector<double> CurveData;
typedef std::vector<double> ErrorData;

#define QT_DIAMOND_SIZE 5

class GraphLabel
{
public:
	QString text;
	QString fontName;
	int fontSize;
	QColor color;
	GraphLabel() : text(""), fontName("Helvetica [Cronyx]"), fontSize(12), color(Qt::blue) {}
};

class QtCanvas : public QWidget, public WorldCanvasTranslator
{
	Q_OBJECT
public:
	QtCanvas(QWidget *parent = 0);

	void setPlotSettings(const QtPlotSettings &settings);
	void setCurveData(int id, const CurveData &data, const ErrorData &error=ErrorData(), const QString& lb="");
	void setTopAxisRange(const Vector<Float> &xValues, bool topAxisDescending );
	CurveData* getCurveData(int);
	ErrorData* getCurveError(int id);
	QString getCurveName(int);
	int getLineCount();
	//void clearCurve(int id = -1);
	void clearCurve();
	void setDataRange();
	void setImageMode(bool);
	//void setPixmap(const QImage&);
	QPixmap* graph();
	void drawBackBuffer(QPainter *);

	//Plotting curves
	void plotPolyLines(QString);
	template<class T> void plotPolyLine(const Vector<T>&, const Vector<T>&);
	void plotPolyLine(const Vector<Float> &x, const Vector<Float> &y, const Vector<Float> &e,
			const QString& lb="");
	void addPolyLine(const Vector<Float> &x, const Vector<Float> &y,
			const QString& lb="");
	template<class T> void plotPolyLine(const Matrix<T> &verts);
	// template<class T>
	// void drawImage(const Matrix<uInt> &data, Matrix<uInt> *mask);
	// void drawImage(const Matrix<uInt> &data);
	//QColor getLinearColor(double);
	QColor getDiscreteColor(const int &d);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	~QtCanvas();
	void increaseCurZoom();
	int getCurZoom();
	int getZoomStackSize();

	void setTitle(const QString &text, int fontSize = 12,   int iclr = 1,
			const QString &font = "Helvetica [Cronyx]");
	QString getTitle(){return title.text;};

	void setXLabel(const QString &text, int fontSize = 10,  int iclr = 1,
			const QString &font = "Helvetica [Cronyx]", QtPlotSettings::AxisIndex axisIndex=QtPlotSettings::xBottom);
	void setYLabel(const QString &text, int fontSize = 10,  int iclr = 1,
			const QString &font = "Helvetica [Cronyx]");
	void setWelcome(const QString &text, int fontSize = 14, int iclr = 1,
			const QString &font = "Helvetica [Cronyx]");
	void setAutoScaleX(int a) {autoScaleX = a;}
	void setAutoScaleY(int a) {autoScaleY = a;}
	void setShowGrid(int a)   {showGrid = a; refreshPixmap();}
	int getAutoScaleX( ) {return autoScaleX;}
	int getAutoScaleY( ) {return autoScaleY;}
	int getShowGrid( )   {return showGrid;}
	void setPlotError(int a)  {plotError = a; setDataRange();}
	void setShowTopAxis( bool showAxis );
	void setShowToolTips( bool toolTipsVisible ){ showToolTips = toolTipsVisible; }
	bool getShowTopAxis() const { return showTopAxis; }
	bool getShowToolTips() const { return showToolTips; }
	void setToolTipYUnit( const QString& yUnit ){ toolTipYUnit = yUnit; }
	void setToolTipXUnit( const QString& xUnit ){ toolTipXUnit = xUnit; }
	void setProfileFitMarkerCenterPeak( int index, double center, double peak);
	void setProfileFitMarkerFWHM( int index, double fwhm, double fwhmHeight);

public slots:
   void zoomIn();
   void zoomOut();
   void zoomNeutral();
   void gaussianCenterPeakSelected();
   void gaussianFWHMSelected();

signals:
	void xRangeChanged(float xmin, float xmax);
	void channelSelect(float xvalue);
	void specFitEstimateSpecified( double xValue, double yValue, bool centerPeak );

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void wheelEvent(QWheelEvent *event);

protected:
	void updateRubberBandRegion();
	void updatexRangeBandRegion();
	void drawGrid(QPainter *painter);
	void drawTicks(QPainter *painter);
	void drawLabels(QPainter *painter);
	void drawWelcome(QPainter *painter);
	void drawCurves(QPainter *painter);
	void drawRects(QPainter *painter);
	void drawxRange(QPainter *painter);
	void defaultZoomIn();
	void defaultZoomOut();
	void refreshPixmap();

private:
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
	 * the user can use to specify Gaussian estimates.
	 */
	void initGaussianEstimateContextMenu();

	enum { MARGIN = 80 , FRACZOOM=20};

	GraphLabel title;
	GraphLabel xLabel[2];
	GraphLabel yLabel;
	GraphLabel welcome;


	std::map<int, QString> legend;
	std::map<int, CurveData> curveMap;
	std::map<int, ErrorData> errorMap;
	std::vector<QtPlotSettings> zoomStack;
	std::map<int, CurveData> markerStack;
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

	int autoScaleX;
	int autoScaleY;
	int plotError;
	int showGrid;

	int xRectStart;
	int xRectEnd;


	bool showTopAxis;
	bool showToolTips;
	QString toolTipXUnit;
	QString toolTipYUnit;

	//Profile Fitting Gaussian Estimation
	QMenu gaussianContextMenu;
	double gaussianEstimateX;
	double gaussianEstimateY;
	QList<ProfileFitMarker> profileFitMarkers;

	ProfileFitMarker getMarker( int index ) const;
	void setMarker( int index, ProfileFitMarker& marker );

};

}
#endif

