//# PlotMSCacheBase.h: Generic Data cache for plotms.
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
#ifndef PLOTMSCACHEBASE_H_
#define PLOTMSCACHEBASE_H_

#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/PlotMS/PlotMSFlagging.h>
#include <plotms/PlotMS/PlotMSTransformations.h>
//#include <plotms/Threads/PlotMSCacheThread.qo.h>
//#include <plotms/Data/PlotMSIndexer.h>

#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>

namespace casa {

//# Forward declarations.
class PlotMSApp;
class PlotMSIndexer;
class ThreadCommunication;

class PlotMSCacheBase {
  
  // Friend class declarations.
  friend class PlotMSIndexer;

  //TBD:    friend class PlotMSData;

public:    

  // Varieties of cache
  // TBD: move to PlotMSConstants?
  enum Type {MS, CAL};
  
  static const unsigned int THREAD_SEGMENT;
  static const PMS::Axis METADATA[];
  static const unsigned int N_METADATA;
    
  static bool axisIsMetaData(PMS::Axis axis);

  
  // Constructor which takes parent PlotMS.
  PlotMSCacheBase(PlotMSApp* parent);
  
  // Destructor
  virtual ~PlotMSCacheBase();

  // Identify myself
  //  pure virtual
  virtual PlotMSCacheBase::Type cacheType()=0;

  // Access to pol names
  virtual String polname(Int ipol)=0;

  // Meta axes info
  int nmetadata() const {return N_METADATA;};
  PMS::Axis metadata(int i) {return METADATA[i];};
  
  // Reference an indexer
  Int nIter() const { return indexer_.nelements(); };
  PlotMSIndexer& indexer(uInt i) {return (*indexer_[i]);};
  PlotMSIndexer& indexer0() { return *indexer0_; };

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
  // TBD: actually make this axis-dep?
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
		    /*PlotMSCacheThread**/ThreadCommunication* thread = NULL);
  
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
  inline Vector<Bool>& flagrow(Int chunk) { return *flagrow_[chunk]; };

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
  inline Double getUwave(Int chnk,Int irel) { return *(uwave_[chnk]->data()+irel); };
  inline Double getVwave(Int chnk,Int irel) { return *(vwave_[chnk]->data()+irel); };
  inline Double getWwave(Int chnk,Int irel) { return *(wwave_[chnk]->data()+irel); };

  inline Double getAmp(Int chnk,Int irel)  { return *(amp_[chnk]->data()+irel); };
  inline Double getPha(Int chnk,Int irel)  { return *(pha_[chnk]->data()+irel); };
  inline Double getReal(Int chnk,Int irel) { return *(real_[chnk]->data()+irel); };
  inline Double getImag(Int chnk,Int irel) { return *(imag_[chnk]->data()+irel); };

  inline Double getFlag(Int chnk,Int irel) { return *(flag_[chnk]->data()+irel); };
  inline Double getFlagRow(Int chnk,Int irel) { return *(flagrow_[chnk]->data()+irel); };
  inline Double getRow(Int chnk,Int irel) { return *(row_[chnk]->data()+irel); };

  inline Double getWt(Int chnk,Int irel) { return *(wt_[chnk]->data()+irel); };
  inline Double getWtxAmp(Int chnk, Int irel) { return *(wtxamp_[chnk]->data()+irel); }

  // These are array-global (one value per chunk)
  inline Double getAz0(Int chnk,Int irel) { return az0_(chnk);  (void)irel; };
  inline Double getEl0(Int chnk,Int irel) { return el0_(chnk);  (void)irel; };
  inline Double getRadialVelocity0(Int chnk, Int irel){ return radialVelocity_(chnk); (void)irel;};
  inline Double getRHO0(Int chnk, Int irel){return rho_(chnk); (void)irel; };

  inline Double getHA0(Int chnk,Int irel) { return ha0_(chnk);  (void)irel; };
  inline Double getPA0(Int chnk,Int irel) { return pa0_(chnk);  (void)irel; };

