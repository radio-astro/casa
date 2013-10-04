//# PlotMSOverPlot.cc: Subclass of PlotMSPlot for a single plot/canvas.
//# Copyright (C) 2012
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
#include <plotms/Plots/PlotMSOverPlot.h>

//#include <plotms/Gui/PlotMSPlotter.qo.h>
//#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Plots/PlotInformationManager.h>
#include <plotms/Data/PlotMSCacheBase.h>
#include <plotms/Data/MSCache.h>
#include <plotms/Data/CalCache.h>
//#include <casaqt/QwtPlotter/QPCanvas.qo.h>

#include <algorithm>
#include <cmath>

#define THREADLOAD True

namespace casa {

////////////////////////////////
// PlotMSOverPlot Definitions //
////////////////////////////////

// Static

PlotMSPlotParameters PlotMSOverPlot::makeParameters(PlotMSApp *plotms) {
    PlotMSPlotParameters p = PlotMSPlot::makeParameters(plotms);
    makeParameters(p, plotms);
    return p;
}

void PlotMSOverPlot::makeParameters(PlotMSPlotParameters &params,
                                    PlotMSApp *plotms) {
    PlotMSPlot::makeParameters(params, plotms);

    // Add cache parameters if needed
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

// Constructors/Destructors

PlotMSOverPlot::PlotMSOverPlot(PlotMSApp *parent)
    :
    PlotMSPlot(parent),
    iter_(0),
    iterStep_(1) {
    constructorSetup();
}

PlotMSOverPlot::~PlotMSOverPlot() {
}

String PlotMSOverPlot::name() const {
    const PMS_PP_MSData *data = itsParams_.typedGroup<PMS_PP_MSData>();
    const PMS_PP_Cache *cache = itsParams_.typedGroup<PMS_PP_Cache>();
    const PMS_PP_Display *display = itsParams_.typedGroup<PMS_PP_Display>();

    if(data == NULL || cache == NULL || display == NULL || !data->isSet())
        return "Over Plot";
    return display->titleFormat().getLabel(cache->xAxis(), cache->yAxis());
    //return "Over Plot for " + data->filename();
}

vector<MaskedScatterPlotPtr> PlotMSOverPlot::plots() const {
    if((itsPlots_.size() == 0) || (itsPlots_[0].size() == 0))
        return vector<MaskedScatterPlotPtr>();
    uInt index = 0;
    uInt nIter = itsCache_->nIter();
    vector<MaskedScatterPlotPtr> v(nIter);
    for(unsigned int i = 0; i < itsPlots_.size(); i++) {
        for(unsigned int j = 0; j < itsPlots_[i].size(); j++) {
            if(index >= nIter) break;
            v[index] = itsPlots_[i][j];
            ++index;
        }
    }
    return v;
}

vector<PlotCanvasPtr> PlotMSOverPlot::canvases() const {
    if((itsCanvases_.size() == 0) || (itsCanvases_[0].size() == 0))
        return vector<PlotCanvasPtr>();
    uInt index = 0;
    uInt nIter = itsCache_->nIter();
    vector<PlotCanvasPtr> v(
        std::min(nIter, uInt(itsCanvases_.size() * itsCanvases_[0].size())));
    for(uInt i = 0; i < itsCanvases_.size(); i++) {
        for(uInt j = 0; j < itsCanvases_[i].size(); j++) {
            if(index >= nIter) break;
            v[index] = itsCanvases_[i][j];
            ++index;
        }
    }
    return v;
}

void PlotMSOverPlot::setupPlotSubtabs(PlotInformationManager& tab) const {
//void PlotMSOverPlot::setupPlotSubtabs(PlotMSPlotTab &tab) const {
    tab.insertData(0);
    tab.insertAxes(1);
    tab.insertIterate(2);
    tab.insertTransformations(3);
    tab.insertDisplay(4);
    tab.insertCanvas(5);
    tab.insertExport(6);
    tab.clearAfter(7);
}

void PlotMSOverPlot::attachToCanvases() {
    Int iter = iter_;
    Int nIter = itsCache_->nIter();
    for(uInt r = 0; (r < itsCanvases_.size()); ++r) {
        for(uInt c = 0; (c < itsCanvases_[r].size()); ++c) {
            if(!itsCanvases_[r][c].null()) {
                if(!itsPlots_[r][c].null() && (iter < nIter)) {
                    itsCanvases_[r][c]->plotItem(itsPlots_[r][c]);
                    ++iter;
                }
                (/*(QPCanvas*)*/(&*itsCanvases_[r][c]))->show();
                (/*(QPCanvas*)*/(&*itsCanvases_[r][c]))->setMinimumSize(5,5);
            }
        }
    }
}

void PlotMSOverPlot::detachFromCanvases() {
    for(uInt r = 0; r < itsCanvases_.size(); ++r) {
        for(uInt c = 0; c < itsCanvases_[r].size(); ++c) {
            if(!itsCanvases_[r][c].null()) {
                if(itsCanvases_[r][c]->numPlotItems() > 0) {
                    itsCanvases_[r][c]->removePlotItem(itsPlots_[r][c]);
                }
                (/*(QPCanvas*)*/(&*itsCanvases_[r][c]))->hide();
            }
        }
    }
}

// Protected members

void PlotMSOverPlot::resize(PlotMSPages &pages, uInt rows, uInt cols) {
    // Resize canvases and plots
    itsCanvases_.resize(rows);
    itsPlots_.resize(rows);
    for(uInt r = 0; r < rows; ++r) {
        itsCanvases_[r].resize(cols);
        itsPlots_[r].resize(cols);
    }
    iterStep_ = rows * cols;
    // Resize pages
    double nIter = itsCache_->nIter();
    if(nIter == 0) nIter = 1;
    pages.resize(int(ceil(nIter / iterStep_)));
    for(size_t i = 0; i < pages.totalPages(); ++i) {
        pages[i].resize(rows, cols);
    }
    //pages[pages.currentPageNumber()].resize(rows, cols);
}

bool PlotMSOverPlot::assignCanvases(PlotMSPages &pages) {
    if(pages.totalPages() == 0) {
        pages.insertPage();
        pages.firstPage();
    }

    uInt rows = PMS_PP_RETCALL(itsParams_, PMS_PP_Iteration, numRows, 1);
    uInt cols = PMS_PP_RETCALL(itsParams_, PMS_PP_Iteration, numColumns, 1);
    //if(rows != itsCanvases_.size() || cols != itsCanvases_[0].size()) {
        resize(pages, rows, cols);
    //}
    PlotMSPage& page = pages[pages.currentPageNumber()];
    for(uInt r = 0; r < rows; ++r) {
        for(uInt c = 0; c < cols; ++c) {
            if(!page.isOwned(r, c)) {
                page.setOwner(r, c, this);
            }
            itsCanvases_[r][c] = page.canvas(r, c);
        }
    }
    page.setupPage();
    return true;
}

bool PlotMSOverPlot::initializePlot() {
    uInt rows = PMS_PP_RETCALL(itsParams_, PMS_PP_Iteration, numRows, 1);
    uInt cols = PMS_PP_RETCALL(itsParams_, PMS_PP_Iteration, numColumns, 1);

    uInt iter = 0;
    uInt nIter = itsCache_->nIter();
    for(uInt r = 0; r < rows; ++r) {
        for(uInt c = 0; c < cols; ++c) {
            PlotMaskedPointDataPtr data(&(itsCache_->indexer0()), false);
            itsPlots_[r][c] = itsFactory_->maskedPlot(data);
            ++iter;

            // We want to execute this loop at least once to fill in
            // a single plot scenario; but after that, if there are
            // no iterations, break out
            if(iter >= nIter) break;
        }
        if(iter >= nIter) break;
    }

    setColors();
    return true;
}

bool PlotMSOverPlot::parametersHaveChanged_(const PlotMSWatchedParameters &p,
                                            int updateFlag,
                                            bool releaseWhenDone) {
    if(&p != &itsParams_) return false;

    const PMS_PP_MSData *data = itsParams_.typedGroup<PMS_PP_MSData>();
    const PMS_PP_Iteration *iter = itsParams_.typedGroup<PMS_PP_Iteration>();
    if(data == NULL || iter == NULL)
        return true;
    itsTCLParams_.releaseWhenDone = releaseWhenDone;
    itsTCLParams_.updateCanvas = (updateFlag & PMS_PP::UPDATE_AXES) ||
        (updateFlag & PMS_PP::UPDATE_CACHE) ||
        (updateFlag & PMS_PP::UPDATE_CANVAS) ||
        (updateFlag & PMS_PP::UPDATE_ITERATION) ||
        (updateFlag & PMS_PP::UPDATE_MSDATA) || !data->isSet();
    itsTCLParams_.updateDisplay = updateFlag & PMS_PP::UPDATE_DISPLAY;
    itsTCLParams_.endCacheLog = false;

    // Clear selection if axes change
    //if(updateFlag & PMS_PP::UPDATE_AXES) {
    // Apparently UPDATE_AXES is not triggered by anything...
    // UPDATE_CACHE should be close enough for now (I hope)
    if(updateFlag & PMS_PP::UPDATE_CACHE) {
        for(size_t r = 0; r < itsCanvases_.size(); ++r) {
            for(size_t c = 0; c < itsCanvases_[r].size(); ++c) {
                itsCanvases_[r][c]->standardMouseTools(
                    )->selectTool()->clearSelectedRects();
                itsCanvases_[r][c]->clearAnnotations();
            }
        }
    }

    // Resize if iteration parameters have changed
    uInt rows = iter->numRows();
    uInt cols = iter->numColumns();
    itsTCLParams_.updateIteration = ((updateFlag & PMS_PP::UPDATE_ITERATION) &&
                                     ((itsPlots_.size() != rows) ||
                                      (itsPlots_[0].size() != cols)));

    // Update cache if needed
    if(data->isSet() && (updateFlag & PMS_PP::UPDATE_MSDATA ||
                         updateFlag & PMS_PP::UPDATE_CACHE)) {
        return !updateCache();
    }

    //itsTCLParams_.releaseWhenDone = false;
    cacheLoaded_(false);
    return true;
}

PlotMSRegions PlotMSOverPlot::selectedRegions(
    const vector<PlotCanvasPtr>& canvases) const {
    PlotMSRegions r;
    PMS::Axis x = (PMS::Axis)PMS_PP_RETCALL(itsParams_, PMS_PP_Cache,
                                            xAxis, 0);
    PMS::Axis y = (PMS::Axis)PMS_PP_RETCALL(itsParams_, PMS_PP_Cache,
                                            yAxis, 0);

    for(uInt i = 0; i < canvases.size(); ++i) {
        r.addRegions(x, y, canvases[i]);
    }
    return r;
}

void PlotMSOverPlot::constructorSetup() {
    PlotMSPlot::constructorSetup();
    makeParameters(itsParams_, itsParent_);
}

// Private Methods //

bool PlotMSOverPlot::updateCache() {
    PMS_PP_MSData* data = itsParams_.typedGroup<PMS_PP_MSData>();
    PMS_PP_Cache* cache = itsParams_.typedGroup<PMS_PP_Cache>();
    PMS_PP_Iteration* iter = itsParams_.typedGroup<PMS_PP_Iteration>();
    if(data == NULL || cache == NULL || iter == NULL)
        return false;

    // Don't load if data isn't set or there was an error during data opening.
    if(!data->isSet()) return false;

    // Trap bad averaging/iteration combo
    if (data->averaging().baseline() &&
	iter->iterationAxis()==PMS::ANTENNA) {
        stringstream ss;
        ss << "Cannot iterate on Antenna if averaging over baseline, "
           << "so turning off iteration.";
        itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
                                             PMS::LOG_ORIGIN_PLOT, 
                                             ss.str(), 
                                             PMS::LOG_EVENT_PLOT);
        iter->setIterationAxis(PMS::NONE);
    }

    // Notify the plots that the data will change
    updatePlots();

    // Set up cache loading parameters
    if(cache->numXAxes() != cache->numYAxes()) return false;
    vector<PMS::Axis> caxes(cache->numXAxes() + cache->numYAxes());
    vector<PMS::DataColumn> cdata(cache->numXAxes() + cache->numYAxes());
    for(uInt i = 0; i < cache->numXAxes(); ++i) {
        caxes[i] = cache->xAxis(i);
        cdata[i] = cache->xDataColumn(i);
    }
    for(uInt i = cache->numYAxes(); i < caxes.size(); ++i) {
        caxes[i] = cache->yAxis(i - cache->numXAxes());
        cdata[i] = cache->yDataColumn(i - cache->numXAxes());
    }

    itsParent_->getLogger()->markMeasurement(PMS::LOG_ORIGIN,
                                             PMS::LOG_ORIGIN_LOAD_CACHE,
                                             PMS::LOG_EVENT_LOAD_CACHE);
    itsTCLParams_.endCacheLog = true;

    {
        if (Table::isReadable(data->filename())) {
            Table tab(data->filename());

            // Delete existing cache if it doesn't match
            if (itsCache_ &&
                (itsCache_->cacheType()==PlotMSCacheBase::CAL &&
                 tab.tableInfo().type()!="Calibration") ||
                (itsCache_->cacheType()==PlotMSCacheBase::MS &&
                 tab.tableInfo().type()=="Calibration")) {
                delete itsCache_;
                itsCache_=NULL;
            }

            // Construct proper empty cache if necessary
            if (!itsCache_) {
                if (tab.tableInfo().type()=="Calibration")
                    itsCache_ = new CalCache(itsParent_);
                else
                    itsCache_ = new MSCache(itsParent_);
            }
        }
    }

    /*if(THREADLOAD) {
        PlotMSCacheThread *ct = new PlotMSCacheThread(
            this, itsCache_, caxes, cdata, data->filename(), data->selection(),
            data->averaging(), data->transformations(), true,
            &PlotMSOverPlot::cacheLoaded, this);
        //itsParent_->getPlotter()->doThreadedOperation(ct);
    } else {*/
        //itsCache_->load(caxes, cdata, data->filename(), data->selection(),
        //                data->averaging(), data->transformations());
        //this->cacheLoaded_(false);
    //}
    bool result = itsParent_->updateCachePlot( this,
    		PlotMSOverPlot::cacheLoaded, true );
    return result;
}

bool PlotMSOverPlot::updateCanvas() {
//    try {
        bool set = PMS_PP_RETCALL(itsParams_, PMS_PP_MSData, isSet, false);

        PMS_PP_Axes *axes = itsParams_.typedGroup<PMS_PP_Axes>();
        PMS_PP_Cache *cache = itsParams_.typedGroup<PMS_PP_Cache>();
        PMS_PP_Canvas *canv = itsParams_.typedGroup<PMS_PP_Canvas>();
        PMS_PP_Iteration *iter = itsParams_.typedGroup<PMS_PP_Iteration>();
        if(axes == NULL || cache == NULL || canv == NULL || iter == NULL) {
            return false;
        }

        PMS::Axis x = cache->xAxis();
        PMS::Axis y = cache->yAxis();
        PlotAxis cx = axes->xAxis();
        PlotAxis cy = axes->yAxis();

        uInt nIter = itsCache_->nIter();
        uInt rows = itsCanvases_.size();
        for(uInt r = 0; r < rows; ++r) {
            uInt cols = itsCanvases_[r].size();
            uInt iterationRows = iter_ + r * cols;
            if(iterationRows >= nIter) break;
            for(uInt c = 0; c < cols; ++c) {
                uInt iteration = iterationRows + c;
                if(iteration >= nIter) break;

                PlotCanvasPtr canvas = itsCanvases_[r][c];
                if(canvas.null()) continue;

                // Set axes scales
                canvas->setAxisScale(cx, PMS::axisScale(x));
                canvas->setAxisScale(cy, PMS::axisScale(y));

                // Set reference values
                bool xref = itsCache_->hasReferenceValue(x);
                bool yref = itsCache_->hasReferenceValue(y);
                double xrefval = itsCache_->referenceValue(x);
                double yrefval = itsCache_->referenceValue(y);
                canvas->setAxisReferenceValue(cx, xref, xrefval);
                canvas->setAxisReferenceValue(cy, yref, yrefval);

                // Set axes labels
                canvas->clearAxesLabels();
                if(set) {
                    canvas->setAxisLabel(
                        cx, canv->xLabelFormat().getLabel(x, xref, xrefval));
                    canvas->setAxisLabel(
                        cy, canv->yLabelFormat().getLabel(y, yref, yrefval));
                    PlotFontPtr xFont = canvas->axisFont(cx);
                    PlotFontPtr yFont = canvas->axisFont(cy);
                    xFont->setPointSize(std::max(12. - rows*cols+1., 8.));
                    yFont->setPointSize(std::max(12. - rows*cols+1., 8.));
                    canvas->setAxisFont(cx, xFont);
                    canvas->setAxisFont(cy, yFont);
                }

                // Custom axes ranges
                canvas->setAxesAutoRescale(true);
                if(set && axes->xRangeSet() && axes->yRangeSet()) {
                    canvas->setAxesRanges(cx, axes->xRange(),
                                          cy, axes->yRange());
                } else if(set && axes->xRangeSet()) {
                    canvas->setAxisRange(cx, axes->xRange());
                } else if(set && axes->yRangeSet()) {
                    canvas->setAxisRange(cy, axes->yRange());
                }

                // Show/hide axes
                bool showx = set && canv->xAxisShown();
                bool showy = set && canv->yAxisShown();
                canvas->showAllAxes(false);
                // If using global scale, we only want to draw an X axis
                // for the bottom plots and a Y axis for left plots;
                // otherwise, draw an X and Y axis for every plot
                //if(iter->xAxisScaleMode() == PMS_PP_Iteration::GLOBAL) {
                //    if(r == (rows-1)) canvas->showAxis(cx, showx);
                //} else {
                    canvas->showAxis(cx, showx);
                //}
                //if(iter->yAxisScaleMode() == PMS_PP_Iteration::GLOBAL) {
                //    if(c == 0) canvas->showAxis(cy, showy);
                //} else {
                    canvas->showAxis(cy, showy);
                //}

                // Legend
                canvas->showLegend(set && canv->legendShown(),
                                   canv->legendPosition());

                // Title font
                PlotFontPtr font = canvas->titleFont();
                font->setPointSize(std::max(16. - rows*cols+1., 8.));
                font->setBold(true);
                canvas->setTitleFont(font);
                // Title
                bool resetTitle = set || (iter->iterationAxis() != PMS::NONE);
                String iterTxt;
                if(iter->iterationAxis() != PMS::NONE &&
                   itsCache_->nIter() > 0) {
                    iterTxt = itsCache_->indexer(iteration).iterLabel();
                }
                String title = "";
                if(resetTitle) {
                    title = canv->titleFormat().getLabel(
                        x, y, xref, xrefval, yref, yrefval) + " " + iterTxt;
                }
                canvas->setTitle(title);

                // Grids
                canvas->showGrid(canv->gridMajorShown(),
                                 canv->gridMinorShown(),
                                 canv->gridMajorShown(),
                                 canv->gridMinorShown());

                PlotLinePtr major_line =
                    itsFactory_->line(canv->gridMajorLine());
                if(!canv->gridMajorShown()) {
                    major_line->setStyle(PlotLine::NOLINE);
                }
                canvas->setGridMajorLine(major_line);

                PlotLinePtr minor_line =
                    itsFactory_->line(canv->gridMinorLine());
                if(!canv->gridMinorShown()) {
                    minor_line->setStyle(PlotLine::NOLINE);
                }
                canvas->setGridMinorLine(minor_line);
            }
        }
//     } catch(AipsError &err) {
//         itsParent_->showError("Could not update canvas: " + err.getMesg());
//         return false;
//     } catch(...) {
//         itsParent_->showError("Could not update canvas, for unknown reasons!");
//         return false;
//     }
    //scaleCanvases();
    return true;
}

bool PlotMSOverPlot::updateDisplay() {
    try {
        PMS_PP_Cache *cache = itsParams_.typedGroup<PMS_PP_Cache>();
        PMS_PP_Axes *axes = itsParams_.typedGroup<PMS_PP_Axes>();
        PMS_PP_Display *display = itsParams_.typedGroup<PMS_PP_Display>();
        if(cache == NULL || axes == NULL || display == NULL) return false;
        MaskedScatterPlotPtr plot;
        uInt nIter = itsCache_->nIter();
        uInt rows = itsPlots_.size();

        for(uInt row = 0; row < rows; ++row) {
            uInt cols = itsPlots_[row].size();
            uInt iter = iter_ + row * cols;
            if(iter >= nIter) break;
            for(uInt col = 0; col < itsPlots_[row].size(); ++col) {
                if(iter >= nIter) break;
                // Set symbols.
                PlotSymbolPtr symbolUnmasked = itsParent_->createSymbol ( display->unflaggedSymbol() );
                uInt dataSize = itsCache_->indexer(iter).sizeUnmasked();
                customizeAutoSymbol( symbolUnmasked, dataSize );

                PlotSymbolPtr symbolMasked = itsParent_->createSymbol ( display->flaggedSymbol() );
                dataSize = itsCache_->indexer(iter).sizeMasked();
                customizeAutoSymbol( symbolMasked, dataSize );

                plot = itsPlots_[row][col];
                if(plot.null()) continue;
                plot->setSymbol(symbolUnmasked);
                plot->setMaskedSymbol(symbolMasked);

                // Colorize and set data changed, if redraw is needed
                if(nIter > 0 && itsCache_->indexer(iter).colorize(
                       display->colorizeFlag(), display->colorizeAxis())) {
                    plot->dataChanged();
                }

                // Set item axes
                plot->setAxes(axes->xAxis(), axes->yAxis());

                // Set plot title
                PMS::Axis x = cache->xAxis();
                PMS::Axis y = cache->yAxis();
                plot->setTitle(display->titleFormat().getLabel(
                                   x, y,
                                   itsCache_->hasReferenceValue(x),
                                   itsCache_->referenceValue(x),
                                   itsCache_->hasReferenceValue(y),
                                   itsCache_->referenceValue(y)));

                ++iter;
            }
        }
    } catch(AipsError &err) {
        itsParent_->showError("Could not update plot: " + err.getMesg());
        return false;
    } catch(...) {
        itsParent_->showError("Could not update plot, for unknown reasons!");
        return false;
    }
    return true;
}

void PlotMSOverPlot::setColors() {
    uInt nIter = itsCache_->nIter();
    uInt rows = itsPlots_.size();
    itsColoredPlots_.resize(rows);
    for(uInt row = 0; row < rows; ++row) {
        uInt cols = itsPlots_[row].size();
        itsColoredPlots_[row].resize(cols);
        for(uInt col = 0; col < cols; ++col) {
            uInt iteration = iter_ + row * cols + col;
            if(iteration >= nIter) break;
            itsColoredPlots_[row][col] = ColoredPlotPtr(
                dynamic_cast<ColoredPlot*>(&*itsPlots_[row][col]), false);
                //dynamic_cast<QPScatterPlot*>(&*itsPlots_[row][col]);
            if(!itsColoredPlots_[row][col].null()) {
            //if(itsColoredPlots_[row][col] != NULL) {
                const vector<String> &colors = PMS::COLORS_LIST();
                for(uInt i = 0; i < colors.size(); ++i) {
                    itsColoredPlots_[row][col]->setColorForBin(
                        i ,itsFactory_->color(colors[i]));
                }
            } else {
                std::cout << "Could not convert plot (" << row << ", " << col
                          << ") into a ColoredPlot" << std::endl;
                itsParent_->showError("Could not convert a plot in a ColoredPlot");
            }
        }
    }
}

bool PlotMSOverPlot::firstIter() {
    Int nIter = itsCache_->nIter();
    if((nIter > 0) && (iter_ != 0)) {
        PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
        pages.firstPage();
        iter_ = 0;
        recalculateIteration();
        return true;
    }
    return false;
}

bool PlotMSOverPlot::prevIter() {
    Int nIter = itsCache_->nIter();
    if((nIter > 0) && ((iter_ - iterStep_) >= 0)) {
        PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
        pages.previousPage();
        iter_ -= iterStep_;
        recalculateIteration();
        return true;
    }
    return false;
}

bool PlotMSOverPlot::nextIter() {
    Int nIter = itsCache_->nIter();
    if((nIter > 0) && ((iter_ + iterStep_) < nIter)) {
        PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
        pages.nextPage();
        iter_ += iterStep_;
        recalculateIteration();
        return true;
    }
    return false;
}

bool PlotMSOverPlot::lastIter() {
    Int nIter = itsCache_->nIter();
    if((nIter > 0) && (iter_ < (nIter - iterStep_))) {
        PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
        pages.lastPage();
        iter_ = int(double(nIter-1) / iterStep_) * iterStep_;
        if(iterStep_ == 1) iter_ = nIter - 1;
        recalculateIteration();
        return true;
    }
    return false;
}

bool PlotMSOverPlot::resetIter() {
    Int nIter = itsCache_->nIter();
    if(nIter > 0) {
        PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
        pages.firstPage();
        iter_ = 0;
        recalculateIteration();
        return true;
    }
    return false;
}

void PlotMSOverPlot::recalculateIteration() {
    detachFromCanvases();
    if(itsTCLParams_.updateIteration) {
        PlotMSPages &pages = itsParent_->getPlotManager().itsPages_;
        assignCanvases(pages);
    }
    uInt nIter = itsCache_->nIter();
    uInt rows = itsPlots_.size();
    for(uInt r = 0; r < rows; ++r) {
        uInt cols = itsPlots_[r].size();
        uInt iterationRows = iter_ + r * cols;
        if(iterationRows >= nIter) break;
        for(uInt c = 0; c < cols; ++c) {
            uInt iteration = iterationRows + c;
            if(iteration >= nIter) break;
            logIter(iteration, nIter);
            PlotMaskedPointDataPtr data(&(itsCache_->indexer(iteration)),
                                        false);
            itsPlots_[r][c] = itsFactory_->maskedPlot(data);
        }
    }
    setColors();
    itsTCLParams_.updateDisplay = true;
    attachToCanvases();
    updatePlots();
    updateCanvas();
    updateDisplay();
    releaseDrawing();
    logPoints();
}

void PlotMSOverPlot::updatePlots() {
    for(uInt row = 0; row < itsPlots_.size(); ++row) {
        for(uInt col = 0; col < itsPlots_[row].size(); ++col) {
            if(!itsPlots_[row][col].null()) {
                itsPlots_[row][col]->dataChanged();
            }
        }
    }
}

bool PlotMSOverPlot::updateIndexing() {
    PMS_PP_Iteration *iter = itsParams_.typedGroup<PMS_PP_Iteration>();
    itsCache_->setUpIndexer(iter->iterationAxis(),
                            iter->globalXRange(),
                            iter->globalYRange());
    return true;
}

void PlotMSOverPlot::logPoints() {
    PMS_PP_Display *display = itsParams_.typedGroup<PMS_PP_Display>();
    bool showUnflagged =
        display->unflaggedSymbol()->symbol() != PlotSymbol::NOSYMBOL;
    bool showFlagged =
        display->flaggedSymbol()->symbol() != PlotSymbol::NOSYMBOL;

    stringstream ss;
    ss << "Plotting ";
    if(showUnflagged) {
        ss << itsCache_->indexer(iter_).sizeUnmasked() << " unflagged"
           << (showFlagged ? ", " : "");
    }
    if(showFlagged) {
        ss << itsCache_->indexer(iter_).sizeMasked() << " flagged";
    }
    ss << " points.";

    itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
                                         PMS::LOG_ORIGIN_PLOT,
                                         ss.str(),
                                         PMS::LOG_EVENT_PLOT);
}

