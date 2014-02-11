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
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <QDebug>

namespace casa {

///////////////////////////////////
// PLOTMSPLOTMANAGER DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

PlotMSPlotManager::PlotMSPlotManager() : itsParent_(NULL), itsPages_(*this) {

}

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

    itsPages_.setupCurrentPage();
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

unsigned int PlotMSPlotManager::numPlots() const {
	return itsPlots_.size();
}

const vector<PlotMSPlot*>& PlotMSPlotManager::plots() const {
    return itsPlots_; }

PlotMSPlot* PlotMSPlotManager::plot(unsigned int index) {
    if(index >= itsPlots_.size()){
    	return NULL;
    }
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

PlotMSOverPlot* PlotMSPlotManager::addOverPlot(const PlotMSPlotParameters* params) {
    if(itsParent_ == NULL) return NULL;
    PlotMSOverPlot *plot = new PlotMSOverPlot(itsParent_);
    addPlot(plot, params);
    return plot;
}

void PlotMSPlotManager::unassignPlots(){
	for(unsigned int i = 0; i < itsPlots_.size(); i++){
		itsPlots_[i]->detachFromCanvases();
	}
}

void PlotMSPlotManager::clearPlotsAndCanvases() {
    unassignPlots();
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
    itsPlots_.push_back(plot);
    if(params != NULL){
    	plot->parameters() = *params;
    }
    itsPlotParameters_.push_back(&plot->parameters());
    itsPages_.setupCurrentPage();


    bool locationFound = isPlottable( plot );
    if ( locationFound ){
    	plot->initializePlot(itsPages_);
    }
	notifyWatchers();
}

bool PlotMSPlotManager::isPlottable( PlotMSPlot* plot ) const {
	bool locationFound = false;
	 //Before we set up any graphic elements we need to make sure the plot has a
	//valid location.  First check to see if its current one is valid.
	PlotMSPlotParameters& plotParams = plot->parameters();
	PMS_PP_Iteration* iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
	if ( iterParams != NULL ){
		int desiredRow = iterParams->getGridRow();
		int desiredCol = iterParams->getGridCol();
		locationFound = itsPages_.isSpot( desiredRow, desiredCol, plot );
	}

   	if ( !locationFound ){
   		//Try to find an empty spot to put it.
   		pair<int,int> location = itsPages_.findEmptySpot();
   		if ( location.first != -1 && location.second != -1 ){
   			locationFound = true;
   			if ( iterParams == NULL ){
   				plotParams.setGroup<PMS_PP_Iteration>();
   			    iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
   			}
   			iterParams->setGridRow( location.first );
   			iterParams->setGridCol( location.second );
   		}
   	}
   	return locationFound;
}


void PlotMSPlotManager::removePlot( PlotMSPlot* plot ){
	std::vector<PlotMSPlot*>::iterator plotLoc = std::find( itsPlots_.begin(), itsPlots_.end(), plot );
	if ( plotLoc != itsPlots_.end() ){
		plot->detachFromCanvases();
		itsPages_.disown( plot );
		itsPlots_.erase(plotLoc);
		delete plot;
	}
	notifyWatchers();
}

void PlotMSPlotManager::getGridSize( Int& rows, Int& cols ){
	rows = 1;
	cols = 1;
	if ( itsParent_ != NULL ){
		PlotMSParameters params = itsParent_->getParameters();
		rows = params.getRowCount();
		cols = params.getColCount();
	}
}

bool PlotMSPlotManager::pageGridChanged( int rows, int cols, bool override ){
	//Detach the plots from the canvases
	bool resized = itsPages_.isGridChanged( rows, cols );
	if ( resized || override ){
		bool guiShown = itsParent_->guiShown();
		int plotCount = itsPlots_.size();
		for ( int i =0; i < plotCount; i++ ){

			//Wait for existing draw threads to finish before we proceed so
			//we don't get a seg fault from a draw thread hanging onto deleted
			//data.
			itsPlots_[i]->waitForDrawing( true );

			//This is needed to avoid a segfault if the grid size is shrinking so
			//some of the existing plots may no longer be plotted.

			//Clears the canvas and sets the owner to NULL
			itsPages_.disown( itsPlots_[i]);

		}
		//We delete all the plots and canvases in scripting mode because
		//it is easy enough to script them back in.
		if ( !guiShown ){
			clearPlotsAndCanvases();
			itsPlotter_->setCanvasLayout(PlotCanvasLayoutPtr());
		}
		//In GUI mode the user has the ability to delete individual plots
		//as needed and would probably be mad if all the plots disappeared
		//when the grid size changed.  Instead, we change the grid, and then
		//try to relocate the plots as best we can.
		else {
			itsPages_.gridChanged( rows, cols );
			itsPages_.setupCurrentPage();
			for ( int i = 0; i < plotCount; i++ ){
				bool showPlot = isPlottable( itsPlots_[i]);
				if ( showPlot ){
					itsPlots_[i]->updateLocation();
				}
			}
		}
	}
	return resized;
}

void PlotMSPlotManager::notifyWatchers() const {
    for(unsigned int i = 0; i < itsWatchers_.size(); i++){
        itsWatchers_[i]->plotsChanged(*this);
    }
}

vector<String> PlotMSPlotManager::getFiles() const {
	vector<String> fileNames;
	int plotCount = itsPlotParameters_.size();
	for ( int i = 0; i < plotCount; i++ ){
		PMS_PP_MSData* data = itsPlotParameters_[i]->typedGroup<PMS_PP_MSData>();
		if ( data != NULL ){
			String fileName = data->filename();
			if ( ! fileName.empty() ){
				vector<String>::iterator last = fileNames.end();
				vector<String>::iterator location = find( fileNames.begin(), last, fileName );
				if ( fileNames.size() == 0 || location != last){
					fileNames.push_back( fileName );
				}
			}
		}
	}
	return fileNames;
}

PlotMSParameters PlotMSPlotManager::getPageParameters(){
	return itsParent_->getParameters();
}

}
