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
#include <QDebug>
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



void FeatherPlotWidgetSlice::zoom90( double dishPosition,  CurveType cType, DataType dType ){
	if ( curvePreferences[cType].isDisplayed()){
		pair<QVector<double>, QVector<double> > result = limitX( dType, dishPosition );
		bool sumCurve = FeatherCurveType::isSumCurve( cType );
		addPlotCurve( result.first, result.second, dType, sumCurve );
	}
}

void FeatherPlotWidgetSlice::zoom90Other( double dishPosition){
	int prefCount = curvePreferences.size();
	for ( int i = 0; i < prefCount; i++  ){
		FeatherCurveType::CurveType cType = static_cast<FeatherCurveType::CurveType>(i);
		if ( FeatherCurveType::isSliceCurve( cType ) ){
			DataType dType = getDataTypeForCurve( cType );
			zoom90( dishPosition, cType, dType );
		}
	}

	//Sum curve
	QVector<double> sumX;
	QVector<double> sumY;
	bool logAmplitude = plot->isLogAmplitude();
	initializeSumData( sumX, sumY, logAmplitude );

	QVector<double> xLimited;
	QVector<double> yLimited;
	QVector<double> xValues = plotData[FeatherDataType::LOW_WEIGHTED].first;
	for ( int i = 0; i < static_cast<int>(sumX.size()); i++ ){
		if ( xValues[i] < dishPosition){
			xLimited.append( sumX[i] );
			yLimited.append( sumY[i] );
		}
	}
	addPlotCurve( xLimited, yLimited, sliceAxis, FeatherCurveType::SUM_LOW_HIGH, true);
}

void FeatherPlotWidgetSlice::zoomRect( double minX, double maxX, CurveType cType, DataType dType ){
	if ( curvePreferences[cType].isDisplayed()){
		pair<QVector<double>, QVector<double> > result = limitX( dType, minX, maxX );
		bool sumCurve = FeatherCurveType::isSumCurve( cType );
		addPlotCurve( result.first, result.second, dType, sumCurve );
	}
}

void FeatherPlotWidgetSlice::zoomRectangleOther( double minX, double maxX, double /*minY*/, double /*maxY*/  ){
	int prefCount = curvePreferences.size();
	for ( int i = 0; i < prefCount; i++  ){
		FeatherCurveType::CurveType cType = static_cast<FeatherCurveType::CurveType>(i);
		if ( FeatherCurveType::isSliceCurve( cType ) ){
			DataType dType = getDataTypeForCurve( cType );
			zoomRect( minX, maxX, cType, dType );
		}
	}

	//Sum curve
	QVector<double> sumX;
	QVector<double> sumY;
	initializeSumData( sumX, sumY, plot->isLogAmplitude() );
	QVector<double> xLimited;
	QVector<double> yLimited;
	QVector<double> xValues = plotData[FeatherDataType::LOW_WEIGHTED].first;
	for ( int i = 0; i < static_cast<int>(sumX.size()); i++ ){
		if ( minX < xValues[i] && xValues[i] < maxX){
			xLimited.append( sumX[i] );
			yLimited.append( sumY[i] );
		}
	}
	addPlotCurve( xLimited, yLimited, sliceAxis, FeatherCurveType::SUM_LOW_HIGH, true );
}


void FeatherPlotWidgetSlice::addSumData(){
	addSumData( plot->isLogAmplitude());
}

void FeatherPlotWidgetSlice::addSumData( bool logScale ){
	QVector<double> sumX;
	QVector<double> sumY;
	initializeSumData( sumX, sumY, logScale );
	if ( sumX.size() > 0 ){
		addPlotCurve( sumX, sumY, sliceAxis, FeatherCurveType::SUM_LOW_HIGH, true );
		plot->replot();
	}
}

void FeatherPlotWidgetSlice::addDisplayedPlotCurve( FeatherCurveType::CurveType curveType,
		FeatherDataType::DataType dataType ){

	if ( curvePreferences[curveType].isDisplayed() ){
		bool sumCurve = FeatherCurveType::isSumCurve( curveType );
		addPlotCurve( plotData[dataType].first, plotData[dataType].second, dataType, sumCurve );
	}
}

void FeatherPlotWidgetSlice::addZoomNeutralCurves(){
	int prefCount = curvePreferences.size();
	for ( int i = 0; i < prefCount; i++  ){
		FeatherCurveType::CurveType cType = static_cast<FeatherCurveType::CurveType>(i);
		if ( FeatherCurveType::isSliceCurve( cType ) ){
			DataType dType = getDataTypeForCurve( cType );
			addDisplayedPlotCurve( cType, dType );
		}
	}

	if ( curvePreferences[FeatherCurveType::SUM_LOW_HIGH].isDisplayed() ){
		//Sum curve
		addSumData();
	}
}

FeatherPlotWidgetSlice::~FeatherPlotWidgetSlice() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
