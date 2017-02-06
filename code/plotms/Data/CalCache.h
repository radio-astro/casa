//# CalCache.h: CalTable-specific casacore::Data cache for plotms.
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
#ifndef CALCACHE_H_
#define CALCACHE_H_

#include <plotms/Data/PlotMSCacheBase.h>

#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/PlotMS/PlotMSFlagging.h>
//#include <plotms/Threads/PlotMSCacheThread.qo.h>

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTIter.h>
#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>

namespace casa {

//# Forward declarations.
class PlotMSApp;
class PlotMSIndexer;

class CalCache : public PlotMSCacheBase {
    
  // Friend class declarations.
  friend class PlotMSIndexer;

public:    
  
  // Constructor which takes parent PlotMS.
  CalCache(PlotMSApp* parent);
  
  // Destructor
  virtual ~CalCache();

  // Identify myself
  PlotMSCacheBase::Type cacheType() const { return PlotMSCacheBase::CAL; };

  // Is the underlying table complex?
  inline casacore::Bool parsAreComplex() { return parsAreComplex_; };

  // Access to channel averaging bounds
  casacore::Matrix<casacore::Int>& chanAveBounds(casacore::Int spw) { return chanAveBounds_p(spw); };
  
  // ...not yet CAL-specific... (or ever?)
  // Set up indexing for the plot
  //  void setUpIndexer(PMS::Axis iteraxis=PMS::SCAN,
  //		    casacore::Bool globalXRange=false, casacore::Bool globalYRange=false);

  // Convert poln index->name and name->index
  virtual casacore::String polname(casacore::Int ipol);

  // given filename, get cal type
  void setFilename(casacore::String filename);

protected:

  // CAL-specific loadIt method
  virtual void loadIt(vector<PMS::Axis>& loadAxes,
		      vector<PMS::DataColumn>& loadData,
		      ThreadCommunication* thread = NULL);

private:
    
  // Forbid copy for now
  CalCache(const CalCache&);

  // Setup the CalIter
  void setUpCalIter(const casacore::String& calname,
		    PlotMSSelection& selection,
		    casacore::Bool readonly=true,
		    casacore::Bool chanselect=true,
		    casacore::Bool corrselect=true);

  // Count the chunks required in the cache
  void countChunks(ROCTIter& ci,
            vector<PMS::Axis>& loadAxes,
		    vector<PMS::DataColumn>& loadData,
            ThreadCommunication* thread);  // old

  // Trap attempt to use to much memory (too many points)
  //  void trapExcessVolume(map<PMS::Axis,casacore::Bool> pendingLoadAxes);

  // Loop over VisIter, filling the cache
  void loadCalChunks(ROCTIter& ci,
		  const vector<PMS::Axis> loadAxes,
		  ThreadCommunication* thread);

  // Loads the specific axis/metadata into the cache using the given VisBuffer.
  void loadCalAxis(ROCTIter& cti, casacore::Int chunk, PMS::Axis axis, casacore::String pol);

  // Check axis and slice param column appropriately
  casacore::Slice getParSlice(casacore::String axis, casacore::String polnSel);
  // Get axis string for VisCal Slice code
  casacore::String toVisCalAxis(PMS::Axis axis);

  // Check for divide-by-zero (=inf); set to 1.0 and flag it
  void checkRatioArray(casacore::Array<float>& array, int chunk);

  // Set flags in the CalTable
  virtual void flagToDisk(const PlotMSFlagging& flagging,
			  casacore::Vector<casacore::Int>& chunks, 
			  casacore::Vector<casacore::Int>& relids,
			  casacore::Bool flag,
			  PlotMSIndexer* indexer, int index);
  

  // A container for channel averaging bounds
  casacore::Vector<casacore::Matrix<casacore::Int> > chanAveBounds_p;

  // Provisional flagging helpers
  casacore::Vector<casacore::Int> nVBPerAve_;
 
  // The polarization basis
  casacore::String basis_;
  // Had to adjust for divide-by-zero in ratio plot (checkRatioArray)
  bool divZero_;

  // VisIterator pointer
  ROCTIter* ci_p;
  CTIter* wci_p;

  // Is parameter column complex?
  casacore::Bool parsAreComplex_;

  // Volume meter for volume calculation
  //  PMSCacheVolMeter vm_;

    
};
typedef casacore::CountedPtr<CalCache> CalCachePtr;


}

#endif /* CALCACHE_H_ */
