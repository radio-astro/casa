//# PlotMSPlotTab.cc: Subclass of PlotMSTab for controlling plot parameters.
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
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/GuiTabs/PlotMSAxesTab.qo.h>
#include <plotms/GuiTabs/PlotMSCacheTab.qo.h>
#include <plotms/GuiTabs/PlotMSCanvasTab.qo.h>
#include <plotms/GuiTabs/PlotMSDataTab.qo.h>
#include <plotms/GuiTabs/PlotMSDisplayTab.qo.h>
#include <plotms/GuiTabs/PlotMSExportTab.qo.h>
#include <plotms/GuiTabs/PlotMSMultiAxesTab.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSMultiPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Plots/PlotMSSinglePlot.h>

namespace casa {

//////////////////////////////////
// PLOTMSPLOTSUBTAB DEFINITIONS //
//////////////////////////////////

PlotMSPlotSubtab::PlotMSPlotSubtab(PlotMSPlotTab* plotTab,
        PlotMSPlotter* parent) : PlotMSTab(parent), itsPlotTab_(plotTab) { }

PlotMSPlotSubtab::~PlotMSPlotSubtab() { }

PlotMSPlotParameters PlotMSPlotSubtab::currentlySetParameters() const {
    return itsPlotTab_->currentlySetParameters(); }


///////////////////////////////
// PLOTMSPLOTTAB DEFINITIONS //
///////////////////////////////

PlotMSPlotTab::PlotMSPlotTab(PlotMSPlotter* parent) :  PlotMSTab(parent),
        itsPlotManager_(parent->getParent()->getPlotManager()),
        itsCurrentPlot_(NULL), itsCurrentParameters_(NULL),
        itsUpdateFlag_(true) {
    setupUi(this);
    
    // Add as watcher.
    itsPlotManager_.addWatcher(this);    
    
    // Setup go.
    plotsChanged(itsPlotManager_);
    
    // Setup tab widget.
    tabWidget->removeTab(0);        
    
    // Initialize to no plot (empty).
    setupForPlot(NULL);    
    
    // Connect widgets.
    connect(goChooser, SIGNAL(currentIndexChanged(int)), SLOT(goChanged(int)));
    connect(goButton, SIGNAL(clicked()), SLOT(goClicked()));
    
    // Synchronize plot button.
    itsPlotter_->synchronizeAction(PlotMSAction::PLOT, plotButton);
}

PlotMSPlotTab::~PlotMSPlotTab() { }

QList<QToolButton*> PlotMSPlotTab::toolButtons() const {
    QList<QToolButton*> list;
    foreach(PlotMSPlotSubtab* tab, itsSubtabs_) list << tab->toolButtons();
    return list;
}

void PlotMSPlotTab::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag) {
    if(&p == itsCurrentParameters_ && itsCurrentPlot_ != NULL)
        setupForPlot(itsCurrentPlot_);
}

void PlotMSPlotTab::plotsChanged(const PlotMSPlotManager& manager) {
    goChooser->clear();
    
    // Add plot names to go chooser.
    const PlotMSPlot* plot;
    int setIndex = -1;
    for(unsigned int i = 0; i < manager.numPlots(); i++) {
        plot = manager.plot(i);
        goChooser->addItem(plot->name().c_str());
        
        // Keep the chooser on the same plot
        if(itsCurrentPlot_ != NULL && itsCurrentPlot_ == plot)
            setIndex = (int)i;
    }
    if(manager.numPlots() == 0) {
        itsCurrentPlot_ = NULL;
        itsCurrentParameters_ = NULL;
    }
    
    // Add "new" action(s) to go chooser.
    goChooser->addItem("New Single Plot");
    goChooser->addItem("New Multi Plot");
    int plotTypes = 2;
    
    // Add "clear" action to go chooser.
    goChooser->addItem("Clear Plotter");
    
    // If not showing a current plot, pick the latest plot if it exists.
    if(itsCurrentPlot_ == NULL && goChooser->count() > plotTypes + 1)
        setIndex = goChooser->count() - (plotTypes + 2);
    
    // Set to current plot, or latest plot if no current plot, and set tab.
    if(setIndex >= 0 && setIndex < goChooser->count() - (plotTypes + 1)) {
        goChooser->setCurrentIndex(setIndex);
        goClicked();
    }
}

PlotMSPlot* PlotMSPlotTab::currentPlot() const { return itsCurrentPlot_; }

