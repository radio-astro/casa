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

#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <casa/aipstype.h>
#include <casa/vector.h>
#include <QTextStream>

namespace casa {

template <class T> class ImageInterface;
class ImageRegion;
class HeightSource;

/**
 * Generates and Manages the data corresponding to a histogram.
 */

class Histogram {
public:
	Histogram( HeightSource* source );
	int getDataCount() const;
	std::pair<float,float> getDataRange() const;
	bool reset(const ImageInterface<Float>* image, const ImageRegion* region);
	void defineLine( int index, QVector<double>& xVals, QVector<double>& yVals, bool useLog ) const;
	void defineStepHorizontal( int index, QVector<double>& xVals, QVector<double>& yVals, bool useLog ) const;
	void defineStepVertical( int index, QVector<double>& xVals, QVector<double>& yVals, bool useLog ) const;
	std::pair<float,float> getZoomRange( float peakPercent ) const;
	vector<float> getXValues() const;
	vector<float> getYValues() const;
	void toAscii( QTextStream& out ) const;
	virtual ~Histogram();
	static double computeYValue( double value, bool useLog );
signals:
	void postStatus( const QString& msg );
private:
	int getPeakIndex() const;
	float getTotalCount() const;
	HeightSource* heightSource;
	vector<Float> xValues;
	vector<Float> yValues;
};

} /* namespace casa */
#endif /* HISTOGRAM_H_ */
