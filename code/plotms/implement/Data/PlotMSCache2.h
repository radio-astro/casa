//# PlotMSCache2.h: Data cache for plotms.
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
#ifndef PLOTMSCACHE2_H_
#define PLOTMSCACHE2_H_

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
class PlotMSApp;
class PlotMSIndexer;

class PMSCacheVolMeter {

public:

  // Constructor/Destructor
  PMSCacheVolMeter();
  PMSCacheVolMeter(const MeasurementSet& ms, const PlotMSAveraging ave);
  ~PMSCacheVolMeter();

  // reset (as if default ctor was run)
  void reset();

  // add in via a VisBuffer
  void add(const VisBuffer& vb);

  // add in via counts
  void add(Int DDID,Int nRows);

  // evaluate the volume for specified axes, and complain if 
  String evalVolume(map<PMS::Axis,Bool> axes,Vector<Bool> axesmask);

private:

  // The number of DATA_DESCRIPTIONs
  Int nDDID_;

  // Counters
  Vector<uInt64> nPerDDID_,nRowsPerDDID_,nChanPerDDID_,nCorrPerDDID_;

  // The number of antennas (max)
  Int nAnt_;

};

class PlotMSCache2 {
    
    // Friend class declarations.
    friend class PlotMSData;
    friend class PlotMSIndexer;

public:    

    // Varieties of cache
    enum Type {MS, CAL};

    static const PMS::Axis METADATA[];
    static const unsigned int N_METADATA;
    
    static bool axisIsMetaData(PMS::Axis axis);
    
    static const unsigned int THREAD_SEGMENT;

  // Constructor which takes parent PlotMS.
  PlotMSCache2(PlotMSApp* parent);
  
  // Destructor
  virtual ~PlotMSCache2();

  // Identify myself
  PlotMSCache2::Type cacheType() { return PlotMSCache2::MS; };

  // Reference an indexer
  Int nIter() { return indexer_.nelements(); };
  PlotMSIndexer& indexer(uInt i) {return (*indexer_[i]);};

  // Report the number of chunks
  Int nChunk() const { return nChunk_; };

  // Returns whether cache is filled
  bool cacheReady() const { return dataLoaded_; }

  // Report the data shapes
  inline Matrix<Int>& chunkShapes() {return chshapes_;};

  // A chunk is good (T) if it contains data
  //  (when averaging, some chunks may have nrows=0)
  inline Bool goodChunk(Int ichunk) {return goodChunk_(ichunk); };

  // Is there a reference value for the specified axis?
  inline bool hasReferenceValue(PMS::Axis axis) { return (axis==PMS::TIME && cacheReady()); };
  inline double referenceValue(PMS::Axis axis) { return (hasReferenceValue(axis) ? refTime() : 0.0); };
  
  // Report the reference time for this cache (in seconds)
  inline Double refTime() const { return refTime_p; };

  // Returns which axes have been loaded into the cache, including metadata.
  // Also includes the size (number of points) for each axis (which will
  // eventually be used for a cache manager to let the user know the
  // relative memory use of each axis).
  vector<pair<PMS::Axis, unsigned int> > loadedAxes() const;

  // Access to averaging state in the cache:
  PlotMSAveraging& averaging() { return averaging_; }

  // Access to transformations state in the cache
  PlotMSTransformations& transformations() { return transformations_; }

  // Access to channel averaging bounds
  Matrix<Int>& chanAveBounds(Int spw) { return chanAveBounds_p(spw); };

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
  
  // Clears the cache of all stored values.  This should be called when the
  // underlying MS or MS selection is changed, thus invalidating stored data.
  void clear();
  
  // Releases the given axes from the cache.
  void release(const vector<PMS::Axis>& axes);
  
  // Set up indexing for the plot
  void setUpIndexer(PMS::Axis iteraxis=PMS::SCAN,
		    Bool globalXRange=False, Bool globalYRange=False);

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

  inline Double getWt(Int chnk,Int irel) { return *(wt_[chnk]->data()+irel); };

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

protected:
    
  // Forbid copy for now
  PlotMSCache2(const PlotMSCache2& mc);

  // Increase the number of chunks
  void increaseChunks(Int nc=0);

  // Clean up the PtrBlocks
  void deleteCache();
  void deleteIndexer();

  // Setup the VisIter
  void setUpVisIter(const String& msname,
		    const PlotMSSelection& selection,
		    Bool readonly=True,
		    Bool chanselect=True,
		    Bool corrselect=True);

  // Count the chunks required in the cache
  void countChunks(ROVisibilityIterator& vi,PlotMSCacheThread* thread);  // old
  void countChunks(ROVisibilityIterator& vi, Vector<Int>& nIterPerAve,  // supports time-averaging 
		   const PlotMSAveraging& averaging,PlotMSCacheThread* thread);

  // Trap attempt to use to much memory (too many points)
  void trapExcessVolume(map<PMS::Axis,Bool> pendingLoadAxes);

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

  // Loads the specific axis/metadata into the cache using the given VisBuffer.
  void loadAxis(VisBuffer& vb, Int vbnum, PMS::Axis axis,
		PMS::DataColumn data = PMS::DEFAULT_DATACOLUMN);
  
  // Set the net axes mask (defines how to collapse flags for the chosen plot axes)
  void setAxesMask(PMS::Axis axis,Vector<Bool>& axismask);

  // Return the net axes mask for the currently set plot axes
  Vector<Bool> netAxesMask(PMS::Axis xaxis,PMS::Axis yaxis);

  // Derive the plot mask by appropriately collapsing the flags
  void setPlotMask();           // all chunks
  void setPlotMask(Int chunk);  // per chunk

  // Delete the whole plot mask
  void deletePlotMask();

  // Set flags in the MS
  virtual void flagToDisk(const PlotMSFlagging& flagging,
			  Vector<Int>& chunks, 
			  Vector<Int>& relids,
			  Bool flag,
			  PlotMSIndexer* indexer);

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
  //  (used only for access to logger, so far)
  PlotMSApp* plotms_;

  // The indexer into the cache
  PtrBlock<PlotMSIndexer*> indexer_;

  // The number of chunks in the cache
  Int nChunk_;

  // The reference time for this cache, in seconds
  Double refTime_p;

  // The number of antennas
  Int nAnt_;

  // Global min/max
  Double minX_,maxX_,minY_,maxY_;

  // The fundamental meta-data cache
  Matrix<Int> chshapes_;
  Vector<Bool> goodChunk_;
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

  PtrBlock<Vector<Float>*> parang_;
  PtrBlock<Vector<Int>*> antenna_;
  PtrBlock<Vector<Double>*> az_,el_;

  Vector<Double> az0_,el0_,ha0_,pa0_;

  // Current setup/state.
  bool dataLoaded_;
  PMS::Axis currentX_, currentY_;
  map<PMS::Axis, bool> loadedAxes_;
  map<PMS::Axis, PMS::DataColumn> loadedAxesData_;

  // Global ranges
  Double xminG_,yminG_,xflminG_,yflminG_,xmaxG_,ymaxG_,xflmaxG_,yflmaxG_;

  // A copy of the Data parameters 
  String msname_;
  PlotMSSelection selection_;
  PlotMSAveraging averaging_;
  PlotMSTransformations transformations_;

  // Axes mask
  Vector<Bool> netAxesMask_;

  // collapsed flag mask for plotting
  PtrBlock<Array<Bool>*> plmask_;

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

  // Volume meter for volume calculation
  PMSCacheVolMeter vm_;

    
};
typedef CountedPtr<PlotMSCache2> PlotMSCache2Ptr;


}

#endif /* PLOTMSCACHE2_H_ */
