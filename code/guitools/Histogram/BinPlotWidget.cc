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
#include <guitools/Histogram/FitWidget.qo.h>
#include <guitools/Histogram/HistogramMarkerGaussian.h>
#include <guitools/Histogram/PlotControlsWidget.qo.h>
#include <guitools/Histogram/RangeControlsWidget.qo.h>
#include <guitools/Histogram/RangePicker.h>

#include <QDebug>
#include <QPainter>
#include <QMessageBox>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

namespace casa {


BinPlotWidget::BinPlotWidget( bool plotControls, bool fitControls, bool rangeControls, QWidget* parent ):
    QWidget(parent),
    curveColor( Qt::blue ), selectionColor( 205, 201, 201, 127 ),
    histogramMaker( NULL ), binPlot( this ),
    lambdaAction("Lambda",this), centerPeakAction( "(Center,Peak)",this),
    fwhmAction( "Center +/- FWHM/2", this), contextMenu(this){

	ui.setupUi(this);

	//Add the plot to the widget
	QHBoxLayout* layout = new QHBoxLayout(ui.plotHolder);
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( &binPlot );
	binPlot.setCanvasBackground( Qt::white );
	ui.plotHolder->setLayout( layout );

	initializeFitWidget( fitControls );
	initializePlotControls( plotControls );
	initializeRangeControls( rangeControls );

	displayPlotTitle = false;
	displayAxisTitles = false;
}

void BinPlotWidget::setDisplayPlotTitle( bool display ){
	displayPlotTitle = display;
	resetPlotTitle();
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
	if ( displayPlotTitle ){

	}
	else {
		binPlot.setTitle( "" );
	}
}

//-------------------------------------------------------------------------------
//                            Plot Controls
//-------------------------------------------------------------------------------

void BinPlotWidget::initializePlotControls( bool plotControls ){
	displayStep = true;
	displayLog = false;
	if ( plotControls ){
		plotControlsWidget = new PlotControlsWidget( this );
		QHBoxLayout* layout = new QHBoxLayout();
		layout->addWidget( plotControlsWidget );
		ui.plotControlsHolder->setLayout( layout );
		plotControlsWidget->setDisplayStep( displayStep );
		plotControlsWidget->setDisplayLogs( displayLog );
		connect( plotControlsWidget, SIGNAL(displayLogChanged(bool)), this, SLOT(setDisplayLog(bool)));
		connect( plotControlsWidget, SIGNAL(displayStepChanged(bool)), this, SLOT(setDisplayStep( bool)));
	}
	else {
		plotControlsWidget = NULL;
	}
}

void BinPlotWidget::setDisplayStep( bool display ){
	if ( displayStep != display ){
		displayStep = display;
		defineCurve();
	}
}

void BinPlotWidget::setDisplayLog( bool display ){
	if ( displayLog != display ){
		displayLog = display;
		defineCurve();
		resetAxisTitles();
	}
}
//-------------------------------------------------------------------------------
//                          Marking Ranges on the Histogram
//-------------------------------------------------------------------------------

void BinPlotWidget::initializeRangeControls( bool enable ){
	if ( enable ){

		//Put the range controls in
		QHBoxLayout* layout = new QHBoxLayout();
		rangeControlWidget = new RangeControlsWidget( this );
		layout->addWidget( rangeControlWidget );
		ui.rangeControlsHolder->setLayout( layout );
		connect( rangeControlWidget, SIGNAL(minMaxChanged()), this, SLOT(minMaxChanged()));

		//This is what draws the final rectangle.
		rectMarker = new QwtPlotMarker();
		QwtSymbol* symbol = new QwtSymbol();
		QPen pen( selectionColor );
		symbol->setPen( pen );
		symbol->setBrush( selectionColor );
		symbol->setStyle( QwtSymbol::Rect );
		rectMarker->setSymbol( *symbol );
		rectMarker->attach( &binPlot );
		rectMarker->hide();

		//This is what draws the rectangle while it is being dragged.
		rangeTool = new RangePicker( binPlot.canvas() );
		rangeTool->setHeightSource( this );
		connect( rangeTool, SIGNAL(selected(const QwtDoubleRect &)), this, SLOT(rectangleSelected(const QwtDoubleRect &)));
		connect( rangeTool, SIGNAL(appended(const QPoint&)), this, SLOT(clearRange()));
	}
	else {
		rectMarker = NULL;
		rangeControlWidget = NULL;
		rangeTool = NULL;
	}
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
		qDebug() << "Warning:  min/max values won't be valid without range control enabled";
	}
	return minMaxValues;
}


