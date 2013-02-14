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

#ifndef SLICEPLOT_H_
#define SLICEPLOT_H_

#include <qwt_plot.h>
#include <QMap>
#include <casa/Arrays/Vector.h>
#include <display/region/RegionEnums.h>

class QwtPlotCurve;

namespace casa {

template <class T> class ImageInterface;
class ImageAnalysis;
class ImageSlice;

class SlicePlot : public QwtPlot {

Q_OBJECT

public:
	SlicePlot(QWidget *parent = NULL, bool fullVersion = false );

	//Data
	void setImage( ImageInterface<float>* img );
	void updateChannel( int channel );

	//Look and feel
	void initAxisFont( int axisId, const QString& axisTitle );
	void resetCurveColors( bool viewerColors, QColor curveColor,
			QList<QColor> accumulateCurveColors);
	void setViewerCurveColor( int regionId, const QString& colorName );
	void setUseViewerColors( bool viewerColors );

	//Setting slice parameters.
	void setSampleCount( int sampleCount );
	void setInterpolationMethod( const String& method );

	void clearCurves();
	bool toAscii( const QString& fileName );
	virtual ~SlicePlot();

	const static QString DISTANCE_AXIS;
	const static QString POSITION_X_AXIS;
	const static QString POSITION_Y_AXIS;

public slots:
	void updatePolyLine(  int regionId, viewer::region::RegionChanges regionChanges,
				const QList<double> & worldX, const QList<double> & worldY,
				const QList<int> &pixelX, const QList<int> & pixelY );
	void setAccumulateSlices( bool accumulate );
	void setXAxis( const QString& newAxis );

private:
	SlicePlot( const SlicePlot& other );
	SlicePlot operator=(const SlicePlot& other );
	ImageSlice* getSlicerFor( int regionId );
	void initPlot();
	void sliceFinished( int regionId, bool selected = true );
	void updateSelectedRegionId( int selectedRegionId );
	void updatePolyLine(  int regionId, const QList<int>& pixelX,
			const QList<int>& pixelY, bool selected = true );
	void deletePolyLine( int regionId );

	void setCurveColor( int regionId );
	int getColorIndex( int regionId ) const;
	void resetExistingCurveColors();

	QColor sliceCurveColor;
	QList<QColor> accumulatedSliceCurveColors;
	ImageInterface<float>* image;
	ImageAnalysis* imageAnalysis;
	QMap<int, ImageSlice*> sliceMap;


	Vector<Int> coords;
	bool accumulateSlices;
	bool fullVersion;
	bool viewerColors;
	int sampleCount;
	int currentRegionId;
	const int AXIS_FONT_SIZE;
	String interpolationMethod;
	QString xAxis;
	Vector<Int> axes;


};

} /* namespace casa */
#endif /* SLICEPLOT_H_ */
