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
#include "FeatherPlotWidget.qo.h"

#include <QDebug>
#include <QGridLayout>
#include <QMouseEvent>
#include <qwt_plot_picker.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

namespace casa {

FeatherPlotWidget::FeatherPlotWidget(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent)
    : QWidget(parent), plot( NULL ),
      singleDishWeightColor(Qt::cyan), singleDishDataColor(Qt::green),
      interferometerWeightColor( Qt::magenta), interferometerDataColor(Qt::blue),
      singleDishFunction( "Low Resolution Slice"), interferometerFunction( "High Resolution Slice"),
      singleDishWeightFunction("Low Resolution Weight"), interferometerWeightFunction( "High Resolution Weight"),
      legendVisible( true ), permanentScatter( false ),
      lineThickness(1), plotTitle(title), MARKER_WIDTH(2){

	ui.setupUi(this);

	sliceAxis  = QwtPlot::yLeft;
	weightAxis = QwtPlot::yRight;

	resetPlot( plotType );

	leftMouseMode = RECTANGLE_ZOOM;
	zoomer = NULL;
	diameterSelector = NULL;
	diameterMarker = NULL;
	initializeZooming();
	initializeDiameterMarker();
	initializeDiameterSelector();
}

FeatherPlotWidget::~FeatherPlotWidget(){
	delete zoomer;
	zoomer = NULL;
	delete diameterMarker;
	diameterMarker = NULL;
	delete diameterSelector;
	diameterSelector = NULL;
}

void FeatherPlotWidget::changePlotType( FeatherPlot::PlotType newPlotType ){
	//Scatter plots do not change type.
	removeMarkers();
	if ( !permanentScatter ){
		resetPlot( newPlotType );
		if ( plot->isScatterPlot() ){
			addScatterData();
		}
		else {
			addZoomNeutralCurves();
		}
	}
}

void FeatherPlotWidget::resetPlot( FeatherPlot::PlotType plotType ){

	if ( plot == NULL ){
		QLayout* layoutPlot = layout();
		if ( layoutPlot == NULL ){
			layoutPlot = new QHBoxLayout( this );
		}
		plot = new FeatherPlot( this );
		plot->setLegendVisibility( legendVisible );
		plot->setLineThickness( lineThickness );
		layoutPlot->addWidget( plot );
		layoutPlot->setContentsMargins(0,0,0,0);
		setLayout( layoutPlot );
	}
	else {
		plot->clearCurves();
	}
	plot->initializePlot( plotTitle, plotType );

}

//-----------------------------------------------------------------------------
//                                 Properties
//----------------------------------------------------------------------------

void FeatherPlotWidget::setLineThickness( int thickness ){
	lineThickness = thickness;
	plot->setLineThickness( thickness );
	plot->replot();
}

void FeatherPlotWidget::setLegendVisibility( bool visible ){
	legendVisible = visible;
	plot->setLegendVisibility( visible );
	plot->replot();
}

void FeatherPlotWidget::setPermanentScatter( bool scatter ){
	permanentScatter = scatter;
}

void FeatherPlotWidget::setPlotColors( const QMap<PreferencesColor::FunctionColor,QColor>& colorMap,
		const QColor& scatterColor, const QColor& dishDiameterColor,
		const QColor& zoomRectangleColor ){
	singleDishDataColor = colorMap[PreferencesColor::SD_SLICE_COLOR];
	interferometerDataColor = colorMap[PreferencesColor::INT_SLICE_COLOR];
	singleDishWeightColor = colorMap[PreferencesColor::SD_WEIGHT_COLOR];
	interferometerWeightColor = colorMap[PreferencesColor::INT_WEIGHT_COLOR];
	scatterPlotColor = scatterColor;
	dishDiameterLineColor = dishDiameterColor;
	zoomRectColor = zoomRectangleColor;
	resetColors();
}

QWidget* FeatherPlotWidget::getExternalAxisWidget( QwtPlot::Axis position ){
	return plot->getExternalAxisWidget( position );
}

void FeatherPlotWidget::insertLegend( QWidget* parent ){
	plot->insertSingleLegend( parent );
}

void FeatherPlotWidget::setRectangleZoomMode(){
	leftMouseMode = RECTANGLE_ZOOM;
	changeLeftMouseMode();
}

void FeatherPlotWidget::setDiameterSelectorMode(){
	leftMouseMode = DIAMETER_SELECTION;
	changeLeftMouseMode();
}

void FeatherPlotWidget::resetColors(){
	if ( plot->isSliceCut() ){
		plot->setFunctionColor( singleDishFunction, singleDishDataColor );
		plot->setFunctionColor( interferometerFunction, interferometerDataColor );
		plot->setFunctionColor( singleDishWeightFunction, singleDishWeightColor );
		plot->setFunctionColor( interferometerWeightFunction, interferometerWeightColor );
	}
	else if ( plot->isScatterPlot() ){
		plot->setFunctionColor( "", scatterPlotColor );
	}
	else {
		plot->setFunctionColor( this->singleDishFunction, singleDishDataColor );
		plot->setFunctionColor( this->interferometerFunction, interferometerDataColor );
	}
	resetDishDiameterLineColor();
	resetZoomRectangleColor();
	plot->replot();
}

//-------------------------------------------------------------------------------
//                          Event handling
//-------------------------------------------------------------------------------

void FeatherPlotWidget::resizeEvent( QResizeEvent* event ){
	QWidget::resizeEvent( event );
	setDishDiameter();
}


//-----------------------------------------------------------------------------------
//                        Diameter Marking
//----------------------------------------------------------------------------------

void FeatherPlotWidget::initializeDiameterSelector(){
	//This is what draws the rectangle while it is being dragged.
	if ( diameterSelector == NULL ){
		diameterSelector = new QwtPlotPicker(plot->canvas());
		QPen pen(Qt::black );
		pen.setWidth( MARKER_WIDTH );
		diameterSelector -> setTrackerPen( pen );
		diameterSelector -> setAxis(QwtPlot::xBottom, QwtPlot::yLeft);
		diameterSelector->setSelectionFlags(QwtPlotPicker::PointSelection | QwtPlotPicker::DragSelection);
		diameterSelector->setRubberBand( QwtPlotPicker::VLineRubberBand );
		diameterSelector->setTrackerMode( QwtPlotPicker::AlwaysOff );
		connect( diameterSelector, SIGNAL(selected ( const QwtDoublePoint& )), this, SLOT(diameterSelected( const QwtDoublePoint& )));
		diameterSelector->setMousePattern( QwtEventPattern::MouseSelect1, Qt::NoButton, Qt::ShiftModifier );
	}
}

void FeatherPlotWidget::initializeDiameterMarker(){
	//The diameter marker draws a line on the plot indicating the
	//effective dish diameter.
	if ( diameterMarker == NULL ){
		diameterMarker = new QwtPlotMarker();
		QwtSymbol* symbol = new QwtSymbol();
		QPen pen( Qt::black );
		pen.setWidth( MARKER_WIDTH );
		symbol->setPen( pen );
		symbol->setBrush( Qt::blue );
		symbol->setStyle( QwtSymbol::VLine );
		diameterMarker->setSymbol( *symbol );
		diameterMarker->setXAxis( QwtPlot::xBottom );
		diameterMarker->setYAxis( QwtPlot::yLeft );
	}
}

void FeatherPlotWidget::diameterSelected( const QwtDoublePoint& pos ){
	if ( !plot->isScatterPlot()){
		double diameter = pos.x();
		setDishDiameter( diameter );
		emit dishDiameterChanged( diameter );
	}
}

void FeatherPlotWidget::resetDishDiameterLineColor(){
	if ( diameterMarker != NULL && diameterSelector != NULL ){
		QwtSymbol diameterSymbol = diameterMarker->symbol();
		QPen pen = diameterSymbol.pen();
		pen.setColor( dishDiameterLineColor );
		diameterSymbol.setPen( pen );
		diameterMarker->setSymbol( diameterSymbol );
		diameterMarker->setLinePen( pen );
		diameterSelector->setRubberBandPen( pen );
	}
}

double FeatherPlotWidget::getDishDiameter() const {
	double position = -1;
	if ( diameterMarker != NULL ){
		position = diameterMarker->xValue();
	}
	return position;
}

void FeatherPlotWidget::setDishDiameter( double position ){
	//Set the size
	QwtSymbol symbol = diameterMarker->symbol();
	int canvasHeight = height();
	symbol.setSize( 2 * canvasHeight );

	//Set the position
	if ( position >= 0 ){
		diameterMarker->setSymbol( symbol );
		diameterMarker->setXValue( position );
		if ( !plot->isScatterPlot() ){
			diameterMarker->show();
		}
	}
	plot->replot();
}

bool FeatherPlotWidget::isDiameterSelectorMode() const {
	return false;
}


//------------------------------------------------------------------------------------
//                      Zooming
//------------------------------------------------------------------------------------


void FeatherPlotWidget::initializeZooming(){
	if ( zoomer == NULL ){
		zoomer = new QwtPlotPicker(plot->canvas());
		QPen pen(zoomRectColor );
		pen.setWidth( 2 );
		zoomer -> setTrackerPen( pen );
		zoomer -> setAxis(QwtPlot::xBottom, QwtPlot::yLeft);
		zoomer -> setSelectionFlags(QwtPlotPicker::RectSelection | QwtPlotPicker::DragSelection);
		zoomer -> setRubberBand( QwtPlotPicker::RectRubberBand );
		zoomer->setTrackerMode( QwtPlotPicker::AlwaysOff );
		zoomer->setMousePattern( QwtEventPattern::MouseSelect1, Qt::NoButton );
		connect( zoomer, SIGNAL(selected ( const QwtDoubleRect& )), this, SLOT(zoomRectangleSelected( const QwtDoubleRect& )));
	}
}

void FeatherPlotWidget::zoomRectangleSelected( const QwtDoubleRect& zoomRect ){
	double minX = zoomRect.x();
	double maxX = minX + zoomRect.width();
	double minY = zoomRect.y();
	double maxY = minY + zoomRect.height();

	//If we are a scatter plot, we don't have to let anyone know about
	//the zoom, just update our own coordinates.
	if ( plot->isScatterPlot() ){
		zoomRectangleScatter( minX, maxX, minY, maxY );
	}
	//Tell the plot holder about the zoom so all the plots can zoom in sync.
	else {
		emit rectangleZoomed( minX, maxX, minY, maxY );
	}
}

void FeatherPlotWidget::zoomRectangleWeight( double minX, double maxX, double minY, double maxY ){
	//The y-values are in terms of the left axis.  Weight functions use the right axis.  We
	//must translate the bounds to corresponding bounds on the right axis.
	int minYPixels = plot->transform( sliceAxis, minY );
	int maxYPixels = plot->transform( sliceAxis, maxY );
	double minYWeight = plot->invTransform( weightAxis, minYPixels );
	double maxYWeight = plot->invTransform( weightAxis, maxYPixels );

	QVector<double> singleDishWeightX;
	QVector<double> singleDishWeightY;
	initializeDomainRangeLimitedData( minX, maxX, minYWeight, maxYWeight, singleDishWeightX, singleDishWeightY, singleDishWeightXValues, singleDishWeightYValues );
	plot->addCurve( singleDishWeightX, singleDishWeightY, singleDishWeightColor, singleDishWeightFunction, weightAxis );

	QVector<double> interferometerWeightX;
	QVector<double> interferometerWeightY;
	initializeDomainRangeLimitedData( minX, maxX, minYWeight, maxYWeight, interferometerWeightX, interferometerWeightY, interferometerWeightXValues, interferometerWeightYValues );
	plot->addCurve( interferometerWeightX, interferometerWeightY, interferometerWeightColor, interferometerWeightFunction, weightAxis );
}

void FeatherPlotWidget::zoomRectangle( double minX, double maxX, double minY, double maxY ){
	if ( plot->isSliceCut() ){
		zoomRectangleWeight( minX, maxX, minY, maxY );
	}
	QVector<double> singleDishX;
	QVector<double> singleDishY;
	initializeDomainRangeLimitedData( minX, maxX, minY, maxY, singleDishX, singleDishY, singleDishDataXValues, singleDishDataYValues );
	plot->addCurve( singleDishX, singleDishY, singleDishDataColor, singleDishFunction, sliceAxis );

	QVector<double> interferometerX;
	QVector<double> interferometerY;
	initializeDomainRangeLimitedData( minX, maxX, minY, maxY, interferometerX, interferometerY, interferometerDataXValues, interferometerDataYValues );
	plot->addCurve( interferometerX, interferometerY, interferometerDataColor, interferometerFunction, sliceAxis );
	plot->replot();
}

void FeatherPlotWidget::zoomRectangleScatter( double minX, double maxX, double minY, double maxY ){
	int count = qMin( singleDishDataYValues.size(), interferometerDataYValues.size());
	int dataCount = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minX <= singleDishDataYValues[i] && singleDishDataYValues[i] <= maxX ){
			if ( minY <= interferometerDataYValues[i] && interferometerDataYValues[i] <= maxY ){
				dataCount++;
			}
		}
	}

	QVector<double> xValues( dataCount );
	QVector<double> yValues( dataCount );
	int j = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minX <= singleDishDataYValues[i] && singleDishDataYValues[i] <= maxX ){
			if ( minY <= interferometerDataYValues[i] && interferometerDataYValues[i] <= maxY ){
				xValues[j] = singleDishDataYValues[i];
				yValues[j] = interferometerDataYValues[i];
				j++;
			}
		}
	}
	plot->clearCurves();
	plot->addCurve( xValues, yValues, scatterPlotColor, "", sliceAxis );
	plot->replot();
}

