//# PlotMSPlot.cc: High level plot concept across potentially multiple objects.
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
#include <plotms/Plots/PlotMSPlot.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

////////////////////////////
// PLOTMSPLOT DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

PlotMSPlot::PlotMSPlot(PlotMS* parent) : itsParent_(parent),
        itsFactory_(parent->getPlotter()->getFactory()), itsVisSet_(NULL) { }

PlotMSPlot::~PlotMSPlot() {
    // Clean up MS
    if(itsVisSet_ != NULL) delete itsVisSet_;
    
    // Clean up plots
    detachFromCanvases();
}


// Public Methods //

unsigned int PlotMSPlot::layoutNumCanvases() const {
    return layoutNumRows() * layoutNumCols(); }

vector<PlotCanvasPtr> PlotMSPlot::canvases() const { return itsCanvases_; }

bool PlotMSPlot::initializePlot(const vector<PlotCanvasPtr>& canvases) {
    if(canvases.size() != layoutNumCanvases()) return false;
    itsCanvases_ = canvases;
    
    bool hold = drawingHeld();
    if(!hold) holdDrawing();
    
    // Initialize plot objects and assign canvases.
    if(!initializePlot() || !assignCanvases()) {
        if(!hold) releaseDrawing();
        return false;
    }
    
    // Attach plot objects to their assigned canvases.
    attachToCanvases();
    
    // Update objects with set parameters.
    parameters().releaseNotification();
    parametersHaveChanged(parameters(), PlotMSWatchedParameters::ALL, false);
    
    // Draw if necessary.
    if(!hold) releaseDrawing();
    
    return true;
}

void PlotMSPlot::attachToCanvases() {
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        itsCanvasMap_[&*itsPlots_[i]]->plotItem(itsPlots_[i]);
}

void PlotMSPlot::detachFromCanvases() {
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        itsCanvasMap_[&*itsPlots_[i]]->removePlotItem(itsPlots_[i]);
}

PlotMSData& PlotMSPlot::data() { return itsData_; }
const PlotMSData& PlotMSPlot::data() const { return itsData_; }

VisSet* PlotMSPlot::visSet() { return itsVisSet_; }
const VisSet* PlotMSPlot::visSet() const { return itsVisSet_; }

PlotMS* PlotMSPlot::parent() { return itsParent_; }

void PlotMSPlot::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag, bool redrawRequired) {   
    // Make sure it's this plot's parameters.
    PlotMSPlotParameters& params = parameters();
    if(&p != &params) return;
    
    bool msUpdated = updateFlag & PlotMSWatchedParameters::MS,
         cacheUpdated = updateFlag & PlotMSWatchedParameters::CACHE,
         canvasUpdated = updateFlag & PlotMSWatchedParameters::CANVAS,
         plotUpdated = updateFlag & PlotMSWatchedParameters::PLOT;
    
    /*
    cout << "PlotMSPlot::parametersHaveChanged(" << updateFlag << ") "
         << msUpdated << ' ' << cacheUpdated << ' ' << canvasUpdated << ' '
         << plotUpdated << ' ' << redrawRequired << endl;
    */
    
    if(!msUpdated && !cacheUpdated && !canvasUpdated && !plotUpdated) return;
    
    bool hold = drawingHeld();
    if(!hold && redrawRequired) holdDrawing();
    
    itsTCLendLog_ = itsTCLlogNumPoints_ = false;
    itsTCLupdateCanvas_ = msUpdated || canvasUpdated || !params.isSet();
    itsTCLupdatePlot_ = plotUpdated;
    itsTCLrelease_ = !hold && redrawRequired;
    
    // Update MS/cache as needed.
    bool msSuccess = true, callCacheLoaded = true;
    if(params.isSet()) {
        if(msUpdated || cacheUpdated) {
            // Let the plot(s) know that the data will be updated.
            for(unsigned int i = 0; i < itsPlots_.size(); i++)
                itsPlots_[i]->dataChanged();
            
            itsTCLlogNumPoints_ = true;
            
            startLogCache();
            itsTCLendLog_ = true;
            
            if(msUpdated) msSuccess = updateMS();
            itsTCLupdateCanvas_ |= !msSuccess;
            
            // Only update cache if MS opening succeeded.
            if(msSuccess) {
                callCacheLoaded = !hasThreadedCaching();
                updateCache();
            } else itsData_.clearCache();
        }        
    } else itsData_.clearCache();
    
    // Do the rest immediately if 1) cache not updated, or 2) cache not
    // threaded.  Otherwise wait for the thread to call cacheLoaded_().
    if(callCacheLoaded) cacheLoaded_(false);
}

