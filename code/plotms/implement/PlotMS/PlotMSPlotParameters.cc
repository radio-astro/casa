//# PlotMSPlotParameters.cc: Parameter classes for PlotMSPlot classes.
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
#include <plotms/PlotMS/PlotMSPlotParameters.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

//////////////////////////////////////
// PLOTMSPLOTPARAMETERS DEFINITIONS //
//////////////////////////////////////

PlotMSPlotParameters::PlotMSPlotParameters(const String& filename) :
        itsMSFilename_(filename), updateFlag_(true) { }

PlotMSPlotParameters::PlotMSPlotParameters(const PlotMSPlotParameters& copy) :
        PlotMSWatchedParameters(copy), updateFlag_(true) {
    operator=(copy);
}

PlotMSPlotParameters::~PlotMSPlotParameters() { }


bool PlotMSPlotParameters::equals(const PlotMSWatchedParameters& other,
        int updateFlags) const {
    const PlotMSPlotParameters* o = dynamic_cast<const PlotMSPlotParameters*>(
                                    &other);
    if(o == NULL) return false;
    
    if(updateFlags & MS) {
        if(itsMSFilename_ != o->itsMSFilename_ ||
           (!itsMSFilename_.empty() && itsMSSelection_ != o->itsMSSelection_)||
           (!itsMSFilename_.empty() && itsMSAveraging_ != o->itsMSAveraging_))
            return false;
    }
    
    return true;
}

bool PlotMSPlotParameters::isSet() const { return !itsMSFilename_.empty(); }

const String& PlotMSPlotParameters::filename() const { return itsMSFilename_; }
void PlotMSPlotParameters::setFilename(const String& filename) {
    if(filename != itsMSFilename_) {
        itsMSFilename_ = filename;
        if(updateFlag_) updateFlag(MS);
    }
}

const PlotMSSelection& PlotMSPlotParameters::selection() const {
    return itsMSSelection_; }
void PlotMSPlotParameters::setSelection(const PlotMSSelection& sel) {
    if(sel != itsMSSelection_) {
        itsMSSelection_ = sel;
        if(updateFlag_) updateFlag(MS);
    }
}

const PlotMSAveraging& PlotMSPlotParameters::averaging() const {
    return itsMSAveraging_; }
void PlotMSPlotParameters::setAveraging(const PlotMSAveraging& avg) {
    if(avg != itsMSAveraging_) {
        itsMSAveraging_ = avg;
        if(updateFlag_) updateFlag(MS);
    }
}

PlotMSPlotParameters&
PlotMSPlotParameters::operator=(const PlotMSPlotParameters& copy) {
    PlotMSWatchedParameters::operator=(copy);
    
    if(!equals(copy, MS)) {
        bool oldupdate = updateFlag_;
        updateFlag_ = false;
                
        setFilename(copy.filename());
        setSelection(copy.selection());
        setAveraging(copy.averaging());
        if(oldupdate) updateFlag(MS);
        
        updateFlag_ = oldupdate;
    }
    return *this;
}


////////////////////////////////////////////
// PLOTMSSINGLEPLOTPARAMETERS DEFINITIONS //
////////////////////////////////////////////

// Constructors/Destructors //

PlotMSSinglePlotParameters::PlotMSSinglePlotParameters(PlotFactoryPtr factory,
        const String& msFilename) : PlotMSPlotParameters(msFilename) {
    setDefaults(factory);
}

PlotMSSinglePlotParameters::PlotMSSinglePlotParameters(PlotMS* parent,
        const String& msFilename) : PlotMSPlotParameters(msFilename) {
    setDefaults(parent->getPlotter()->getFactory());
}

PlotMSSinglePlotParameters::PlotMSSinglePlotParameters(
        const PlotMSSinglePlotParameters& copy) : PlotMSPlotParameters(copy) {
    operator=(copy);
}

PlotMSSinglePlotParameters::~PlotMSSinglePlotParameters() { }


// Public Methods //

