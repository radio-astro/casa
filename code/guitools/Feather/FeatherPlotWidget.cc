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

namespace casa {

FeatherPlotWidget::FeatherPlotWidget(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent)
    : QWidget(parent), plot( NULL ),
      singleDishWeightColor(Qt::darkYellow), singleDishDataColor(Qt::green), sumDataColor( Qt::cyan ),
      interferometerWeightColor( Qt::magenta), interferometerDataColor(Qt::blue),
      singleDishFunction( "Low Resolution Slice"), interferometerFunction( "High Resolution Slice"),
      singleDishWeightFunction("Low Resolution Weight"), interferometerWeightFunction( "High Resolution Weight"),
      sumFunction( "Sum Slice"),
      permanentScatter( false ),
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
	double markerPosition = diameterMarker->xValue();
	bool scaleChanged = plot->setLogScale( uvScale, ampScale );
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


void FeatherPlotWidget::setPermanentScatter( bool scatter ){
	permanentScatter = scatter;
}


void FeatherPlotWidget::setPlotColors( const QMap<PreferencesColor::FunctionColor,QColor>& colorMap,
		const QColor& scatterColor, const QColor& dishDiameterColor,
		const QColor& zoomRectangleColor, const QColor& sumSliceColor ){
	singleDishDataColor = colorMap[PreferencesColor::SD_SLICE_COLOR];
	interferometerDataColor = colorMap[PreferencesColor::INT_SLICE_COLOR];
	singleDishWeightColor = colorMap[PreferencesColor::SD_WEIGHT_COLOR];
	interferometerWeightColor = colorMap[PreferencesColor::INT_WEIGHT_COLOR];
	scatterPlotColor = scatterColor;
	dishDiameterLineColor = dishDiameterColor;
	zoomRectColor = zoomRectangleColor;
	sumDataColor = sumSliceColor;
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
		plot->setFunctionColor( sumFunction, sumDataColor );
	}
	else if ( plot->isScatterPlot() ){
		plot->setFunctionColor( "", scatterPlotColor );
		plot->setFunctionColor( FeatherPlot::Y_EQUALS_X, dishDiameterLineColor );
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
		pen.setColor( dishDiameterLineColor );
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
			zoomRectangleScatter( minX, maxX, minY, maxY );
		}
		//Tell the plot holder about the zoom so all the plots can zoom in sync.
		else {
			emit rectangleZoomed( minX, maxX, minY, maxY );
		}
	}
}


void FeatherPlotWidget::zoomRectangleWeight( double minX, double maxX  ){

	QVector<double> singleDishWeightX;
	QVector<double> singleDishWeightY;
	initializeDomainLimitedData( minX, maxX, singleDishWeightX, singleDishWeightY, singleDishWeightXValues, singleDishWeightYValues );
	plot->addCurve( singleDishWeightX, singleDishWeightY, singleDishWeightColor, singleDishWeightFunction, weightAxis );

	QVector<double> interferometerWeightX;
	QVector<double> interferometerWeightY;
	initializeDomainLimitedData( minX, maxX, interferometerWeightX, interferometerWeightY, interferometerWeightXValues, interferometerWeightYValues );
	plot->addCurve( interferometerWeightX, interferometerWeightY, interferometerWeightColor, interferometerWeightFunction, weightAxis );
}

void FeatherPlotWidget::zoomRectangle( double minX, double maxX, double /*minY*/, double /*maxY*/ ){
	plot->clearCurves();
	if ( plot->isSliceCut() ){
		zoomRectangleWeight( minX, maxX );
	}
	QVector<double> singleDishX;
	QVector<double> singleDishY;
	initializeDomainLimitedData( minX, maxX, singleDishX, singleDishY, singleDishDataXValues, singleDishDataYValues );
	plot->addCurve( singleDishX, singleDishY, singleDishDataColor, singleDishFunction, sliceAxis );

	QVector<double> interferometerX;
	QVector<double> interferometerY;
	initializeDomainLimitedData( minX, maxX, interferometerX, interferometerY, interferometerDataXValues, interferometerDataYValues );
	plot->addCurve( interferometerX, interferometerY, interferometerDataColor, interferometerFunction, sliceAxis );

	//Sum curve
	QVector<double> sumX;
	QVector<double> sumY;
	initializeSumData( singleDishX, singleDishY, interferometerX, interferometerY, sumX, sumY );
	plot->addCurve( sumX, sumY, sumDataColor, sumFunction, sliceAxis );
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
	plot->addCurve( xValues, yValues, scatterPlotColor, "", scatterAxis );
	plot->addDiagonal( xValues, dishDiameterLineColor, scatterAxis );
	plot->replot();
}


