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
#include <images/Images/ImageInterface.h>
#include <guitools/Histogram/Histogram.h>
#include <guitools/Histogram/FitWidget.qo.h>
#include <guitools/Histogram/HistogramMarkerGaussian.h>
#include <guitools/Histogram/HistogramMarkerPoisson.h>
#include <guitools/Histogram/InvisibleAxis.h>
#include <guitools/Histogram/RangeControlsWidget.qo.h>
#include <guitools/Histogram/RangePicker.h>
#include <guitools/Histogram/ToolTipPicker.h>
#include <guitools/Histogram/BinCountWidget.qo.h>
#include <guitools/Histogram/ChannelRangeWidget.qo.h>
#include <guitools/Histogram/ZoomWidget.qo.h>
#include <casa/Arrays/Vector.h>
#include <synthesis/MSVis/UtilJ.h>

#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QMouseEvent>
#include <QWidgetAction>
#include <QSpinBox>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_text_label.h>
#include <qwt_scale_div.h>
#include <qwt_color_map.h>
#include <qwt_double_interval.h>
#include <qwt_scale_widget.h>

namespace casa {


BinPlotWidget::BinPlotWidget( bool fitControls, bool rangeControls,
		bool plotModeControls, bool controlBinCountOnly, QWidget* parent ):
    QWidget(parent),
    curveColor( Qt::blue ), selectionColor( 205, 201, 201, 127 ),
    colorMap( NULL ), colorCurve(NULL),image( NULL ), binPlot( this ),
    NO_DATA( "No Data"), NO_DATA_MESSAGE( "Data is needed in order to zoom."), IMAGE_ID(-1),
	dragLine( NULL ), rectMarker( NULL ),
    toolTipPicker(NULL), contextMenuZoom(this),
    zoomActionContext(NULL), zoomWidgetContext( NULL ),
    zoomActionMenu(NULL), zoomWidgetMenu( NULL ),
    lambdaAction("Lambda",this), centerPeakAction( "(Center,Peak)",this),
    fwhmAction( "Center +/- FWHM/2", this), contextMenu(this),
    LOG_COUNT( "Log(Count)"), LOG_INTENSITY( "Log(|Intensity|)"),
    stepFunctionNoneAction("Line",this), stepFunctionAction( "Histogram Outline",this), stepFunctionFilledAction( "Filled Histogram",this),
    logActionY( LOG_COUNT, this), logActionX( LOG_INTENSITY,this), clearAction( "Clear", this ),
    contextMenuDisplay( this ),
    regionModeAction( "Selected Region", this), imageModeAction("Image", this),
    regionAllModeAction( "All Regions", this ),
    binCountActionContext(NULL), channelRangeActionContext(NULL),
    binCountActionMenu(NULL), channelRangeActionMenu(NULL),
    binCountWidgetContext(NULL), channelRangeWidgetContext(NULL),
    binCountWidgetMenu(NULL), channelRangeWidgetMenu(NULL),
    contextMenuConfigure( this ){

	ui.setupUi(this);

	//Add the plot to the widget
	QHBoxLayout* layout = new QHBoxLayout(ui.plotHolder);
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( &binPlot );
	binPlot.setCanvasBackground( Qt::white );
	setAxisLabelFont( 8);

	ui.plotHolder->setLayout( layout );
	ui.plotHolder->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

	initializeDisplayActions();
	initializeFitWidget( fitControls );
	initializeRangeControls( rangeControls );
	initializeZoomControls( true );
	initializePlotModeControls( plotModeControls, controlBinCountOnly );

	displayPlotTitle = false;
	displayAxisTitles = false;
	colorBarVisible = false;
	selectedId = IMAGE_ID;
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

void BinPlotWidget::setColorBarVisible( bool visible ){
	if ( visible != colorBarVisible ){
		colorBarVisible = visible;
		if ( colorBarVisible ){
			addColorBar();
		}
		else {
			removeColorBar();
		}
		binPlot.replot();
	}
}

void BinPlotWidget::setColorScaleMax( int scaleCount ){
	colorScaleMax = scaleCount;
}

void BinPlotWidget::setColorLookups( const Vector<uInt>& lookups ){
	int count = lookups.size();
	colorLookups.resize( count );
	for ( int i = 0; i < count; i++ ){
		colorLookups[i] = lookups[i];
	}
	clearCurves();
	resetImage();
}

void BinPlotWidget::resetColorCurve(){
	//Add the color curve in.
	int count = colorLookups.size();
	if ( count > 0 ){
		QVector<double> xVals( count );
		QVector<double> yVals( count );
		for( int i = 0; i < count; i++ ){
			xVals[i] = colorLookups[i];
			bool logScaleY = logActionY.isChecked();
			yVals[i]= Histogram::computeYValue( i, logScaleY );
		}
		if ( colorCurve != NULL ){
			colorCurve->detach();
			delete colorCurve;
			colorCurve = NULL;
		}
		colorCurve  = new QwtPlotCurve();
		colorCurve->setAxis( QwtPlot::xTop, QwtPlot::yRight );
		colorCurve->setData( xVals, yVals );
		QPen curvePen( Qt::black );
		curvePen.setWidth( 2 );
		colorCurve->setPen(curvePen);
		colorCurve->attach(&binPlot);
		binPlot.replot();
	}
}

void BinPlotWidget::setColorMap(QwtLinearColorMap* linearMap ){
	colorMap = linearMap;
	resetColorBar();
}

void BinPlotWidget::resetColorBar(){
	if ( colorMap != NULL ){
		QwtDoubleInterval range( 0, colorScaleMax);
		QwtScaleWidget* scale = binPlot.axisWidget( QwtPlot::xTop );
		scale->setColorBarEnabled( true );
		scale->setColorMap( range, *colorMap );
		QwtScaleDraw* axisPaint = new InvisibleAxis();
		scale->setScaleDraw( axisPaint );
		binPlot.setAxisScale( QwtPlot::xTop, range.minValue(), range.maxValue() );
	}
}

void BinPlotWidget::removeColorBar(){
	binPlot.enableAxis( QwtPlot::xTop, false );
}

void BinPlotWidget::addColorBar(){
	resetColorBar();
	binPlot.enableAxis(QwtPlot::xTop );
}

void BinPlotWidget::resetAxisTitles(){
	if ( displayAxisTitles ){
		QString yAxisTitle = "Count";
		if ( displayLogY ){
			yAxisTitle = LOG_COUNT;
		}
		QwtText leftTitle = binPlot.axisTitle(QwtPlot::yLeft);
		QFont titleFont = leftTitle.font();
		titleFont.setPointSize(8);
		leftTitle.setFont( titleFont );
		leftTitle.setText( yAxisTitle );
		binPlot.setAxisTitle( QwtPlot::yLeft, leftTitle );
		QString xAxisTitle = "Intensity";
		if ( displayLogX ){
			xAxisTitle = LOG_INTENSITY;
		}
		leftTitle.setText( xAxisTitle );
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

void BinPlotWidget::setMultipleHistogramColors( const QList<QColor>& colors ){
	multipleHistogramColors.clear();
	for (int i = 0; i < colors.size(); i++ ){
		multipleHistogramColors.append( colors[i]);
	}
	if ( plotMode == REGION_ALL_MODE ){
		clearCurves();
		this->resetRegion( );
	}
}

bool BinPlotWidget::isEmpty() const {
	bool empty = false;
	if ( histogramMap.size() == 0 ){
		empty = true;
	}
	return empty;
}

//-------------------------------------------------------------------------------
//                            Display Settings
//-------------------------------------------------------------------------------

void BinPlotWidget::initializePlotModeControls( bool enable, bool binCountOnly ){

	if ( enable ){
		binCountWidgetContext = new BinCountWidget( &contextMenuConfigure );
		binCountActionContext = new QWidgetAction( &contextMenuConfigure );
		binCountActionContext->setDefaultWidget( binCountWidgetContext );
		connect( binCountWidgetContext, SIGNAL(binCountChanged(int)), this, SLOT(binCountChanged(int)));

		if ( !binCountOnly ){
			QActionGroup* plotControlsGroup = new QActionGroup( this );
			plotControlsGroup->addAction( &regionModeAction );
			plotControlsGroup->addAction( &imageModeAction );
			plotControlsGroup->addAction( &regionAllModeAction );

			regionModeAction.setCheckable( true );
			imageModeAction.setCheckable( true );
			regionAllModeAction.setCheckable( true );
			regionModeAction.setChecked( true );
			connect( &imageModeAction, SIGNAL(triggered(bool)), this, SLOT(imageModeSelected(bool)));
					connect( &regionModeAction, SIGNAL(triggered(bool)), this, SLOT(regionModeSelected(bool)));
					connect( &regionAllModeAction, SIGNAL(triggered(bool)), this, SLOT(regionAllModeSelected(bool)));

			channelRangeWidgetContext = new ChannelRangeWidget( &contextMenuConfigure );
			channelRangeActionContext = new QWidgetAction( &contextMenuConfigure );
			channelRangeActionContext->setDefaultWidget( channelRangeWidgetContext );
			connect( channelRangeWidgetContext, SIGNAL(rangeChanged(int,int,bool,bool)), this, SLOT(channelRangeChanged(int,int,bool,bool)));
		}

		addPlotModeActions( &contextMenuConfigure, binCountActionContext, channelRangeActionContext, binCountOnly );
	}
	plotMode = REGION_MODE;
}

void BinPlotWidget::imageModeSelected( bool enabled ){
	if ( enabled ){
		if ( plotMode != IMAGE_MODE ){
			plotMode = IMAGE_MODE;
			if ( fitWidget != NULL ){
				fitWidget->clearFit();
			}
			resetImage();
		}
	}
}

void BinPlotWidget::regionModeSelected( bool enabled ){
	if ( enabled ){
		if ( plotMode != REGION_MODE ){
			plotMode = REGION_MODE;
			if ( fitWidget != NULL ){
				fitWidget->clearFit();
			}
			resetRegion(  );
		}
	}
}

void BinPlotWidget::regionAllModeSelected( bool enabled ){
	if ( enabled ){
		if ( plotMode != REGION_ALL_MODE ){
			plotMode = REGION_ALL_MODE;
			if ( fitWidget != NULL ){
				fitWidget->clearFit();
			}
			resetRegion( );
		}
	}
}



void BinPlotWidget::channelRangeChanged( int minValue, int maxValue, bool allChannels, bool automatic ){

	//Coordinate the channel range from the two different menus
	if ( channelRangeWidgetContext != NULL ){
		channelRangeWidgetContext->setAutomatic( automatic );
		channelRangeWidgetContext->setRange( minValue, maxValue );
	}
	if ( channelRangeWidgetMenu != NULL ){
		channelRangeWidgetMenu->setAutomatic( automatic );
		channelRangeWidgetMenu->setRange( minValue, maxValue );
	}

	if ( allChannels ){
		Histogram::setChannelRangeDefault();
	}
	else {
		Histogram::setChannelRange( minValue, maxValue );
	}
	int selectedId = getSelectedId();
	if ( histogramMap.contains( selectedId )){
		histogramMap[selectedId]->reset();
		reset();
	}
}

void BinPlotWidget::binCountChanged( int count ){
	//Coordinate the bin counts from the two different menus.
	if ( binCountWidgetContext != NULL ){
		binCountWidgetContext->setBinCount( count );
	}
	if ( binCountWidgetMenu != NULL ){
		binCountWidgetMenu->setBinCount( count );
	}
	Histogram::setBinCount( count );
	int selectedId = getSelectedId();
	if ( histogramMap.contains(selectedId)){
		histogramMap[selectedId]->compute();
		reset();
	}
}

void BinPlotWidget::setChannelValue( int value ){
	if ( channelRangeWidgetContext != NULL ){
		channelRangeWidgetContext->setChannelValue( value );
	}
	if ( channelRangeWidgetMenu != NULL ){
		channelRangeWidgetMenu->setChannelValue( value );
	}
}

void BinPlotWidget::setChannelCount( int count ){
	if ( channelRangeWidgetContext != NULL ){
		channelRangeWidgetContext->setChannelCount( count );
	}
	if ( channelRangeWidgetMenu != NULL ){
		channelRangeWidgetMenu->setChannelCount( count );
	}
}

void BinPlotWidget::addPlotModeActions( QMenu* menu, QWidgetAction* binCountAction, QWidgetAction* channelRangeAction, bool binCountOnly ){

	if ( binCountAction == NULL ){
		binCountActionMenu = new QWidgetAction( menu );
		binCountWidgetMenu = new BinCountWidget( menu );
		binCountActionMenu->setDefaultWidget( binCountWidgetMenu );
		connect( binCountWidgetMenu, SIGNAL(binCountChanged(int)), this, SLOT(binCountChanged(int)));
		binCountAction = binCountActionMenu;
	}

	if ( channelRangeAction == NULL && !binCountOnly ){
		channelRangeActionMenu = new QWidgetAction( menu );
		channelRangeWidgetMenu = new ChannelRangeWidget( menu );
		connect( channelRangeWidgetMenu, SIGNAL(rangeChanged(int,int,bool,bool)), this, SLOT(channelRangeChanged(int,int,bool,bool)));
		channelRangeActionMenu->setDefaultWidget( channelRangeWidgetMenu );
		channelRangeAction = channelRangeActionMenu;
	}
	menu->addAction( binCountAction );
	if ( !binCountOnly ){
		menu->addSeparator();
		menu->addAction( channelRangeAction );
		menu->addSeparator();
		menu->addAction( &imageModeAction );
		menu->addAction( &regionModeAction );
		menu->addAction( &regionAllModeAction );
	}
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

void BinPlotWidget::initializeDisplayActions(){

	displayLogY = false;
	displayLogX = false;
	logActionY.setCheckable( true );
	logActionY.setChecked( displayLogY );
	logActionX.setCheckable( true );
	logActionX.setChecked( displayLogX );

	stepFunctionAction.setCheckable( true );
	stepFunctionAction.setChecked( true );
	stepFunctionNoneAction.setCheckable( true );
	stepFunctionNoneAction.setChecked( false );
	stepFunctionFilledAction.setCheckable( true );
	stepFunctionFilledAction.setChecked( false );
	QActionGroup* histogramGroup = new QActionGroup( this );
	histogramGroup->addAction( &stepFunctionNoneAction );
	histogramGroup->addAction( &stepFunctionAction );
	histogramGroup->addAction( &stepFunctionFilledAction );
	addDisplayActions( &contextMenuDisplay );
	connect( &clearAction, SIGNAL(triggered(bool)), this, SLOT(clearAll()));
	connect( &logActionY, SIGNAL(triggered(bool)), this, SLOT(setDisplayLogY(bool)));
	connect( &logActionX, SIGNAL(triggered(bool)), this, SLOT(setDisplayLogX(bool)));
	connect( &stepFunctionAction, SIGNAL(triggered(bool)), this, SLOT(setDisplayStep(bool)));
	connect( &stepFunctionNoneAction, SIGNAL(triggered(bool)), this, SLOT(setDisplayStep(bool)));
	connect( &stepFunctionFilledAction, SIGNAL(triggered(bool)), this, SLOT(setDisplayStep(bool)));
	if ( toolTipPicker != NULL ){
		toolTipPicker->setLogScaleY( displayLogY );
		toolTipPicker->setLogScaleX( displayLogX );
	}
}


void BinPlotWidget::addDisplayActions( QMenu* menu ){
	menu->addAction( &clearAction );
	menu->addSeparator();
	menu->addAction( &logActionY );
	//menu->addAction( &logActionX );
	menu->addSeparator();
	menu->addAction( &stepFunctionNoneAction );
	menu->addAction( &stepFunctionAction );
	menu->addAction( &stepFunctionFilledAction );
}

void BinPlotWidget::reset(){
	if ( plotMode == IMAGE_MODE ){
		resetImage();
	}
	else {
		resetRegion();
	}
}

void BinPlotWidget::setDisplayStep( bool ){
	reset();
}

void BinPlotWidget::setDisplayLogX( bool display ){
	if ( displayLogX != display ){
		displayLogX = display;
	}
	logActionX.setChecked( display );
	if ( fitWidget != NULL ){
		fitWidget->clearFit();
	}
	reset();
	resetAxisTitles();
	if ( toolTipPicker != NULL ){
		toolTipPicker->setLogScaleX( display );
	}

}

void BinPlotWidget::setDisplayLogY( bool display ){
	if ( displayLogY != display ){
		displayLogY = display;
		logActionY.setChecked( displayLogY );
		if ( fitWidget != NULL ){
			fitWidget->clearFit();
		}
		clearFit();
		reset();
		resetAxisTitles();
		if ( toolTipPicker != NULL ){
			toolTipPicker->setLogScaleY( display );
		}
	}
}


//-------------------------------------------------------------------------------
//                          Marking Ranges on the Histogram
//-------------------------------------------------------------------------------

void BinPlotWidget::initializeRangeControls( bool enable ){
	//Put the range controls in
	if ( enable ){
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
}

void BinPlotWidget::zoomContextFinished(){
	contextMenuZoom.close();
	if ( zoomWidgetMenu != NULL ){
		zoomWidgetMenu->copyState( zoomWidgetContext );
	}
}

void BinPlotWidget::zoomMenuFinished(){
	if ( zoomWidgetContext != NULL ){
		zoomWidgetContext->copyState( zoomWidgetMenu );
	}
}

void BinPlotWidget::zoomPercentage( float minValue, float maxValue ){
	if ( !isEmpty() ){
		Histogram::setIntensityRange( minValue, maxValue );
		int selectedId = getSelectedId();
		if ( histogramMap.contains( selectedId )){
			histogramMap[selectedId]->compute();
			reset();
			zoomRangeMarker( minValue, maxValue );
		}
	}
	else {
		QMessageBox::warning( this, NO_DATA, NO_DATA_MESSAGE );
	}
}


void BinPlotWidget::connectZoomActions( ZoomWidget* zoomWidget ){

	connect( zoomWidget, SIGNAL(zoomRange(float,float)), this, SLOT(zoomPercentage(float,float)));
	connect( zoomWidget, SIGNAL( zoomNeutral()), this, SLOT(zoomNeutral()));
	connect( zoomWidget, SIGNAL( zoomGraphicalRange()), this, SLOT(zoomRange()));
}

void BinPlotWidget::initializeZoomControls( bool rangeControls ){

	zoomWidgetContext = new ZoomWidget( rangeControls, &contextMenuZoom );
	zoomActionContext = new QWidgetAction( &contextMenuZoom );
	zoomActionContext->setDefaultWidget( zoomWidgetContext );
	contextMenuZoom.addAction( zoomActionContext );
	connectZoomActions( zoomWidgetContext );
	connect( zoomWidgetContext, SIGNAL(finished()), this, SLOT(zoomContextFinished()));
	contextMenuMode = ZOOM_CONTEXT;
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
		binPlot.replot();
	}
}

int BinPlotWidget::getSelectedId() const {
	int id = selectedId;
	//There is not region selected, just grab the first one we
	//can find.
	if ( selectedId == IMAGE_ID && plotMode != IMAGE_MODE ){
		QList<int> keys = histogramMap.keys();
		if ( keys.size() > 0 ){
			id = keys[0];
		}
	}
	return id;
}

void BinPlotWidget::zoomRange( ){
	if ( !isEmpty() ){
		pair<double,double> bounds = this->getMinMaxValues();
		if ( bounds.first < bounds.second ){
			Histogram::setIntensityRange( bounds.first, bounds.second );
			int id = getSelectedId();
			if ( histogramMap.contains(id)){
				histogramMap[id]->compute();
				reset();
				zoomRangeMarker( bounds.first, bounds.second );
			}
		}
		else {
			QMessageBox::warning( this, "Invalid Range","Please drag the left mouse button on the histogram to specify a graphical range.");
		}
	}
	else {
		QMessageBox::warning( this, NO_DATA, NO_DATA_MESSAGE );
	}
}

void BinPlotWidget::zoomNeutral(){
	if ( !isEmpty() ){
		Histogram::setIntensityRangeDefault();
		int id = getSelectedId();
		if ( histogramMap.contains( id )){
			histogramMap[id]->compute();
			reset();
			pair<float,float> imageBounds = histogramMap[id]->getDataRange();
			zoomRangeMarker( imageBounds.first, imageBounds.second );
		}
	}
}


void BinPlotWidget::addZoomActions( bool rangeControls, QMenu* zoomMenu ){
	zoomWidgetMenu = new ZoomWidget( rangeControls, zoomMenu );
	zoomActionMenu = new QWidgetAction( zoomMenu );
	zoomActionMenu->setDefaultWidget( zoomWidgetMenu );
	zoomMenu->addAction( zoomActionMenu );
	connectZoomActions( zoomWidgetMenu );
	connect( zoomWidgetMenu, SIGNAL(finished()), this, SLOT(zoomMenuFinished()));
}

void BinPlotWidget::setMinMaxValues( double minValue, double maxValue,
		bool updateGraph ){
	if ( rangeControlWidget != NULL ){
		rangeControlWidget->setRange( minValue, maxValue, updateGraph );

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
		ui.fitWidgetHolder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding );
		ui.fitWidgetHolder->setMaximumWidth( 400 );
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
	if ( displayLogY ){
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
	Vector<Float> xVector = fitWidget->getFitValuesX();
	if ( yValues.size() > 0 ){
		if ( fitCurve != NULL ){
			fitCurve->detach();
			delete fitCurve;
			fitCurve = NULL;
		}
		QVector<double> curveXValues( xVector.size() );
		QVector<double> curveYValues( xVector.size() );
		for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
			curveXValues[i] = Histogram::computeXValue( xVector[i], displayLogX);
			if ( !displayLogY ){
				curveYValues[i] = yValues[i];
			}
			else {
				//Round to the nearest integer.
				int yValueInt = qRound( yValues[i] );
				if ( yValueInt > 0  ){
					curveYValues[i] = qLn( yValueInt) / qLn(10);
				}
				else {
					curveYValues[i] = 0;
				}
			}
		}
		fitCurve = addCurve( curveXValues, curveYValues, fitCurveColor );
		binPlot.replot();
	}
}

//--------------------------------------------------------------------------------
//                           Plot Data
//--------------------------------------------------------------------------------

QwtPlotCurve* BinPlotWidget::addCurve( QVector<double>& xValues,
		QVector<double>& yValues, const QColor& histColor ){
	QwtPlotCurve* curve  = new QwtPlotCurve();
	curve->setData( xValues, yValues );
	QPen curvePen( histColor );
	if ( histColor == curveColor ){
		curvePen.setWidth( 2 );
	}
	else {
		curvePen.setWidth( 1 );
	}
	curve->setPen(curvePen);
	curve->attach(&binPlot);

	if ( histColor != fitCurveColor ){
		if ( stepFunctionFilledAction.isChecked() ){
			curve->setBaseline( 0 );
			QBrush brush;
			brush.setColor( histColor );
			brush.setStyle( Qt::SolidPattern );
			curve->setBrush( brush );
		}
		curves.append( curve );
	}
	return curve;
}

QColor BinPlotWidget::getPieceColor( int index, const QColor& defaultColor ) const {
	QColor pieceColor = defaultColor;
	int colorLookUpCount = colorLookups.size();
	if ( colorBarVisible && colorLookUpCount > 0 ){
		QwtDoubleInterval range(0,colorScaleMax);
		pieceColor = colorMap->rgb( range, colorLookups[index]);
		//So the first segment color is assigned.
		if ( pieceColor == defaultColor && colorLookUpCount > 1 ){
			pieceColor =colorMap->rgb( range,colorLookups[1] );
		}
	}
	return pieceColor;
}

void BinPlotWidget::defineCurveHistogram( int id, const QColor& histogramColor ){
	if ( histogramMap.contains(id)){
		int pointCount = histogramMap[id]->getDataCount();
		QVector<double> xValues(2);
		QVector<double> yValues(2);
		QColor pieceColor;
		for ( int i = 0; i < pointCount; i++ ){
			pieceColor = getPieceColor( i, histogramColor );

			//Draw vertical line
			histogramMap[id]->defineStepVertical( i, xValues, yValues, displayLogX, displayLogY );
			addCurve( xValues, yValues, pieceColor );

			//Draw horizontal line connecting to previous point.
			histogramMap[id]->defineStepHorizontal(i, xValues, yValues, displayLogX, displayLogY);
			addCurve( xValues, yValues, pieceColor );
		}
		//Add the last vertical line
		xValues[0] = xValues[1];
		yValues[0] = 0;
		addCurve( xValues, yValues, pieceColor);
	}
}

void BinPlotWidget::defineCurveLine( int id, const QColor& lineColor ){
	if ( histogramMap.contains (id ) ){
		int count = histogramMap[id]->getDataCount();
		for ( int i = 0; i < count; i++ ){
			QVector<double> xValues(2);
			QVector<double> yValues(2);
			histogramMap[id]->defineLine( i, xValues, yValues, displayLogX, displayLogY );
			QColor pieceColor = getPieceColor( i, lineColor );
			addCurve( xValues, yValues, pieceColor );
		}
	}
}

void BinPlotWidget::defineCurve( int id, const QColor& curveColor, bool clearCurve ){
	if ( clearCurve ){
		clearCurves();
	}

	if ( stepFunctionAction.isChecked() || stepFunctionFilledAction.isChecked() ){
		defineCurveHistogram( id, curveColor );
	}
	else {
		defineCurveLine( id, curveColor );
	}
	binPlot.replot();
}

void BinPlotWidget::deleteImageRegion( int id ){
	bool histogramFound = false;
	if ( histogramMap.contains(id) ){
		Histogram* histogram = histogramMap[id];
		histogramMap.remove(id);
		if ( histogram != NULL ){
			delete histogram;
			histogramFound = true;
		}
	}
	if ( id == selectedId ){
		//Arbitrarily select another one
		QList<int> keyList = histogramMap.keys();
		if ( keyList.size() > 0 ){
			selectedId = keyList[0];
			if ( histogramFound ){
				reset();
			}
		}
		else {
			selectedId = -1;
		}
	}

}

void BinPlotWidget::postMessage( const QString& msg ){
	emit postStatusMessage( msg );
}

Histogram* BinPlotWidget::findHistogramFor( int id ){
	Histogram* histogram = NULL;
	if ( histogramMap.contains(id) ){
		histogram = histogramMap[id];
	}
	else {
		histogram = new Histogram( this );
		histogramMap.insert(id, histogram );
	}
	return histogram;
}

bool BinPlotWidget::setImageRegion( ImageRegion* region, int id ){
	bool success = false;
	if ( image != NULL && region != NULL ){
		Histogram* histogram = findHistogramFor( id );
		histogram->setRegion( region );
		success = histogram->reset();
		if ( plotMode != IMAGE_MODE && success ){
			selectedId = id;
			resetRegion();
		}
	}
	else {
		qWarning() << "Please specify an image before specifying an image region";
	}
	return success;
}

std::vector<float> BinPlotWidget::getXValues() const {
	std::vector<float> values;
	if ( histogramMap.contains(IMAGE_ID)){
		values = histogramMap[IMAGE_ID]->getXValues();
	}
	return values;
}

bool BinPlotWidget::setImage( ImageInterface<Float>* img ){
	bool success = true;
	if ( image != img && img != NULL ){
		image = img;
		Histogram::setImage( image );
		clearHistograms();
		clearAll();
		success = resetImage();
		if ( success ){
			if ( zoomWidgetContext != NULL ){
				zoomWidgetContext->setImage( image );
			}
			if ( zoomWidgetMenu != NULL ){
				zoomWidgetMenu->setImage( image );
			}
			if ( fitWidget != NULL ){
				fitWidget->clearFit();
			}
		}
	}
	else {
		success = false;
	}
	return success;
}

void BinPlotWidget::resetRegion( ){
	//Make the histogram(s)
	if ( plotMode == REGION_MODE ){
		int id = getSelectedId();
		makeHistogram( id, curveColor );
	}
	else if ( plotMode == REGION_ALL_MODE ){
		clearCurves();
		QList<int> keys = histogramMap.keys();
		int keyCount = keys.size();
		for ( int i = 0; i < keyCount; i++ ){
			if ( keys[i] != IMAGE_ID ){
				int colorIndex = i % multipleHistogramColors.size();
				QColor histogramColor = multipleHistogramColors[colorIndex];
				if ( keys[i] == selectedId ){
					histogramColor = curveColor;
				}
				makeHistogram( keys[i], histogramColor, false );
			}
		}
	}
	else {
		qDebug() << "Unrecognized region mode: "<< plotMode;
	}
}

bool BinPlotWidget::resetImage(){
	bool success = true;
	if ( image != NULL ){
		if ( plotMode == IMAGE_MODE ){
			Histogram* histogram = findHistogramFor( IMAGE_ID );
			success = histogram->reset( );
			if ( success ){
				makeHistogram( IMAGE_ID, curveColor );
				resetColorCurve();
			}
			else {
				QString msg( "Could not make a histogram from the image.");
				QMessageBox::warning( this, "Error Making Histogram", msg);
			}
		}
		this->resetPlotTitle();
	}
	return success;
}

bool BinPlotWidget::isPrincipalHistogram( int id ) const {
	bool principalHistogram = false;
	if ( plotMode == IMAGE_MODE ){
		if ( id == IMAGE_ID ){
			principalHistogram = true;
		}
	}
	//We are histogramming regions
	else if ( selectedId == id ){
		//We have a selected region, and this is it.
		principalHistogram = true;
	}
	else if ( selectedId == IMAGE_ID ){
		//There is not a selected region
		principalHistogram = true;
	}
	return principalHistogram;
}

void BinPlotWidget::makeHistogram( int id, const QColor& curveColor,
		bool clearCurves ){
	defineCurve( id, curveColor, clearCurves );
	if ( isPrincipalHistogram( id ) ){
		setValidatorLimits();
		vector<float> xVector = histogramMap[id]->getXValues();
		vector<float> yVector = histogramMap[id]->getYValues();
		if ( fitWidget != NULL ){
			fitWidget->setValues( id, xVector, yVector );
		}
		if ( toolTipPicker != NULL ){
			this->toolTipPicker->setData( xVector, yVector );
		}
		if ( rangeControlWidget != NULL ){
			pair<float,float> limits = histogramMap[id]->getDataRange();
			rangeControlWidget->setDataLimits( limits.first, limits.second );
		}
	}
	update();
}

void BinPlotWidget::setValidatorLimits(){
	if ( rangeControlWidget != NULL ){
		if ( histogramMap.contains(selectedId)){
			pair<float,float> rangeLimits = histogramMap[selectedId]->getDataRange();
			rangeControlWidget->setDataLimits( rangeLimits.first, rangeLimits.second);
		}
	}
}

void BinPlotWidget::clearCurves(){
	while( ! curves.isEmpty() ){
		QwtPlotCurve* curve = curves.takeLast();
		if ( curve != NULL ){
			curve->detach();
			delete curve;
		}
	}
	if ( colorCurve != NULL ){
		colorCurve->detach();
	}
}


//-----------------------------------------------------------------------
//                   Rectangle Range Specification
//-----------------------------------------------------------------------

void BinPlotWidget::enterEvent( QEvent* /*event*/ ){
	this->setFocus(Qt::OtherFocusReason );
}

void BinPlotWidget::resizeEvent( QResizeEvent* event ){
	QWidget::resizeEvent( event );
	rectangleSizeChanged();
	resetGaussianFitMarker();
	resetPoissonFitMarker();
	if ( rangeControlWidget != NULL ){
		int plotWidth = ui.plotHolder->width();
		rangeControlWidget->setMinimumWidth( plotWidth );
		rangeControlWidget->setMaximumWidth( plotWidth );
	}
}

void BinPlotWidget::mousePressEvent( QMouseEvent* event ){
	Qt::MouseButton mouseButton = event->button();
	if ( mouseButton == Qt::LeftButton ){
		if ( contextMenuMode == FIT_CONTEXT ){
			QPoint relativePos = event->pos();
			bool ptInPlot = isPlotContains( relativePos.x(), relativePos.y() );
			if ( ptInPlot ){
				QPoint globalPos = event->globalPos();
				contextMenuConfigure.exec( globalPos );
			}
		}
	}
	QWidget::mousePressEvent( event );
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

		emit rangeChanged();
	}
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

bool BinPlotWidget::isPrintOut( int id ) const {
	bool printOut = false;
	if ( plotMode == IMAGE_MODE ){
		if ( id == IMAGE_ID ){
			printOut = true;
		}
	}
	else if ( plotMode == REGION_MODE ){
		if ( id == selectedId ){
			printOut = true;
		}
	}
	else if ( plotMode == REGION_ALL_MODE ){
		if ( id != IMAGE_ID ){
			printOut = true;
		}
	}
	return printOut;
}

void BinPlotWidget::toAscii( const QString& filePath ){
	QFile file( filePath );
	bool success = file.open( QIODevice::WriteOnly | QIODevice::Text );
	const QString LINE_END( "\n");
	if ( success ){
		QTextStream out( &file );
		out.setFieldWidth( 20 );
		QList<int> keys = histogramMap.keys();
		int keyCount = keys.size();
		String imageName = image->name();
		QString title = "Histogram for "+QString(imageName.c_str());
		out << title << LINE_END << LINE_END;
		for ( int i = 0; i < keyCount; i++ ){
			bool printOut = isPrintOut( keys[i] );
			if ( printOut ){
				if ( histogramMap[keys[i]]->getDataCount() > 0 ){
					QString subTitle;
					if ( keys[i] != IMAGE_ID ){
						subTitle.append(" Region: "+QString::number(keys[i]));
						out << subTitle << LINE_END;
					}

					histogramMap[keys[i]]->toAscii( out );
					out << LINE_END;
					out.flush();
				}
			}
		}

		if ( fitWidget != NULL  ){
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
	if ( plotMode != IMAGE_MODE ){
		selectedId = id;
		resetRegion( );
	}
}

void BinPlotWidget::clearHistograms(){
	QList<int> histKeys = histogramMap.keys();
	int keyCount = histKeys.size();
	for( int i = 0; i < keyCount; i++ ){
		int key = histKeys[i];
		Histogram* histogram = histogramMap[key];
		delete histogram;
	}
	histogramMap.clear();
}

void BinPlotWidget::clearAll(){
	clearFit();
	clearCurves();
	clearRange();
	binPlot.replot();
}

BinPlotWidget::~BinPlotWidget(){
	//delete colorMap;
	clearAll();
	clearHistograms();
	delete colorCurve;
}

}

