/*
 * FeatherPlotWidgetScatter.cc
 *
 *  Created on: May 22, 2013
 *      Author: slovelan
 */

#include "FeatherPlotWidgetScatter.h"
#include <QtCore/qmath.h>
#include <limits>
#include <QDebug>

namespace casa {

FeatherPlotWidgetScatter::FeatherPlotWidgetScatter(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent):
			FeatherPlotWidget( title, plotType,parent){
}

void FeatherPlotWidgetScatter::addScatterCurve( const QVector<double>& xVals, const QVector<double>& yVals,
		double dataLimit, FeatherCurveType::CurveType curveType, bool sumCurve ){
	int valueCount = xVals.size();
	QVector<double> scatterXValues;
	QVector<double> scatterYValues;

	for ( int i = 0; i < valueCount; i++ ){
		if ( yVals[i] <= dataLimit && xVals[i] <= dataLimit ){
			scatterXValues.append( xVals[i]);
			scatterYValues.append( yVals[i]);
		}
	}
	addPlotCurve( scatterXValues, scatterYValues, scatterAxis, curveType, sumCurve );
}


void FeatherPlotWidgetScatter::addZoomNeutralCurves(){
	int scatterYCount = yScatters.size();
	if ( scatterYCount == 0 ){
		return;
	}
	QVector<double> xVals = populateVector( xScatter );
	pair<double,double> xMinMax = getMaxMin( xVals, xScatter );
	//When we add curves (x, y1), (x, y2), (x,y3), etc.  We need to set the
	//scatter plot upper bound to max( max(x),max(y1), max(y2), max(y3), etc).
	//To further complicate things, for sum curves, the values may already
	//be in a log scale and for other curves they will not be.  We compute
	//the upper bound on a non-logarithm scale, and later take the log of it
	//if the values we are adding are already scaled.
	double valueLimit = xMinMax.second;
	double valueMin = xMinMax.first;
	bool sumCurveX = FeatherCurveType::isSumCurve( xScatter );


	//Compute the corresponding yValues, storing them in a map,
	//and using them to compute an upper limit on the data that will
	//be sent to the plot.
	QMap<int, QVector<double> > yData;
	for ( int i = 0; i < scatterYCount; i++ ){
		QVector<double> yVals = populateVector( yScatters[i]);
		//Need to take logs of y values (if we have not already done it
		//because y is a sum curve.
		pair<double,double> yMinMax = getMaxMin( yVals, yScatters[i] );
		double maxYValue = yMinMax.second;
		if ( maxYValue > valueLimit ){
			valueLimit = maxYValue;
		}
		if ( yMinMax.first < valueMin ){
			valueMin = yMinMax.first;
		}
		yData.insert( i, yVals );
	}

	//Now actually add the data to the plot, restricting it to within
	//the correct bounds.
	for ( int i = 0; i < scatterYCount; i++ ){
		bool sumCurveY = FeatherCurveType::isSumCurve(yScatters[i]);
		//We need to take logs of the xvalues, if we have not already done it,
		//because the plot will not do it for sum curves.
		bool logAmplitude = plot->isLogAmplitude();
		QVector<double> scaledXVals = xVals;
		if ( logAmplitude && sumCurveY && !sumCurveX ){
			scaledXVals = scaleValues( xVals );
		}
		QVector<double> scaledYVals = yData[i];
		if ( logAmplitude && sumCurveX && ! sumCurveY  ){
			scaledYVals = scaleValues( yData[i]);
		}

		bool sumCurve = sumCurveX || sumCurveY;
		double curveValueLimit = valueLimit;
		if ( sumCurve && logAmplitude ){
			if ( valueLimit > 0 ){
				curveValueLimit = qLn( valueLimit ) / qLn(10 );
			}
			/*else {
				qDebug() << "Scatter: Can't take the log of valueLimit="<<valueLimit;
			}*/
		}
		addScatterCurve( scaledXVals, scaledYVals, curveValueLimit, yScatters[i], sumCurve );
	}

	//Append the diagonal line x=y
	if ( curvePreferences[FeatherCurveType::X_Y].isDisplayed()){
		QVector<double> scatterXValues;

		for ( int i = 0; i < xVals.size(); i++ ){
			double xValue = xVals[i];
			//The x values will come already scaled.  We need to unscale them.
			if ( FeatherCurveType::isSumCurve( xScatter) && plot->isLogAmplitude() ){
				xValue = qPow( 10, xValue );
			}
			if ( xValue <= valueLimit ){
				scatterXValues.append( xValue);
			}
		}
		if ( scatterXValues.size() > 0 ){
			//So that y=x looks like a diagonal line.
			scatterXValues.append(valueLimit);
			scatterXValues.append( valueMin );
			QColor xyColor = curvePreferences[FeatherCurveType::X_Y].getColor();
			plot->addDiagonal( scatterXValues, xyColor, scatterAxis );
		}
	}
}

QVector<double> FeatherPlotWidgetScatter::scaleValues( const QVector<double>& values ) const {
	QVector<double> scaledValues;
	for ( int j = 0; j < values.size(); j++ ){
		if (  plot->isLogAmplitude() ){
			if ( values[j] > 0 ){
				scaledValues.append( qLn( values[j]) / qLn( 10 ));
			}
			/*else {
				qDebug() << "Scatter: ScaleValues j="<<j<<" couldn't take log value="<<values[j];
			}*/
		}
		else {
			scaledValues.append( values[j]);
		}
	}
	return scaledValues;
}

QVector<double> FeatherPlotWidgetScatter::unscaleValues( const QVector<double>& values ) const {
	QVector<double> origValues;
	for ( int j = 0; j < values.size(); j++ ){
		if (  plot->isLogAmplitude() ){
			origValues.append( qPow( 10 , values[j]));
		}
		else {
			origValues.append( values[j]);
		}
	}
	return origValues;
}

QVector<double> FeatherPlotWidgetScatter::populateVector(
		FeatherCurveType::CurveType curveType){
	DataType dType = getDataTypeForCurve( curveType );
	QVector<double> values;
	if ( dType != FeatherDataType::END_DATA ){
		values = plotData[dType].second;
	}
	else {
		QVector<double> sumX;
		bool ampScale = plot->isLogAmplitude();
		initializeSumData( sumX, values, ampScale );
	}
	return values;
}

void FeatherPlotWidgetScatter::setScatterCurves( const QString& title,
		CurveType xScatterCurve, const QList<CurveType>& yScatterList ){
	xScatter = xScatterCurve;
	yScatters.clear();
	int yScatterCount = yScatterList.size();
	for ( int i = 0; i < yScatterCount; i++ ){
		yScatters.append( yScatterList[i] );
	}
	plot->addAxisTitle( QwtPlot::xBottom, title );
}


pair<QVector<double>, QVector<double> > FeatherPlotWidgetScatter::restrictData( const QVector<double>& sourceX,
		const QVector<double>& sourceY, double valueMin, double valueMax ){
	QVector<double> restrictX;
	QVector<double> restrictY;
	int sourceCount = sourceY.size();
	for ( int i = 0; i < sourceCount; i++ ){
		if ( valueMin <= sourceX[i] && sourceX[i]<= valueMax ){
			if ( valueMin <= sourceY[i] && sourceY[i] <= valueMax ){
				restrictX.append( sourceX[i]);
				restrictY.append( sourceY[i]);
			}
		}
	}
	pair<QVector<double>, QVector<double> > result( restrictX, restrictY );
	return result;
}

pair<QVector<double>, QVector<double> > FeatherPlotWidgetScatter::restrictData(
		const QVector<double>& sourceX, const QVector<double>& sourceY,
		double minX, double maxX, double minY, double maxY){

	QVector<double> restrictX;
	QVector<double> restrictY;
	int sourceCount = sourceY.size();
	for ( int i = 0; i < sourceCount; i++ ){
		if ( minX <= sourceX[i] && sourceX[i]<= maxX ){

			if ( minY <= sourceY[i] && sourceY[i] <= maxY ){
				restrictX.append( sourceX[i]);
				restrictY.append( sourceY[i]);
			}
		}
	}
	pair<QVector<double>, QVector<double> > result( restrictX, restrictY );
	return result;
}

void FeatherPlotWidgetScatter::zoom90Other( double dishPosition ){
	int scatterYCount = yScatters.size();
	if ( scatterYCount == 0 ){
		return;
	}

	//Get the x values for the data we are plotting on the x-axis.
	bool sumCurveX = FeatherCurveType::isSumCurve( xScatter );
	FeatherDataType::DataType dataType = getDataTypeForCurve( xScatter );
	if (sumCurveX ){
		dataType = FeatherDataType::LOW_WEIGHTED;
	}
	pair<QVector<double>, QVector<double> > xyScatterX = plotData[dataType];
	QVector<double> xVals = xyScatterX.first;
	if ( sumCurveX ){
		if ( plot->isLogUV() ){
			for ( int j = 0; j < xVals.size(); j++ ){
				xVals[j] = qPow( 10, xVals[j] );
			}
		}
	}

	//Get the y- amplitude values we will use for the scatter plot.
	QVector<double> ampVals = populateVector( xScatter );
	if ( sumCurveX ){
		ampVals = unscaleValues( ampVals );
	}

	//Use only the x-values that don't exceed the dish diameter.
	QVector<double> unscaledXVals;
	int zoomEndIndex = 0;
	for ( zoomEndIndex = 0; zoomEndIndex < xVals.size(); zoomEndIndex++ ){
		if ( xVals[zoomEndIndex] >= dishPosition ){
			break;
		}
		unscaledXVals.append( ampVals[zoomEndIndex]);
	}
	QVector<double> scaledXVals = scaleValues( unscaledXVals );

	pair<double,double> xMinMax = getMaxMin( unscaledXVals, xScatter );
	double xMin = xMinMax.first;
	double xMax = xMinMax.second;

	double yMin = std::numeric_limits<double>::max();
	double yMax = -1 * yMin;

	//Compute the corresponding yValues, storing them in a map,
	//and using them to compute an upper limit on the data that will
	//be sent to the plot.
	QMap<int, QVector<double> > yData;
	for ( int i = 0; i < scatterYCount; i++ ){
		bool sumCurveY = FeatherCurveType::isSumCurve(yScatters[i]);
		QVector<double> yVals = populateVector( yScatters[i]);
		QVector<double> scaledYVals;
		for ( int j = 0; j < zoomEndIndex; j++ ){
			scaledYVals.append( yVals[j]);
		}
		if ( sumCurveY ){
			scaledYVals = unscaleValues( scaledYVals );
		}

		bool sumCurve = sumCurveX || sumCurveY;
		QVector<Double> actualXVals = unscaledXVals;
		if ( sumCurve ){
			scaledYVals = scaleValues( scaledYVals );
			actualXVals = scaledXVals;
		}


		pair<double,double> yMinMax = getMaxMin( scaledYVals, yScatters[i] );
		if ( yMinMax.first < yMin ){
			yMin = yMinMax.first;
		}
		if ( yMinMax.second > yMax ){
			yMax = yMinMax.second;
		}

		addPlotCurve( actualXVals, scaledYVals, scatterAxis, yScatters[i], sumCurve );
	}

	//Append the diagonal line x=y
	if ( curvePreferences[FeatherCurveType::X_Y].isDisplayed() && isOverlap(xMin,xMax,yMin,yMax) ){

		QVector<double> scatterXValues;

		scatterXValues.append(xMin);
		scatterXValues.append(xMax );
		QColor xyColor = curvePreferences[FeatherCurveType::X_Y].getColor();
		plot->addDiagonal( scatterXValues, xyColor, scatterAxis );
	}
}




void FeatherPlotWidgetScatter::resetColors(){
	//plot->setFunctionColor( "", curvePreferences[FeatherCurveType::SCATTER_LOW_HIGH].getColor() );
	plot->setFunctionColor( FeatherPlot::Y_EQUALS_X, curvePreferences[FeatherCurveType::X_Y].getColor());
	FeatherPlotWidget::resetColors();
}

bool FeatherPlotWidgetScatter::isOverlap( double minX, double maxX, double minY, double maxY ) const {
	bool overlap = false;
	if ( minX <= minY && minY <= maxX){
		overlap = true;
	}
	else if ( minX <= maxY && maxY <= maxX ){
		overlap = true;
	}
	return overlap;
}

void FeatherPlotWidgetScatter::zoomRectangleOther( double minX, double maxX, double minY, double maxY ){
	int scatterYCount = yScatters.size();
	if ( scatterYCount == 0 ){
		return;
	}

	QVector<double> xVals = populateVector( xScatter );
	bool sumCurveX = FeatherCurveType::isSumCurve( xScatter );
	if ( sumCurveX ){
		xVals = unscaleValues( xVals );
	}

	//Compute the corresponding yValues, storing them in a map,
	//and using them to compute an upper limit on the data that will
	//be sent to the plot.
	for ( int i = 0; i < scatterYCount; i++ ){
		bool sumCurveY = FeatherCurveType::isSumCurve(yScatters[i]);
		QVector<double> yVals = populateVector( yScatters[i]);
		if ( sumCurveY ){
			yVals = unscaleValues( yVals );
		}
		pair<QVector<double>, QVector<double> > restrictedVals =
				restrictData( xVals, yVals, minX, maxX, minY, maxY );


		//The restrictedVals are not scaled, but they need to be for
		//sum curves;
		QVector<double> scaledXVals = restrictedVals.first;
		QVector<double> scaledYVals = restrictedVals.second;
		bool sumCurve = sumCurveX || sumCurveY;
		if ( sumCurve ){
			scaledXVals = scaleValues( scaledXVals );
			scaledYVals = scaleValues( scaledYVals );
		}

		addPlotCurve( scaledXVals, scaledYVals, scatterAxis, yScatters[i], sumCurve );
	}

	//Append the diagonal line x=y
	if ( curvePreferences[FeatherCurveType::X_Y].isDisplayed() && isOverlap( minX, minX, minY, maxY)){
		QVector<double> scatterXValues;
		for ( int i = 0; i < xVals.size(); i++ ){
			if ( minX<=xVals[i] && xVals[i]<=maxX ){
				if ( minY <= xVals[i] && xVals[i]<= maxY ){
					double xValue = xVals[i];
					//The x values are not scaled.  We need to them.
					if ( FeatherCurveType::isSumCurve( xScatter) && plot->isLogAmplitude() ){
						if ( xValue > 0 ){
							xValue = qLn( xValue ) / qLn( 10 );
						}
						else {
							//qDebug() << "Scatter diagonal log problem line i="<<i<<" xValue="<<xValue;
						}
					}
					scatterXValues.append( xValue);
				}
			}

		}
		//So it looks like a diagonal line
		double minValue = qMin( minX, minY );
		double maxValue = qMax( maxX, maxY );
		scatterXValues.append( minValue );
		scatterXValues.append( maxValue );
		QColor xyColor = curvePreferences[FeatherCurveType::X_Y].getColor();
		plot->addDiagonal( scatterXValues, xyColor, scatterAxis );
	}
}


FeatherPlotWidgetScatter::~FeatherPlotWidgetScatter() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
