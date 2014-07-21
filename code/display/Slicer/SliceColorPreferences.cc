//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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
#include "SliceColorPreferences.qo.h"
#include <display/QtPlotter/ColorSummaryDelegate.h>
#include <QColorDialog>
#include <QMessageBox>
#include <QDebug>

namespace casa {

	const QString SliceColorPreferences::APPLICATION = "1D Slice Tool";
	const QString SliceColorPreferences::ORGANIZATION = "NRAO/CASA";
	const QString SliceColorPreferences::POLYLINE_UNIT = "Polyline Unit";
	const QString SliceColorPreferences::ACCUMULATED_COLOR = "Accumulated Color";
	const QString SliceColorPreferences::ACCUMULATED_COLOR_COUNT = "Accumulated Slice Color Count";
	const QString SliceColorPreferences::VIEWER_COLORS = "Viewer Colors";

	SliceColorPreferences::SliceColorPreferences(QWidget *parent)
		: QDialog(parent), POLYGONAL_CHAIN( "Polygonal Chain"),
		  LINE_SEGMENT( "Line Segment") {
		ui.setupUi(this);
		setWindowTitle( "Spatial Profile Color Preferences");

		viewerColors = true;
		accumulateColorList << "#40E0D0" << "#FF69B4" << "#00FF00" << "#FF8C00" << "#BA55D3" <<
		                    "#A52A2A" << "#808080" << "#FF6347" << "#FFFF00";
		ui.accumulatedColorList->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.accumulatedColorList->setSelectionMode( QAbstractItemView::SingleSelection );
		ui.accumulatedColorList->setItemDelegate( new ColorSummaryDelegate( this ));

		polylineUnit = true;
		colorUnits << POLYGONAL_CHAIN << LINE_SEGMENT;
		for ( int i = 0; i < colorUnits.size(); i++ ) {
			ui.colorUnitComboBox->addItem( colorUnits[i] );
		}

		initializeUserColors();
		resetColors();
		ui.viewerColorsCheckBox->setChecked( viewerColors );
		useViewerColorsChanged( viewerColors );

		connect( ui.viewerColorsCheckBox, SIGNAL(toggled(bool)), this, SLOT(useViewerColorsChanged(bool)));
		connect( ui.addButton, SIGNAL(clicked()), this, SLOT(addColorAccumulated()));
		connect( ui.deleteButton, SIGNAL(clicked()), this, SLOT(removeColorAccumulated()));
		connect( ui.okButton, SIGNAL(clicked()), this, SLOT(colorsAccepted()));
		connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(colorsRejected()));
	}

	void SliceColorPreferences::useViewerColorsChanged( bool viewerColors ) {
		if ( viewerColors ) {
			ui.colorUnitComboBox->setCurrentIndex( colorUnits.indexOf( POLYGONAL_CHAIN) );
		}
		ui.colorUnitComboBox->setEnabled( !viewerColors );
		ui.addButton->setEnabled( !viewerColors );
		ui.deleteButton->setEnabled( !viewerColors );
		ui.accumulatedColorList->setEnabled( !viewerColors );
	}

	bool SliceColorPreferences::isViewerColors() const {
		return viewerColors;
	}

	bool SliceColorPreferences::isPolylineUnit() const {
		return polylineUnit;
	}

	QList<QColor> SliceColorPreferences::getAccumulatedSliceColors() const {
		QList<QColor> copy;
		for ( int i = 0; i < accumulateColorList.size(); i++  ) {
			QColor color = QColor( accumulateColorList[i]);
			copy.append( color );
		}
		return copy;
	}



	void SliceColorPreferences::initializeUserColors() {
		//Only use the default values passed in if the user has not indicated
		//any preferences.
		QSettings settings( ORGANIZATION, APPLICATION );

		viewerColors = settings.value( VIEWER_COLORS, viewerColors ).toBool();
		polylineUnit = settings.value( POLYLINE_UNIT, polylineUnit).toBool();

		//If the user has specified an accumulated color scheme, read it in.
		if ( settings.contains( ACCUMULATED_COLOR_COUNT) ) {
			readCustomColor( settings, ACCUMULATED_COLOR_COUNT, ACCUMULATED_COLOR, accumulateColorList );
		}
	}

	void SliceColorPreferences::readCustomColor( QSettings& settings,
	        const QString& countKey, const QString& baseLookup,
	        QList<QString>& colorList ) {
		colorList.clear();
		int colorCount = settings.value( countKey, 0 ).toInt();
		for ( int i = 0; i < colorCount; i++ ) {
			QString lookupStr = baseLookup + QString::number(i);
			QString colorName = settings.value( lookupStr ).toString();
			colorList.append( colorName );
		}
	}



	void SliceColorPreferences::resetColors() {
		ui.viewerColorsCheckBox->setChecked( viewerColors );
		if ( polylineUnit ) {
			int chainIndex = colorUnits.indexOf( POLYGONAL_CHAIN );
			ui.colorUnitComboBox->setCurrentIndex( chainIndex );
		} else {
			int unitIndex = colorUnits.indexOf( LINE_SEGMENT );
			ui.colorUnitComboBox->setCurrentIndex( unitIndex );
		}
		populateAccumulateColors();
	}



	void SliceColorPreferences::populateAccumulateColors( ) {
		ui.accumulatedColorList->clear();
		for( int i = 0; i < accumulateColorList.size(); i++ ) {
			QColor itemColor;
			itemColor.setNamedColor( accumulateColorList[i] );
			addColorListItem( ui.accumulatedColorList, itemColor );
		}
	}

	void SliceColorPreferences::addColorListItem( QListWidget* list, const QColor& listColor ) {
		QListWidgetItem* listItem = new QListWidgetItem( list  );
		listItem->setBackground( listColor );
		list->addItem( listItem );
	}

	void SliceColorPreferences::addColorAccumulated() {
		QColor selectedColor = QColorDialog::getColor( Qt::white, this );
		if ( selectedColor.isValid() ) {
			addColorListItem( ui.accumulatedColorList, selectedColor );
		}
	}

	void SliceColorPreferences::removeColorAccumulated() {
		int row = ui.accumulatedColorList->currentRow();
		if ( row >= 0 ) {
			QListWidgetItem* listItem = ui.accumulatedColorList->takeItem( row );
			if ( listItem != NULL ) {
				delete listItem;
			}
		} else {
			QString msg( "Please select a color in the list to delete.");
			QMessageBox::warning( this, "No Color Selected", msg );
		}
	}


	void SliceColorPreferences::colorsAccepted() {
		persistColors();
		QDialog::close();
		emit colorsChanged();
	}

	void SliceColorPreferences::colorsRejected() {
		resetColors();
		QDialog::close();
	}

	void SliceColorPreferences::persistColors() {
		//Copy the colors from the buttons into color variables

		//Save the colors in the persistent settings.
		QSettings settings( ORGANIZATION, APPLICATION );
		settings.clear();

		//Use viewer colors
		viewerColors = ui.viewerColorsCheckBox->isChecked();
		settings.setValue( VIEWER_COLORS, viewerColors );

		//Polygonal color unit.
		int selectedIndex = ui.colorUnitComboBox->currentIndex();
		int polylineIndex = colorUnits.indexOf( POLYGONAL_CHAIN );
		if ( polylineIndex == selectedIndex ) {
			polylineUnit = true;
		} else {
			polylineUnit = false;
		}
		settings.setValue( POLYLINE_UNIT, polylineUnit );

		//Accumulated slice colors
		accumulateColorList.clear();
		int colorCount = ui.accumulatedColorList->count();
		settings.setValue( ACCUMULATED_COLOR_COUNT, colorCount );
		for ( int i = 0; i < colorCount; i++ ) {
			QString persistStr = ACCUMULATED_COLOR + QString::number(i);
			QListWidgetItem* listItem = ui.accumulatedColorList->item(i);
			if ( listItem != NULL ) {
				QColor listColor = listItem->backgroundColor();
				QString colorStr = listColor.name();
				settings.setValue( persistStr, colorStr );
				accumulateColorList.append( colorStr );
			}
		}
	}

	SliceColorPreferences::~SliceColorPreferences() {

	}
}
