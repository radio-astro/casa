//# PlotMSData.h: Connects data cache and plotting data objects.
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
#ifndef PLOTMSDATA_H_
#define PLOTMSDATA_H_

#include <plotms/Data/PlotMSCache.h>
#include <plotms/PlotMS/PlotMSParameters.h>

#include <casa/Arrays/Vector.h>
#include <graphics/GenericPlotter/PlotData.h>
#include <msvis/MSVis/VisSet.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// Layer between plot cache and the rest of PlotMS and the GenericPlotter
// classes.
class PlotMSData : public PlotMaskedPointData {
public:
    // Constructor.
    PlotMSData();
    
    // Copy constructor.  See operator=().
    PlotMSData(const PlotMSData& copy);
    
    // Destructor.
    ~PlotMSData();
    
    
    // Implemented PlotData methods.
    // <group>
    bool willDeleteData() const { return true; }
    void setDeleteData(bool del = true) { }
    bool isValid() const;
    // </group>
    
    // Implemented PlotPointData methods.
    // <group>
    unsigned int size() const;
    double xAt(unsigned int i) const;
    double yAt(unsigned int i) const;
    void xAndYAt(unsigned int index, double& x, double& y) const;
    bool minsMaxes(double& xMin, double& xMax, double& yMin, double& yMax);
    // </group>
    
    // Implemented PlotMaskedPointData methods.  Not implemented for now until
    // flagging is ready in the cache.
    // <group>
    unsigned int sizeMasked() const { return 0; }
    unsigned int sizeUnmasked() const { return size(); }
    bool maskedAt(unsigned int index) const { return itsCache_->getFlagMask((int)index); }
    bool maskedMinsMaxes(double& xMin, double& xMax, double& yMin,
            double& yMax) { return minsMaxes(xMin, xMax, yMin, yMax); }
    bool unmaskedMinsMaxes(double& xMin, double& xMax, double& yMin,
            double& yMax) { return minsMaxes(xMin, xMax, yMin, yMax); }
    void xyAndMaskAt(unsigned int index, double& x, double& y,
            bool& mask) const {
      mask = itsCache_->getFlagMask((int)index);
      xAndYAt(index, x, y);
    }
    // </group>
    
    
    // See PlotMSCache::refTime().
    double cacheReferenceTime() const;
    
    // See PlotMSCache::readyForPlotting().
    bool cacheReady() const;
    
    // See PlotMSCache::clear().
    void clearCache();
    
    // Loads values into the cache using the given vis set, with the given
    // axes used for x and y values.  See PlotMSCache::load().
    // <group>
    void loadCache(VisSet& visSet, PMS::Axis xAxis, PMS::Axis yAxis,
            PMS::DataColumn xData, PMS::DataColumn yData,
            const PlotMSAveraging& averaging,
            PlotMSCacheThread* thread = NULL);
    void loadCache(VisSet& visSet, const vector<PMS::Axis>& axes,
            const vector<PMS::DataColumn>& data,
            const PlotMSAveraging& averaging,
            PlotMSCacheThread* thread = NULL);
    // </group>
    
    // See PlotMSCache::setUpPlot().
    void setupCache(PMS::Axis xAxis, PMS::Axis yAxis);
    
    // See PlotMSCache::locateRange().
    PlotLogMessage* locateRange(double xMin, double xMax, double yMin,
            double yMax);
    
    // See PlotMSCache::flagRange().
    PlotLogMessage* flagRange(const PlotMSFlagging& flagging, double xMin,
            double xMax, double yMin, double yMax, bool flag = true);
    
    // See PlotMSCache::loadedAxes().
    vector<pair<PMS::Axis, unsigned int> > loadedAxes() const;
    
    
    // Copy operator.
    PlotMSData& operator=(const PlotMSData& copy);
    
private:    
    // Cache.
    PlotMSCachePtr itsCache_;
};

}

#endif /* PLOTMSDATA_H_ */
