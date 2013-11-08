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
#include <guitools/Feather/PreferencesFunction.qo.h>
#include <QColorDialog>
#include <QSettings>
#include <QDebug>

namespace casa {

const QString PreferencesColor::SCATTER_X_CURVE = "Scatter x-axis curve";
const QString PreferencesColor::SCATTER_Y_CURVES = "Scatter y-axis curves";


PreferencesColor::PreferencesColor(QWidget *parent)
    : QDialog(parent){

	ui.setupUi(this);

	setWindowTitle( "Feather Plot Curve Preferences");
	setModal( false );
	scatterXIndex = FeatherCurveType::LOW_WEIGHTED;
	scatterYIndices.append( FeatherCurveType::HIGH_WEIGHTED );

	initializeCurvePreferences();

	//Set up the scatter parameters
	ui.scatterYAxisList->setSelectionMode( QAbstractItemView::ExtendedSelection );
	connect( ui.scatterXAxisCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(populateScatterAxisY()));


	//Read in any persisted user settings.
	initializeUser();
	//Fill in the list and combo based on user settings.
	populateScatterAxes();
	//Update the UI with user settings.
	reset();

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
}

void PreferencesColor::populateScatterAxes(){
	populateScatterAxisX();
	populateScatterAxisY();
}

void PreferencesColor::initializeCurvePreferences(){
	for ( int i = 0; i < FeatherCurveType::CURVES_END; i++ ){
		PreferencesFunction* functionPreferences = new PreferencesFunction( i, this );
		curvePreferences.insert( static_cast<CurveType>(i), functionPreferences );
		connect( functionPreferences, SIGNAL(displayStatusChanged()), this, SLOT(populateScatterAxes()));
	}

	//Set up the initial (default) colors
	setCurveDefaults();

	//Add the curve preferences to the ui.
	addCurvePreferences();

}

void PreferencesColor::setCurveDefaults(){

	//First the colors
	curvePreferences[FeatherCurveType::WEIGHT_LOW]->setColor( "#FFD700" );
	curvePreferences[FeatherCurveType::WEIGHT_HIGH]->setColor( "#FF8C00" );
	curvePreferences[FeatherCurveType::ZOOM]->setColor(Qt::black);
	curvePreferences[FeatherCurveType::DISH_DIAMETER]->setColor(Qt::black);
	curvePreferences[FeatherCurveType::X_Y]->setColor( Qt::black);
	curvePreferences[FeatherCurveType::LOW_ORIGINAL]->setColor( Qt::cyan );
	curvePreferences[FeatherCurveType::LOW_WEIGHTED]->setColor( "#008080");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_HIGH]->setColor("#66CDAA");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_HIGH_WEIGHTED]->setColor(Qt::green);
	//curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY]->setColor("#AFEEEE");
	//curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED]->setColor( "#5F93A0");
	curvePreferences[FeatherCurveType::HIGH_ORIGINAL]->setColor( Qt::magenta);
	curvePreferences[FeatherCurveType::HIGH_WEIGHTED]->setColor("#9370DB");
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW]->setColor( "#800080");
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED]->setColor("#DDA0DD");
	curvePreferences[FeatherCurveType::DIRTY_ORIGINAL]->setColor( "#BDB76B");
	curvePreferences[FeatherCurveType::DIRTY_WEIGHTED]->setColor("#DAA520");
	curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW]->setColor( "#A0522D");
	curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED]->setColor("#D2B48C");
	curvePreferences[FeatherCurveType::SUM_LOW_HIGH]->setColor( Qt::blue );

	//Now the names of the curves
	curvePreferences[FeatherCurveType::WEIGHT_LOW]->setName( "Weight Low" );
	curvePreferences[FeatherCurveType::WEIGHT_HIGH]->setName( "Weight High" );
	curvePreferences[FeatherCurveType::LOW_ORIGINAL]->setName( "Low Data" );
	curvePreferences[FeatherCurveType::LOW_WEIGHTED]->setName( "Low Data, Weighted & Scaled");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_HIGH]->setName("Low Data x High Beam");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_HIGH_WEIGHTED]->setName("Low Data x High Beam, Weighted & Scaled");
	//curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY]->setName("Low x Dirty");
	//curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED]->setName( "Low x Dirty, Weighted/Scaled");
	curvePreferences[FeatherCurveType::HIGH_ORIGINAL]->setName( "High Data");
	curvePreferences[FeatherCurveType::HIGH_WEIGHTED]->setName("High Data, Weighted");
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW]->setName( "High Data x Low Beam");
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED]->setName("High Data x Low Beam, Weighted");
	curvePreferences[FeatherCurveType::DIRTY_ORIGINAL]->setName( "Dirty Data");
	curvePreferences[FeatherCurveType::DIRTY_WEIGHTED]->setName("Dirty Data, Weighted");
	curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW]->setName( "Dirty Data x Low Beam");
	curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED]->setName("Dirty Data x Low Beam, Weighted");
	curvePreferences[FeatherCurveType::SUM_LOW_HIGH]->setName( "Sum" );

	//Which curves should be visible
	curvePreferences[FeatherCurveType::SUM_LOW_HIGH]->setDisplayed( true );
	curvePreferences[FeatherCurveType::X_Y]->setDisplayed( true );
	curvePreferences[FeatherCurveType::LOW_WEIGHTED]->setDisplayed( true );
	curvePreferences[FeatherCurveType::HIGH_WEIGHTED]->setDisplayed( true );
	curvePreferences[FeatherCurveType::WEIGHT_LOW]->setDisplayed( true );
	curvePreferences[FeatherCurveType::WEIGHT_HIGH]->setDisplayed( true );
	curvePreferences[FeatherCurveType::DISH_DIAMETER]->setDisplayed( true );
	curvePreferences[FeatherCurveType::ZOOM]->setDisplayed( true );
	curvePreferences[FeatherCurveType::ZOOM]->setDisplayHidden();

	//Which curves are not eligible for the scatter plot
	curvePreferences[FeatherCurveType::X_Y]->setScatterEligible( false );
	curvePreferences[FeatherCurveType::ZOOM]->setScatterEligible( false );
	curvePreferences[FeatherCurveType::DISH_DIAMETER]->setScatterEligible( false );
	curvePreferences[FeatherCurveType::WEIGHT_LOW]->setScatterEligible( false );
	curvePreferences[FeatherCurveType::WEIGHT_HIGH]->setScatterEligible( false );

}



