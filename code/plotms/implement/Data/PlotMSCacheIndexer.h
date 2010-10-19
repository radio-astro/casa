//# PlotMSCache.h: Data cache for plotms.
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
#ifndef PLOTMSCACHEINDEXER_H_
#define PLOTMSCACHEINDEXER_H_

#include <plotms/PlotMS/PlotMSConstants.h>

#include <plotms/Data/PlotMSCache.h>
#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>

namespace casa {

//# Forward declarations.
class PlotMS;
class PlotMSCacheIndexer;  // needed for method pointer typedefs

typedef Double(PlotMSCache::*PlotMSCacheMemPtr)(Int,Int);
typedef    Int(PlotMSCacheIndexer::*IndexerMethodPtr)(Int,Int);
typedef   void(PlotMSCacheIndexer::*CollapseMethodPtr)(Int,Array<Bool>&);
 
class PlotMSCacheIndexer {

public:
    // Convenient access to class name.
    static const String CLASS_NAME;
    
  // Constructor which takes parent PlotMS.
  PlotMSCacheIndexer(PlotMSCache* plotmscache);
  
  // Destructor
  ~PlotMSCacheIndexer();

  // Set up indexing for the plot 
  void setUpPlot(PMS::Axis xAxis, PMS::Axis yAxis);

  // Report the total number of points currently arranged for plotting
  //  (TBD: this is incorrect unless ALL cache spaces are full!!)
  Int nPoints() const { return (nPoints_.nelements() > 0) ? nPoints_(nChunk()-1) : 0; };

  // CAche shapes:
  Matrix<Int>& chunkShapes() { return plotmscache_->chunkShapes(); };

  // Return if the indexer is ready (setUpPlot has been run)
  inline Bool indexerReady() const { return indexerReady_; };

  // Get X and Y limits 
  // TBD: flagged/unflagged
  void getRanges(Double& minX, Double& maxX, Double& minY, Double& maxY);

  // Get single values for x-y plotting by global index
  Double getX(Int i);
  Double getY(Int i);
  void getXY(Int i, Double& x, Double& y);
  Bool getFlagMask(Int i);

  // Get single values by chunk and relative index  
  //  (implicitly requires that get?FromCache_ method pointers are set)
  Double getX(Int ch,Int irel) { return ((*plotmscache_).*getXFromCache_)(ch,irel); };
  Double getY(Int ch,Int irel) { return ((*plotmscache_).*getYFromCache_)(ch,irel); };

  // Locate datum nearest to specified x,y (amp vs freq hardwired versions)
  PlotLogMessage* locateRange(const Vector<PlotRegion>& regions);
  PlotLogMessage* flagRange(const PlotMSFlagging& flagging,
			    const Vector<PlotRegion>& regions, Bool flag = True);


  // Report meta info for current value of currChunk_/irel_
  void reportMeta(Double x, Double y,stringstream& ss);

  // Set flags in the cache
  void flagInCache(const PlotMSFlagging& flagging,Bool flag);

  // Sets the plot mask for a single chunk
  void setPlotMask(Int chunk);

  // Clean up the plot mask
  void deleteMask();

  // Set flags in the MS
  void flagInVisSet(const PlotMSFlagging& flagging,Vector<Int>& chunks, Vector<Int>& relids,Bool flag);


private:
    
  // Forbid copy for now
  PlotMSCacheIndexer(const PlotMSCacheIndexer& mc);

  void getAxesMask(PMS::Axis axis,Vector<Bool>& axismask);

  void setMethod(PlotMSCacheMemPtr& getmethod, PMS::Axis axis);
  void setIndexer(IndexerMethodPtr& indexmethod, PMS::Axis axis);
  void setCollapser(CollapseMethodPtr& collmethod, PMS::Axis axis);

