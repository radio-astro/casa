//# PlotMSMultiPlot.cc: plot with one x (or y) vs. many y's (or x's).
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
#include <plotms/Plots/PlotMSMultiPlot.h>

//#include <plotms/Gui/PlotMSPlotter.qo.h>
//#include <plotms/GuiTabs/PlotMSDisplayTab.qo.h>
//#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPage.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Plots/PlotMSSinglePlot.h>

namespace casa {

/////////////////////////////////
// PLOTMSMULTIPLOT DEFINITIONS //
/////////////////////////////////

// Static //

PlotMSPlotParameters PlotMSMultiPlot::makeParameters(PlotMSApp* plotms) {

  cout << "PMSMP::makeParameters" << endl;

    PlotMSPlotParameters p = PlotMSPlot::makeParameters(plotms);
    makeParameters(p, plotms);
    return p;
}

void PlotMSMultiPlot::makeParameters(PlotMSPlotParameters& params, PlotMSApp* plotms) {
    // Uses the same parameter groups as PlotMSSinglePlot.

  cout << "PMSMP::makeParameters2" << endl;

    PlotMSSinglePlot::makeParameters(params, plotms);
    
    // Add iteration parameters if needed.
    if(params.typedGroup<PMS_PP_Iteration>() == NULL)
        params.setGroup<PMS_PP_Iteration>();

}


// Constructors/Destructors //

PlotMSMultiPlot::PlotMSMultiPlot(PlotMSApp* parent) : PlotMSPlot(parent) {
  cout << "PMSMP::PlotMSMultiPlot" << endl;
    constructorSetup(); }

PlotMSMultiPlot::~PlotMSMultiPlot() {
  cout << "PMSMP::~PlotMSMultiPlot" << endl;
  
    for(unsigned int i = 0; i < itsIndexers_.size(); i++)
        for(unsigned int j = 0; j < itsIndexers_[i].size(); j++)
            delete itsIndexers_[i][j];
    itsIndexers_.clear();
}


// Public Methods //

String PlotMSMultiPlot::name() const {

    const PMS_PP_MSData* d = itsParams_.typedGroup<PMS_PP_MSData>();
    
    if(d == NULL || !d->isSet()) return "Multi Plot";
    else return "Multi Plot for " + d->filename();
}

vector<MaskedScatterPlotPtr> PlotMSMultiPlot::plots() const {

    if(itsPlots_.size() == 0 || itsPlots_[0].size() == 0) // shouldn't happen
        return vector<MaskedScatterPlotPtr>();
    vector<MaskedScatterPlotPtr> v(itsPlots_.size() * itsPlots_[0].size());
    unsigned int index = 0;
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        for(unsigned int j = 0; j < itsPlots_[i].size(); j++)
            v[index++] = itsPlots_[i][j];
    return v;
}

vector<PlotCanvasPtr> PlotMSMultiPlot::canvases() const {

    if(itsCanvases_.size() == 0 || itsCanvases_[0].size() == 0) // shouldn't happen
        return vector<PlotCanvasPtr>();
    vector<PlotCanvasPtr> v(itsCanvases_.size() * itsCanvases_[0].size());
    unsigned int index = 0;
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        for(unsigned int j = 0; j < itsCanvases_[i].size(); j++)
            v[index++] = itsCanvases_[i][j];
    return v;
}

void PlotMSMultiPlot::setupPlotSubtabs(/*PlotMSPlotTab*/PlotInformationManager& tab) const {

    tab.insertData(0);
    tab.insertAxes(1);
    tab.insertIterate(2);
    tab.insertCache(3);
    tab.insertDisplay(4);
    tab.insertCanvas(5);
    tab.insertExport(6);
    tab.clearAfter(7);

}

void PlotMSMultiPlot::attachToCanvases() {
  cout << "PMSMP::attachToCanvases" << endl;
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        for(unsigned int j = 0; j < itsPlots_[i].size(); j++)
            itsCanvases_[i][j]->plotItem(itsPlots_[i][j]);
}

void PlotMSMultiPlot::detachFromCanvases() {
  cout << "PMSMP::detachFromCanvases" << endl;
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        for(unsigned int j = 0; j < itsPlots_[i].size(); j++)
            itsCanvases_[i][j]->removePlotItem(itsPlots_[i][j]);
}

//void PlotMSMultiPlot::plotTabHasChanged(PlotMSPlotTab& tab) {
//	(void)tab;
	
