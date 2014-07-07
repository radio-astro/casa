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
#include <msvis/MSVis/UtilJ.h>
#include <QDebug>
#include <QtCore/qmath.h>
namespace casa {

	SliceWorker::SliceWorker( int identifier ) {
		id = identifier;
		sampleCount = 0;
		method="";
	}

	int SliceWorker::getSegmentCount() const {
		return verticesX.size() - 1;
	}



	void SliceWorker::setImageAnalysis( ImageAnalysis* analysis ) {
		imageAnalysis = analysis;
	}

	void SliceWorker::setVertices( const QList<int>& xValues,
	                               const QList<int>& yValues, const QList<double>& xValuesWorld,
	                               const QList<double>& yValuesWorld ) {

		int xCount = xValues.size();
		int yCount = yValues.size();
		Assert( xCount == yCount );
		verticesX.resize( xCount);
		verticesY.resize( yCount);
		for ( int i = 0; i < xCount; i++ ) {
			verticesX[i] = xValues[i];
			verticesY[i] = yValues[i];
		}

		int xCountWorld = xValuesWorld.size();
		int yCountWorld = yValuesWorld.size();
		Assert( xCountWorld == yCountWorld );
		verticesXWorld.resize( xCountWorld );
		verticesYWorld.resize( yCountWorld );
		for ( int i = 0; i < xCount; i++ ) {
			verticesXWorld[i] = xValuesWorld[i];
			verticesYWorld[i] = yValuesWorld[i];
		}
	}


	void SliceWorker::setAxes( const Vector<Int>& imageAxes ) {
		int axesCount = imageAxes.size();
		axes.resize( axesCount );
		for ( int i = 0; i < axesCount; i++ ) {
			axes[i] = imageAxes[i];
		}
	}

	void SliceWorker::setCoords( const Vector<Int>& imageCoordinates ) {
		int coordinateCount = imageCoordinates.size();
		coords.resize( coordinateCount );
		for ( int i = 0; i < coordinateCount; i++ ) {
			coords[i] = imageCoordinates[i];
		}
	}

	void SliceWorker::setSampleCount( int count ) {
		sampleCount = count;
	}


	void SliceWorker::setMethod( const String& method ) {
		this->method = method;
	}



	void SliceWorker::compute() {
		Assert( imageAnalysis != NULL );
		try {
			clearResults();

			int resultCount = getSegmentCount();
			for ( int i = 0; i < resultCount; i++ ) {
				Vector<double> xSegmentValues( 2 );
				Vector<double> ySegmentValues( 2 );
				xSegmentValues[0] = verticesX[i];
				ySegmentValues[0] = verticesY[i];
				xSegmentValues[1] = verticesX[i+1];
				ySegmentValues[1] = verticesY[i+1];
				computeSlice( xSegmentValues, ySegmentValues );
			}
		}
		catch( AipsError& error ){
			String errorMesg = error.getMesg();
			qDebug() << "Could not compute slice: "<<errorMesg.c_str();
		}
	}

	void SliceWorker::computeSlice( const Vector<double>& xValues, const Vector<double>& yValues ) {
		Record* sliceResult = NULL;
		if ( method.length() > 0 ) {
			sliceResult = imageAnalysis-> getslice(xValues, yValues,
			                                       axes, coords, sampleCount, method);
		} else if ( sampleCount > 0 ) {
			sliceResult = imageAnalysis-> getslice(xValues, yValues,
			                                       axes, coords, sampleCount );
		} else {
			//Use all default arguments
			sliceResult = imageAnalysis-> getslice(xValues, yValues,
			                                       axes, coords );
		}
		sliceResults.append( sliceResult );
	}





	QVector<double> SliceWorker::getValues( int index, const QVector<double>& pixels,
	                                        SliceStatistics* statistics ) const {
		int nextIndex = index+1;
		double end   = statistics->getEnd( verticesXWorld[index], verticesYWorld[index],
		                                   verticesXWorld[nextIndex], verticesYWorld[nextIndex]);
		double start = statistics->getStart( verticesXWorld[index], verticesYWorld[index],
		                                     verticesXWorld[nextIndex], verticesYWorld[nextIndex]);
		QVector<double> values = statistics->interpolate( start, end, pixels );
		return values;
	}
	QVector<double> SliceWorker::getData( int index, SliceStatistics* statistics ) const {
		QVector<double> values;
		if ( sliceResults.size() > index && index >= 0 ) {
			QVector<double> valuesPixels = statistics->fromResults( sliceResults[index]);
			values = getValues( index, valuesPixels, statistics );

		}
		return values;
	}



	QVector<double> SliceWorker::getPixels( int index ) const {
		QVector<double> pixels;
		if ( sliceResults.size() > index && index >= 0 ) {
			Array<float> pixelArray = sliceResults[index]->asArrayFloat("pixel");
			pixels = SliceStatistics::getFromArray( pixelArray );
		}
		return pixels;
	}

	void SliceWorker::toAscii( QTextStream& stream, SliceStatistics* statistics ) const {
		if ( !sliceResults.isEmpty()) {
			const QString END_OF_LINE( "\n");
			stream << "Region: "<< QString::number(id)<< END_OF_LINE;
			QString lengthLabel = statistics->getLengthLabel();
			stream << lengthLabel <<"Pixel"<<END_OF_LINE;
			int resultCount = sliceResults.size();
			double xIncr = 0;
			for ( int i = 0; i < resultCount; i++ ) {
				QVector<double> values = getData( i, statistics );

				//So that everything is zero based.
				if ( i == 0 ) {
					statistics->storeIncrement( &xIncr, values, -1  );
				}
				statistics->adjustStart( values, xIncr );
				statistics->storeIncrement( &xIncr, values, i  );

				QVector<double> pixels = getPixels( i );
				int count = values.size();
				if ( count > 0 ) {
					stream << END_OF_LINE <<"# Segment: "<< (i+1)<<END_OF_LINE;
					for ( int j = 0; j < count; j++ ) {
						stream << QString::number( values[j]) <<
						       QString::number(pixels[j]) <<
						       END_OF_LINE;
					}
				}
			}
			stream << END_OF_LINE;
		}
	}

	void SliceWorker::clearResults() {
		while( !sliceResults.isEmpty()) {
			Record* result = sliceResults.takeLast();
			delete result;
		}
	}

	SliceWorker::~SliceWorker() {
		clearResults();
	}

} /* namespace casa */