bool PlotMSSinglePlotParameters::equals(const PlotMSWatchedParameters& other,
        int updateFlags) const {
    const PlotMSSinglePlotParameters* o = dynamic_cast<
                                    const PlotMSSinglePlotParameters*>(&other);
    if(o == NULL) return false;
    
    // This will check MS.
    if(!PlotMSPlotParameters::equals(other, updateFlags)) return false;
        
    // Check axes
    if(updateFlags & CACHE) {
        if(itsXAxis_!= o->itsXAxis_ || itsYAxis_!= o->itsYAxis_) return false;
        if(PMS::axisIsData(itsXAxis_) && itsXDataColumn_ != o->itsXDataColumn_)
            return false;
        if(PMS::axisIsData(itsYAxis_) && itsYDataColumn_ != o->itsYDataColumn_)
            return false;
    }
    
    // Check canvas
    if(updateFlags & CANVAS) {
        if(itsXAxis_ != o->itsXAxis_ || itsYAxis_ != o->itsYAxis_ ||
           itsXCanvasAxis_ != o->itsXCanvasAxis_ ||
           itsYCanvasAxis_ != o->itsYCanvasAxis_ ||
           itsXRangeSet_ != o->itsXRangeSet_ ||
           itsYRangeSet_ != o->itsYRangeSet_ ||
           (itsXRangeSet_ && itsXRange_ != o->itsXRange_) ||
           (itsYRangeSet_ && itsYRange_ != o->itsYRange_) ||
           itsShowXAxis_ != o->itsShowXAxis_ ||
           itsShowYAxis_ != o->itsShowYAxis_ ||
           (itsShowXAxis_ &&
                 itsCanvasXAxisLabelFormat_!= o->itsCanvasXAxisLabelFormat_) ||
           (itsShowYAxis_ &&
                 itsCanvasYAxisLabelFormat_!= o->itsCanvasYAxisLabelFormat_) ||
           itsShowLegend_ != o->itsShowLegend_ ||
           (itsShowLegend_ && itsLegendPos_ != o->itsLegendPos_) ||
           itsCanvasTitleFormat_ != o->itsCanvasTitleFormat_) return false;
    }
    
    // Check plot
    if(updateFlags & PLOT) {
        if(itsXAxis_ != o->itsXAxis_ || itsYAxis_ != o->itsYAxis_ ||
           itsXCanvasAxis_ != o->itsXCanvasAxis_ ||
           itsYCanvasAxis_ != o->itsYCanvasAxis_) return false;
        
        if(itsSymbol_.null() != o->itsSymbol_.null() ||
           itsMaskedSymbol_.null() != o->itsMaskedSymbol_.null()) return false;
        if(!itsSymbol_.null() && *itsSymbol_ != *o->itsSymbol_) return false;
        if(!itsMaskedSymbol_.null() &&
           *itsMaskedSymbol_ != *o->itsMaskedSymbol_) return false;
        if(itsPlotTitleFormat_ != o->itsPlotTitleFormat_) return false;
    }
    
    return true;
}


PMS::Axis PlotMSSinglePlotParameters::xAxis() const { return itsXAxis_; }
PMS::Axis PlotMSSinglePlotParameters::yAxis() const { return itsYAxis_; }
PMS::DataColumn PlotMSSinglePlotParameters::xDataColumn() const {
    return itsXDataColumn_; }
PMS::DataColumn PlotMSSinglePlotParameters::yDataColumn() const {
    return itsYDataColumn_; }

void PlotMSSinglePlotParameters::setAxesAndDataColumns(PMS::Axis x,
        PMS::DataColumn xData, PMS::Axis y, PMS::DataColumn yData) {
    if(itsXAxis_ != x || itsYAxis_ != y || itsXDataColumn_ != xData ||
       itsYDataColumn_ != yData) {
        itsXAxis_ = x;
        itsYAxis_ = y;
        itsXDataColumn_ = xData;
        itsYDataColumn_ = yData;

        // The canvas and plot labels etc. will also need to be updated.
        if(updateFlag_) updateFlags(currentUpdateFlag()|CACHE | CANVAS | PLOT);
    }
}


PlotAxis PlotMSSinglePlotParameters::canvasXAxis() const {
    return itsXCanvasAxis_; }
PlotAxis PlotMSSinglePlotParameters::canvasYAxis() const {
    return itsYCanvasAxis_; }

void PlotMSSinglePlotParameters::setCanvasAxes(PlotAxis xAxis, PlotAxis yAxis){
    if(itsXCanvasAxis_ != xAxis || itsYCanvasAxis_ != yAxis) {
        itsXCanvasAxis_ = xAxis;
        itsYCanvasAxis_ = yAxis;
        
        // Both the plot and canvas will need to be updated.
        if(updateFlag_) updateFlags(currentUpdateFlag() | CANVAS | PLOT);
    }
}

