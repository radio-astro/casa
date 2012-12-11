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
#include "BinPlotWidget.qo.h"
#include <images/Images/ImageHistograms.h>
#include <images/Images/SubImage.h>
#include <guitools/Histogram/FitWidget.qo.h>
#include <guitools/Histogram/HistogramMarkerGaussian.h>
#include <guitools/Histogram/HistogramMarkerPoisson.h>
#include <guitools/Histogram/RangeControlsWidget.qo.h>
#include <guitools/Histogram/RangePicker.h>
#include <guitools/Histogram/ToolTipPicker.h>

#include <QDebug>
#include <QPainter>
#include <QMessageBox>
#include <QFile>
#include <QToolTip>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_text_label.h>

namespace casa {


BinPlotWidget::BinPlotWidget( bool fitControls, bool rangeControls,
		bool plotModeControls, QWidget* parent ):
    QWidget(parent),
    curveColor( Qt::blue ), selectionColor( 205, 201, 201, 127 ),
    histogramMaker( NULL ), image( NULL ), binPlot( this ),
    NO_DATA( "No Data"), NO_DATA_MESSAGE( "Data is needed in order to zoom."), contextMenuZoom(this),
    zoomRangeAction( "Zoom Range", this), zoom95Action( "Zoom 95% of Peak", this),
    zoom98Action("Zoom 98% of Peak", this), zoom995Action ( "Zoom 99.5% of Peak", this),
    zoom999Action( "Zoom 99.9% of Peak", this), zoomNeutralAction( "Zoom Neutral", this),
    lambdaAction("Lambda",this), centerPeakAction( "(Center,Peak)",this),
    fwhmAction( "Center +/- FWHM/2", this), contextMenu(this),
    stepFunctionAction( "Histogram",this), logAction( "Log(Count)", this),
    regionModeAction( "Plot Selected Region", this), imageModeAction("Plot Image", this),
    regionAllModeAction( "Plot All Regions", this ), contextMenuDisplay( this ){

	ui.setupUi(this);

	//Add the plot to the widget
	QHBoxLayout* layout = new QHBoxLayout(ui.plotHolder);
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( &binPlot );
	binPlot.setCanvasBackground( Qt::white );
	setAxisLabelFont( 8);

	ui.plotHolder->setLayout( layout );

	initializePlotControls();
	initializeFitWidget( fitControls );
	initializeRangeControls( rangeControls );
	initializePlotModeControls( plotModeControls );

	displayPlotTitle = false;
	displayAxisTitles = false;
}

void BinPlotWidget::setAxisLabelFont( int size ){
	QFont font = binPlot.axisFont( QwtPlot::xBottom );
	font.setPointSize( size );
	binPlot.setAxisFont( QwtPlot::xBottom, font );
	binPlot.setAxisFont( QwtPlot::yLeft, font );
}

void BinPlotWidget::setDisplayPlotTitle( bool display ){
	if ( displayPlotTitle != display ){
		displayPlotTitle = display;
		resetPlotTitle();
	}
}

void BinPlotWidget::setDisplayAxisTitles( bool display ){
	displayAxisTitles = display;
	resetAxisTitles();
}

void BinPlotWidget::resetAxisTitles(){
	if ( displayAxisTitles ){
		QString yAxisTitle = "Count";
		if ( this->displayLog ){
			yAxisTitle = "Log(Count)";
		}
		QwtText leftTitle = binPlot.axisTitle(QwtPlot::yLeft);
		QFont titleFont = leftTitle.font();
		titleFont.setPointSize(8);
		leftTitle.setFont( titleFont );
		leftTitle.setText( yAxisTitle );
		binPlot.setAxisTitle( QwtPlot::yLeft, leftTitle );
		leftTitle.setText( "Intensity");
		binPlot.setAxisTitle( QwtPlot::xBottom, leftTitle );
	}
	else {
		binPlot.setAxisTitle( QwtPlot::yLeft, "");
		binPlot.setAxisTitle( QwtPlot::xBottom, "");
	}
}

void BinPlotWidget::resetPlotTitle(){
	QString imageNameStr;
	if ( image != NULL && displayPlotTitle ){
		String imageName = image->name(True );
		imageNameStr = imageName.c_str();
	}
	QwtText titleText = binPlot.title();
	QFont titleFont = titleText.font();
	titleFont.setPointSize( 9 );
	titleFont.setBold( true );
	titleText.setFont( titleFont );
	titleText.setText( imageNameStr );
	binPlot.setTitle( titleText );
}

void BinPlotWidget::setHistogramColor( QColor color ){
	if ( curveColor != color ){
		curveColor = color;
		for( int i = 0; i <curves.size(); i++ ){
			QPen pen = curves[i]->pen();
			pen.setColor( curveColor );
			curves[i]->setPen( pen );
		}
		binPlot.replot();
	}
}

void BinPlotWidget::setFitEstimateColor( QColor color ){
	if ( fitEstimateColor != color ){
		fitEstimateColor = color;
		if ( fitEstimateMarkerGaussian != NULL ){
			fitEstimateMarkerGaussian->setColor(fitEstimateColor);
			binPlot.replot();
		}
		else if ( fitEstimateMarkerPoisson != NULL ){
			fitEstimateMarkerPoisson->setColor( fitEstimateColor );
			binPlot.replot();
		}
	}
}

void BinPlotWidget::setFitCurveColor( QColor color ){
	if ( fitCurveColor != color ){
		fitCurveColor = color;
		if ( fitCurve != NULL ){
			QPen curvePen = fitCurve->pen();
			curvePen.setColor( fitCurveColor );
			fitCurve->setPen( curvePen );
			binPlot.replot();
		}
	}
}

bool BinPlotWidget::isEmpty() const {
	bool empty = false;
	if ( xVector.size() == 0 ){
		empty = true;
	}
	return empty;
}

//-------------------------------------------------------------------------------
//                            Display Settings
//-------------------------------------------------------------------------------

void BinPlotWidget::initializePlotModeControls( bool enable ){

	if ( enable ){
		QActionGroup* plotControlsGroup = new QActionGroup( this );
		plotControlsGroup->addAction( &regionModeAction );
		plotControlsGroup->addAction( &imageModeAction );
		plotControlsGroup->addAction( &regionAllModeAction );
		regionModeAction.setCheckable( true );
		imageModeAction.setCheckable( true );
		regionAllModeAction.setCheckable( true );
		regionModeAction.setChecked( true );
		contextMenuDisplay.addSeparator();
		contextMenuDisplay.addAction( &imageModeAction );
		contextMenuDisplay.addAction( &regionModeAction );
		contextMenuDisplay.addAction( &regionAllModeAction );
		connect( &imageModeAction, SIGNAL(triggered(bool)), this, SLOT(imageModeSelected(bool)));
		connect( &regionModeAction, SIGNAL(triggered(bool)), this, SLOT(regionModeSelected(bool)));
		connect( &regionAllModeAction, SIGNAL(triggered(bool)), this, SLOT(regionAllModeSelected(bool)));
	}
	plotMode = REGION_MODE;
}

void BinPlotWidget::imageModeSelected( bool enabled ){
	if ( enabled ){
		if ( plotMode != IMAGE_MODE ){
			plotMode = IMAGE_MODE;
			resetImage();
		}
	}
}

void BinPlotWidget::regionModeSelected( bool enabled ){
	if ( enabled ){
		if ( plotMode != REGION_MODE ){
			plotMode = REGION_MODE;
			resetRegion( -1 );
		}
	}
}

void BinPlotWidget::regionAllModeSelected( bool enabled ){
	if ( enabled ){
		if ( plotMode != REGION_ALL_MODE ){
			plotMode = REGION_ALL_MODE;
			resetRegion( -1 );
		}
	}
}

void BinPlotWidget::addPlotModeActions( QMenu* menu ){
	menu->addSeparator();
	menu->addAction( &imageModeAction );
	menu->addAction( &regionModeAction );
	menu->addAction( &regionAllModeAction );
}

void BinPlotWidget::setPlotMode( int mode ){
	if ( mode == REGION_MODE ){
		plotMode = REGION_MODE;
		regionModeAction.setChecked( true );
	}
	else if ( mode == IMAGE_MODE ){
		plotMode = IMAGE_MODE;
		imageModeAction.setChecked( true );
	}
	else if ( mode == REGION_ALL_MODE ){
		plotMode = REGION_ALL_MODE;
		regionAllModeAction.setChecked( true );
	}
	else {
		qWarning() << "Unrecognized plot mode";
	}
}

void BinPlotWidget::initializePlotControls(){
	displayStep = true;
	displayLog = false;
	logAction.setCheckable( true );
	logAction.setChecked( displayLog );
	stepFunctionAction.setCheckable( true );
	stepFunctionAction.setChecked( displayStep );
	contextMenuDisplay.addAction( &logAction );
	contextMenuDisplay.addAction( &stepFunctionAction );
	connect( &logAction, SIGNAL(triggered(bool)), this, SLOT(setDisplayLog(bool)));
	connect( &stepFunctionAction, SIGNAL(triggered(bool)), this, SLOT(setDisplayStep(bool)));
	if ( toolTipPicker != NULL ){
		setDisplayLog( displayLog );
	}
}

void BinPlotWidget::addDisplayActions( QMenu* menu ){
	menu->addAction( &logAction );
	menu->addAction( &stepFunctionAction );
}

void BinPlotWidget::setDisplayStep( bool display ){
	if ( displayStep != display ){
		displayStep = display;
		stepFunctionAction.setChecked( displayStep );
		defineCurve();
	}
}

void BinPlotWidget::setDisplayLog( bool display ){
	if ( displayLog != display ){
		displayLog = display;
		logAction.setChecked( displayLog );
		defineCurve();
		resetAxisTitles();
		if ( toolTipPicker != NULL ){
			toolTipPicker->setLogScale( display );
		}
	}
}


//-------------------------------------------------------------------------------
//                          Marking Ranges on the Histogram
//-------------------------------------------------------------------------------

void BinPlotWidget::initializeRangeControls( bool enable ){
	//This is what draws the final rectangle.
	rectMarker = new RangePicker();
	rectMarker->setHeightSource( this );
	rectMarker->attach( &binPlot );

	//This is what draws the rectangle while it is being dragged.
	dragLine = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
					QwtPlotPicker::PointSelection | QwtPlotPicker::DragSelection,
					QwtPlotPicker::VLineRubberBand, QwtPlotPicker::ActiveOnly, binPlot.canvas() );
	connect( dragLine, SIGNAL(selected(const QwtDoublePoint &)), this, SLOT(lineSelected()));
	connect( dragLine, SIGNAL(moved(const QPoint&)), this, SLOT(lineMoved( const QPoint&)));

