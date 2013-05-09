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

#ifndef IMAGE_SLICE_QO_H_
#define IMAGE_SLICE_QO_H_

#include <display/Slicer/ImageSlice.ui.h>
#include <display/Slicer/SliceStatisticsFactory.h>
#include <display/Slicer/SliceStatistics.h>
#include <display/Slicer/SliceWorker.h>
#include <casa/Arrays/Vector.h>
#include <QtGui/QFrame>
#include <QColor>
#include <QString>
#include <QList>
#include <QTextStream>

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlot;

namespace casa {

	class ImageAnalysis;
	class SliceSegment;
	class ImageSliceColorBar;
	class SliceStatistics;

	/**
	 * Represents a slice cut of an image.  The slice cut may be
	 * a polygonal chain or a single line segment.
	 */

	class ImageSlice : public QFrame {

		Q_OBJECT

	public:
		ImageSlice( int id, QWidget* parent=NULL );
		QColor getCurveColor( int index) const;
		int getColorCount() const;
		void setShowCorners( bool show );
		void setSampleCount( int count );
		void setAxes( const Vector<Int>& axes );
		void setCoords( const Vector<Int>& coords );
		void setCurveColor( QList<QColor> colors );
		bool isSelected() const;
		void setSelected( bool selected );
		void setInterpolationMethod( const String& method );
		void setImageAnalysis( ImageAnalysis* analysis );
		void setUseViewerColors( bool useViewerColors );
		void setPolylineColorUnit( bool polyline );
		void setPlotPreferences( int curveWidth, int markerSize );
		void setViewerCurveColor( const QString& colorName );
		void updatePolyLine(  const QList<int> &pixelX, const QList<int> & pixelY,
		                      const QList<double>& worldX, const QList<double>& worldY );
		void updatePositionInformation(const QVector<String>& info );
		void toAscii( QTextStream& );
		void clearCurve();
		void addPlotCurve( QwtPlot* plot);

		virtual ~ImageSlice();

		//X-Axis
		void setStatistics( SliceStatistics* statistics );

	private slots:
		void openCloseDisplay();

	private:
		void minimizeDisplay();
		void maximizeDisplay();
		void resetPlotCurve();
		void updateSliceStatistics();
		void clearCorners();
		void addCorner( double xValue, double yValue, QwtPlot* plot );
		void addSegment( SliceSegment* segment );
		void removeSegment( SliceSegment* segment );
		void resetSegmentColors();
		void runSliceWorker();

		SliceStatistics* statistics;
		ImageSlice( const ImageSlice& other );
		ImageSlice operator=( const ImageSlice& other );

		int markerSize;
		int curveWidth;
		int regionId;
		bool selected;
		bool useViewerColors;
		bool showCorners;
		bool polylineUnit;
		bool minimized;
		QColor viewerColor;
		QList<QColor> segmentColors;
		SliceWorker* sliceWorker;
		QList<SliceSegment*> segments;
		QList<QwtPlotMarker*> segmentCorners;
		ImageSliceColorBar* colorBar;
		Ui::ImageSliceClass ui;
	};

} /* namespace casa */
#endif /* IMAGE_SLICE_H_ */
