//# PlotMSRegions.cc: Properties of selected regions for a PlotMSPlot.
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
#include <plotms/PlotMS/PlotMSRegions.h>

namespace casa {

//////////////////////////////
// PLOTMSREGION DEFINITIONS //
//////////////////////////////

PlotMSRegion::PlotMSRegion() : itsXMin_(0), itsXMax_(0), itsYMin_(0),
        itsYMax_(0) { }

PlotMSRegion::PlotMSRegion(double xMin, double xMax, double yMin, double yMax):
        itsXMin_(xMin), itsXMax_(xMax), itsYMin_(yMin), itsYMax_(yMax) { }

PlotMSRegion::PlotMSRegion(const PlotMSRegion& copy) { operator=(copy); }

PlotMSRegion::~PlotMSRegion() { }

bool PlotMSRegion::isValid() const {
    return itsXMin_ < itsXMax_ && itsYMin_ < itsYMax_; }

bool PlotMSRegion::contains(double x, double y, bool exclusive) const {
    if(exclusive)
        return itsXMin_ < x && x < itsXMax_ && itsYMin_ < y && y < itsYMax_;
    else return itsXMin_<= x && x<= itsXMax_ && itsYMin_<= y && y<= itsYMax_;
}

double PlotMSRegion::xMin() const { return itsXMin_; }
double PlotMSRegion::xMax() const { return itsXMax_; }
double PlotMSRegion::yMin() const { return itsYMin_; }
double PlotMSRegion::yMax() const { return itsYMax_; }

void PlotMSRegion::setValues(double xMin,double xMax,double yMin,double yMax) {
    itsXMin_ = xMin;
    itsXMax_ = xMax;
    itsYMin_ = yMin;
    itsYMax_ = yMax;
}

void PlotMSRegion::setX(double min, double max) {
    itsXMin_ = min; itsXMax_ = max; }
void PlotMSRegion::setY(double min, double max) {
    itsYMin_ = min; itsYMax_ = max; }
void PlotMSRegion::setXMin(double val) { itsXMin_ = val; }
void PlotMSRegion::setXMax(double val) { itsXMax_ = val; }
void PlotMSRegion::setYMin(double val) { itsYMin_ = val; }
void PlotMSRegion::setYMax(double val) { itsYMax_ = val; }

bool PlotMSRegion::operator==(const PlotMSRegion& region) const {
    return region.itsXMin_ == itsXMin_ && region.itsXMax_ == itsXMax_ &&
           region.itsYMin_ == itsYMin_ && region.itsYMax_ == itsYMax_;
}

PlotMSRegion& PlotMSRegion::operator=(const PlotMSRegion& copy) {
    setValues(copy.itsXMin_, copy.itsXMax_, copy.itsYMin_, copy.itsYMax_);
    return *this;
}


///////////////////////////////
// PLOTMSREGIONS DEFINITIONS //
///////////////////////////////

PlotMSRegions::PlotMSRegions() { }

PlotMSRegions::~PlotMSRegions() { }

Vector<pair<PMS::Axis, PMS::Axis> > PlotMSRegions::allAxisPairs() const {
    Vector<pair<PMS::Axis, PMS::Axis> > v(itsRegions_.size());
    unsigned int index = 0;
    for(PMSRMap::const_iterator i=itsRegions_.begin();i!=itsRegions_.end();i++)
        v[index++] = i->first;
    return v;
}

bool PlotMSRegions::hasRegionsFor(PMS::Axis x, PMS::Axis y) const {
    return itsRegions_.find(pair<PMS::Axis, PMS::Axis>(x, y)) !=
        itsRegions_.end();
}

Vector<PlotMSRegion> PlotMSRegions::regionsFor(PMS::Axis x, PMS::Axis y) const{
    Vector<PlotMSRegion> v;
    PMSRMap::const_iterator it = itsRegions_.find(
            pair<PMS::Axis, PMS::Axis>(x, y));
    if(it != itsRegions_.end()) v = it->second;
    return v;
}

void PlotMSRegions::addRegions(PMS::Axis x, PMS::Axis y,
        const Vector<PlotMSRegion>& regs) {
    Vector<PlotMSRegion> v = regionsFor(x, y);
    v.resize(v.size() + regs.size());
    v(Slice(v.size() - regs.size(), regs.size())) = regs;
    setRegions(x, y, v);
}

void PlotMSRegions::addRegions(PMS::Axis x, PMS::Axis y, PlotCanvasPtr canvas){
    if(!canvas.null()) {
        vector<PlotRegion> v = canvas->standardMouseTools()->selectTool()
                               ->getSelectedRects(PlotCoordinate::WORLD);
        vector<PlotMSRegion> v2(v.size());
        for(unsigned int i = 0; i < v.size(); i++)
            v2[i].setValues(v[i].left(),v[i].right(),v[i].bottom(),v[i].top());
        addRegions(x, y, v2);
    }
}

void PlotMSRegions::setRegions(PMS::Axis x, PMS::Axis y,
        const Vector<PlotMSRegion>& regs) {
    vector<PlotMSRegion> unique;
    bool found = false;
    for(uInt i = 0; i < regs.size(); i++) {
        found = false;
        for(unsigned int j = 0; !found && j < unique.size(); j++)
            if(unique[j] == regs[i]) found = true;
        if(!found) unique.push_back(regs[i]);
    }
    itsRegions_[pair<PMS::Axis,PMS::Axis>(x,y)] = Vector<PlotMSRegion>(unique);
}

void PlotMSRegions::setRegions(PMS::Axis x, PMS::Axis y, PlotCanvasPtr canvas){
    if(!canvas.null()) {
        vector<PlotRegion> v = canvas->standardMouseTools()->selectTool()
                               ->getSelectedRects(PlotCoordinate::WORLD);
        vector<PlotMSRegion> v2(v.size());
        for(unsigned int i = 0; i < v.size(); i++)
            v2[i].setValues(v[i].left(),v[i].right(),v[i].bottom(),v[i].top());
        setRegions(x, y, v2);
    }
}

void PlotMSRegions::clearRegions(PMS::Axis x, PMS::Axis y) {
    PMSRMap::iterator it = itsRegions_.find(pair<PMS::Axis, PMS::Axis>(x, y));
    if(it != itsRegions_.end()) itsRegions_.erase(it);
}

}
