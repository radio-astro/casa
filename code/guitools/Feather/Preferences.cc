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

namespace casa {

const QString Preferences::ORGANIZATION = "NRAO/CASA";
const QString Preferences::APPLICATION = "Feather";
const QString Preferences::LINE_THICKNESS = "Plot Line Thickness";
const QString Preferences::DOT_SIZE = "Dot Size";
const QString Preferences::DISPLAY_ORIGINAL_FUNCTIONS = "Display Original Functions";
const QString Preferences::DISPLAY_LEGEND = "Display Legend";
const QString Preferences::DISPLAY_OUTPUT_FUNCTIONS = "Display Output Functions";
const QString Preferences::DISPLAY_OUTPUT_SCATTERPLOT = "Display Output Scatter Plot";
const QString Preferences::DISPLAY_Y_PLOTS = "Display Y Plots";

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
      lineThickness( 1 ),
      dotSize( 2 ),
      displayOriginalFunctions(false),
      displayOutputFunctions( true ),
      displayOutputScatterPlot( false ),
      displayYPlots( true ),
      displayLegend(true){

	ui.setupUi(this);
	setWindowTitle( "Feather Plot Display");

	ui.lineThicknessSpinBox->setMinimum( 1 );
	ui.lineThicknessSpinBox->setMaximum( 5 );
	ui.dotSizeSpinBox->setMinimum( 1 );
	ui.dotSizeSpinBox->setMaximum( 10 );

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
	displayOriginalFunctions = settings.value( DISPLAY_ORIGINAL_FUNCTIONS, displayOriginalFunctions).toBool();
	displayLegend = settings.value( DISPLAY_LEGEND, displayLegend ).toBool();
	displayOutputFunctions = settings.value( DISPLAY_OUTPUT_FUNCTIONS, displayOutputFunctions ).toBool();
	displayOutputScatterPlot = settings.value( DISPLAY_OUTPUT_SCATTERPLOT, displayOutputScatterPlot).toBool();
	displayYPlots = settings.value( DISPLAY_Y_PLOTS, displayYPlots ).toBool();
}
bool Preferences::isDisplayOutputFunctions() const {
	return displayOutputFunctions;
}
bool Preferences::isDisplayOriginalFunctions() const {
	return displayOriginalFunctions;
}

bool Preferences::isDisplayLegend() const {
	return displayLegend;
}

bool Preferences::isDisplayOutputScatterPlot() const {
	return displayOutputScatterPlot;
}

bool Preferences::isDisplayXOnly() const {
	return !displayYPlots;
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
	this->close();
}

void Preferences::preferencesRejected(){
	reset();
	this->close();
}

void Preferences::reset(){
	ui.lineThicknessSpinBox->setValue( lineThickness );
	ui.dotSizeSpinBox->setValue( dotSize );
	ui.originalCheckBox->setChecked( displayOriginalFunctions );
	ui.legendCheckBox->setChecked( displayLegend );
	ui.outputCheckBox->setChecked( displayOutputFunctions );
	ui.outputScatterCheckBox->setChecked( displayOutputScatterPlot );
	ui.yPlotCheckBox->setChecked( displayYPlots );

}

void Preferences::persist(){
	QSettings settings( ORGANIZATION, APPLICATION );

	lineThickness = ui.lineThicknessSpinBox->value();
	settings.setValue( LINE_THICKNESS, lineThickness );

	dotSize = ui.dotSizeSpinBox->value();
	settings.setValue( DOT_SIZE, dotSize );

	displayOriginalFunctions = ui.originalCheckBox->isChecked();
	settings.setValue( DISPLAY_ORIGINAL_FUNCTIONS, displayOriginalFunctions );

	displayLegend = ui.legendCheckBox->isChecked();
	settings.setValue( DISPLAY_LEGEND, displayLegend );

	displayOutputFunctions = ui.outputCheckBox->isChecked();
	settings.setValue( DISPLAY_OUTPUT_FUNCTIONS, displayOutputFunctions );

	displayOutputScatterPlot = ui.outputScatterCheckBox->isChecked();
	settings.setValue( DISPLAY_OUTPUT_SCATTERPLOT, displayOutputScatterPlot );

	displayYPlots = ui.yPlotCheckBox->isChecked();
	settings.setValue( DISPLAY_Y_PLOTS, displayYPlots );
}

Preferences::~Preferences(){
}

}
