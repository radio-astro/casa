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
namespace casa {
FeatherPlotWidget::FeatherPlotWidget(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent)
    : QWidget(parent), plot( NULL ),
      singleDishWeightColor(Qt::cyan), singleDishDataColor(Qt::green),
      interferometerWeightColor( Qt::magenta), interferometerDataColor(Qt::blue),

      singleDishFunction( "Low Resolution Slice"), interferometerFunction( "High Resolution Slice"),
      singleDishWeightFunction("Low Resolution Weight"), interferometerWeightFunction( "High Resolution Weight"),
      legendVisible( true ), permanentScatter( false ), lineThickness(1), plotTitle(title){

	ui.setupUi(this);

	sliceAxis = QwtPlot::yLeft;
	weightAxis = QwtPlot::yRight;

	resetPlot( plotType );

	zoomer = NULL;
	initializeZooming();
}

void FeatherPlotWidget::changePlotType( FeatherPlot::PlotType newPlotType ){
	//Scatter plots do not change type.
	if ( !permanentScatter ){
		resetPlot( newPlotType );
		if ( newPlotType ==FeatherPlot::SCATTER_PLOT ){
			addScatterData();
		}
		else {
			addZoomNeutralCurves();
		}
	}
}

void FeatherPlotWidget::setLineThickness( int thickness ){
	lineThickness = thickness;
	plot->setLineThickness( thickness );
	plot->replot();
}

void FeatherPlotWidget::setDishDiameter( double value ){
	plot->setDiameterPosition( value );
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
		const QColor& scatterColor, const QColor& dishDiameterColor ){
	singleDishDataColor = colorMap[PreferencesColor::SD_SLICE_COLOR];
	interferometerDataColor = colorMap[PreferencesColor::INT_SLICE_COLOR];
	singleDishWeightColor = colorMap[PreferencesColor::SD_WEIGHT_COLOR];
	interferometerWeightColor = colorMap[PreferencesColor::INT_WEIGHT_COLOR];
	scatterPlotColor = scatterColor;
	dishDiameterLineColor = dishDiameterColor;
	resetColors();
}
void FeatherPlotWidget::resizeEvent( QResizeEvent* event ){
	QWidget::resizeEvent( event );
	plot->setDiameterPosition();
}

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
		zoomer->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton );
		connect( zoomer, SIGNAL(selected ( const QwtDoubleRect& )), this, SLOT(zoomRectangleSelected( const QwtDoubleRect& )));
	}
}

void FeatherPlotWidget::zoomRectangleSelected( const QwtDoubleRect& zoomRect ){
	double firstPoint = zoomRect.x();
	double secondPoint = firstPoint + zoomRect.width();

	//If we are a scatter plot, we don't have to let anyone know about
	//the zoom, just update our own coordinates.
	if ( plot->getPlotType() == FeatherPlot::SCATTER_PLOT ){

	}
	//Tell the plot holder about the zoom so all the plots can zoom in sync.
	else {
		emit rectangleZoomed( firstPoint, secondPoint );
	}
}


void FeatherPlotWidget::mouseReleaseEvent( QMouseEvent* event ){
	//Check to see if the mouse release represents a change in the dish
	//diameter.
	Qt::KeyboardModifiers modifiers = event->modifiers();
	bool diameterPlacement = false;
	if ( modifiers & Qt::ShiftModifier ){
		diameterPlacement = true;
	}
	else if ( plot->isDiameterSelectorMode() ){
		diameterPlacement = true;
	}

	//We could be changing the dish diameter.  If so, let all the plots
	//know about the change by percolating the event upward.
	if ( diameterPlacement ){
		bool diameterMoved = plot->moveDiameterMarker( event->pos() );
		if ( diameterMoved ){
			double dishDiameter = plot->getDishDiameter();
			if ( dishDiameter >= 0 ){
				emit dishDiameterChanged( dishDiameter);
			}
		}
	}
	QWidget::mouseReleaseEvent( event );
}

