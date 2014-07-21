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

#include "ImageSlice.qo.h"
#include <display/Slicer/SliceWorker.h>
#include <display/Slicer/SliceSegment.qo.h>
#include <display/Slicer/ImageSliceColorBar.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <msvis/MSVis/UtilJ.h>
#include <QPen>
#include <QDebug>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

namespace casa {

	ImageSlice::ImageSlice( int id, QWidget* parent ): QFrame(parent) {

		ui.setupUi(this);
		regionId = id;
		sliceWorker = new SliceWorker( id );
		useViewerColors = true;
		showCorners = false;
		polylineUnit = true;
		selected = false;
		segmentColors.append(Qt::magenta);

		curveWidth = 1;
		markerSize = 1;

		QHBoxLayout* layout = new QHBoxLayout();
		colorBar = new ImageSliceColorBar( ui.colorBarHolder );
		layout->addWidget( colorBar );
		layout->setContentsMargins(0,0,0,0);
		ui.colorBarHolder->setLayout( layout );

		maximizeDisplay();
		connect( ui.openCloseButton, SIGNAL(clicked()), this, SLOT(openCloseDisplay()));
	}

//------------------------------------------------------------------------
//                         Setters
//-------------------------------------------------------------------------
	void ImageSlice::setPlotPreferences( int lineWidth, int cornerSize ) {
		if ( cornerSize != markerSize ) {
			markerSize = cornerSize;
			int markerCount = segmentCorners.size();
			for ( int i = 0; i < markerCount; i++ ) {
				QwtSymbol symbol = segmentCorners[i]->symbol();
				symbol.setSize( markerSize );
				segmentCorners[i]->setSymbol(symbol);
			}
		}
		if ( lineWidth != curveWidth ) {
			curveWidth = lineWidth;
			int segmentCount = segments.size();
			for ( int i = 0; i < segmentCount; i++ ) {
				segments[i]->setCurveWidth( curveWidth );
			}
		}
	}

	bool ImageSlice::isSelected() const {
		return selected;
	}

	void ImageSlice::setSelected( bool selected ) {
		this->selected = selected;
	}

	void ImageSlice::setImageAnalysis( ImageAnalysis* analysis ) {
		sliceWorker->setImageAnalysis( analysis );
	}

	void ImageSlice::setInterpolationMethod( const String& method ) {
		sliceWorker->setMethod( method );
		if ( segments.size() > 0 ) {
			//We have to rerun the slicer using the new method to feed
			//new data into the curve.
			runSliceWorker();
			resetPlotCurve();
		}
	}

	void ImageSlice::runSliceWorker() {
		sliceWorker->compute();

		clearCorners();

		int segmentCount = sliceWorker->getSegmentCount();

		//Don't think this will happen, but we need to delete segments until
		//we have the right number.
		int currentSegmentCount = segments.size();
		while ( segmentCount < currentSegmentCount ) {
			SliceSegment* segment = segments.takeLast();
			removeSegment( segment );
			delete segment;
			currentSegmentCount = segments.size();
		}

		//Add segments until we have the right number
		while ( currentSegmentCount < segmentCount ) {
			SliceSegment* sliceSegment = new SliceSegment(regionId, currentSegmentCount, this );
			sliceSegment->setCurveWidth( curveWidth );
			addSegment( sliceSegment );
			segments.append( sliceSegment );
			currentSegmentCount = segments.size();
		}

		resetSegmentColors();
	}

	void ImageSlice::setSampleCount( int count ) {
		sliceWorker->setSampleCount( count );
		if ( segments.size() > 0 ) {
			//We have to rerun the slicer with the new sample count
			//in order to get new data into the curve.
			runSliceWorker();
			resetPlotCurve();
		}
	}

	void ImageSlice::setAxes( const Vector<Int>& axes ) {
		sliceWorker->setAxes( axes );
	}

	void ImageSlice::setCoords( const Vector<Int>& coords ) {
		sliceWorker->setCoords( coords );
	}

	void ImageSlice::setStatistics( SliceStatistics* statistics ) {
		this->statistics = statistics;
		if ( segments.size() > 0 ) {
			resetPlotCurve();
			updateSliceStatistics();
		}
	}