//-------------------------------------------------------------------------------
//                         Fitting the Histogram
//-------------------------------------------------------------------------------

void BinPlotWidget::clearFitMarkers(){
	delete fitEstimateMarkerGaussian;
	fitEstimateMarkerGaussian = NULL;
}

void BinPlotWidget::initializeGaussianFitMarker(){
	fitEstimateMarkerGaussian = new HistogramMarkerGaussian();
	/*QwtSymbol* symbol = new QwtSymbol();
	QPen pen( Qt::black );
	pen.setWidth( MARKER_WIDTH );
	symbol->setPen( pen );
	symbol->setBrush( Qt::blue );
	symbol->setStyle( QwtSymbol::Diamond );
	fitEstimateMarkerGaussian->setSymbol( *symbol );*/
	fitEstimateMarkerGaussian->setXAxis( QwtPlot::xBottom );
	fitEstimateMarkerGaussian->setYAxis( QwtPlot::yLeft );
	fitEstimateMarkerGaussian->attach( &binPlot );
}


void BinPlotWidget::initializeFitWidget( bool fitControls ){
	fitCurve = NULL;
	fitEstimateMarkerGaussian = NULL;
	if ( fitControls ){
		QVBoxLayout* fitLayout = new QVBoxLayout();
		fitWidget = new FitWidget( this );
		fitLayout->addWidget( fitWidget );
		ui.fitWidgetHolder->setLayout( fitLayout );
		fitModeChanged();

		setContextMenuPolicy( Qt::CustomContextMenu);
		connect( this, SIGNAL(customContextMenuRequested( const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
		connect( fitWidget, SIGNAL(fitModeChanged()), this, SLOT(fitModeChanged()));
		connect( fitWidget, SIGNAL(dataFitted()), this, SLOT(fitDone()));
		connect( fitWidget, SIGNAL(gaussianFitChanged()), this, SLOT(resetGaussianFitMarker()));
		connect( &centerPeakAction, SIGNAL(triggered()), this, SLOT(centerPeakSpecified()));
		connect( &fwhmAction, SIGNAL(triggered()), this, SLOT(fwhmSpecified()));
		connect( &lambdaAction, SIGNAL(triggered()), this, SLOT(lambdaSpecified()));
	}
	else {
		fitWidget = NULL;
		setContextMenuPolicy( Qt::NoContextMenu );
	}
}

void BinPlotWidget::centerPeakSpecified(){
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
	qDebug() << "No support yet for specifying lambda";
}

void BinPlotWidget::fitModeChanged(){
	contextMenu.clear();
	if ( fitWidget->isGaussian() ){
		contextMenu.addAction(&centerPeakAction );
		contextMenu.addAction( &fwhmAction );
	}
	else {
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
	qDebug() << "left="<<left<<" right="<<right;
	qDebug() << "top="<<top<<" bottom="<<bottom;
	if ( left <= x && x <= right ){
		if ( top <= y && y <= bottom ){
			qDebug() << "Inside plot";
			insidePlot = true;
		}
	}
	return insidePlot;
}

void BinPlotWidget::showContextMenu( const QPoint& pt ){
	QPoint globalPos = mapToGlobal( pt );
	int x = pt.x();
	int y = pt.y();
	qDebug() << "Point x="<<pt.x()<<" y="<<pt.y();
	bool ptInPlot = isPlotContains( x, y );
	if ( ptInPlot ){
		//Change x by the amount the y-axis takes up.
		QRect plotGeom = ui.plotHolder->geometry();
		int yAxisSpace = ui.plotHolder->width() - binPlot.canvas()->width();
		int adjustedX = x - yAxisSpace - plotGeom.x();
		qDebug() << "adjustedX="<<adjustedX;
		int adjustedY = y - plotGeom.y();
		fitPosition = QPoint( adjustedX, adjustedY);
		contextMenu.exec( globalPos );
	}
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

void BinPlotWidget::fitDone(){
	Vector<Float> yValues = fitWidget->getFitValues();
	if ( yValues.size() > 0 ){
		if ( fitCurve != NULL ){
			fitCurve->detach();
			delete fitCurve;
		}
		QVector<double> curveXValues( xVector.size() );
		QVector<double> curveYValues( xVector.size() );
		qDebug() << "Adding curve: "<<yValues.size();
		for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
			curveXValues[i] = xVector[i];
			curveYValues[i] = yValues[i];
		}
		fitCurve = addCurve( curveXValues, curveYValues );
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
	double previousY = 0;
	for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
		//Draw vertical line
		QVector<double> xValues(2);
		QVector<double> yValues(2);
		xValues[0] = xVector[i];
		xValues[1] = xVector[i];
		yValues[0] = checkLogValue(previousY);
		yValues[1] = checkLogValue(yVector[i]);
		addCurve( xValues, yValues );

		//Draw horizontal line connecting to previous point.
		if ( i > 0 ){
			xValues[0] = xVector[i-1];
			xValues[1] = xVector[i];
			yValues[0] = checkLogValue(previousY);
			yValues[1] = checkLogValue(previousY);
			addCurve( xValues, yValues );
		}
		previousY = yVector[i];
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

bool BinPlotWidget::setImage( ImageInterface<Float>* img ){
	bool success = false;
	if ( img != NULL ){
		if ( histogramMaker == NULL ){
			histogramMaker = new ImageHistograms<Float>( *img );
			rectangleSizeChanged();
		}
		else {
			histogramMaker->setNewImage( *img );
		}
		Array<Float> values;
		Array<Float> counts;
		success = histogramMaker->getHistograms( values, counts );
		if ( success ){
			//Store the data
			values.tovector( xVector );
			counts.tovector( yVector );
			defineCurve();
			setValidatorLimits();
			if ( fitWidget != NULL ){
				fitWidget->setXValues( xVector );
			}
			update();
		}
		else {
			QString msg( "Could not make a histogram from the image.");
			QMessageBox::warning( this, "Error Making Histogram", msg);
		}
	}
	return success;
}

void BinPlotWidget::setValidatorLimits(){
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

void BinPlotWidget::clearCurves(){
	while( ! curves.isEmpty() ){
		QwtPlotCurve* curve = curves.takeFirst();
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
}

void BinPlotWidget::rectangleSizeChanged(){
	resetRectangleMarker();
	binPlot.replot();
}

void BinPlotWidget::rectangleSelected(const QwtDoubleRect & rect){
	rectX = rect.x();
	rectWidth = rect.width();
	resetRectangleMarker();
	rectMarker->show();
	binPlot.replot();
	setMinMaxValues( rectX, rectX + rectWidth, false );
}

void BinPlotWidget::resetRectangleMarker(){
	QwtSymbol symbol = rectMarker->symbol();
	int pixelXStart = static_cast<int>( binPlot.transform( QwtPlot::xBottom, rectX ) );
	int pixelXEnd = static_cast<int>(binPlot.transform( QwtPlot::xBottom, rectX+rectWidth ));
	int height = getCanvasHeight();
	int width = pixelXEnd - pixelXStart;
	symbol.setSize( width, height );
	rectMarker->setSymbol( symbol );
	double centerX = rectX + rectWidth / 2;
	double centerY = binPlot.invTransform( QwtPlot::yLeft, height / 2 );
	rectMarker->setValue( centerX, centerY );
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
		qDebug() << "Range tools need to be enabled for minMaxChanged";
	}
}

void BinPlotWidget::clearRange(){
	if ( rectMarker->isVisible() ){
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

BinPlotWidget::~BinPlotWidget(){
	clearCurves();
	delete histogramMaker;
	delete fitEstimateMarkerGaussian;
}



}

