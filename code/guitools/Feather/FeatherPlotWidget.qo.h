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
#include <guitools/Feather/FeatherDataType.h>
#include <guitools/Feather/FeatherCurveType.h>
#include <guitools/Feather/CurveDisplay.h>
#include <casa/Arrays/Vector.h>
#include <qwt_plot.h>
#include <qwt_legend.h>
#include <qwt_polygon.h>
#include <qwt_double_rect.h>
using namespace std;

class QwtPlotPicker;
class QwtPlotMarker;

namespace casa {


class FeatherPlotWidget : public QWidget
{
    Q_OBJECT

public:
    typedef FeatherDataType::DataType DataType;
    typedef FeatherCurveType::CurveType CurveType;

    FeatherPlotWidget(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent = 0);

    //Setting the data into the plots
    void setData( const Vector<Float>& xValues, const Vector<Float>& yValues, DataType dType );
    virtual void addSumData();

    void clearPlot();
    void clearData();
    void clearLegend();

    //Preferences
    void setPlotColors( const QMap<CurveType,CurveDisplay>& colorMap);
    virtual void setScatterCurves( CurveType /*xScatter*/, const QList<CurveType>& /*yScatters*/ ){};
    void setLineThickness( int thickness );
    void setLegendVisibility( bool v );
    void setDotSize( int size );
    void setLogScale( bool uvScale, bool logScale );
    void refresh();

    //Zooming
    void changeZoom90( bool zoom );
    void zoomRectangle( double minX, double maxX, double minY, double maxY);

    void zoomNeutral();
    void resetZoomRectangleColor();
    virtual void addZoomNeutralCurves() = 0;

    QWidget* getExternalAxisWidget( QwtPlot::Axis position );

    void insertLegend( QWidget* parent );
    ~FeatherPlotWidget();

    //Dish diameter marker
	bool moveDiameterMarker( const QPoint& pos );
	double getDishDiameter() const;
	bool isDiameterSelectorMode() const;
	void setDishDiameter( double value = -1, bool scale=true);
	void resetDishDiameterLineColor();

	//Left mouse mode
	void setRectangleZoomMode();
	void setDiameterSelectorMode();

signals:
	void dishDiameterChanged( double newValue);
	void rectangleZoomed( double minX, double maxX, double minY, double maxY );

protected:
	FeatherDataType::DataType getDataTypeForCurve( CurveType cType ) const;

    void resizeEvent( QResizeEvent* event );
    void resetData( DataType dataType, const Vector<Float>& xValues, const Vector<Float>& yValues );
    virtual void addSumData( bool logAmplitude);
    pair<double,double> getMaxMin( QVector<double> values, FeatherCurveType::CurveType curveType ) const;
    virtual void zoomRectangleOther( double minX, double maxX, double minY, double maxY )=0;
    virtual void zoom90Other( double dishPosition) = 0;
    void addPlotCurve( const QVector<double>& xValues, const QVector<double>& yValues,
    		DataType dType, bool sumCurve );
    void addPlotCurve( const QVector<double>& xValues,
    		const QVector<double>& yValues, QwtPlot::Axis axis,
    		CurveType curveType, bool sumCurve );
    pair<QVector<double>,QVector<double> > limitX( DataType dType, double xCutOff );
    pair<QVector<double>,QVector<double> > limitX( DataType dType, double minValue, double maxValue );
    void initializeDomainLimitedData( double minValue, double maxValue,
           		QVector<double>& xValues, QVector<double>& yValues,
           		const QVector<double>& originalXValues, const QVector<double>& originalYValues) const;
    void initializeSumData( QVector<double>& sumX, QVector<double>& sumY, bool logScale );
    void initializeMarkers();
    virtual void resetColors();
    FeatherPlot* plot;
    QwtPlot::Axis sliceAxis;
    QwtPlot::Axis weightAxis;
    QwtPlot::Axis scatterAxis;
    QMap<DataType, std::pair<QVector<double>,QVector<double> > > plotData;
    QMap<CurveType,CurveDisplay> curvePreferences;

private slots:
	void zoomRectangleSelected( const QwtDoubleRect& rect );
	void diameterSelected( const QwtDoublePoint& pos );

private:

	void resetPlot( FeatherPlot::PlotType plotType );
	void initializeZooming();
	void initializeDiameterMarker();
	void initializeDiameterSelector();

	void removeMarkers();
	void changeLeftMouseMode();
	QwtPlot::Axis getAxisYForData( DataType dType );
	CurveType getCurveTypeForData( DataType dType );

    QString plotTitle;
    double dishPosition;
    const int MARKER_WIDTH;
    QwtPlotPicker* zoomer;
    QwtPlotMarker* diameterMarker;
    QwtPlotPicker* diameterSelector;


    enum LeftMouseMode { RECTANGLE_ZOOM, DIAMETER_SELECTION };
    LeftMouseMode leftMouseMode;
    Ui::FeatherPlotWidgetClass ui;
};

}
#endif // FEATHERPLOTWIDGET_H
