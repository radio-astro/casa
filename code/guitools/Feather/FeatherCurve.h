/*
 * FeatherCurve.h
 *
 *  Created on: Feb 5, 2013
 *      Author: slovelan
 */

#ifndef FEATHERCURVE_H_
#define FEATHERCURVE_H_

#include <qwt_plot.h>
class QwtPlotCurve;

namespace casa {

class FeatherPlot;

/**
 * Represents a curve on one of the plots.
 */

class FeatherCurve {

public:
	FeatherCurve( FeatherPlot* plot, QwtPlot::Axis xAxis, QwtPlot::Axis yAxis);
	void initScatterPlot( int dotSize );
	void setTitle( const QString& title );
	void setFunctionColor( const QColor& color, bool diagonalLine );

	//Change the data the curve contains.
	void setCurveData( const QVector<double>& xVals, const QVector<double>& yVals );
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
	bool isWeightCurve() const;
	double logarithm( double value ) const;
	void doLogs( double* values, int count ) const;

	void resetDataBounds();
	bool scatterPlot;
	bool scaleLogUV;
	bool scaleLogAmplitude;
	double minX;
	double maxX;
	double minY;
	double maxY;
	QwtPlotCurve* plotCurve;
	QVector<double> xValues;
	QVector<double> yValues;

};

} /* namespace casa */
#endif /* FEATHERCURVE_H_ */
