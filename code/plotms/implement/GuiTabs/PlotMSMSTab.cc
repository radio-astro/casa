//# PlotMSMSTab.cc: Plot tab for MS parameters.
//# Copyright (C) 2009
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
#include <plotms/GuiTabs/PlotMSMSTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>

namespace casa {

/////////////////////////////
// PLOTMSMSTAB DEFINITIONS //
/////////////////////////////

// Constructors/Destructors//

PlotMSMSTab::PlotMSMSTab(PlotMSPlotter* parent) : PlotMSPlotSubtab(parent) {
    setupUi(this);
    
    // Setup widgets
    itsFileWidget_ = new QtFileWidget(true, false);
    QtUtilities::putInFrame(locationFrame, itsFileWidget_);
    
    itsSelectionWidget_ = new PlotMSSelectionWidget();
    QtUtilities::putInFrame(selectionFrame, itsSelectionWidget_);
    QtUtilities::putInScrollArea(selectionFrame);
    
    itsAveragingWidget_ = new PlotMSAveragingWidget();
    QtUtilities::putInFrame(averagingFrame, itsAveragingWidget_);
    
    itsLabelDefaults_.insert(locationLabel, locationLabel->text());
    itsLabelDefaults_.insert(selectionLabel, selectionLabel->text());
    itsLabelDefaults_.insert(averagingLabel, averagingLabel->text());
    
    // Connect widgets
    connect(itsFileWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsSelectionWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsAveragingWidget_, SIGNAL(changed()), SIGNAL(changed()));
}

PlotMSMSTab::~PlotMSMSTab() { }


// Public Methods //

void PlotMSMSTab::getValue(PlotMSPlotParameters& params) const {
    params.setFilename(itsFileWidget_->getFile());
    params.setSelection(itsSelectionWidget_->getValue());
    params.setAveraging(itsAveragingWidget_->getValue());
}

void PlotMSMSTab::setValue(const PlotMSPlotParameters& params) {
    itsFileWidget_->setFile(params.filename());
    itsSelectionWidget_->setValue(params.selection());
    itsAveragingWidget_->setValue(params.averaging());
}

void PlotMSMSTab::update(const PlotMSPlot& plot) {
    const PlotMSPlotParameters& params = plot.parameters();
    changedText(locationLabel, itsFileWidget_->getFile() != params.filename());
    changedText(selectionLabel,
            itsSelectionWidget_->getValue() != params.selection());
    changedText(averagingLabel,
            itsAveragingWidget_->getValue() != params.averaging());
}

}
