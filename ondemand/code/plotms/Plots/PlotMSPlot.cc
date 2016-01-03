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
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Data/MSCache.h>
#include <QDebug>


namespace casa {

////////////////////////////
// PLOTMSPLOT DEFINITIONS //
////////////////////////////

// Static //

PlotMSPlotParameters PlotMSPlot::makeParameters(PlotMSApp* plotms) {
    PlotMSPlotParameters p(plotms->getPlotFactory());
    makeParameters(p, plotms);
    return p;    
}

void PlotMSPlot::makeParameters(PlotMSPlotParameters& params, PlotMSApp* /*plotms*/) {
    // Add data parameters if needed.
    if(params.typedGroup<PMS_PP_MSData>() == NULL)
        params.setGroup<PMS_PP_MSData>();
}
const uInt PlotMSPlot::PIXEL_THRESHOLD = 1000000;
const uInt PlotMSPlot::MEDIUM_THRESHOLD = 10000;
const uInt PlotMSPlot::LARGE_THRESHOLD = 1000;

// Constructors/Destructors //

PlotMSPlot::PlotMSPlot(PlotMSApp* parent) : 
  itsParent_(parent),
  itsFactory_(parent->getPlotFactory()),
  itsParams_(itsFactory_),
  itsCache_(NULL) { 
  
  itsCache_ = new MSCache(itsParent_);

}

PlotMSPlot::~PlotMSPlot() {
    
    // Clean up plots
    // detachFromCanvases();

  if (itsCache_)
    delete itsCache_;
  itsCache_=NULL;

}
void PlotMSPlot::customizeAutoSymbol( const PlotSymbolPtr& baseSymbol, uInt dataSize ){
	if( baseSymbol->symbol() == PlotSymbol::AUTOSCALING) {

		if( dataSize > PIXEL_THRESHOLD ) {
			baseSymbol->setSymbol(PlotSymbol::PIXEL);
		    baseSymbol->setSize(1,1);
		}
		else if( dataSize > MEDIUM_THRESHOLD ) {
		    baseSymbol->setSymbol( PlotSymbol::CIRCLE);
		    baseSymbol->setSize(2,2);
		}
		else if( dataSize > LARGE_THRESHOLD ) {
		    baseSymbol->setSymbol( PlotSymbol::CIRCLE );
		    baseSymbol->setSize(4,4);
		}
		else {
		    baseSymbol->setSymbol( PlotSymbol::CIRCLE );
		    baseSymbol->setSize(6,6);
		}
	}
}

// Public Methods //

vector<PMS::DataColumn> PlotMSPlot::getCachedData(){
	PMS_PP_Cache* cache = itsParams_.typedGroup<PMS_PP_Cache>();
	int count = cache->numXAxes() + cache->numYAxes();
	vector<PMS::DataColumn> cdata( count );
	for(uInt i = 0; i < cache->numXAxes(); ++i) {
		cdata[i] = cache->xDataColumn(i);
	}
	for( int i = cache->numYAxes(); i < count; ++i) {
	    cdata[i] = cache->yDataColumn(i - cache->numXAxes());
	}
	return cdata;
}


vector<PMS::Axis> PlotMSPlot::getCachedAxes() {
	PMS_PP_Cache* c = itsParams_.typedGroup<PMS_PP_Cache>();
	vector<PMS::Axis> axes(c->numXAxes() + c->numYAxes());
	for(unsigned int i = 0; i < c->numXAxes(); i++)
		axes[i] = c->xAxis(i);
	for(unsigned int i = c->numXAxes(); i < axes.size(); i++)
	    axes[i] = c->yAxis(i - c->numXAxes());
	/*vector<PMS::Axis> axes(2);
	axes[0] = c->xAxis();
	axes[1] = c->yAxis();*/
	return axes;
}

const PlotMSPlotParameters& PlotMSPlot::parameters() const{ return itsParams_;}
PlotMSPlotParameters& PlotMSPlot::parameters() { return itsParams_; }

vector<PlotCanvasPtr> PlotMSPlot::visibleCanvases() const {
    vector<PlotCanvasPtr> v;
    vector<PlotCanvasPtr> canv = canvases();
                         // visCanv= itsParent_->getPlotter()->currentCanvases();
    //bool found = false;
    for(unsigned int i = 0; i < canv.size(); i++) {
        /*found = false;
        for(unsigned int j = 0; !found && j < visCanv.size(); j++)
            if(canv[i] == visCanv[j]) found = true;
        if(found) v.push_back(canv[i]);*/
    	bool visible = itsParent_->isVisible( canv[i] );
    	if ( visible ){
    		v.push_back( canv[i] );
    	}
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
    if(dataSuccess && (updateFlag & PMS_PP::UPDATE_MSDATA)){
    	bool fileExists = true;
    	String fileName = d->filename();
    	ifstream ifile( fileName.c_str() );
    	if ( !ifile ){
    		fileExists = false;
    	}
    	else {
    		ifile.close();
    	}

    	if ( fileExists ){
    		dataSuccess = updateData();
    	}
    	else {
    		String errorMessage( "Please check that the file ");
    		errorMessage.append( fileName );
    		errorMessage.append( " is valid.");
    		itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN, PMS::LOG_ORIGIN_PLOT, errorMessage, PlotLogger::MSG_WARN);
    		dataSuccess = false;
    	}
    }

    // If something went wrong, clear the cache and plots.
    if(!dataSuccess) {
        itsCache_->clear();
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
    bool exportSuccess = true;

    //Determine how many pages we need to print.
    int pageCount = 1;
    PMS_PP_Export* exportParams = itsParams_.typedGroup<PMS_PP_Export>();
    if ( exportParams != NULL ){
    	PMS::ExportRange range = exportParams->getExportRange();
    	if ( range == PMS::PAGE_ALL ){
    	    pageCount = itsCache_->nIter();
    	    float divResult = (pageCount * 1.0f) / canv.size();
    	    pageCount = static_cast<int>(ceil( divResult ));
    	}
    }

    //Store the current page.
    Int currentIter = iter();

    //Loop over all the iterations, exporting them
    firstIter();
    PlotExportFormat exportFormat( format );
    String baseFileName = format.location;
    String suffix = "";
    int periodIndex = baseFileName.find_last_of( ".");
    if ( periodIndex != static_cast<int>(String::npos) ){
    	suffix = baseFileName.substr( periodIndex, baseFileName.size() - periodIndex);
    	baseFileName = baseFileName.substr(0, periodIndex );
    }

    for ( int i = 0; i < pageCount; i++ ){
    	int canvasCount = canv.size();
    	if ( i > 0 ){
    		//Remove the last '.' from the storage location.
    		String pageStr = String::toString( i+1 );
    		exportFormat.location = baseFileName + pageStr + suffix;
    	}
    	if(canvasCount == 1){
    		exportSuccess = canv[0]->exportToFile(exportFormat);
    	}
    	else if ( canvasCount > 1){
    		exportSuccess = itsParent_->exportToFormat( exportFormat );
    	}
    	nextIter();
    }

    //Restore the current page
    setIter( currentIter );
    return exportSuccess;
}

void PlotMSPlot::exportToFormatCancel(){
	vector<PlotCanvasPtr> canv = canvases();
	PlotOperationPtr op;
	for(unsigned int i = 0; i < canv.size(); i++) {
		if(canv[i].null()) continue;
	    op = canv[i]->operationExport();
	    if(op.null()) continue;
	    op->setCancelRequested(true);
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