PlotMSPlotParameters PlotMSPlotTab::currentlySetParameters() const {
    PlotMSPlotParameters params(itsPlotter_->getFactory());
    if(itsCurrentParameters_ != NULL) params = *itsCurrentParameters_;
    
    foreach(PlotMSPlotSubtab* tab, itsSubtabs_) tab->getValue(params);
    
    return params;
}

PlotExportFormat PlotMSPlotTab::currentlySetExportFormat() const {
    const PlotMSExportTab* tab;
    foreach(const PlotMSPlotSubtab* t, itsSubtabs_) {
        if((tab = dynamic_cast<const PlotMSExportTab*>(t)) != NULL)
            return tab->currentlySetExportFormat();
    }
    return PlotExportFormat(PlotExportFormat::PNG, "");
}

bool PlotMSPlotTab::msSummaryVerbose() const {
    const PlotMSDataTab* tab;
    foreach(const PlotMSPlotSubtab* t, itsSubtabs_) {
        if((tab = dynamic_cast<const PlotMSDataTab*>(t)) != NULL)
            return tab->summaryVerbose();
    }
    return false;
}

PMS::SummaryType PlotMSPlotTab::msSummaryType() const {
    const PlotMSDataTab* tab;
    foreach(const PlotMSPlotSubtab* t, itsSubtabs_) {
        if((tab = dynamic_cast<const PlotMSDataTab*>(t)) != NULL)
            return tab->summaryType();
    }
    return PMS::S_ALL;
}


// Public Slots //

void PlotMSPlotTab::plot() {
    if(itsCurrentParameters_ != NULL) {
        PlotMSPlotParameters params = currentlySetParameters();
        PMS_PP_MSData* d = params.typedGroup<PMS_PP_MSData>(),
                     *cd = itsCurrentParameters_->typedGroup<PMS_PP_MSData>();
        PMS_PP_Cache* c = params.typedGroup<PMS_PP_Cache>(),
                    *cc = itsCurrentParameters_->typedGroup<PMS_PP_Cache>();
        
        // Plot if 1) parameters have changed, 2) cache loading was canceled
        // previously.
        bool pchanged = params != *itsCurrentParameters_,
             cload = !itsCurrentPlot_->data().cacheReady();
        if(pchanged || cload) {
            if(pchanged) {
                if(itsParent_->getParameters().clearSelectionsOnAxesChange() &&
                   ((c != NULL && cc != NULL && (c->xAxis() != cc->xAxis() ||
                     c->yAxis() != cc->yAxis())) || (d != NULL && cd != NULL &&
                     d->filename() != cd->filename()))) {
                    vector<PlotCanvasPtr> canv = itsCurrentPlot_->canvases();
                    for(unsigned int i = 0; i < canv.size(); i++)
                        canv[i]->standardMouseTools()->selectTool()
                               ->clearSelectedRects();
                    itsPlotter_->getAnnotator().clearAll();
                }
                
                itsCurrentParameters_->holdNotification(this);
                *itsCurrentParameters_ = params;
                itsCurrentParameters_->releaseNotification();
            } else if(cload) {
                // Tell the plot to redraw itself because of the cache.
                itsCurrentPlot_->parametersHaveChanged(*itsCurrentParameters_,
                        PMS_PP::UPDATE_REDRAW & PMS_PP::UPDATE_CACHE);
            }
            plotsChanged(itsPlotManager_);
        }
    }
}


// Protected //

void PlotMSPlotTab::clearSubtabs() {
    itsSubtabs_.clear();
    tabWidget->clear();
}

void PlotMSPlotTab::clearSubtabsAfter(int index) {
    while(index < itsSubtabs_.size()) {
        itsSubtabs_.removeAt(itsSubtabs_.size() - 1);
        tabWidget->removeTab(tabWidget->count() - 1);
    }
}

void PlotMSPlotTab::addSubtab(PlotMSPlotSubtab* tab) {
    insertSubtab(itsSubtabs_.size(), tab); }

void PlotMSPlotTab::insertSubtab(int index, PlotMSPlotSubtab* tab) {
    if(tab == NULL) return;
    
    if(itsSubtabs_.contains(tab)) {
        if(index == itsSubtabs_.indexOf(tab)) return;
        itsSubtabs_.removeAll(tab);
        tabWidget->removeTab(tabWidget->indexOf(tab));
    } else {
        connect(tab, SIGNAL(changed()), SLOT(tabChanged()));
    }
    
    itsSubtabs_.insert(index, tab);
    tabWidget->insertTab(index, tab, tab->tabName());
}