void FeatherPlotWidget::changeZoom90(bool zoom ){
	if ( zoom ){
		plot->clearCurves();

		//Calculate the yValues that represent 90% of the amplitude.
		pair<double,double> singleDishMinMax = getMaxMin( singleDishWeightYValues );
		pair<double,double> interferometerMinMax = getMaxMin( interferometerWeightYValues );
		double minValue = qMin( singleDishMinMax.first, interferometerMinMax.first );
		double maxValue = qMax( singleDishMinMax.second, interferometerMinMax.second );
		double increase = (maxValue - minValue) * .25;
		minValue = minValue + increase;
		maxValue = maxValue - increase;

		//Reset the weight values to 90% of the yRange while figuring out the
		//xcut off values minX and maxX.
		Double minX = numeric_limits<double>::max();
		Double maxX = numeric_limits<double>::min();
		QVector<double> singleDishXZoom;
		QVector<double> singleDishYZoom;
		initializeRangeLimitedData( minValue, maxValue, singleDishXZoom, singleDishYZoom,
				singleDishWeightXValues, singleDishWeightYValues, &minX, &maxX );
		QVector<double> interferometerXZoom;
		QVector<double> interferometerYZoom;
		initializeRangeLimitedData( minValue, maxValue, interferometerXZoom, interferometerYZoom,
					interferometerWeightXValues, interferometerWeightYValues, &minX, &maxX );
		if ( plot->isSliceCut() ){
			plot->addCurve( singleDishXZoom, singleDishYZoom, singleDishWeightColor, singleDishWeightFunction, weightAxis );
			plot->addCurve( interferometerXZoom, interferometerYZoom, interferometerWeightColor, interferometerWeightFunction, weightAxis );
		}

		//Use the xbounds to redo the slice cut data.
		QVector<double> singleDishX;
		QVector<double> singleDishY;
		initializeDomainLimitedData( minX, maxX, singleDishX, singleDishY, singleDishDataXValues, singleDishDataYValues );
		QVector<double> interferometerX;
		QVector<double> interferometerY;
		initializeDomainLimitedData( minX, maxX, interferometerX, interferometerY, interferometerDataXValues, interferometerDataYValues );
		if ( plot->isScatterPlot() ){
			plot->addCurve( singleDishX, singleDishY, singleDishDataColor, singleDishFunction, sliceAxis );
			plot->addCurve( interferometerX, interferometerY, interferometerDataColor, interferometerFunction, sliceAxis );
		}

		//Finally use the generated ydata from the slice cut to zoom the scatterplot
		else {
			plot->addCurve(singleDishY, interferometerY, scatterPlotColor, "", sliceAxis );
		}

		plot->replot();
	}
	else {
		zoomNeutral();
	}
}