	//Put the range controls in
	if ( enable ){
		QHBoxLayout* layout = new QHBoxLayout();
		layout->setContentsMargins(1,1,1,1);
		rangeControlWidget = new RangeControlsWidget( this );
		layout->addWidget( rangeControlWidget );
		ui.rangeControlsHolder->setLayout( layout );
	}
	else {
		rangeControlWidget = new RangeControlsWidget(NULL);
	}
	connect( rangeControlWidget, SIGNAL(minMaxChanged()), this, SLOT(minMaxChanged()));
	connect( rangeControlWidget, SIGNAL(rangeCleared()), this, SLOT(clearRange()));

	//Tool tips
	toolTipPicker = new ToolTipPicker( QwtPlot::xBottom, QwtPlot::yLeft,
			QwtPlotPicker::PointSelection, QwtPlotPicker::NoRubberBand,
			QwtPlotPicker::AlwaysOn, binPlot.canvas());

	//Zooming
	QActionGroup* zoomGroup = new QActionGroup( this );
	zoomGroup->addAction( &zoomRangeAction );
	zoomGroup->addAction( &zoom95Action );
	zoomGroup->addAction( &zoom98Action );
	zoomGroup->addAction( &zoom995Action );
	zoomGroup->addAction( &zoom999Action );
	zoomGroup->addAction( &zoomNeutralAction );
	zoomRangeAction.setCheckable(true);
	zoom95Action.setCheckable(true);
	zoom98Action.setCheckable(true);
	zoom995Action.setCheckable(true);
	zoom999Action.setCheckable(true);
	zoomNeutralAction.setCheckable(true);
	zoomNeutralAction.setChecked( true );
	connect( &zoomRangeAction, SIGNAL(triggered(bool)), this, SLOT(zoomRange(bool)));
	connect( &zoom95Action, SIGNAL(triggered(bool)), this, SLOT(zoom95(bool)));
	connect( &zoom98Action, SIGNAL(triggered(bool)), this, SLOT(zoom98(bool)));
	connect( &zoom995Action, SIGNAL(triggered(bool)), this, SLOT(zoom995(bool)));
	connect( &zoom999Action, SIGNAL(triggered(bool)), this, SLOT(zoom999(bool)));
	connect( &zoomNeutralAction, SIGNAL(triggered(bool)), this, SLOT(zoomNeutral(bool)));
	contextMenuZoom.addAction( &zoomRangeAction );
	contextMenuZoom.addAction( &zoom95Action );
	contextMenuZoom.addAction( &zoom98Action );
	contextMenuZoom.addAction( &zoom995Action );
	contextMenuZoom.addAction( &zoom999Action );
	contextMenuZoom.addAction( &zoomNeutralAction );
	contextMenuMode = ZOOM_CONTEXT;
}

