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
#include "ColorPreferences.qo.h"
#include <QColorDialog>
#include <QSettings>

namespace casa {

const QString ColorPreferences::APPLICATION = "Histogram";
const QString ColorPreferences::ORGANIZATION = "NRAO/CASA";
const QString ColorPreferences::HISTOGRAM_COLOR = "Histogram Color";
const QString ColorPreferences::FIT_ESTIMATE_COLOR = "Histogram Fit Estimate Color";
const QString ColorPreferences::FIT_CURVE_COLOR = "Histogram Fit Curve Color";

ColorPreferences::ColorPreferences(QWidget *parent)
    : QDialog(parent),
      histogramColor(Qt::blue), fitEstimateColor(Qt::green), fitCurveColor(Qt::cyan){
	ui.setupUi(this);
	setWindowTitle( "Histogram Color Preferences");

	initializeUserColors();
	resetColors();

	connect( ui.histogramColorButton, SIGNAL(clicked()), this, SLOT(selectHistogramColor()));
	connect( ui.fitCurveColorButton, SIGNAL(clicked()), this, SLOT(selectFitCurveColor()));
	connect( ui.fitEstimateColorButton, SIGNAL(clicked()), this, SLOT( selectFitEstimateColor()));

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
}

void ColorPreferences::initializeUserColors(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( ORGANIZATION, APPLICATION );

	QString histogramColorName = readCustomColor( settings, HISTOGRAM_COLOR );
	if ( histogramColorName.length() > 0 ){
		histogramColor = QColor( histogramColorName );
	}

	QString fitEstimateColorName = readCustomColor( settings, FIT_ESTIMATE_COLOR );
	if ( fitEstimateColorName.length() > 0 ){
		fitEstimateColor = QColor( fitEstimateColorName );
	}

	QString fitCurveColorName = readCustomColor( settings, FIT_CURVE_COLOR );
	if ( fitCurveColorName.length() > 0 ){
		fitCurveColor = QColor( fitCurveColorName );
	}
}

QString ColorPreferences::readCustomColor( QSettings& settings, const QString& identifier){
	QString colorName = settings.value( identifier, "" ).toString();
	return colorName;
}

void ColorPreferences::persistColors(){
	//Copy the colors from the buttons into color variables
	histogramColor = getButtonColor( ui.histogramColorButton );
	fitEstimateColor = getButtonColor( ui.fitEstimateColorButton );
	fitCurveColor = getButtonColor( ui.fitCurveColorButton );

	//Save the colors in the persistent settings.
	QSettings settings( ORGANIZATION, APPLICATION );
	settings.clear();
	settings.setValue( HISTOGRAM_COLOR, histogramColor.name() );
	settings.setValue( FIT_ESTIMATE_COLOR, fitEstimateColor.name());
	settings.setValue( FIT_CURVE_COLOR, fitCurveColor.name());
}

void ColorPreferences::colorsAccepted(){
	persistColors();
	QDialog::close();
	emit colorsChanged();
}

void ColorPreferences::colorsRejected(){
	resetColors();
	QDialog::close();
}

void ColorPreferences::resetColors(){
	setButtonColor( ui.histogramColorButton, histogramColor );
	setButtonColor( ui.fitEstimateColorButton, fitEstimateColor );
	setButtonColor( ui.fitCurveColorButton, fitCurveColor );
}

void ColorPreferences::setButtonColor( QPushButton* button, QColor color ){
	QPalette p = button->palette();
	p.setBrush(QPalette::Button, color);
	button->setPalette( p );
}

QColor ColorPreferences::getButtonColor( QPushButton* button ) const {
	QPalette p = button->palette();
	QBrush brush = p.brush(QPalette::Button );
	QColor backgroundColor = brush.color();
	return backgroundColor;
}

void ColorPreferences::showColorDialog( QPushButton* source ){
	QColor initialColor = getButtonColor( source );
	QColor selectedColor = QColorDialog::getColor( initialColor, this );
	if ( selectedColor.isValid() ){
		setButtonColor( source, selectedColor );
	}
}

void ColorPreferences::selectHistogramColor(){
	showColorDialog( ui.histogramColorButton );
}

void ColorPreferences::selectFitCurveColor(){
	showColorDialog( ui.fitCurveColorButton );
}

void ColorPreferences::selectFitEstimateColor(){
	showColorDialog( ui.fitEstimateColorButton );
}

QColor ColorPreferences::getHistogramColor() const {
	return histogramColor;
}

QColor ColorPreferences::getFitEstimateColor() const {
	return fitEstimateColor;
}

QColor ColorPreferences::getFitCurveColor() const {
	return fitCurveColor;
}

void ColorPreferences::setFitColorsVisible( bool visible ){
	ui.fitCurveColorButton->setVisible( visible );
	ui.fitEstimateColorButton->setVisible( visible );
	ui.fitLabel->setVisible( visible );
	ui.estimateLabel->setVisible( visible );
}
ColorPreferences::~ColorPreferences(){

}
}
