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
#include <display/Slicer/ImageSlice.qo.h>

#include <tr1/memory>

class QwtPlotCurve;


namespace casa {

	template <class T> class ImageInterface;
	class ImageAnalysis;

	class SlicePlot : public QwtPlot {

		Q_OBJECT

	public:
		SlicePlot(QWidget *parent = NULL, bool fullVersion = false );

		//Data
		void setImage( ImageInterface<float>* img );
		void updateChannel( int channel );
		void setRegionSelected( int regionId, bool selected );

		//Look and feel
		void resetCurveColors( bool viewerColors, bool polylineColorUnit,
		                       QList<QColor> accumulateCurveColors);
		void setViewerCurveColor( int regionId, const QString& colorName );
		void setUseViewerColors( bool viewerColors );
		void setPlotPreferences( int lineWidth, int markerSize );

		//Setting slice parameters.
		void setSampleCount( int sampleCount );
		void setInterpolationMethod( const String& method );
		void setAccumulateSlices( bool accumulate );

		//Wipe out only the curves that shouldn't be displayed under
		//the current settings.
		void clearCurves();
		//Wipe out all curves.
		void clearCurvesAll();
		bool toAscii( const QString& fileName );

		//Statistics
		void addStatistic( int regionId );
		void removeStatistics( );
		void removeStatistic( int regionId);
		void setStatisticsLayout( QLayout* layout );
		void updatePositionInformation( int id, const QVector<String>& info );
		void markPositionChanged(int regionId,int segmentIndex,float percentage);
		void markVisibilityChanged(int regionId,bool showMarker);
		bool isFullVersion() const;

		virtual ~SlicePlot();

		const static QString DISTANCE_AXIS;
		const static QString POSITION_X_AXIS;
		const static QString POSITION_Y_AXIS;
		const static QString UNIT_X_PIXEL;
		const static QString UNIT_X_ARCSEC;
		const static QString UNIT_X_ARCMIN;
		const static QString UNIT_X_ARCDEG;

	signals:
		void markerPositionChanged(int regionId,int segmentIndex,float percentage);
		void markerVisibilityChanged(int regionId,bool showMarker);

	public slots:
		void updatePolyLine(  int regionId,viewer::region::RegionChanges regionChanges,
		                      const QList<double> & linearX, const QList<double> & linearY,
		                      const QList<int> &pixelX, const QList<int> & pixelY);
		void setXAxis( const QString& newAxis );
		void xAxisUnitsChanged( const QString& units );
		void segmentMarkerVisibilityChanged( bool visible );

	private:
		SlicePlot( const SlicePlot& other );
		SlicePlot operator=(const SlicePlot& other );
		ImageSlice* getSlicerFor( int regionId );
		SliceStatisticsFactory::AxisXUnits getUnitMode() const;
		SliceStatisticsFactory::AxisXChoice getXAxis() const;
		void initPlot();
		void resetCurves();
		void addPlotCurve( int regionId );
		void initAxisFont( int axisId, const QString& axisTitle );
		void sliceFinished( int regionId);
		QString getAxisLabel() const;
		void updateSelectedRegionId( int selectedRegionId );
		void updatePolyLine(  int regionId, const QList<double>& worldX,
		                      const QList<double>& worldY, const QList<int>& pixelX,
		                      const QList<int>& pixelY);
		void deletePolyLine( int regionId );

		int getColorIndex( int regionId ) const;
		int assignCurveColors( int initialColorIndex, int regionId );
		void resetExistingCurveColors();

		QList<QColor> curveColors;
		std::tr1::shared_ptr<ImageInterface<float> > image;
		ImageAnalysis* imageAnalysis;
		QMap<int, ImageSlice*> sliceMap;

		Vector<Int> coords;
		int curveWidth;
		int markerSize;
		bool accumulateSlices;
		bool fullVersion;
		bool viewerColors;
		bool polylineColorUnit;
		bool segmentMarkers;
		int sampleCount;
		int currentRegionId;
		const int AXIS_FONT_SIZE;
		String interpolationMethod;
		QString xAxis;
		QString xAxisUnits;
		Vector<Int> axes;
		QLayout* statLayout;
		SliceStatisticsFactory* factory;
	};

} /* namespace casa */
#endif /* SLICEPLOT_H_ */