void FeatherPlotWidget::zoomNeutral(){
	FeatherPlot::PlotType originalPlotType = plot->getPlotType();
	resetPlot( originalPlotType );
	if ( !plot->isScatterPlot() ){
		addZoomNeutralCurves();
	}
	else {
		addScatterData();
	}
}

void FeatherPlotWidget::addZoomNeutralCurves(){
	if ( plot->isSliceCut()){
		plot->addCurve( singleDishWeightXValues, singleDishWeightYValues, singleDishWeightColor, singleDishWeightFunction, weightAxis );
		plot->addCurve( interferometerWeightXValues, interferometerWeightYValues, interferometerWeightColor, interferometerWeightFunction, weightAxis );
	}
	plot->addCurve( singleDishDataXValues, singleDishDataYValues, singleDishDataColor, singleDishFunction, sliceAxis );
	plot->addCurve( interferometerDataXValues, interferometerDataYValues, interferometerDataColor, interferometerFunction, sliceAxis );
	initializeMarkers();
	plot->replot();
}

void FeatherPlotWidget::resetZoomRectangleColor(){
	if ( zoomer != NULL ){
		QPen zoomerPen = zoomer->trackerPen();
		zoomerPen.setColor( zoomRectColor );
		zoomer->setTrackerPen( zoomerPen );
		zoomer->setRubberBandPen( zoomerPen );
	}
}


