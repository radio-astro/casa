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

#include <tr1/memory>

namespace casa {

template <class T> class ImageInterface;
template <class T> class ImageHistograms;
template <class T> class SubImage;
class ImageRegion;
class HeightSource;

/**
 * Generates and Manages the data corresponding to a histogram.
 */

class Histogram {
public:
	Histogram( HeightSource* source );
	int getDataCount() const;

	//Just recompute the histogram if a parameter such as bin count
	//or intensity changes.
	bool compute();

	//Completely reset the histogram if the image, region, or channels change
	bool reset();

	void setRegion(ImageRegion* region);
	void defineLine( int index, QVector<double>& xVals, QVector<double>& yVals,
			bool useLogY ) const;
	void defineStepHorizontal( int index, QVector<double>& xVals, QVector<double>& yVals,
			bool useLogY ) const;
	void defineStepVertical( int index, QVector<double>& xVals, QVector<double>& yVals,
			bool useLogY ) const;
	std::pair<float,float> getMinMaxBinCount() const;
	vector<float> getXValues() const;
	vector<float> getYValues() const;
	std::pair<float,float> getDataRange() const;
	void toAscii( QTextStream& out ) const;
	virtual ~Histogram();

	//common to all histograms
	void setBinCount( int count );
	void setChannelRangeDefault();
	void setIntensityRangeDefault();
	void setChannelRange( int minChannel, int maxChannel );
	void setIntensityRange( float minimumIntensity, float maximumIntensity );
	void setImage( const std::tr1::shared_ptr<const ImageInterface<Float> > image );
	static double computeYValue( double value, bool useLog );

signals:
	void postStatus( const QString& msg );

private:
	Histogram( const Histogram& other );
	Histogram operator=( const Histogram& other );
	ImageHistograms<Float>* filterByChannels( const std::tr1::shared_ptr<const ImageInterface<Float> >  image );
	HeightSource* heightSource;
	vector<Float> xValues;
	vector<Float> yValues;
	ImageHistograms<Float>* histogramMaker;
	ImageRegion* region;

	const int ALL_CHANNELS;
	const int ALL_INTENSITIES;
    std::tr1::shared_ptr<const ImageInterface<Float> >  image;
	int channelMin;
	int channelMax;
	float intensityMin;
	float intensityMax;
	int binCount;

};

} /* namespace casa */
#endif /* HISTOGRAM_H_ */
