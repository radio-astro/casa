//# PlotMSData.h: Connects data cache and plotting data objects.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#ifndef PLOTMSDATA_H_
#define PLOTMSDATA_H_

#include <casa/Arrays/Vector.h>
#include <graphics/GenericPlotter/PlotData.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/PlotMS/PlotMSOptions.h>
#include <msvis/PlotMS/PMSMemCache.h>

#include <casa/namespace.h>

namespace casa {

// Single data/cache container.
class PlotMSData : public PlotPointData {
public:
    PlotMSData();
    
    ~PlotMSData();
    
    // Implemented PlotData methods.
    // <group>
    bool willDeleteData() const { return true; }
    void setDeleteData(bool del = true) { }
    bool isValid() const;
    unsigned int size() const;
    double xAt(unsigned int i) const;
    double yAt(unsigned int i) const;
    bool minsMaxes(double& xMin, double& xMax, double& yMin, double& yMax);
    // </group>
    
    // Sets up the x and y methods to return the proper value.
    void setupCache(PMS::Axis xAxis, PMS::Axis yAxis);
    
    // Loads values into the cache using the given vis set, with the given
    // axes used for x and y values.
    void loadCache(VisSet& visSet, PMS::Axis xAxis, PMS::Axis yAxis);
    
    // Loads values into the cache using the given vis set.  Ensures that the
    // given axes are available as axes choices for the cache.
    // void loadCache(VisSet& visSet, const vector<PMS::Axis>& axes);
    
    // Locates all the points in the given range by calling the proper method
    // on the cache.
    void locate(double xMin, double xMax, double yMin, double yMax);
    
private:
    PMSMemCache itsCache_;
};


// Set of PlotMSData, arranged in row/column.  Data can be shared among many
// cells (although access to this functionality is not yet implemented).
class PlotMSDatas {
public:
    PlotMSDatas(unsigned int rows = PlotMSOptions::DEFAULT_ROWS,
                unsigned int cols = PlotMSOptions::DEFAULT_COLS);
    
    ~PlotMSDatas();
    
    unsigned int nrows() const;
    unsigned int ncols() const;
    void setRowsCols(unsigned int rows, unsigned int cols,
                     bool clearData = true);    
    
    unsigned int size(unsigned int row = 0, unsigned int col = 0) const;
    
    PlotMSData& getData(unsigned int row = 0, unsigned int col = 0);
    
    void loadCache(VisSet& visSet, PMS::Axis xAxis, PMS::Axis yAxis,
                   unsigned int row = 0, unsigned int col = 0);
    
    //void loadCache(VisSet& visSet, const vector<PMS::Axis>& axes,
    //               unsigned int row = 0, unsigned int col = 0);
    
    void setupCache(PMS::Axis xAxis, PMS::Axis yAxis, unsigned int row = 0,
                    unsigned int col = 0);
    
    void locate(double xMin, double xMax, double yMin, double yMax,
                int row = 0, int col = 0);
    
private:
    unsigned int itsRows_;
    unsigned int itsCols_;
    vector<vector<PlotMSData*> > itsData_;
    
    void clearData();
};

}

#endif /* PLOTMSDATA_H_ */