	void ImageSlice::updateSliceStatistics() {
		QList<SliceSegment*>::iterator statIter = segments.begin();
		while ( statIter != segments.end()) {
			(*statIter)->updateStatistics( statistics );
			statIter++;
		}
	}



//------------------------------------------------------------------
//                  Color Management
//------------------------------------------------------------------
	void ImageSlice::setUseViewerColors( bool viewerColors ) {
		if ( viewerColors != useViewerColors ) {
			useViewerColors = viewerColors;
			if ( useViewerColors ) {
				segmentColors.clear();
				segmentColors.append( viewerColor );
			}
		}
	}

	void ImageSlice::setCurveColor( QList<QColor> colors ) {
		if ( !useViewerColors ) {
			segmentColors.clear();
			int colorCount = colors.size();
			for( int i = 0; i < colorCount; i++ ) {
				segmentColors.append( colors[i]);
			}
		}
		resetSegmentColors();
	}

	void ImageSlice::resetSegmentColors() {
		int colorCount = segmentColors.size();
		if ( colorCount > 0 ) {
			for ( int i = 0; i < segments.size(); i++ ) {
				int colorIndex = i % colorCount;
				segments[i]->setColor( segmentColors[colorIndex] );
			}
		}
		colorBar->setColors( segmentColors );
		colorBar->update();
	}


	void ImageSlice::setViewerCurveColor( const QString& colorName ) {
		viewerColor = Qt::black;
		if ( colorName == "green") {
			viewerColor = Qt::green;
		} else if ( colorName == "gray") {
			viewerColor = Qt::gray;
		} else {
			viewerColor.setNamedColor( colorName );
		}

		if ( useViewerColors ) {
			segmentColors.clear();
			segmentColors.append( viewerColor );
			resetSegmentColors();
		}
	}

	void ImageSlice::setPolylineColorUnit( bool polyline ) {
		polylineUnit = polyline;
	}

	int ImageSlice::getColorCount() const {
		int colorCount = 1;
		if ( !polylineUnit ) {
			colorCount = sliceWorker->getSegmentCount();
		}
		return colorCount;
	}


	void ImageSlice::toAscii( QTextStream& out ) {
		if ( segments.size() > 0 && sliceWorker != NULL ) {
			sliceWorker->toAscii( out, statistics );
		}
	}

	void ImageSlice::resetPlotCurve() {
		if ( segments.size() > 0 ) {
			//Now reset the curve with the new data.
			QwtPlot* curvePlot = segments[0]->getPlot();
			if ( curvePlot != NULL ){
				addPlotCurve( curvePlot );
				resetSegmentColors();
			}
		}
	}



//------------------------------------------------------------------
//                 Data & Curve Management
//------------------------------------------------------------------

	void ImageSlice::addPlotCurve( QwtPlot* plot) {
		clearCurve();
		int segmentCount = segments.size();
		int cornerCount = sliceWorker->getSegmentCount();
		int cornerCountPerSegment = 1;
		if ( segmentCount > 0 ) {
			cornerCountPerSegment = cornerCount / segmentCount;
		}

		double xIncr = 0;
		for ( int i = 0; i < segmentCount; i++ ) {

			SliceSegment* sliceSegment = segments[i];
			QVector<double> xValues = sliceWorker->getData( i, statistics );

			//So that everything is zero based.
			if ( i == 0 ) {
				statistics->storeIncrement( &xIncr, xValues, -1  );
			}
			statistics->adjustStart( xValues, xIncr );
			statistics->storeIncrement( &xIncr, xValues, i );

			QVector<double> pixels = sliceWorker->getPixels( i );
			sliceSegment->addCurve( plot, xValues, pixels );

			//Add the corner points
			int pointCountPerSegment = qFloor(xValues.size() / cornerCountPerSegment);
			for ( int j = 0; j < cornerCountPerSegment; j++ ) {
				int cornerIndex = (j+1)*pointCountPerSegment - 1;
				addCorner( xValues[cornerIndex], pixels[cornerIndex], plot );
			}


		}

	}

