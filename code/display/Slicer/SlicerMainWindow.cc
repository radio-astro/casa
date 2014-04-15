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
#include "SlicerMainWindow.qo.h"

#include <display/Slicer/SliceZoomer.h>
#include <display/Slicer/SliceColorPreferences.qo.h>
#include <display/Slicer/SlicePlotPreferences.qo.h>

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QCursor>
#include <QFile>
#include <QIntValidator>

#include <qwt_plot_zoomer.h>

#include <limits>

namespace casa {

	SlicerMainWindow::SlicerMainWindow(QWidget *parent)
		: QMainWindow( parent ),  colorPreferences( NULL ),
		  slicePlot( this, true ) {
		ui.setupUi(this);

		setWindowTitle( "Spatial Profile Tool");

		connect( &slicePlot, SIGNAL(markerPositionChanged(int,int,float)), this, SLOT(markPositionChanged(int,int,float)));
		connect( &slicePlot, SIGNAL(markerVisibilityChanged(int,bool)), this, SLOT( markVisibilityChanged(int,bool)));

		//Sample Size
		connect( ui.autoCountCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoCountChanged(bool)));
		ui.autoCountCheckBox->setChecked( true );
		QIntValidator* sampleCountValidator = new QIntValidator( std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), this);
		ui.pointCountLineEdit->setValidator( sampleCountValidator );
		connect( ui.pointCountLineEdit, SIGNAL(editingFinished()), this, SLOT(sampleCountChanged()));

		//Color preferences
		colorPreferences = new SliceColorPreferences( this );
		connect( colorPreferences, SIGNAL( colorsChanged()), this, SLOT(resetColors()));
		resetColors();

		//PlotPreferences
		plotPreferences = new SlicePlotPreferences( this );
		connect( plotPreferences, SIGNAL( plotPreferencesChanged()), this, SLOT(resetPlotPreferences()));
		resetPlotPreferences();

		//Interpolation Method
		methodList = QStringList() << "Nearest" << "Linear" << "Cubic";
		for ( int i = 0; i < methodList.size(); i++ ) {
			ui.methodComboBox->addItem( methodList[i] );
		}
		connect( ui.methodComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(interpolationMethodChanged(const QString&)));

		//SegmentMarkerCheckBox
		connect( ui.segmentCornerCheckBox, SIGNAL(toggled(bool)), &slicePlot, SLOT(segmentMarkerVisibilityChanged(bool)));

		//Add the plot to the widget
		QHBoxLayout* layout = new QHBoxLayout(ui.plotFrame);
		layout->setContentsMargins( 0, 0, 0, 0 );
		layout->addWidget( &slicePlot );

		connect( ui.markedRegionCheckBox, SIGNAL(toggled(bool)), this, SLOT(accumulateChanged(bool)));

		//QwtPlotPanner* plotPanner = new QwtPlotPanner( slicePlot.canvas() );
		//plotPanner->setMouseButton( Qt::RightButton, Qt::NoButton);
		ui.plotFrame->setLayout( layout );
		ui.plotFrame->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

		//X-Axis units
		ui.xAxisComboBox->addItem( SlicePlot::DISTANCE_AXIS );
		ui.xAxisComboBox->addItem( SlicePlot::POSITION_X_AXIS );
		ui.xAxisComboBox->addItem( SlicePlot::POSITION_Y_AXIS );
		ui.unitComboBox->addItem( SlicePlot::UNIT_X_PIXEL );
		ui.unitComboBox->addItem( SlicePlot::UNIT_X_ARCSEC );
		ui.unitComboBox->addItem( SlicePlot::UNIT_X_ARCMIN );
		ui.unitComboBox->addItem( SlicePlot::UNIT_X_ARCDEG );
		connect(ui.xAxisComboBox, SIGNAL(currentIndexChanged(const QString&)), &slicePlot, SLOT(setXAxis(const QString&)));
		connect(ui.unitComboBox, SIGNAL(currentIndexChanged(const QString&)), &slicePlot, SLOT(xAxisUnitsChanged( const QString& )));

		//Clear
		connect( ui.clearButton, SIGNAL(clicked()), this, SLOT(clearCurves()));

		//Actions
		connect(ui.actionExport, SIGNAL(triggered()), this, SLOT(exportSlice()));
		connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(showPlotPreferences()));
		connect(ui.actionColor, SIGNAL(triggered()), this, SLOT(showColorDialog()));

		initializeZooming();

		updateStatisticsLayout();
	}

