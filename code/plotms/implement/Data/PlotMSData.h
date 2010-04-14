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

#include <graphics/GenericPlotter/PlotData.h>

#include <plotms/Data/PlotMSCache.h>
#include <plotms/Data/PlotMSCacheIndexer.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward declarations.
class PlotMS;


// Layer between plot cache and the rest of PlotMS and the GenericPlotter
// classes.
class PlotMSData : public PlotMaskedPointData, public PlotBinnedData {
public:
    // Constructor which takes the parent PlotMS.
    PlotMSData(PlotMS* plotms);
    
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
    
    // Implemented PlotMaskedPointData methods.
    // <group>
    bool maskedAt(unsigned int index) const;
    void xyAndMaskAt(unsigned int index, double& x, double& y,
            bool& mask) const;
    // </group>
    
    // Unimplemented PlotMaskedPointData methods.
    // <group>
    unsigned int sizeMasked() const { return 0; }
    unsigned int sizeUnmasked() const { return size(); }
    bool maskedMinsMaxes(double& xMin, double& xMax, double& yMin,
            double& yMax) { return minsMaxes(xMin, xMax, yMin, yMax); }
    bool unmaskedMinsMaxes(double& xMin, double& xMax, double& yMin,
            double& yMax) { return minsMaxes(xMin, xMax, yMin, yMax); }
    // </group>
    
    // Implemented PlotBinnedData methods.
    // <group>
    unsigned int numBins() const;
    unsigned int binAt(unsigned int i) const;
    bool isBinned() const;
    // </group>
    
    
    // Tells the cache what to colorize on, on the next draw.  Returns whether
    // the plot needs to be redrawn or not.
    bool colorize(bool doColorize, PMS::Axis colorizeAxis);    
    
    // Returns the reference value for the given axis, if applicable.
    // <group>
    bool hasReferenceValue(PMS::Axis axis);
    double referenceValue(PMS::Axis axis);
    // </group>
    
    // See PlotMSCache::readyForPlotting().
    bool cacheReady() const;
    
    // See PlotMSCache::clear().
    void clearCache();
    
    // Loads values into the cache using the given vis set, with the given
    // axes used for x and y values.  See PlotMSCache::load().
    // <group>
    void loadCache(PMS::Axis xAxis, PMS::Axis yAxis,
		   PMS::DataColumn xData, PMS::DataColumn yData,
		   const String& msname,
		   const PlotMSSelection& selection,
		   const PlotMSAveraging& averaging,
		   const PlotMSTransformations& transformations,
		   PlotMSCacheThread* thread = NULL);
    void loadCache(const vector<PMS::Axis>& axes,
		   const vector<PMS::DataColumn>& data,
		   const String& msname,
		   const PlotMSSelection& selection,
		   const PlotMSAveraging& averaging,
		   const PlotMSTransformations& transformations,
		   PlotMSCacheThread* thread = NULL);
    // </group>
    
    // See PlotMSCache::setUpPlot().
    void setupCache(PMS::Axis xAxis, PMS::Axis yAxis);
    
    // See PlotMSCache::release().
    void releaseCache(const vector<PMS::Axis>& axes);
    
    // See PlotMSCache::locateRange().
    PlotLogMessage* locateRange(const Vector<PlotRegion>& regions);
    
    // See PlotMSCache::flagRange().
    PlotLogMessage* flagRange(const PlotMSFlagging& flagging,
            const Vector<PlotRegion>& regions, bool flag = true);
    
    // See PlotMSCache::loadedAxes().
    vector<pair<PMS::Axis, unsigned int> > loadedAxes() const;
    
    
    // Copy operator.
    PlotMSData& operator=(const PlotMSData& copy);
    
private:
    // Parent.
    PlotMS* itsPlotms_;
    
    // Cache.
    PlotMSCachePtr itsCache_;

    // Indexer
    PlotMSCacheIndexerPtr itsIndexer_;
    
    // Last set colorizing parameters.
    // <group>
    bool itsColorize_;
    PMS::Axis itsColorizeAxis_;
    // </group>

};

}

#endif /* PLOTMSDATA_H_ */
