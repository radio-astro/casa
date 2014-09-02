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
    if(itsPlots_.size() > 0){
    	clearPlotsAndCanvases();
    }
    else {
    	itsPages_.clearPages();
    }
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
    return itsPlots_;
}

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

void PlotMSPlotManager::clearCanvas( int row, int col ){
	itsPages_.clearCanvas( row, col );
}

void PlotMSPlotManager::clearPlotsAndCanvases( bool clearCanvases ) {

	//Make sure all drawing is finished before we start clearing
	//the existing plots and canvases.
	waitForDrawing();

    //Remove the plots(data) from the canvases.
	unassignPlots();
    if ( clearCanvases ){
    	//Reset the page layout.
    	itsPages_.clearPages();
    }
    else {
    	//Remove axes and titles from the plots.
    	itsPages_.clearCanvases();

    	//Remove all trace of the plots on the pages.
    	int plotCount = itsPlots_.size();
    	for ( int i = 0; i < plotCount; i++ ){
    		itsPages_.disown( itsPlots_[i] );
    	}
    }

    //Delete the manager's copy of the plots.
    vector<PlotMSPlot*> plotsCopy = itsPlots_;
    itsPlots_.clear();
    itsPlotParameters_.clear();
    for(unsigned int i = 0; i < plotsCopy.size(); i++) {
        delete plotsCopy[i];
    }
    notifyWatchers();
}

bool PlotMSPlotManager::isOwner( int row, int col, PlotMSPlot* plot ){
	return itsPages_.canvasIsOwnedBy( row, col, plot );
}


QList<PlotMSPlot*> PlotMSPlotManager::getCanvasPlots(int row, int col) const {
	QList<PlotMSPlot*> canvasPlots;
	int plotCount = itsPlots_.size();
	for ( int i = 0; i < plotCount; i++ ){
		if ( itsPages_.canvasIsOwnedBy( row, col, itsPlots_[i] ) ){
			canvasPlots.append( itsPlots_[i]);
		}
	}
	return canvasPlots;
}

// Private Methods //
void PlotMSPlotManager::logMessage( const QString& msg ) const {
	if ( itsParent_ != NULL ){
		stringstream ss;
		ss << msg.toStdString().c_str();
		itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
					PMS::LOG_ORIGIN_PLOT,
					ss.str(),
					PMS::LOG_EVENT_PLOT);
	}
}

void PlotMSPlotManager::addPlot(PlotMSPlot* plot,
        const PlotMSPlotParameters* params) {
    if(plot == NULL) return;
    itsPlots_.push_back(plot);
    if(params != NULL){
    	plot->parameters() = *params;
    }

    itsPlotParameters_.push_back(&plot->parameters());
    if ( params == NULL ){
    	//We don't have a location preset for the plot so try to find an empty one.
    	Int availableRow = 1;
    	Int availableCol = 1;
    	bool emptySpot = findEmptySpot( availableRow, availableCol );
    	if ( emptySpot ){
    	   PlotMSPlotParameters& params = plot->parameters();
    	   PMS_PP_Iteration* iterParams = params.typedGroup<PMS_PP_Iteration>();
    	   if ( iterParams == NULL ){
    	       params.setGroup<PMS_PP_Iteration>();
    	       iterParams = params.typedGroup<PMS_PP_Iteration>();
    	   }
    	   iterParams->setGridRow( availableRow);
    	   iterParams->setGridCol( availableCol);
    	}
    }
    itsPages_.setupCurrentPage();

    bool locationFound = isPlottable( plot );
    if ( locationFound ){
    	plot->initializePlot(itsPages_);
    }

	notifyWatchers();
}

bool PlotMSPlotManager::isPlottable( PlotMSPlot* plot ) {
	bool locationFound = false;

	 //Before we set up any graphic elements we need to make sure the plot has a
	//valid location.  First check to see if its current one is valid.
	PlotMSPlotParameters& plotParams = plot->parameters();
	PMS_PP_Iteration* iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
	int desiredRow = 0;
	int desiredCol = 0;
	if ( iterParams != NULL ){
		desiredRow = iterParams->getGridRow();
		desiredCol = iterParams->getGridCol();
	}

	if ( desiredRow >= 0 && desiredCol >= 0 ){
		locationFound = itsPages_.isSpot( desiredRow, desiredCol, plot );

	}
   	return locationFound;
}

bool PlotMSPlotManager::findEmptySpot( Int& row, Int& col ){
	bool emptySpot = false;
	pair<int,int> spot = itsPages_.findEmptySpot();
	if ( spot.first >= 0 && spot.second >= 0 ){
		row = spot.first;
		col = spot.second;
		emptySpot = true;
	}
	return emptySpot;
}


void PlotMSPlotManager::removePlot( PlotMSPlot* plot ){
	std::vector<PlotMSPlot*>::iterator plotLoc = std::find( itsPlots_.begin(), itsPlots_.end(), plot );
	if ( plotLoc != itsPlots_.end() ){
		//Hold the drawing so we don't trigger a draw thread that causes
		//a segfault as we delete the plot out from under it.
		plot->waitForDrawing(true);
		plot->clearCanvases();
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

void PlotMSPlotManager::waitForDrawing(){
	int plotCount = itsPlots_.size();
	for ( int i = 0; i < plotCount; i++ ){

		itsPlots_[i]->waitForDrawing( true );

	}
}



bool PlotMSPlotManager::pageGridChanged( int rows, int cols, bool override ){
	//Detach the plots from the canvases
	bool resized = itsPages_.isGridChanged( rows, cols );
	if ( resized || override ){
		int plotCount = itsPlots_.size();
		waitForDrawing();
		for ( int i =0; i < plotCount; i++ ){

			//This is needed to avoid a segfault if the grid size is shrinking so
			//some of the existing plots may no longer be plotted.

			//Clears the canvas and sets the owner to NULL
			itsPages_.disown( itsPlots_[i]);
		}

		//We delete all the plots and canvases in scripting mode because
		//it is easy enough to script them back in.
		bool guiShown = itsParent_->guiShown();
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
				/*if ( !showPlot ){
				   	showPlot = assignEmptySpot( itsPlots_[i] );
				}*/
				if ( showPlot ){
					//Without this call the plot will appear as a separate window from
					//plotms.
					itsPlots_[i]->updateLocation();
				}
			}
		}
		//For GUI mode, the plots do not redraw themselves in their new grid when
		//the grid size is changed unless we tell them to redraw.
		if ( guiShown ){
			for ( int i = 0; i < plotCount; i++ ){
				if ( isPlottable( itsPlots_[i])){
					itsPlots_[i]->parametersHaveChanged( itsPlots_[i]->parameters(),
							PMS_PP::UPDATE_MSDATA | PMS_PP::UPDATE_REDRAW | PMS_PP::UPDATE_ITERATION | PMS_PP::UPDATE_CANVAS);
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
