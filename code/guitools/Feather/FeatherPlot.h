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

#include <QList>
#include <qwt_plot.h>
#include <qwt_legend.h>

class QwtPlotCurve;
class QGridLayout;

namespace casa {

class ExternalAxisWidgetBottom;
class ExternalAxisWidgetRight;
class ExternalAxisWidgetLeft;
class ExternalAxisWidget;
class ExternalAxis;
class FeatherCurve;

class FeatherPlot : public QwtPlot {

friend class ExternalAxisWidgetBottom;
friend class ExternalAxisWidgetRight;
friend class ExternalAxisWidgetLeft;

public:
	FeatherPlot(QWidget* parent );
	enum PlotType { SLICE_CUT, SCATTER_PLOT, NO_TYPE };
	virtual ~FeatherPlot();
	void clearCurves();
	void clearLegend();
	void resetLegend();

	void addCurve( QVector<double> xValues, QVector<double> yValues,
			QColor curveColor, const QString& curveTitle, QwtPlot::Axis yAxis, bool sumCurve );
	void addDiagonal( QVector<double> xValues, QColor lineColor, QwtPlot::Axis yAxis );
	void initializePlot( const QString& title, PlotType plotType );

	//Preferences
	void setFunctionColor( const QString& curveID, const QColor& color );
	void setLineThickness( int thickness );
	void setDotSize( int dotSize );
	void setLegendVisibility( bool visible );
	/**
	 * Returns true if the uvScale changed.  The log scale does not
	 * impact the return of this method.
	 */
	bool setLogScale( bool uvScale, bool ampScale );

	//Legend
	void insertSingleLegend( QWidget* parent );
	QWidget* getExternalAxisWidget( QwtPlot::Axis position );

	//Accessors
	FeatherPlot::PlotType getPlotType() const;
	bool isLogUV() const;
	bool isLogAmplitude() const;
	bool isEmpty() const;
	bool isScatterPlot() const;
	bool isSliceCut() const;


	static const QString Y_EQUALS_X;

private:
	void setCurvePenColor( int curveIndex, const QColor& color );
	int getCurveIndex( const QString& curveTitle ) const;
	void setCurveSize( int curveIndex );
	void updateAxes();
	void setAxisLabels();
	void adjustPlotBounds( std::pair<double,double> curveBounds, QwtPlot::Axis yAxis );
	void resetPlotBounds();
	void setLegendSize();
	void initAxes( );
	void setCurveData( FeatherCurve* curve, QwtPlot::Axis yAxis );

	QwtLegend* externalLegend;
	QWidget* legendParent;
	PlotType plotType;

	int lineThickness;
	int dotSize;
	bool scaleLogAmplitude;
	bool scaleLogUV;

	const int AXIS_COUNT;
	const int MINIMUM_LEGEND_LINE_WIDTH;

    QList<FeatherCurve*> curves;

    //External axis drawing support
    QList<QString> axisLabels;
    QList<ExternalAxisWidget*> axisWidgets;
    QList<ExternalAxis*> axisBlanks;

    double minX;
    double maxX;
    double minY;
    double maxY;
    double minYRight;
    double maxYRight;
};

} /* namespace casa */
#endif /* FEATHERPLOT_H_ */