bool PlotMSSinglePlotParameters::xRangeSet() const { return itsXRangeSet_; }
bool PlotMSSinglePlotParameters::yRangeSet() const { return itsYRangeSet_; }
pair<double, double> PlotMSSinglePlotParameters::xRange() const {
    return itsXRange_; }
pair<double, double> PlotMSSinglePlotParameters::yRange() const {
    return itsYRange_; }

void PlotMSSinglePlotParameters::setRanges(bool xSet,
        pair<double, double> xRange, bool ySet, pair<double, double> yRange) {
    if(xSet != itsXRangeSet_ || ySet != itsYRangeSet_ ||
       (xSet && xRange != itsXRange_) || (ySet && yRange != itsYRange_)) {
        itsXRangeSet_ = xSet;
        itsXRange_ = xRange;
        itsYRangeSet_ = ySet;
        itsYRange_ = yRange;
        
        if(updateFlag_) updateFlag(CANVAS);
    }
}


const PlotMSLabelFormat&
PlotMSSinglePlotParameters::canvasXAxisLabelFormat() const {
    return itsCanvasXAxisLabelFormat_; }
const PlotMSLabelFormat&
PlotMSSinglePlotParameters::canvasYAxisLabelFormat() const {
    return itsCanvasYAxisLabelFormat_; }

void PlotMSSinglePlotParameters::setCanvasAxesLabelFormats(
        const PlotMSLabelFormat& x, const PlotMSLabelFormat& y) {
    if(itsCanvasXAxisLabelFormat_ != x || itsCanvasYAxisLabelFormat_ != y) {
        // Only redraw if the label is shown/hidden.
        bool redraw = (x.getLabel(itsXAxis_).empty() !=
            itsCanvasXAxisLabelFormat_.getLabel(itsXAxis_).empty()) ||
            (y.getLabel(itsYAxis_).empty() !=
            itsCanvasYAxisLabelFormat_.getLabel(itsYAxis_).empty());           
        
        itsCanvasXAxisLabelFormat_ = x;
        itsCanvasYAxisLabelFormat_ = y;
        
        if(updateFlag_) updateFlag(CANVAS, true, redraw);
    }
}

String PlotMSSinglePlotParameters::canvasXAxisLabel() const {
    return itsCanvasXAxisLabelFormat_.getLabel(itsXAxis_); }
String PlotMSSinglePlotParameters::canvasYAxisLabel() const {
    return itsCanvasYAxisLabelFormat_.getLabel(itsYAxis_); }


bool PlotMSSinglePlotParameters::showXAxis() const { return itsShowXAxis_; }
bool PlotMSSinglePlotParameters::showYAxis() const { return itsShowYAxis_; }
void PlotMSSinglePlotParameters::setShowAxes(bool showX, bool showY) {
    if(itsShowXAxis_ != showX || itsShowYAxis_ != showY) {
        itsShowXAxis_ = showX;
        itsShowYAxis_ = showY;
        
        if(updateFlag_) updateFlag(CANVAS);
    }
}


bool PlotMSSinglePlotParameters::showLegend() const { return itsShowLegend_; }
PlotCanvas::LegendPosition PlotMSSinglePlotParameters::legendPosition() const {
    return itsLegendPos_; }

void PlotMSSinglePlotParameters::setLegend(bool show,
        PlotCanvas::LegendPosition position) {
    if(itsShowLegend_ != show || itsLegendPos_ != position) {
        bool redraw = false;
        if(show != show) { // only redraw if external legend
            redraw = !PlotCanvas::legendPositionIsInternal(position);
        } else {// only redraw if going from external to internal or vice-versa
            redraw = PlotCanvas::legendPositionIsInternal(position) !=
                     PlotCanvas::legendPositionIsInternal(itsLegendPos_);
        }
        
        itsShowLegend_ = show;
        itsLegendPos_ = position;
        
        if(updateFlag_) updateFlag(CANVAS, true, redraw);
    }
}


const PlotMSLabelFormat& PlotMSSinglePlotParameters::canvasTitleFormat() const{
    return itsCanvasTitleFormat_; }

void PlotMSSinglePlotParameters::setCanvasTitleFormat(
        const PlotMSLabelFormat& format) {
    if(itsCanvasTitleFormat_ != format) {
        // Only redraw if the title is shown/hidden.
        bool redraw = format.getLabel(itsXAxis_, itsYAxis_).empty() !=
            itsCanvasTitleFormat_.getLabel(itsXAxis_, itsYAxis_).empty();
        
        itsCanvasTitleFormat_ = format;
        
        if(updateFlag_) updateFlag(CANVAS, true, redraw);
    }
}

