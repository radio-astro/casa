//# TBOptions.cc: Widget to edit the browsing options.
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
//# $Id: $
#include <casaqt/QtBrowser/TBOptions.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>

namespace casa {

///////////////////////////
// TBOPTIONS DEFINITIONS //
///////////////////////////

// Constructors/Destructors //

TBOptions::TBOptions() : QDialog() {
    setupUi(this);
    restoreDefaults();
    connect(defaultsButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

TBOptions::~TBOptions() {
    
}

// Public Methods //

bool TBOptions::saveView() {
    return viewBox->isChecked();
}

void TBOptions::setSaveView(bool sv) {
    viewBox->setChecked(sv);
}

int TBOptions::debugLevel() {
    switch(debugChooser->currentIndex()) {
    case 0: return TBConstants::DEBUG_OFF;
    case 1: return TBConstants::DEBUG_LOW;
    case 2: return TBConstants::DEBUG_MED;
    case 3: return TBConstants::DEBUG_HIGH;
    default: return TBConstants::DEBUG_OFF;
    }
}

void TBOptions::setDebugLevel(int newLevel) {
	if(newLevel == TBConstants::DEBUG_LOW)
		debugChooser->setCurrentIndex(1);
	else if(newLevel == TBConstants::DEBUG_MED)
		debugChooser->setCurrentIndex(2);
	else if(newLevel == TBConstants::DEBUG_HIGH)
		debugChooser->setCurrentIndex(3);
	else
		debugChooser->setCurrentIndex(0);
}

// Private Slots //

void TBOptions::restoreDefaults() {
    viewBox->setChecked(true);
    debugChooser->setCurrentIndex(0);
}

void TBOptions::save() {
    emit saveOptions(this);
}

void TBOptions::cancel() {
    close();
    delete this;
}

}
