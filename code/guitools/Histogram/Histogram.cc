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
#include <imageanalysis/ImageAnalysis/ImageHistograms.h>
#include <images/Regions/ImageRegion.h>
#include <guitools/Histogram/HeightSource.h>

namespace casa {

Histogram::Histogram( HeightSource* heightSource ):
	histogramMaker(NULL), region(NULL),
	ALL_CHANNELS(-1),
	ALL_INTENSITIES( -1),
	image(),
	channelMin( ALL_CHANNELS ),
	channelMax( ALL_CHANNELS ),
	intensityMin( ALL_INTENSITIES ),
	intensityMax( ALL_INTENSITIES),
	binCount( 25 ){
	this->heightSource = heightSource;
}

void Histogram::setChannelRangeDefault(){
	channelMin = ALL_CHANNELS;
	channelMax = ALL_CHANNELS;
}

void Histogram::setChannelRange( int minChannel, int maxChannel ){
	channelMin = minChannel;
	channelMax = maxChannel;
}

void Histogram::setBinCount( int count ){
	binCount = count;
}

void Histogram::setIntensityRangeDefault(){
	intensityMin = ALL_INTENSITIES;
	intensityMax = ALL_INTENSITIES;
}

void Histogram::setIntensityRange( float minimumIntensity, float maximumIntensity ){
	intensityMin = minimumIntensity;
	intensityMax = maximumIntensity;
}

bool Histogram::compute( ){
	bool success = true;
	if ( histogramMaker != NULL ){

		//Set the number of bins.
		histogramMaker->setNBins( binCount );

		//Set the intensity range.
		Vector<Float> includeRange;
		if ( intensityMin != ALL_INTENSITIES && intensityMax != ALL_INTENSITIES ){
			includeRange.resize(2);
			includeRange[0] = intensityMin;
			includeRange[1] = intensityMax;
		}
		histogramMaker->setIncludeRange( includeRange );
		try {

			//Calculate the histogram
			Array<Float> values;
			Array<Float> counts;
			success = histogramMaker->getHistograms( values, counts );
			if ( success ){
				//Store the data
				xValues.resize( values.size());
				yValues.resize( counts.size());
				values.tovector( xValues );
				counts.tovector( yValues );
			}
		}
		catch( AipsError& error ){
			success = false;
			qDebug() << "Exception: "<<error.what();
		}
	}
	else {
		success = false;
	}
	return success;
}

ImageHistograms<Float>* Histogram::filterByChannels( const std::tr1::shared_ptr<const ImageInterface<Float> > image ){
	ImageHistograms<Float>* imageHistogram = NULL;
	if ( channelMin != ALL_CHANNELS && channelMax != ALL_CHANNELS ){

		//Create a slicer from the image
		CoordinateSystem cSys = image->coordinates();
		if ( cSys.hasSpectralAxis() ){
			int spectralIndex = cSys.spectralCoordinateNumber();
			IPosition imShape = image->shape();
			int shapeCount = imShape.nelements();

			IPosition startPos( shapeCount, 0);
			IPosition endPos(imShape - 1);
			IPosition stride( shapeCount, 1);

			startPos(spectralIndex) = channelMin;
			endPos(spectralIndex) = channelMax;
			Slicer channelSlicer( startPos, endPos, stride, Slicer::endIsLast );
			SubImage<Float> subImage(*image, channelSlicer );
			imageHistogram = new ImageHistograms<Float>( subImage );
		}
	}
	else {
		imageHistogram = new ImageHistograms<Float>(*image );
	}
	return imageHistogram;
}

void Histogram::setImage(const std::tr1::shared_ptr<const ImageInterface<Float> > img ){
	image = img;
}

void Histogram::setRegion( ImageRegion* region ){
	this->region = region;
}

bool Histogram::reset(){
	bool success = true;
	if ( image.get() != NULL ){
		if ( histogramMaker != NULL ){
			delete histogramMaker;
			histogramMaker = NULL;
		}
		try {
			if ( region == NULL ){
				//Make the histogram based on the image
				histogramMaker = filterByChannels( image );
			}
			else {
				//Make the histogram based on the region
				std::tr1::shared_ptr<SubImage<Float> > subImage(new SubImage<Float>( *image, *region ));
				if ( subImage.get() != NULL ){
					histogramMaker = filterByChannels( subImage );
				}
				else {
					success = false;
				}
			}
			if ( success ){
				success = compute();
			}
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
	else {
		success = false;
	}
	return success;
}

void Histogram::defineLine( int index, QVector<double>& xVals,
		QVector<double>& yVals, bool useLogY ) const{
	assert( xVals.size() == 2 );
	assert( yVals.size() == 2 );
	int dataCount = xValues.size();
	assert( index >= 0 && index < dataCount);
	xVals[0] = xValues[index];
	xVals[1] = xValues[index];
	yVals[0] = computeYValue( 0, useLogY );
	yVals[1] = computeYValue( yValues[index], useLogY );
}

void Histogram::defineStepHorizontal( int index, QVector<double>& xVals,
		QVector<double>& yVals, bool useLogY ) const{
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
	yVals[0] = computeYValue(yValues[index], useLogY);
	yVals[1] = yVals[0];
}

void Histogram::defineStepVertical( int index, QVector<double>& xVals,
		QVector<double>& yVals, bool useLogY ) const {
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
		yVals[0] = computeYValue(yValues[index-1], useLogY );
	}
	else {
		yVals[0] = computeYValue( 0, useLogY);
	}
	yVals[1] = computeYValue(yValues[index], useLogY );
}


double Histogram::computeYValue( double value, bool useLog ){
	double resultValue = value;
	//Log of 0 becomes infinity, and some of the counts are 0.
	if ( useLog ){
		if (value < 1 ){
			resultValue = 1;
		}
	}
	return resultValue;
}

std::pair<float,float> Histogram::getMinMaxBinCount() const {
	std::pair<float,float> minMaxBinCount;
	int valueCount = yValues.size();
	for ( int i = 0; i < valueCount; i++ ){
		if ( yValues[i]>minMaxBinCount.second){
			minMaxBinCount.second = yValues[i];
		}
		if ( yValues[i] < minMaxBinCount.first){
			minMaxBinCount.first = yValues[i];
		}
	}
	return minMaxBinCount;
}

int Histogram::getDataCount() const {
	return xValues.size();
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
	QString centerStr( "#Bin Center(");
	Unit unit = Histogram::image->units();
	QString unitStr( unit.getName().c_str());
	centerStr.append( unitStr + ")");
	out << centerStr << "Count";
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
	delete histogramMaker;
	//delete subImage;
}

} /* namespace casa */
