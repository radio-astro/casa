//# PlotMSIndexer.h: Cache indexer for plotms.
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
#ifndef PLOTMSINDEXER_H_
#define PLOTMSINDEXER_H_

#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>
#include <graphics/GenericPlotter/PlotData.h>

#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/Data/PlotMSCacheBase.h>

namespace casa {

//# Forward declarations.
class PlotMSApp;
class PlotMSIndexer;  // needed for method pointer typedefs

typedef Double(PlotMSCacheBase::*CacheMemPtr)(Int,Int);
typedef    Int(PlotMSIndexer::*IndexerMethPtr)(Int,Int);
typedef   void(PlotMSIndexer::*CollapseMethPtr)(Int,Array<Bool>&);
 
class PlotMSIndexer : public PlotMaskedPointData, public PlotBinnedData {

public:

  // Convenient access to class name.
  static const String CLASS_NAME;
  
  // A ctor that makes an empty Indexer (for plot initialization)
  PlotMSIndexer();
    
  // Constructor which takes parent PlotMSCache, x and y axes (non-iteration)
  PlotMSIndexer(PlotMSCacheBase* plotmscache, PMS::Axis xAxis, PMS::Axis yAxis);
  // Constructor which supports iteration
  PlotMSIndexer(PlotMSCacheBase* plotmscache, PMS::Axis xAxis, PMS::Axis yAxis,
                PMS::Axis iterAxis, Int iterValue);
  
  // Destructor
  ~PlotMSIndexer();

  // Implemented PlotData methods.
  // <group>
  bool willDeleteData() const { return true; }
  void setDeleteData(bool del = true)   { (void)del; }
  bool isValid() const { return true;};
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
  unsigned int sizeMasked() const { return sizeMasked_; }
  unsigned int sizeUnmasked() const { return sizeUnMasked_; }
  bool maskedMinsMaxes(double& xMin, double& xMax, double& yMin,double& yMax);
  bool unmaskedMinsMaxes(double& xMin, double& xMax, double& yMin,double& yMax);
  // </group>
    
  // PlotBinnedData methods
  // <group>
  unsigned int numBins() const;
  unsigned int binAt(unsigned int i) const;
  bool isBinned() const;
  // </group>

  // Set up indexing for the plot 
  void setUpIndexing();

  // Set global min/max flag
  void setGlobalMinMax(Bool globalX=False,Bool globalY=False) { 
    globalXMinMax_=globalX; globalYMinMax_=globalY; };

  // Report per-chunk point counters
  Vector<uInt> nPoints() { return nPoints_; };
  Vector<uInt> nCumulative() { return nCumulative_; };

  // Return if the indexer is ready (setUpPlot has been run)
  inline Bool indexerReady() const { return indexerReady_; };

  // Locate datum nearest to specified x,y
  Record getPointMetaData(Int i);
  Record locateInfo(const Vector<PlotRegion>& regions,
                    Bool showUnflagged, Bool showFlagged,
                    Bool selectAll = true);
  PlotLogMessage* locateRange(const Vector<PlotRegion>& regions,
			      Bool showUnflagged, Bool showFlagged);
  PlotLogMessage* flagRange(const PlotMSFlagging& flagging,
			    const Vector<PlotRegion>& regions, Bool flag = True);


  // Report meta info for current value of currChunk_/irel_
  void reportMeta(Double x, Double y, Bool masked, stringstream& ss);

  // Set flags in the cache
  void flagInCache(const PlotMSFlagging& flagging,Bool flag);

  // Iteration label
  String iterLabel();

  // Access to raw min/max data (no auto-global)
  bool maskedMinsMaxesRaw(double& xMin, double& xMax, double& yMin,double& yMax);
  bool unmaskedMinsMaxesRaw(double& xMin, double& xMax, double& yMin,double& yMax);


