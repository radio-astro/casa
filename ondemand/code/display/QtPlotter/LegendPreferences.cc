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
#include "LegendPreferences.qo.h"
#include <display/QtPlotter/Util.h>
#include <display/QtPlotter/CanvasHolder.qo.h>
#include <QSettings>
#include <QListWidgetItem>

namespace casa {

	const QString LegendPreferences::LEGEND_VISIBLE="Legend Visible";
	const QString LegendPreferences::LEGEND_LOCATION="Legend Location";
	const QString LegendPreferences::LEGEND_COLOR_BAR="Legend Colorbar";

	LegendPreferences::LegendPreferences( CanvasHolder* holder,QWidget *parent)
		: QDialog(parent),  showLegendDefault( true ), showColorBarDefault( true ),
		  legendPositionDefault( CanvasHolder::CANVAS ), canvasHolder( holder ) {
		ui.setupUi(this);
		setWindowTitle( "Legend Preferences");

		connect( ui.showLegendCheckBox, SIGNAL(clicked()),
		         this, SLOT( legendVisibilityChanged()));
		connect( ui.locationComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT(legendLocationChanged(int)));
		connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

		QList<QString> legendLocations;
		legendLocations << "Top Left" << "Bottom" << "Right";
		for ( int i = 0; i < legendLocations.size(); i++ ) {
			ui.locationComboBox->addItem( legendLocations[i] );
		}

		//Read the user settings
		initializeUserPreferences();

		//Set up the canvas to use them.
		canvasLegendChange();
		colorBarVisibilityChange();

		//Initialize the labels for the curves
		initializeCurveLabels();
	}


	void LegendPreferences::initializeCurveLabels() {
		ui.curveTextList->clear();
		QList<QString> labelList = canvasHolder->getCurveLabels();
		for ( int i = 0; i < labelList.size(); i++ ) {
			QListWidgetItem* item = new QListWidgetItem( labelList[i], ui.curveTextList );
			item->setFlags( item->flags() | Qt::ItemIsEditable );
		}
	}


	void LegendPreferences::legendVisibilityChanged() {
		bool visible = ui.showLegendCheckBox-> isChecked();
		ui.locationComboBox->setEnabled( visible );
		ui.colorBarCheckBox->setEnabled( visible );
		ui.curveTextList->setEnabled( visible );
	}

	void LegendPreferences::initializeUserPreferences() {
		//Read information from the persistent legend settings
		QSettings settings( Util::ORGANIZATION, Util::APPLICATION );

		//If the user has a custom color scheme, read it in.
		bool legendVisible = settings.value(LEGEND_VISIBLE, showLegendDefault).toBool();
		ui.showLegendCheckBox->setChecked( legendVisible );

		int legendPosition = settings.value( LEGEND_LOCATION, legendPositionDefault).toInt();
		ui.locationComboBox->setCurrentIndex( legendPosition );

		bool showColorBar = settings.value( LEGEND_COLOR_BAR, showColorBarDefault).toBool();
		ui.colorBarCheckBox->setChecked( showColorBar );

		legendVisibilityChanged( );
	}

	void LegendPreferences::show() {
		//Initialize our displayed list with the curve names
		initializeCurveLabels();

		//Now do the normal stuff
		QDialog::show();
	}
	void LegendPreferences::persist() {
		//Store legend preferences indicated by the user
		QSettings settings( Util::ORGANIZATION, Util::APPLICATION );
		settings.clear();

		bool showLegend = ui.showLegendCheckBox->isChecked();
		settings.setValue( LEGEND_VISIBLE, showLegend );

		int positionIndex = ui.locationComboBox->currentIndex();
		settings.setValue( LEGEND_LOCATION, positionIndex );

		bool showColorBar = ui.colorBarCheckBox->isChecked();
		settings.setValue( LEGEND_COLOR_BAR, showColorBar );
	}

	void LegendPreferences::colorBarVisibilityChange() {
		bool showColorBar = ui.colorBarCheckBox->isChecked();
		canvasHolder->setColorBarVisibility( showColorBar );
	}

	void LegendPreferences::legendLocationChanged( int index ) {
		bool externalLegend = true;
		if ( index == CanvasHolder::CANVAS) {
			externalLegend = false;
			ui.colorBarCheckBox->setChecked( false );
		}
		ui.colorBarCheckBox->setEnabled( externalLegend );
	}

	void LegendPreferences::canvasLegendChange() {
		bool showLegend = ui.showLegendCheckBox->isChecked();
		canvasHolder->setShowLegend( showLegend );

		int legendLocation = ui.locationComboBox->currentIndex();
		canvasHolder->setLegendPosition( legendLocation );
	}

	void LegendPreferences::curveLabelChange() {
		int curveCount = ui.curveTextList->count();
		QList<QString> curveLabels;
		for ( int i = 0; i < curveCount; i++ ) {
			QListWidgetItem* listItem = ui.curveTextList->item(i);
			if ( listItem != NULL ) {
				curveLabels.append(listItem->text());
			}
		}
		canvasHolder->setCurveLabels( curveLabels );
	}

	void LegendPreferences::accept() {
		//Tell the canvas about the legend changes
		canvasLegendChange();
		colorBarVisibilityChange();
		curveLabelChange();

		//Persist the legend changes
		persist();

		//Close the dialog
		this->hide();
	}

	void LegendPreferences::reject() {
		initializeUserPreferences();

		//Close the dialog
		this->hide();
	}


	LegendPreferences::~LegendPreferences() {

	}
}
