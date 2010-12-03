//# PlotMS.cc: Main controller for plotms.
//# Copyright (C) 2008
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#include <plotms/PlotMS/PlotMS.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMSDBusApp.h>

namespace casa {

// TODO PlotMSAction: iteration, release cache.  action for new plots.  update
//      hold/release action/button text.
// TODO PlotMSCache: multi-region locate
// TODO PlotMSLogger: log source (std out, text widget, casapy logger), better
//      locate message, log message for parameters updated and action
//      execution, log event flag for flag/unflag
// TODO PlotMSParameters: canvas background, fonts, grids, spacing, cartesian
//      axes, limit zoom/pan cache size
// TODO PlotMSPlot: different colors within one plot, different types, shared
//      caches
// TODO PlotMSPlotter: range padding, customize toolbars/tabs
// TODO PlotMSThread: background, pause/resume
// TODO PlotMSWidgets: label creator
// TODO PlotTool: set tracker font

////////////////////////
// PLOTMS DEFINITIONS //
////////////////////////

// Constructors/Destructors //

PlotMS::PlotMS(bool connectToDBus) : itsDBus_(NULL) {
    initialize(connectToDBus); }

PlotMS::PlotMS(const PlotMSParameters& params, bool connectToDBus) :
        itsPlotter_(NULL), itsParameters_(params), itsDBus_(NULL) 
{
    initialize(connectToDBus); 
}

PlotMS::~PlotMS() {
    if(itsDBus_ != NULL) delete itsDBus_;
}


// Public Methods //

PlotMSPlotter* PlotMS::getPlotter() { return itsPlotter_; }
void PlotMS::showGUI(bool show) { itsPlotter_->showGUI(show); }
bool PlotMS::guiShown() const { return itsPlotter_->guiShown(); }
int PlotMS::execLoop() { return itsPlotter_->execLoop(); }
int PlotMS::showAndExec(bool show) { return itsPlotter_->showAndExec(show); }
void PlotMS::close() { itsPlotter_->close(); }

void PlotMS::showError(const String& message, const String& title,
        bool isWarning) { itsPlotter_->showError(message, title, isWarning); }
void PlotMS::showWarning(const String& message, const String& title) {
    itsPlotter_->showError(message, title, true); }
void PlotMS::showMessage(const String& message, const String& title) {
    itsPlotter_->showMessage(message, title); }

PlotMSParameters& PlotMS::getParameters() { return itsParameters_; }
void PlotMS::setParameters(const PlotMSParameters& params) {
    itsParameters_ = params; }

void PlotMS::parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag) {
	(void)updateFlag;
    // We only care about PlotMS's parameters.
    if(&params == &itsParameters_) {
        itsLogger_->setSinkLocation(itsParameters_.logFilename());
        itsLogger_->setFilterEventFlags(itsParameters_.logEvents());
        itsLogger_->setFilterMinPriority(itsParameters_.logPriority());
        
        pair<int, int> cis = itsParameters_.cachedImageSize();
        if(itsPlotter_ != NULL && !itsPlotter_->getPlotter().null() &&
           !itsPlotter_->getPlotter()->canvasLayout().null()) {
            vector<PlotCanvasPtr> canv = itsPlotter_->getPlotter()
                                     ->canvasLayout()->allCanvases();
            for(unsigned int i = 0; i < canv.size(); i++)
                if(!canv[i].null())
                    canv[i]->setCachedAxesStackImageSize(cis.first,cis.second);
        }
    }
}

PlotLoggerPtr PlotMS::getLogger() { return itsLogger_; }
PlotMSPlotManager& PlotMS::getPlotManager() { return itsPlotManager_; }

PlotMSSinglePlot* PlotMS::addSinglePlot(const PlotMSPlotParameters* p) {
    return itsPlotManager_.addSinglePlot(p); }

PlotMSMultiPlot* PlotMS::addMultiPlot(const PlotMSPlotParameters* p) {
    return itsPlotManager_.addMultiPlot(p); }


bool PlotMS::isDrawing() const {
	return itsPlotter_->isDrawing();
}

bool PlotMS::isClosed() const {
	return itsPlotter_ == NULL ||
               itsPlotter_->isClosed();
}

bool PlotMS::save(const PlotExportFormat& format, const bool interactive) {
	return itsPlotter_->exportPlot(format, interactive, false);
}

// Private Methods //

void PlotMS::initialize(bool connectToDBus) {

	its_want_avoid_popups=false;
	
    itsParameters_.addWatcher(this);
    
    itsPlotter_ = new PlotMSPlotter(this);
    itsPlotter_->showIterationButtons(false);
    itsLogger_ = itsPlotter_->getPlotter()->logger();
    
    itsPlotManager_.setParent(this);
    
    // Update internal state to reflect parameters.
    parametersHaveChanged(itsParameters_,
            PlotMSWatchedParameters::ALL_UPDATE_FLAGS());
    
    if(connectToDBus) {
        itsDBus_ = new PlotMSDBusApp(*this);
        itsDBus_->connectToDBus();
    }
}



}

