//# PlotMSDataTab.cc: Plot tab for data parameters.
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
#include <plotms/GuiTabs/PlotMSDataTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Gui/PlotMSAveragingWidget.qo.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/Gui/PlotMSSelectionWidget.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

///////////////////////////////
// PLOTMSDATATAB DEFINITIONS //
///////////////////////////////

// Constructors/Destructors//

PlotMSDataTab::PlotMSDataTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent) :
        PlotMSPlotSubtab(plotTab, parent) {
    setupUi(this);
    
    // Setup widgets
    itsFileWidget_ = new QtFileWidget(true, false);
    QtUtilities::putInFrame(locationFrame, itsFileWidget_);
    
    itsSelectionWidget_ = new PlotMSSelectionWidget();
    QtUtilities::putInFrame(selectionFrame, itsSelectionWidget_);
    QtUtilities::putInScrollArea(selectionFrame);
    
    itsAveragingWidget_ = new PlotMSAveragingWidget();
    QtUtilities::putInFrame(averagingFrame, itsAveragingWidget_);
    
    // Set up summary choices
    const vector<String>& types = PMS::summaryTypeStrings();
    for(unsigned int i = 0; i < types.size(); i++)
        MSDataTab::summaryType->addItem(types[i].c_str());
    
    itsLabelDefaults_.insert(locationLabel, locationLabel->text());
    itsLabelDefaults_.insert(selectionLabel, selectionLabel->text());
    itsLabelDefaults_.insert(averagingLabel, averagingLabel->text());
    
    // Connect widgets
    connect(itsFileWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsSelectionWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsAveragingWidget_, SIGNAL(changed()), SIGNAL(changed()));
    
    // Synchronize summary button.
    itsPlotter_->synchronizeAction(PlotMSAction::MS_SUMMARY, summaryButton);
}

PlotMSDataTab::~PlotMSDataTab() { }


// Public Methods //

void PlotMSDataTab::getValue(PlotMSPlotParameters& params) const {
    PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
    if(d == NULL) {
        params.setGroup<PMS_PP_MSData>();
        d = params.typedGroup<PMS_PP_MSData>();
    }
    
    d->setFilename(itsFileWidget_->getFile());
    d->setSelection(itsSelectionWidget_->getValue());
    d->setAveraging(itsAveragingWidget_->getValue());
}

void PlotMSDataTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
    if(d == NULL) return;
    
    itsFileWidget_->setFile(d->filename());
    itsSelectionWidget_->setValue(d->selection());
    itsAveragingWidget_->setValue(d->averaging());
}

void PlotMSDataTab::update(const PlotMSPlot& plot) {
    const PMS_PP_MSData* d = plot.parameters().typedGroup<PMS_PP_MSData>();
    if(d == NULL) return;
    
    changedText(locationLabel, itsFileWidget_->getFile() != d->filename());
    changedText(selectionLabel,
            itsSelectionWidget_->getValue() != d->selection());
    changedText(averagingLabel,
            itsAveragingWidget_->getValue() != d->averaging());
}

bool PlotMSDataTab::summaryVerbose() const {
    return MSDataTab::summaryVerbose->isChecked(); }
PMS::SummaryType PlotMSDataTab::summaryType() const {
    bool ok;
    PMS::SummaryType t = PMS::summaryType(
            MSDataTab::summaryType->currentText().toStdString(), &ok);
    if(!ok) t = PMS::S_ALL;
    return t;
}

}