  // These are antenna-based
  inline Double getAntenna(Int chnk,Int irel) { return *(antenna_[chnk]->data()+irel); };
  inline Double getAz(Int chnk,Int irel)      { return *(az_[chnk]->data()+irel); };
  inline Double getEl(Int chnk,Int irel)      { return *(el_[chnk]->data()+irel); };
  inline Double getParAng(Int chnk,Int irel)  { return *(parang_[chnk]->data()+irel); };

  // These support generic non-complex calibration
  inline Double getPar(Int chnk,Int irel)  { return *(par_[chnk]->data()+irel); };


protected:
    
  // Forbid copy for now
  PlotMSCacheBase(const PlotMSCacheBase&);

  // Increase the number of chunks
  void increaseChunks(Int nc=0);

  // Specialized method for loading the cache
  //  (pure virtual: implemented specifically in child classes)
  virtual void loadIt(vector<PMS::Axis>& loadAxes,
		      vector<PMS::DataColumn>& loadData,
		      /*PlotMSCacheThread**/ThreadCommunication* thread = NULL)=0;

  virtual void flagToDisk(const PlotMSFlagging& flagging,
			  Vector<Int>& chunks, 
			  Vector<Int>& relids,
			  Bool flag,
			  PlotMSIndexer* indexer)=0;
  
  // Clean up the PtrBlocks
  void deleteCache();
  void deleteIndexer();

  virtual bool isEphemeris() {return false;};
  bool isEphemerisAxis( PMS::Axis axis ) const;
  // Set the net axes mask (defines how to collapse flags for the chosen plot axes)
  void setAxesMask(PMS::Axis axis,Vector<Bool>& axismask);

  // Return the net axes mask for the currently set plot axes
  Vector<Bool> netAxesMask(PMS::Axis xaxis,PMS::Axis yaxis);

  // Derive the plot mask by appropriately collapsing the flags
  void setPlotMask();           // all chunks
  void setPlotMask(Int chunk);  // per chunk

  // Delete the whole plot mask
  void deletePlotMask();

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

  // An empty indexer (its an empty PlotData object used for initialization)
  PlotMSIndexer* indexer0_;

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
  PtrBlock<Matrix<Double>*> uvdistL_, uwave_, vwave_, wwave_;
  PtrBlock<Vector<Double>*> freq_, vel_;
  PtrBlock<Vector<Int>*> chan_;
  PtrBlock<Vector<Int>*> corr_;

  // Optional parts of the cache
  PtrBlock<Vector<Float>*> pa_;

  // Data (the heavy part)
  PtrBlock<Array<Float>*> amp_, pha_, real_, imag_, wtxamp_;
  PtrBlock<Array<Bool>*> flag_;
  PtrBlock<Vector<Bool>*> flagrow_;
  
  PtrBlock<Array<Float>*> wt_;

  PtrBlock<Vector<Float>*> parang_;
  PtrBlock<Vector<Int>*> antenna_;
  PtrBlock<Vector<Double>*> az_,el_;


  Vector<Double> radialVelocity_, rho_;
  Vector<Double> az0_,el0_,ha0_,pa0_;

  PtrBlock<Array<Float>*> par_;

  // Current setup/state.
  bool dataLoaded_;
  PMS::Axis currentX_, currentY_;
  map<PMS::Axis, bool> loadedAxes_;
  map<PMS::Axis, PMS::DataColumn> loadedAxesData_;
  map<PMS::Axis, bool> pendingLoadAxes_;

  // Global ranges
  Double xminG_,yminG_,xflminG_,yflminG_,xmaxG_,ymaxG_,xflmaxG_,yflmaxG_;

  // A copy of the Data parameters 
  String filename_;
  PlotMSSelection selection_;
  PlotMSAveraging averaging_;
  PlotMSTransformations transformations_;

  // Axes mask
  Vector<Bool> netAxesMask_;

  // collapsed flag mask for plotting
  PtrBlock<Array<Bool>*> plmask_;

  // meta info for locate output
  Vector<String> antnames_; 	 
  Vector<String> stanames_; 	 
  Vector<String> antstanames_; 	 
  Vector<String> fldnames_; 	 

};
typedef CountedPtr<PlotMSCacheBase> PlotMSCacheBasePtr;


}

#endif /* PLOTMSCACHEBASE_H_ */