float BinPlotWidget::getTotalCount() const {
	float count = 0;
	for ( int i = 0; i < static_cast<int>(yVector.size()); i++ ){
		count = count + yVector[i];
	}
	return count;
}

int BinPlotWidget::getPeakIndex() const {
	int peakIndex = -1;
	double maxCount = numeric_limits<double>::min();
	for ( int i = 0; i < static_cast<int>(yVector.size()); i++ ){
		if ( yVector[i] > maxCount ){
			maxCount = yVector[i];
			peakIndex = i;
		}
	}
	return peakIndex;
}

void BinPlotWidget::zoomRangeMarker( double startValue, double endValue ){
	clearGaussianFitMarker();
	clearPoissonFitMarker();
	pair<double,double> worldRange = rangeControlWidget->getMinMaxValues();
	double worldMin = qMax( worldRange.first, startValue );
	double worldMax = qMin( worldRange.second, endValue );
	if ( worldMin < worldMax ){
		//Change to plot coordinates
		int canvasWidth = binPlot.canvas()->width();
		int lowerBoundPixel = static_cast<int>(qAbs(worldMin - startValue)/qAbs(endValue - startValue)*canvasWidth);
		int upperBoundPixel = static_cast<int>(qAbs(worldMax - startValue)/qAbs(endValue - startValue)*canvasWidth);
		rectMarker->setBoundaryValues( lowerBoundPixel, upperBoundPixel );
	}
}