bool PlotMSPlot::exportToFormat(const PlotExportFormat& format) {
    if(itsCanvases_.size() == 0) return true;
    else if(itsCanvases_.size() == 1)
        return itsCanvases_[0]->exportToFile(format);
    else {
        bool success = true;
        
        PlotExportFormat form(format);
        for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
            form.location = String::toString(i) + "-" + format.location;
            if(!itsCanvases_[i]->exportToFile(form)) success = false;
        }
        
        return success;        
    }
}


// Protected Methods //

void PlotMSPlot::constructorSetup() {
    PlotMSPlotParameters& params = parameters();
    params.addWatcher(this);
    
    // hold notification until initializePlot is called
    params.holdNotification(this);
}

bool PlotMSPlot::updateMS() {
    if(itsVisSet_ != NULL) {
        delete itsVisSet_;
        itsVisSet_ = NULL;
    }
    
    try {
        // Clear cache.
        itsData_.clearCache();
        
        // Open MS and lock until we're done.
        itsMS_ = MeasurementSet(parameters().filename(),
                 TableLock(TableLock::AutoLocking), Table::Update);

        // Apply the MS selection.
        Matrix<Int> chansel;
        parameters().selection().apply(itsMS_, itsSelectedMS_, chansel);
        
        // Sort appropriately.
        double solint(DBL_MAX);
        double interval(max(solint, DBL_MIN));
        if(solint < 0) interval = 0;
      
        Block<Int> columns(5);
        columns[0]=MS::ARRAY_ID;
        columns[1]=MS::SCAN_NUMBER;  // force scan boundaries
        columns[2]=MS::FIELD_ID;      
        columns[3]=MS::DATA_DESC_ID;  // force 
        columns[4]=MS::TIME;
        
        // Open VisSet.
        itsVisSet_ = new VisSet(itsSelectedMS_, columns, Matrix<int>(),False,
                                interval);
        itsVisSet_->selectChannel(chansel);
        return true;
        
    } catch(AipsError& err) {
        itsParent_->showError("Could not open MS: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not open MS, for unknown reasons!");
        return false;
    }
}

bool PlotMSPlot::drawingHeld() {
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        if(itsCanvases_[i]->drawingIsHeld()) return true;
    return false;
}

void PlotMSPlot::holdDrawing() {
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        itsCanvases_[i]->holdDrawing();
}

void PlotMSPlot::releaseDrawing() {
    //itsParent_->getPlotter()->doThreadedRedraw(this);
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        itsCanvases_[i]->releaseDrawing();
}

void PlotMSPlot::startLogCache() {
    PlotMSLogger* log = itsParent_->loggerFor(PlotMSLogger::LOAD_CACHE);
    if(log != NULL)
        log->markMeasurement(PlotMS::CLASS_NAME, PlotMS::LOG_LOAD_CACHE);
}

void PlotMSPlot::endLogCache() {
    PlotMSLogger* log = itsParent_->loggerFor(PlotMSLogger::LOAD_CACHE);
    if(log != NULL) log->releaseMeasurement();
}

void PlotMSPlot::logNumPoints() {
    PlotMSLogger* log = itsParent_->loggerFor(PlotMSLogger::NUM_POINTS);
    if(log != NULL) {
        PlotMaskedPointDataPtr data;
        unsigned int total = 0, flagged = 0, unflagged = 0;
        for(unsigned int i = 0; i < itsPlots_.size(); i++) {
            data = itsPlots_[i]->maskedData();
            total += data->size();
            flagged += data->sizeMasked();
            unflagged += data->sizeUnmasked();
        }
        
        stringstream ss;
        ss << "Plotted " << total << " points (" << unflagged << " unflagged, "
           << flagged << " flagged).";
        log->postMessage(PlotMS::CLASS_NAME, "plot", ss.str());
    }
}

void PlotMSPlot::cacheLoaded_(bool wasCanceled) {
    // End log as needed.
    if(itsTCLendLog_) endLogCache();
    
    // Update canvas as needed.
    if(itsTCLupdateCanvas_) updateCanvas();
    
    // Update plot as needed.
    if(itsTCLupdatePlot_) updatePlot();
    
    // Release drawing if needed.
    if(itsTCLrelease_) releaseDrawing();
    
    // Log number of points plotted, if needed.
    if(itsTCLlogNumPoints_) logNumPoints();
}

}
