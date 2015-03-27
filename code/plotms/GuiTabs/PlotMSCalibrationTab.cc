//# PlotMSCalibrationTab.cc: Plot tab for calibration parameters.
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
#include <plotms/GuiTabs/PlotMSCalibrationTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Gui/PlotMSCalibrationWidget.qo.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

//////////////////////////////////////////
// PLOTMSCALIBRATIONTAB DEFINITIONS //
//////////////////////////////////////////

// Constructors/Destructors//

PlotMSCalibrationTab::PlotMSCalibrationTab(PlotMSPlotTab* plotTab, 
						   PlotMSPlotter* parent) :
        PlotMSPlotSubtab(plotTab, parent) {
    setupUi(this);
    
    // Setup widgets
    itsCalibrationWidget_ = new PlotMSCalibrationWidget();
    QtUtilities::putInFrame(calibFrame, itsCalibrationWidget_);
    
    itsLabelDefaults_.insert(calibLabel, calibLabel->text());
    
    // Connect widgets
    connect(itsCalibrationWidget_, SIGNAL(changed()), SIGNAL(changed()));
    
}

PlotMSCalibrationTab::~PlotMSCalibrationTab() { }


// Public Methods //

void PlotMSCalibrationTab::getValue(PlotMSPlotParameters& params) const {
    PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
    if(d == NULL) {
        params.setGroup<PMS_PP_MSData>();
        d = params.typedGroup<PMS_PP_MSData>();
    }

    // Just deal with useCallib for now;  later add file widget for callib filename
    // Get MSData calibration so you don't wipe out filename and record
    PlotMSCalibration calibparams = d->calibration();
    PlotMSCalibration guiCalib = itsCalibrationWidget_->getValue();
    // check if calibration library record is set, else can't use it!
    if ((guiCalib.useCallib()) && (d->calibration().callibRec().nfields()==0)) {
	String message("Calibration library has not been set from the casapy console.");
	this->itsPlotter_->showError( message, "callib not set", true);
	calibparams.setUseCallib(false);
	d->setCalibration(calibparams);
    }
    else {
	calibparams.setUseCallib(guiCalib.useCallib());
	d->setCalibration(calibparams);
    }
}

void PlotMSCalibrationTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
    if(d == NULL) return;
    
    itsCalibrationWidget_->setValue(d->calibration());
}

void PlotMSCalibrationTab::update(const PlotMSPlot& plot) {
    const PMS_PP_MSData* d = plot.parameters().typedGroup<PMS_PP_MSData>();
    if(d == NULL) return;

    highlightWidgetText(calibLabel,
		itsCalibrationWidget_->getValue() != d->calibration());
}

}
