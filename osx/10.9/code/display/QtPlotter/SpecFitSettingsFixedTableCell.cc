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
#include "SpecFitSettingsFixedTableCell.qo.h"

SpecFitSettingsFixedTableCell::SpecFitSettingsFixedTableCell(QWidget *parent)
	: QWidget(parent),
	  GAUSSIAN_ESTIMATE_FIXED_CENTER("c"),
	  GAUSSIAN_ESTIMATE_FIXED_PEAK( "p"),
	  GAUSSIAN_ESTIMATE_FIXED_FWHM( "f") {
	ui.setupUi(this);
}

QSize SpecFitSettingsFixedTableCell::sizeHint() const {
	QSize suggestedSize( 150, 35 );
	return suggestedSize;
}

void SpecFitSettingsFixedTableCell::setFixedCenter( bool fixed ) {
	ui.centerCheckBox->setChecked( fixed );
}

void SpecFitSettingsFixedTableCell::setFixedPeak( bool fixed ) {
	ui.peakCheckBox->setChecked( fixed );
}

void SpecFitSettingsFixedTableCell::setFixedFwhm( bool fixed ) {
	ui.fwhmCheckBox->setChecked( fixed );
}

bool SpecFitSettingsFixedTableCell::isCenterFixed() const {
	return ui.centerCheckBox->isChecked();
}

bool SpecFitSettingsFixedTableCell::isFWHMFixed() const {
	return ui.fwhmCheckBox->isChecked();
}

bool SpecFitSettingsFixedTableCell::isPeakFixed() const {
	return ui.peakCheckBox->isChecked();
}

QString SpecFitSettingsFixedTableCell::getFixedStr() const {
	QString fixedEstimateStr;
	if ( ui.centerCheckBox->isChecked() ) {
		fixedEstimateStr.append( GAUSSIAN_ESTIMATE_FIXED_CENTER );
	}
	if ( ui.peakCheckBox->isChecked() ) {
		fixedEstimateStr.append( GAUSSIAN_ESTIMATE_FIXED_PEAK );
	}
	if ( ui.fwhmCheckBox->isChecked() ) {
		fixedEstimateStr.append( GAUSSIAN_ESTIMATE_FIXED_FWHM );
	}
	return fixedEstimateStr;
}

void SpecFitSettingsFixedTableCell::clear() {
	ui.centerCheckBox->setChecked( false );
	ui.peakCheckBox->setChecked( false );
	ui.fwhmCheckBox->setChecked( false );
}


SpecFitSettingsFixedTableCell::~SpecFitSettingsFixedTableCell() {

}