//----------------------------------------------------------------------------------
//                         Zooming
//----------------------------------------------------------------------------------

	void SlicerMainWindow::initializeZooming() {

		plotZoomer = new SliceZoomer( slicePlot.canvas() );
		connect( plotZoomer, SIGNAL(zoomed(const QwtDoubleRect&)), this, SLOT(checkZoom()));

		connect(ui.actionZoomIn, SIGNAL(triggered()), this, SLOT( zoomIn()));
		connect(ui.actionZoomOut, SIGNAL(triggered()), this, SLOT( zoomOut()));
		connect(ui.actionZoomNeutral, SIGNAL(triggered()), this, SLOT( zoomNeutral()));
	}

	bool SlicerMainWindow::checkZoom() {
		//We we revert a zoom back to the original, we won't see
		//the plot unless auto-scaling is turned back on.
		bool neutralState = false;
		if ( plotZoomer->zoomRectIndex() == 0 ) {
			bool scaleChanged = false;
			if ( ! slicePlot.axisAutoScale( QwtPlot::xBottom )) {
				slicePlot.setAxisAutoScale( QwtPlot::xBottom);
				scaleChanged = true;
			}
			if ( !slicePlot.axisAutoScale( QwtPlot::yLeft)) {
				slicePlot.setAxisAutoScale( QwtPlot::yLeft);
				scaleChanged = true;
			}
			if ( scaleChanged ) {
				slicePlot.replot();
			}
			neutralState = true;
		}
		return neutralState;
	}

	void SlicerMainWindow::zoomIn() {
		plotZoomer->zoomIn( );
	}

	void SlicerMainWindow::zoomOut() {
		plotZoomer->zoomOut();
	}

	void SlicerMainWindow::zoomNeutral() {
		if ( !checkZoom()) {
			plotZoomer->zoomNeutral();
		}
	}

//-----------------------------------------------------------------------
//             Colors
//-----------------------------------------------------------------------

	void SlicerMainWindow::resetColors() {
		bool viewerColors = colorPreferences->isViewerColors();
		bool polylineColorUnit = colorPreferences->isPolylineUnit();
		slicePlot.resetCurveColors( viewerColors, polylineColorUnit,
		                            colorPreferences->getAccumulatedSliceColors() );
	}


	void SlicerMainWindow::showColorDialog() {
		colorPreferences->show();
	}

	void SlicerMainWindow::setCurveColor( int regionId, const QString& colorName ) {
		slicePlot.setViewerCurveColor( regionId, colorName );
	}

//-------------------------------------------------------------------------
//           Plot Preferences
//------------------------------------------------------------------------

	void SlicerMainWindow::showPlotPreferences() {
		plotPreferences->show();
	}

	void SlicerMainWindow::resetPlotPreferences() {
		int curveWidth = plotPreferences->getLineWidth();
		int markerSize = plotPreferences->getMarkerSize();
		slicePlot.setPlotPreferences( curveWidth, markerSize );
	}

//---------------------------------------------------------------------------------
//                   Export
//---------------------------------------------------------------------------------

	void SlicerMainWindow::exportSlice() {
		QString fileName = QFileDialog::getSaveFileName(this,tr("Export 1D Slice"),
		                   QString(), tr( "Images (*.png *.jpg);; Text files (*.txt)"));
		if ( fileName.length() > 0 ) {
			QString ext = fileName.section('.', -1);
			bool successfulExport = true;
			if (ext =="png" || ext == "jpg") {
				successfulExport = toImageFormat(fileName, ext );
			} else {
				if (ext != "txt") {
					fileName.append(".txt");
				}
				successfulExport = toASCII(fileName);
			}
		}
	}

	bool SlicerMainWindow::toASCII( const QString& fileName ) {
		return slicePlot.toAscii( fileName );
	}


	bool SlicerMainWindow::toImageFormat( const QString& fileName, const QString& format ) {
		QSize plotSize = ui.plotFrame->size();
		QPixmap pixmap(plotSize.width(), plotSize.height());
		pixmap.fill(Qt::white );
		QwtPlotPrintFilter filter;
		int options = QwtPlotPrintFilter::PrintFrameWithScales | QwtPlotPrintFilter::PrintBackground;
		filter.setOptions( options );
		slicePlot.print( pixmap, filter );
		bool imageSaved = pixmap.save( fileName, format.toStdString().c_str());
		if ( !imageSaved ) {
			QString msg("There was a problem saving the histogram.\nPlease check the file path.");
			QMessageBox::warning( this, "Save Problem", msg);
		}
		return imageSaved;
	}