//---------------------------------------------------------------------------------------
//                                   Data
//---------------------------------------------------------------------------------------
void FeatherPlotWidget::initializeMarkers(){
	if ( !plot->isScatterPlot() ){
		diameterMarker->attach( plot );

	}
	this->changeLeftMouseMode();
}

void FeatherPlotWidget::removeMarkers(){
	diameterMarker->detach();
	diameterSelector->setMousePattern( QwtEventPattern::MouseSelect1, Qt::NoButton);
	zoomer->setMousePattern( QwtEventPattern::MouseSelect1, Qt::NoButton );
}

void FeatherPlotWidget::addScatterData(){
	plot->addCurve( singleDishDataYValues, interferometerDataYValues, scatterPlotColor, "", sliceAxis );
	initializeMarkers();
	plot->replot();
}

void FeatherPlotWidget::setSingleDishWeight( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	singleDishWeightXValues.resize( count );
	singleDishWeightYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		singleDishWeightXValues[i] = xValues[i];
		singleDishWeightYValues[i] = yValues[i];
	}
	if ( plot->isSliceCut() ){
		plot->addCurve( singleDishWeightXValues, singleDishWeightYValues, singleDishWeightColor, singleDishWeightFunction, weightAxis );
		plot->replot();
	}

}

void FeatherPlotWidget::setSingleDishData( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	singleDishDataXValues.resize( count );
	singleDishDataYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		singleDishDataXValues[i] = xValues[i];
		singleDishDataYValues[i] = yValues[i];
	}
	if ( !plot->isScatterPlot() ){
		plot->addCurve( singleDishDataXValues, singleDishDataYValues, singleDishDataColor, singleDishFunction, sliceAxis );
	}
	initializeMarkers();
	plot->replot();
}

