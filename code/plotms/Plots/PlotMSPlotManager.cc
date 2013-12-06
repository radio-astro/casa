//# PlotMSPlotManager.cc: Manages PlotMSPlots for plotms.
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
#include <plotms/Plots/PlotMSPlotManager.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSOverPlot.h>
#include <QDebug>

namespace casa {

///////////////////////////////////
// PLOTMSPLOTMANAGER DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

PlotMSPlotManager::PlotMSPlotManager() : itsParent_(NULL), itsPages_(*this) { }

PlotMSPlotManager::~PlotMSPlotManager() {
    if(itsPlots_.size() > 0) clearPlotsAndCanvases();
    else                     itsPages_.clearPages();
}


// Public Methods //

PlotMSApp* PlotMSPlotManager::parent() { return itsParent_; }
void PlotMSPlotManager::setParent(PlotMSApp* parent) {
    itsParent_ = parent;
    itsPlotter_ = parent->getPlotter();
    itsFactory_ = parent->getPlotFactory();
    itsPlotter_->setCanvasLayout(PlotCanvasLayoutPtr());
}

PlotterPtr PlotMSPlotManager::plotter() { return itsPlotter_; }

void PlotMSPlotManager::addWatcher(PlotMSPlotManagerWatcher* watcher) {
    if(watcher == NULL) return;
    for(unsigned int i = 0; i < itsWatchers_.size(); i++)
        if(itsWatchers_[i] == watcher) return;
    itsWatchers_.push_back(watcher);
}

void PlotMSPlotManager::removeWatcher(PlotMSPlotManagerWatcher* watcher) {
    if(watcher == NULL) return;
    for(unsigned int i = 0; i < itsWatchers_.size(); i++) {
        if(itsWatchers_[i] == watcher) {
            itsWatchers_.erase(itsWatchers_.begin() + i);
            break;
        }
    }
}

unsigned int PlotMSPlotManager::numPlots() const { return itsPlots_.size(); }

const vector<PlotMSPlot*>& PlotMSPlotManager::plots() const {
    return itsPlots_; }

PlotMSPlot* PlotMSPlotManager::plot(unsigned int index) {
    if(index >= itsPlots_.size()) return NULL;
    else return itsPlots_[index];
}



const PlotMSPlot* PlotMSPlotManager::plot(unsigned int index) const {
    if(index >= itsPlots_.size()) return NULL;
    else return itsPlots_[index];
}

const vector<PlotMSPlotParameters*>& PlotMSPlotManager::plotParameters() const{
    return itsPlotParameters_; }

PlotMSPlotParameters* PlotMSPlotManager::plotParameters(unsigned int index) {
    if(index >= itsPlotParameters_.size()) return NULL;
    else return itsPlotParameters_[index];
}

PlotMSOverPlot* PlotMSPlotManager::addOverPlot(
    const PlotMSPlotParameters* params) {
    if(itsParent_ == NULL) return NULL;
    PlotMSOverPlot *plot = new PlotMSOverPlot(itsParent_);
    addPlot(plot, params);
    return plot;
}

void PlotMSPlotManager::clearPlotsAndCanvases() {
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        itsPlots_[i]->detachFromCanvases();
    
    vector<PlotMSPlot*> plotsCopy = itsPlots_;
    itsPlots_.clear();
    
    itsPlotParameters_.clear();
    itsPages_.clearPages();
    for(unsigned int i = 0; i < plotsCopy.size(); i++) {
        plotsCopy[i]->detachFromCanvases();
        delete plotsCopy[i];
    }
    
    notifyWatchers();
}


// Private Methods //

void PlotMSPlotManager::addPlot(PlotMSPlot* plot,
        const PlotMSPlotParameters* params) {
    if(plot == NULL) return;
    
    if(params != NULL) plot->parameters() = *params;
    
    itsPlots_.push_back(plot);
    itsPlotParameters_.push_back(&plot->parameters());
    
    itsPages_.setupCurrentPage();
    
    plot->initializePlot(itsPages_);
    
    notifyWatchers();
}

void PlotMSPlotManager::notifyWatchers() const {
    for(unsigned int i = 0; i < itsWatchers_.size(); i++)
        itsWatchers_[i]->plotsChanged(*this);
}


}
