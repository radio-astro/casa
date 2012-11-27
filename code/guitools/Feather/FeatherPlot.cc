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

#include "FeatherPlot.h"
#include <guitools/Feather/ExternalAxisWidget.h>
#include <guitools/Feather/ExternalAxis.h>
#include <guitools/Feather/ExternalAxisWidgetLeft.h>
#include <guitools/Feather/ExternalAxisWidgetRight.h>
#include <guitools/Feather/ExternalAxisWidgetBottom.h>
#include <QDebug>
#include <QHBoxLayout>
#include <qwt_plot_curve.h>
//#include <qwt_plot_panner.h>
#include <qwt_symbol.h>
#include <qwt_scale_draw.h>

namespace casa {

FeatherPlot::FeatherPlot(QWidget* parent):QwtPlot( parent ),
	legend( NULL), lineThickness(1),
	dotSize(1), AXIS_COUNT(3){

	this->setAxisAutoScale( QwtPlot::xBottom );
	for ( int i = 0; i < AXIS_COUNT; i++ ){
		axisBlanks.append( new ExternalAxis() );
		axisWidgets.append( NULL );
		axisLabels.append( "" );
	}

	//Panning with Ctrl+Left
	/*QwtPlotPanner* panner = new QwtPlotPanner( this->canvas() );
	panner->setMouseButton( Qt::LeftButton, Qt::ControlModifier );
	*/
	setCanvasBackground( Qt::white );
}

void FeatherPlot::initializePlot( const QString& graphTitle, PlotType plotType ){

	//Store what type of plot we are.
	this->plotType = plotType;

	//Set the title of the plot
	setTitle( graphTitle );
	QwtText titleText = title();
	QFont titleFont = titleText.font();
	titleFont.setPointSize(10);
	titleText.setFont( titleFont );
	setTitle( titleText );

	//Set up the axis scales and labels
	setAxisLabels();
	setAxisScaleDraw( QwtPlot::xBottom, axisBlanks[QwtPlot::xBottom] );
	setAxisScaleDraw( QwtPlot::yLeft, axisBlanks[QwtPlot::yLeft] );
	bool useRightYAxis = false;
	if ( plotType == FeatherPlot::SLICE_CUT ){
		useRightYAxis = true;
		setAxisScaleDraw( QwtPlot::yRight, axisBlanks[QwtPlot::yRight] );
	}
	enableAxis( QwtPlot::yRight, useRightYAxis );
}

void FeatherPlot::setAxisLabels(){
	const QString AMPLITUDE = "Amplitude";
	const QString DISTANCE = "Distance(m)";
	if ( plotType==SCATTER_PLOT ){
		axisLabels[QwtPlot::xBottom] = "Low Resolution " + AMPLITUDE;
		axisLabels[QwtPlot::yLeft] = "High Resolution "+AMPLITUDE;
	}
	else {
		axisLabels[QwtPlot::xBottom] = DISTANCE;
		axisLabels[QwtPlot::yLeft] = "Slice "+AMPLITUDE;
		axisLabels[QwtPlot::yRight] = "Weight "+AMPLITUDE;
	}
}

FeatherPlot::~FeatherPlot() {
	for ( int i = 0; i < axisWidgets.size(); i++ ){
		delete axisWidgets[i];
		axisWidgets[i] = NULL;
	}
	delete legend;
	legend = NULL;
}

//--------------------------------------------------------------------------
//                     External Axis
//-------------------------------------------------------------------------

QWidget* FeatherPlot::getExternalAxisWidget( QwtPlot::Axis position ){
	QWidget* axisWidget = NULL;
	if ( position < AXIS_COUNT ){
		if ( axisWidgets[position] == NULL ){
			if ( position == QwtPlot::xBottom ){
				axisWidgets[position] = new ExternalAxisWidgetBottom( NULL );
			}
			else if ( position == QwtPlot::yLeft ){
				if ( isScatterPlot() ){
					axisWidgets[position] = new ExternalAxisWidgetLeft( NULL);
				}
				else {
					axisWidgets[position] = new ExternalAxisWidgetLeft( NULL );
				}
			}
			else {
				axisWidgets[position] = new ExternalAxisWidgetRight( NULL );
			}
			axisWidgets[position]->setPlotCanvas( canvas() );
			axisWidgets[position]->setScaleDiv( axisScaleDiv( position ));
			axisWidgets[position]->setAxisLabel( axisLabels[position] );
		}
		axisWidget = axisWidgets[position];
	}
	else {
		qDebug() << "Unsupported axis position"<<position;
	}
	return axisWidget;
}

void FeatherPlot::updateAxes(){
	for ( int i = 0; i < AXIS_COUNT; i++ ){
		if ( axisWidgets[i] != NULL ){
			axisWidgets[i]->setAxisLabel( axisLabels[i] );
			axisWidgets[i]->update();
		}
	}
}

//-----------------------------------------------------------------
//                        Legend
//-----------------------------------------------------------------

void FeatherPlot::clearLegend(){
	if ( legend != NULL ){
		insertLegend( NULL, QwtPlot::ExternalLegend );
		delete legend;
		legend = NULL;
	}
}

void FeatherPlot::insertSingleLegend( QWidget* parent ){
	if ( legend == NULL ){
		legend = new QwtLegend( );
	}
	insertLegend( legend, QwtPlot::ExternalLegend );
	if ( parent != NULL ){
		QLayout* parentLayout = parent->layout();
		if ( parentLayout == NULL ){
			parentLayout = new QHBoxLayout();
		}
		parentLayout->addWidget( legend );
		parent->setLayout( parentLayout );
	}
}

void FeatherPlot::setLegendVisibility( bool visible ){
	bool legendVisible = visible && !isScatterPlot();
	if ( legend != NULL ){
		legend->setVisible( legendVisible );
		update();
	}
}

//---------------------------------------------------------------------------
//                        Accessors
//---------------------------------------------------------------------------

FeatherPlot::PlotType FeatherPlot::getPlotType() const{
	return plotType;
}

bool FeatherPlot::isEmpty() const {
	bool empty = false;
	if ( curves.size() == 0 ){
		empty = true;
	}
	return empty;
}

bool FeatherPlot::isScatterPlot() const{
	bool scatterPlot = false;
	if ( plotType == SCATTER_PLOT ){
		scatterPlot = true;
	}
	return scatterPlot;
}

bool FeatherPlot::isSliceCut() const {
	bool sliceCut = false;
	if ( plotType == SLICE_CUT ){
		sliceCut = true;
	}
	return sliceCut;
}

bool FeatherPlot::isSliceCutOriginal() const {
	bool sliceCut = false;
	if ( plotType == ORIGINAL ){
		sliceCut = true;
	}
	return sliceCut;
}
//--------------------------------------------------------------------------
//                      Curves
//--------------------------------------------------------------------------

void FeatherPlot::setCurveSize( int curveIndex ){
	QPen curvePen = curves[curveIndex]->pen();
	QString curveTitle = curves[curveIndex]->title().text();
	if ( isScatterPlot() ){
		QwtSymbol symbol = curves[curveIndex]->symbol();
		symbol.setSize( dotSize, dotSize );
		curves[curveIndex]->setSymbol( symbol );
	}
	else {
		curvePen.setWidth( lineThickness );
		curves[curveIndex]->setPen( curvePen );
	}
}

void FeatherPlot::setLineThickness( int thickness ){
	lineThickness = thickness;
	for ( int i = 0; i < curves.size(); i++ ){
		setCurveSize( i );
	}
}

void FeatherPlot::setDotSize( int size ){
	dotSize = size;
	for ( int i = 0; i < curves.size(); i++ ){
		setCurveSize( i );
	}
}

void FeatherPlot::clearCurves(){
	int count = curves.size();
	for( int i = 0; i < count; i++ ){
		QwtPlotCurve* curve = curves[i];
		curve->detach();
		delete curve;
		curves[i] = NULL;
	}
	curves.clear();
}

void FeatherPlot::setFunctionColor( const QString& curveID, const QColor& color ){
	int curveIndex = getCurveIndex( curveID );
	if ( curveIndex >= 0 ){
		if ( !isScatterPlot() ){
			QPen curvePen = curves[curveIndex]->pen();
			curvePen.setColor( color );
			curves[curveIndex]->setPen( curvePen );
		}
		else {
			QwtSymbol curveSymbol = curves[curveIndex]->symbol();
			QPen curvePen = curveSymbol.pen();
			curvePen.setColor( color );
			curveSymbol.setPen( curvePen );
			curves[curveIndex]->setSymbol( curveSymbol );
		}
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
		if ( isScatterPlot() ){
			curve->setStyle( QwtPlotCurve::Dots );
			QwtSymbol* symbol = new QwtSymbol();
			symbol->setSize( dotSize, dotSize );
			symbol->setStyle( QwtSymbol::XCross );
			curve->setSymbol( *symbol );
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
		setCurveSize( index );
	}

	//Because the scales on the plots may have changed, we need to
	//update the external axes.
	updateAxes();
}
} /* namespace casa */