void FeatherPlotWidget::setInterferometerWeight( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	interferometerWeightXValues.resize( count );
	interferometerWeightYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		interferometerWeightXValues[i] = xValues[i];
		interferometerWeightYValues[i] = yValues[i];
	}

	if ( plot->isSliceCut() ){
		plot->addCurve( interferometerWeightXValues, interferometerWeightYValues, interferometerWeightColor, interferometerWeightFunction, weightAxis );
		plot->replot();
	}
}

void FeatherPlotWidget::setInterferometerData( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	interferometerDataXValues.resize( count );
	interferometerDataYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		interferometerDataXValues[i] = xValues[i];
		interferometerDataYValues[i] = yValues[i];
	}
	initializeMarkers();
	if ( !plot->isScatterPlot() ){
		plot->addCurve( interferometerDataXValues, interferometerDataYValues, interferometerDataColor, interferometerFunction, sliceAxis );
	}
	plot->replot();
}

void FeatherPlotWidget::clearPlot(){
	removeMarkers();
	plot->clearCurves();
	plot->replot();
}


//-------------------------------------------------------------------------------------
//                             Utility
//-------------------------------------------------------------------------------------

void FeatherPlotWidget::initializeDomainRangeLimitedData( double minXValue, double maxXValue,
		double minYValue, double maxYValue,
		QVector<double>& xValues, QVector<double>& yValues,
		const QVector<double>& originalXValues, const QVector<double>& originalYValues) const{
	int count = qMin( originalXValues.size(), originalYValues.size());
	int domainCount = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minXValue <= originalXValues[i] && originalXValues[i] <= maxXValue ){
			if ( minYValue <= originalYValues[i] && originalYValues[i] <= maxYValue ){
				domainCount++;
			}
		}
	}

	xValues.resize( domainCount );
	yValues.resize( domainCount );
	int j = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minXValue <= originalXValues[i] && originalXValues[i] <= maxXValue ){
			if ( minYValue <= originalYValues[i] && originalYValues[i] <= maxYValue ){
				xValues[j] = originalXValues[i];
				yValues[j] = originalYValues[i];
				j++;
			}
		}
	}
}

