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
#include "ThresholdingBinPlotWidget.qo.h"
#include <images/Images/ImageHistograms.h>
#include <display/QtPlotter/Util.h>
#include <QDebug>
#include <QPainter>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

namespace casa {

ThresholdingBinPlotWidget::ThresholdingBinPlotWidget(QString yAxisUnits, QWidget *parent)
    : QWidget(parent), histogramMaker( NULL ), binPlot( this ), rangeTool( NULL ), rectMarker( NULL),
      curveColor( Qt::blue ), selectionColor( 205, 201, 201, 127 ){

	ui.setupUi(this);

	//Add the plot to the dialog
	QHBoxLayout* layout = new QHBoxLayout(ui.plotHolder);
	layout->addWidget( &binPlot );
	binPlot.setAxisTitle( QwtPlot::yLeft, "Count" );
	binPlot.setAxisTitle( QwtPlot::xBottom, yAxisUnits );
	binPlot.setCanvasBackground( Qt::white );
	ui.plotHolder->setLayout( layout );

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
	rangeTool = new RangePicker(QwtPlot::xBottom, QwtPlot::yLeft,
			QwtPlotPicker::RectSelection | QwtPlotPicker::DragSelection,
		    QwtPlotPicker::RectRubberBand, QwtPlotPicker::AlwaysOn,
		    binPlot.canvas());
	rangeTool->setHeightSource( this );
	connect( rangeTool, SIGNAL(selected(const QwtDoubleRect &)), this, SLOT(rectangleSelected(const QwtDoubleRect &)));
	connect( rangeTool, SIGNAL(appended(const QPoint&)), this, SLOT(clearRange()));

	//histogram of values
	connect( ui.histogramCheckBox, SIGNAL(toggled(bool)), this, SLOT(defineCurve()));
	//log of values
	connect( ui.logCheckBox, SIGNAL(toggled(bool)), this, SLOT(defineCurve()));

	//Min/max line edits
	minMaxValidator = new QDoubleValidator( this );
	ui.minLineEdit->setValidator( minMaxValidator );
	ui.maxLineEdit->setValidator( minMaxValidator );
	connect( ui.minLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(minMaxChanged()));
	connect( ui.maxLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(minMaxChanged()));
}

//------------------------------------------------------------------------------
//                      Getters/Setters
//------------------------------------------------------------------------------
void ThresholdingBinPlotWidget::setMinMaxValues( double minValue, double maxValue,
		bool updateGraph ){
	ui.minLineEdit->setText( QString::number( minValue ));
	ui.maxLineEdit->setText( QString::number( maxValue ));
	if ( updateGraph ){
		minMaxChanged();
	}
}

pair<double,double> ThresholdingBinPlotWidget::getMinMaxValues() const {
	QString minValueStr = ui.minLineEdit->text();
	QString maxValueStr = ui.maxLineEdit->text();
	double minValue = minValueStr.toDouble();
	double maxValue = maxValueStr.toDouble();
	if ( minValue > maxValue ){
		double tmp = minValue;
		minValue = maxValue;
		maxValue = tmp;
	}
	pair<double,double> maxMinValues(minValue,maxValue);
	return maxMinValues;
}



//--------------------------------------------------------------------------------
//                           Plot Data
//--------------------------------------------------------------------------------

void ThresholdingBinPlotWidget::addCurve( QVector<double>& xValues, QVector<double>& yValues ){
	QwtPlotCurve* curve  = new QwtPlotCurve();
	curve->setData( xValues, yValues );
	QPen curvePen( curveColor );
	curve->setPen(curvePen);
	curve->attach(&binPlot);
	curves.append( curve );
}

double ThresholdingBinPlotWidget::checkLogValue( double value ) const {
	double resultValue = value;
	if ( ui.logCheckBox->isChecked() ){
		if ( value != 0 ){
			resultValue = qLn( value ) / qLn( 10 );
		}
	}
	return resultValue;
}

void ThresholdingBinPlotWidget::defineCurveHistogram(){
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

void ThresholdingBinPlotWidget::defineCurveLine(){
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

void ThresholdingBinPlotWidget::defineCurve(){
	clearCurves();
	if ( ui.histogramCheckBox->isChecked() ){
		defineCurveHistogram();
	}
	else {
		defineCurveLine();
	}
	binPlot.replot();
}

void ThresholdingBinPlotWidget::setImage( ImageInterface<Float>* img ){
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
		bool success = histogramMaker->getHistograms( values, counts );
		if ( success ){
			//Store the data
			values.tovector( xVector );
			counts.tovector( yVector );
			defineCurve();
			setValidatorLimits();
		}
		else {
			QString msg( "Could not make a histogram from the image.");
			Util::showUserMessage( msg, this );
		}
	}
}

void ThresholdingBinPlotWidget::setValidatorLimits(){
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
		minMaxValidator->setBottom( minValue );
		minMaxValidator->setTop( maxValue );
	}
}

void ThresholdingBinPlotWidget::clearCurves(){
	while( ! curves.isEmpty() ){
		QwtPlotCurve* curve = curves.takeFirst();
		curve->detach();
		delete curve;
	}
}


//-----------------------------------------------------------------------
//                   Rectangle Range Specification
//-----------------------------------------------------------------------

void ThresholdingBinPlotWidget::resizeEvent( QResizeEvent* event ){
	QWidget::resizeEvent( event );
	rectangleSizeChanged();
}

void ThresholdingBinPlotWidget::rectangleSizeChanged(){
	resetRectangleMarker();
	binPlot.replot();
}

void ThresholdingBinPlotWidget::rectangleSelected(const QwtDoubleRect & rect){
	rectX = rect.x();
	rectWidth = rect.width();
	resetRectangleMarker();
	rectMarker->show();
	binPlot.replot();
	setMinMaxValues( rectX, rectX + rectWidth, false );
}

void ThresholdingBinPlotWidget::resetRectangleMarker(){
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


void ThresholdingBinPlotWidget::minMaxChanged(){
	pair<double,double> minMaxValues = getMinMaxValues();
	rectX = minMaxValues.first;
	rectWidth = qAbs( minMaxValues.second - minMaxValues.first );
	resetRectangleMarker();
	rectMarker->show();
	binPlot.replot();
}

void ThresholdingBinPlotWidget::clearRange(){
	if ( rectMarker->isVisible() ){
		rectangleSizeChanged();
		rectMarker->hide();
		binPlot.replot();
	}
}

int ThresholdingBinPlotWidget::getCanvasHeight() {
	QwtPlotCanvas* canvas = binPlot.canvas();
		QSize canvasSize = canvas->size();
		int height = canvasSize.height();
		return height;
}

ThresholdingBinPlotWidget::~ThresholdingBinPlotWidget(){
	clearCurves();
	delete histogramMaker;
}

//-----------------------------------------------------------------------
//                    Drawing the rectangle
//-----------------------------------------------------------------------

RangePicker::RangePicker( int xAxis, int yAxis, int selectionFlags, RubberBand rubberBand,
	DisplayMode trackerMode, QwtPlotCanvas* canvas):
	QwtPlotPicker( xAxis, yAxis, selectionFlags, rubberBand,trackerMode, canvas ){
}

void RangePicker::setHeightSource( HeightSource* source ){
	heightSource = source;
}

void RangePicker::drawRubberBand( QPainter* painter )const {
	QPolygon polySelect = selection();
	int count = polySelect.count();
	if ( count >= 2 ){
		QPoint firstPoint = polySelect.point(0);
		QPoint secondPoint = polySelect.point( 1 );
		int ptX = static_cast<int>( firstPoint.x() );
		if ( firstPoint.x() > secondPoint.x() ){
			ptX = static_cast<int>( secondPoint.x() );
		}
		int width = qAbs( firstPoint.x() - secondPoint.x() );
		QColor shadeColor(100,100,100 );
		shadeColor.setAlpha( 100 );
		int rectHeight = heightSource->getCanvasHeight();
		QRect rect(ptX, 0, width, rectHeight );
		painter->fillRect( rect , shadeColor );
	}
}

RangePicker::~RangePicker(){

}
}