void BinPlotWidget::zoom( float percent ){
	int peakIndex = getPeakIndex();
	if ( peakIndex >= 0 ){
		float totalCount = getTotalCount();
		float targetCount = totalCount * percent;
		int startIndex = peakIndex;
		int endIndex = peakIndex;
		float runningCount = yVector[peakIndex];
		while( runningCount < targetCount ){
			startIndex = startIndex - 1;
			if ( startIndex > 0 ){
				runningCount = runningCount + yVector[startIndex];
			}
			if ( runningCount >= targetCount ){
				break;
			}
			endIndex = endIndex + 1;
			if ( endIndex < static_cast<int>(yVector.size()) ){
				runningCount = runningCount + yVector[endIndex];
			}
		}
		startIndex = qMax( 0, startIndex );
		int yVectorCount = yVector.size();
		endIndex = qMin( yVectorCount - 1, endIndex );
		if ( startIndex == endIndex ){
			if ( startIndex > 0 ){
				startIndex--;
			}
			else {
				if ( endIndex < yVectorCount - 1){
					endIndex++;
				}
			}
		}
		binPlot.setAxisScale( QwtPlot::xBottom, xVector[startIndex], xVector[endIndex]);
		zoomRangeMarker( xVector[startIndex], xVector[endIndex]);
		binPlot.replot();
	}
}

void BinPlotWidget::zoomRange( bool rangeZoom ){
	if ( !isEmpty() ){
		if ( rangeZoom ){
			int lowerBound = rectMarker->getLowerBound();
			int upperBound = rectMarker ->getUpperBound();
			if ( lowerBound < upperBound ){
				//Change to world coordinates
				double lowerBoundWorld = binPlot.invTransform( QwtPlot::xBottom, lowerBound );
				double upperBoundWorld = binPlot.invTransform( QwtPlot::xBottom, upperBound );
				binPlot.setAxisScale( QwtPlot::xBottom, lowerBoundWorld, upperBoundWorld );
				zoomRangeMarker( lowerBoundWorld, upperBoundWorld );
				binPlot.replot();
			}
			else {
				QMessageBox::warning( this, NO_DATA, NO_DATA_MESSAGE );
				zoomNeutralAction.setChecked( true );
			}
		}
	}
	else {
		if ( rangeZoom ){
			QMessageBox::warning( this, "Range Not Specified", "Please specify a zoom range on the x-axis by dragging the left mouse.");
			zoomNeutralAction.setChecked( true );
		}
	}
}

void BinPlotWidget::zoomNeutral( bool neutralZoom ){
	if ( neutralZoom ){
		if ( !isEmpty() ){
			float lastValue = xVector[xVector.size() - 1];
			float firstValue = xVector[0];
			binPlot.setAxisScale( QwtPlot::xBottom, firstValue, lastValue);
			zoomRangeMarker( firstValue, lastValue );
			binPlot.replot();
		}
	}
}

void BinPlotWidget::zoom95( bool zoomValues ){
	if ( zoomValues ){
		if ( !isEmpty() ){
			zoom( 0.95f);
		}
		else {
			QMessageBox::warning( this, NO_DATA, NO_DATA_MESSAGE );
			zoomNeutralAction.setChecked( true );
		}
	}
}

void BinPlotWidget::zoom98( bool zoomValues ){
	if ( zoomValues ){
		if ( !isEmpty() ){
			zoom( 0.98f );
		}
		else {
			QMessageBox::warning( this, NO_DATA, NO_DATA_MESSAGE );
			zoomNeutralAction.setChecked( true );
		}
	}
}

void BinPlotWidget::zoom995( bool zoomValues ){
	if ( zoomValues ){
		if ( !isEmpty() ){
			zoom( 0.995f );
		}
		else {
			QMessageBox::warning( this, NO_DATA, NO_DATA_MESSAGE );
			zoomNeutralAction.setChecked( true );
		}
	}
}

void BinPlotWidget::zoom999( bool zoomValues ){
	if ( zoomValues ){
		if ( !isEmpty() ){
			zoom( 0.999f );
		}
		else {
			QMessageBox::warning( this, NO_DATA, NO_DATA_MESSAGE );
			zoomNeutralAction.setChecked( true );
		}
	}
}
void BinPlotWidget::addZoomActions( QMenu* zoomMenu ){
	zoomMenu->addAction( &zoomRangeAction );
	zoomMenu->addAction( &zoom95Action );
	zoomMenu->addAction( &zoom98Action );
	zoomMenu->addAction( &zoom995Action );
	zoomMenu->addAction( &zoom999Action );
	zoomMenu->addAction( &zoomNeutralAction );
}

void BinPlotWidget::setMinMaxValues( double minValue, double maxValue,
		bool updateGraph ){
	if ( rangeControlWidget != NULL ){
		rangeControlWidget->setRange( minValue, maxValue );
	}
	if ( updateGraph ){
		minMaxChanged();
	}
}

pair<double,double> BinPlotWidget::getMinMaxValues() const {
	pair<double,double> minMaxValues( numeric_limits<double>::min(), numeric_limits<double>::max());
	if ( rangeControlWidget != NULL ){
		minMaxValues = rangeControlWidget->getMinMaxValues();
	}
	else {
		qWarning() << "Warning:  min/max values won't be valid without range control enabled";
	}
	return minMaxValues;
}


//-------------------------------------------------------------------------------
//                         Fitting the Histogram
//-------------------------------------------------------------------------------


void BinPlotWidget::clearGaussianFitMarker(){
	delete fitEstimateMarkerGaussian;
	fitEstimateMarkerGaussian = NULL;
}

