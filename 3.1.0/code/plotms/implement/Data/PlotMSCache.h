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
#ifndef PLOTMSCACHE_H_
#define PLOTMSCACHE_H_

#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/PlotMS/PlotMSFlagging.h>
#include <plotms/Actions/PlotMSCacheThread.qo.h>
#include <plotms/Data/PlotMSVBAverager.h>

#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>
#include <msvis/MSVis/VisIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBufferUtil.h>

namespace casa {

//# Forward declarations.
class PlotMS;


class PlotMSCache {
    
    // Friend class declarations.
    friend class PlotMSData;
  
public:    

    // Varieties of cache
    enum Type {MS, CAL};

    static const PMS::Axis METADATA[];
    static const unsigned int N_METADATA;
    
    static bool axisIsMetaData(PMS::Axis axis);
    
    static const unsigned int THREAD_SEGMENT;

  // Constructor which takes parent PlotMS.
  PlotMSCache(PlotMS* parent);
  
  // Destructor
  virtual ~PlotMSCache();

  // Identify myself
  PlotMSCache::Type cacheType() { return PlotMSCache::MS; };

  // Report the number of chunks
  Int nChunk() const { return nChunk_; };

  // Report the data shapes
  inline Matrix<Int>& chunkShapes() {return chshapes_;};


  // Report the total number of points currently arranged for plotting
  //  (TBD: this is incorrect unless ALL cache spaces are full!!)
  Int nPoints() const { return nPoints_(nChunk_-1); };

  // Report the reference time for this cache (in seconds)
  inline Double refTime() { return refTime_p; };

  // Clears the cache of all stored values.  This should be called when the
  // underlying MS or MS selection is changed, thus invalidating stored data.
  void clear();
  
  // Loads the cache for the given axes and data
  // columns.  IMPORTANT: this method assumes that any currently loaded data is
  // valid for the given VisIter; i.e., if the meta-information or either of
  // the axes are already loaded, then they don't need to be reloaded.  If this
  // is not the case, then clear() should be called BEFORE append().  If a
  // PlotMSCacheThreadHelper object is given, it will be used to report
  // progress information.
  virtual void load(const vector<PMS::Axis>& axes,
		    const vector<PMS::DataColumn>& data,
		    const String& msname,
		    const PlotMSSelection& selection,
		    const PlotMSAveraging& averaging,
		    const PlotMSTransformations& transformations,
		    PlotMSCacheThread* thread = NULL);

  // Convenience method for loading x and y axes.
  void load(PMS::Axis xAxis, PMS::Axis yAxis,
            PMS::DataColumn xData, PMS::DataColumn yData,
            const String& msname,
            const PlotMSSelection& selection,
            const PlotMSAveraging& averaging,
            const PlotMSTransformations& transformations,
            PlotMSCacheThread* thread = NULL) {

    cout << "AHHHHHHHHHHHH*********************" << endl;

      vector<PMS::Axis> axes(2);
      axes[0] = xAxis; axes[1] = yAxis;
      vector<PMS::DataColumn> data(2);
      data[0] = xData; data[1] = yData;
      load(axes, data, msname, selection, averaging,transformations, thread);
  }
  
  // Releases the given axes from the cache.
  void release(const vector<PMS::Axis>& axes);
  
  // Set up indexing for the plot (amp vs freq hardwired version)
  void setUpPlot(PMS::Axis xAxis, PMS::Axis yAxis);
  void getAxesMask(PMS::Axis axis,Vector<Bool>& axismask);
  
  // Returns whether cache is filled
  bool cacheReady() const { return dataLoaded_; }

  // Returns whether the cache is filled AND ready to return plotting 
  //  data via the get methods or not.
  bool readyForPlotting() const { return cacheReady() && axesSet_; };

  // Get X and Y limits (amp vs freq hardwired version)
  void getRanges(Double& minX, Double& maxX, Double& minY, Double& maxY);

  // Get single values for x-y plotting 
  Double getX(Int i);
  Double getY(Int i);
  void getXY(Int i, Double& x, Double& y);
  Double get(PMS::Axis axis);

  Bool getFlagMask(Int i);

  // Access to flags per chunk
  inline Array<Bool>& flag(Int chunk) { return *flag_[chunk]; };


  // Axis-specific generic gets
  inline Double getScan(Int chnk,Int irel)     { return scan_(chnk);   (void)irel; };
  inline Double getField(Int chnk,Int irel)    { return field_(chnk);  (void)irel; };
  inline Double getTime(Int chnk,Int irel)     { return time_(chnk);  (void)irel; };
  inline Double getTimeIntr(Int chnk,Int irel) { return timeIntr_(chnk);  (void)irel; };
  inline Double getSpw(Int chnk,Int irel)      { return spw_(chnk);  (void)irel; };

