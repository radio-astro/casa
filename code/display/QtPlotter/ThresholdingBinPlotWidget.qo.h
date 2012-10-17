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
#ifndef THRESHOLDINGBINPLOTWIDGET_QO_H
#define THRESHOLDINGBINPLOTWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/ThresholdingBinPlotWidget.ui.h>
#include <casa/aips.h>
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <vector.h>
#include <QDebug>
using namespace std;

class QwtPlotMarker;
class QwtPlotCurve;

namespace casa {

template <class T> class ImageInterface;
template <class T> class ImageHistograms;

/**
 * Interface so to minimize coupling between the class that
 * draws rectangles and the main class.  Allows the rectangle
 * drawer to get the height of the canvas draw area on the fly.
 */
class HeightSource {
public:
	HeightSource(){}
	virtual ~HeightSource(){}
	virtual int getCanvasHeight() = 0;
};

/**
 * Handles drawing the range selection rectangle for
 * the bin plot.
 */
class RangePicker : public QwtPlotPicker {
public:
	RangePicker( int xAxis, int yAxis, int selectionFlags, RubberBand rubberBand,
			DisplayMode trackerMode, QwtPlotCanvas* );
	virtual void drawRubberBand( QPainter*)const;
	void setHeightSource( HeightSource* heightSource );
	virtual ~RangePicker();

private:
	HeightSource* heightSource;
};

/**
 * Display a histogram of intensity vs count.  Manages selection
 * of minimum and maximum intensity.
 */

class ThresholdingBinPlotWidget : public QWidget, HeightSource
{
    Q_OBJECT

public:
    ThresholdingBinPlotWidget(QString yAxisUnits, QWidget *parent = 0);
    void setImage( ImageInterface<Float>* img );
    pair<double,double> getMinMaxValues() const;
    void setMinMaxValues( double minValue, double maxValue, bool updateGraph=true );
    virtual int getCanvasHeight();
    ~ThresholdingBinPlotWidget();

protected:
    void resizeEvent( QResizeEvent* event );

private slots:
	void rectangleSelected(const QwtDoubleRect & rect);
	void clearRange();
	void defineCurve();
	void minMaxChanged();

private:
	void rectangleSizeChanged();
	void resetRectangleMarker();
	void defineCurveLine();
	void defineCurveHistogram();
	void addCurve( QVector<double>& xValues, QVector<double>& yValues );
	void clearCurves();
	double checkLogValue( double value ) const;
	void setValidatorLimits();

    Ui::ThresholdingBinPlotWidgetClass ui;
    ImageHistograms<Float>* histogramMaker;
    QwtPlot binPlot;
    double rectX;
    double rectWidth;
    RangePicker* rangeTool;
    QwtPlotMarker* rectMarker;
    QList<QwtPlotCurve*> curves;
    QColor curveColor;
    QColor selectionColor;
    vector<float> xVector;
    vector<float> yVector;
    QDoubleValidator* minMaxValidator;
};

}

#endif // THRESHOLDINGBINPLOTWIDGET_H
