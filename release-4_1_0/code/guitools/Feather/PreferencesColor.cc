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
#include "PreferencesColor.qo.h"
#include <guitools/Feather/Preferences.qo.h>
#include <QColorDialog>

namespace casa {

const QString PreferencesColor::FUNCTION_COLOR = "Function Color";

PreferencesColor::PreferencesColor(QWidget *parent)
    : QDialog(parent), SCATTER_INDEX(-1), DISH_DIAMETER_INDEX(-2),
      ZOOM_INDEX( -3 ){

	ui.setupUi(this);
	setWindowTitle( "Feather Plot Color Preferences");

	//Default colors
	colorMap.insert( SD_WEIGHT_COLOR, QColor( "#8B4513"));
	colorMap.insert( SD_SLICE_COLOR, QColor("#DEB887"));
	colorMap.insert( INT_WEIGHT_COLOR, QColor("#008080"));
	colorMap.insert( INT_SLICE_COLOR, QColor("#4682B4"));
	scatterPlotColor = Qt::black;
	zoomRectColor = QColor("#6A5ACD");
	dishDiameterLineColor = QColor( "#483D8B");

	initializeUserColors();
	resetColors();

	connect( ui.zoomRectangleColorButton, SIGNAL(clicked()), this, SLOT(selectZoomRectColor()));
	connect( ui.scatterColorButton, SIGNAL(clicked()), this, SLOT(selectScatterPlotColor()));
	connect( ui.dishDiameterColorButton, SIGNAL(clicked()), this, SLOT( selectDishDiameterLineColor()));
	connect( ui.singleDishWeightColorButton, SIGNAL(clicked()), this, SLOT(selectSDWeightColor()));
	connect( ui.singleDishSliceColorButton, SIGNAL(clicked()), this, SLOT(selectSDSliceColor()));
	connect( ui.interferometerWeightColorButton, SIGNAL(clicked()), this, SLOT(selectINTWeightColor()));
	connect( ui.interferometerSliceColorButton, SIGNAL(clicked()), this, SLOT(selectINTSliceColor()));

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
}

void PreferencesColor::initializeUserColors(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( Preferences::ORGANIZATION, Preferences::APPLICATION );
	for ( int i = 0; i < END_COLOR; i++ ){
		QString colorName = readCustomColor( settings, i );
		if ( colorName.length() > 0 ){
			QColor customColor( colorName );
			colorMap[static_cast<FunctionColor>(i)] = customColor;
		}
	}
	QString scatterColorName = readCustomColor( settings, SCATTER_INDEX );
	if ( scatterColorName.length() > 0 ){
		scatterPlotColor = QColor( scatterColorName );
	}

	QString dishDiameterColorName = readCustomColor( settings, DISH_DIAMETER_INDEX );
	if ( dishDiameterColorName.length() > 0 ){
		dishDiameterLineColor = QColor( dishDiameterColorName );
	}

	QString zoomRectColorName = readCustomColor( settings, ZOOM_INDEX );
	if ( zoomRectColorName.length() > 0 ){
		zoomRectColor = QColor( zoomRectColorName );
	}
}

QMap<PreferencesColor::FunctionColor,QColor> PreferencesColor::getFunctionColors( ) const {
	return colorMap;
}

QColor PreferencesColor::getScatterPlotColor() const{
	return scatterPlotColor;
}

QColor PreferencesColor::getDishDiameterLineColor() const {
	return dishDiameterLineColor;
}

QColor PreferencesColor::getZoomRectColor() const {
	return zoomRectColor;
}

void PreferencesColor::storeCustomColor( QSettings& settings, FunctionColor index ){
	QString storageKey = FUNCTION_COLOR + QString::number( index );
	QString colorName = colorMap[index].name();
	settings.setValue( storageKey, colorName );
}

QString PreferencesColor::readCustomColor( QSettings& settings, int index){
	QString lookupStr = FUNCTION_COLOR + QString::number(index);
	QString colorName = settings.value( lookupStr, "" ).toString();
	return colorName;
}

void PreferencesColor::storeMapColor( QPushButton* button, FunctionColor colorType ){
	QColor buttonColor = getButtonColor( button );
	QString buttonColorName = buttonColor.name();
	colorMap[colorType] = buttonColorName;
}

void PreferencesColor::persistColors(){
	//Copy the colors from the buttons into the map.
	storeMapColor( ui.singleDishWeightColorButton, SD_WEIGHT_COLOR);
	storeMapColor( ui.singleDishSliceColorButton, SD_SLICE_COLOR);

	storeMapColor( ui.interferometerWeightColorButton, INT_WEIGHT_COLOR);
	storeMapColor( ui.interferometerSliceColorButton, INT_SLICE_COLOR);

	scatterPlotColor = getButtonColor( ui.scatterColorButton );
	dishDiameterLineColor = getButtonColor( ui.dishDiameterColorButton );
	zoomRectColor = getButtonColor( ui.zoomRectangleColorButton );

	//Save the colors in the map
	QSettings settings( Preferences::ORGANIZATION, Preferences::APPLICATION );
	settings.clear();
	for ( int i = 0; i < END_COLOR; i++ ){
		storeCustomColor( settings, static_cast<FunctionColor>(i) );
	}

	QString scatterKey = FUNCTION_COLOR + QString::number( SCATTER_INDEX );
	settings.setValue( scatterKey, scatterPlotColor.name() );

	QString dishDiameterKey = FUNCTION_COLOR + QString::number( DISH_DIAMETER_INDEX );
	settings.setValue( dishDiameterKey, dishDiameterLineColor.name());

	QString zoomRectKey = FUNCTION_COLOR + QString::number( ZOOM_INDEX );
	settings.setValue( zoomRectKey, zoomRectColor.name());
}

void PreferencesColor::colorsAccepted(){
	persistColors();
	QDialog::close();
	emit colorsChanged();
}

void PreferencesColor::colorsRejected(){
	resetColors();
	QDialog::close();
}

void PreferencesColor::resetColors(){
	setButtonColor( ui.scatterColorButton, Qt::black );
	setButtonColor( ui.singleDishWeightColorButton, colorMap[SD_WEIGHT_COLOR]);
	setButtonColor( ui.singleDishSliceColorButton, colorMap[SD_SLICE_COLOR]);
	setButtonColor( ui.interferometerWeightColorButton, colorMap[INT_WEIGHT_COLOR]);
	setButtonColor( ui.interferometerSliceColorButton, colorMap[INT_SLICE_COLOR]);
	setButtonColor( ui.scatterColorButton, scatterPlotColor );
	setButtonColor( ui.dishDiameterColorButton, dishDiameterLineColor );
	setButtonColor( ui.zoomRectangleColorButton, zoomRectColor );
}

void PreferencesColor::setButtonColor( QPushButton* button, QColor color ){
	QPalette p = button->palette();
	p.setBrush(QPalette::Button, color);
	button->setPalette( p );
}

QColor PreferencesColor::getButtonColor( QPushButton* button ) const {
	QPalette p = button->palette();
	QBrush brush = p.brush(QPalette::Button );
	QColor backgroundColor = brush.color();
	return backgroundColor;
}

void PreferencesColor::showColorDialog( QPushButton* source ){
	QColor initialColor = getButtonColor( source );
	QColor selectedColor = QColorDialog::getColor( initialColor, this );
	if ( selectedColor.isValid() ){
		setButtonColor( source, selectedColor );
	}
}

void PreferencesColor::selectSDWeightColor(){
	showColorDialog( ui.singleDishWeightColorButton );
}

void PreferencesColor::selectSDSliceColor(){
	showColorDialog( ui.singleDishSliceColorButton );
}

void PreferencesColor::selectINTWeightColor(){
	showColorDialog( ui.interferometerWeightColorButton );
}

void PreferencesColor::selectINTSliceColor(){
	showColorDialog( ui.interferometerSliceColorButton );
}

void PreferencesColor::selectScatterPlotColor(){
	showColorDialog( ui.scatterColorButton );
}

void PreferencesColor::selectDishDiameterLineColor(){
	showColorDialog( ui.dishDiameterColorButton );
}

void PreferencesColor::selectZoomRectColor(){
	showColorDialog( ui.zoomRectangleColorButton );
}

PreferencesColor::~PreferencesColor(){
}

}
