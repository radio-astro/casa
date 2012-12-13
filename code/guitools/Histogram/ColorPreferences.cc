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
#include <guitools/Histogram/ColorDelegate.h>
#include <QColorDialog>
#include <QSettings>
#include <QMessageBox>

namespace casa {

const QString ColorPreferences::APPLICATION = "Histogram";
const QString ColorPreferences::ORGANIZATION = "NRAO/CASA";
const QString ColorPreferences::HISTOGRAM_COLOR = "Histogram Color";
const QString ColorPreferences::FIT_ESTIMATE_COLOR = "Histogram Fit Estimate Color";
const QString ColorPreferences::FIT_CURVE_COLOR = "Histogram Fit Curve Color";
const QString ColorPreferences::MULTIPLE_HISTOGRAM_COLORS = "Multiple Histogram Colors";

ColorPreferences::ColorPreferences(QWidget *parent)
    : QDialog(parent),
      histogramColor(Qt::blue), fitEstimateColor(Qt::green), fitCurveColor(Qt::cyan){
	ui.setupUi(this);
	setWindowTitle( "Histogram Color Preferences");

	//Color list for multiple histogram
	ui.multipleHistogramColorList->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.multipleHistogramColorList->setSelectionMode( QAbstractItemView::SingleSelection );
	ui.multipleHistogramColorList->setItemDelegate( new ColorDelegate( this ));
	multipleHistogramColors << "#800000"<<"#FF00FF" << "#FFFF00" << "#FF6600";

	initializeUserColors();
	resetColors();

	connect( ui.histogramColorButton, SIGNAL(clicked()), this, SLOT(selectHistogramColor()));
	connect( ui.fitCurveColorButton, SIGNAL(clicked()), this, SLOT(selectFitCurveColor()));
	connect( ui.fitEstimateColorButton, SIGNAL(clicked()), this, SLOT( selectFitEstimateColor()));
	connect( ui.addHistogramColorButton, SIGNAL(clicked()), this, SLOT( addHistogramColor()));
	connect( ui.deleteHistogramColorButton, SIGNAL(clicked()), this, SLOT( deleteHistogramColor()));

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
}

void ColorPreferences::initializeUserColors(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( ORGANIZATION, APPLICATION );

	QString histogramColorName = readCustomColor( settings, HISTOGRAM_COLOR, histogramColor.name() );
	if ( histogramColorName.length() > 0 ){
		histogramColor = QColor( histogramColorName );
	}

	QString fitEstimateColorName = readCustomColor( settings, FIT_ESTIMATE_COLOR, fitEstimateColor.name() );
	if ( fitEstimateColorName.length() > 0 ){
		fitEstimateColor = QColor( fitEstimateColorName );
	}

	QString fitCurveColorName = readCustomColor( settings, FIT_CURVE_COLOR, fitCurveColor.name() );
	if ( fitCurveColorName.length() > 0 ){
		fitCurveColor = QColor( fitCurveColorName );
	}

	readCustomColorList( settings );
}

QString ColorPreferences::readCustomColor( QSettings& settings,
		const QString& identifier, const QString& defaultColor){
	QString colorName = settings.value( identifier, defaultColor ).toString();
	return colorName;
}

void ColorPreferences::readCustomColorList( QSettings& settings){
	int colorCount = settings.value( MULTIPLE_HISTOGRAM_COLORS, 0 ).toInt();
	if ( colorCount > 0 ){
		multipleHistogramColors.clear();
		for ( int i = 0; i < colorCount; i++ ){
			QString lookupStr = MULTIPLE_HISTOGRAM_COLORS + QString::number(i);
			QString colorName = settings.value( lookupStr ).toString();
			multipleHistogramColors.append( colorName );
		}
	}
}

void ColorPreferences::persistColorList( QSettings& settings ){
	int colorCount = ui.multipleHistogramColorList->count();
	settings.setValue( MULTIPLE_HISTOGRAM_COLORS, colorCount );
	for ( int i = 0; i < colorCount; i++ ){
		QString persistStr = MULTIPLE_HISTOGRAM_COLORS + QString::number(i);
		QListWidgetItem* listItem = ui.multipleHistogramColorList->item(i);
		if ( listItem != NULL ){
			QColor listColor = listItem->backgroundColor();
			QString colorStr = listColor.name();
			settings.setValue( persistStr, colorStr );
		}
	}
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

	//Persist and save colors associated with multiple histograms
	multipleHistogramColors.clear();
	int colorCount = ui.multipleHistogramColorList->count();
	settings.setValue( MULTIPLE_HISTOGRAM_COLORS, colorCount );
	for ( int i = 0; i < colorCount; i++ ){
		QListWidgetItem* listItem = ui.multipleHistogramColorList->item(i);
		if ( listItem != NULL ){
			QColor listColor = listItem->backgroundColor();
			QString colorStr = listColor.name();
			multipleHistogramColors.append( colorStr );
			QString persistStr = MULTIPLE_HISTOGRAM_COLORS + QString::number(i);
			settings.setValue( persistStr, colorStr );
		}
	}
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
	ui.multipleHistogramColorList->clear();
	for( int i = 0; i < multipleHistogramColors.size(); i++ ){
		QColor itemColor;
		itemColor.setNamedColor( multipleHistogramColors[i] );
		addColorToList( itemColor );
	}
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

QList<QColor> ColorPreferences::getMultipleHistogramColors() const {
	int count = multipleHistogramColors.size();
	QList<QColor> colorList;
	for ( int i = 0; i < count; i++ ){
		colorList.append( QColor( multipleHistogramColors[i]));
	}
	return colorList;
}

void ColorPreferences::setFitColorsVisible( bool visible ){
	ui.fitCurveColorButton->setVisible( visible );
	ui.fitEstimateColorButton->setVisible( visible );
	ui.fitLabel->setVisible( visible );
	ui.estimateLabel->setVisible( visible );
}

void ColorPreferences::setMultipleHistogramColorsVisible( bool visible ){
	QLayout* colorLayout = layout();
	if ( !visible ){
		colorLayout->removeWidget( ui.multipleHistogramsColorHolder );
		ui.multipleHistogramsColorHolder->setParent( NULL );
	}

}

void ColorPreferences::addColorToList( QColor color ){
	QListWidgetItem* listItem = new QListWidgetItem( ui.multipleHistogramColorList );
	listItem->setBackground( color );
	ui.multipleHistogramColorList->addItem( listItem );
}

void ColorPreferences::addHistogramColor(){
	QColor selectedColor = QColorDialog::getColor( Qt::white, this );
	if ( selectedColor.isValid() ){
		addColorToList( selectedColor );
	}
}

void ColorPreferences::deleteHistogramColor(){
	int row = ui.multipleHistogramColorList->currentRow();
	if ( row >= 0 ){
		QListWidgetItem* listItem = ui.multipleHistogramColorList->takeItem( row );
		if ( listItem != NULL ){
			delete listItem;
		}
		//registerColorChange();
	}
	else {
		QString msg( "Please select a color in the list to delete.");
		QMessageBox::warning( this, "Color Not Selected", msg );
	}
}



ColorPreferences::~ColorPreferences(){

}
}