// Helper macro for adding/insert known subtab types.
#define PT_ST(TYPE)                                                           \
PlotMS##TYPE##Tab* PlotMSPlotTab::add##TYPE##Subtab() {                       \
    return insert##TYPE##Subtab(itsSubtabs_.size()); }                        \
PlotMS##TYPE##Tab* PlotMSPlotTab::insert##TYPE##Subtab(int index) {           \
    PlotMS##TYPE##Tab* tab = NULL;                                            \
    foreach(PlotMSPlotSubtab* t, itsSubtabs_) {                               \
        tab = dynamic_cast<PlotMS##TYPE##Tab*>(t);                            \
        if(tab != NULL) break;                                                \
    }                                                                         \
    if(tab == NULL) tab = new PlotMS##TYPE##Tab(this, itsPlotter_);           \
    insertSubtab(index, tab);                                                 \
    return tab;                                                               \
}

PT_ST(Axes)
PT_ST(Cache)
PT_ST(Canvas)
PT_ST(Data)
PT_ST(Display)
PT_ST(Export)
PT_ST(MultiAxes)


// Private //

void PlotMSPlotTab::setupForPlot(PlotMSPlot* plot) {
    itsCurrentPlot_ = plot;
    tabWidget->setEnabled(plot != NULL);
    
    if(itsCurrentParameters_ != NULL)
        itsCurrentParameters_->removeWatcher(this);
    itsCurrentParameters_ = NULL;
    
    if(plot == NULL) return;
    
    bool oldupdate = itsUpdateFlag_;
    itsUpdateFlag_ = false;
    
    PlotMSPlotParameters& params = plot->parameters();
    params.addWatcher(this);
    itsCurrentParameters_ = &params;
    
    plot->setupPlotSubtabs(*this);
    // TODO update tool buttons
    
    foreach(PlotMSPlotSubtab* tab, itsSubtabs_) tab->setValue(params);
    
    itsUpdateFlag_ = oldupdate;
    
    tabChanged();
}

vector<PMS::Axis> PlotMSPlotTab::selectedLoadOrReleaseAxes(bool load) const {
    const PlotMSCacheTab* tab;
    foreach(const PlotMSPlotSubtab* t, itsSubtabs_) {
        if((tab = dynamic_cast<const PlotMSCacheTab*>(t)) != NULL)
            return tab->selectedLoadOrReleaseAxes(load);
    }
    return vector<PMS::Axis>();
}


// Private Slots //

void PlotMSPlotTab::goChanged(int index) {
    if(index < (int)itsPlotManager_.numPlots()) {
        // show "edit" button if not current plot
        goButton->setText("Edit");
        goButton->setVisible(itsPlotManager_.plot(index) != itsCurrentPlot_);
    } else {
        // show "go" button
        goButton->setText("Go");
        goButton->setVisible(true);
    }
}

void PlotMSPlotTab::goClicked() {
    int index = goChooser->currentIndex();
    
    if(index < (int)itsPlotManager_.numPlots()) {
        setupForPlot(itsPlotManager_.plot(index));
        goChanged(index);
        
    } else {
        int newSinglePlot = goChooser->count() - 3,
            newMultiPlot  = goChooser->count() - 2,
            clearPlotter  = goChooser->count() - 1;
        
        if(index == newSinglePlot || index == newMultiPlot) {
            // this will update the go chooser as necessary
            PlotMSPlot* plot = index == newSinglePlot ?
                               (PlotMSPlot*)itsPlotManager_.addSinglePlot() :
                               (PlotMSPlot*)itsPlotManager_.addMultiPlot();
            
            // switch to new plot if needed
            if(itsCurrentPlot_ != NULL) {
                for(unsigned int i = 0; i < itsPlotManager_.numPlots(); i++) {
                    if(itsPlotManager_.plot(i) == plot) {
                        goChooser->setCurrentIndex(i);
                        goClicked();
                        break;
                    }
                }
            }
            
        } else if(index == clearPlotter) {
            // this will update the go chooser as necessary
            itsPlotter_->plotActionMap().value(
                    PlotMSAction::CLEAR_PLOTTER)->trigger();
            setupForPlot(NULL);
        }
    }
}

void PlotMSPlotTab::tabChanged() {
    if(itsUpdateFlag_ && itsCurrentPlot_ != NULL) {
        itsUpdateFlag_ = false;
        
        foreach(PlotMSPlotSubtab* tab, itsSubtabs_)
            tab->update(*itsCurrentPlot_);
        
        itsCurrentPlot_->plotTabHasChanged(*this);
        
        itsUpdateFlag_ = true;
    }
}

}