void FeatherPlotWidget::changeZoom90(bool zoom ){
	if ( zoom ){
		plot->clearCurves();

		//See if we know the dish position.  We want to take the dish position,
		//plus another 1/3 of it, if it is available.  If it is not, we will just
		//zoom in on the x-axis by 1/3.
		double dishPosition = 0;
		if ( diameterMarker != NULL ){
			dishPosition = diameterMarker->xValue();
			dishPosition = dishPosition + dishPosition / 3;
		}
		if ( dishPosition == 0 ){
			pair<double,double> minMaxPair = getMaxMin( singleDishDataYValues );
			dishPosition = minMaxPair.second / 3;
		}

		//If we are using a log scale on the x-axis, we need to undo the log.
		if ( plot->isLogUV() ){
			dishPosition = qPow( 10, dishPosition );
		}

		//Add in the zoomed weight functions
		if ( plot->isSliceCut() ){
			QVector<double> singleDishWeightZoomX;
			QVector<double> singleDishWeightZoomY;
			for ( int i = 0; i < singleDishWeightXValues.size(); i++ ){
				if ( singleDishWeightXValues[i] < dishPosition ){
					singleDishWeightZoomX.append( singleDishWeightXValues[i] );
					singleDishWeightZoomY.append( singleDishWeightYValues[i] );
				}
			}
			QVector<double> interferometerWeightZoomX;
			QVector<double> interferometerWeightZoomY;
			for ( int i = 0; i < interferometerWeightXValues.size(); i++ ){
				if ( interferometerWeightXValues[i] < dishPosition ){
					interferometerWeightZoomX.append( interferometerWeightXValues[i] );
					interferometerWeightZoomY.append( interferometerWeightYValues[i] );
				}
			}
			plot->addCurve( singleDishWeightZoomX, singleDishWeightZoomY, singleDishWeightColor, singleDishWeightFunction, weightAxis );
			plot->addCurve( interferometerWeightZoomX, interferometerWeightZoomY, interferometerWeightColor, interferometerWeightFunction, weightAxis );
		}

		//Now take all the x,y values where the x values is less than the dish position.
		QVector<double> singleDishZoomDataX;
		QVector<double> singleDishZoomDataY;
		for ( int i = 0; i < static_cast<int>(singleDishDataXValues.size()); i++ ){
			if ( singleDishDataXValues[i] < dishPosition ){
				singleDishZoomDataX.append( singleDishDataXValues[i] );
				singleDishZoomDataY.append( singleDishDataYValues[i] );
			}
		}
		QVector<double> interferometerZoomDataX;
		QVector<double> interferometerZoomDataY;
		for ( int i = 0; i < interferometerDataXValues.size(); i++ ){
			if ( interferometerDataXValues[i] < dishPosition ){
				interferometerZoomDataX.append( interferometerDataXValues[i] );
				interferometerZoomDataY.append( interferometerDataYValues[i] );
			}
		}
		if ( !plot->isScatterPlot() ){
			plot->addCurve( singleDishZoomDataX, singleDishZoomDataY, singleDishDataColor, singleDishFunction, sliceAxis );
			plot->addCurve( interferometerZoomDataX, interferometerZoomDataY, interferometerDataColor, interferometerFunction, sliceAxis );

			//Sum curve
			QVector<double> sumX;
			QVector<double> sumY;
			initializeSumData( singleDishZoomDataX, singleDishZoomDataY,
						interferometerZoomDataX, interferometerZoomDataY, sumX, sumY );
			plot->addCurve( sumX, sumY, sumDataColor, sumFunction, sliceAxis );
		}
		else {
			//The plot needs to have the same values in both directions.
			pair<double,double> singleDishMinMax = getMaxMin( singleDishZoomDataY );
			pair<double,double> interferometerMinMax = getMaxMin( interferometerZoomDataY );
			double valueMax = qMin( singleDishMinMax.second, interferometerMinMax.second);
			double valueMin = qMax( singleDishMinMax.first, interferometerMinMax.first );
			QVector<double> restrictX;
			QVector<double> restrictY;
			for ( int i = 0; i < singleDishZoomDataY.size(); i++ ){
				if ( valueMin <= singleDishZoomDataY[i] && singleDishZoomDataY[i]<= valueMax ){
					if ( valueMin <= interferometerZoomDataY[i] && interferometerZoomDataY[i] <= valueMax ){
						restrictX.append( singleDishZoomDataY[i]);
						restrictY.append( interferometerZoomDataY[i]);
					}
				}
			}
			plot->addCurve( restrictX, restrictY, scatterPlotColor, "", scatterAxis );
			restrictX.append( valueMax );
			plot->addDiagonal( restrictX, dishDiameterLineColor, scatterAxis );
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

	//Sum curve
	addSumData();

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

void FeatherPlotWidget::initializeSumData( const QVector<double>& singleDishX, const QVector<double>& singleDishY,
		const QVector<double>& interferometerX, const QVector<double>& interferometerY,
		QVector<double>& sumX, QVector<double>& sumY ){
	int countX = qMin( singleDishX.size(), interferometerX.size() );
	sumX.resize( countX );
	sumY.resize( countX );
	for ( int i = 0; i < countX; i++ ){
		sumX[i] = singleDishX[i];
		sumY[i] = singleDishY[i] + interferometerY[i];
	}
}


void FeatherPlotWidget::removeMarkers(){
	diameterMarker->detach();
	diameterSelector->setMousePattern( QwtEventPattern::MouseSelect1, Qt::NoButton);
	zoomer->setMousePattern( QwtEventPattern::MouseSelect1, Qt::NoButton );
}


void FeatherPlotWidget::addScatterData(){
	//The scatter plot data needs to be on the same scale.
	//Find the max value of each data set, and choose the smallest of the
	//two upper bounds as the limit.
	pair<double,double> singleDishMinMax = getMaxMin( singleDishDataYValues );
	pair<double,double> interferometerMinMax = getMaxMin( interferometerDataYValues );
	double valueLimit = qMin( singleDishMinMax.second, interferometerMinMax.second );
	QVector<double> scatterXValues;
	QVector<double> scatterYValues;
	double actualLimit = numeric_limits<double>::min();
	for ( int i = 0; i < singleDishDataYValues.size(); i++ ){
		if ( singleDishDataYValues[i] <= valueLimit && interferometerDataYValues[i] <= valueLimit ){
			scatterXValues.append( singleDishDataYValues[i]);
			scatterYValues.append( interferometerDataYValues[i]);
			if ( singleDishDataYValues[i] > actualLimit ){
				actualLimit = singleDishDataYValues[i];
			}
			if ( interferometerDataYValues[i] > actualLimit ){
				actualLimit = interferometerDataYValues[i];
			}
		}
	}

	plot->addCurve( scatterXValues, scatterYValues, scatterPlotColor, "", scatterAxis );
	scatterXValues.append(actualLimit);
	plot->addDiagonal( scatterXValues, dishDiameterLineColor, scatterAxis );
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

void FeatherPlotWidget::addSumData(){
	QVector<double> sumX;
	QVector<double> sumY;
	initializeSumData( singleDishDataXValues, singleDishDataYValues,
			interferometerDataXValues, interferometerDataYValues, sumX, sumY );
	if ( sumX.size() > 0 ){
		plot->addCurve( sumX, sumY, sumDataColor, sumFunction, sliceAxis );
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


pair<double,double> FeatherPlotWidget::getMaxMin( QVector<double> values ) const {
	double minValue = numeric_limits<double>::max();
	double maxValue = numeric_limits<double>::min();
	for ( int i = 0; i < values.size(); i++ ){
		if ( values[i]< minValue ){
			minValue = values[i];
		}
		if ( values[i] > maxValue ){
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