void FeatherPlotWidget::resetColors(){
	FeatherPlot::PlotType plotType = plot->getPlotType();
	if ( plotType == FeatherPlot::SLICE_CUT ){
		plot->setFunctionColor( singleDishFunction, singleDishDataColor );
		plot->setFunctionColor( interferometerFunction, interferometerDataColor );
		plot->setFunctionColor( singleDishWeightFunction, singleDishWeightColor );
		plot->setFunctionColor( interferometerWeightFunction, interferometerWeightColor );
	}
	else if ( plotType == FeatherPlot::SCATTER_PLOT ){
		plot->setFunctionColor( "", scatterPlotColor );
	}
	else {
		plot->setFunctionColor( this->singleDishFunction, singleDishDataColor );
		plot->setFunctionColor( this->interferometerFunction, interferometerDataColor );
	}
	plot->setDishDiameterLineColor( dishDiameterLineColor );
	plot->replot();
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

void FeatherPlotWidget::zoomRectangle( double minX, double maxX ){
	QVector<double> singleDishX;
	QVector<double> singleDishY;
	initializeDomainLimitedData( minX, maxX, singleDishX, singleDishY, singleDishDataXValues, singleDishDataYValues );
	plot->addCurve( singleDishX, singleDishY, singleDishDataColor, singleDishFunction, sliceAxis );

	QVector<double> interferometerX;
	QVector<double> interferometerY;
	initializeDomainLimitedData( minX, maxX, interferometerX, interferometerY, interferometerDataXValues, interferometerDataYValues );
	plot->addCurve( interferometerX, interferometerY, interferometerDataColor, interferometerFunction, sliceAxis );

	if ( plot->getPlotType() == FeatherPlot::SLICE_CUT ){
		QVector<double> singleDishWeightX;
		QVector<double> singleDishWeightY;
		initializeDomainLimitedData( minX, maxX, singleDishWeightX, singleDishWeightY, singleDishWeightXValues, singleDishWeightYValues );
		plot->addCurve( singleDishWeightX, singleDishWeightY, singleDishWeightColor, singleDishWeightFunction, weightAxis );

		QVector<double> interferometerWeightX;
		QVector<double> interferometerWeightY;
		initializeDomainLimitedData( minX, maxX, interferometerWeightX, interferometerWeightY, interferometerWeightXValues, interferometerWeightYValues );
		plot->addCurve( interferometerWeightX, interferometerWeightY, interferometerWeightColor, interferometerWeightFunction, weightAxis );

	}
	plot->replot();
}

/*void FeatherPlotWidget::zoomRectangleScatter( double minX, double maxX, double minY, double maxY ){
	int count = qMin( singleDishDataYValues.size(), interferometerDataYValues.size());
	int dataCount = 0;
	for ( int i = 0; i < count; i++ ){
		if ( minX <= singleDishDataYValues[i] && singleDishDataYValues[i] <= maxX ){
			if ( minY <= interferometerDataYValues[i] && interferometerDataYValues[i] <= maxY ){
				dataCount++;
			}
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
	plot->addCurve( singleDishDataYValues, interferometerDataYValues, scatterPlotColor, "", sliceAxis );
}*/

void FeatherPlotWidget::changeZoom90(bool zoom ){
	if ( zoom ){

		//Calculate the yValues that represent 90% of the amplitude.
		pair<double,double> singleDishMinMax = getMaxMin( singleDishWeightYValues );
		pair<double,double> interferometerMinMax = getMaxMin( interferometerWeightYValues );
		double minValue = qMin( singleDishMinMax.first, interferometerMinMax.first );
		double maxValue = qMax( singleDishMinMax.second, interferometerMinMax.second );
		double increase = (maxValue - minValue) * .25;
		minValue = minValue + increase;
		maxValue = maxValue - increase;

		FeatherPlot::PlotType plotType = plot->getPlotType();

		//Reset the weight values to 90% of the yRange while figuring out the
		//xcut off values minX and maxX.
		Double minX = numeric_limits<double>::max();
		Double maxX = numeric_limits<double>::min();
		QVector<double> singleDishXZoom;
		QVector<double> singleDishYZoom;
		initializeRangeLimitedData( minValue, maxValue, singleDishXZoom, singleDishYZoom,
				singleDishWeightXValues, singleDishWeightYValues, &minX, &maxX );
		if ( plotType == FeatherPlot::SLICE_CUT ){
			plot->addCurve( singleDishXZoom, singleDishYZoom, singleDishWeightColor, singleDishWeightFunction, weightAxis );
		}

		QVector<double> interferometerXZoom;
		QVector<double> interferometerYZoom;
		initializeRangeLimitedData( minValue, maxValue, interferometerXZoom, interferometerYZoom,
					interferometerWeightXValues, interferometerWeightYValues, &minX, &maxX );
		if ( plotType == FeatherPlot::SLICE_CUT ){
			plot->addCurve( interferometerXZoom, interferometerYZoom, interferometerWeightColor, interferometerWeightFunction, weightAxis );
		}

		//Use the xbounds to redo the slice cut data.
		QVector<double> singleDishX;
		QVector<double> singleDishY;
		initializeDomainLimitedData( minX, maxX, singleDishX, singleDishY, singleDishDataXValues, singleDishDataYValues );
		plot->addCurve( singleDishX, singleDishY, singleDishDataColor, singleDishFunction, sliceAxis );

		QVector<double> interferometerX;
		QVector<double> interferometerY;
		initializeDomainLimitedData( minX, maxX, interferometerX, interferometerY, interferometerDataXValues, interferometerDataYValues );
		plot->addCurve( interferometerX, interferometerY, interferometerDataColor, interferometerFunction, sliceAxis );
		plot->replot();
	}
	else {
		zoomNeutral();
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

void FeatherPlotWidget::zoomNeutral(){
	FeatherPlot::PlotType originalPlotType = plot->getPlotType();
	resetPlot( originalPlotType );
	if ( originalPlotType == FeatherPlot::SLICE_CUT ||
			originalPlotType == FeatherPlot::ORIGINAL ){
		addZoomNeutralCurves();
	}
	else {
		addScatterData();
	}
}

void FeatherPlotWidget::addScatterData(){
	plot->addCurve( singleDishDataYValues, interferometerDataYValues, scatterPlotColor, "", sliceAxis );
	plot->replot();
}

void FeatherPlotWidget::addZoomNeutralCurves(){
	FeatherPlot::PlotType plotType = plot->getPlotType();
	if ( plotType != FeatherPlot::ORIGINAL ){
		plot->addCurve( singleDishWeightXValues, singleDishWeightYValues, singleDishWeightColor, singleDishWeightFunction, weightAxis );
		plot->addCurve( interferometerWeightXValues, interferometerWeightYValues, interferometerWeightColor, interferometerWeightFunction, weightAxis );
	}
	plot->addCurve( singleDishDataXValues, singleDishDataYValues, singleDishDataColor, singleDishFunction, sliceAxis );
	plot->addCurve( interferometerDataXValues, interferometerDataYValues, interferometerDataColor, interferometerFunction, sliceAxis );
	plot->replot();
}

QWidget* FeatherPlotWidget::getExternalAxisWidget( QwtPlot::Axis position ){
	return plot->getExternalAxisWidget( position );
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

void FeatherPlotWidget::insertLegend( QWidget* parent ){
	plot->insertSingleLegend( parent );
}

void FeatherPlotWidget::setSingleDishWeight( const Vector<Float>& xValues, const Vector<Float>& yValues ){
	int count = qMin( xValues.size(), yValues.size() );
	singleDishWeightXValues.resize( count );
	singleDishWeightYValues.resize( count );
	for ( int i = 0; i < count; i++ ){
		singleDishWeightXValues[i] = xValues[i];
		singleDishWeightYValues[i] = yValues[i];
	}
	FeatherPlot::PlotType plotType = plot->getPlotType();
	if ( plotType == FeatherPlot::SLICE_CUT ){
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
	plot->addCurve( singleDishDataXValues, singleDishDataYValues, singleDishDataColor, singleDishFunction, sliceAxis );
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
	FeatherPlot::PlotType plotType = plot->getPlotType();
	if ( plotType == FeatherPlot::SLICE_CUT ){
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

	plot->addCurve( interferometerDataXValues, interferometerDataYValues, interferometerDataColor, interferometerFunction, sliceAxis );
	plot->replot();
}


FeatherPlotWidget::~FeatherPlotWidget(){
	//delete plot;
}
}
