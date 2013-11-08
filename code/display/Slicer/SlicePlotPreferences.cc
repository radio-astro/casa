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

#include "SlicePlotPreferences.qo.h"
#include <QSettings>

namespace casa {

	const QString SlicePlotPreferences::ORGANIZATION = "NRAO/CASA";
	const QString SlicePlotPreferences::APPLICATION = "Slice 1D";
	const QString SlicePlotPreferences::LINE_THICKNESS = "Line Thickness";
	const QString SlicePlotPreferences::MARKER_SIZE = "Marker Size";

	SlicePlotPreferences::SlicePlotPreferences(QWidget *parent)
		: QDialog(parent) {
		lineThickness = 1;
		markerSize = 5;

		ui.setupUi(this);
		ui.lineWidthSpinBox->setMinimum(1);
		ui.lineWidthSpinBox->setMaximum( 10 );
		ui.lineWidthSpinBox->setValue( lineThickness );

		ui.segmentMarkerSpinBox->setMinimum(1);
		ui.segmentMarkerSpinBox->setMaximum( 10 );
		ui.segmentMarkerSpinBox->setValue( markerSize );

		initializeCustomSettings();
		reset();

		connect( ui.okButton, SIGNAL(clicked()), this, SLOT(preferencesAccepted()));
		connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(preferencesRejected()));
	}

	void SlicePlotPreferences::initializeCustomSettings() {
		//Only use the default values passed in if the user has not indicated
		//any preferences.
		QSettings settings( ORGANIZATION, APPLICATION );
		lineThickness = settings.value( LINE_THICKNESS, lineThickness).toInt();
		markerSize = settings.value( MARKER_SIZE, markerSize).toInt();
	}

	int SlicePlotPreferences::getLineWidth() const {
		return lineThickness;
	}

	int SlicePlotPreferences::getMarkerSize() const {
		return markerSize;
	}

	void SlicePlotPreferences::preferencesAccepted() {
		persist();
		emit plotPreferencesChanged();
		this->close();
	}
	void SlicePlotPreferences::preferencesRejected() {
		reset();
		this->close();
	}

	void SlicePlotPreferences::reset() {
		ui.lineWidthSpinBox->setValue( lineThickness );
		ui.segmentMarkerSpinBox->setValue( markerSize );
	}

	void SlicePlotPreferences::persist() {
		QSettings settings( ORGANIZATION, APPLICATION );

		lineThickness = ui.lineWidthSpinBox->value();
		settings.setValue( LINE_THICKNESS, lineThickness );

		markerSize = ui.segmentMarkerSpinBox->value();
		settings.setValue( MARKER_SIZE, markerSize );
	}

	SlicePlotPreferences::~SlicePlotPreferences() {

	}
}