  // Directly implemented index calculators
  //  (generic index methods point to one of these depending upon axis choice)
  Int getIndex0000(Int ch,Int irel) { return 0;  (void)irel; (void)ch; };
  Int getIndex1000(Int ch,Int irel) { return irel%icorrmax_(ch);  (void)irel; };
  Int getIndex0100(Int ch,Int irel) { return (irel/nperchan_(ch))%ichanmax_(ch);  (void)irel; };
  Int getIndex0010(Int ch,Int irel) { return (irel/nperbsln_(ch))%ibslnmax_(ch);  (void)irel; };
  Int getIndex0110(Int ch,Int irel) { return (irel/nperchan_(ch))%ichanbslnmax_(ch);  (void)irel; };
  Int getIndex1110(Int ch,Int irel) { return irel%idatamax_(ch);  (void)irel; };
  Int getIndex0001(Int ch,Int irel) { return (irel/nperant_(ch))%iantmax_(ch);  (void)irel; };
  //  TBD:  corr/bsln-dep (e.g., WEIGHT)
  //  inline Int getIndex1010(Int ch,Int irel) { return 
  
  // Generate collapsed versions of the plmask 
  void collapseMask0000(Int ch,Array<Bool>& collmask);
  void collapseMask1000(Int ch,Array<Bool>& collmask);
  void collapseMask0100(Int ch,Array<Bool>& collmask);
  void collapseMask0010(Int ch,Array<Bool>& collmask);
  void collapseMask0110(Int ch,Array<Bool>& collmask);
  void collapseMask1110(Int ch,Array<Bool>& collmask);
  void collapseMask0001(Int ch,Array<Bool>& collmask);
  //  void collapseMask1010(Int ch,Array<Bool>& collmask);

  // Report the number of chunks
  Int nChunk() const { return plotmscache_->nChunk(); };

  // Report the reference time for this cache (in seconds)
  inline Double refTime() { return plotmscache_->refTime(); };

  // Set currChunk_ according to a supplied index
  void setChunk(Int i);

  // Computes the X and Y limits for the currently set axes.  In the future we
  // may want to cache ALL ranges for all loaded values to avoid recomputation.
  void computeRanges();
  
  // Returns the number of points loaded for the given axis or 0 if not loaded.
  unsigned int nPointsForAxis(PMS::Axis axis) const;
  
  // Convenience methods that call log() with the given method name and the
  // appropriate event type.
  // <group>
  void logInfo(const String& method, const String& message) {
      log(method, message, PlotLogger::MSG_INFO); }
  void logDebug(const String& method, const String& message) {
      log(method, message, PlotLogger::MSG_DEBUG); }
  void logWarn(const String& method, const String& message) {
      log(method, message, PlotLogger::MSG_WARN); }
  void logError(const String& method, const String& message) {
      log(method, message, PlotLogger::MSG_ERROR); }
  // </group>
  
  // Logs the given message from the given method name as the given event type
  // (see PlotLogger).
  void log(const String& method, const String& message, int eventType);

  // Private data
   
  // Parent plotms.
  PlotMSCache* plotmscache_;

  // Pointers to methods for axis flexibility
  PlotMSCacheMemPtr getXFromCache_, getYFromCache_;
  IndexerMethodPtr XIndexer_, YIndexer_;
  CollapseMethodPtr collapseXMask_, collapseYMask_;

  // The in-focus chunk and relative index offset
  Int currChunk_, irel_, lasti_;

  // The cumulative running total of points
  Vector<Int> nPoints_;

  // Current setup/state.
  PMS::Axis currentX_, currentY_;
  bool indexerReady_;

  // Axes mask
  Vector<Bool> netAxesMask_;

  // Indexing parameters
  Vector<Int> icorrmax_, ichanmax_, ibslnmax_, idatamax_;
  Vector<Int> nperchan_, nperbsln_, nperant_;
  Vector<Int> ichanbslnmax_;
  Vector<Int> iantmax_;

  // collapsed flag mask for plotting
  PtrBlock<Array<Bool>*> plmask_;

  // Nominal axes limits
  Double xmin_,ymin_,xflmin_,yflmin_,xmax_,ymax_,xflmax_,yflmax_;


};

typedef CountedPtr<PlotMSCacheIndexer> PlotMSCacheIndexerPtr;

}

#endif /* PLOTMSCACHEINDER_H_ */