void PlotMSOverPlot::logIter(Int iter, Int nIter) {
    if(nIter > 1) {
        stringstream ss;
        ss << "Stepping to iteration = " << iter+1
           << " (of " << nIter << "): "
           << itsCache_->indexer(iter).iterLabel();
        itsParent_->getLogger()->postMessage(PMS::LOG_ORIGIN,
                                             PMS::LOG_ORIGIN_PLOT,
                                             ss.str(),
                                             PMS::LOG_EVENT_PLOT);
    }
}

void PlotMSOverPlot::cacheLoaded_(bool wasCanceled) {
    // Ensure we fail gracefully if cache loading yielded nothing
    // or was cancelled
    if (!itsCache_->cacheReady() || wasCanceled) {
        detachFromCanvases();
        initializePlot();
        releaseDrawing();
        itsCache_->clear();
        return;
    }

    // Make this more specific than canvas-triggered
    if (itsTCLParams_.updateCanvas)
        updateIndexing();

    // Reset the iterator (if data are new)
    resetIter();

    // Let the plot know that the data has been changed as needed, unless the
    // thread was canceled.
    updatePlots();

    // End cache log as needed.
    if(itsTCLParams_.endCacheLog)
        itsParent_->getLogger()->releaseMeasurement();

    // Update canvas as needed.
    if(itsTCLParams_.updateCanvas)
        updateCanvas();

    // Update display as needed.
    if(itsTCLParams_.updateDisplay)
        updateDisplay();

    // Release drawing if needed.
    if(itsTCLParams_.releaseWhenDone)
        releaseDrawing();

    // Log number of points as needed.
    if(itsTCLParams_.endCacheLog)
        logPoints();
}

} //namespace casa
