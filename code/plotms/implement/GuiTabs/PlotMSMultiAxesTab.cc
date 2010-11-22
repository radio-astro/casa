//# PlotMSMultiAxesTab.cc: Plot tab for multi axes parameters.
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
#include <plotms/GuiTabs/PlotMSMultiAxesTab.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSAxisWidget.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

////////////////////////////////////
// PLOTMSMULTIAXESTAB DEFINITIONS //
////////////////////////////////////

// Constructors/Destructors //

PlotMSMultiAxesTab::PlotMSMultiAxesTab(PlotMSPlotTab* t, PlotMSPlotter* p) :
        PlotMSPlotSubtab(t, p), itsXAxes_(this), itsYAxes_(this) {
    // Setup widgets.
    setupUi(this);
    itsXAxes_.setupUi(xFrame);
    itsYAxes_.setupUi(yFrame);
    QtUtilities::putInScrollArea(xFrame);
    QtUtilities::putInScrollArea(yFrame);
    
    // Insert label defaults.
    itsLabelDefaults_.insert(xLabel, xLabel->text());
    itsLabelDefaults_.insert(yLabel, yLabel->text());
    
    // Connect widgets.
    connect(xNewButton, SIGNAL(clicked()), SLOT(addX()));
    connect(yNewButton, SIGNAL(clicked()), SLOT(addY()));
}

PlotMSMultiAxesTab::~PlotMSMultiAxesTab() { }


// Public Methods //

void PlotMSMultiAxesTab::getValue(PlotMSPlotParameters& params) const {
    PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>();
    PMS_PP_Axes* a = params.typedGroup<PMS_PP_Axes>();
    
    if(c == NULL || a == NULL) return;
    
    vector<PMS::Axis> x(itsXAxes_.size(), PMS::DEFAULT_XAXIS),
                      y(itsYAxes_.size(), PMS::DEFAULT_YAXIS);
    vector<PMS::DataColumn> xd(x.size(), PMS::DEFAULT_DATACOLUMN),
                            yd(y.size(), PMS::DEFAULT_DATACOLUMN);
    vector<PlotAxis> xa(itsXAxes_.size(), PMS::DEFAULT_CANVAS_XAXIS),
                     ya(itsYAxes_.size(), PMS::DEFAULT_CANVAS_YAXIS);
    vector<bool> xr(itsXAxes_.size(), false), yr(itsYAxes_.size(), false);
    vector<prange_t> xr2(itsXAxes_.size(), prange_t(0.0, 0.0)),
                     yr2(itsYAxes_.size(), prange_t(0.0, 0.0));
    
    for(unsigned int i = 0; i < x.size(); i++) {
        x[i] = itsXAxes_.axis(i)->axis();
        xd[i] = itsXAxes_.axis(i)->data();
        xa[i] = itsXAxes_.axis(i)->attachAxis();
        xr[i] = itsXAxes_.axis(i)->rangeCustom();
        xr2[i] = itsXAxes_.axis(i)->range();
    }
    c->setXAxes(x);
    c->setXDataColumns(xd);
    a->setXAxes(xa);
    a->setXRanges(xr);
    a->setXRanges(xr2);
    
    for(unsigned int i = 0; i < y.size(); i++) {
        y[i] = itsYAxes_.axis(i)->axis();
        yd[i] = itsYAxes_.axis(i)->data();
        ya[i] = itsYAxes_.axis(i)->attachAxis();
        yr[i] = itsYAxes_.axis(i)->rangeCustom();
        yr2[i] = itsYAxes_.axis(i)->range();
    }
    c->setYAxes(y);
    c->setYDataColumns(yd);
    a->setYAxes(ya);
    a->setYRanges(yr);
    a->setYRanges(yr2);
}

void PlotMSMultiAxesTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>();
    const PMS_PP_Axes* a = params.typedGroup<PMS_PP_Axes>();
    if(c == NULL || a == NULL) return; // shouldn't happen
    
    // Resize axes.
    int x = c->numXAxes(), y = c->numYAxes();
    while(itsXAxes_.size() > x) itsXAxes_.removeAxis(itsXAxes_.size() - 1);
    while(itsXAxes_.size() < x) addX();
    while(itsYAxes_.size() > y) itsYAxes_.removeAxis(itsYAxes_.size() - 1);
    while(itsYAxes_.size() < y) addY();
    
    // Set values.
    for(int i = 0; i < x; i++)
        itsXAxes_.axis(i)->setValue(c->xAxis(i), c->xDataColumn(i),
                a->xAxis(i), a->xRangeSet(i), a->xRange(i));
    
    for(int i = 0; i < y; i++)
        itsYAxes_.axis(i)->setValue(c->yAxis(i), c->yDataColumn(i),
                a->yAxis(i), a->yRangeSet(i), a->yRange(i));
}

void PlotMSMultiAxesTab::update(const PlotMSPlot& plot) {
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
    
    // Update widgets and labels.
    vector<pair<PMS::Axis,unsigned int> > laxes = plot.data().loadedAxes();
    bool found = false;
    const vector<PMS::Axis>& x = c2->xAxes(), &y = c2->yAxes();
    bool changed = d->isSet() && c->numXAxes() != c2->numXAxes();
    for(unsigned int i = 0; i < x.size(); i++) {
        // Update "in cache" for widgets.
        found = false;
        for(unsigned int j = 0; !found && j < laxes.size(); j++)
            if(laxes[i].first == x[i]) found = true;
        itsXAxes_.axis(i)->setInCache(found);
        
        if(i < c->numXAxes())
            changed |= d->isSet() && (x[i] != c->xAxes()[i] ||
                       (PMS::axisIsData(x[i]) &&
                        c->xDataColumns()[i] != c2->xDataColumns()[i]));
        
        highlightWidgetText(itsXAxes_.axis(i)->dataLabel(), d->isSet() &&
                    c->xDataColumns()[i] != c2->xDataColumns()[i]);
        highlightWidgetText(itsXAxes_.axis(i)->attachLabel(), d->isSet() &&
                    a->xAxis(i) != a2->xAxis(i));
        highlightWidgetText(itsXAxes_.axis(i)->rangeLabel(), d->isSet() &&
                    (a->xRangeSet(i) != a2->xRangeSet(i) || (a->xRangeSet(i) &&
                    a->xRange(i) != a2->xRange(i))));
    }
    highlightWidgetText(xLabel, changed);
    
    changed = d->isSet() && c->numYAxes() != c2->numYAxes();
    for(unsigned int i = 0; i < y.size(); i++) {
        // Update "in cache" for widgets.
        found = false;
        for(unsigned int j = 0; !found && j < laxes.size(); j++)
            if(laxes[i].first == y[i]) found = true;
        itsYAxes_.axis(i)->setInCache(found);
        
        if(i < c->numYAxes())
            changed |= d->isSet() && (y[i] != c->yAxes()[i] ||
                       (PMS::axisIsData(y[i]) &&
                        c->yDataColumns()[i] != c2->yDataColumns()[i]));
        
        highlightWidgetText(itsYAxes_.axis(i)->dataLabel(), d->isSet() &&
                    c->yDataColumns()[i] != c2->yDataColumns()[i]);
        highlightWidgetText(itsYAxes_.axis(i)->attachLabel(), d->isSet() &&
                    a->yAxis(i) != a2->yAxis(i));
        highlightWidgetText(itsYAxes_.axis(i)->rangeLabel(), d->isSet() &&
                    (a->yRangeSet(i) != a2->yRangeSet(i) || (a->yRangeSet(i) &&
                    a->yRange(i) != a2->yRange(i))));
    }
    highlightWidgetText(yLabel, changed);
}


// Private Slots //

void PlotMSMultiAxesTab::addX() {
    itsXAxes_.addAxis(PMS::DEFAULT_XAXIS, X_BOTTOM | X_TOP, SLOT(removeX())); }
