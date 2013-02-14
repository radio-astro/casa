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

#ifndef IMAGE_SLICE_H_
#define IMAGE_SLICE_H_

#include <casa/Arrays/Vector.h>
#include <QColor>
#include <QString>
#include <QList>
#include <QTextStream>

class QwtPlotCurve;
class QwtPlot;

namespace casa {

class ImageAnalysis;
class SliceWorker;

class ImageSlice {

public:
	ImageSlice( int id );
	void setSampleCount( int count );
	void setAxes( const Vector<Int>& axes );
	void setCoords( const Vector<Int>& coords );
	void setCurveColor( QColor color );
	void setInterpolationMethod( const String& method );
	void setImageAnalysis( ImageAnalysis* analysis );
	void setUseViewerColors( bool useViewerColors );
	void setViewerCurveColor( const QString& colorName );
	void updatePolyLine(  const QList<int> &pixelX, const QList<int> & pixelY );
	void toAscii( QTextStream& );
	void clearCurve();
	void addPlotCurve( QwtPlot* plot);
	virtual ~ImageSlice();
	enum AxisXChoice {DISTANCE,X_POSITION, Y_POSITION};
	void setAxisXChoice( AxisXChoice choice );

private:
	void resetPlotCurve();
	QColor getCurveColor() const;
	AxisXChoice xAxisChoice;
	ImageSlice( const ImageSlice& other );
	ImageSlice operator=( const ImageSlice& other );
	bool useViewerColors;
	QColor viewerColor;
	SliceWorker* sliceWorker;
	QwtPlotCurve* plotCurve;
};

} /* namespace casa */
#endif /* IMAGE_SLICE_H_ */
