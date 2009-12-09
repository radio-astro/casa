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
        itsParams_(itsFactory_), itsData_(parent) { }

PlotMSPlot::~PlotMSPlot() {
    
    // Clean up plots
    // detachFromCanvases();
}


// Public Methods //

const PlotMSPlotParameters& PlotMSPlot::parameters() const{ return itsParams_;}
PlotMSPlotParameters& PlotMSPlot::parameters() { return itsParams_; }

vector<PlotCanvasPtr> PlotMSPlot::visibleCanvases() const {
    vector<PlotCanvasPtr> v, canv = canvases(),
                          visCanv= itsParent_->getPlotter()->currentCanvases();
    bool found = false;
    for(unsigned int i = 0; i < canv.size(); i++) {
        found = false;
        for(unsigned int j = 0; !found && j < visCanv.size(); j++)
            if(canv[i] == visCanv[j]) found = true;
        if(found) v.push_back(canv[i]);
    }
    return v;
}

PlotMSRegions PlotMSPlot::selectedRegions() const {
    return selectedRegions(canvases()); }
PlotMSRegions PlotMSPlot::visibleSelectedRegions() const {
    return selectedRegions(visibleCanvases()); }

bool PlotMSPlot::initializePlot(PlotMSPages& pages) {    
    bool hold = allDrawingHeld();
    if(!hold) holdDrawing();
    
    // Initialize plot objects and assign canvases.
    if(!assignCanvases(pages) || !initializePlot()) {
        if(!hold) releaseDrawing();
        return false;
    }
    
    // Set up page.
    pages.setupCurrentPage();
    
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

/*
void PlotMSPlot::attachToCanvases() {
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        itsCanvasMap_[&*itsPlots_[i]]->plotItem(itsPlots_[i]);
}

void PlotMSPlot::detachFromCanvases() {
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        itsCanvasMap_[&*itsPlots_[i]]->removePlotItem(itsPlots_[i]);
}
*/

PlotMSData& PlotMSPlot::data() { return itsData_; }
const PlotMSData& PlotMSPlot::data() const { return itsData_; }

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
    
    vector<MaskedScatterPlotPtr> p = plots();
    for(unsigned int i = 0; i < p.size(); i++)
        if(!p[i].null()) p[i]->dataChanged();
    
    if(!hold) releaseDrawing();
}

bool PlotMSPlot::exportToFormat(const PlotExportFormat& format) {
    vector<PlotCanvasPtr> canv = canvases();
    if(canv.size() == 0) return true;
    else if(canv.size() == 1)
        return canv[0]->exportToFile(format);
    else {
        bool success = true;
        
        PlotExportFormat form(format);
        for(unsigned int i = 0; i < canv.size(); i++) {
            form.location = String::toString(i) + "-" + format.location;
            if(!canv[i]->exportToFile(form)) success = false;
        }
        
        return success;        
    }
}

void PlotMSPlot::canvasWasDisowned(PlotCanvasPtr canvas) {
    if(canvas.null()) return;

    vector<MaskedScatterPlotPtr> p = plots();
    for(unsigned int i = 0; i < p.size(); i++)
        if(!p[i].null()) canvas->removePlotItem(p[i]);
}


// Protected Methods //

void PlotMSPlot::constructorSetup() {
    PlotMSPlotParameters& params = parameters();
    params.addWatcher(this);
    
    // hold notification until initializePlot is called
    params.holdNotification(this);
    
    makeParameters(params, itsParent_);
}


bool PlotMSPlot::allDrawingHeld() {
    vector<PlotCanvasPtr> canv = canvases();
    for(unsigned int i = 0; i < canv.size(); i++)
        if(!canv[i].null() && !canv[i]->drawingIsHeld()) return false;
    return true;
}

void PlotMSPlot::holdDrawing() {
    vector<PlotCanvasPtr> canv = canvases();
    for(unsigned int i = 0; i < canv.size(); i++) canv[i]->holdDrawing();
}

void PlotMSPlot::releaseDrawing() {
    vector<PlotCanvasPtr> canv = canvases();
    for(unsigned int i = 0; i < canv.size(); i++)
        if(!canv[i].null()) canv[i]->releaseDrawing();
}

}


  /*  in the following, VisSet is commented
bool PlotMSPlot::updateData() {
    
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
	OrderedMap<Int, Vector<Vector<Int> > > corrsel(Vector<Vector<Int> >(0));
        d->selection().apply(itsMS_, itsSelectedMS_, chansel,corrsel);

	cout << "chansel = " << chansel << endl;
        
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
	//        itsVisSet_->selectChannel(chansel);

	itsVisSet_->iter().selectChannel(chansel);
	itsVisSet_->iter().selectCorrelation(corrsel);


        return true;
        
    } catch(AipsError& err) {
        itsParent_->showError("Could not open MS: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not open MS, for unknown reasons!");
        return false;
    }
}
  */