  // Directly implemented index calculators
  //  (generic index methods point to one of these depending upon axis choice)
  Int getIndex0000(Int ch,Int irel) { return 0;  (void)irel; (void)ch; };
  Int getIndex1000(Int ch,Int irel) { return irel%icorrmax_(ch);};
  Int getIndex0100(Int ch,Int irel) { return (irel/nperchan_(ch))%ichanmax_(ch);};
  Int getIndex0010(Int ch,Int irel) { return (irel/nperbsln_(ch))%ibslnmax_(ch);};
  Int getIndex0110(Int ch,Int irel) { return (irel/nperchan_(ch))%ichanbslnmax_(ch);};
  Int getIndex1010(Int ch,Int irel) { return (irel/nperbsln_(ch))*nperchan_(ch) + irel%nperchan_(ch);};
  Int getIndex1110(Int ch,Int irel) { return irel%idatamax_(ch);};
  Int getIndex0001(Int ch,Int irel) { return (irel/nperant_(ch))%iantmax_(ch);};


  // 
  bool colorize(bool doColorize, PMS::Axis colorizeAxis);


  

private:
    
  // Forbid copy for now
  PlotMSIndexer(const PlotMSIndexer& mc);

  void setMethod(CacheMemPtr& getmethod, PMS::Axis axis);
  void setIndexer(IndexerMethPtr& indexmethod, PMS::Axis axis);
  //  void setCollapser(CollapseMethPtr& collmethod, PMS::Axis axis);

  // Generate collapsed versions of the plmask 
  /* not needed?  (gmoellen, 2011Mar15)
  void collapseMask0000(Int ch,Array<Bool>& collmask);
  void collapseMask1000(Int ch,Array<Bool>& collmask);
  void collapseMask0100(Int ch,Array<Bool>& collmask);
  void collapseMask0010(Int ch,Array<Bool>& collmask);
  void collapseMask0110(Int ch,Array<Bool>& collmask);
  void collapseMask1010(Int ch,Array<Bool>& collmask);
  void collapseMask1110(Int ch,Array<Bool>& collmask);
  void collapseMask0001(Int ch,Array<Bool>& collmask);
  */

  // Report the number of chunks
  Int nChunk() const { return (plotmscache_ ? plotmscache_->nChunk() : 0); };

  // Report the reference time for this cache (in seconds)
  inline Double refTime() { return plotmscache_->refTime(); };

  // Set currChunk_ according to a supplied index
  void setChunk(uInt i) const;

  // Computes the X and Y limits for the currently set axes.  In the future we
  // may want to cache ALL ranges for all loaded values to avoid recomputation.
  void computeRanges();
  
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
  PlotMSCacheBase* plotmscache_;

  // Pointers to methods for axis flexibility
  CacheMemPtr getXFromCache_, getYFromCache_,getColFromCache_;
  IndexerMethPtr XIndexer_, YIndexer_, ColIndexer_;
  //  CollapseMethPtr collapseXMask_, collapseYMask_;

  // The in-focus chunk and relative index offset
  mutable Int currChunk_, irel_;
  mutable uInt lasti_;

  // The number of points per chunk
  Vector<uInt> nPoints_;

  // The cumulative running total of points
  Vector<uInt> nCumulative_;

  // Segment point-counting Vectors
  Int nSegment_;
  mutable Int currSeg_;
  Vector<uInt> nSegPoints_,nCumulPoints_,cacheChunk_,cacheOffset_;
  
  // Current setup/state.
  PMS::Axis currentX_, currentY_;
  bool indexerReady_;

  // Indexing parameters
  Vector<Int> icorrmax_, ichanmax_, ibslnmax_, idatamax_;
  Vector<Int> nperchan_, nperbsln_, nperant_;
  Vector<Int> ichanbslnmax_;
  Vector<Int> iantmax_;

  // Nominal axes limits
  Double xmin_,ymin_,xflmin_,yflmin_,xmax_,ymax_,xflmax_,yflmax_;
  Int sizeMasked_, sizeUnMasked_;
  Bool globalXMinMax_,globalYMinMax_;

  // Iteration
  // <group>
  Bool iterate_;
  PMS::Axis iterAxis_;
  Int iterValue_;
  // </group>

  // Colorization
  // <group>
  bool itsColorize_;
  PMS::Axis itsColorizeAxis_;
  // </group>
  
  // Cope with const-ness in the get methods
  PlotMSIndexer* self;

};

typedef CountedPtr<PlotMSIndexer> PlotMSIndexerPtr;

}

#endif /* PLOTMSCACHEINDER_H_ */
