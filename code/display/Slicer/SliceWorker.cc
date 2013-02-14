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

#include "SliceWorker.h"
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <casa/Containers/Record.h>
#include <synthesis/MSVis/UtilJ.h>
#include <QDebug>
namespace casa {

SliceWorker::SliceWorker( int identifier ):sliceResult( NULL ) {
	id = identifier;
	sampleCount = 0;
	method="";
}

void SliceWorker::setImageAnalysis( ImageAnalysis* analysis ){
	imageAnalysis = analysis;
}

void SliceWorker::setVertices( const QList<int>& xValues, const QList<int>& yValues ){
	int xCount = xValues.size();
	int yCount = yValues.size();
	Assert( xCount == yCount );
	verticesX.resize( xCount);
	verticesY.resize( yCount);
	for ( int i = 0; i < xCount; i++ ){
		verticesX[i] = xValues[i];
		verticesY[i] = yValues[i];
	}
}

void SliceWorker::updatePolyline(){

}

void SliceWorker::setAxes( const Vector<Int>& imageAxes ){
	int axesCount = imageAxes.size();
	axes.resize( axesCount );
	for ( int i = 0; i < axesCount; i++ ){
		axes[i] = imageAxes[i];
	}
}

void SliceWorker::setCoords( const Vector<Int>& imageCoordinates ){
	int coordinateCount = imageCoordinates.size();
	coords.resize( coordinateCount );
	for ( int i = 0; i < coordinateCount; i++ ){
		coords[i] = imageCoordinates[i];
	}
}

void SliceWorker::setSampleCount( int count ){
	sampleCount = count;
}
void SliceWorker::setMethod( const String& method ){
	this->method = method;
}

void SliceWorker::run(){
	Assert( imageAnalysis != NULL );
	if ( method.length() > 0 ){
		sliceResult = imageAnalysis-> getslice(verticesX, verticesY,
			axes, coords, sampleCount, method);
	}
	else if ( sampleCount > 0 ){
		sliceResult = imageAnalysis-> getslice(verticesX, verticesY,
					axes, coords, sampleCount );
	}
	else {
		//Use all default arguments
		sliceResult = imageAnalysis-> getslice(verticesX, verticesY,
							axes, coords );
	}
}

QVector<double> SliceWorker::getFromArray( const Array<float>& source ) const {
	int count = source.size();
	QVector<double> result( count );
	std::vector<float> distanceVector = source.tovector();
	for ( int i = 0; i < count; i++ ){
		result[i] = distanceVector[i];
	}
	return result;
}

QVector<double> SliceWorker::getDistances() const {
	QVector<double> distances;
	if ( sliceResult != NULL ){
		Array<float> distanceArray = sliceResult->asArrayFloat("distance");
		distances = getFromArray( distanceArray );
	}
	return distances;
}

QVector<double> SliceWorker::getXPositions() const {
	QVector<double> xPositions;
	if ( sliceResult != NULL ){
		Array<float> xPositionArray = sliceResult->asArrayFloat("xpos");
		xPositions= getFromArray( xPositionArray );
	}
	return xPositions;
}

QVector<double> SliceWorker::getYPositions() const {
	QVector<double> yPositions;
	if ( sliceResult != NULL ){
		Array<float> yPositionArray = sliceResult->asArrayFloat("ypos");
		yPositions= getFromArray( yPositionArray );
	}
	return yPositions;
}

QVector<double> SliceWorker::getPixels() const {
	QVector<double> pixels;
	if ( sliceResult != NULL ){
		Array<float> pixelArray = sliceResult->asArrayFloat("pixel");
		pixels = getFromArray( pixelArray );
	}
	return pixels;
}

void SliceWorker::toAscii( QTextStream& stream ) const {
	QVector<double> distances = getDistances();
	QVector<double> xPositions = getXPositions();
	QVector<double> yPositions = getYPositions();
	QVector<double> pixels = getPixels();
	int count = distances.size();
	if ( count > 0 ){
		const QString END_OF_LINE( "\n");
		stream << "Region: "<< QString::number(id)<< END_OF_LINE;
		stream << "Distance"<<"X Position"<<"Y Position"<<"Pixel"<<END_OF_LINE;
		for ( int i = 0; i < count; i++ ){
			stream << QString::number( distances[i]) << QString::number( xPositions[i])<<
					QString::number(yPositions[i]) << QString::number(pixels[i]) << END_OF_LINE;
		}
		stream << END_OF_LINE;
	}
}

SliceWorker::~SliceWorker() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
