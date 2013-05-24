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

namespace casa {

PreferencesColor::PreferencesColor(QWidget *parent)
    : QDialog(parent){

	ui.setupUi(this);

	setWindowTitle( "Feather Plot Curve Preferences");
	setModal( false );

	initializeCurvePreferences();
	initializeUser();
	reset();

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
}

void PreferencesColor::initializeCurvePreferences(){
	for ( int i = 0; i < FeatherCurveType::CURVES_END; i++ ){
		PreferencesFunction* functionPreferences = new PreferencesFunction( i, this );
		curvePreferences.insert( static_cast<CurveType>(i), functionPreferences );
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
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY]->setColor("#AFEEEE");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED]->setColor( "#5F93A0");
	curvePreferences[FeatherCurveType::HIGH_ORIGINAL]->setColor( Qt::magenta);
	curvePreferences[FeatherCurveType::HIGH_WEIGHTED]->setColor("#9370DB");
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW]->setColor( "#800080");
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED]->setColor("#DDA0DD");
	curvePreferences[FeatherCurveType::DIRTY_ORIGINAL]->setColor( "#BDB76B");
	curvePreferences[FeatherCurveType::DIRTY_WEIGHTED]->setColor("#DAA520");
	curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW]->setColor( "#A0522D");
	curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED]->setColor("#D2B48C");
	curvePreferences[FeatherCurveType::SCATTER_LOW_HIGH]->setColor( Qt::blue );
	curvePreferences[FeatherCurveType::SUM_LOW_HIGH]->setColor( Qt::blue );

	//Now the names of the curves
	curvePreferences[FeatherCurveType::WEIGHT_LOW]->setName( "Weight Low" );
	curvePreferences[FeatherCurveType::WEIGHT_HIGH]->setName( "Weight High" );
	curvePreferences[FeatherCurveType::LOW_ORIGINAL]->setName( "Low" );
	curvePreferences[FeatherCurveType::LOW_WEIGHTED]->setName( "Low, Weighted/Scaled");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_HIGH]->setName("Low x High");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_HIGH_WEIGHTED]->setName("Low x High, Weighted/Scaled");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY]->setName("Low x Dirty");
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED]->setName( "Low x Dirty, Weighted/Scaled");
	curvePreferences[FeatherCurveType::HIGH_ORIGINAL]->setName( "High");
	curvePreferences[FeatherCurveType::HIGH_WEIGHTED]->setName("High, Weighted/Scaled");
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW]->setName( "High x Low");
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED]->setName("High x Low, Weighted/Scaled");
	curvePreferences[FeatherCurveType::DIRTY_ORIGINAL]->setName( "Dirty");
	curvePreferences[FeatherCurveType::DIRTY_WEIGHTED]->setName("Dirty, Weighted/Scaled");
	curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW]->setName( "Dirty x Low");
	curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED]->setName("Dirty x Low, Weighted/Scaled");
	curvePreferences[FeatherCurveType::SUM_LOW_HIGH]->setName( "Sum" );

	//Which curves should be visible
	curvePreferences[FeatherCurveType::SUM_LOW_HIGH]->setDisplayed( true );
	curvePreferences[FeatherCurveType::SCATTER_LOW_HIGH]->setDisplayed( true );
	curvePreferences[FeatherCurveType::X_Y]->setDisplayed( true );
	curvePreferences[FeatherCurveType::LOW_CONVOLVED_HIGH_WEIGHTED]->setDisplayed( true );
	curvePreferences[FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED]->setDisplayed( true );
	curvePreferences[FeatherCurveType::WEIGHT_LOW]->setDisplayed( true );
	curvePreferences[FeatherCurveType::WEIGHT_HIGH]->setDisplayed( true );
	curvePreferences[FeatherCurveType::DISH_DIAMETER]->setDisplayed( true );
	curvePreferences[FeatherCurveType::ZOOM]->setDisplayed( true );
	curvePreferences[FeatherCurveType::ZOOM]->setDisplayHidden();
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
	addCurvePreference( ui.lowConvolvedDirtyPreferences, FeatherCurveType::LOW_CONVOLVED_DIRTY );
	addCurvePreference( ui.lowConvolvedDirtyWeightedPreferences, FeatherCurveType::LOW_CONVOLVED_DIRTY_WEIGHTED );
	addCurvePreference( ui.highOriginalPreferences, FeatherCurveType::HIGH_ORIGINAL );
	addCurvePreference( ui.highWeightedPreferences, FeatherCurveType::HIGH_WEIGHTED );
	addCurvePreference( ui.highConvolvedLowPreferences, FeatherCurveType::HIGH_CONVOLVED_LOW );
	addCurvePreference( ui.highConvolvedLowWeightedPreferences, FeatherCurveType::HIGH_CONVOLVED_LOW_WEIGHTED );
	addCurvePreference( ui.dirtyOriginalPreferences, FeatherCurveType::DIRTY_ORIGINAL );
	addCurvePreference( ui.dirtyWeightedPreferences, FeatherCurveType::DIRTY_WEIGHTED );
	addCurvePreference( ui.dirtyConvolvedLowPreferences, FeatherCurveType::DIRTY_CONVOLVED_LOW );
	addCurvePreference( ui.dirtyConvolvedLowWeightedPreferences, FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED );
	addCurvePreference(ui.scatterLowHighPreferences, FeatherCurveType::SCATTER_LOW_HIGH);
	addCurvePreference( ui.sliceSumPreferences, FeatherCurveType::SUM_LOW_HIGH );

}

void PreferencesColor::addCurvePreference( QWidget* holder, CurveType index ){
	QLayout* layout = new QHBoxLayout();
	layout->addWidget( curvePreferences[index]);
	holder->setLayout( layout );
}

void PreferencesColor::initializeUser(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( Preferences::ORGANIZATION, Preferences::APPLICATION );
	QList<CurveType> keys = curvePreferences.keys();
	for ( QList<CurveType>::iterator iter = keys.begin(); iter != keys.end(); iter++ ){
		curvePreferences[*iter]->initialize( settings );
	}
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

void PreferencesColor::setDirtyEnabled( bool enabled ){
	ui.dirtyGroupBox->setEnabled( enabled );
	if ( !enabled ){
		curvePreferences[FeatherCurveType::DIRTY_ORIGINAL]->setDisplayed( false );
		curvePreferences[FeatherCurveType::DIRTY_WEIGHTED]->setDisplayed( false );
		curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW]->setDisplayed( false );
		curvePreferences[FeatherCurveType::DIRTY_CONVOLVED_LOW_WEIGHTED]->setDisplayed( false );
	}
}


void PreferencesColor::persist(){
	//Copy the colors from the buttons into the map.
	QSettings settings( Preferences::ORGANIZATION, Preferences::APPLICATION );
	QList<CurveType> keys = curvePreferences.keys();
	for ( QList<CurveType>::iterator iter = keys.begin(); iter != keys.end(); iter++ ){
		curvePreferences[*iter]->persist( settings );
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
}



PreferencesColor::~PreferencesColor(){
	QList<CurveType> keys = curvePreferences.keys();
	for ( QList<CurveType>::iterator iter = keys.begin(); iter != keys.end(); iter++ ){
		PreferencesFunction* funct = curvePreferences.take(*iter);
		delete funct;
	}
}

}
