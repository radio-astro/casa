/*
 * FeatherPlot.cc
 *
 *  Created on: Oct 23, 2012
 *      Author: slovelan
 */

#include "FeatherPlot.h"
#include <QDebug>
#include <QMouseEvent>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>

namespace casa {

FeatherPlot::FeatherPlot(QWidget* parent):QwtPlot( parent ),
	legend( this), legendVisible(true), lineThickness(1), DOT_FACTOR( 3 ){

	this->setAxisAutoScale( QwtPlot::xBottom);

	//Set-up zooming
	zoomer = new QwtPlotZoomer(this->canvas());
	zoomer->setRubberBandPen(QPen(Qt::black, 2, Qt::DotLine));
	zoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::PointSelection );
	zoomer->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton );

	zoomerRight = new QwtPlotZoomer(QwtPlot::xTop, QwtPlot::yRight, this->canvas());
	zoomerRight->setRubberBandPen(QPen(Qt::black, 2, Qt::DotLine));
	zoomerRight->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::PointSelection);
	zoomerRight->setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton );
	zoomerRight->setTrackerMode( QwtPicker::AlwaysOff );

	//Panning with Ctrl+Left
	QwtPlotPanner* panner = new QwtPlotPanner( this->canvas() );
	panner->setMouseButton( Qt::LeftButton, Qt::ControlModifier );
}

/*void FeatherPlot::zoomNeutral(){
	qDebug() << "Neutral zoom";
	//int zoomIndex = zoomer->
	zoomer->zoom( 0 );
	zoomerRight->zoom( 0 );
	//replot();
}*/
FeatherPlot::PlotType FeatherPlot::getPlotType() const{
	return plotType;
}
void FeatherPlot::initializePlot( const QString& graphTitle, PlotType plotType ){
	this->plotType = plotType;
	insertLegend(&legend, QwtPlot::BottomLegend );
	setCanvasBackground( Qt::white );
	setTitle( graphTitle );
	const QString AMPLITUDE = "Amplitude";
	const QString DISTANCE = "Distance(m)";
	QString xAxisTitle;
	QString yAxisLeftTitle;
	QString yAxisRightTitle;
	if ( plotType==SCATTER_PLOT ){
		xAxisTitle = "Single Dish " + AMPLITUDE;
		yAxisLeftTitle = "Interferometer "+AMPLITUDE;
	}
	else {
		xAxisTitle = DISTANCE;
		yAxisLeftTitle = "Weight "+AMPLITUDE;
		yAxisRightTitle = "Slice Cut "+AMPLITUDE;
	}
	setAxisTitle( QwtPlot::yLeft, yAxisLeftTitle );
	setAxisTitle( QwtPlot::xBottom, xAxisTitle );
	if ( plotType==SLICE_CUT ){
		setAxisTitle( QwtPlot::yRight, yAxisRightTitle );
		//setAxisAutoScale( QwtPlot::yRight);
	}

	QwtText axisText = axisTitle( QwtPlot::xBottom );
	QFont defaultFont = axisText.font();
	defaultFont.setPointSize( 9 );
	axisText.setFont( defaultFont );
	setAxisTitle( QwtPlot::xBottom, axisText );

	QwtText axisTitleY = axisTitle( QwtPlot::yLeft );
	axisTitleY.setFont( defaultFont );
	setAxisTitle( QwtPlot::yLeft, axisTitleY );

	if ( plotType==SLICE_CUT){
		QwtText axisTitleY2 = axisTitle( QwtPlot::yRight );
		axisTitleY2.setFont( defaultFont );
		setAxisTitle( QwtPlot::yRight, axisTitleY2 );
	}

	QwtText titleText = title();
	QFont titleFont = titleText.font();
	titleFont.setPointSize(10);
	titleText.setFont( titleFont );
	setTitle( titleText );
	bool useRightYAxis = false;
	if ( plotType == FeatherPlot::SLICE_CUT ){
		useRightYAxis = true;
	}
	enableAxis( QwtPlot::yRight, useRightYAxis );
}

void FeatherPlot::clearCurves(){
	int count = curves.size();
	for( int i = 0; i < count; i++ ){
		QwtPlotCurve* curve = curves[i];
		curve->detach();
		delete curve;
	}
	curves.clear();
	qDebug() << "Number of curves is "<< curves.size();
}

void FeatherPlot::setCurveLineThickness( int curveIndex ){
	QPen curvePen = curves[curveIndex]->pen();
	QString curveTitle = curves[curveIndex]->title().text();
	if ( curveTitle.length() > 0 ){
		curvePen.setWidth( lineThickness );
	}
	else {
		curvePen.setWidth( lineThickness * DOT_FACTOR );
	}
	curves[curveIndex]->setPen( curvePen );
}
void FeatherPlot::setLineThickness( int thickness ){
	lineThickness = thickness;
	for ( int i = 0; i < curves.size(); i++ ){
		setCurveLineThickness( i );
	}
}

void FeatherPlot::setLegendVisibility( bool visible ){
	legendVisible = visible;
	legend.contentsWidget()->setVisible( visible );
}

void FeatherPlot::setFunctionColor( const QString& curveID, const QColor& color ){
	int curveIndex = getCurveIndex( curveID );
	qDebug() << "curveIndex="<<curveIndex<<" for curve "<<curveID;
	if ( curveIndex >= 0 ){
		QPen curvePen = curves[curveIndex]->pen();
		qDebug() << "Setting color "<<color.name();
		curvePen.setColor( color );
		curves[curveIndex]->setPen( curvePen );
	}
}

int FeatherPlot::getCurveIndex( const QString& curveTitle ) const {
	int curveIndex = -1;
	for ( int i = 0; i < curves.size(); i++ ){
		QString existingTitle = curves[i]->title().text();
		if ( existingTitle == curveTitle ){
			curveIndex = i;
			break;
		}
	}
	return curveIndex;
}

void FeatherPlot::addCurve( QVector<double> xValues, QVector<double> yValues,
		QColor curveColor, const QString& curveTitle, QwtPlot::Axis yAxis ){

	//See if we already have the curve
	int curveIndex = getCurveIndex( curveTitle);

	//Set the data into the curve
	QwtPlotCurve* curve = NULL;
	//We need to make a new curve
	if ( curveIndex < 0 ){
	    curve  = new QwtPlotCurve();


		curve->setTitle( curveTitle );
		curve->setAxis( QwtPlot::xBottom, yAxis );
		curve->attach( this );
		if ( curveTitle.length() == 0 ){
			curve->setStyle( QwtPlotCurve::Dots );
		}
	}
	//We are just going to change the data in an existing curve
	else {
		curve = curves[curveIndex];
	}
	curve->setData( xValues, yValues );

	//Store the curve if it is not already there
	if ( curveIndex < 0 ){
		curves.append( curve );
		int index = curves.size() - 1;
		setFunctionColor( curveTitle, curveColor );
		setCurveLineThickness( index );
	}

	//Add the curve adds a legend item.  In the case of scatter plots,
	//we don't want it.
	//qDebug() <<"Add curve visibility="<<legendVisible;
	//this->setLegendVisibility( legendVisible );
}

FeatherPlot::~FeatherPlot() {
	//clearCurves();
}
} /* namespace casa */
