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
#include <display/Slicer/SliceStatistics.h>
#include <casa/Containers/Record.h>
#include <synthesis/MSVis/UtilJ.h>
#include <QDebug>
#include <QtCore/qmath.h>
namespace casa {

SliceWorker::SliceWorker( int identifier ){
	id = identifier;
	sampleCount = 0;
	method="";
}

int SliceWorker::getSegmentCount() const {
	return verticesX.size() - 1;
}



void SliceWorker::setImageAnalysis( ImageAnalysis* analysis ){
	imageAnalysis = analysis;
}

void SliceWorker::setVertices( const QList<int>& xValues,
		const QList<int>& yValues, const QList<double>& xValuesWorld,
		const QList<double>& yValuesWorld ){

	int xCount = xValues.size();
	int yCount = yValues.size();
	Assert( xCount == yCount );
	verticesX.resize( xCount);
	verticesY.resize( yCount);
	for ( int i = 0; i < xCount; i++ ){
		verticesX[i] = xValues[i];
		verticesY[i] = yValues[i];
	}

	int xCountWorld = xValuesWorld.size();
	int yCountWorld = yValuesWorld.size();
	Assert( xCountWorld == yCountWorld );
	verticesXWorld.resize( xCountWorld );
	verticesYWorld.resize( yCountWorld );
	for ( int i = 0; i < xCount; i++ ){
		verticesXWorld[i] = xValuesWorld[i];
		verticesYWorld[i] = yValuesWorld[i];
	}
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


void SliceWorker::compute(){
	Assert( imageAnalysis != NULL );
	clearResults();

	int resultCount = getSegmentCount();
	for ( int i = 0; i < resultCount; i++ ){
		Vector<double> xSegmentValues( 2 );
		Vector<double> ySegmentValues( 2 );
		xSegmentValues[0] = verticesX[i];
		ySegmentValues[0] = verticesY[i];
		xSegmentValues[1] = verticesX[i+1];
		ySegmentValues[1] = verticesY[i+1];
		computeSlice( xSegmentValues, ySegmentValues );
	}
}

void SliceWorker::computeSlice( const Vector<double>& xValues, const Vector<double>& yValues ){
	Record* sliceResult = NULL;
	if ( method.length() > 0 ){
		sliceResult = imageAnalysis-> getslice(xValues, yValues,
				axes, coords, sampleCount, method);
	}
	else if ( sampleCount > 0 ){
		sliceResult = imageAnalysis-> getslice(xValues, yValues,
						axes, coords, sampleCount );
	}
	else {
		//Use all default arguments
		sliceResult = imageAnalysis-> getslice(xValues, yValues,
								axes, coords );
	}
	sliceResults.append( sliceResult );
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




QVector<double> SliceWorker::getValues( int index, const QVector<double>& pixels ) const {
	SliceStatistics* statistics = SliceStatisticsFactory::getInstance()->getStatistics();
	int nextIndex = index+1;
	double start = statistics->getLength( verticesXWorld[index], verticesYWorld[index],
					verticesX[index], verticesY[index]);
	double end   = statistics->getLength( verticesXWorld[nextIndex], verticesYWorld[nextIndex],
					verticesX[nextIndex], verticesY[nextIndex]);
	QVector<double> values = statistics->interpolate( start, end, pixels );


	delete statistics;
	return values;
}

QVector<double> SliceWorker::getDistances( int index, double lastX ) const {
	QVector<double> distances;
	if ( sliceResults.size() > index && index >= 0 ){
		Array<float> distanceArray = sliceResults[index]->asArrayFloat("distance");
		QVector<double> distancePixels= getFromArray( distanceArray );
		distances = getValues( index, distancePixels );

		//In the case of distance in pixels, we want the distance to increase along
		//the individual line segments rather than starting over with each line segment.
		if ( index > 0 ){
			int distanceCount = distances.size();
			for ( int i = 0; i < distanceCount; i++ ){
				distances[i] = distances[i] + lastX;
			}
		}
	}
	return distances;
}

QVector<double> SliceWorker::getXPositions( int index ) const {
	QVector<double> xPositions;
	if ( sliceResults.size() > index && index >= 0 ){
		Array<float> xPositionArray = sliceResults[index]->asArrayFloat("xpos");
		QVector<double> xPositionPixels= getFromArray( xPositionArray );
		xPositions = getValues( index, xPositionPixels );
	}
	return xPositions;
}

QVector<double> SliceWorker::getYPositions( int index ) const {
	QVector<double> yPositions;
	if ( sliceResults.size() > index && index >= 0 ){
		Array<float> yPositionArray = sliceResults[index]->asArrayFloat("ypos");
		QVector<double> yPositionPixels= getFromArray( yPositionArray );
		yPositions = getValues( index, yPositionPixels );
	}
	return yPositions;
}

QVector<double> SliceWorker::getPixels( int index ) const {
	QVector<double> pixels;
	if ( sliceResults.size() > index && index >= 0 ){
		Array<float> pixelArray = sliceResults[index]->asArrayFloat("pixel");
		pixels = getFromArray( pixelArray );
	}
	return pixels;
}

void SliceWorker::toAscii( QTextStream& stream ) const {
	if ( !sliceResults.isEmpty()){
		const QString END_OF_LINE( "\n");
		stream << "Region: "<< QString::number(id)<< END_OF_LINE;
		stream << "Distance"<<"X Position"<<"Y Position"<<"Pixel"<<END_OF_LINE;
		int resultCount = sliceResults.size();
		double xIncr = 0;
		for ( int i = 0; i < resultCount; i++ ){

			QVector<double> distances = getDistances( i, xIncr );
			int distanceCount = distances.size();
			if ( distanceCount > 0 ){
				xIncr = distances[distances.size() - 1] - distances[0];
			}
			else {
				xIncr = 0;
			}
			QVector<double> xPositions = getXPositions( i );
			QVector<double> yPositions = getYPositions( i );
			QVector<double> pixels = getPixels( i );
			int count = distances.size();
			if ( count > 0 ){
				stream << END_OF_LINE <<"# Segment: "<< (i+1)<<END_OF_LINE;
				for ( int j = 0; j < count; j++ ){
					stream << QString::number( distances[j]) <<
							QString::number( xPositions[j])<<
							QString::number(yPositions[j]) <<
							QString::number(pixels[j]) <<
							END_OF_LINE;
				}
			}
		}
		stream << END_OF_LINE;
	}
}

void SliceWorker::clearResults(){
	while( !sliceResults.isEmpty()){
		Record* result = sliceResults.takeLast();
		delete result;
	}
}

SliceWorker::~SliceWorker() {
	clearResults();
}

} /* namespace casa */