void PreferencesColor::addCurvePreferences(){
	addCurvePreference( ui.weightLowPreferences, FeatherCurveType::WEIGHT_LOW );
	addCurvePreference( ui.weightHighPreferences, FeatherCurveType::WEIGHT_HIGH );
	addCurvePreference( ui.zoomPreferences, FeatherCurveType::ZOOM);
	addCurvePreference( ui.dishDiameterPreferences, FeatherCurveType::DISH_DIAMETER);
	addCurvePreference( ui.scatterXYPreferences, FeatherCurveType::X_Y);
	addCurvePreference( ui.lowOriginalPreferences, FeatherCurveType::LOW_ORIGINAL );
	addCurvePreference( ui.lowWeightedPreferences, FeatherCurveType::LOW_WEIGHTED );
	addCurvePreference( ui.lowConvolvedHighPreferences, FeatherCurveType::LOW_CONVOLVED_HIGH );
	addCurvePreference( ui.lowConvolvedHighWeightedPreferences, FeatherCurveType::LOW_CONVOLVED_HIGH_WEIGHTED );
	//addCurvePreference( ui.lowConvolvedDirtyPreferences, FeatherCurveType::LOW_CONVOLVED_DIRTY );
	//addCurvePreference( ui.lowConvolvedDirtyWeightedPreferences, FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED );
	addCurvePreference( ui.highOriginalPreferences, FeatherCurveType::HIGH_ORIGINAL );
	addCurvePreference( ui.highWeightedPreferences, FeatherCurveType::HIGH_WEIGHTED );
	addCurvePreference( ui.highConvolvedLowPreferences, FeatherCurveType::HIGH_CONVOLVED_LOW );
	addCurvePreference( ui.highConvolvedLowWeightedPreferences, FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED );
	addCurvePreference( ui.dirtyOriginalPreferences, FeatherCurveType::DIRTY_ORIGINAL );
	addCurvePreference( ui.dirtyWeightedPreferences, FeatherCurveType::DIRTY_WEIGHTED );
	addCurvePreference( ui.dirtyConvolvedLowPreferences, FeatherCurveType::DIRTY_CONVOLVED_LOW );
	addCurvePreference( ui.dirtyConvolvedLowWeightedPreferences, FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED );
	addCurvePreference( ui.sliceSumPreferences, FeatherCurveType::SUM_LOW_HIGH );
}

