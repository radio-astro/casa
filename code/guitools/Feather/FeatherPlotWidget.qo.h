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
class QwtPlotMarker;

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
    void clearPlot();
    void clearLegend();

    //Preferences
    void setPlotColors( const QMap<PreferencesColor::FunctionColor,QColor>& colorMap,
    		const QColor& scatterPlotColor, const QColor& dishDiameterLineColor,
    		const QColor& zoomRectColor );
    void setLineThickness( int thickness );
    void setLegendVisibility( bool v );
    void setDotSize( int size );

    //Actions
    void setPermanentScatter( bool permanentScatter );
    void changePlotType( FeatherPlot::PlotType revertType);

    //Zooming
    void changeZoom90( bool zoom );
    void zoomRectangle( double minX, double maxX, double minY, double maxY );
    void zoomRectangleScatter( double minX, double maxX, double minY, double maxY );
    void zoomNeutral();
    void resetZoomRectangleColor();

    QWidget* getExternalAxisWidget( QwtPlot::Axis position );

    void insertLegend( QWidget* parent );
    ~FeatherPlotWidget();

    //Dish diameter marker
	bool moveDiameterMarker( const QPoint& pos );
	double getDishDiameter() const;
	bool isDiameterSelectorMode() const;
	void setDishDiameter( double value = -1);
	void resetDishDiameterLineColor();

	//Left mouse mode
	void setRectangleZoomMode();
	void setDiameterSelectorMode();

signals:
	void dishDiameterChanged( double newValue);
	void rectangleZoomed( double minX, double maxX, double minY, double maxY );

private slots:
	void zoomRectangleSelected( const QwtDoubleRect& rect );
	void diameterSelected( const QwtDoublePoint& pos );

protected:
    void resizeEvent( QResizeEvent* event );


private:
	void resetColors();
	void resetPlot( FeatherPlot::PlotType plotType );
	void initializeZooming();
	void initializeDiameterMarker();
	void initializeDiameterSelector();
	void initializeMarkers();
	void removeMarkers();
	void changeLeftMouseMode();
    void initializeDomainLimitedData( double minValue, double maxValue,
        		QVector<double>& xValues, QVector<double>& yValues,
        		const QVector<double>& originalXValues, const QVector<double>& originalYValues) const;
    pair<double,double> getMaxMin( QVector<double> values ) const;
    void zoomRectangleWeight( double minX, double maxX );
    void addZoomNeutralCurves();


    FeatherPlot* plot;

    QColor singleDishWeightColor;
    QColor singleDishDataColor;
    QColor interferometerWeightColor;
    QColor interferometerDataColor;
    QColor scatterPlotColor;
    QColor dishDiameterLineColor;
    QColor zoomRectColor;

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

    bool permanentScatter;
    QString plotTitle;
    const int MARKER_WIDTH;

    QwtPlot::Axis sliceAxis;
    QwtPlot::Axis weightAxis;
    QwtPlot::Axis scatterAxis;
    QwtPlotPicker* zoomer;
    QwtPlotMarker* diameterMarker;
    QwtPlotPicker* diameterSelector;

    enum LeftMouseMode { RECTANGLE_ZOOM, DIAMETER_SELECTION };
    LeftMouseMode leftMouseMode;
    Ui::FeatherPlotWidgetClass ui;
};

}
#endif // FEATHERPLOTWIDGET_H
