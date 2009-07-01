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
#include <plotms/Plots/PlotMSPlotParameters.h>

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
           itsMSSelection_ != o->itsMSSelection_ ||
           itsMSAveraging_ != o->itsMSAveraging_)
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

}