void PreferencesColor::addCurvePreference( QWidget* holder, CurveType index ){
	QLayout* layout = new QHBoxLayout();
	layout->addWidget( curvePreferences[index]);
	holder->setLayout( layout );
}

QList<QString> PreferencesColor::getCurveNames() {
	int preferenceCount = curvePreferences.size();
	QList<QString> curveNames;
	for ( int i = 0; i < preferenceCount; i++ ){
		CurveType index = static_cast<CurveType>(i);
		bool displayed = curvePreferences[index]->isDisplayedGUI();
		bool scatter = curvePreferences[index]->isScatterEligible();
		bool enabled = curvePreferences[index]->isEnabled();
		if ( displayed && scatter && enabled ){
			curveNames.append( curvePreferences[index]->getName());
		}
	}
	return curveNames;
}

void PreferencesColor::populateScatterAxisX(){
	QList<QString> curveNames = getCurveNames();
	int selectedIndex = 0;
	QString defaultCurve = curvePreferences[scatterXIndex]->getName();
	int defaultIndex = curveNames.indexOf( defaultCurve );
	if ( defaultIndex >= 0 ){
		selectedIndex = defaultIndex;
	}

	//Populate the xAxis combo
	int nameCount = curveNames.size();
	ui.scatterXAxisCombo->clear();
	QString selectedName = ui.scatterXAxisCombo->currentText();
	for ( int i = 0; i < nameCount; i++ ){
		ui.scatterXAxisCombo->addItem( curveNames[i] );
		if ( selectedName == curveNames[i] ){
			selectedIndex = i;
		}
	}
	ui.scatterXAxisCombo->setCurrentIndex( selectedIndex );
}

void PreferencesColor::populateScatterAxisY(){

	//Populate the yAxis Combo taking care not to add the one selected on the
	//xAxis
	QString xName = ui.scatterXAxisCombo->currentText();

	//Get the ones that were previously selected so we can restore the
	//user's choices if possible.
	QList<QListWidgetItem*> yItems = ui.scatterYAxisList->selectedItems();
	int yNameCount = yItems.size();
	QList<QString> yNames;
	for ( int i = 0; i < yNameCount; i++ ){
		yNames.append( yItems[i]->text() );
	}

	//Get the list of available curves.
	QList<QString> curveNames = getCurveNames();

	//If we didn't have any curves that were previously selected, use the
	//defaults.
	if ( yNames.size() == 0  && curveNames.size() > 0 ){
		int yCount = scatterYIndices.size();
		for ( int i = 0; i < yCount; i++ ){
			CurveType cType = static_cast<CurveType>(i);
			yNames.append( curvePreferences[cType]->getName());
		}
	}

	//Remove the previous list
	ui.scatterYAxisList->clear();

	//Add the new list back in, selecting any that were previously selected.
	int nameCount = curveNames.size();
	int currentRow = 0;
	for ( int i = 0; i < nameCount; i++ ){
		if ( curveNames[i] != xName ){
			ui.scatterYAxisList->addItem( curveNames[i] );
			if ( yNames.contains(curveNames[i])){
				QListWidgetItem* item = ui.scatterYAxisList->item( currentRow );
				ui.scatterYAxisList->setItemSelected( item, true );
			}
			currentRow++;
		}
	}

	//We still might now have anything selected, so we just default in such
	//a case to the first item in the list.
	QList<QListWidgetItem*> selectedNames = ui.scatterYAxisList->selectedItems();
	if ( selectedNames.size() == 0 ){
		QListWidgetItem* item = ui.scatterYAxisList->item(0);
		if ( item != NULL ){
			ui.scatterYAxisList->setItemSelected( item, true );
		}
	}
}

