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
#include <guitools/Feather/FeatherCurve.h>
#include <synthesis/MSVis/UtilJ.h>
#include <QDebug>
#include <QHBoxLayout>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_data.h>
#include <qwt_scale_draw.h>
#include <qwt_legend_item.h>

namespace casa {

const QString FeatherPlot::Y_EQUALS_X = "y=x";

FeatherPlot::FeatherPlot(QWidget* parent):QwtPlot( parent ),
	externalLegend( NULL), lineThickness(1),
	dotSize(1), AXIS_COUNT(3),
	MINIMUM_LEGEND_LINE_WIDTH(3){

	scaleLogAmplitude = false;
	scaleLogUV = false;

	initAxes();

	setCanvasBackground( Qt::white );
}


void FeatherPlot::initializePlot( const QString& graphTitle, PlotType plotType ){

	//Store what type of plot we are.
	this->plotType = plotType;

	//Set the title of the plot
	setTitle( graphTitle );
	QwtText titleText = title();
	QFont titleFont = titleText.font();
	titleFont.setPointSize(9);
	titleText.setFont( titleFont );
	setTitle( titleText );

	//Set up the axis scales and labels
	setAxisLabels();
	resetPlotBounds();
}


void FeatherPlot::initAxes(){
	for ( int i = 0; i < AXIS_COUNT; i++ ){
		axisBlanks.append( new ExternalAxis() );
		axisWidgets.append( NULL );
		axisLabels.append( "" );
		setAxisScaleDraw( i, axisBlanks[i] );
	}
}


void FeatherPlot::resetPlotBounds(){
	minX = std::numeric_limits<double>::max();
	maxX = -minX;
	minY = std::numeric_limits<double>::max();
	maxY = -minY;
	minYRight = std::numeric_limits<double>::max();
	maxYRight = -minYRight;
}


void FeatherPlot::setAxisLabels(){
	const QString AMPLITUDE = "Amplitude";
	QString amplitudeStr = AMPLITUDE;
	const QString Y_UNITS = "(Jansky)";
	amplitudeStr = amplitudeStr + Y_UNITS;
	if ( isLogAmplitude() ){
		amplitudeStr = "Log["+amplitudeStr+"]";
	}

	const QString DISTANCE = "Distance";
	QString distanceStr = DISTANCE;
	distanceStr = distanceStr + "(m)";
	if ( isLogUV()){
		distanceStr = "Log["+distanceStr+"]";
	}


	if ( plotType==SCATTER_PLOT ){
		axisLabels[QwtPlot::xBottom] = amplitudeStr;
		axisLabels[QwtPlot::yLeft] = amplitudeStr;
		axisLabels[QwtPlot::yRight] = axisLabels[QwtPlot::yLeft];
	}
	else {
		axisLabels[QwtPlot::xBottom] = distanceStr;
		axisLabels[QwtPlot::yLeft] = amplitudeStr;
		axisLabels[QwtPlot::yRight] = "Weight "+AMPLITUDE;
	}



	if ( axisWidgets[QwtPlot::xBottom] != NULL ){
		axisWidgets[QwtPlot::xBottom]->setAxisLabel(axisLabels[QwtPlot::xBottom]);
	}
	if ( axisWidgets[QwtPlot::yLeft] != NULL ){
		axisWidgets[QwtPlot::yLeft]->setAxisLabel(axisLabels[QwtPlot::yLeft]);
	}
	if ( axisWidgets[QwtPlot::yRight] != NULL ){
		axisWidgets[QwtPlot::yRight]->setAxisLabel(axisLabels[QwtPlot::yRight]);
	}
}


FeatherPlot::~FeatherPlot() {
	for ( int i = 0; i < axisWidgets.size(); i++ ){
		delete axisWidgets[i];
		axisWidgets[i] = NULL;
	}
	delete externalLegend;
	externalLegend = NULL;
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
				axisWidgets[position] = new ExternalAxisWidgetLeft( NULL);
			}
			else {
				axisWidgets[position] = new ExternalAxisWidgetRight( NULL );
			}
			axisWidgets[position]->setPlot( this );
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
	if ( externalLegend != NULL ){
		insertLegend( NULL, QwtPlot::ExternalLegend );
		delete externalLegend;
		externalLegend = NULL;
	}
}

