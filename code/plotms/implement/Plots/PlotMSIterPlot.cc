//# PlotMSIterPlot.cc: Subclass of PlotMSPlot for a single plot/canvas.
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
#include <plotms/Plots/PlotMSIterPlot.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Data/PlotMSCache2.h>

namespace casa {

//////////////////////////////////
// PLOTMSSINGLEPLOT DEFINITIONS //
//////////////////////////////////

// Static //

PlotMSPlotParameters PlotMSIterPlot::makeParameters(PlotMSApp* plotms) {
    PlotMSPlotParameters p = PlotMSPlot::makeParameters(plotms);
    makeParameters(p, plotms);
    return p;
}

void PlotMSIterPlot::makeParameters(PlotMSPlotParameters& params,
        PlotMSApp* plotms) {
    PlotMSPlot::makeParameters(params, plotms);
    
    // Add cache parameters if needed.
    if(params.typedGroup<PMS_PP_Cache>() == NULL)
        params.setGroup<PMS_PP_Cache>();
    
    // Add axes parameters if needed.
    if(params.typedGroup<PMS_PP_Axes>() == NULL)
        params.setGroup<PMS_PP_Axes>();
        
    // Add canvas parameters if needed.
    if(params.typedGroup<PMS_PP_Canvas>() == NULL)
        params.setGroup<PMS_PP_Canvas>();
    
    // Add display parameters if needed.
    if(params.typedGroup<PMS_PP_Display>() == NULL)
        params.setGroup<PMS_PP_Display>();

    // Add iteration parameters if needed.
    if(params.typedGroup<PMS_PP_Iteration>() == NULL)
        params.setGroup<PMS_PP_Iteration>();


}


// Constructors/Destructors //