String PlotMSSinglePlotParameters::canvasTitle() const {
    return itsCanvasTitleFormat_.getLabel(itsXAxis_, itsYAxis_); }


PlotSymbolPtr PlotMSSinglePlotParameters::symbol() const { return itsSymbol_; }
PlotSymbolPtr PlotMSSinglePlotParameters::maskedSymbol() const {
    return itsMaskedSymbol_; }

void PlotMSSinglePlotParameters::setSymbols(PlotSymbolPtr normal,
        PlotSymbolPtr masked) {
    // first check for different nulls
    bool update = itsSymbol_.null() != normal.null() ||
                  itsMaskedSymbol_.null() != masked.null();
    
    // if they're all the same, check equality if not null
    if(!update && !itsSymbol_.null()) update = *itsSymbol_ != *normal;
    if(!update&& !itsMaskedSymbol_.null()) update= *itsMaskedSymbol_!= *masked;
    
    if(update) {
        itsSymbol_ = normal;
        itsMaskedSymbol_ = masked;
        
        if(updateFlag_) updateFlag(PLOT);
    }
}


const PlotMSLabelFormat& PlotMSSinglePlotParameters::plotTitleFormat() const {
    return itsPlotTitleFormat_; }

void PlotMSSinglePlotParameters::setPlotTitleFormat(
        const PlotMSLabelFormat& format) {
    if(itsPlotTitleFormat_ != format) {        
        itsPlotTitleFormat_ = format;
        
        // Redraw is not required.
        if(updateFlag_) updateFlag(PLOT, true, false);
    }
}

String PlotMSSinglePlotParameters::plotTitle() const {
    return itsPlotTitleFormat_.getLabel(itsXAxis_, itsYAxis_); }


PlotMSSinglePlotParameters& PlotMSSinglePlotParameters::operator=(
        const PlotMSSinglePlotParameters& copy) {
    // This will copy the MS stuff.
    PlotMSPlotParameters::operator=(copy);

    setAxesAndDataColumns(copy.xAxis(), copy.xDataColumn(), copy.yAxis(),
                          copy.yDataColumn());
    
    setCanvasAxes(copy.canvasXAxis(), copy.canvasYAxis());
    setRanges(copy.xRangeSet(), copy.xRange(), copy.yRangeSet(),copy.yRange());
    setCanvasAxesLabelFormats(copy.canvasXAxisLabelFormat(),
                              copy.canvasYAxisLabelFormat());
    setShowAxes(copy.showXAxis(), copy.showYAxis());
    setLegend(copy.showLegend(), copy.legendPosition());
    setCanvasTitleFormat(copy.canvasTitleFormat());

    setSymbols(copy.symbol(), copy.maskedSymbol());
    setPlotTitleFormat(copy.plotTitleFormat());

    return *this;
}


// Private Methods //

void PlotMSSinglePlotParameters::setDefaults(PlotFactoryPtr factory) {
    itsXAxis_ = PMS::DEFAULT_XAXIS;
    itsYAxis_ = PMS::DEFAULT_YAXIS;
    itsXDataColumn_ = itsYDataColumn_ = PMS::DEFAULT_DATACOLUMN;
    
    itsXCanvasAxis_ = PMS::DEFAULT_CANVAS_XAXIS;
    itsYCanvasAxis_ = PMS::DEFAULT_CANVAS_YAXIS;
    itsXRangeSet_ = itsYRangeSet_ = false;
    itsXRange_ = itsYRange_ = pair<double, double>(0, 0);
    
    itsCanvasXAxisLabelFormat_ = itsCanvasYAxisLabelFormat_ =
        PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT;
    itsShowXAxis_ = itsShowYAxis_ = PMS::DEFAULT_SHOWAXIS;
    itsShowLegend_ = PMS::DEFAULT_SHOWLEGEND;
    itsLegendPos_ = PMS::DEFAULT_LEGENDPOSITION;
    itsCanvasTitleFormat_ = itsPlotTitleFormat_ = PMS::DEFAULT_TITLE_FORMAT;
    itsSymbol_ = PMS::DEFAULT_SYMBOL(factory);
    itsMaskedSymbol_ = PMS::DEFAULT_MASKED_SYMBOL(factory);
    
    if(updateFlag_) updateFlags(ALL);
}

}
