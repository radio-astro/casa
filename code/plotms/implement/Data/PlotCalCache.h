//# PlotCalCache.h: Calibration cache for plotms.
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
#ifndef PLOTCALCACHE_H_
#define PLOTCALCACHE_H_

#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/PlotMS/PlotMSFlagging.h>
#include <plotms/Actions/PlotMSCacheThread.qo.h>
#include <plotms/Data/PlotMSVBAverager.h>

#include <plotms/Data/PlotMSCache.h>

#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>

namespace casa {

//# Forward declarations.
class PlotMS;


class PlotCalCache : public PlotMSCache {
    
  // Friend class declarations.
  friend class PlotMSData;
  
public:    

  // Constructor which takes parent PlotMS.
  PlotCalCache(PlotMS* parent);
  
  // Destructor
  virtual ~PlotCalCache();

  // Identify myself
  PlotMSCache::Type cacheType() { return PlotMSCache::CAL; };
  
  
  // Loads the cache for the given axes and data
  // columns.  IMPORTANT: this method assumes that any currently loaded data is
  // valid for the given VisIter; i.e., if the meta-information or either of
  // the axes are already loaded, then they don't need to be reloaded.  If this
  // is not the case, then clear() should be called BEFORE append().  If a
  // PlotMSCacheThreadHelper object is given, it will be used to report
  // progress information.
  virtual void load(const vector<PMS::Axis>& axes,
		    const vector<PMS::DataColumn>& data,
		    const String& filename,
		    const PlotMSSelection& selection,
		    const PlotMSAveraging& averaging,
		    const PlotMSTransformations& transformations,
		    PlotMSCacheThread* thread = NULL);

  // Set flags in the Cal table
  virtual void flagToDisk(const PlotMSFlagging& flagging,
			  Vector<Int>& chunks, 
			  Vector<Int>& relids,
			  Bool flag);

private:
    
  // Forbid copy for now
  PlotCalCache(const PlotCalCache& mc);

  // Manage the cal table
  void openCal(const String& msname,
	       const PlotMSSelection& selection,
	       Bool readonly=True);
  void closeCal();


  // Select on the caltable (via MSSelection semantics)
  void selectCal(const PlotMSSelection& selection);

  // Count the chunks required in the cache
  void countCalChunks();  // old

  // Iterate over selected cal table, filling the cache
  void loadCalChunks(const vector<PMS::Axis> loadAxes,
		     const vector<PMS::DataColumn> loadData,
		     PlotMSCacheThread* thread);


  // Issue meta info report to the given stringstream.
  void reportMeta(Double x, Double y, stringstream& ss);

  // Loads the specific axis/metadata into the cache using the given VisBuffer.
  void loadCalAxis(Table& tab, Int vbnum, PMS::Axis axis);

  // Support MS-oriented selection
  Vector<Int> getAntIdx(const String& antenna);
  Vector<Int> getFieldIdx(const String& field);
  Vector<Int> getSpwIdx(const String& spw);
  Vector<Int> getCalDescIds(const Vector<Int> selspws);
  Matrix<Double> getTimeRange(const String& time);
  
  // Manage freq info we get from the MS
  void getFrequencyInfo();
  void deleteFreqInfo();

  // Private data

  // The name of the caltable, associated MS
  String calname_, msname_;

  // Spw index, nchan, startchan for each CAL_DESC_ID index
  Int nCDI_;
  Vector<Int> cdi_;
  Vector<Int> spwFromCDI_;
  Vector<Int> nchanFromCDI_;
  Vector<Int> startFromCDI_;

  PtrBlock<Vector<Double>*>freqFromCDI_;

  // Transient Table objects used when loading/flagging
  Table *tab_p, *seltab_p;
  TableIterator *iter_p;
    
};
typedef CountedPtr<PlotCalCache> PlotCalCachePtr;


}

#endif /* PLOTMSCACHE_H_ */