void PreferencesColor::initializeUser(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( Preferences::ORGANIZATION, Preferences::APPLICATION );
	QList<CurveType> keys = curvePreferences.keys();
	for ( QList<CurveType>::iterator iter = keys.begin(); iter != keys.end(); iter++ ){
		curvePreferences[*iter]->initialize( settings );
	}
	QString scatterXStr = QString::number( scatterXIndex );
	scatterXIndex = static_cast<CurveType>(settings.value( SCATTER_X_CURVE, scatterXStr).toInt());

	int scatterYCount = settings.value( SCATTER_Y_CURVES, "0").toInt();
	if ( scatterYCount > 0 ){
		scatterYIndices.clear();
		for ( int i = 0; i < scatterYCount; i++ ){
			QString key = SCATTER_Y_CURVES+QString::number(i);
			int index = settings.value( key, "-1").toInt();
			if ( index >= 0 ){
				scatterYIndices.append( static_cast<CurveType>(index) );
			}
		}
	}
	reset();
}


QMap<PreferencesColor::CurveType,CurveDisplay> PreferencesColor::getFunctionColors( ) const {
	QMap<PreferencesColor::CurveType, CurveDisplay> curveMap;
	QList<CurveType> curveID = curvePreferences.keys();
	for (QList<CurveType>::iterator iter = curveID.begin();
			iter != curveID.end(); iter++ ){
		curveMap.insert( (*iter), curvePreferences[*iter]->getFunctionPreferences());
	}
	return curveMap;
}

FeatherCurveType::CurveType PreferencesColor::getType( const QString& title ) const {
	CurveType curveType = FeatherCurveType::CURVES_END;
	QList<CurveType> curveIDs = curvePreferences.keys();
	for (QList<CurveType>::iterator iter = curveIDs.begin();
					iter != curveIDs.end(); iter++ ){
		QString curveTitle = curvePreferences[*iter]->getName();
		if ( curveTitle == title ){
			curveType = *iter;
			break;
		}
	}
	return curveType;
}

FeatherCurveType::CurveType PreferencesColor::getScatterXCurve() const {
	QString xTitle = ui.scatterXAxisCombo->currentText();
	CurveType xType = getType( xTitle );
	return xType;
}

QList<FeatherCurveType::CurveType> PreferencesColor::getScatterYCurve() const {
	QList<CurveType> yCurves;
	QList<QListWidgetItem*> yCurveItems = ui.scatterYAxisList->selectedItems();
	for ( QList<QListWidgetItem*>::iterator iter = yCurveItems.begin();
			iter != yCurveItems.end(); iter++ ){
		QString curveTitle = (*iter)->text();
		CurveType yType = getType( curveTitle );
		if ( yType != FeatherCurveType::CURVES_END ){
			yCurves.append( yType );
		}
	}
	return yCurves;
}

void PreferencesColor::setDirtyEnabled( bool enabled ){
	ui.dirtyGroupBox->setEnabled( enabled );
	//curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY]->setEnabled( enabled );
	//curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED]->setEnabled( enabled );
	if ( !enabled ){
		curvePreferences[FeatherCurveType::DIRTY_ORIGINAL]->setDisplayed( false );
		curvePreferences[FeatherCurveType::DIRTY_WEIGHTED]->setDisplayed( false );
		curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW]->setDisplayed( false );
		curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED]->setDisplayed( false );
		//curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY]->setDisplayed( false );
		//curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED]->setDisplayed( false );
	}
	//Reset the curves that can be selected for the scatter
	//axis based on whether or not there is a dirty image.
	populateScatterAxes();
}

