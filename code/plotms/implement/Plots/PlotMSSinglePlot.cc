//# PlotMSSinglePlot.cc: Subclass of PlotMSPlot for a single plot/canvas.
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
#include <plotms/Plots/PlotMSSinglePlot.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

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
