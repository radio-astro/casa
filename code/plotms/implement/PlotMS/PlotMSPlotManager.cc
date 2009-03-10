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
#include <plotms/PlotMS/PlotMSPlotManager.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

///////////////////////////////////
// PLOTMSPLOTMANAGER DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

PlotMSPlotManager::PlotMSPlotManager() : itsParent_(NULL) { }

PlotMSPlotManager::~PlotMSPlotManager() {
    if(itsPlots_.size() > 0) clearPlotsAndCanvases(); }


// Public Methods //

void PlotMSPlotManager::setParent(PlotMS* parent) {
    itsParent_ = parent;
    itsPlotter_ = parent->getPlotter()->getPlotter();
    itsFactory_ = parent->getPlotter()->getFactory();
    itsPlotter_->setCanvasLayout(PlotCanvasLayoutPtr());
}

void PlotMSPlotManager::addWatcher(PlotMSPlotManagerWatcher* watcher) {
    if(watcher == NULL) return;
    for(unsigned int i = 0; i < itsWatchers_.size(); i++)
        if(itsWatchers_[i] == watcher) return;
    itsWatchers_.push_back(watcher);
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

PlotMSSinglePlot* PlotMSPlotManager::addSinglePlot(PlotMS* parent,
        const PlotMSSinglePlotParameters* params) {
    PlotMSSinglePlot* plot = new PlotMSSinglePlot(parent);
    if(params != NULL) plot->singleParameters() = *params;
    itsPlots_.push_back(plot);
    itsPlotParameters_.push_back(&plot->parameters());
    addPlotToPlotter(plot);
    return plot;
}

void PlotMSPlotManager::clearPlotsAndCanvases() {
    for(unsigned int i = 0; i < itsPlots_.size(); i++) {
        itsPlots_[i]->detachFromCanvases();
        delete itsPlots_[i];
    }
    itsPlotter_->setCanvasLayout(PlotCanvasLayoutPtr());
    itsPlots_.clear();
    itsPlotParameters_.clear();
    notifyWatchers();
}


// Private Methods //

void PlotMSPlotManager::addPlotToPlotter(PlotMSPlot* plot) {
    if(plot == NULL || plot->layoutNumCanvases() == 0) return;
    
    // Find current grid dimensions.
    PlotCanvasLayoutPtr layout = itsPlotter_->canvasLayout();
    PlotLayoutGrid* grid = NULL;
    if(!layout.null()) grid = dynamic_cast<PlotLayoutGrid*>(&*layout);
    
    unsigned int rows = 0, cols = 0;
    if(grid != NULL) {
        rows = grid->rows(); cols = grid->cols();
    }
    
    // Find what dimensions the new plot requires.    
    unsigned int newRows = plot->layoutNumRows(),
                 newCols = plot->layoutNumCols();
    
    // See which is better: to the right or below.
    unsigned int rightRows = max(rows, newRows), rightCols = cols + newCols,
                 belowRows = rows + newRows, belowCols = max(cols, newCols);
    double rightRatio = max(rightRows, rightCols) / min(rightRows, rightCols),
           belowRatio = max(belowRows, belowCols) / min(belowRows, belowCols);
    bool useRight = rightRatio <= belowRatio;
    if(useRight) { newRows = rightRows; newCols = rightCols; }
    else         { newRows = belowRows; newCols = belowCols; }
    
    // Make new grid.
    PlotLayoutGrid* newGrid = new PlotLayoutGrid(newRows, newCols);
    PlotGridCoordinate coord(0, 0);
    
    // Copy over old canvases.
    for(unsigned int r = 0; r < rows; r++) {
        for(unsigned int c = 0; c < cols; c++) {
            coord.row = r; coord.col = c;
            newGrid->setCanvasAt(coord, grid->canvasAt(coord));
        }
    }
    
    // Make new canvases.
    PlotCanvasPtr canvas;
    vector<PlotCanvasPtr> canvases(plot->layoutNumCanvases());
    PlotStandardMouseToolGroupPtr tools;
    PlotMSToolsTab* toolsTab = itsParent_->getPlotter()->getToolsTab();
    unsigned int i = 0;
    for(unsigned int r = 0; r < newRows; r++) {
        for(unsigned int c = 0; c < newCols; c++) {
            coord.row = r; coord.col = c;
            
            if(!newGrid->canvasAt(coord).null()) continue;
            
            canvas = itsFactory_->canvas();
            newGrid->setCanvasAt(coord, canvas);
            
            if(i < canvases.size()) canvases[i++] = canvas;
            
            // connect new canvases' tracker to tool tab
            tools = canvas->standardMouseTools();
            tools->trackerTool()->addNotifier(toolsTab);
            tools->selectTool()->setDrawRects(true);
        }
    }
    
    // Set plotter with new layout.
    itsPlotter_->setCanvasLayout(newGrid);
    
    itsParent_->getPlotter()->holdDrawing();
    
    // Initialize plot.
    plot->initializePlot(canvases);
    
    itsParent_->getPlotter()->releaseDrawing();
    
    // Notify watchers.
    notifyWatchers();
}

void PlotMSPlotManager::notifyWatchers() const {
    for(unsigned int i = 0; i < itsWatchers_.size(); i++)
        itsWatchers_[i]->plotsChanged(*this);
}

}