FeatherCurveType::CurveType PreferencesColor::findCurve( const QString& title ) const {
	CurveType cType = FeatherCurveType::CURVES_END;
	int count = curvePreferences.size();
	for ( int i = 0; i < count; i++ ){
		CurveType curveType = static_cast<CurveType>(i);
		QString curveName = curvePreferences[curveType]->getName();
		if ( curveName == title ){
			cType = curveType;
			break;
		}
	}
	return cType;
}

void PreferencesColor::resetScatterSettings(){
	QString xCurveName = curvePreferences[scatterXIndex]->getName();
	int comboIndex = ui.scatterXAxisCombo->findText( xCurveName);
	if ( comboIndex >= 0 ){
		ui.scatterXAxisCombo->setCurrentIndex( comboIndex );
	}

	int count = scatterYIndices.size();
	for ( int i = 0; i < count; i++ ){
		QString yCurveName = curvePreferences[scatterYIndices[i]]->getName();
		QList<QListWidgetItem*> matchingItems =
				ui.scatterYAxisList->findItems( yCurveName, Qt::MatchFixedString);
		if ( matchingItems.size() > 0 ){
			ui.scatterYAxisList->setItemSelected( matchingItems[0], true );
		}
		else {
			QListWidgetItem* firstItem = ui.scatterYAxisList->item( 0 );
			ui.scatterYAxisList->setItemSelected( firstItem, true );
		}
	}
}

void PreferencesColor::saveScatterSettings(){
	scatterYIndices.clear();
	QList<QListWidgetItem*> selectedItems = ui.scatterYAxisList->selectedItems();
	int count = selectedItems.size();
	for ( int i = 0; i < count; i++ ){
		QString curveName = selectedItems[i]->text();
		CurveType cType = findCurve( curveName );
		if ( cType != FeatherCurveType::CURVES_END ){
			scatterYIndices.append( cType );
		}
	}

	QString xAxisStr = ui.scatterXAxisCombo->currentText();
	CurveType cType = findCurve( xAxisStr );
	if ( cType != FeatherCurveType::CURVES_END ){
		scatterXIndex = cType;
	}
}

void PreferencesColor::persist(){
	saveScatterSettings();

	QSettings settings( Preferences::ORGANIZATION, Preferences::APPLICATION );
	QList<CurveType> keys = curvePreferences.keys();
	for ( QList<CurveType>::iterator iter = keys.begin(); iter != keys.end(); iter++ ){
		curvePreferences[*iter]->persist( settings );
	}

	QString scatterXStr = QString::number( scatterXIndex );
	settings.setValue( SCATTER_X_CURVE, scatterXStr);

	int scatterYCount = scatterYIndices.count();
	for ( int i = 0; i < scatterYCount; i++ ){
		QString key = SCATTER_Y_CURVES+QString::number(i);
		QString valueStr = QString::number(scatterYIndices[i]);
	}
}


void PreferencesColor::colorsAccepted(){
	persist();
	emit colorsChanged();
}

void PreferencesColor::colorsRejected(){
	reset();
	QDialog::close();
}

void PreferencesColor::reset(){
	QList<CurveType> keys = curvePreferences.keys();
	for ( QList<CurveType>::iterator iter = keys.begin(); iter != keys.end(); iter++ ){
		curvePreferences[*iter]->reset();
	}
	resetScatterSettings();
}


PreferencesColor::~PreferencesColor(){
	QList<CurveType> keys = curvePreferences.keys();
	for ( QList<CurveType>::iterator iter = keys.begin(); iter != keys.end(); iter++ ){
		PreferencesFunction* funct = curvePreferences.take(*iter);
		delete funct;
	}
}

}