void FeatherPlot::resetLegend(){
	setLegendSize();
}

void FeatherPlot::setLegendSize(){
	if ( legendParent != NULL && externalLegend != NULL ){
		int itemCount = externalLegend->itemCount();
		int rowCount = static_cast<int>(itemCount / 4.0) + 1;
		int height = rowCount * 25;
		legendParent->setMinimumSize( 800, height);
	}
}

void FeatherPlot::insertSingleLegend( QWidget* parent ){
	legendParent = parent;
	if ( externalLegend == NULL ){
		externalLegend = new QwtLegend( );
		QPalette legendPalette = externalLegend->palette();
		legendPalette.setColor( QPalette::Background, Qt::white );
		externalLegend->setAutoFillBackground( true );
		externalLegend->setPalette( legendPalette );
		externalLegend->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
	}
	insertLegend( externalLegend, QwtPlot::ExternalLegend );
	if ( parent != NULL ){
		QLayout* parentLayout = parent->layout();
		if ( parentLayout == NULL ){
			parentLayout = new QHBoxLayout();
		}
		parentLayout->setContentsMargins(0,0,0,0);
		parent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
		setLegendSize();
		QSpacerItem* item1 = new QSpacerItem(5,5,QSizePolicy::Expanding, QSizePolicy::Preferred);
		parentLayout->addItem( item1 );
		parentLayout->addWidget( externalLegend );
		QSpacerItem* item2 = new QSpacerItem(5,5,QSizePolicy::Expanding, QSizePolicy::Preferred);
		parentLayout->addItem( item2 );
		parent->setLayout( parentLayout );
	}
}


