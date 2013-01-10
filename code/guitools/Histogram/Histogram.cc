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

#include "Histogram.h"
#include <assert.h>
#include <QDebug>
#include <QtCore/qmath.h>
#include <images/Images/SubImage.h>
#include <images/Images/ImageHistograms.h>
#include <images/Regions/ImageRegion.h>
#include <guitools/Histogram/HeightSource.h>

namespace casa {

Histogram::Histogram( HeightSource* heightSource ) {
	this->heightSource = heightSource;
}

bool Histogram::reset(const ImageInterface<Float>* image, const ImageRegion* region){
	bool success = true;
	if ( image != NULL ){
		ImageHistograms<Float>* histogramMaker = NULL;
		SubImage<Float>* subImage = NULL;
		if ( region == NULL ){
			//Make the histogram based on the image
			histogramMaker = new ImageHistograms<Float>(*image );
		}
		else {
			//Make the histogram based on the region
			try {
				subImage = new SubImage<Float>( *image, *region );
				histogramMaker = new ImageHistograms<Float>(*subImage);
			}
			catch( AipsError& error ){
				success = false;
				if ( heightSource != NULL ){
					QString msg( "Could not make a histogram of the region: ");
					msg.append( error.getMesg().c_str() );
					heightSource->postMessage( msg );
				}
			}
		}
		Array<Float> values;
		Array<Float> counts;
		if ( success ){
			success = histogramMaker->getHistograms( values, counts );
			if ( success ){
				//Store the data
				xValues.resize( values.size());
				yValues.resize( counts.size());
				values.tovector( xValues );
				counts.tovector( yValues );
			}
		}
		delete histogramMaker;
		delete subImage;
	}
	else {
		success = false;
		qDebug() << "Cant reset histogram with a null image.";
	}
	return success;
}

void Histogram::defineLine( int index, QVector<double>& xVals,
		QVector<double>& yVals, bool useLog ) const{
	assert( xVals.size() == 2 );
	assert( yVals.size() == 2 );
	int dataCount = xValues.size();
	assert( index >= 0 && index < dataCount);
	xVals[0] = xValues[index];
	xVals[1] = xValues[index];
	yVals[0] = 0;
	yVals[1] = computeYValue( yValues[index], useLog );
}

void Histogram::defineStepHorizontal( int index, QVector<double>& xVals,
		QVector<double>& yVals, bool useLog ) const{
	assert( xVals.size() == 2 );
	assert( yVals.size() == 2 );
	int pointCount = xValues.size();
	assert( index >= 0 && index < pointCount);
	if ( index > 0 ){
		xVals[0] = (xValues[index] + xValues[index-1])/2;
	}
	else {
		xVals[0] = xValues[0];
	}
	if ( index < pointCount - 1){
		xVals[1] = ( xValues[index] + xValues[index+1] ) / 2;
	}
	else {
		xVals[1] = xValues[index];
	}
	yVals[0] = computeYValue(yValues[index], useLog);
	yVals[1] = yVals[0];
}

double Histogram::computeYValue( double value, bool useLog ){
	double resultValue = value;
	if ( useLog ){
		if ( value != 0 ){
			resultValue = qLn( value ) / qLn( 10 );
		}
	}
	return resultValue;
}

void Histogram::defineStepVertical( int index, QVector<double>& xVals,
		QVector<double>& yVals, bool useLog ) const {
	assert( xVals.size() == 2 );
	assert( yVals.size() == 2 );
	int count = xValues.size();
	assert( index >= 0 && index < count );
	if ( index > 0 ){
		xVals[0] = (xValues[index] + xValues[index-1])/2;
	}
	else {
		xVals[0] = xValues[0];
	}
	xVals[1] = xVals[0];

	if ( index > 0 ){
		yVals[0] = computeYValue(yValues[index-1], useLog );
	}
	else {
		yVals[0] = 0;
	}
	yVals[1] = computeYValue(yValues[index], useLog );
}

int Histogram::getDataCount() const {
	return xValues.size();
}


float Histogram::getTotalCount() const {
	float count = 0;
	for ( int i = 0; i < static_cast<int>(yValues.size()); i++ ){
		count = count + yValues[i];
	}
	return count;
}

int Histogram::getPeakIndex() const {
	int peakIndex = -1;
	double maxCount = std::numeric_limits<double>::min();
	for ( int i = 0; i < static_cast<int>(yValues.size()); i++ ){
		if ( yValues[i] > maxCount ){
			maxCount = yValues[i];
			peakIndex = i;
		}
	}
	return peakIndex;
}

pair<float,float> Histogram::getZoomRange( float peakPercent ) const {
	int peakIndex = getPeakIndex();
	pair<float,float> zoomRange;
	if ( peakIndex >= 0 ){
		float totalCount = getTotalCount();
		float targetCount = totalCount * peakPercent;
		int startIndex = peakIndex;
		int endIndex = peakIndex;
		float runningCount = yValues[peakIndex];
		int valueCount = yValues.size();
		//Keep going until we have reached our target or we have no more values to add in.
		while( runningCount < targetCount && (startIndex>=0 || endIndex<valueCount) ){
			startIndex = startIndex - 1;
			if ( startIndex > 0 ){
				runningCount = runningCount + yValues[startIndex];
				if ( runningCount >= targetCount ){
					break;
				}
			}

			endIndex = endIndex + 1;
			if ( endIndex < valueCount ){
				runningCount = runningCount + yValues[endIndex];
			}

		}
		startIndex = qMax( 0, startIndex );
		endIndex = qMin( valueCount - 1, endIndex );
		if ( startIndex == endIndex ){
			if ( startIndex > 0 ){
				startIndex--;
			}
			else {
				if ( endIndex < valueCount - 1){
					endIndex++;
				}
			}
		}
		zoomRange.first = xValues[startIndex];
		zoomRange.second = xValues[endIndex];
	}
	return zoomRange;
}
vector<float> Histogram::getXValues() const {
	return xValues;
}
vector<float> Histogram::getYValues() const {
	return yValues;
}
pair<float,float> Histogram::getDataRange() const {
	int count = xValues.size();
	pair<float,float> range;
	if ( count >= 1 ){
		double minValue = xValues[0];
		double maxValue = xValues[0];
		for ( int i = 1; i < count; i++ ){
			if ( xValues[i] < minValue ){
				minValue = xValues[i];
			}
			else if ( xValues[i] > maxValue ){
				maxValue = xValues[i];
			}
		}
		range.first = minValue;
		range.second = maxValue;
	}
	return range;
}

void Histogram::toAscii( QTextStream& out ) const {
	const QString LINE_END( "\n");
	out << "Intensity" << "Count";
	out << LINE_END;
	out.flush();
	int count = xValues.size();
	for ( int i = 0; i < count; i++ ){
		out << QString::number(xValues[i]) << QString::number( yValues[i]);
		out << LINE_END;
		out.flush();
	}
}

Histogram::~Histogram() {
}

} /* namespace casa */