void BinPlotWidget::clearPoissonFitMarker(){
	delete fitEstimateMarkerPoisson;
	fitEstimateMarkerPoisson = NULL;
}

void BinPlotWidget::initializeGaussianFitMarker(){
	fitEstimateMarkerGaussian = new HistogramMarkerGaussian();
	fitEstimateMarkerGaussian->attach( &binPlot );
	fitEstimateMarkerGaussian->setColor( fitEstimateColor );
}

void BinPlotWidget::initializePoissonFitMarker(){
	fitEstimateMarkerPoisson = new HistogramMarkerPoisson();
	fitEstimateMarkerPoisson->attach( &binPlot );
	fitEstimateMarkerPoisson->setColor( fitEstimateColor );
}


void BinPlotWidget::initializeFitWidget( bool fitControls ){
	fitCurve = NULL;
	fitEstimateMarkerGaussian = NULL;
	fitEstimateMarkerPoisson = NULL;
	setContextMenuPolicy( Qt::CustomContextMenu);
	connect( this, SIGNAL(customContextMenuRequested( const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

	if ( fitControls ){
		QVBoxLayout* fitLayout = new QVBoxLayout();
		fitWidget = new FitWidget( this );
		fitLayout->setContentsMargins(1,1,1,1);
		fitLayout->addWidget( fitWidget );
		ui.fitWidgetHolder->setLayout( fitLayout );
		fitModeChanged();

		connect( fitWidget, SIGNAL(fitModeChanged()), this, SLOT(fitModeChanged()));
		connect( fitWidget, SIGNAL(fitCleared()), this, SLOT(clearFit()));
		connect( fitWidget, SIGNAL(dataFitted(const QString&)), this, SLOT(fitDone(const QString&)));
		connect( fitWidget, SIGNAL(gaussianFitChanged()), this, SLOT(resetGaussianFitMarker()));
		connect( fitWidget, SIGNAL(poissonFitChanged()), this, SLOT(resetPoissonFitMarker()));
		connect( &centerPeakAction, SIGNAL(triggered()), this, SLOT(centerPeakSpecified()));
		connect( &fwhmAction, SIGNAL(triggered()), this, SLOT(fwhmSpecified()));
		connect( &lambdaAction, SIGNAL(triggered()), this, SLOT(lambdaSpecified()));
	}
	else {
		fitWidget = NULL;
	}
}

void BinPlotWidget::centerPeakSpecified(){
	clearPoissonFitMarker();
	if ( fitEstimateMarkerGaussian == NULL ){
		initializeGaussianFitMarker();
	}

	//Translate the pixels to world coordinates so
	//that they can be displayed in the fit widget.
	int pixelX = fitPosition.x();
	int pixelY = fitPosition.y();
	double xValue = binPlot.invTransform( QwtPlot::xBottom, pixelX );
	double yValue = binPlot.invTransform( QwtPlot::yLeft, pixelY );
	if ( displayLog ){
		yValue = pow( 10.0, yValue );
	}
	fitWidget->setCenterPeak( xValue, yValue );

	fitEstimateMarkerGaussian->setCenterPeak( pixelX, pixelY);
	fitEstimateMarkerGaussian->show();
	binPlot.replot();
}

void BinPlotWidget::fwhmSpecified(){
	clearPoissonFitMarker();
	if ( fitEstimateMarkerGaussian == NULL ){
		initializeGaussianFitMarker();
	}
	int pixelX = fitPosition.x();
	int pixelY = fitPosition.y();
	double xValue = binPlot.invTransform( QwtPlot::xBottom, pixelX );
	fitWidget->setFWHM( xValue );

	fitEstimateMarkerGaussian->setFWHM( pixelX, pixelY);
	fitEstimateMarkerGaussian->show();
	binPlot.replot();
}

void BinPlotWidget::lambdaSpecified(){
	clearGaussianFitMarker();
	if ( fitEstimateMarkerPoisson == NULL ){
		initializePoissonFitMarker();
	}
	int pixelX = fitPosition.x();
	double xValue = binPlot.invTransform( QwtPlot::xBottom, pixelX );
	fitWidget->setLambda( xValue );
	fitEstimateMarkerPoisson->setLambda( pixelX );
	fitEstimateMarkerPoisson->show();
	binPlot.replot();
}

void BinPlotWidget::clearFit(){
	if (fitCurve != NULL ){
		fitCurve->detach();
		delete fitCurve;
		fitCurve = NULL;
	}
	clearGaussianFitMarker();
	clearPoissonFitMarker();
	binPlot.replot();
	emit postStatusMessage( "" );
}

void BinPlotWidget::fitModeChanged(){
	contextMenu.clear();

	if ( fitWidget->isGaussian() ){
		contextMenu.addAction(&centerPeakAction );
		contextMenu.addAction( &fwhmAction );
		clearPoissonFitMarker();
	}
	else {
		clearGaussianFitMarker();
		contextMenu.addAction( &lambdaAction );
	}
}



bool BinPlotWidget::isPlotContains( int x, int y ){
	int insidePlot = false;
	QRect plotHolderGeometry = ui.plotHolder->frameGeometry();
	int canvasWidth = binPlot.canvas()->width();
	int canvasHeight = binPlot.canvas()->height();
	int left = plotHolderGeometry.x() + (plotHolderGeometry.width() - canvasWidth);
	int top = plotHolderGeometry.y();
	int right = left + canvasWidth;
	int bottom = top + canvasHeight;
	if ( left <= x && x <= right ){
		if ( top <= y && y <= bottom ){
			insidePlot = true;
		}
	}
	return insidePlot;
}

void BinPlotWidget::showContextMenu( const QPoint& pt ){
	QPoint globalPos = mapToGlobal( pt );
	int x = pt.x();
	int y = pt.y();
	bool ptInPlot = isPlotContains( x, y );
	if ( ptInPlot ){
		if ( contextMenuMode == FIT_CONTEXT ){
			//Change x by the amount the y-axis takes up.
			QRect plotGeom = ui.plotHolder->geometry();
			QwtPlotCanvas* plotCanvas = binPlot.canvas();
			int yAxisSpace = binPlot.width() - plotCanvas->width();
			QwtTextLabel* titleLabel = binPlot.titleLabel();
			int titleSpace = 0;
			if ( titleLabel != NULL ){
				titleSpace = titleLabel->sizeHint().height();
			}
			int adjustedX = x - yAxisSpace - plotGeom.x();
			int adjustedY = y - titleSpace - plotGeom.y();
			fitPosition = QPoint( adjustedX, adjustedY);
			if ( ! isEmpty() ){
				contextMenu.exec( globalPos );
			}
		}
		else if ( contextMenuMode == DISPLAY_CONTEXT ){
			contextMenuDisplay.exec( globalPos );
		}
		else if ( contextMenuMode == ZOOM_CONTEXT ){
			if ( ! isEmpty() ){
				contextMenuZoom.exec( globalPos );
			}
		}
	}
	contextMenuMode = ZOOM_CONTEXT;
}

void BinPlotWidget::resetGaussianFitMarker(){
	if ( this->fitEstimateMarkerGaussian != NULL ){
		double centerValue = fitWidget->getCenter();
		double peakValue = fitWidget->getPeak();
		double fwhmLength = fitWidget->getFWHM();
		double fwhmValue = centerValue + fwhmLength;
		int centerPixel = binPlot.transform( QwtPlot::xBottom, centerValue );
		int fwhmPixel = binPlot.transform( QwtPlot::xBottom, fwhmValue );
		int peakPixel = binPlot.transform( QwtPlot::yLeft, peakValue );
		fitEstimateMarkerGaussian->setCenterPeak( centerPixel, peakPixel );
		fitEstimateMarkerGaussian->setFWHM( fwhmPixel, peakPixel/2 );
		binPlot.replot();
	}
}

void BinPlotWidget::resetPoissonFitMarker(){
	if ( this->fitEstimateMarkerPoisson != NULL ){
		double lambdaValue = fitWidget->getLambda();
		int lambdaPixel = binPlot.transform( QwtPlot::xBottom, lambdaValue );
		fitEstimateMarkerPoisson->setLambda( lambdaPixel );
		binPlot.replot();
	}
}


void BinPlotWidget::fitDone( const QString& msg ){
	emit postStatusMessage( msg );
	Vector<Float> yValues = fitWidget->getFitValues();
	if ( yValues.size() > 0 ){
		if ( fitCurve != NULL ){
			fitCurve->detach();
			delete fitCurve;
		}
		QVector<double> curveXValues( xVector.size() );
		QVector<double> curveYValues( xVector.size() );
		for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
			curveXValues[i] = xVector[i];
			curveYValues[i] = yValues[i];
		}
		fitCurve = addCurve( curveXValues, curveYValues );
		QPen fitCurvePen = fitCurve->pen();
		fitCurvePen.setColor( fitCurveColor );
		fitCurve->setPen( fitCurvePen );
		binPlot.replot();
	}
}

//--------------------------------------------------------------------------------
//                           Plot Data
//--------------------------------------------------------------------------------

QwtPlotCurve* BinPlotWidget::addCurve( QVector<double>& xValues, QVector<double>& yValues ){
	QwtPlotCurve* curve  = new QwtPlotCurve();
	curve->setData( xValues, yValues );
	QPen curvePen( curveColor );
	curve->setPen(curvePen);
	curve->attach(&binPlot);
	curves.append( curve );
	return curve;
}

double BinPlotWidget::checkLogValue( double value ) const {
	double resultValue = value;
	if ( displayLog ){
		if ( value != 0 ){
			resultValue = qLn( value ) / qLn( 10 );
		}
	}
	return resultValue;
}

void BinPlotWidget::defineCurveHistogram(){
	int pointCount = xVector.size();
	for ( int i = 0; i < pointCount; i++ ){
		//Draw vertical line
		QVector<double> xValues(2);
		QVector<double> yValues(2);
		if ( i > 0 ){
			xValues[0] = (xVector[i] + xVector[i-1])/2;
		}
		else {
			xValues[0] = xVector[0];
		}
		xValues[1] = xValues[0];
		if ( i > 0 ){
			yValues[0] = checkLogValue(yVector[i-1]);
		}
		else {
			yValues[0] = 0;
		}
		yValues[1] = checkLogValue(yVector[i]);
		addCurve( xValues, yValues );

		//Draw horizontal line connecting to previous point.
		if ( i < pointCount - 1){
			xValues[1] = ( xVector[i] + xVector[i+1] ) / 2;
		}
		else {
			xValues[1] = xVector[i];
		}
		yValues[0] = checkLogValue(yVector[i]);
		yValues[1] = yValues[0];
		addCurve( xValues, yValues );
	}
}

void BinPlotWidget::defineCurveLine(){
	for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
		QVector<double> xValues(2);
		QVector<double> yValues(2);
		xValues[0] = xVector[i];
		xValues[1] = xVector[i];
		yValues[0] = 0;
		yValues[1] = checkLogValue(yVector[i]);
		addCurve( xValues, yValues );
	}
}

void BinPlotWidget::defineCurve(){
	clearCurves();
	if ( displayStep ){
		defineCurveHistogram();
	}
	else {
		defineCurveLine();
	}
	binPlot.replot();
}

void BinPlotWidget::deleteImageRegion( int id ){
	if ( regionMap.contains(id) ){
		SubImage<Float>* subImage = regionMap[id];
		if ( subImage != NULL ){
			regionMap.remove(id);
			delete subImage;
			resetRegion( -1 );
		}
	}
}

bool BinPlotWidget::setImageRegion( const ImageRegion* region, int id ){
	bool success = false;
	if ( image != NULL && region != NULL ){
		SubImage<Float>* subImage = new SubImage<Float>( *image, *region );
		SubImage<Float>* oldImage = NULL;
		if ( regionMap.contains( id )){
			oldImage = regionMap[id];
		}
		regionMap.insert( id, subImage );
		delete oldImage;
		if ( plotMode != IMAGE_MODE ){
			success = resetRegion( id );
		}
	}
	else {
		qWarning() << "Please specify an image before specifying an image region";
	}
	return success;
}

bool BinPlotWidget::setImage( ImageInterface<Float>* img ){
	image = img;
	bool success = true;
	if ( image != NULL ){
		success = resetImage();
	}
	return success;
}

bool BinPlotWidget::resetRegion( int id ){
	QList<int> regionKeys = regionMap.keys();
	bool success = true;
	if ( regionKeys.size() > 0 ){
		int key = regionKeys[regionKeys.size() - 1];
		if ( regionKeys.contains(id) ){
			key = id;
		}
		if ( histogramMaker == NULL ){
			histogramMaker = new ImageHistograms<Float>(*regionMap[key]);
			rectangleSizeChanged();
		}
		else {
			histogramMaker->setNewImage(*regionMap[key]);
		}
		success = makeHistogram();
	}
	else {
		this->clearCurves();
	}
	return success;
}

bool BinPlotWidget::resetImage(){
	bool success = true;
	if ( image != NULL ){
		if ( plotMode == IMAGE_MODE ){
			if ( histogramMaker == NULL ){
				histogramMaker = new ImageHistograms<Float>( *image );
				rectangleSizeChanged();
			}
			else {
				histogramMaker->setNewImage( *image );
			}
			success = makeHistogram();
		}
		this->resetPlotTitle();
	}
	return success;
}

bool BinPlotWidget::makeHistogram(){
	Array<Float> values;
	Array<Float> counts;
	bool success = histogramMaker->getHistograms( values, counts );
	if ( success ){
		//Store the data
		values.tovector( xVector );
		counts.tovector( yVector );
		defineCurve();
		setValidatorLimits();
		if ( fitWidget != NULL ){
			fitWidget->setValues( xVector, yVector );
		}
		if ( toolTipPicker != NULL ){
			this->toolTipPicker->setData( xVector, yVector );
		}
		if ( rangeControlWidget != NULL ){
			rangeControlWidget->setDataLimits( xVector );
		}
		update();
	}
	else {
		QString msg( "Could not make a histogram from the image.");
		QMessageBox::warning( this, "Error Making Histogram", msg);
	}
	return success;
}

void BinPlotWidget::setValidatorLimits(){
	if ( rangeControlWidget != NULL ){
		int count = xVector.size();
		if ( count >= 1 ){
			double minValue = xVector[0];
			double maxValue = xVector[0];
			for ( int i = 1; i < count; i++ ){
				if ( xVector[i] < minValue ){
					minValue = xVector[i];
				}
				else if ( xVector[i] > maxValue ){
					maxValue = xVector[i];
				}
			}
			rangeControlWidget->setRangeLimits( minValue, maxValue );
		}
	}
}

void BinPlotWidget::clearCurves(){
	while( ! curves.isEmpty() ){
		QwtPlotCurve* curve = curves.takeLast();
		curve->detach();
		delete curve;
	}
}


//-----------------------------------------------------------------------
//                   Rectangle Range Specification
//-----------------------------------------------------------------------

void BinPlotWidget::resizeEvent( QResizeEvent* event ){
	QWidget::resizeEvent( event );
	rectangleSizeChanged();
	resetGaussianFitMarker();
	resetPoissonFitMarker();
}

void BinPlotWidget::keyPressEvent( QKeyEvent* event ){
	Qt::KeyboardModifiers modifiers = event->modifiers();
	if (modifiers & Qt::ShiftModifier ){
		contextMenuMode = DISPLAY_CONTEXT;
	}
	else if ( modifiers & Qt::ControlModifier ){
		contextMenuMode = FIT_CONTEXT;
	}
	else {
		contextMenuMode = ZOOM_CONTEXT;
	}
	QWidget::keyPressEvent( event );
}

void BinPlotWidget::rectangleSizeChanged(){
	if ( rectMarker != NULL ){
		resetRectangleMarker();
		binPlot.replot();
	}
}

void BinPlotWidget::lineSelected(){

	int lowerBound = rectMarker->getLowerBound();
	int upperBound = rectMarker->getUpperBound();
	double lowerBoundWorld = binPlot.invTransform( QwtPlot::xBottom, lowerBound );
	double upperBoundWorld = binPlot.invTransform( QwtPlot::xBottom, upperBound );
	setMinMaxValues( lowerBoundWorld, upperBoundWorld, false );
}

void BinPlotWidget::lineMoved( const QPoint& pt ){
	rectMarker->boundaryLineMoved( pt );
	rectMarker->show();
	binPlot.replot();
}

void BinPlotWidget::resetRectangleMarker(){
	if ( rectMarker != NULL ){
		int pixelXStart = static_cast<int>( binPlot.transform( QwtPlot::xBottom, rectX ) );
		int pixelXEnd = static_cast<int>(binPlot.transform( QwtPlot::xBottom, rectX+rectWidth ));
		rectMarker->setBoundaryValues( pixelXStart, pixelXEnd );
	}
}


void BinPlotWidget::minMaxChanged(){
	if ( rectMarker != NULL ){
		pair<double,double> minMaxValues = getMinMaxValues();
		rectX = minMaxValues.first;
		rectWidth = qAbs( minMaxValues.second - minMaxValues.first );
		resetRectangleMarker();
		rectMarker->show();
		binPlot.replot();
	}
	else {
		qWarning() << "Range tools need to be enabled for minMaxChanged";
	}
	emit rangeChanged();
}

void BinPlotWidget::clearRange(){
	if ( rectMarker != NULL && rectMarker->isVisible() ){
		rectMarker->reset();
		rectangleSizeChanged();
		rectMarker->hide();
		binPlot.replot();
	}
}

int BinPlotWidget::getCanvasHeight() {
	QwtPlotCanvas* canvas = binPlot.canvas();
	QSize canvasSize = canvas->size();
	int height = canvasSize.height();
	return height;
}

//--------------------------------------------------------------------------
//                      Saving the File
//--------------------------------------------------------------------------

void BinPlotWidget::toAscii( const QString& filePath ){
	QFile file( filePath );
	bool success = file.open( QIODevice::WriteOnly | QIODevice::Text );
	const QString LINE_END( "\n");
	if ( success ){
		bool addFitValues = false;
		Vector<float> fitValues;
		if ( fitWidget != NULL ){
			fitValues = fitWidget->getFitValues();
			if ( fitValues.size() > 0 ){
				addFitValues = true;
			}
		}
		QTextStream out( &file );
		out.setFieldWidth( 20 );
		out << "Intensity" << "Count";
		if ( addFitValues ){
			out << "Fit Count";
		}
		out << LINE_END;
		out.flush();
		for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
			out << QString::number(xVector[i]) << QString::number( yVector[i]);
			if ( addFitValues ){
				out << QString::number(fitValues[i]);
			}
			out << LINE_END;
			out.flush();
		}

		if ( addFitValues ){
			out << LINE_END << LINE_END;
			fitWidget->toAscii( out );
			out.flush();
		}
		file.close();
	}
	else {
		QMessageBox::warning( this, "Save Problem", "There was a problem saving the image.\nPlease check the file path.");
	}
}


void BinPlotWidget::toPing( const QString& filePath, int width, int height ){

	QPixmap pixmap(width, height);
	pixmap.fill(Qt::white );
	QwtPlotPrintFilter filter;
	int options = QwtPlotPrintFilter::PrintFrameWithScales | QwtPlotPrintFilter::PrintBackground;
	filter.setOptions( options );
	binPlot.print( pixmap, filter );
	bool imageSaved = pixmap.save( filePath, "png");
	if ( !imageSaved ){
		QString msg("There was a problem saving the histogram.\nPlease check the file path.");
		QMessageBox::warning( this, "Save Problem", msg);
	}
}

void BinPlotWidget::imageRegionSelected( int id ){
	if ( plotMode == REGION_MODE ){
		resetRegion( id );
	}
}

void BinPlotWidget::clearRegionList(){
	QList<int> regionKeys = regionMap.keys();
	for( int i = 0; i < regionKeys.size(); i++ ){
		int key = regionKeys[i];
		SubImage<Float>* subImage = regionMap[key];
		delete subImage;
	}
	regionMap.clear();
}

BinPlotWidget::~BinPlotWidget(){
	clearCurves();
	clearRegionList();
	delete histogramMaker;
	delete fitEstimateMarkerGaussian;
}

}