void PlotMSMultiAxesTab::addY() {
    itsYAxes_.addAxis(PMS::DEFAULT_YAXIS, Y_LEFT | Y_RIGHT, SLOT(removeY())); }

void PlotMSMultiAxesTab::removeX() {
    if(itsXAxes_.size() <= 1) return;
    itsXAxes_.removeAxis(itsXAxes_.indexOfButton(
            dynamic_cast<QPushButton*>(sender())));
}
void PlotMSMultiAxesTab::removeY() {
    if(itsYAxes_.size() <= 1) return;
    itsYAxes_.removeAxis(itsYAxes_.indexOfButton(
            dynamic_cast<QPushButton*>(sender())));
}


///////////////////////////////////////////////
// PLOTMSMULTIAXESTAB::MULTIAXES DEFINITIONS //
///////////////////////////////////////////////

// Constructors/Destructors //

PlotMSMultiAxesTab::MultiAxes::MultiAxes(PlotMSMultiAxesTab* parent) :
        itsParent_(parent), itsLayout_(NULL) { }
    
PlotMSMultiAxesTab::MultiAxes::~MultiAxes() { }


// Public Methods //

void PlotMSMultiAxesTab::MultiAxes::setupUi(QWidget* widget) {
    if(itsLayout_ != NULL) return;
    
    itsLayout_ = new QVBoxLayout(widget);
    itsLayout_->setContentsMargins(3, 3, 3, 3);
    itsLayout_->setSpacing(3);
}

int PlotMSMultiAxesTab::MultiAxes::size() const { return itsLayouts_.size(); }

PlotMSAxisWidget* PlotMSMultiAxesTab::MultiAxes::axis(int index) {
    return itsAxisWidgets_[index]; }
const PlotMSAxisWidget* PlotMSMultiAxesTab::MultiAxes::axis(int index) const {
    return itsAxisWidgets_[index]; }

void PlotMSMultiAxesTab::MultiAxes::addAxis(PMS::Axis value, int attachAxes,
        const char* removeSlot) {
    if(itsLayout_ == NULL) return;
    
    // Generate widgets.
    QHBoxLayout* l = new QHBoxLayout();
    l->setSpacing(3);
    PlotMSAxisWidget* w = new PlotMSAxisWidget(value, attachAxes);
    QPushButton* b = new QPushButton("Remove");
    l->addWidget(w);
    l->addWidget(b);
    
    // Insert label defaults.
    QMap<QLabel*, QString>& lb = itsParent_->itsLabelDefaults_;
    lb.insert(w->dataLabel(), w->dataLabel()->text());
    lb.insert(w->attachLabel(), w->attachLabel()->text());
    lb.insert(w->rangeLabel(), w->rangeLabel()->text());
    
    // Add to lists and layout.
    itsLayouts_.append(l);
    itsAxisWidgets_.append(w);
    itsButtons_.append(b);
    itsLayout_->addLayout(l);
    
    // Connect widgets.
    connect(b, SIGNAL(clicked()), itsParent_, removeSlot);
    
    // Emit changed signal.
    itsParent_->changed();
}

void PlotMSMultiAxesTab::MultiAxes::removeAxis(int index) {
    if(itsLayout_ == NULL || index < 0 || index > itsLayouts_.size()) return;
    
    // Remove from lists.
    QHBoxLayout* l = itsLayouts_.takeAt(index);
    PlotMSAxisWidget* w = itsAxisWidgets_.takeAt(index);
    QPushButton* b = itsButtons_.takeAt(index);
    
    // Remove label defaults.
    QMap<QLabel*, QString>& lb = itsParent_->itsLabelDefaults_;
    lb.remove(w->dataLabel());
    lb.remove(w->attachLabel());
    lb.remove(w->rangeLabel());
    
    // Remove from main layout and delete.
    b->disconnect();
    itsLayout_->removeItem(l);
    delete l;
    
    // Emit changed signal.
    itsParent_->changed();
}

int PlotMSMultiAxesTab::MultiAxes::indexOfButton(QPushButton* button) {
    return itsButtons_.indexOf(button); }

}
