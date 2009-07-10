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

#include <msvis/MSVis/VisSet.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

////////////////////////////
// PLOTMSPLOT DEFINITIONS //
////////////////////////////

// Static //

PlotMSPlotParameters PlotMSPlot::makeParameters(PlotMS* plotms) {
    PlotMSPlotParameters p(plotms->getPlotter()->getFactory());
    makeParameters(p, plotms);
    return p;    
}

void PlotMSPlot::makeParameters(PlotMSPlotParameters& params, PlotMS* plotms) {
    // Add data parameters if needed.
    if(params.typedGroup<PMS_PP_MSData>() == NULL)
        params.setGroup<PMS_PP_MSData>();
}


// Constructors/Destructors //

PlotMSPlot::PlotMSPlot(PlotMS* parent) : itsParent_(parent),
        itsFactory_(parent->getPlotter()->getFactory()),
        itsParams_(itsFactory_), itsVisSet_(NULL), itsData_(parent) { }

PlotMSPlot::~PlotMSPlot() {
    // Clean up MS
    if(itsVisSet_ != NULL) delete itsVisSet_;
    
    // Clean up plots
    detachFromCanvases();
}


// Public Methods //

const PlotMSPlotParameters& PlotMSPlot::parameters() const{ return itsParams_;}
PlotMSPlotParameters& PlotMSPlot::parameters() { return itsParams_; }

vector<PlotCanvasPtr> PlotMSPlot::canvases() const { return itsCanvases_; }

vector<PlotCanvasPtr> PlotMSPlot::visibleCanvases() const {
    vector<PlotCanvasPtr> v, canv= itsParent_->getPlotter()->currentCanvases();
    bool found = false;
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        found = false;
        for(unsigned int j = 0; !found && j < canv.size(); j++)
            if(itsCanvases_[i] == canv[j]) found = true;
        if(found) v.push_back(itsCanvases_[i]);
    }
    return v;
}

bool PlotMSPlot::initializePlot(const vector<PlotCanvasPtr>& canvases) {
    if(canvases.size() != layoutNumCanvases()) return false;
    itsCanvases_ = canvases;
    
    bool hold = allDrawingHeld();
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
    parametersHaveChanged(parameters(),
            PlotMSWatchedParameters::ALL_UPDATE_FLAGS());
    
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
MeasurementSet& PlotMSPlot::ms() { return itsMS_; }
const MeasurementSet& PlotMSPlot::ms() const { return itsMS_; }
MeasurementSet& PlotMSPlot::selectedMS() { return itsSelectedMS_; }
const MeasurementSet& PlotMSPlot::selectedMS() const { return itsSelectedMS_; }

PlotMS* PlotMSPlot::parent() { return itsParent_; }

void PlotMSPlot::parametersHaveChanged(const PlotMSWatchedParameters& p,
        int updateFlag) {    
    // Make sure it's this plot's parameters.
    if(&p != &parameters()) return;
    
    vector<String> updates =
        PlotMSWatchedParameters::UPDATE_FLAG_NAMES(updateFlag);
    if(updates.size() == 0) return;
    
    // Log what we're going to be updating.
    stringstream ss;
    ss << "Updating: ";
    for(unsigned int i = 0; i < updates.size(); i++) {
        if(i > 0) ss << ", ";
        ss << updates[i];
    }
    ss << ".";
    itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
            PMS::LOG_ORIGIN_PARAMS_CHANGED, ss.str(),
            PMS::LOG_EVENT_PARAMS_CHANGED);
    
    bool releaseWhenDone = !allDrawingHeld() &&
                           (updateFlag & PMS_PP::UPDATE_REDRAW);
    if(releaseWhenDone) holdDrawing();
    
    // Update MS as needed.
    const PMS_PP_MSData* d = parameters().typedGroup<PMS_PP_MSData>();
    bool dataSuccess = d->isSet();
    if(dataSuccess && (updateFlag & PMS_PP::UPDATE_MSDATA))
        dataSuccess = updateData();

    // If something went wrong, clear the cache and plots.
    if(!dataSuccess) {
        itsData_.clearCache();
        plotDataChanged();
    }
    
    // Let the child handle the rest of the parameter changes, and release
    // drawing if needed.
    if(parametersHaveChanged_(p,updateFlag,releaseWhenDone) && releaseWhenDone)
        releaseDrawing();
}

void PlotMSPlot::plotDataChanged() {
    bool hold = allDrawingHeld();
    if(!hold) holdDrawing();
    
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        itsPlots_[i]->dataChanged();
    
    if(!hold) releaseDrawing();
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

void PlotMSPlot::canvasWasDisowned(PlotCanvasPtr canvas) {
    if(canvas.null()) return;

    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        canvas->removePlotItem(itsPlots_[i]);
}


// Protected Methods //

void PlotMSPlot::constructorSetup() {
    PlotMSPlotParameters& params = parameters();
    params.addWatcher(this);
    
    // hold notification until initializePlot is called
    params.holdNotification(this);
    
    makeParameters(params, itsParent_);
}

bool PlotMSPlot::updateData() {
    if(itsVisSet_ != NULL) {
        delete itsVisSet_;
        itsVisSet_ = NULL;
    }
    
    PMS_PP_MSData* d = parameters().typedGroup<PMS_PP_MSData>();
    if(d == NULL) return false; // shouldn't happen
    
    try {
        // Clear cache.
        itsData_.clearCache();
        
        // Open MS and lock until we're done.
        itsMS_ = MeasurementSet(d->filename(),
                TableLock(TableLock::AutoLocking), Table::Update);

        // Apply the MS selection.
        Matrix<Int> chansel;
        d->selection().apply(itsMS_, itsSelectedMS_, chansel);
        
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

bool PlotMSPlot::allDrawingHeld() {
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        if(!itsCanvases_[i]->drawingIsHeld()) return false;
    return true;
}

void PlotMSPlot::holdDrawing() {
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        itsCanvases_[i]->holdDrawing();
}

void PlotMSPlot::releaseDrawing() {
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        itsCanvases_[i]->releaseDrawing();
}

}