void FeatherPlot::setLegendVisibility( bool visible ){
	bool legendVisible = visible && !isScatterPlot();
	if ( externalLegend != NULL ){
		externalLegend->setVisible( legendVisible );
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


bool FeatherPlot::isLogUV() const {
	return scaleLogUV;
}


bool FeatherPlot::isLogAmplitude() const {
	return scaleLogAmplitude;
}


//--------------------------------------------------------------------------
//                      Curves
//--------------------------------------------------------------------------

void FeatherPlot::setCurveSize( int curveIndex ){
	bool diagonalLine = false;
	QString curveTitle = curves[curveIndex]->getTitle();
	if ( curveTitle == Y_EQUALS_X ){
		diagonalLine = true;
	}
	curves[curveIndex]->setCurveSize(isScatterPlot(), diagonalLine,
			dotSize, lineThickness);

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


bool FeatherPlot::setLogScale( bool uvLogScale, bool ampLogScale ){
	bool uvScaleChanged = false;
	if ( uvLogScale != scaleLogUV || ampLogScale != scaleLogAmplitude ){

		//Reset the scale state
		if ( uvLogScale != scaleLogUV ){
			uvScaleChanged = true;
		}
		scaleLogUV = uvLogScale;
		scaleLogAmplitude = ampLogScale;
		setAxisLabels();

		//Adjust the data in each curve.
		resetPlotBounds();
		for ( int i = 0; i < curves.size(); i++ ){
			//Put the data back into the curve.
			QwtPlot::Axis verticalAxis = curves[i]->getVerticalAxis();
			setCurveData( curves[i], verticalAxis );
		}
		//replot();
	}
	return uvScaleChanged;
}


void FeatherPlot::clearCurves(){
	int count = curves.size();
	for( int i = 0; i < count; i++ ){
		FeatherCurve* curve = curves[i];
		delete curve;
		curves[i] = NULL;
	}
	curves.clear();
	this->resetPlotBounds();
}


void FeatherPlot::setFunctionColor( const QString& curveID, const QColor& color ){
	int curveIndex = getCurveIndex( curveID );

	if ( curveIndex >= 0 ){
		bool diagonalLine = false;
		if ( curveID == Y_EQUALS_X ){
			diagonalLine = true;
		}
		curves[curveIndex]->setFunctionColor( color, diagonalLine );
	}
}


int FeatherPlot::getCurveIndex( const QString& curveTitle ) const {
	int curveIndex = -1;
	for ( int i = 0; i < curves.size(); i++ ){
		QString existingTitle = curves[i]->getTitle();
		if ( existingTitle == curveTitle ){
			curveIndex = i;
			break;
		}
	}
	return curveIndex;
}


void FeatherPlot::addDiagonal( QVector<double> values, QColor lineColor, QwtPlot::Axis axis ){
	//The line y=x.
	//See if we already have the curve.  Make one if it is not there already.
	int curveIndex = getCurveIndex( FeatherPlot::Y_EQUALS_X );
	FeatherCurve* diagonalLine = NULL;
	if ( curveIndex >= 0 ){
		diagonalLine = curves[curveIndex];
	}
	else {
		diagonalLine = new FeatherCurve(this,QwtPlot::xBottom, axis, false);
		diagonalLine->setTitle( FeatherPlot::Y_EQUALS_X );
		diagonalLine->setFunctionColor( lineColor, true );
		curves.append( diagonalLine );
		curveIndex = curves.size() - 1;
		setCurveSize( curveIndex );
	}
	diagonalLine->setCurveData( values, values );
	setCurveData( diagonalLine, axis );
}


void FeatherPlot::adjustPlotBounds( std::pair<double,double> curveBounds, QwtPlot::Axis axis ){

	//Decide if we are finding x or y bounds
	double* min = &minX;
	double* max = &maxX;
	if ( axis == QwtPlot::yLeft){
		min = &minY;
		max = &maxY;
	}
	else if ( axis == QwtPlot::yRight){
		min = &minYRight;
		max = &maxYRight;
	}

	//See if the new bounds result in changes to the current ones.
	if ( *min > curveBounds.first ){
		*min = curveBounds.first;
	}
	if ( *max < curveBounds.second ){
		*max = curveBounds.second;
	}

	//Reset the axes to use the new bounds.
	setAxisScale( axis, *min, *max );
}


void FeatherPlot::setCurveData( FeatherCurve* curve, QwtPlot::Axis yAxis ){
	curve->adjustData( scaleLogUV, scaleLogAmplitude );
	std::pair<double,double> xBounds = curve->getBoundsX();
	std::pair<double,double> yBounds = curve->getBoundsY();
	adjustPlotBounds( yBounds, yAxis );
	adjustPlotBounds( xBounds, QwtPlot::xBottom );
	//Because the scales on the plots may have changed, we need to
	//update the external axes.
	updateAxes();
}


void FeatherPlot::addCurve( QVector<double> xValues, QVector<double> yValues,
		QColor curveColor, const QString& curveTitle, QwtPlot::Axis yAxis, bool sumCurve ){

	//See if we already have the curve
	int curveIndex = getCurveIndex( curveTitle);
	FeatherCurve* curve = NULL;
	//We need to make a new curve
	if ( curveIndex < 0 ){
	    curve  = new FeatherCurve( this, QwtPlot::xBottom, yAxis, sumCurve );
	    if ( isScatterPlot() ){
			if ( axisWidgets[QwtPlot::yRight] != NULL ){
				ExternalAxisWidgetRight* rightWidget = dynamic_cast<ExternalAxisWidgetRight*>(axisWidgets[QwtPlot::yRight]);
				rightWidget->setUseLeftScale( true );
			}
			curve->initScatterPlot( dotSize );
		}
	    curve->setTitle( curveTitle );
	}
	//We are just going to change the data in an existing curve
	else {
		curve = curves[curveIndex];
	}

	//Store the curve if it is not already there
	if ( curveIndex < 0 ){
		 curves.append( curve );
		 setFunctionColor( curveTitle, curveColor );
		 int index = curves.size() - 1;
	     setCurveSize( index );
	}

	curve->setCurveData( xValues, yValues );
	setCurveData( curve, yAxis );
}
} /* namespace casa */
