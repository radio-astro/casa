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
#include <plotms/PlotMS/PlotMSPlot.h>

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
    
    itsTCLendLog_ = false;
    itsTCLupdateCanvas_ = msUpdated || canvasUpdated || !params.isSet();
    itsTCLupdatePlot_ = plotUpdated;
    itsTCLrelease_ = !hold && redrawRequired;
    
    // Update MS/cache as needed.
    bool msSuccess = true, callCacheLoaded = true;
    if(params.isSet()) {
        if(msUpdated || cacheUpdated) {
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
        itsVisSet_ = new VisSet(itsSelectedMS_, columns, Matrix<int>(),
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

void PlotMSPlot::cacheLoaded_(bool wasCanceled) {
    // End log as needed.
    if(itsTCLendLog_) endLogCache();
    
    // Update canvas as needed.
    if(itsTCLupdateCanvas_) updateCanvas();
    
    // Update plot as needed.
    if(itsTCLupdatePlot_) updatePlot();
    
    // Release drawing if needed.
    if(itsTCLrelease_) releaseDrawing();
}


//////////////////////////////////
// PLOTMSSINGLEPLOT DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

PlotMSSinglePlot::PlotMSSinglePlot(PlotMS* parent) : PlotMSPlot(parent),
        itsParameters_(itsFactory_) {
    constructorSetup();
}

PlotMSSinglePlot::~PlotMSSinglePlot() { }


// Public Methods //

String PlotMSSinglePlot::name() const {
    String title = itsParameters_.plotTitle();
    if(!itsParameters_.isSet() || title.empty()) return "Single Plot";
    else                                         return title;
}

unsigned int PlotMSSinglePlot::layoutNumRows() const { return 1; }
unsigned int PlotMSSinglePlot::layoutNumCols() const { return 1; }

const PlotMSPlotParameters& PlotMSSinglePlot::parameters() const {
    return itsParameters_; }
PlotMSPlotParameters& PlotMSSinglePlot::parameters() { return itsParameters_; }

const PlotMSSinglePlotParameters& PlotMSSinglePlot::singleParameters() const {
    return itsParameters_; }
PlotMSSinglePlotParameters& PlotMSSinglePlot::singleParameters() {
    return itsParameters_; }


// Protected Methods //

bool PlotMSSinglePlot::initializePlot() {
    PlotMaskedPointDataPtr data(&itsData_, false);
    itsPlot_ = itsFactory_->maskedPlot(data);
    itsPlots_.push_back(itsPlot_);
    return true;
}

bool PlotMSSinglePlot::assignCanvases() {
    if(!itsPlot_.null() && itsCanvases_.size() > 0) {
        // clear up any unused canvases (shouldn't happen)
        itsCanvases_.resize(1);
        
        itsCanvas_ = itsCanvases_[0];        
        itsCanvasMap_[&*itsPlot_] = itsCanvas_;
        
        return true;        
    }
    return false;
}

bool PlotMSSinglePlot::updateCache() {
    vector<PMS::Axis> axes(2);
    axes[0] = itsParameters_.xAxis();
    axes[1] = itsParameters_.yAxis();
    vector<PMS::DataColumn> data(2);
    data[0] = itsParameters_.xDataColumn();
    data[1] = itsParameters_.yDataColumn();
    
    PlotMSCacheThread* ct = new PlotMSCacheThread(this, axes, data,
            itsParameters_.averaging(), true, &PlotMSPlot::cacheLoaded, this);
    itsParent_->getPlotter()->doThreadedOperation(ct);
    return true;
    
    /*
    try {
        // VisSet will be null on an error.
        if(itsParameters_.isSet() && itsVisSet_ != NULL) {
            PMS::Axis x = itsParameters_.xAxis(), y = itsParameters_.yAxis();
            itsData_.loadCache(*itsVisSet_, x, y, itsParameters_.xDataColumn(),
                               itsParameters_.yDataColumn(),
                               itsParameters_.averaging());
            itsData_.setupCache(x, y);
        }
        return true;
        
    } catch(AipsError& err) {
        itsParent_->showError("Could not load cache: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not load cache, for unknown reasons!");
        return false;
    }
    */
}

bool PlotMSSinglePlot::updateCanvas() {
    try {
        // VisSet will be null on an error.
        bool set = itsParameters_.isSet() && itsVisSet_ != NULL;
        
        PlotAxis x = itsParameters_.canvasXAxis(),
                 y = itsParameters_.canvasYAxis();
        
        // Set axes scales
        itsCanvas_->setAxisScale(x, PMS::axisScale(itsParameters_.xAxis()));
        itsCanvas_->setAxisScale(y, PMS::axisScale(itsParameters_.yAxis()));
        
        // Custom ranges
        itsCanvas_->setAxesAutoRescale(true);
        if(set && itsParameters_.xRangeSet() && itsParameters_.yRangeSet())
            itsCanvas_->setAxesRanges(x, itsParameters_.xRange(),
                                      y, itsParameters_.yRange());
        else if(set && itsParameters_.xRangeSet())
            itsCanvas_->setAxisRange(x, itsParameters_.xRange());
        else if(set && itsParameters_.yRangeSet())
            itsCanvas_->setAxisRange(y, itsParameters_.yRange());
        
        // Axes labels
        itsCanvas_->clearAxesLabels();
        if(set) {
            itsCanvas_->setAxisLabel(x, itsParameters_.canvasXAxisLabel());
            itsCanvas_->setAxisLabel(y, itsParameters_.canvasYAxisLabel());
        }
        
        // Show/hide axes
        itsCanvas_->showAxes(false);
        itsCanvas_->showAxis(x, set && itsParameters_.showXAxis());
        itsCanvas_->showAxis(y, set && itsParameters_.showYAxis());
        
        // Legend
        itsCanvas_->showLegend(set && itsParameters_.showLegend(),
                               itsParameters_.legendPosition());
        
        // Canvas title
        itsCanvas_->setTitle(set ? itsParameters_.canvasTitle() : "");    
        return true;
        
    } catch(AipsError& err) {
        itsParent_->showError("Could not update canvas: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not update canvas, for unknown reasons!");
        return false;
    }
}

bool PlotMSSinglePlot::updatePlot() {
    try {
        // Set symbols.
        itsPlot_->setSymbol(itsParameters_.symbol());
        itsPlot_->setMaskedSymbol(itsParameters_.maskedSymbol());    
        
        // Set item axes.
        itsPlot_->setAxes(itsParameters_.canvasXAxis(),
                          itsParameters_.canvasYAxis());
        
        // Set plot title.
        itsPlot_->setTitle(itsParameters_.plotTitle());
        return true;
    } catch(AipsError& err) {
        itsParent_->showError("Could not update plot: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not update plot, for unknown reasons!");
        return false;
    }
}

}