  inline Double getFreq(Int chnk,Int irel) { return *(freq_[chnk]->data()+irel); };
  inline Double getVel(Int chnk,Int irel)  { return *(vel_[chnk]->data()+irel); };
  inline Double getChan(Int chnk,Int irel) { return *(chan_[chnk]->data()+irel); };
  inline Double getCorr(Int chnk,Int irel) { return *(corr_[chnk]->data()+irel); };
  inline Double getAnt1(Int chnk,Int irel) { return *(antenna1_[chnk]->data()+irel); };
  inline Double getAnt2(Int chnk,Int irel) { return *(antenna2_[chnk]->data()+irel); };
  inline Double getBsln(Int chnk,Int irel) { return *(baseline_[chnk]->data()+irel); };
  inline Double getUVDist(Int chnk,Int irel) { return *(uvdist_[chnk]->data()+irel); };
  inline Double getUVDistL(Int chnk,Int irel) { return *(uvdistL_[chnk]->data()+irel); };
  inline Double getU(Int chnk,Int irel) { return *(u_[chnk]->data()+irel); };
  inline Double getV(Int chnk,Int irel) { return *(v_[chnk]->data()+irel); };
  inline Double getW(Int chnk,Int irel) { return *(w_[chnk]->data()+irel); };

  inline Double getAmp(Int chnk,Int irel)  { return *(amp_[chnk]->data()+irel); };
  inline Double getPha(Int chnk,Int irel)  { return *(pha_[chnk]->data()+irel); };
  inline Double getReal(Int chnk,Int irel) { return *(real_[chnk]->data()+irel); };
  inline Double getImag(Int chnk,Int irel) { return *(imag_[chnk]->data()+irel); };

  inline Double getFlag(Int chnk,Int irel) { return *(flag_[chnk]->data()+irel); };
  inline Double getFlagRow(Int chnk,Int irel) { return *(flagrow_[chnk]->data()+irel); };
  inline Double getRow(Int chnk,Int irel) { return *(row_[chnk]->data()+irel); };

  // These are array-global (one value per chunk)
  inline Double getAz0(Int chnk,Int irel) { return az0_(chnk);  (void)irel; };
  inline Double getEl0(Int chnk,Int irel) { return el0_(chnk);  (void)irel; };
  inline Double getHA0(Int chnk,Int irel) { return ha0_(chnk);  (void)irel; };
  inline Double getPA0(Int chnk,Int irel) { return pa0_(chnk);  (void)irel; };

  // These are antenna-based
  inline Double getAntenna(Int chnk,Int irel) { return *(antenna_[chnk]->data()+irel); };
  inline Double getAz(Int chnk,Int irel)      { return *(az_[chnk]->data()+irel); };
  inline Double getEl(Int chnk,Int irel)      { return *(el_[chnk]->data()+irel); };
  inline Double getParAng(Int chnk,Int irel)  { return *(parang_[chnk]->data()+irel); };