  PlotMSIterPlot::PlotMSIterPlot(PlotMSApp* parent) : PlotMSPlot(parent),iter_(-1) {
  constructorSetup(); }

PlotMSIterPlot::~PlotMSIterPlot() { }


// Public Methods //

String PlotMSIterPlot::name() const {
    const PMS_PP_MSData* d = itsParams_.typedGroup<PMS_PP_MSData>();
    const PMS_PP_Cache* c = itsParams_.typedGroup<PMS_PP_Cache>();
    const PMS_PP_Display* dp = itsParams_.typedGroup<PMS_PP_Display>();
    
    if(d== NULL || c== NULL || dp == NULL || !d->isSet()) return "Iterated Plot";
    else return dp->titleFormat().getLabel(c->xAxis(), c->yAxis());
}

vector<MaskedScatterPlotPtr> PlotMSIterPlot::plots() const {
    return vector<MaskedScatterPlotPtr>(1, itsPlot_); }

vector<PlotCanvasPtr> PlotMSIterPlot::canvases() const {
    return vector<PlotCanvasPtr>(1, itsCanvas_); }

void PlotMSIterPlot::setupPlotSubtabs(PlotMSPlotTab& tab) const {
    tab.insertDataSubtab(0);
    tab.insertAxesSubtab(1);
    tab.insertIterateSubtab(2);
    tab.insertTransformationsSubtab(3);
    tab.insertCacheSubtab(4);
    tab.insertDisplaySubtab(5);
    tab.insertCanvasSubtab(6);
    tab.insertExportSubtab(7);
    tab.clearSubtabsAfter(8);
}

void PlotMSIterPlot::attachToCanvases() {
  //  cout << " PMSIP::attachToCanvases" << endl;
    itsCanvas_->plotItem(itsPlot_); }

void PlotMSIterPlot::detachFromCanvases() {
  //  cout << " PMSIP::detachFromCanvases" << endl;
    itsCanvas_->removePlotItem(itsPlot_); }


// Protected Methods //

bool PlotMSIterPlot::assignCanvases(PlotMSPages& pages) {

  //  cout << " PMSIP::assignCanvases" << endl;

    // Use the current page.
    if(pages.totalPages() == 0) pages.insertPage();
    PlotMSPage page = pages.currentPage();
    
    // First try to find a canvas that's already there, but unowned.
    bool found = false;
    unsigned int nrows = page.canvasRows(), ncols = page.canvasCols(),
                 row = 0, col = 0;
    for(unsigned int r = 0; !found && r < nrows; r++) {
        for(unsigned int c = 0; !found && c < ncols; c++) {
            if(!page.isOwned(r, c)) {
                row = r;
                col = c;
                found = true;
            }
        }
    }
    
    // If no free canvases were found, add another row or column (whichever is
    // most "square") and use one of those.
    if(!found) {
        if(nrows == 0 && ncols == 0) {
            nrows = ncols = 1;
            row = col = 0;
        } else if(nrows <= ncols) {
            nrows++;
            row = nrows - 1;
            col = 0;
        } else {
            ncols++;
            row = 0;
            col = ncols - 1;
        }
        page.resize(nrows, ncols);
    }
    
    // Set the owner, and replace the page.
    page.setOwner(row, col, this);
    pages.itsPages_[pages.itsCurrentPageNum_] = page;
    
    // Assign the canvas.
    itsCanvas_ = page.canvas(row, col);
    return true;
}

bool PlotMSIterPlot::initializePlot() {

  //  cout << " PMSIP::initializePlot" << endl;

    PlotMaskedPointDataPtr data(&itsData_, false);
    itsPlot_ = itsFactory_->maskedPlot(data);
    
    setColors();

    return true;
}

bool PlotMSIterPlot::parametersHaveChanged_(const PlotMSWatchedParameters& p,
        int updateFlag, bool releaseWhenDone) {

  /*
  cout << "PMSIP::parametersHaveChanged_:  " << boolalpha << endl
       << "  updateFlag = " << updateFlag << endl
       << "  PMS_PP::UPDATE_MSDATA  = " << PMS_PP::UPDATE_MSDATA << " " << (updateFlag & PMS_PP::UPDATE_MSDATA) << endl
       << "  PMS_PP::UPDATE_CACHE   = " << PMS_PP::UPDATE_CACHE << " " << (updateFlag & PMS_PP::UPDATE_CACHE) << endl
       << "  PMS_PP::UPDATE_CANVAS  = " << PMS_PP::UPDATE_CANVAS << " " << (updateFlag & PMS_PP::UPDATE_CANVAS) << endl
       << "  PMS_PP::UPDATE_DISPLAY = " << PMS_PP::UPDATE_DISPLAY << " " << (updateFlag & PMS_PP::UPDATE_DISPLAY) << endl
       << "  PMS_PP::UPDATE_AXES    = " << PMS_PP::UPDATE_AXES << " " << (updateFlag & PMS_PP::UPDATE_AXES) << endl
       << "  PMS_PP::UPDATE_ITERATION = " << PMS_PP::UPDATE_ITERATION << " " << (updateFlag & PMS_PP::UPDATE_ITERATION) << endl
       << endl;
  */

    if(&p != &itsParams_) return true; // shouldn't happen
    
    const PMS_PP_MSData* d = itsParams_.typedGroup<PMS_PP_MSData>();
    if(d == NULL) return true;
    
    // Update TCL params.
    itsTCLParams_.releaseWhenDone = releaseWhenDone;
    itsTCLParams_.updateCanvas = (updateFlag & PMS_PP::UPDATE_MSDATA) ||
            (updateFlag & PMS_PP::UPDATE_CACHE) ||
            (updateFlag & PMS_PP::UPDATE_ITERATION) ||
            (updateFlag & PMS_PP::UPDATE_CANVAS) || 
            (updateFlag & PMS_PP::UPDATE_AXES) || !d->isSet();
    itsTCLParams_.updateDisplay = updateFlag & PMS_PP::UPDATE_DISPLAY;
    itsTCLParams_.endCacheLog = false;
    
    // Update cache if needed.
    if(d->isSet() && (updateFlag & PMS_PP::UPDATE_MSDATA ||
       updateFlag & PMS_PP::UPDATE_CACHE)) {

        return !updateCache();
        
    } else {

        itsTCLParams_.releaseWhenDone = false;
        cacheLoaded_(false);
        return true;
    }
    
}

PlotMSRegions PlotMSIterPlot::selectedRegions(
            const vector<PlotCanvasPtr>& canvases) const {
    PlotMSRegions r;
    PMS::Axis x = (PMS::Axis)PMS_PP_RETCALL(itsParams_,PMS_PP_Cache, yAxis, 0),
              y = (PMS::Axis)PMS_PP_RETCALL(itsParams_,PMS_PP_Cache, yAxis, 0);
    if(x == 0 || y == 0) return r; // shouldn't happen
    for(unsigned int i = 0; i < canvases.size(); i++)
        r.addRegions(x, y, canvases[i]);
    return r;
}

void PlotMSIterPlot::constructorSetup() {
    PlotMSPlot::constructorSetup();
    makeParameters(itsParams_, itsParent_);
}


// Private Methods //

bool PlotMSIterPlot::updateCache() {

  //  cout << " PMSIP::updateCache" << endl;

    PMS_PP_MSData* d = itsParams_.typedGroup<PMS_PP_MSData>();
    PMS_PP_Cache* c = itsParams_.typedGroup<PMS_PP_Cache>();
    if(d == NULL || c == NULL) return false; // shouldn't happen
    
    // Don't load if data isn't set or there was an error during data opening.
    if(!d->isSet()) return false;
    
    // Let the plot know that the data (will) change.
    itsPlot_->dataChanged();
    
    // Set up cache loading parameters.
    vector<PMS::Axis> axes(2);
    axes[0] = c->xAxis();
    axes[1] = c->yAxis();
    vector<PMS::DataColumn> data(2);
    data[0] = c->xDataColumn();
    data[1] = c->yDataColumn();
    
    // Log the cache loading.
    itsParent_->getLogger()->markMeasurement(PMS::LOG_ORIGIN,
            PMS::LOG_ORIGIN_LOAD_CACHE, PMS::LOG_EVENT_LOAD_CACHE);
    itsTCLParams_.endCacheLog = true;

    PlotMSCacheThread* ct = new PlotMSCacheThread(this, &itsCache2_, axes, data,
						  d->filename(), 
						  d->selection(), 
						  d->averaging(), 
						  d->transformations(), 
						  false, 
						  &PlotMSIterPlot::cacheLoaded, this);
    itsParent_->getPlotter()->doThreadedOperation(ct);
    
    return true;
}


bool PlotMSIterPlot::updateIndexing() {

  PMS_PP_Iteration* it = itsParams_.typedGroup<PMS_PP_Iteration>();

  /*
  cout << "PMSIP::updateIndexing:  iteraxis = " 
       << PMS::axis(it->iterationAxis()) 
       << " GlobalXRange = " << it->globalXRange() 
       << " GlobalYRange = " << it->globalYRange() 
       << endl;
  */

  itsCache2_.setUpIndexer(it->iterationAxis(),
			  it->globalXRange(),it->globalYRange());

  return true;

}

bool PlotMSIterPlot::firstIter() {

  Int nIter=itsCache2_.nIter();  

  // If more than one iteration and not already at first one
  if (nIter>0 && iter_!=0) {
    detachFromCanvases();
    iter_=0;
    cout << "Stepping to iteration = " << iter_ << " (of " << nIter << "): " << itsCache2_.indexer(iter_).iterLabel() << endl;
    PlotMaskedPointDataPtr data(&(itsCache2_.indexer(iter_)), false);
    itsPlot_ = itsFactory_->maskedPlot(data);
    setColors();
    //    itsTCLParams_.updateDisplay=True;  // needed?
    attachToCanvases();
    
    // Since we detached, we have to re-update Canvas and Display
    updateCanvas();
    updateDisplay();

    // Report the number of points plotted
    logPoints();

    return true;
  }
 
  // Apparently no more iterations, so return False
  return false;

}



bool PlotMSIterPlot::prevIter() {

  Int nIter=itsCache2_.nIter();

  // If more than one iteration and not already at first one
  if (nIter>0 && iter_> 0) {
    detachFromCanvases();
    iter_-=1;
    cout << "Stepping to iteration = " << iter_ << " (of " << nIter << "): " << itsCache2_.indexer(iter_).iterLabel() << endl;
    PlotMaskedPointDataPtr data(&(itsCache2_.indexer(iter_)), false);
    itsPlot_ = itsFactory_->maskedPlot(data);
    setColors();
    //    itsTCLParams_.updateDisplay=True;  // needed?
    attachToCanvases();
    
    // Since we detached, we have to re-update Canvas and Display
    updateCanvas();
    updateDisplay();

    // Report the number of points plotted
    logPoints();

    return true;
  }
 
  // Apparently no more iterations, so return False
  return false;

}

bool PlotMSIterPlot::nextIter() {

  Int nIter=itsCache2_.nIter();

  // If more than one iteration and not already at end
  if (nIter>0 && iter_<nIter-1) {
    detachFromCanvases();
    iter_+=1;
    cout << "Stepping to iteration = " << iter_ << " (of " << nIter << "): " << itsCache2_.indexer(iter_).iterLabel() << endl;
    PlotMaskedPointDataPtr data(&(itsCache2_.indexer(iter_)), false);
    itsPlot_ = itsFactory_->maskedPlot(data);
    setColors();
    //    itsTCLParams_.updateDisplay=True;  // needed?
    attachToCanvases();
    
    // Since we detached, we have to re-update Canvas and Display
    updateCanvas();
    updateDisplay();

    // Report the number of points plotted
    logPoints();

    return true;
  }
 
  // Apparently no more iterations, so return False
  return false;

}

bool PlotMSIterPlot::lastIter() {

  Int nIter=itsCache2_.nIter();

  // If more than one iteration and not already at last one
  if (nIter>0 && iter_!=(nIter-1)) {
    detachFromCanvases();
    iter_=nIter-1;
    cout << "Stepping to iteration = " << iter_ << " (of " << nIter << "): " << itsCache2_.indexer(iter_).iterLabel() << endl;
    PlotMaskedPointDataPtr data(&(itsCache2_.indexer(iter_)), false);
    itsPlot_ = itsFactory_->maskedPlot(data);
    setColors();
    //    itsTCLParams_.updateDisplay=True;  // needed?
    attachToCanvases();
    
    // Since we detached, we have to re-update Canvas and Display
    updateCanvas();
    updateDisplay();

    // Report the number of points plotted
    logPoints();
    return true;
  }
 
  // Apparently no more iterations, so return False
  return false;

}



bool PlotMSIterPlot::resetIter() {

  Int nIter=itsCache2_.nIter();

  if (itsCache2_.nIter()>0) {
    detachFromCanvases();
    iter_=0;
    cout << "Reset to iteration = " << iter_ << " (of " << nIter << "): " << itsCache2_.indexer(iter_).iterLabel() << endl;
    PlotMaskedPointDataPtr data(&(itsCache2_.indexer(iter_)), false);
    itsPlot_ = itsFactory_->maskedPlot(data);
    setColors();
    itsTCLParams_.updateDisplay=True;
    attachToCanvases();

    updateCanvas();
    updateDisplay();
    
    logPoints();
    
    itsPlot_->dataChanged();
  }

  return true;

}

bool PlotMSIterPlot::updateCanvas() {

    try {

        bool set = PMS_PP_RETCALL(itsParams_, PMS_PP_MSData, isSet, false); 

        PMS_PP_Axes* a = itsParams_.typedGroup<PMS_PP_Axes>();
        PMS_PP_Cache* d = itsParams_.typedGroup<PMS_PP_Cache>();
        PMS_PP_Canvas* c = itsParams_.typedGroup<PMS_PP_Canvas>();
	PMS_PP_Iteration* it = itsParams_.typedGroup<PMS_PP_Iteration>();
        if(a== NULL || d== NULL || c== NULL || it==NULL) return false; // shouldn't happen
	
	String itertxt="";
	if (it->iterationAxis()!=PMS::NONE && itsCache2_.nIter()>0)
	  itertxt=itsCache2_.indexer(iter_).iterLabel();

        PlotAxis cx = a->xAxis(); 
        PlotAxis cy = a->yAxis();
        PMS::Axis x = d->xAxis();
        PMS::Axis y = d->yAxis();
        
        // Set axes scales
        itsCanvas_->setAxisScale(cx, PMS::axisScale(x));
        itsCanvas_->setAxisScale(cy, PMS::axisScale(y));
        
        // Set reference values.
        bool xref = itsCache2_.hasReferenceValue(x),
             yref = itsCache2_.hasReferenceValue(y);
        double xrefval = itsCache2_.referenceValue(x),
               yrefval = itsCache2_.referenceValue(y);
        itsCanvas_->setAxisReferenceValue(cx, xref, xrefval);
        itsCanvas_->setAxisReferenceValue(cy, yref, yrefval);
        
        // Set axes labels.
        itsCanvas_->clearAxesLabels();
        if(set) {
            itsCanvas_->setAxisLabel(cx, c->xLabelFormat().getLabel(
                    x, xref, xrefval));
            itsCanvas_->setAxisLabel(cy, c->yLabelFormat().getLabel(
                    y, yref, yrefval));
        }
        
        // Custom ranges
        itsCanvas_->setAxesAutoRescale(true);
        if(set && a->xRangeSet() && a->yRangeSet())
            itsCanvas_->setAxesRanges(cx, a->xRange(), cy, a->yRange());
        else if(set && a->xRangeSet())
            itsCanvas_->setAxisRange(cx, a->xRange());
        else if(set && a->yRangeSet())
            itsCanvas_->setAxisRange(cy, a->yRange());
        
        // Show/hide axes
        bool showx = set && c->xAxisShown();
        bool showy = set && c->yAxisShown();
        itsCanvas_->showAllAxes(false);
        itsCanvas_->showAxis(cx, showx);
        itsCanvas_->showAxis(cy, showy);
        
        // Legend
        itsCanvas_->showLegend(set && c->legendShown(), c->legendPosition());
        
        // Canvas title
	Bool resetTitle(set || it->iterationAxis()!=PMS::NONE); 
        itsCanvas_->setTitle(resetTitle ? c->titleFormat().getLabel(x, y,
                xref, xrefval, yref, yrefval)+" "+itertxt : "");
        
        // Grids
        itsCanvas_->showGrid(c->gridMajorShown(), c->gridMinorShown(),
                c->gridMajorShown(), c->gridMinorShown());
        
        PlotLinePtr line = itsFactory_->line(c->gridMajorLine());
        if(!c->gridMajorShown()) line->setStyle(PlotLine::NOLINE);
        itsCanvas_->setGridMajorLine(line);
        
        line = itsFactory_->line(c->gridMinorLine());
        if(!c->gridMinorShown()) line->setStyle(PlotLine::NOLINE);
        itsCanvas_->setGridMinorLine(line);
        
        return true;
        
    } catch(AipsError& err) {
        itsParent_->showError("Could not update canvas: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not update canvas, for unknown reasons!");
        return false;
    }
}

bool PlotMSIterPlot::updateDisplay() {

    try {
        PMS_PP_Cache* h = itsParams_.typedGroup<PMS_PP_Cache>();
        PMS_PP_Axes* a = itsParams_.typedGroup<PMS_PP_Axes>();
        PMS_PP_Display* d = itsParams_.typedGroup<PMS_PP_Display>();
        
        // shouldn't happen
        if(h == NULL || a == NULL || d == NULL) return false;
        
        // Set symbols.
	/*	
	cout << "Unflagged symbol=" << d->unflaggedSymbol()->symbol() 
	     << " size=" << d->unflaggedSymbol()->size().first << " " << d->unflaggedSymbol()->size().second
	     << " fill=" <<  d->unflaggedSymbol()->areaFill()->pattern()
	     << " color=" <<  d->unflaggedSymbol()->areaFill()->color()->asName() 
	     << endl;
	cout << "Flagged   symbol=" << d->flaggedSymbol()->symbol()
	     << " size=" << d->flaggedSymbol()->size().first << " " << d->flaggedSymbol()->size().second
	     << " fill=" <<  d->flaggedSymbol()->areaFill()->pattern()
	     << " color=" <<  d->flaggedSymbol()->areaFill()->color()->asName()
	     << endl;
	*/

        itsPlot_->setSymbol(d->unflaggedSymbol());
        itsPlot_->setMaskedSymbol(d->flaggedSymbol());
        
        // Colorize, and set data changed if redraw is needed.
	if(itsCache2_.nIter()>0 && 
	   itsCache2_.indexer(iter_).colorize(d->colorizeFlag(), d->colorizeAxis()))
	  itsPlot_->dataChanged();
        
        // Set item axes.
        itsPlot_->setAxes(a->xAxis(), a->yAxis());
        
        // Set plot title.
        PMS::Axis x = h->xAxis(), y = h->yAxis();
        itsPlot_->setTitle(d->titleFormat().getLabel(x,y,
                itsCache2_.hasReferenceValue(x), itsCache2_.referenceValue(x),
                itsCache2_.hasReferenceValue(y), itsCache2_.referenceValue(y)));
        return true;
    } catch(AipsError& err) {
        itsParent_->showError("Could not update plot: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not update plot, for unknown reasons!");
        return false;
    }
}

void PlotMSIterPlot::cacheLoaded_(bool wasCanceled) {

  /*
  cout << " PMSIP::cacheLoaded_" << boolalpha << " wasCanceled=" << wasCanceled << " "
       << "itsTCLParams_.updateCanvas=" << itsTCLParams_.updateCanvas << " "
       << "itsTCLParams_.updateDisplay=" << itsTCLParams_.updateDisplay << " "
       << "itsTCLParams_.releaseWhenDone=" << itsTCLParams_.releaseWhenDone << " "
       << endl;
  */

  // Make this more specific than canvas-triggered
  if (!wasCanceled && itsTCLParams_.updateCanvas) updateIndexing();


  // Reset the iterator (conditions for this need refining)
  resetIter();

    // Let the plot know that the data has been changed as needed, unless the
    // thread was canceled.
    if(wasCanceled) itsPlot_->dataChanged();
    
    // End cache log as needed.
    if(itsTCLParams_.endCacheLog)
        itsParent_->getLogger()->releaseMeasurement();
    
    // Update canvas as needed.
    if(!wasCanceled && itsTCLParams_.updateCanvas) updateCanvas();
    
    // Update display as needed.
    if(!wasCanceled && itsTCLParams_.updateDisplay) updateDisplay();
    
    // Release drawing if needed.
    if(itsTCLParams_.releaseWhenDone) releaseDrawing();
    
    // Log number of points as needed.
    if(!wasCanceled && itsTCLParams_.endCacheLog)
      logPoints();


}

void PlotMSIterPlot::setColors() {

    // Set colors using list.
    itsColoredPlot_ = ColoredPlotPtr(dynamic_cast<ColoredPlot*>(&*itsPlot_),
                                     false);
    if(!itsColoredPlot_.null()) {
        const vector<String>& colors = PMS::COLORS_LIST();
        for(unsigned int i = 0; i < colors.size(); i++)
            itsColoredPlot_->setColorForBin(i, itsFactory_->color(colors[i]));
    }
}    

void PlotMSIterPlot::logPoints() {

  PMS_PP_Display* d = itsParams_.typedGroup<PMS_PP_Display>();
  Bool showUnflagged=(d->unflaggedSymbol()->symbol()!=PlotSymbol::NOSYMBOL);
  Bool showFlagged=(d->flaggedSymbol()->symbol()!=PlotSymbol::NOSYMBOL);
  
  stringstream ss;
  ss << "Plotting ";
  if (showUnflagged) ss << itsCache2_.indexer(iter_).sizeUnmasked() << " unflagged"
			<< (showFlagged ? ", " : "");
  if (showFlagged) ss << itsCache2_.indexer(iter_).sizeMasked() << " flagged";
  ss << " points.";
      
  itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
				       PMS::LOG_ORIGIN_PLOT, ss.str(), PMS::LOG_EVENT_PLOT);
}


}