	void ImageSlice::updatePositionInformation(const QVector<String>& info ) {
		int segmentCount = segments.size();
		int infoCount = info.size();
		Assert( segmentCount = infoCount - 1 );
		for ( int i = 0; i< segmentCount; i++ ) {
			segments[i]->updateEnds( info[i], info[i+1]);
		}
	}

	void ImageSlice::updatePolyLine(  const QList<int>& pixelX,
	                                  const QList<int>& pixelY, const QList<double>& worldX,
	                                  const QList<double>& worldY) {
		sliceWorker->setVertices( pixelX, pixelY, worldX, worldY );

		runSliceWorker();

		//Update the segments with new data.
		int segmentCount = segments.size();
		for ( int i = 0; i < segmentCount; i++ ) {
			segments[i]->setEndPointsWorld( worldX[i], worldY[i],
			                                worldX[i+1], worldY[i+1]);
			segments[i]->setEndPointsPixel( pixelX[i], pixelY[i],
			                                pixelX[i+1], pixelY[i+1]);
			segments[i]->updateStatistics( statistics );
		}
	}




	void ImageSlice::clearCurve() {
		QList<SliceSegment*>::iterator iter = segments.begin();
		while ( iter != segments.end() ) {
			(*iter)->clearCurve();
			iter++;
		}
		selected = false;
		clearCorners();
	}

//---------------------------------------------------------------------
//                  Markers between Segments
//---------------------------------------------------------------------

	void ImageSlice::setShowCorners( bool show ) {
		if ( show != showCorners ) {
			showCorners = show;
			if ( showCorners ) {
				resetPlotCurve();
			} else {
				clearCorners();
			}
		}
	}

	void ImageSlice::clearCorners() {
		while ( !segmentCorners.isEmpty() ) {
			QwtPlotMarker* corner = segmentCorners.takeLast();
			corner->detach();
			delete corner;
		}
	}

	void ImageSlice::addCorner( double xValue, double yValue, QwtPlot* plot ) {
		if ( showCorners ) {
			QwtSymbol* sym = new QwtSymbol( QwtSymbol::Cross, QBrush(Qt::black), QPen( Qt::black), QSize(5,5));
			QwtPlotMarker* corner = new QwtPlotMarker();
			sym->setSize( markerSize );
			corner->setSymbol( *sym );
			corner->setValue( xValue, yValue );
			corner->attach( plot );
			segmentCorners.append( corner );
		}
	}

//-----------------------------------------------------------------------------
//               Statistics
//-----------------------------------------------------------------------------

	void ImageSlice::addSegment( SliceSegment* segment ) {
		QLayout* layout = ui.segmentHolder->layout();
		if ( layout == NULL ) {
			layout = new QHBoxLayout();
			ui.segmentHolder->setLayout( layout );
		}
		QHBoxLayout* hLayout = dynamic_cast<QHBoxLayout*>(layout);
		hLayout->addWidget( segment );
	}

	void ImageSlice::removeSegment( SliceSegment* segment ) {
		QLayout* layout = ui.segmentHolder->layout();
		if ( layout != NULL ) {
			layout->removeWidget( segment );
		}
	}

	void ImageSlice::openCloseDisplay() {
		if ( minimized ) {
			maximizeDisplay();
		} else {
			minimizeDisplay();
		}
	}


	void ImageSlice::minimizeDisplay() {
		QLayout* statLayout = this->layout();
		statLayout->removeWidget( ui.regionInfoWidget );
		ui.regionInfoWidget->setParent( NULL );
		minimized = true;
		QIcon icon( ":/images/statsMaximize.png");
		ui.openCloseButton->setIcon( icon );
	}

	void ImageSlice::maximizeDisplay() {
		QLayout* statLayout = this->layout();
		statLayout->addWidget( ui.regionInfoWidget );
		minimized = false;
		QIcon icon( ":/images/statsMinimize.png");
		ui.openCloseButton->setIcon( icon );
	}



	ImageSlice::~ImageSlice() {
		if ( minimized ) {
			delete ui.regionInfoWidget;
		}
		delete sliceWorker;
		clearCorners();
		clearCurve();
	}


} /* namespace casa */
