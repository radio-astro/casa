//# PlotMSData.cc: Connects data cache and plotting data objects.
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
#include <plotms/Data/PlotMSData.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

////////////////////////////
// PLOTMSDATA DEFINITIONS //
////////////////////////////

PlotMSData::PlotMSData(PlotMS* parent) : itsCache_(new PlotMSCache(parent)) { }

PlotMSData::PlotMSData(const PlotMSData& copy) { operator=(copy); }

PlotMSData::~PlotMSData() { }


bool PlotMSData::isValid() const {
    return true; // has to be valid for PlotMSPlot::initialize() to create the
                 // plot object.
}

unsigned int PlotMSData::size() const {
    if(!itsCache_->readyForPlotting()) return 0;
    else return (unsigned int)itsCache_->nPoints();
}

double PlotMSData::xAt(unsigned int i) const {
    return itsCache_->getX((int)i); }

double PlotMSData::yAt(unsigned int i) const {
    return itsCache_->getY((int)i); }

void PlotMSData::xAndYAt(unsigned int i, double& x, double& y) const {
    itsCache_->getXY((int)i, x, y); }

bool PlotMSData::minsMaxes(double& xMin, double& xMax, double& yMin,
        double& yMax) {
    if(size() > 0) {
        itsCache_->getRanges(xMin, xMax, yMin, yMax);
        return true;
    } else return false;
}


bool PlotMSData::maskedAt(unsigned int index) const {
    return itsCache_->getFlagMask((int)index); }

void PlotMSData::xyAndMaskAt(unsigned int index, double& x, double& y,
        bool& mask) const {
  mask = itsCache_->getFlagMask((int)index);
  xAndYAt(index, x, y);
}


double PlotMSData::cacheReferenceTime() const { return itsCache_->refTime(); }

bool PlotMSData::cacheReady() const { return itsCache_->readyForPlotting(); }

void PlotMSData::clearCache() { itsCache_->clear(); }

void PlotMSData::loadCache(VisSet& visSet, PMS::Axis xAxis, PMS::Axis yAxis,
        PMS::DataColumn xData, PMS::DataColumn yData,
        const PlotMSAveraging& averaging, PlotMSCacheThread* thread) {
    itsCache_->load(visSet, xAxis, yAxis, xData, yData, averaging, thread); }

void PlotMSData::loadCache(VisSet& visSet, const vector<PMS::Axis>& axes,
        const vector<PMS::DataColumn>& data, const PlotMSAveraging& averaging,
        PlotMSCacheThread* thread) {
    itsCache_->load(visSet, axes, data, averaging, thread); }

void PlotMSData::setupCache(PMS::Axis xAxis, PMS::Axis yAxis) {
    itsCache_->setUpPlot(xAxis, yAxis); }

PlotLogMessage* PlotMSData::locateRange(const Vector<PlotRegion>& regions) {
    return itsCache_->locateRange(regions); }

PlotLogMessage* PlotMSData::flagRange(const PlotMSFlagging& flagging,
        const Vector<PlotRegion>& regions, bool flag) {
    return itsCache_->flagRange(flagging, regions, flag); }

vector<pair<PMS::Axis, unsigned int> > PlotMSData::loadedAxes() const {
    return itsCache_->loadedAxes(); }


PlotMSData& PlotMSData::operator=(const PlotMSData& copy) {
    itsPlotms_ = copy.itsPlotms_;
    itsCache_ = copy.itsCache_;
    return *this;
}

}