//-------------------------------------------------------------------------------
//                        UI SLOTS
//-------------------------------------------------------------------------------

	void SlicerMainWindow::interpolationMethodChanged( const QString& method ) {
		QString methodQ = method;
		methodQ = methodQ.toLower();
		slicePlot.setInterpolationMethod( methodQ.toStdString() );
	}

	void SlicerMainWindow::accumulateChanged( bool accumulate ) {
		slicePlot.setAccumulateSlices( accumulate );
	}

	void SlicerMainWindow::autoCountChanged( bool selected ) {
		ui.pointCountLineEdit->setEnabled( !selected );
		if ( !selected ) {
			ui.pointCountLineEdit->setText( "0");
		}
		sampleCountChanged();
	}


	int SlicerMainWindow::populateSampleCount() const {
		int sampleCount = 0;
		if ( !ui.autoCountCheckBox->isChecked()) {
			QString countStr = ui.pointCountLineEdit->text();
			sampleCount = countStr.toInt();
		}
		return sampleCount;
	}

	void SlicerMainWindow::sampleCountChanged() {
		int sampleCount = populateSampleCount();
		slicePlot.setSampleCount( sampleCount );
	}


	void SlicerMainWindow::clearCurves() {
		slicePlot.clearCurvesAll();
	}

	void SlicerMainWindow::markPositionChanged(int regionId,int segmentIndex,float percentage) {
		emit markerPositionChanged(regionId,segmentIndex,percentage);
	}
	void SlicerMainWindow::markVisibilityChanged(int regionId,bool showMarker) {
		emit markerVisibilityChanged(regionId, showMarker);
	}

//--------------------------------------------------------------------------------
//               Region and image changes
//--------------------------------------------------------------------------------

	bool SlicerMainWindow::setRegionSelected( int regionId, bool selected ) {
		return slicePlot.setRegionSelected( regionId, selected );
	}

	void SlicerMainWindow::updateStatisticsLayout() {
		QWidget* statsWidget = new QWidget( this );
		ui.statsScrollArea->setWidget( statsWidget );
		QVBoxLayout* layout = new QVBoxLayout();
		layout->setSizeConstraint(QLayout::SetMinAndMaxSize );
		statsWidget->setLayout( layout );
		slicePlot.setStatisticsLayout( layout  );
	}

	void SlicerMainWindow::updatePositionInformation( int id, const QVector<String>& info ) {
		slicePlot.updatePositionInformation( id, info );
	}

	void SlicerMainWindow::updatePolyLine(  int regionId, viewer::region::RegionChanges regionChanges,
	                                        const QList<double> & worldX, const QList<double> & worldY,
	                                        const QList<int> &pixelX, const QList<int> & pixelY ) {
		slicePlot.updatePolyLine( regionId, regionChanges, worldX, worldY, pixelX, pixelY);
	}


	void SlicerMainWindow::addPolyLine(  int regionId, viewer::region::RegionChanges regionChanges,
	                                     const QList<double> & worldX, const QList<double> & worldY,
	                                     const QList<int> &pixelX, const QList<int> & pixelY, const QString& colorName ) {

		if ( regionChanges == viewer::region::RegionChangeCreate ) {
			//Update the plot
			slicePlot.updatePolyLine( regionId, regionChanges,
			                          worldX, worldY, pixelX, pixelY );
			slicePlot.setViewerCurveColor( regionId, colorName );
		}
	}

	void SlicerMainWindow::updateChannel( int channel ) {
		slicePlot.updateChannel( channel );
	}


	void SlicerMainWindow::setImage( std::tr1::shared_ptr<ImageInterface<float> > img ) {
		slicePlot.setImage( img );
	}


	SlicerMainWindow::~SlicerMainWindow() {
		delete plotZoomer;
		delete colorPreferences;
	}
}
