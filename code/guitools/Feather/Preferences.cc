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
#include "Preferences.qo.h"
#include <QSettings>
#include <QDebug>
#include <limits>

namespace casa {

const QString Preferences::ORGANIZATION = "NRAO/CASA";
const QString Preferences::APPLICATION = "Feather";
const QString Preferences::LINE_THICKNESS = "Plot Line Thickness";
const QString Preferences::DOT_SIZE = "Dot Size";
const QString Preferences::DISPLAY_LEGEND = "Display Legend";
const QString Preferences::DISPLAY_OUTPUT_FUNCTIONS = "Display Output Functions";
const QString Preferences::DISPLAY_OUTPUT_SCATTERPLOT = "Display Output Scatter Plot";
const QString Preferences::DISPLAY_Y_PLOTS = "Display Y Plots";
const QString Preferences::DISPLAY_X_PLOTS = "Display X Plots";
const QString Preferences::DISPLAY_X_AXIS_UV = "Display X Axis U/V";
const QString Preferences::LOG_AMPLITUDE = "Logarithm of Amplitude";
const QString Preferences::LOG_UV = "Logarithm of u/v Axis";
const QString Preferences::PLANE_AVERAGED = "Plane Averaged";

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
      lineThickness( 1 ),
      dotSize( 2 ),
      displayOutputFunctions( true ),
      displayOutputScatterPlot( false ),
      displayYPlots( true ),
      displayXPlots( true ),
      displayLegend(true),
      logAmplitude(true),
      logUV(false),
      xAxisUV(false),
      planeAveraged(true),
      planeIndex(0){

	ui.setupUi(this);
	setWindowTitle( "Feather Plot Display");
	setModal( false );

	//Plane initialization
	ui.planeMaxLabel->setText("");
	ui.planeLineEdit->setText( "0");
	QButtonGroup* bGroup = new QButtonGroup( this );
	bGroup->addButton( ui.singlePlaneRadio );
	bGroup->addButton( ui.averagedPlaneRadio );
	connect( ui.singlePlaneRadio, SIGNAL(clicked()), this, SLOT(planeModeChanged()) );
	connect( ui.averagedPlaneRadio, SIGNAL(clicked()), this, SLOT(planeModeChanged()) );
	QIntValidator* planeValidator = new QIntValidator( 0, std::numeric_limits<int>::max(),this );
	ui.planeLineEdit->setValidator( planeValidator );
	QPalette palette = ui.planeMaxLabel->palette();
	palette.setColor(QPalette::Foreground, Qt::red );
	ui.planeMaxLabel->setPalette( palette );

	ui.lineThicknessSpinBox->setMinimum( 1 );
	ui.lineThicknessSpinBox->setMaximum( 5 );
	ui.dotSizeSpinBox->setMinimum( 1 );
	ui.dotSizeSpinBox->setMaximum( 10 );

	//xAxis Units
	QButtonGroup* xAxisGroup = new QButtonGroup( this );
	xAxisGroup->addButton( ui.axisUVRadio );
	xAxisGroup->addButton( ui.axisRadialRadio );
	connect( ui.axisUVRadio, SIGNAL(clicked()), this, SLOT( xAxisChanged()));
	connect( ui.axisRadialRadio, SIGNAL(clicked()), this, SLOT( xAxisChanged()));

	initializeCustomSettings();
	reset();

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(preferencesAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(preferencesRejected()));


}

void Preferences::initializeCustomSettings(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( ORGANIZATION, APPLICATION );
	lineThickness = settings.value( LINE_THICKNESS, lineThickness).toInt();
	dotSize = settings.value( DOT_SIZE, dotSize).toInt();
	displayLegend = settings.value( DISPLAY_LEGEND, displayLegend ).toBool();
	displayOutputScatterPlot = settings.value( DISPLAY_OUTPUT_SCATTERPLOT, displayOutputScatterPlot).toBool();
	displayYPlots = settings.value( DISPLAY_Y_PLOTS, displayYPlots ).toBool();
	displayXPlots = settings.value( DISPLAY_X_PLOTS, displayXPlots ).toBool();
	logAmplitude = settings.value( LOG_AMPLITUDE, logAmplitude ).toBool();
	logUV = settings.value( LOG_UV, logUV ).toBool();
	//xAxisUV = settings.value( DISPLAY_X_AXIS_UV, xAxisUV ).toBool();
	xAxisUV = true;
	//planeAveraged = settings.value(PLANE_AVERAGED, planeAveraged ).toBool();
	planeAveraged = false;
}

void Preferences::planeModeChanged(){
	bool planeMode = ui.singlePlaneRadio->isChecked();
	ui.planeLineEdit->setEnabled( planeMode );
}

void Preferences::xAxisChanged(){
	bool uvXAxis = false;
	if ( ui.axisUVRadio->isChecked() ){
		uvXAxis = true;
	}
	ui.xPlotCheckBox->setEnabled(uvXAxis);
	ui.yPlotCheckBox->setEnabled(uvXAxis);
}

void Preferences::setPlaneCount( int planeCount ){
	ui.planeMaxLabel->setText( "<"+QString::number( planeCount ) );
	QString currentPlaneText = ui.planeLineEdit->text();
	bool valid = false;
	int currentPlane = currentPlaneText.toInt(&valid);

	if ( currentPlane >= planeCount || !valid ){

		ui.planeLineEdit->setText( "0");
	}
	QIntValidator* planeValidator = new QIntValidator( 0, planeCount - 1, this );
	ui.planeLineEdit->setValidator( planeValidator );
}

bool Preferences::isLogAmplitude() const {
	return logAmplitude;
}

bool Preferences::isLogUV() const {
	return logUV;
}

bool Preferences::isDisplayOutputFunctions() const {
	return displayOutputFunctions;
}


bool Preferences::isDisplayLegend() const {
	return displayLegend;
}

bool Preferences::isDisplayOutputScatterPlot() const {
	return displayOutputScatterPlot;
}

bool Preferences::isDisplayX() const {
	return displayXPlots;
}

bool Preferences::isDisplayY() const {
	return displayYPlots;
}

bool Preferences::isXAxisUV() const {
	return xAxisUV;
}

bool Preferences::isPlaneAveraged() const {
	bool averaged = true;
	if ( ui.singlePlaneRadio->isChecked() ){
		averaged = false;
	}
	return averaged;
}

int Preferences::getPlaneIndex() const {
	return planeIndex;
}

int Preferences::getLineThickness() const {
	return lineThickness;
}

int Preferences::getDotSize() const {
	return dotSize;
}

void Preferences::preferencesAccepted(){
	persist();
	emit preferencesChanged();
}

void Preferences::preferencesRejected(){
	reset();
	this->close();
}

void Preferences::reset(){
	ui.lineThicknessSpinBox->setValue( lineThickness );
	ui.dotSizeSpinBox->setValue( dotSize );
	ui.legendCheckBox->setChecked( displayLegend );
	ui.outputCheckBox->setChecked( displayOutputFunctions );
	ui.outputScatterCheckBox->setChecked( displayOutputScatterPlot );
	ui.yPlotCheckBox->setChecked( displayYPlots );
	ui.xPlotCheckBox->setChecked( displayXPlots );
	ui.logAmplitudeCheckBox->setChecked( logAmplitude );
	ui.logUVCheckBox->setChecked( logUV );
	if ( xAxisUV ){
		ui.axisUVRadio->setChecked(true);
	}
	else {
		ui.axisRadialRadio->setChecked( true );
	}
	if ( planeAveraged ){
		ui.averagedPlaneRadio->setChecked(true);
	}
	else {
		ui.singlePlaneRadio->setChecked( true );
	}
	ui.planeLineEdit->setText( QString::number(planeIndex) );
	//Call axis changed to sync up the enable/disable state
	xAxisChanged();
	planeModeChanged();
}

void Preferences::persist(){
	QSettings settings( ORGANIZATION, APPLICATION );

	lineThickness = ui.lineThicknessSpinBox->value();
	settings.setValue( LINE_THICKNESS, lineThickness );

	dotSize = ui.dotSizeSpinBox->value();
	settings.setValue( DOT_SIZE, dotSize );

	displayLegend = ui.legendCheckBox->isChecked();
	settings.setValue( DISPLAY_LEGEND, displayLegend );

	displayOutputFunctions = ui.outputCheckBox->isChecked();
	settings.setValue( DISPLAY_OUTPUT_FUNCTIONS, displayOutputFunctions );

	displayOutputScatterPlot = ui.outputScatterCheckBox->isChecked();
	settings.setValue( DISPLAY_OUTPUT_SCATTERPLOT, displayOutputScatterPlot );

	displayYPlots = ui.yPlotCheckBox->isChecked();
	settings.setValue( DISPLAY_Y_PLOTS, displayYPlots );

	displayXPlots = ui.xPlotCheckBox->isChecked();
	settings.setValue( DISPLAY_X_PLOTS, displayXPlots );

	logAmplitude = ui.logAmplitudeCheckBox->isChecked();
	settings.setValue( LOG_AMPLITUDE, logAmplitude );

	logUV = ui.logUVCheckBox->isChecked();
	settings.setValue( LOG_UV, logUV );

	xAxisUV = ui.axisUVRadio->isChecked();
	settings.setValue( DISPLAY_X_AXIS_UV, xAxisUV );

	planeAveraged = ui.averagedPlaneRadio->isChecked();
	settings.setValue( PLANE_AVERAGED, planeAveraged );
	if ( !planeAveraged ){
		QString planeStr = ui.planeLineEdit->text();
		planeIndex = planeStr.toInt();
	}
}

Preferences::~Preferences(){
}

}
