//# PlotMSAxesTab.cc: Plot tab for axes parameters.
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
#include <plotms/GuiTabs/PlotMSAxesTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSAxisWidget.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

///////////////////////////////
// PLOTMSAXESTAB DEFINITIONS //
///////////////////////////////

PlotMSAxesTab::PlotMSAxesTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent) :
        PlotMSPlotSubtab(plotTab, parent) {
    setupUi(this);
    
    // Setup axes widgets.
    itsXWidget_ = new PlotMSAxisWidget(PMS::DEFAULT_XAXIS, X_BOTTOM | X_TOP);
    itsYWidget_ = new PlotMSAxisWidget(PMS::DEFAULT_YAXIS, Y_LEFT | Y_RIGHT);
    QtUtilities::putInFrame(xFrame, itsXWidget_);
    QtUtilities::putInFrame(yFrame, itsYWidget_);
    
    // Insert label defaults.
    itsLabelDefaults_.insert(xLabel, xLabel->text());
    itsLabelDefaults_.insert(itsXWidget_->dataLabel(),
                             itsXWidget_->dataLabel()->text());
    itsLabelDefaults_.insert(itsXWidget_->attachLabel(),
                             itsXWidget_->attachLabel()->text());
    itsLabelDefaults_.insert(itsXWidget_->rangeLabel(),
                             itsXWidget_->rangeLabel()->text());
    itsLabelDefaults_.insert(yLabel, yLabel->text());
    itsLabelDefaults_.insert(itsYWidget_->dataLabel(),
                             itsYWidget_->dataLabel()->text());
    itsLabelDefaults_.insert(itsYWidget_->attachLabel(),
                             itsYWidget_->attachLabel()->text());
    itsLabelDefaults_.insert(itsYWidget_->rangeLabel(),
                             itsYWidget_->rangeLabel()->text());
    
    // Connect widgets.
    connect(itsXWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsYWidget_, SIGNAL(changed()), SIGNAL(changed()));
}

PlotMSAxesTab::~PlotMSAxesTab() { }


void PlotMSAxesTab::getValue(PlotMSPlotParameters& params) const {
    PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>();
    PMS_PP_Axes* a = params.typedGroup<PMS_PP_Axes>();
    if(c == NULL) {
        params.setGroup<PMS_PP_Cache>();
        c = params.typedGroup<PMS_PP_Cache>();
    }
    if(a == NULL) {
        params.setGroup<PMS_PP_Axes>();
        a = params.typedGroup<PMS_PP_Axes>();
    }
    
    c->setXAxis(itsXWidget_->axis(), itsXWidget_->data());
    a->setXAxis(itsXWidget_->attachAxis());
    a->setXRange(itsXWidget_->rangeCustom(), itsXWidget_->range());
    
    c->setYAxis(itsYWidget_->axis(), itsYWidget_->data());
    a->setYAxis(itsYWidget_->attachAxis());
    a->setYRange(itsYWidget_->rangeCustom(), itsYWidget_->range());
}

void PlotMSAxesTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>();
    const PMS_PP_Axes* a = params.typedGroup<PMS_PP_Axes>();
    if(c == NULL || a == NULL) return; // shouldn't happen
    
    itsXWidget_->setValue(c->xAxis(), c->xDataColumn(), a->xAxis(),
            a->xRangeSet(), a->xRange());
    itsYWidget_->setValue(c->yAxis(), c->yDataColumn(), a->yAxis(),
            a->yRangeSet(), a->yRange());
}

void PlotMSAxesTab::update(const PlotMSPlot& plot) {
    const PlotMSPlotParameters& params = plot.parameters();
    PlotMSPlotParameters newParams(params);
    getValue(newParams);
    
    const PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>();
    const PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>(),
                      *c2 = newParams.typedGroup<PMS_PP_Cache>();
    const PMS_PP_Axes* a = params.typedGroup<PMS_PP_Axes>(),
                     *a2 = newParams.typedGroup<PMS_PP_Axes>();
    
    // shouldn't happen
    if(d == NULL || c == NULL || c2 == NULL || a == NULL || a2 == NULL) return;
    
    // Update "in cache" for widgets.
    vector<pair<PMS::Axis,unsigned int> > laxes = plot.data().loadedAxes();
    bool found = false;
    for(unsigned int i = 0; !found && i < laxes.size(); i++)
        if(laxes[i].first == c2->xAxis()) found = true;
    itsXWidget_->setInCache(found);
    
    found = false;
    for(unsigned int i = 0; !found && i < laxes.size(); i++)
        if(laxes[i].first == c2->yAxis()) found = true;
    itsYWidget_->setInCache(found);
    
    // Update labels.
    highlightWidgetText(xLabel, d->isSet() && (c->xAxis() != c2->xAxis() ||
                (PMS::axisIsData(c->xAxis()) &&
                 c->xDataColumn() != c2->xDataColumn())));
    highlightWidgetText(itsXWidget_->dataLabel(), d->isSet() &&
                c->xDataColumn() != c2->xDataColumn());
    highlightWidgetText(itsXWidget_->attachLabel(), d->isSet() &&
                a->xAxis() != a2->xAxis());
    highlightWidgetText(itsXWidget_->rangeLabel(), d->isSet() &&
                (a->xRangeSet() != a2->xRangeSet() ||
                (a->xRangeSet() && a->xRange() != a2->xRange())));
    
    highlightWidgetText(yLabel, d->isSet() && (c->yAxis() != c2->yAxis() ||
                (PMS::axisIsData(c->yAxis()) &&
                 c->yDataColumn() != c2->yDataColumn())));
    highlightWidgetText(itsYWidget_->dataLabel(), d->isSet() &&
                c->yDataColumn() != c2->yDataColumn());
    highlightWidgetText(itsYWidget_->attachLabel(), d->isSet() &&
                a->yAxis() != a2->yAxis());
    highlightWidgetText(itsYWidget_->rangeLabel(), d->isSet() &&
                (a->yRangeSet() != a2->yRangeSet() ||
                (a->yRangeSet() && a->yRange() != a2->yRange())));
}

}
