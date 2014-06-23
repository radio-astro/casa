//# PlotMSTransformationsTab.cc: Plot tab for transformations parameters.
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
#include <plotms/GuiTabs/PlotMSTransformationsTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Gui/PlotMSTransformationsWidget.qo.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

//////////////////////////////////////////
// PLOTMSTRANSFORMATIONSTAB DEFINITIONS //
//////////////////////////////////////////

// Constructors/Destructors//

PlotMSTransformationsTab::PlotMSTransformationsTab(PlotMSPlotTab* plotTab, 
						   PlotMSPlotter* parent) :
        PlotMSPlotSubtab(plotTab, parent) {
    setupUi(this);
    
    // Setup widgets
    itsTransformationsWidget_ = new PlotMSTransformationsWidget();
    QtUtilities::putInFrame(transFrame, itsTransformationsWidget_);
    
    itsLabelDefaults_.insert(transLabel, transLabel->text());
    
    // Connect widgets
    connect(itsTransformationsWidget_, SIGNAL(changed()), SIGNAL(changed()));
    
}

PlotMSTransformationsTab::~PlotMSTransformationsTab() { }


// Public Methods //

void PlotMSTransformationsTab::getValue(PlotMSPlotParameters& params) const {
    PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
    if(d == NULL) {
        params.setGroup<PMS_PP_MSData>();
        d = params.typedGroup<PMS_PP_MSData>();
    }
    
    d->setTransformations(itsTransformationsWidget_->getValue());
}

void PlotMSTransformationsTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
    if(d == NULL) return;
    
    itsTransformationsWidget_->setValue(d->transformations());
}

void PlotMSTransformationsTab::update(const PlotMSPlot& plot) {
    const PMS_PP_MSData* d = plot.parameters().typedGroup<PMS_PP_MSData>();
    if(d == NULL) return;
    
    highlightWidgetText(transLabel,
		itsTransformationsWidget_->getValue() != d->transformations());
}

}
