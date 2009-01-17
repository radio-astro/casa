//# PMSMemCache.h: Data cache for plotms.
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
#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/PlotMS/PlotMSConstants.h>

namespace casa {

class PMSMemCache {
  
public:
    
    static const PMS::Axis METADATA[];
    static const unsigned int N_METADATA;

  // Constructor
  PMSMemCache();
  
  // Destructor
  ~PMSMemCache();

  // Report the number of chunks
  Int nChunk() const { return nChunk_; };

  // Report the total number of points currently arranged for plotting
  //  (TBD: this is incorrect unless ALL cache spaces are full!!)
  Int nPoints() const { return nPoints_(nChunk_-1); };

  // Increase the number of chunks
  void increaseChunks(Int nc=0);

  // Fill a chunk with a VisBuffer
  void append(const VisBuffer& vb, Int vbnum, PMS::Axis xAxis,PMS::Axis yAxis);

  // Returns whether the cache is ready to return plotting data via the get
  // methods or not.
  bool readyForPlotting() const;
  
  // Set up indexing for the plot (amp vs freq hardwired version)
  void setUpPlot(PMS::Axis xAxis, PMS::Axis yAxis);

  // Get X and Y limits (amp vs freq hardwired version)
  void getRanges(Double& minX, Double& maxX, Double& minY, Double& maxY);

  // Get single values for x-y plotting (amp vs freq hardwired versions)
  Double getX(Int i);
  Double getY(Int i);
  void getXY(Int i, Double& x, Double& y);

  // Locate datum nearest to specified x,y (amp vs freq hardwired versions)
  void locateNearest(Double x, Double y);
  void locateRange(Double xmin,Double xmax,Double ymin,Double ymax);
  
  // Returns which axes have been loaded into the cache, including metadata.
  // Also includes the size (number of points) for each axis (which will
  // eventually be used for a cache manager to let the user know the
  // relative memory use of each axis).
  vector<pair<PMS::Axis, unsigned int> > loadedAxes() const;


private:
    
  // Forbid copy for now
  PMSMemCache(const PMSMemCache& mc);

  // Issue mete info report
  void reportMeta(Int ic,Int i);

  // Set currChunk_ according to a supplied index
  void setChunk(Int i);

  // Clean up the PtrBlocks
  void deleteCache();
  
  // Loads the specific axis/metadata into the cache using the given VisBuffer.
  void loadAxis(const VisBuffer& vb, Int vbnum, PMS::Axis axis);
  
  // Returns the number of points loaded for the given axis or 0 if not loaded.
  unsigned int nPointsForAxis(PMS::Axis axis) const;
  
  // Computes the X and Y limits for the currently set axes.  In the future we
  // may want to cache ALL ranges for all loaded values to avoid recomputation.
  void computeRanges();

  // Private data

  // The number of chunks
  Int nChunk_;

  // The in-focus chunk
  Int currChunk_;

  // The cumulative running total of points
  Vector<Int> nPoints_;

  Double minX_,maxX_,minY_,maxY_;

  // The fundamental meta-data cache
  Vector<Double> time_, timeIntr_;
  Vector<Int> field_, spw_, scan_;
  PtrBlock<Vector<Int>*> antenna1_, antenna2_;

  // Optional bits of the cache
  PtrBlock<Vector<Double>*> freq_;

  // Data (the heavy part)
  PtrBlock<Array<Float>*> amp_, pha_;
  PtrBlock<Array<Bool>*> flag_;
  PtrBlock<Vector<Bool>*> flagrow_;


  // Indexing help
  Int xdiv_,xmod_;
  
  
  // Current setup/state.
  bool dataLoaded_;
  bool currentSet_;
  PMS::Axis currentX_, currentY_;
  vector<PMS::Axis> loadedAxes_;  

};

}