void FeatherPlotWidget::initializeDomainLimitedData( double minValue, double maxValue,
		QVector<double>& xValues, QVector<double>& yValues,
		const QVector<double>& originalXValues, const QVector<double>& originalYValues) const{
	int count = qMin( originalXValues.size(), originalYValues.size());
	int domainCount = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minValue <= originalXValues[i] && originalXValues[i] <= maxValue ){
			domainCount++;
		}
	}

	xValues.resize( domainCount );
	yValues.resize( domainCount );
	int j = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minValue <= originalXValues[i] && originalXValues[i] <= maxValue ){
			xValues[j] = originalXValues[i];
			yValues[j] = originalYValues[i];
			j++;
		}
	}
}

void FeatherPlotWidget::initializeRangeLimitedData( double minValue, double maxValue,
		QVector<double>& xValues, QVector<double>& yValues,
		const QVector<double>& originalXValues, const QVector<double>& originalYValues,
		Double* minX, Double* maxX) const{
	int count = qMin( originalXValues.size(), originalYValues.size());
	int rangeCount = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minValue <= originalYValues[i] && originalYValues[i] <= maxValue ){
			rangeCount++;
		}
	}

	xValues.resize( rangeCount );
	yValues.resize( rangeCount );
	int j = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minValue <= originalYValues[i] && originalYValues[i] <= maxValue ){
			xValues[j] = originalXValues[i];
			yValues[j] = originalYValues[i];
			if ( xValues[j] < *minX ){
				*minX = xValues[j];
			}
			else if (xValues[j] > *maxX ){
				*maxX = xValues[j];
			}
			j++;
		}
	}
}

pair<double,double> FeatherPlotWidget::getMaxMin( QVector<double> values ) const {
	double minValue = numeric_limits<double>::max();
	double maxValue = numeric_limits<double>::min();
	for ( int i = 0; i < values.size(); i++ ){
		if ( values[i]< minValue ){
			minValue = values[i];
		}
		else if ( values[i] > maxValue ){
			maxValue = values[i];
		}
	}
	pair<double,double> minMaxPair( minValue, maxValue );
	return minMaxPair;
}

void FeatherPlotWidget::changeLeftMouseMode(){
	if ( ! plot->isEmpty() ){
		if ( leftMouseMode == RECTANGLE_ZOOM ){
			//Get the left mouse to the rectangle zoom, and a shift qualifier to the
			//dish diameter selector
			if ( !plot->isScatterPlot() ){
				diameterSelector->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier );
			}
			zoomer->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton );
		}
		else if ( leftMouseMode == DIAMETER_SELECTION ){
			//Turn the zoomer off and give the left mouse to the diameter selector.
			zoomer->setMousePattern( QwtEventPattern::MouseSelect1, Qt::NoButton );
			if ( !plot->isScatterPlot() ){
				diameterSelector->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton );
			}
		}
		else {
			qDebug() <<" Unrecognized leftMouseMode="<<leftMouseMode;
		}
	}
}

}