  // Axis-specific gets
  inline Double getScan()      { return scan_(currChunk_); };
  inline Double getField()     { return field_(currChunk_); };
  inline Double getTime()      { return time_(currChunk_); };
  inline Double getTimeIntr()  { return timeIntr_(currChunk_); };
  inline Double getSpw()       { return spw_(currChunk_); };
  inline Double getFreq() { return *(freq_[currChunk_]->data()+(irel_/nperchan_(currChunk_))%ichanmax_(currChunk_)); };
  inline Double getVel() { return *(vel_[currChunk_]->data()+(irel_/nperchan_(currChunk_))%ichanmax_(currChunk_)); };
  inline Double getChan() { return *(chan_[currChunk_]->data()+(irel_/nperchan_(currChunk_))%ichanmax_(currChunk_)); };
  inline Double getCorr() { return *(corr_[currChunk_]->data()+(irel_%icorrmax_(currChunk_))); };
  inline Double getAnt1() { return *(antenna1_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getAnt2() { return *(antenna2_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getBsln() { return *(baseline_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getUVDist() { return *(uvdist_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getUVDistL() { return *(uvdistL_[currChunk_]->data()+(irel_/nperchan_(currChunk_))%ichanbslnmax_(currChunk_)); };
  inline Double getU() { return *(u_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getV() { return *(v_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getW() { return *(w_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };

  inline Double getAmp()  { return *(amp_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); }; 
  inline Double getPha()  { return  *(pha_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); };
  inline Double getReal() { return *(real_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); };
  //  inline Double getImag() { return *(imag_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); };
  inline Double getImag() { return getImag2(currChunk_,(irel_%idatamax_(currChunk_))); };
  inline Double getImag2(Int ch, Int i) { return *(imag_[ch]->data()+i); };

  inline Double getFlag() { return *(flag_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); };
  inline Double getFlagRow() { return *(flagrow_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getRow() { return *(row_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };

  inline Double getWt() { return *(wt_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))*nperchan_(currChunk_) + irel_%nperchan_(currChunk_)); };

  // These are array-global (one value per chunk):
  inline Double getAz0() { return az0_(currChunk_); };
  inline Double getEl0() { return el0_(currChunk_); };
  inline Double getHA0() { return ha0_(currChunk_); };
  inline Double getPA0() { return pa0_(currChunk_); };

  // These are antenna-based
  inline Double getAntenna() { return *(antenna_[currChunk_]->data()+(irel_/nperant_(currChunk_))%iantmax_(currChunk_)); };
  inline Double getAz() { return *(az_[currChunk_]->data()+(irel_/nperant_(currChunk_))%iantmax_(currChunk_)); };
  inline Double getEl() { return *(el_[currChunk_]->data()+(irel_/nperant_(currChunk_))%iantmax_(currChunk_)); };
  inline Double getParAng() { return *(parang_[currChunk_]->data()+(irel_/nperant_(currChunk_))%iantmax_(currChunk_)); };

  // Locate datum nearest to specified x,y (amp vs freq hardwired versions)
  PlotLogMessage* locateNearest(Double x, Double y);
  PlotLogMessage* locateRange(const Vector<PlotRegion>& regions);
  PlotLogMessage* flagRange(const PlotMSFlagging& flagging,
          const Vector<PlotRegion>& regions, Bool flag = True);

  // Access to averaging state in the cache:
  PlotMSAveraging& averaging() { return averaging_; }

  // Access to transformations state in the cache
  PlotMSTransformations& transformations() { return transformations_; }

  // Access to channel averaging bounds
  Matrix<Int>& chanAveBounds(Int spw) { return chanAveBounds_p(spw); };

  // Set flags in the cache
  void flagInCache(const PlotMSFlagging& flagging,Bool flag);

  // Sets the plot mask for a single chunk
  void setPlotMask(Int chunk);

  // Set flags in the MS
  virtual void flagToDisk(const PlotMSFlagging& flagging,Vector<Int>& chunks, Vector<Int>& relids,Bool flag);

  // Returns which axes have been loaded into the cache, including metadata.
  // Also includes the size (number of points) for each axis (which will
  // eventually be used for a cache manager to let the user know the
  // relative memory use of each axis).
  vector<pair<PMS::Axis, unsigned int> > loadedAxes() const;

protected:
    
  // Forbid copy for now
  PlotMSCache(const PlotMSCache& mc);

  // Increase the number of chunks
  void increaseChunks(Int nc=0);

  // Setup the VisIter
  void setUpVisIter(const String& msname,
		    const PlotMSSelection& selection,
		    Bool readonly=True,
		    Bool chanselect=True,
		    Bool corrselect=True);


  // Loop over VisIter, filling the cache
  void loadChunks(ROVisibilityIterator& vi,
		  const vector<PMS::Axis> loadAxes,
		  const vector<PMS::DataColumn> loadData,
		  const PlotMSAveraging& averaging,
		  PlotMSCacheThread* thread);
  void loadChunks(ROVisibilityIterator& vi,
		  const PlotMSAveraging& averaging,
		  const Vector<Int>& nIterPerAve,
		  const vector<PMS::Axis> loadAxes,
		  const vector<PMS::DataColumn> loadData,
		  PlotMSCacheThread* thread);

  // Force read on vb for requested axes 
  //   (so pre-cache averaging treats all data it should)
  void forceVBread(VisBuffer& vb,
		   vector<PMS::Axis> loadAxes,
		   vector<PMS::DataColumn> loadData);

  // Tell time averager which data column to read
  void discernData(vector<PMS::Axis> loadAxes,
		   vector<PMS::DataColumn> loadData,
		   PlotMSVBAverager& vba);


  // Count the chunks required in the cache
  void countChunks(ROVisibilityIterator& vi,PlotMSCacheThread* thread);  // old
  void countChunks(ROVisibilityIterator& vi, Vector<Int>& nIterPerAve,  // supports time-averaging 
		   const PlotMSAveraging& averaging,PlotMSCacheThread* thread);

  // Fill a chunk with a VisBuffer.  
  void append(const VisBuffer& vb, Int vbnum, PMS::Axis xAxis, PMS::Axis yAxis,
              PMS::DataColumn xData, PMS::DataColumn yData);
  
  // Issue meta info report to the given stringstream.
  void reportMeta(Double x, Double y, stringstream& ss);

  // Set currChunk_ according to a supplied index
  void setChunk(Int i);

  // Clean up the PtrBlocks
  void deleteCache();
  
  // Loads the specific axis/metadata into the cache using the given VisBuffer.
  void loadAxis(VisBuffer& vb, Int vbnum, PMS::Axis axis,
		PMS::DataColumn data = PMS::DEFAULT_DATACOLUMN);
  
  // Returns the number of points loaded for the given axis or 0 if not loaded.
  unsigned int nPointsForAxis(PMS::Axis axis) const;
  
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
  
  void logLoad(const String& message) {
      log(PMS::LOG_ORIGIN_LOAD_CACHE, message, PMS::LOG_EVENT_LOAD_CACHE); }
  void logFlag(const String& message) {
      log(PMS::LOG_ORIGIN_FLAG, message, PMS::LOG_EVENT_FLAG); }
  // </group>
  
  // Logs the given message from the given method name as the given event type
  // (see PlotLogger).
  void log(const String& method, const String& message, int eventType);

  // Private data
  
  // Parent plotms.
  PlotMS* plotms_;

  // The number of antennas
  Int nAnt_;

  // The number of chunks
  Int nChunk_;

  // The in-focus chunk and relative index offset
  Int currChunk_, irel_;

  // The cumulative running total of points
  Vector<Int> nPoints_;

  // the net number of flagged, unflagged, total points
  Int nTotalPoints_,nUnFlagPoints_, nFlagPoints_;

  // The reference time for this cache, in seconds
  Double refTime_p;

  // Axes mask
  Vector<Bool> netAxesMask_;

  Double minX_,maxX_,minY_,maxY_;

  // The fundamental meta-data cache
  Matrix<Int> chshapes_;
  Vector<Double> time_, timeIntr_;
  Vector<Int> field_, spw_, scan_;
  PtrBlock<Vector<uInt>*> row_;
  PtrBlock<Vector<Int>*> antenna1_, antenna2_, baseline_;
  PtrBlock<Vector<Double>*> uvdist_, u_, v_, w_;
  PtrBlock<Matrix<Double>*> uvdistL_;
  PtrBlock<Vector<Double>*> freq_, vel_;
  PtrBlock<Vector<Int>*> chan_;
  PtrBlock<Vector<Int>*> corr_;

  // Optional parts of the cache
  PtrBlock<Vector<Float>*> pa_;

  // Data (the heavy part)
  PtrBlock<Array<Float>*> amp_, pha_, real_, imag_;
  PtrBlock<Array<Bool>*> flag_;
  PtrBlock<Vector<Bool>*> flagrow_;
  
  PtrBlock<Array<Float>*> wt_;

  PtrBlock<Array<Bool>*> plmask_;

  PtrBlock<Vector<Float>*> parang_;
  PtrBlock<Vector<Int>*> antenna_;
  PtrBlock<Vector<Double>*> az_,el_;

  Vector<Double> az0_,el0_,ha0_,pa0_;

  // Indexing help
  Vector<Int> icorrmax_, ichanmax_, ibslnmax_, idatamax_;
  Vector<Int> nperchan_, nperbsln_, nperant_;
  Vector<Int> ichanbslnmax_;
  Vector<Int> iantmax_;

  // Current setup/state.
  bool dataLoaded_;
  bool axesSet_;
  PMS::Axis currentX_, currentY_;
  map<PMS::Axis, bool> loadedAxes_;
  map<PMS::Axis, PMS::DataColumn> loadedAxesData_;

  // A copy of the Data parameters 
  String msname_;
  PlotMSSelection selection_;
  PlotMSAveraging averaging_;
  PlotMSTransformations transformations_;

  // meta info for locate output
  Vector<String> antnames_; 	 
  Vector<String> fldnames_; 	 

  // A container for channel averaging bounds
  Vector<Matrix<Int> > chanAveBounds_p;

  // Provisional flagging helpers
  Vector<Int> nVBPerAve_;

  // VisIterator pointer
  ROVisIterator* rvi_p;
  VisIterator* wvi_p;

  // VisBufferUtil for freq/vel calculations
  VisBufferUtil vbu_;

    
};
typedef CountedPtr<PlotMSCache> PlotMSCachePtr;


}

#endif /* PLOTMSCACHE_H_ */
