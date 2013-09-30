//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#ifndef FEATHERCURVE_H_
#define FEATHERCURVE_H_

#include <qwt_plot.h>
#include <limits>
#include <casa/aipstype.h>
#include <guitools/Feather/ColorProvider.h>
class QwtPlotCurve;
class QwtLegendItem;
namespace casa {

class FeatherPlot;
class LegendCurve;

/**
 * Represents a curve on one of the plots.
 */

class FeatherCurve : public ColorProvider {

public:
	FeatherCurve( FeatherPlot* plot, QwtPlot::Axis xAxis, QwtPlot::Axis yAxis, bool sumCurve);
	void initScatterPlot( int dotSize );
	void setTitle( const QString& title );
	void setFunctionColor( const QColor& color, bool diagonalLine );

	//Change the data the curve contains.
	void setCurveData( const QVector<double>& xVals, const QVector<double>& yVals );
	QVector<double> getXValues() const;
	QVector<double> getYValues() const;
	QColor getRectColor() const;

	//Change the scale of existing data (Log<-->No Log)
	void adjustData( bool uvLog, bool ampLog );

	void setCurveSize( bool scatterPlot, bool diagonalLine,
				int dotSize, int lineThickness );

	std::pair<double,double> getBoundsX() const;
	std::pair<double,double> getBoundsY() const;
	QwtPlot::Axis getVerticalAxis() const;
	QString getTitle() const;
	virtual ~FeatherCurve();

private:
	FeatherCurve( const FeatherCurve& other );
	FeatherCurve operator=( const FeatherCurve& other );

	void setCurvePenColor( const QColor& color );

	bool isSumCurve() const;
	bool isWeightCurve() const;
	double logarithm( double value, Bool& valid ) const;
	QList<int> doLogs( double* values, int count ) const;

	void resetDataBounds();
	bool scatterPlot;
	bool scaleLogUV;
	bool scaleLogAmplitude;
	/**
	 * This variable was added for scatter plots.  Normally if the FeatherCurveType
	 * is SUM_LOW_HIGH, it will be a sum curve.  However, scatter plots can have x-values
	 * which are sums, and y-values which are something else, say a low-weighted curve.
	 * In this case, the FeatherCurveType will be whatever the y-axis.  Thus, we set
	 * a boolean variable to indicated if it is a sum curve.  This is imported because
	 * sums have to be specially calculated in the case of a logarithm scale and cannot
	 * be done directly by taking log( sum).
	 */
	bool sumCurve;
	bool firstTime;
	double minX;
	double maxX;
	double minY;
	double maxY;
	LegendCurve* plotCurve;
	QColor functionColor;
	QVector<double> xValues;
	QVector<double> yValues;

};

} /* namespace casa */
#endif /* FEATHERCURVE_H_ */
