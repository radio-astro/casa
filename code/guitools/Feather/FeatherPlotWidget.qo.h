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
#ifndef FEATHERPLOTWIDGET_QO_H
#define FEATHERPLOTWIDGET_QO_H

#include <QtGui/QWidget>

#include <guitools/Feather/FeatherPlotWidget.ui.h>
#include <guitools/Feather/FeatherPlot.h>
#include <guitools/Feather/PreferencesColor.qo.h>
#include <casa/Arrays/Vector.h>
#include <qwt_plot.h>
#include <qwt_legend.h>
#include <qwt_polygon.h>
#include <qwt_double_rect.h>
#include <pair.h>
using namespace std;

class QwtPlotPicker;

namespace casa {
class FeatherPlotWidget : public QWidget
{
    Q_OBJECT

public:
    FeatherPlotWidget(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent = 0);

    //Setting the data into the plots
    void setSingleDishWeight( const Vector<Float>& xValues, const Vector<Float>& yValues );
    void setInterferometerWeight( const Vector<Float>& xValues, const Vector<Float>& yValues );
    void setSingleDishData( const Vector<Float>& xValues, const Vector<Float>& yValues );
    void setInterferometerData( const Vector<Float>& xValues, const Vector<Float>& yValues );
    void addScatterData();

    //Preferences
    void setPlotColors( const QMap<PreferencesColor::FunctionColor,QColor>& colorMap,
    		const QColor& scatterPlotColor, const QColor& dishDiameterLineColor );
    void setLineThickness( int thickness );
    void setLegendVisibility( bool v );

    //Actions
    void setPermanentScatter( bool permanentScatter );
    void changePlotType( FeatherPlot::PlotType revertType);
    void changeZoom90( bool zoom );
    void zoomRectangle( double minX, double maxX );
    void zoomNeutral();

    void setDishDiameter( double value );

    QWidget* getExternalAxisWidget( QwtPlot::Axis position );

    void insertLegend( QWidget* parent );
    ~FeatherPlotWidget();
signals:
	void dishDiameterChanged( double newValue);
	void rectangleZoomed( double firstValue, double secondValue );
private slots:
	void zoomRectangleSelected( const QwtDoubleRect& rect );

protected:
    void resizeEvent( QResizeEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );
private:
	void resetColors();
	void resetPlot( FeatherPlot::PlotType plotType );
	void initializeZooming();

    void initializeRangeLimitedData( double minValue, double maxValue,
    		QVector<double>& xValues, QVector<double>& yValues,
    		const QVector<double>& originalXValues, const QVector<double>& originalYValues,
    		Double* xMin, Double* xMax ) const;
    void initializeDomainLimitedData( double minValue, double maxValue,
        		QVector<double>& xValues, QVector<double>& yValues,
        		const QVector<double>& originalXValues, const QVector<double>& originalYValues) const;
    pair<double,double> getMaxMin( QVector<double> values ) const;
    void addZoomNeutralCurves();


    FeatherPlot* plot;

    QColor singleDishWeightColor;
    QColor singleDishDataColor;
    QColor interferometerWeightColor;
    QColor interferometerDataColor;
    QColor scatterPlotColor;
    QColor dishDiameterLineColor;

    QVector<double> singleDishWeightXValues;
    QVector<double> singleDishWeightYValues;
    QVector<double> interferometerWeightXValues;
    QVector<double> interferometerWeightYValues;
    QVector<double> singleDishDataXValues;
    QVector<double> singleDishDataYValues;
    QVector<double> interferometerDataXValues;
    QVector<double> interferometerDataYValues;

    QString singleDishFunction;
    QString interferometerFunction;
    QString singleDishWeightFunction;
    QString interferometerWeightFunction;

    bool mouseMove;
    bool legendVisible;
    //bool originalData;
    bool permanentScatter;
    int lineThickness;
    QString plotTitle;

    QwtPlot::Axis sliceAxis;
    QwtPlot::Axis weightAxis;
    QwtPlotPicker* zoomer;
    Ui::FeatherPlotWidgetClass ui;
};

}
#endif // FEATHERPLOTWIDGET_H
