//# PlotMSCalibrationWidget.cc: GUI widget for editing a PlotMSCalibration
//# Copyright (C) 2015
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
#include <plotms/Gui/PlotMSCalibrationWidget.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/GuiTabs/PlotMSTab.qo.h>

namespace casa {

PlotMSCalibrationWidget::PlotMSCalibrationWidget(QWidget* parent) :
        QtEditingWidget(parent), itsFlag_(true) {    

  // Setup the gui
  setupUi(this);

  // Connect widgets
  connect(calOff, SIGNAL(toggled(bool)), SIGNAL(changed()));
  connect(calOn, SIGNAL(toggled(bool)), SIGNAL(changed()));
}

PlotMSCalibrationWidget::~PlotMSCalibrationWidget() { }


PlotMSCalibration PlotMSCalibrationWidget::getValue() const {
    PlotMSCalibration calib;
    calib.setUseCallib(calOn->isChecked());
    return calib;
}

void PlotMSCalibrationWidget::setValue(const PlotMSCalibration& calibration ) {
    // Set widget values
    Bool calibrationOn = calibration.useCallib();
    calOn->setChecked(calibrationOn);
    calOff->setChecked(!calibrationOn);
}

}
