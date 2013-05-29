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

#include "FeatherPlotWidgetSlice.h"

#include <QtCore/qmath.h>

namespace casa {

FeatherPlotWidgetSlice::FeatherPlotWidgetSlice(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent)
		:FeatherPlotWidget(title, plotType, parent){
}


void FeatherPlotWidgetSlice::resetColors(){
	for ( int i = 0; i < FeatherCurveType::ZOOM; i++ ){
		CurveType cType = static_cast<CurveType>(i);
		QString curveName = curvePreferences[cType].getName();
		QColor curveColor = curvePreferences[cType].getColor();
		plot->setFunctionColor( curveName, curveColor );
	}
	FeatherPlotWidget::resetColors();
}

void FeatherPlotWidgetSlice::zoom90Other( double dishPosition,
		const QVector<double>& singleDishZoomDataX, const QVector<double>& singleDishZoomDataY,
		const QVector<double>& interferometerZoomDataX, const QVector<double>& interferometerZoomDataY){
	//Add in the zoomed weight functions
	QVector<double> singleDishWeightZoomX;
	QVector<double> singleDishWeightZoomY;
	DataType dType = FeatherDataType::WEIGHT_SD;
	QVector<double> singleDishWeightXValues = plotData[dType].first;
	QVector<double> singleDishWeightYValues = plotData[dType].second;
	for ( int i = 0; i < singleDishWeightXValues.size(); i++ ){
		if ( singleDishWeightXValues[i] < dishPosition ){
			singleDishWeightZoomX.append( singleDishWeightXValues[i] );
			singleDishWeightZoomY.append( singleDishWeightYValues[i] );
		}
	}
	QVector<double> interferometerWeightZoomX;
	QVector<double> interferometerWeightZoomY;
	dType = FeatherDataType::WEIGHT_INT;
	QVector<double> interferometerWeightXValues = plotData[dType].first;
	QVector<double> interferometerWeightYValues = plotData[dType].second;
	for ( int i = 0; i < interferometerWeightXValues.size(); i++ ){
		if ( interferometerWeightXValues[i] < dishPosition ){
			interferometerWeightZoomX.append( interferometerWeightXValues[i] );
			interferometerWeightZoomY.append( interferometerWeightYValues[i] );
		}
	}
	dType = FeatherDataType::WEIGHT_SD;
	addPlotCurve( singleDishWeightZoomX, singleDishWeightZoomY, dType );
	dType = FeatherDataType::WEIGHT_INT;
	addPlotCurve( interferometerWeightZoomX, interferometerWeightZoomY, dType );

	dType = FeatherDataType::LOW_CONVOLVED_HIGH_WEIGHTED;
	addPlotCurve( singleDishZoomDataX, singleDishZoomDataY, dType );
	dType = FeatherDataType::HIGH_CONVOLVED_LOW_WEIGHTED;
	addPlotCurve( interferometerZoomDataX, interferometerZoomDataY, dType  );

	//Sum curve
	QVector<double> sumX;
	QVector<double> sumY;
	initializeSumData( singleDishZoomDataX, singleDishZoomDataY,
							interferometerZoomDataX, interferometerZoomDataY, sumX, sumY, plot->isLogAmplitude() );
	addPlotCurve( sumX, sumY, sliceAxis, FeatherCurveType::SUM_LOW_HIGH);
}



void FeatherPlotWidgetSlice::zoomRectangleOther( double minX, double maxX, double /*minY*/, double /*maxY*/  ){

	//Weight curves
	QVector<double> singleDishWeightX;
	QVector<double> singleDishWeightY;
	DataType dType = FeatherDataType::WEIGHT_SD;
	initializeDomainLimitedData( minX, maxX, singleDishWeightX, singleDishWeightY,
			plotData[dType].first, plotData[dType].second );
	addPlotCurve( singleDishWeightX, singleDishWeightY, dType );

	QVector<double> interferometerWeightX;
	QVector<double> interferometerWeightY;
	dType = FeatherDataType::WEIGHT_INT;
	initializeDomainLimitedData( minX, maxX,  interferometerWeightX, interferometerWeightY,
			plotData[dType].first, plotData[dType].second);
	addPlotCurve( interferometerWeightX, interferometerWeightY, dType );
}


void FeatherPlotWidgetSlice::addSumData(){
	addSumData( plot->isLogAmplitude());
}

void FeatherPlotWidgetSlice::addSumData( bool logScale ){
	QVector<double> sumX;
	QVector<double> sumY;
	DataType sdType = FeatherDataType::LOW_CONVOLVED_HIGH_WEIGHTED;
	DataType intType = FeatherDataType::HIGH_CONVOLVED_LOW_WEIGHTED;
	initializeSumData( plotData[sdType].first, plotData[sdType].second,
			plotData[intType].first, plotData[intType].second, sumX, sumY, logScale );
	if ( sumX.size() > 0 ){
		addPlotCurve( sumX, sumY, sliceAxis, FeatherCurveType::SUM_LOW_HIGH );
		plot->replot();
	}
}

void FeatherPlotWidgetSlice::addZoomNeutralCurves(){
	DataType sdWeightType = FeatherDataType::WEIGHT_SD;
	DataType intWeightType = FeatherDataType::WEIGHT_INT;
	addPlotCurve( plotData[sdWeightType].first, plotData[sdWeightType].second, sdWeightType );
	addPlotCurve( plotData[intWeightType].first, plotData[intWeightType].second, intWeightType );

	DataType sdType = FeatherDataType::LOW_CONVOLVED_HIGH_WEIGHTED;
	DataType intType = FeatherDataType::HIGH_CONVOLVED_LOW_WEIGHTED;
	addPlotCurve( plotData[sdType].first, plotData[sdType].second, sdType );
	addPlotCurve( plotData[intType].first, plotData[intType].second, intType );

	//Sum curve
	addSumData();

	initializeMarkers();
	plot->replot();
}

FeatherPlotWidgetSlice::~FeatherPlotWidgetSlice() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