    /* (gmoellen, 2010Aug12) the following is no longer necessary
       since we will only support one pair of X/Y axes for now 
    
    PlotMSPlotParameters newParams = tab.currentlySetParameters();    
    const PMS_PP_Cache* c = itsParams_.typedGroup<PMS_PP_Cache>();
    if(c == NULL) return; // shouldn't happen
    
    unsigned int nRows = itsPlots_.size(), nCols = 0;
    if(nRows > 0) nCols = itsPlots_[0].size();
    
    // Update subtabs for the new rows and cols if they've changed.
    if(c->numYAxes() != nRows || c->numXAxes() != nCols)
        updateSubtabs(c->numYAxes(), c->numXAxes());

    */

//}


// Protected Methods //

bool PlotMSMultiPlot::assignCanvases(PlotMSPages& pages) {
  cout << "PMSMP::assignCanvases" << endl;
    // Use the current page.
    if(pages.totalPages() == 0) pages.insertPage();
    PlotMSPage page = pages.currentPage();
    
    // Add the rows/columns needed.
    unsigned int row = page.canvasRows(), col = page.canvasCols(),
                 rows = PMS_PP_RETCALL(itsParams_, PMS_PP_Iteration, numRows, 1),
                 cols = PMS_PP_RETCALL(itsParams_, PMS_PP_Iteration, numColumns, 1);

    //    cout << "   row,rows,col,cols = " << row << "," << rows << "," << col << "," << cols << endl;

    page.resize(row + rows, col + cols);
    
    // Resize the canvases vector.
    itsCanvases_.resize(rows);
    for(unsigned int i = 0; i < itsCanvases_.size(); i++)
        itsCanvases_[i].resize(cols);
    
    // Set the owner, and update canvases.
    for(unsigned int r = row; r < row + rows; r++) {
        for(unsigned int c = col; c < col + cols; c++) {
            page.setOwner(r, c, this);
            itsCanvases_[r - row][c - col] = page.canvas(r, c);
        }
    }
    
    // Replace the page.
    pages.itsPages_[pages.itsCurrentPageNum_] = page;

    return true;
}

bool PlotMSMultiPlot::initializePlot() {    
  cout << "PMSMP::initializePlot" << endl;
    // Resize plots vector.
    unsigned int rows = PMS_PP_RETCALL(itsParams_, PMS_PP_Iteration, numColumns, 1),
                 cols = PMS_PP_RETCALL(itsParams_, PMS_PP_Iteration, numColumns, 1);

    //    cout << "rows, cols = " << rows << "," << cols << endl;

    itsPlots_.resize(rows);
    itsIndexers_.resize(rows);
    for(unsigned int i = 0; i < itsPlots_.size(); i++) {
        itsPlots_[i].resize(cols);
        itsIndexers_[i].resize(cols);
    }
    
    // Assign plots.
    PlotMaskedPointDataPtr data;
    MaskedScatterPlotPtr plot;
    for(unsigned int i = 0; i < itsPlots_.size(); i++) {
        for(unsigned int j = 0; j < itsPlots_[i].size(); j++) {
            itsIndexers_[i][j] = new PlotMSData(itsData_);
            data = PlotMaskedPointDataPtr(itsIndexers_[i][j], false);
            
            plot = itsFactory_->maskedPlot(data);
            itsPlots_[i][j] = plot;

	    // Set colors using list.
	    ColoredPlotPtr itsColoredPlot_ = 
	      ColoredPlotPtr(dynamic_cast<ColoredPlot*>(&*itsPlots_[i][j]),false);
	    if(!itsColoredPlot_.null()) {
	      const vector<String>& colors = PMS::COLORS_LIST();
	      for(unsigned int k = 0; k < colors.size(); k++) {
		itsColoredPlot_->setColorForBin(k, itsFactory_->color(colors[k]));
	      }
	    }

        }
    }

    return true;
}

bool PlotMSMultiPlot::parametersHaveChanged_(const PlotMSWatchedParameters& p,
					     int updateFlag, bool releaseWhenDone) {
  cout << "PMSMP::parametersHaveChanged_" << endl;

    if(&p != &itsParams_) return true; // shouldn't happen
    
    const PMS_PP_MSData* d = itsParams_.typedGroup<PMS_PP_MSData>();
    const PMS_PP_Cache* c = itsParams_.typedGroup<PMS_PP_Cache>();
    const PMS_PP_Axes* a = itsParams_.typedGroup<PMS_PP_Axes>();
    const PMS_PP_Iteration* iter = itsParams_.typedGroup<PMS_PP_Iteration>();
    if(d == NULL || c == NULL || a == NULL || iter == NULL) // shouldn't happen
      return true; 

    // The current size
    Int nRows = itsPlots_.size(), nCols = 0;
    if(nRows > 0) nCols = itsPlots_[0].size();
    
    // Update canvases and plots if number of axes have changed.
    if(iter->numRows() != nRows || iter->numColumns() != nCols)
      //    if(c->numXAxes() != nRows || c->numYAxes() != nCols)
        updateCanvasesAndPlotsForAxes();
    
    // Update TCL params.
    itsTCLParams_.releaseWhenDone = releaseWhenDone;
    itsTCLParams_.updateCanvas = (updateFlag & PMS_PP::UPDATE_MSDATA) ||
            (updateFlag & PMS_PP::UPDATE_CACHE) ||
            (updateFlag & PMS_PP::UPDATE_CANVAS) || !d->isSet();
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

PlotMSRegions PlotMSMultiPlot::selectedRegions(
        const vector<PlotCanvasPtr>& canvases) const {

  cout << "PMSMP::selectedRegions" << endl;

    PlotMSRegions r;
    vector<PMS::Axis> x = PMS_PP_RETCALL(itsParams_, PMS_PP_Cache, xAxes, vector<PMS::Axis>()),
                      y = PMS_PP_RETCALL(itsParams_, PMS_PP_Cache, yAxes, vector<PMS::Axis>());
    
    unsigned int index = 0;
    for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
        for(unsigned int j = 0; j < itsCanvases_.size(); j++) {
            for(unsigned int k = 0; k < canvases.size(); k++) {
                if(canvases[k] == itsCanvases_[i][j]) {
                    r.addRegions(x[index], y[index], canvases[k]);
                }
            }
            index ++;
        }
    }
    
    return r;
}

void PlotMSMultiPlot::constructorSetup() {
  //  cout << "PMSMP::constructorSetup" << endl;
    PlotMSPlot::constructorSetup();
    makeParameters(itsParams_, itsParent_);
}


// Private Methods //

void PlotMSMultiPlot::updateSubtabs(unsigned int nRows, unsigned int nCols) const {    

    PlotMSDisplayTab* dt = itsParent_->getPlotter()->getPlotTab()
                           ->subtab<PlotMSDisplayTab>();
    if(dt != NULL) dt->setIndexRowsCols(nRows, nCols);
}

void PlotMSMultiPlot::updateCanvasesAndPlotsForAxes() {
  cout << "PMSMP::updateCanvasesAndPlotsForAxes" << endl;
    PlotMSPages& pages = itsParent_->getPlotManager().itsPages_;
    
    // Find the page that this plot is on, and the starting row/col of the
    // first canvas.
    PlotMSPage* page = NULL;
    bool found = false;
    unsigned int row = 0, col = 0;
    for(unsigned int i = 0; !found && i < pages.totalPages(); i++) {
        page = &pages.itsPages_[i];
        for(unsigned int r = 0; !found && r < page->canvasRows(); r++) {
            for(unsigned int c = 0; !found && c < page->canvasCols(); c++) {
                if(page->owner(r, c) == this) {
                    row = r;
                    col = c;
                    found = true;
                }
            }
        }
    }
    if(!found) return; // shouldn't happen

    //    cout << "row,col = " << row << "," << col << endl;


    // The current number of rows and columns
    unsigned int oldRows = itsPlots_.size(), oldCols = 0;
    if(oldRows > 0) oldCols = itsPlots_[0].size();

    cout << "oldRows,oldCols = " << oldRows << "," << oldCols << endl;

    unsigned int newRows = PMS_PP_RETCALL(itsParams_,PMS_PP_Iteration,numRows,oldRows);
    unsigned int newCols = PMS_PP_RETCALL(itsParams_,PMS_PP_Iteration,numColumns,oldRows);

    cout << "newRows,newCols = " << newRows << "," << newCols << endl;
    
    // TODO
    // if there are any other canvases after this plot, need to preserve them
    
    itsPlots_.resize(newRows);
    itsCanvases_.resize(newRows);
    itsIndexers_.resize(newRows);
    
    for(unsigned int i = 0; i < itsPlots_.size(); i++) {
        itsPlots_[i].resize(newCols);
        itsCanvases_[i].resize(newCols);
        itsIndexers_[i].resize(newCols);
    }
    
    page->resize(row + newRows, col + newCols);

    // Generate new plots and assign canvases.
    PlotMaskedPointDataPtr data;
    MaskedScatterPlotPtr plot;
    for(unsigned int r = 0; r < itsPlots_.size(); r++) {
        for(unsigned int c = 0; c < itsPlots_[r].size(); c++) {
            if(!itsPlots_[r][c].null()) continue;
            
            // Generate plot.
            itsIndexers_[r][c] = new PlotMSData(itsData_);
            data = PlotMaskedPointDataPtr(itsIndexers_[r][c], false);
            plot = itsFactory_->maskedPlot(data);
            itsPlots_[r][c] = plot;
                    
            // Set colors using list.
	    ColoredPlotPtr itsColoredPlot_ = 
	      ColoredPlotPtr(dynamic_cast<ColoredPlot*>(&*itsPlots_[r][c]),false);
            if(!itsColoredPlot_.null()) {
	      const vector<String>& colors = PMS::COLORS_LIST();
	      for(unsigned int i = 0; i < colors.size(); i++)
		itsColoredPlot_->setColorForBin(i, itsFactory_->color(colors[i]));
            }
            
            // Assign canvases.
            page->setOwner(r, c, this);
            itsCanvases_[r][c] = page->canvas(r, c);
            itsCanvases_[r][c]->plotItem(itsPlots_[r][c]);
        }
    }
    
    if(page == &pages.itsPages_[pages.itsCurrentPageNum_])
        pages.setupCurrentPage();
}

bool PlotMSMultiPlot::updateCache() {

  cout << "PMSMP::updateCache" << endl;

    PMS_PP_MSData* d = itsParams_.typedGroup<PMS_PP_MSData>();
    PMS_PP_Cache* c = itsParams_.typedGroup<PMS_PP_Cache>();
    if(d == NULL || c == NULL) return false; // shouldn't happen
    
    // Don't load if data isn't set or there was an error during data opening.
    if(!d->isSet()) return false;
    
    // Let the plot know that the data (will) change.
    // TODO
    for(unsigned int i = 0; i < itsPlots_.size(); i++)
        for(unsigned int j = 0; j < itsPlots_[i].size(); j++)
            itsPlots_[i][j]->dataChanged();
    
    // Set up cache loading parameters.
    // TBD: simplify the following
    
    /*vector<PMS::Axis> axes(c->numXAxes() + c->numYAxes());
    for(unsigned int i = 0; i < c->numXAxes(); i++)
        axes[i] = c->xAxis(i);
    for(unsigned int i = c->numXAxes(); i < axes.size(); i++)
        axes[i] = c->yAxis(i - c->numXAxes());
    vector<PMS::DataColumn> data(axes.size());
    for(unsigned int i = 0; i < c->numXAxes(); i++)
        data[i] = c->xDataColumn(i);
    for(unsigned int i = c->numXAxes(); i < axes.size(); i++)
        data[i] = c->yDataColumn(i - c->numXAxes());
    */
    // Log the cache loading.
    itsParent_->getLogger()->markMeasurement(PMS::LOG_ORIGIN,
            PMS::LOG_ORIGIN_LOAD_CACHE, PMS::LOG_EVENT_LOAD_CACHE);
    itsTCLParams_.endCacheLog = true;
    
    /*PlotMSCacheThread* ct = new PlotMSCacheThread(this, &itsData_,
						  axes, data,
						  d->filename(), 
						  d->selection(), 
						  d->averaging(), 
						  d->transformations(),
						  true, 
						  &PlotMSMultiPlot::cacheLoaded, this);

    itsParent_->getPlotter()->doThreadedOperation(ct);
    
    return true;*/
    bool result = itsParent_->updateCachePlot( this, PlotMSMultiPlot::cacheLoaded, true );
    return result;
}

bool PlotMSMultiPlot::updateCanvas() {

  cout << "PMSMP::updateCanvas" << endl;

    try {

        bool set = PMS_PP_RETCALL(itsParams_, PMS_PP_MSData, isSet, false);
        
        PMS_PP_Axes* a = itsParams_.typedGroup<PMS_PP_Axes>();
        PMS_PP_Cache* d = itsParams_.typedGroup<PMS_PP_Cache>();
        PMS_PP_Canvas* c = itsParams_.typedGroup<PMS_PP_Canvas>();
        if(a== NULL || d== NULL || c== NULL) return false; // shouldn't happen
        
        PMS::Axis x, y;
        PlotAxis cx, cy;
        PlotCanvasPtr canv;
        bool xref, yref;
        double xrefval, yrefval;
        
        for(unsigned int i = 0; i < itsCanvases_.size(); i++) {
            for(unsigned int j = 0; j < itsCanvases_[i].size(); j++) {
 	        x = d->xAxis(); y = d->yAxis();
                cx = a->xAxis(); cy = a->yAxis();
                canv = itsCanvases_[i][j];
                                    
                // Set axes scales
                canv->setAxisScale(cx, PMS::axisScale(x));
                canv->setAxisScale(cy, PMS::axisScale(y));
                
                // Set reference values
                xref = itsData_.hasReferenceValue(x);
                yref = itsData_.hasReferenceValue(y);
                xrefval = itsData_.referenceValue(x);
                yrefval = itsData_.referenceValue(y);
                canv->setAxisReferenceValue(cx, xref, xrefval);
                canv->setAxisReferenceValue(cy, yref, yrefval);
                
                // Set axes labels
                canv->clearAxesLabels();
                if(set) {
		  canv->setAxisLabel(cx, c->xLabelFormat().getLabel(x, xref, xrefval));
		  canv->setAxisLabel(cy, c->yLabelFormat().getLabel(y, yref, yrefval));
                }
                
                // Custom ranges
		// TBD: gmoellen: where are (per-plot) auto-detected ranges setup?
                canv->setAxesAutoRescale(true);
                if(set) {
		  if(a->xRangeSet()) canv->setAxisRange(cx, a->xRange());
		  if(a->yRangeSet()) canv->setAxisRange(cy, a->yRange());
                }

                // Show/hide axes
		// TBD: control this from the gui info!!
                canv->showAllAxes(false);
                canv->showAxis(cx, set && c->xAxisShown());
                canv->showAxis(cy, set && c->yAxisShown());
                
                // Legend
		// TBD: this should be used to label each iteration
                canv->showLegend(set && c->legendShown(),c->legendPosition());
                
                // Canvas title
		canv->setTitle(set ? c->titleFormat().getLabel(x, y, xref,
							       xrefval, yref, yrefval) : "");
                
                // Grids
                canv->showGrid(c->gridMajorShown(),
			       c->gridMinorShown(), c->gridMajorShown(),
			       c->gridMinorShown());
                
                PlotLinePtr line = itsFactory_->line(c->gridMajorLine());
                if(!c->gridMajorShown()) line->setStyle(PlotLine::NOLINE);
                canv->setGridMajorLine(line);
                
                line = itsFactory_->line(c->gridMinorLine());
                if(!c->gridMinorShown()) line->setStyle(PlotLine::NOLINE);
                canv->setGridMinorLine(line);
                
            }
        }
        
        return true;
        
    } catch(AipsError& err) {
        itsParent_->showError("Could not update canvas: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not update canvas, for unknown reasons!");
        return false;
    }
}

bool PlotMSMultiPlot::updateDisplay() {
  cout << "PMSMP::updateDisplay" << endl;
    try {
        PMS_PP_Cache* h = itsParams_.typedGroup<PMS_PP_Cache>();
        PMS_PP_Axes* a = itsParams_.typedGroup<PMS_PP_Axes>();
        PMS_PP_Display* d = itsParams_.typedGroup<PMS_PP_Display>();
        
        // shouldn't happen
        if(h == NULL || a == NULL || d == NULL) return false;
        
        MaskedScatterPlotPtr plot;
        PMS::Axis x, y;
        for(unsigned int i = 0; i < itsPlots_.size(); i++) {
            for(unsigned int j = 0; j < itsPlots_[i].size(); j++) {
                plot = itsPlots_[i][j];
                
                // Set symbols
                plot->setSymbol(d->unflaggedSymbol());
                plot->setMaskedSymbol(d->flaggedSymbol());
        
                // Colorize, and set data changed if redraw is needed.
                if(itsData_.colorize(d->colorizeFlag(), d->colorizeAxis()))
                    plot->dataChanged();
                
                // Set item axes.
                plot->setAxes(a->xAxis(), a->yAxis());
                
                // Set plot title.
                x = h->xAxis(); y = h->yAxis();
                plot->setTitle(d->titleFormat().getLabel(x,y,
                        itsData_.hasReferenceValue(x),
                        itsData_.referenceValue(x),
                        itsData_.hasReferenceValue(y),
                        itsData_.referenceValue(y)));
                
            }
        }
        
        return true;
    } catch(AipsError& err) {
        itsParent_->showError("Could not update plot: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not update plot, for unknown reasons!");
        return false;
    }
}

void PlotMSMultiPlot::cacheLoaded_(bool wasCanceled) {

  cout << "PMSMP::cacheLoaded_" << endl;

    // TODO
    // Let the plot know that the data has been changed as needed.
    if(wasCanceled)
        for(unsigned int i = 0; i < itsPlots_.size(); i++)
            for(unsigned int j = 0; j < itsPlots_[i].size(); j++)
                itsPlots_[i][j]->dataChanged();

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
    /*
    if(!wasCanceled && itsTCLParams_.endCacheLog) {
        stringstream ss;
        ss << "Plotted " << itsData_.size() << " points ("
           << itsData_.sizeUnmasked() << " unflagged, "<< itsData_.sizeMasked()
           << " flagged).";
        itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
                PMS::LOG_ORIGIN_PLOT, ss.str(), PMS::LOG_EVENT_PLOT);
    }
    */
}

// A temporary override of releaseDrawing to avoid many threads competing for
// a single cache.  the sliced cache will fix this
void PlotMSMultiPlot::releaseDrawing() {

  // Draw just the "middle" [nrow/2,ncol/2] plot:
  unsigned int row = itsCanvases_.size(), col = 0;
  if(row > 0) col = itsCanvases_[0].size();
  row/=2;
  col/=2;
  if (!itsCanvases_[row][col].null())itsCanvases_[row][col]->releaseDrawing();
  
}


}
