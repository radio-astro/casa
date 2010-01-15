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

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMS.h>

namespace casa {

////////////////////////////
// PLOTMSDATA DEFINITIONS //
////////////////////////////


PlotMSData::PlotMSData(PlotMS* parent) : itsCache_(new PlotMSCache(parent)),
        itsColorize_(false), itsColorizeAxis_(PMS::DEFAULT_COLOR_AXIS) {

  itsIndexer_ = new PlotMSCacheIndexer(&*itsCache_);

}

PlotMSData::PlotMSData(const PlotMSData& copy) { operator=(copy); }

PlotMSData::~PlotMSData() { }

bool PlotMSData::isValid() const {
    return true; // has to be valid for PlotMSPlot::initialize() to create the
                 // plot object.
}



  //#define PMSINDEXER
#ifdef PMSINDEXER

unsigned int PlotMSData::size() const {

  if(!itsIndexer_->indexerReady()) return 0;
  return (unsigned int)itsIndexer_->nPoints();
}

double PlotMSData::xAt(unsigned int i) const {
    return itsIndexer_->getX((int)i); }

double PlotMSData::yAt(unsigned int i) const {
    return itsIndexer_->getY((int)i); }

void PlotMSData::xAndYAt(unsigned int i, double& x, double& y) const {
    itsIndexer_->getXY((int)i, x, y); }

bool PlotMSData::minsMaxes(double& xMin, double& xMax, double& yMin,
        double& yMax) {

  if(size() > 0) {
    itsIndexer_->getRanges(xMin, xMax, yMin, yMax);
    return true;
  } else return false;
}


bool PlotMSData::maskedAt(unsigned int index) const {
    return itsIndexer_->getFlagMask((int)index); }

void PlotMSData::xyAndMaskAt(unsigned int index, double& x, double& y,
        bool& mask) const {
  mask = itsIndexer_->getFlagMask((int)index);
  xAndYAt(index, x, y);
}


#else

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

#endif

unsigned int PlotMSData::numBins() const {
    // TODO
    return PMS::COLORS_LIST().size();
}

unsigned int PlotMSData::binAt(unsigned int i) const {
    // TODO
    if(itsColorize_) {
        itsCache_->setChunk(i);
        unsigned int val = (unsigned int)itsCache_->get(itsColorizeAxis_);
        return val % numBins();
    } else return 0;
}

bool PlotMSData::isBinned() const {
    // TODO
    return itsColorize_;
}


bool PlotMSData::colorize(bool doColorize, PMS::Axis colorizeAxis) {
    // TODO
    bool changed = doColorize != itsColorize_ ||
                   (doColorize && colorizeAxis != itsColorizeAxis_);
    itsColorize_ = doColorize;
    itsColorizeAxis_ = colorizeAxis;
    return changed;
}

bool PlotMSData::hasReferenceValue(PMS::Axis axis) {
    return axis == PMS::TIME && cacheReady(); }

double PlotMSData::referenceValue(PMS::Axis axis) {
    if(!cacheReady()) return 0;
    else if(axis == PMS::TIME) return itsCache_->refTime();
    else return 0;
}

void PlotMSData::clearCache() { itsCache_->clear(); }

void PlotMSData::loadCache(PMS::Axis xAxis, PMS::Axis yAxis,
			   PMS::DataColumn xData, PMS::DataColumn yData,
			   const String& msname, 
			   const PlotMSSelection& selection, 
			   const PlotMSAveraging& averaging, 
			   const PlotMSTransformations& transformations, 
			   PlotMSCacheThread* thread) {
  itsCache_->load(xAxis, yAxis, xData, yData, 
		  msname, selection,averaging,transformations, thread); }

void PlotMSData::loadCache(const vector<PMS::Axis>& axes,
			   const vector<PMS::DataColumn>& data, 
			   const String& msname,
			   const PlotMSSelection& selection,
			   const PlotMSAveraging& averaging,
			   const PlotMSTransformations& transformations,
			   PlotMSCacheThread* thread) {
  itsCache_->load(axes, data, 
		  msname, selection,averaging,transformations, thread); }


#ifdef PMSINDEXER

bool PlotMSData::cacheReady() const { return itsIndexer_->indexerReady(); } 


void PlotMSData::setupCache(PMS::Axis xAxis, PMS::Axis yAxis) {

  cout << "setUpPlot" << endl;

    itsIndexer_->setUpPlot(xAxis, yAxis); }
#else

bool PlotMSData::cacheReady() const { return itsCache_->readyForPlotting(); }

void PlotMSData::setupCache(PMS::Axis xAxis, PMS::Axis yAxis) {
    itsCache_->setUpPlot(xAxis, yAxis); }
#endif

void PlotMSData::releaseCache(const vector<PMS::Axis>& axes) {
    itsCache_->release(axes); }

#ifdef PMSINDEXER
PlotLogMessage* PlotMSData::locateRange(const Vector<PlotRegion>& regions) {
    return itsIndexer_->locateRange(regions); }
PlotLogMessage* PlotMSData::flagRange(const PlotMSFlagging& flagging,
        const Vector<PlotRegion>& regions, bool flag) {
    return itsIndexer_->flagRange(flagging, regions, flag); }

#else
PlotLogMessage* PlotMSData::locateRange(const Vector<PlotRegion>& regions) {
    return itsCache_->locateRange(regions); }
PlotLogMessage* PlotMSData::flagRange(const PlotMSFlagging& flagging,
        const Vector<PlotRegion>& regions, bool flag) {
    return itsCache_->flagRange(flagging, regions, flag); }

#endif





vector<pair<PMS::Axis, unsigned int> > PlotMSData::loadedAxes() const {
    return itsCache_->loadedAxes(); }


PlotMSData& PlotMSData::operator=(const PlotMSData& copy) {
    itsPlotms_ = copy.itsPlotms_;
    itsCache_ = copy.itsCache_;
    itsColorize_ = copy.itsColorize_;
    itsColorizeAxis_ = copy.itsColorizeAxis_;
    
    itsIndexer_ = new PlotMSCacheIndexer(&*itsCache_);
    
    return *this;
}

}
