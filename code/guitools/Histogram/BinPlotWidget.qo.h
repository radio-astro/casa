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
#include <QMenu>
#include <guitools/Histogram/BinPlotWidget.ui.h>
#include <guitools/Histogram/HeightSource.h>
#include <images/Regions/ImageRegion.h>
#include <casa/aips.h>
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <vector>
#include <QDebug>
using namespace std;

class QwtPlotMarker;
class QwtPlotCurve;

namespace casa {

template <class T> class ImageInterface;
template <class T> class ImageHistograms;
class FitWidget;
class RangePicker;
class HistogramMarkerGaussian;
class PlotControlsWidget;
class RangeControlsWidget;


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

public:
    BinPlotWidget( bool plotControls, bool fitControls, bool rangeControls, QWidget* parent = 0 );
    bool setImage( ImageInterface<Float>* img );
    bool setImageRegion( const ImageRegion& imageRegion );
    pair<double,double> getMinMaxValues() const;
    void setMinMaxValues( double minValue, double maxValue, bool updateGraph=true );
    virtual int getCanvasHeight();

    //Customizing the display
    void setDisplayPlotTitle( bool display );
    void setDisplayAxisTitles( bool display );
    void setHistogramColor( QColor color );
    void setFitEstimateColor( QColor color );
    void setFitCurveColor( QColor color );
    void setAxisLabelFont( int size );
    ~BinPlotWidget();

public slots:
	void fitModeChanged();
	void setDisplayStep( bool display );
	void setDisplayLog( bool display );

protected:
    void resizeEvent( QResizeEvent* event );

private slots:
	void rectangleSelected(const QwtDoubleRect & rect);
	void clearRange();
	void defineCurve();
	void minMaxChanged();
	void showContextMenu( const QPoint& pt );
	void centerPeakSpecified();
	void lambdaSpecified();
	void fwhmSpecified();
	void fitDone();
	void resetGaussianFitMarker();

private:
	BinPlotWidget( const BinPlotWidget& );
	BinPlotWidget& operator=( const BinPlotWidget& );
	void initializeFitWidget( bool fitControls );
	void initializePlotControls( bool plotControls );
	void initializeGaussianFitMarker();
	void initializeRangeControls( bool rangeControls);
	bool makeHistogram();
	void rectangleSizeChanged();
	void resetAxisTitles();
	void resetPlotTitle();
	void resetRectangleMarker();
	void defineCurveLine();
	void defineCurveHistogram();
	QwtPlotCurve* addCurve( QVector<double>& xValues, QVector<double>& yValues );
	void clearCurves();
	void clearFitMarkers();
	double checkLogValue( double value ) const;
	void setValidatorLimits();
	bool isPlotContains( int x, int y );

    Ui::BinPlotWidgetClass ui;

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
    QwtPlot binPlot;

    //Specifying a range with the histogram
    RangePicker* rangeTool;
    QwtPlotMarker* rectMarker;
    RangeControlsWidget* rangeControlWidget;
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

    //Plot Controls
    bool displayStep;
    bool displayLog;
    PlotControlsWidget* plotControlsWidget;
};

}

#endif // THRESHOLDINGBINPLOTWIDGET_H
