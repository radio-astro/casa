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
#ifndef BINPLOTWIDGET_QO_H
#define BINPLOTWIDGET_QO_H

#include <QtGui/QWidget>

#include <guitools/Histogram/BinPlotWidget.ui.h>
#include <guitools/Histogram/HeightSource.h>
#include <images/Regions/ImageRegion.h>
#include <casa/aips.h>
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <vector>
#include <QDebug>
#include <QMenu>
using namespace std;

class QwtPlotMarker;
class QwtPlotCurve;


namespace casa {

template <class T> class ImageInterface;
template <class T> class SubImage;
template <class T> class ImageHistograms;
class FitWidget;
class RangePicker;
class ToolTipPicker;
class HistogramMarkerGaussian;
class HistogramMarkerPoisson;
class RangeControlsWidget;
class PlotModeWidget;

/**
 * Display a histogram of intensity vs count.  Functionality is pluggable
 * and can either be put in or left out:  selection of a range along the
 * intensity axis of the histogram (rangeControls), fitting a curve to the
 * histogram (fitControls), changing how the histogram is displayed (plotControls).
 */

/**
 * HeightSource is intended to be an interface to reduce the coupling being
 * this class and a subclass that needs to know the height of the plot in
 * order to correctly draw a marker.
 */
class BinPlotWidget : public QWidget, public HeightSource {
    Q_OBJECT

friend class RangePicker;

public:
    /**
     * fitControls: true allows the user to fit curves to the Histogram.
     * rangeControls: true allows the user to specify a min/max value on the histogram.
     * plotModeControls: true allows the user to specify whether a histogram should be
     * 		created for an image, a region, or multiple regions.
     */
    BinPlotWidget( bool fitControls, bool rangeControls, bool plotModeControls, QWidget* parent = 0 );
    bool setImage( ImageInterface<Float>* img );
    bool setImageRegion( const ImageRegion* imageRegion, int id );
    void deleteImageRegion( int id );
    void imageRegionSelected( int id );
    pair<double,double> getMinMaxValues() const;
    void setMinMaxValues( double minValue, double maxValue, bool updateGraph=true );


    //Customizing the display
    void setDisplayPlotTitle( bool display );
    void setDisplayAxisTitles( bool display );
    void setHistogramColor( QColor color );
    void setFitEstimateColor( QColor color );
    void setFitCurveColor( QColor color );
    void setAxisLabelFont( int size );
    void addZoomActions( QMenu* zoomMenu );
    void addDisplayActions( QMenu* menu );
    void addPlotModeActions( QMenu* menu );
    void setPlotMode( int mode );
    bool isEmpty() const;
    ~BinPlotWidget();

signals:
	void postStatusMessage( const QString& msg );
	void rangeChanged();

public slots:
	void fitModeChanged();
	void setDisplayStep( bool display );
	void setDisplayLog( bool display );
	void clearFit();
	//Saving the Histogram
	void toAscii( const QString& filePath );
	void toPing( const QString& filtPath, int width, int height );


protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void keyPressEvent( QKeyEvent* event );

private slots:
	void lineMoved( const QPoint& pt );
	void lineSelected();
	void clearRange();
	void defineCurve();
	void minMaxChanged();
	void showContextMenu( const QPoint& pt );
	void centerPeakSpecified();
	void lambdaSpecified();
	void fwhmSpecified();
	void fitDone( const QString& msg );

	void resetGaussianFitMarker();
	void resetPoissonFitMarker();
	void zoom95( bool zoom );
	void zoom98( bool zoom );
	void zoom995( bool zoom );
	void zoom999( bool zoom );
	void zoomNeutral( bool zoom );
	void zoomRange( bool rangeZoom );

	void imageModeSelected( bool enabled );
	void regionModeSelected( bool enabled );
	void regionAllModeSelected( bool enabled );

private:
	BinPlotWidget( const BinPlotWidget& );
	BinPlotWidget& operator=( const BinPlotWidget& );
	void initializeFitWidget( bool fitControls );
	void initializePlotControls();
	void initializePlotModeControls( bool enable );
	void initializeGaussianFitMarker();
	void initializePoissonFitMarker();
	void initializeRangeControls( bool rangeControls);
	void clearGaussianFitMarker();
	void clearPoissonFitMarker();
	void clearRegionList();
	bool makeHistogram();
	void rectangleSizeChanged();
	void resetAxisTitles();
	void resetPlotTitle();
	bool resetImage();
	bool resetRegion(int id = -1);
	void resetRectangleMarker();
	void defineCurveLine();
	void defineCurveHistogram();
	QwtPlotCurve* addCurve( QVector<double>& xValues, QVector<double>& yValues );
	void clearCurves();
	double checkLogValue( double value ) const;
	void setValidatorLimits();
	bool isPlotContains( int x, int y );
	virtual int getCanvasHeight();
	int getPeakIndex() const;
	float getTotalCount() const;
	void zoom( float percent );
	void zoomRangeMarker( double startValue, double endValue );
    Ui::BinPlotWidgetClass ui;

    enum ContextMenuMode{ ZOOM_CONTEXT,DISPLAY_CONTEXT,FIT_CONTEXT };
    ContextMenuMode contextMenuMode;

    bool displayPlotTitle;
    bool displayAxisTitles;

    QColor curveColor;
    QColor selectionColor;
    QColor fitEstimateColor;
    QColor fitCurveColor;

    //Histogram & data
    vector<float> xVector;
    vector<float> yVector;
    QList<QwtPlotCurve*> curves;
    ImageHistograms<Float>* histogramMaker;
    ImageInterface<Float>* image;
    QMap< int, SubImage<Float>* > regionMap;
    QwtPlot binPlot;
    const QString NO_DATA;
    const QString NO_DATA_MESSAGE;

    //Specifying a range with the histogram
    QwtPlotPicker* dragLine;
    RangePicker* rectMarker;
    ToolTipPicker* toolTipPicker;
    RangeControlsWidget* rangeControlWidget;
    QMenu contextMenuZoom;
    QAction zoomRangeAction;
    QAction zoom95Action;
    QAction zoom98Action;
    QAction zoom995Action;
    QAction zoom999Action;
    QAction zoomNeutralAction;
    double rectX;
    double rectWidth;

    //Fitting the histogram
    QAction lambdaAction;
    QAction centerPeakAction;
    QAction fwhmAction;
    QMenu contextMenu;
    FitWidget* fitWidget;
    QwtPlotCurve* fitCurve;
    QPoint fitPosition;
    HistogramMarkerGaussian* fitEstimateMarkerGaussian;
    HistogramMarkerPoisson* fitEstimateMarkerPoisson;

    //Plot Display
    QAction stepFunctionAction;
    QAction logAction;
    QAction regionModeAction;
    QAction imageModeAction;
    QAction regionAllModeAction;
    bool displayStep;
    bool displayLog;
    enum PlotMode {REGION_MODE,IMAGE_MODE,REGION_ALL_MODE};
    PlotMode plotMode;
    QMenu contextMenuDisplay;

    PlotModeWidget* plotModeWidget;
};

}

#endif // THRESHOLDINGBINPLOTWIDGET_H
