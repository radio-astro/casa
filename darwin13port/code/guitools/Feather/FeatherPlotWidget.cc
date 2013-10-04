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
#include <QtCore/qmath.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <limits>

namespace casa {

FeatherPlotWidget::FeatherPlotWidget(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent)
    : QWidget(parent), plot( NULL ),
      plotTitle(title), MARKER_WIDTH(2){

	ui.setupUi(this);

	sliceAxis  = QwtPlot::yLeft;
	weightAxis = QwtPlot::yRight;
	scatterAxis = QwtPlot::yLeft;

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


void FeatherPlotWidget::resetPlot( FeatherPlot::PlotType plotType ){
	if ( plot == NULL ){
		QLayout* layoutPlot = layout();
		if ( layoutPlot == NULL ){
			layoutPlot = new QHBoxLayout( this );
		}
		plot = new FeatherPlot( this );
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

void FeatherPlotWidget::refresh(){
	plot->replot();
}


void FeatherPlotWidget::setLineThickness( int thickness ){
	if ( diameterSelector != NULL ){
		QPen pen = diameterSelector->trackerPen();
		pen.setWidth( thickness );
		diameterSelector -> setTrackerPen( pen );
	}
	if ( diameterMarker != NULL ){
		QwtSymbol symbol = diameterMarker->symbol();
		QPen pen = symbol.pen();
		pen.setWidth( thickness );
		symbol.setPen( pen );
		diameterMarker->setSymbol( symbol );
	}
	plot->setLineThickness( thickness );
}


void FeatherPlotWidget::setDotSize( int size ){
	plot->setDotSize( size );
}


void FeatherPlotWidget::setLegendVisibility( bool visible ){
	plot->setLegendVisibility( visible );
}


void FeatherPlotWidget::setLogScale( bool uvScale, bool ampScale ){
	//We have to reset the sum slice from here if we are changing
	//amplitude from log to no log or vice versa.  This is because
	//log (a+b) != log a + log b.
	addSumData( ampScale );
	//Change the scale used by the plot curves.
	bool scaleChanged = plot->setLogScale( uvScale, ampScale );

	//Change the scale used by the diameter marker
	double markerPosition = diameterMarker->xValue();
	if ( scaleChanged ){

		if ( uvScale ){
			markerPosition = qLn( markerPosition ) / qLn( 10 );
		}
		else {
			markerPosition = qPow( 10, markerPosition );
		}
		diameterMarker->setXValue( markerPosition );
	}
	plot->replot();
}


void FeatherPlotWidget::setPlotColors( const QMap<CurveType,CurveDisplay>& colorMap){
	curvePreferences = colorMap;
	//Because the visibility of the plot may have changed
	//we need to erase and start over.
	clearPlot();
	addZoomNeutralCurves();

	//We also need to set the visibility of the dish diameter marker
	//in case it has been turned on/off.
	if ( plot->isSliceCut() ){
		bool markerDisplayed = curvePreferences[FeatherCurveType::DISH_DIAMETER].isDisplayed();
		diameterSelector->setEnabled( markerDisplayed );
		if ( markerDisplayed ){
			diameterMarker->show();
		}
		else {
			diameterMarker->hide();
		}
	}

	//Reset the curve colors
	resetColors();
	plot->resetLegend();
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
		diameterMarker->setYAxis( QwtPlot::yRight);
	}
}

void FeatherPlotWidget::diameterSelected( const QwtDoublePoint& pos ){
	//Called when the marker is selected.
	if ( !plot->isScatterPlot()){
		double diameter = pos.x();
		setDishDiameter( diameter, false );
		//Take out any log scale when we propagate the event up the stack
		if ( plot->isLogUV() ){
			diameter = qPow( 10, diameter );
		}
		emit dishDiameterChanged( diameter );
	}
}

void FeatherPlotWidget::resetDishDiameterLineColor(){
	if ( diameterMarker != NULL){
		QwtSymbol diameterSymbol = diameterMarker->symbol();
		QPen pen = diameterSymbol.pen();
		pen.setColor( curvePreferences[FeatherCurveType::DISH_DIAMETER].getColor() );
		diameterSymbol.setPen( pen );
		diameterMarker->setSymbol( diameterSymbol );
		diameterMarker->setLinePen( pen );
		if ( diameterSelector != NULL ){
			diameterSelector->setRubberBandPen( pen );
		}
	}
}

double FeatherPlotWidget::getDishDiameter() const {
	double position = -1;
	if ( diameterMarker != NULL ){
		position = diameterMarker->xValue();
	}
	return position;
}

void FeatherPlotWidget::setDishDiameter( double position, bool scale ){
	//Set the size
	QwtSymbol symbol = diameterMarker->symbol();
	int canvasHeight = height();
	symbol.setSize( 2 * canvasHeight );
	diameterMarker->setSymbol( symbol );

	//Set the position
	if ( position >= 0 ){
		if ( scale ){
			if ( plot->isLogUV() ){
				position = qLn(position) / qLn( 10 );
			}
		}
		diameterMarker->setXValue( position );
		bool markerDisplayed = curvePreferences[FeatherCurveType::DISH_DIAMETER].isDisplayed();
		if ( !plot->isScatterPlot() && markerDisplayed ){
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
		QPen pen(Qt::black );
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
	if ( zoomRect.width() > 0 && zoomRect.height() > 0 ){
		double minX = zoomRect.x();
		if ( minX < 0 ){
			minX = 0;
		}
		double maxX = minX + zoomRect.width();
		double minY = zoomRect.y();
		if ( minY < 0 ){
			minY = 0;
		}
		double maxY = minY + zoomRect.height();

		//If we are using a log scale on either axis, we need to convert
		//the rectangle bounds back to a normal scale.
		if ( plot->isLogUV() ){
			if ( !plot->isScatterPlot() ){
				minX = qPow( 10, minX);
				maxX = qPow( 10, maxX);
			}
		}
		if ( plot->isLogAmplitude() ){
			minY = qPow( 10, minY );
			maxY = qPow( 10, maxY );
			if ( plot->isScatterPlot() ){
				minX = qPow( 10, minX);
				maxX = qPow( 10, maxX);
			}
		}

		//If we are a scatter plot, we don't have to let anyone know about
		//the zoom, just update our own coordinates.
		if ( plot->isScatterPlot() ){
			plot->clearCurves();
			zoomRectangleOther( minX, maxX, minY, maxY );
			plot->replot();
		}
		//Tell the plot holder about the zoom so all the plots can zoom in sync.
		else {
			emit rectangleZoomed( minX, maxX, minY, maxY );
		}
	}
}




void FeatherPlotWidget::zoomRectangle( double minX, double maxX, double minY, double maxY){
	plot->clearCurves();
	zoomRectangleOther( minX, maxX, minY, maxY);
	initializeMarkers();
	plot->replot();
}


void FeatherPlotWidget::initializeSumData( QVector<double>& sumX, QVector<double>& sumY, bool logScale  ){
	QVector<double> singleDishX = plotData[FeatherDataType::LOW_WEIGHTED].first;
	QVector<double> singleDishY = plotData[FeatherDataType::LOW_WEIGHTED].second;
	QVector<double> interferometerX = plotData[FeatherDataType::HIGH_WEIGHTED].first;
	QVector<double> interferometerY = plotData[FeatherDataType::HIGH_WEIGHTED].second;
	int countX = qMin( singleDishX.size(), interferometerX.size() );
	if ( countX == 0 ){
		return;
	}
	sumX.resize( countX );
	sumY.resize( countX );
	for ( int i = 0; i < countX; i++ ){
		sumX[i] = singleDishX[i];
		if ( !logScale ){
			sumY[i] = singleDishY[i] + interferometerY[i];
		}
		else {
			//Occasionally we are getting single dish or interferometer
			//values of zero.  We assume these are bad and are ignoring them.
			double logSingleDish = singleDishY[i];
			if ( logSingleDish > 0 ){
				logSingleDish = qLn( singleDishY[i]) / qLn( 10 );
			}
			double logInterferometer = interferometerY[i];
			if ( logInterferometer > 0 ){
				logInterferometer = qLn( interferometerY[i]) / qLn(10 );
			}
			sumY[i] = logSingleDish + logInterferometer;
		}
	}
}

pair<QVector<double>,QVector<double> > FeatherPlotWidget::limitX( DataType dType, double minValue, double maxValue ){
	QVector<double> xLimited;
	QVector<double> yLimited;
	QVector<double> sourceX = plotData[dType].first;
	QVector<double> sourceY = plotData[dType].second;
	for ( int i = 0; i < static_cast<int>(sourceX.size()); i++ ){
		if ( minValue < sourceX[i] && sourceX[i] < maxValue ){
			xLimited.append( sourceX[i] );
			yLimited.append( sourceY[i] );
		}
	}
	return std::pair<QVector<double>, QVector<double> >(xLimited, yLimited );
}

pair<QVector<double>,QVector<double> > FeatherPlotWidget::limitX( DataType dType, double maxValue){
	return limitX( dType, -1 * std::numeric_limits<double>::max(), maxValue );
}

void FeatherPlotWidget::changeZoom90(bool zoom ){
	plot->clearCurves();
	if ( zoom ){


		//See if we know the dish position.  We want to take the dish position,
		//plus another 1/3 of it, if it is available.  If it is not, we will just
		//zoom in on the x-axis by 1/3.
		double dishPosition = 0;
		if ( diameterMarker != NULL ){
			dishPosition = diameterMarker->xValue();
			dishPosition = dishPosition + dishPosition / 3;
			//If we are using a log scale on the x-axis, we need to undo the log.
			if ( plot->isLogUV() ){
				dishPosition = qPow( 10, dishPosition );
			}
		}
		if ( dishPosition == 0 ){
			pair<double,double> minMaxPair =
					getMaxMin( plotData[FeatherDataType::LOW_WEIGHTED].second, FeatherCurveType::LOW_WEIGHTED );
			dishPosition = minMaxPair.second / 3;
		}
		zoom90Other( dishPosition );
	}
	else {
		zoomNeutral();
	}
	initializeMarkers();
	plot->replot();
}



void FeatherPlotWidget::zoomNeutral(){
	FeatherPlot::PlotType originalPlotType = plot->getPlotType();
	resetPlot( originalPlotType );
	addZoomNeutralCurves();
	initializeMarkers();
	plot->replot();
}

void FeatherPlotWidget::addPlotCurve( const QVector<double>& xValues, const QVector<double>& yValues,
		DataType dType, bool sumCurve ){
	if ( xValues.size() > 0 ){
		QwtPlot::Axis axis = getAxisYForData( dType );
		CurveType curveType = getCurveTypeForData( dType );
		addPlotCurve( xValues, yValues, axis, curveType, sumCurve );
	}
}

void FeatherPlotWidget::addPlotCurve( const QVector<double>& xValues,
		const QVector<double>& yValues, QwtPlot::Axis axis, CurveType curveType, bool sumCurve ){
	if ( curvePreferences[curveType].isDisplayed() && xValues.size() > 0 ){
		QColor curveColor = curvePreferences[curveType].getColor();
		QString curveName = curvePreferences[curveType].getName();
		plot->addCurve( xValues, yValues, curveColor, curveName, axis, sumCurve  );
		initializeMarkers();
	}
}


void FeatherPlotWidget::resetZoomRectangleColor(){
	if ( zoomer != NULL ){
		QPen zoomerPen = zoomer->trackerPen();
		QColor zoomColor = curvePreferences[FeatherCurveType::ZOOM].getColor();
		zoomerPen.setColor( zoomColor );
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

void FeatherPlotWidget::clearData(){
	plotData.clear();
}


void FeatherPlotWidget::resetData( DataType dataType,
		const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int xCount = xValues.size();
	QVector<double> xVals( xCount );
	for ( int i = 0; i < xCount; i++ ){
		xVals[i] = xValues[i];
	}
	int yCount = yValues.size();
	QVector<double> yVals( yCount);
	for ( int i = 0; i < yCount; i++ ){
		yVals[i] = yValues[i];
	}
	pair<QVector<double>,QVector<double> > dataPair( xVals, yVals );
	plotData.insert(dataType, dataPair);
}



void FeatherPlotWidget::addSumData( bool /*logAmplitude*/){
}

void FeatherPlotWidget::addSumData(){

}


QwtPlot::Axis FeatherPlotWidget::getAxisYForData( DataType dType ){
	QwtPlot::Axis axis = sliceAxis;
	if ( dType == FeatherDataType::WEIGHT_SD ||
			dType == FeatherDataType::WEIGHT_INT ){
		axis = weightAxis;
	}
	return axis;
}

FeatherDataType::DataType FeatherPlotWidget::getDataTypeForCurve( CurveType cType ) const {

	DataType dataType = FeatherDataType::END_DATA;
	switch( cType ){
	case FeatherCurveType::WEIGHT_LOW:
		dataType = FeatherDataType::WEIGHT_SD;
		break;
	case FeatherCurveType::WEIGHT_HIGH:
		dataType = FeatherDataType::WEIGHT_INT;
		break;
	case FeatherCurveType::LOW_ORIGINAL:
		dataType = FeatherDataType::LOW;
		break;
	case FeatherCurveType::LOW_WEIGHTED:
		dataType = FeatherDataType::LOW_WEIGHTED;
		break;
	case FeatherCurveType::LOW_CONVOLVED_HIGH:
		dataType = FeatherDataType::LOW_CONVOLVED_HIGH;
		break;
	case FeatherCurveType::LOW_CONVOLVED_HIGH_WEIGHTED:
		dataType = FeatherDataType::LOW_CONVOLVED_HIGH_WEIGHTED;
		break;
	/*case FeatherCurveType::LOW_CONVOLVED_DIRTY:
		dataType = FeatherDataType::LOW_CONVOLVED_DIRTY;
		break;
	case FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED:
		dataType = FeatherDataType::LOW_CONVOLVED_DIRTY_WEIGHTED;
		break;*/
	case FeatherCurveType::HIGH_ORIGINAL:
		dataType = FeatherDataType::HIGH;
		break;
	case FeatherCurveType::HIGH_WEIGHTED:
		dataType = FeatherDataType::HIGH_WEIGHTED;
		break;
	case FeatherCurveType::HIGH_CONVOLVED_LOW:
		dataType = FeatherDataType::HIGH_CONVOLVED_LOW;
		break;
	case FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED:
		dataType = FeatherDataType::HIGH_CONVOLVED_LOW_WEIGHTED;
		break;
	case FeatherCurveType::DIRTY_ORIGINAL:
		dataType = FeatherDataType::DIRTY;
		break;
	case FeatherCurveType::DIRTY_WEIGHTED:
		dataType = FeatherDataType::DIRTY_WEIGHTED;
		break;
	case FeatherCurveType::DIRTY_CONVOLVED_LOW:
		dataType = FeatherDataType::DIRTY_CONVOLVED_LOW;
		break;
	case FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED:
		dataType = FeatherDataType::DIRTY_CONVOLVED_LOW_WEIGHTED;
		break;
	default:
		//qDebug() << "Unmapped curve type "<<cType;
		break;
	}
	return dataType;
}


FeatherCurveType::CurveType FeatherPlotWidget::getCurveTypeForData( DataType dType ){
	CurveType curveType = FeatherCurveType::WEIGHT_LOW;
	switch( dType ){
	case FeatherDataType::WEIGHT_SD:
		curveType = FeatherCurveType::WEIGHT_LOW;
		break;
	case FeatherDataType::WEIGHT_INT:
		curveType = FeatherCurveType::WEIGHT_HIGH;
		break;
	case FeatherDataType::LOW:
		curveType = FeatherCurveType::LOW_ORIGINAL;
		break;
	case FeatherDataType::LOW_WEIGHTED:
		curveType = FeatherCurveType::LOW_WEIGHTED;
		break;
	case FeatherDataType::LOW_CONVOLVED_HIGH:
		curveType = FeatherCurveType::LOW_CONVOLVED_HIGH;
		break;
	case FeatherDataType::LOW_CONVOLVED_HIGH_WEIGHTED:
		curveType = FeatherCurveType::LOW_CONVOLVED_HIGH_WEIGHTED;
		break;
	/*case FeatherDataType::LOW_CONVOLVED_DIRTY:
		curveType = FeatherCurveType::LOW_CONVOLVED_DIRTY;
		break;
	case FeatherDataType::LOW_CONVOLVED_DIRTY_WEIGHTED:
		curveType = FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED;
		break;*/
	case FeatherDataType::HIGH:
		curveType = FeatherCurveType::HIGH_ORIGINAL;
		break;
	case FeatherDataType::HIGH_WEIGHTED:
		curveType = FeatherCurveType::HIGH_WEIGHTED;
		break;
	case FeatherDataType::HIGH_CONVOLVED_LOW:
		curveType = FeatherCurveType::HIGH_CONVOLVED_LOW;
		break;
	case FeatherDataType::HIGH_CONVOLVED_LOW_WEIGHTED:
		curveType = FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED;
		break;
	case FeatherDataType::DIRTY:
		curveType = FeatherCurveType::DIRTY_ORIGINAL;
		break;
	case FeatherDataType::DIRTY_WEIGHTED:
		curveType = FeatherCurveType::DIRTY_WEIGHTED;
		break;
	case FeatherDataType::DIRTY_CONVOLVED_LOW:
		curveType = FeatherCurveType::DIRTY_CONVOLVED_LOW;
		break;
	case FeatherDataType::DIRTY_CONVOLVED_LOW_WEIGHTED:
		curveType = FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED;
		break;
	default:
		qDebug() << "Unsupported data type "<<dType;
		break;
	}
	return curveType;
}


void FeatherPlotWidget::setData( const Vector<Float>& xValues, const Vector<Float>& yValues, DataType dType ){
	resetData( dType, xValues, yValues );
	CurveType curveType = getCurveTypeForData( dType );
	bool sumCurve = FeatherCurveType::isSumCurve( curveType );
	QwtPlot::Axis axis = getAxisYForData( dType );
	if ( !plot->isScatterPlot() ){
		addPlotCurve( plotData[dType].first, plotData[dType].second, axis,
						curveType, sumCurve );
		plot->replot();
	}
}

void FeatherPlotWidget::clearPlot(){
	removeMarkers();
	plot->clearCurves();
	plot->replot();
}


void FeatherPlotWidget::clearLegend(){
	plot->clearLegend();
}


//-------------------------------------------------------------------------------------
//                             Utility
//-------------------------------------------------------------------------------------


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


pair<double,double> FeatherPlotWidget::getMaxMin( QVector<double> values, FeatherCurveType::CurveType curveType ) const {
	double minValue = numeric_limits<double>::max();
	double maxValue = -1 * numeric_limits<double>::max();
	for ( int i = 0; i < values.size(); i++ ){
		if ( values[i]< minValue ){
			minValue = values[i];
		}
		if ( values[i] > maxValue ){
			maxValue = values[i];
		}
	}

	//For a sum curve, we need to undo the built in log scale.
	if ( FeatherCurveType::isSumCurve( curveType) && plot->isLogAmplitude()){
		minValue = qPow( 10, minValue );
		maxValue = qPow( 10, maxValue );
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
