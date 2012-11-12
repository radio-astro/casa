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

#ifndef FEATHERPLOT_H_
#define FEATHERPLOT_H_

#include <QMenu>
#include <QList>
#include <qwt_plot.h>
#include <qwt_legend.h>

class QwtPlotZoomer;
class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotPicker;
class QGridLayout;
class QMouseEvent;

namespace casa {

class ExternalAxisWidgetBottom;
class ExternalAxisWidgetRight;
class ExternalAxisWidgetLeft;
class ExternalAxisWidget;
class ExternalAxis;

class FeatherPlot : public QwtPlot {

public:
	FeatherPlot(QWidget* parent );
	enum PlotType { ORIGINAL, SLICE_CUT, SCATTER_PLOT, NO_TYPE };
	virtual ~FeatherPlot();
	void clearCurves();
	void addCurve( QVector<double> xValues, QVector<double> yValues,
			QColor curveColor, const QString& curveTitle, QwtPlot::Axis yAxis );
	void initializePlot( const QString& title, PlotType plotType );

	FeatherPlot::PlotType getPlotType() const;
	void setFunctionColor( const QString& curveID, const QColor& color );
	void setDishDiameterLineColor( const QColor& color );
	void setLineThickness( int thickness );
	void setLegendVisibility( bool visible );
	void setDiameterPosition( double position = -1 );
	void insertSingleLegend( QWidget* parent );

	QWidget* getExternalAxisWidget( QwtPlot::Axis position );
	bool moveDiameterMarker( const QPoint& pos );
	double getDishDiameter() const;
	bool isDiameterSelectorMode() const;

private:
	bool isScatterPlot() const;
	int getCurveIndex( const QString& curveTitle ) const;
	void setCurveLineThickness( int curveIndex );
	void updateAxes();
	void setAxisLabels();

	void initializeDiameterMarker();
	void initializeDiameterSelector();

	QwtLegend* legend;

	PlotType plotType;
	bool legendVisible;
	int lineThickness;

	enum LeftMouseMode { RECTANGLE_ZOOM, DIAMETER_SELECTION };
	LeftMouseMode leftMouseMode;
	const int DOT_FACTOR;
	const int AXIS_COUNT;
	const int MARKER_WIDTH;
	QwtPlotMarker* diameterMarker;
	QwtPlotPicker* diameterSelector;
	//QwtPlotPicker *zoomer;
	//QwtPlotZoomer *zoomerRight;
    QList<QwtPlotCurve*> curves;

    QList<QString> axisLabels;
    QList<ExternalAxisWidget*> axisWidgets;
    QList<ExternalAxis*> axisBlanks;
};

} /* namespace casa */
#endif /* FEATHERPLOT_H_ */
