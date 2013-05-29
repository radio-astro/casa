/*
 * FeatherPlotWidgetScatter.cc
 *
 *  Created on: May 22, 2013
 *      Author: slovelan
 */

#include "FeatherPlotWidgetScatter.h"

namespace casa {

FeatherPlotWidgetScatter::FeatherPlotWidgetScatter(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent):
	FeatherPlotWidget( title, plotType,parent){
	// TODO Auto-generated constructor stub

}

//The scatter plot data needs to be on the same scale as the slice cut
//Find the max value of each data set, and choose the smallest of the
//two upper bounds as the limit.
void FeatherPlotWidgetScatter::addZoomNeutralCurves(){

	DataType sdType = FeatherDataType::LOW_CONVOLVED_HIGH_WEIGHTED;
	DataType intType = FeatherDataType::HIGH_CONVOLVED_LOW_WEIGHTED;
	pair<double,double> singleDishMinMax = getMaxMin( plotData[sdType].second );
	pair<double,double> interferometerMinMax = getMaxMin( plotData[intType].second );
	double valueLimit = qMin( singleDishMinMax.second, interferometerMinMax.second );
	QVector<double> scatterXValues;
	QVector<double> scatterYValues;
	QVector<double> singleDishDataYValues = plotData[sdType].second;
	QVector<double> interferometerDataYValues = plotData[intType].second;
	double actualLimit = numeric_limits<double>::min();
	for ( int i = 0; i < singleDishDataYValues.size(); i++ ){
		if ( singleDishDataYValues[i] <= valueLimit && interferometerDataYValues[i] <= valueLimit ){
			scatterXValues.append( singleDishDataYValues[i]);
			scatterYValues.append( interferometerDataYValues[i]);
			if ( singleDishDataYValues[i] > actualLimit ){
				actualLimit = singleDishDataYValues[i];
			}
			if ( interferometerDataYValues[i] > actualLimit ){
				actualLimit = interferometerDataYValues[i];
			}
		}
	}


	addPlotCurve( scatterXValues, scatterYValues, scatterAxis, FeatherCurveType::SCATTER_LOW_HIGH );
	if ( curvePreferences[FeatherCurveType::X_Y].isDisplayed()){
		scatterXValues.append(actualLimit);
		QColor xyColor = curvePreferences[FeatherCurveType::X_Y].getColor();
		plot->addDiagonal( scatterXValues, xyColor, scatterAxis );
	}
	initializeMarkers();
	plot->replot();
}

void FeatherPlotWidgetScatter::zoom90Other( double /*dishPosition*/, const QVector<double>& /*singleDishZoomDataX*/, const QVector<double>& singleDishZoomDataY,
		const QVector<double>& /*interferometerZoomDataX*/, const QVector<double>& interferometerZoomDataY ){
	//The plot needs to have the same values in both directions.
	pair<double,double> singleDishMinMax = getMaxMin( singleDishZoomDataY );
	pair<double,double> interferometerMinMax = getMaxMin( interferometerZoomDataY );
	double valueMax = qMin( singleDishMinMax.second, interferometerMinMax.second);
	double valueMin = qMax( singleDishMinMax.first, interferometerMinMax.first );
	QVector<double> restrictX;
	QVector<double> restrictY;
	for ( int i = 0; i < singleDishZoomDataY.size(); i++ ){
		if ( valueMin <= singleDishZoomDataY[i] && singleDishZoomDataY[i]<= valueMax ){
			if ( valueMin <= interferometerZoomDataY[i] && interferometerZoomDataY[i] <= valueMax ){
				restrictX.append( singleDishZoomDataY[i]);
				restrictY.append( interferometerZoomDataY[i]);
			}
		}
	}
	addPlotCurve( restrictX, restrictY, scatterAxis, FeatherCurveType::SCATTER_LOW_HIGH );
	restrictX.append( valueMax );
	if ( curvePreferences[FeatherCurveType::X_Y].isDisplayed()){
		QColor xyColor = curvePreferences[FeatherCurveType::X_Y].getColor();
		plot->addDiagonal( restrictX, xyColor, scatterAxis );
	}
}

void FeatherPlotWidgetScatter::resetColors(){
	plot->setFunctionColor( "", curvePreferences[FeatherCurveType::SCATTER_LOW_HIGH].getColor() );
	plot->setFunctionColor( FeatherPlot::Y_EQUALS_X, curvePreferences[FeatherCurveType::DISH_DIAMETER].getColor());
	FeatherPlotWidget::resetColors();
}

void FeatherPlotWidgetScatter::zoomRectangleOther( double minX, double maxX, double minY, double maxY ){
	DataType sdType = FeatherDataType::LOW_CONVOLVED_HIGH_WEIGHTED;
	DataType intType = FeatherDataType::HIGH_CONVOLVED_LOW_WEIGHTED;
	int sdYSize = plotData[sdType].second.size();
	int intYSize = plotData[intType].second.size();
	int count = qMin( sdYSize, intYSize);
	int dataCount = 0;
	QVector<double> singleDishDataYValues = plotData[sdType].second;
	QVector<double> interferometerDataYValues = plotData[intType].second;
	for ( int i = 0; i < count; i++ ){
		if ( minX <= singleDishDataYValues[i] && singleDishDataYValues[i] <= maxX ){
			if ( minY <= interferometerDataYValues[i] && interferometerDataYValues[i] <= maxY ){
				dataCount++;
			}
		}
	}

	QVector<double> xValues( dataCount );
	QVector<double> yValues( dataCount );
	int j = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minX <= singleDishDataYValues[i] && singleDishDataYValues[i] <= maxX ){
			if ( minY <= interferometerDataYValues[i] && interferometerDataYValues[i] <= maxY ){
				xValues[j] = singleDishDataYValues[i];
				yValues[j] = interferometerDataYValues[i];
				j++;
			}
		}
	}
	plot->clearCurves();

	addPlotCurve( xValues, yValues, scatterAxis, FeatherCurveType::SCATTER_LOW_HIGH );
	if ( curvePreferences[FeatherCurveType::X_Y].isDisplayed()){
		QColor xyColor = curvePreferences[FeatherCurveType::X_Y].getColor();
		plot->addDiagonal( xValues, xyColor, scatterAxis );
		plot->replot();
	}
}


FeatherPlotWidgetScatter::~FeatherPlotWidgetScatter() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
