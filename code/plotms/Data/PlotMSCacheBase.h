//# PlotMSCacheBase.h: Generic casacore::Data cache for plotms.
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
#include <plotms/PlotMS/PlotMSCalibration.h>

#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MFrequency.h>

#include <QVector>

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
  virtual PlotMSCacheBase::Type cacheType() const = 0;

  // Access to pol names
  virtual casacore::String polname(casacore::Int ipol)=0;

  // keep MS/CT filename (set cal type for CT)
  virtual void setFilename(casacore::String filename) = 0;
  casacore::String calType() const { return calType_; };
  bool polnRatio() const { return polnRatio_; };

  // Meta axes info
  int nmetadata() const {return N_METADATA;};
  PMS::Axis metadata(int i) {return METADATA[i];};
  
  // Reference an indexer; returns -1 if there is no indexer
  // for the given dataIndex.
  casacore::Int nIter( int dataIndex ) const;

  PlotMSIndexer& indexer( int dataIndex, casacore::uInt i) {
	  return (*indexer_[dataIndex][i]);
  };
  PlotMSIndexer& indexer0() {
	  return *indexer0_;
  };
  void resizeIndexer( int size );
  int getDataCount() const {
	  return currentX_.size();
  }

  PMS::Axis getIterAxis() const;

  // Report the number of chunks
  casacore::Int nChunk() const { return nChunk_; };

  // Returns whether cache is filled
  bool cacheReady() const { return dataLoaded_; }

  // Returns whether user canceled during loading chunks
  bool wasCanceled() const { return userCanceled_; }

  // Report the data shapes
  inline casacore::Matrix<casacore::Int>& chunkShapes() {return chshapes_;};

  // A chunk is good (T) if it contains data
  //  (when averaging, some chunks may have nrows=0)
  inline casacore::Bool goodChunk(casacore::Int ichunk) {return goodChunk_(ichunk); };

  // Is there a reference value for the specified axis?
  // TBD: actually make this axis-dep?
  inline bool hasReferenceValue(PMS::Axis axis) { return (axis==PMS::TIME && cacheReady()); };
  inline double referenceValue(PMS::Axis axis) { return (hasReferenceValue(axis) ? refTime() : 0.0); };
  
  // Report the reference time for this cache (in seconds)
  inline casacore::Double refTime() const { return refTime_p; };

  // Frequency frame in original casacore::MS or requested by user
  inline casacore::MFrequency::Types getFreqFrame() const { return freqFrame_; };

  // Returns which axes have been loaded into the cache, including metadata.
  // Also includes the size (number of points) for each axis (which will
  // eventually be used for a cache manager to let the user know the
  // relative memory use of each axis).
  std::vector<PMS::Axis> loadedAxes() const;

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
  virtual void load(const std::vector<PMS::Axis>& axes,
		    const std::vector<PMS::DataColumn>& data,
		    const casacore::String& filename,
		    const PlotMSSelection& selection,
		    const PlotMSAveraging& averaging,
		    const PlotMSTransformations& transformations,
		    const PlotMSCalibration& calibration,
		    /*PlotMSCacheThread**/ThreadCommunication* thread = NULL);
  
  // Clears the cache of all stored values.  This should be called when the
  // underlying casacore::MS or casacore::MS selection is changed, thus invalidating stored data.
  void clear();
  void clearRanges();
  
  // Releases the given axes from the cache.
  void release(const std::vector<PMS::Axis>& axes);
  
  // Set up indexing for the plot
  bool isIndexerInitialized( PMS::Axis iteraxis, casacore::Bool globalXRange,
  		casacore::Bool globalYRange, int dataIndex ) const;
  void setUpIndexer(PMS::Axis iteraxis=PMS::SCAN,
		    casacore::Bool globalXRange=false, casacore::Bool globalYRange=false, int dataIndex = 0);

  // Access to flags per chunk
  inline casacore::Array<casacore::Bool>& flag(casacore::Int chunk) { return *flag_[chunk]; };
  inline casacore::Vector<casacore::Bool>& flagrow(casacore::Int chunk) { return *flagrow_[chunk]; };

  // Axis-specific generic gets
  inline casacore::Double getScan(casacore::Int chnk,casacore::Int irel)     { return scan_(chnk);   (void)irel; };
  inline casacore::Double getField(casacore::Int chnk,casacore::Int irel)    { return field_(chnk);  (void)irel; };
  inline casacore::Double getTime(casacore::Int chnk,casacore::Int irel)     { return time_(chnk);  (void)irel; };
  inline casacore::Double getTimeIntr(casacore::Int chnk,casacore::Int irel) { return timeIntr_(chnk);  (void)irel; };
  inline casacore::Double getSpw(casacore::Int chnk,casacore::Int irel)      { return spw_(chnk);  (void)irel; };
  inline casacore::Double getFreq(casacore::Int chnk,casacore::Int irel) { return *(freq_[chnk]->data()+irel); };
  inline casacore::Double getVel(casacore::Int chnk,casacore::Int irel)  { return *(vel_[chnk]->data()+irel); };
  inline casacore::Double getChan(casacore::Int chnk,casacore::Int irel) { return *(chan_[chnk]->data()+irel); };
  inline casacore::Double getCorr(casacore::Int chnk,casacore::Int irel) { return *(corr_[chnk]->data()+irel); };
  inline casacore::Double getAnt1(casacore::Int chnk,casacore::Int irel) { return *(antenna1_[chnk]->data()+irel); };
  inline casacore::Double getAnt2(casacore::Int chnk,casacore::Int irel) { return *(antenna2_[chnk]->data()+irel); };
  inline casacore::Double getBsln(casacore::Int chnk,casacore::Int irel) { return *(baseline_[chnk]->data()+irel); };
  inline casacore::Double getRow(casacore::Int chnk,casacore::Int irel) { return *(row_[chnk]->data()+irel); };
  inline casacore::Double getObsid(casacore::Int chnk,casacore::Int irel) { return *(obsid_[chnk]->data()+irel); };
  inline casacore::Double getIntent(casacore::Int chnk,casacore::Int irel) { return *(intent_[chnk]->data()+irel); };
  inline casacore::Double getFeed1(casacore::Int chnk,casacore::Int irel) { return *(feed1_[chnk]->data()+irel); };
  inline casacore::Double getFeed2(casacore::Int chnk,casacore::Int irel) { return *(feed2_[chnk]->data()+irel); };

  inline casacore::Double getAmp(casacore::Int chnk,casacore::Int irel)  { return *(amp_[chnk]->data()+irel); };
  inline casacore::Double getAmpCorr(casacore::Int chnk,casacore::Int irel)  { return *(ampCorr_[chnk]->data()+irel); };
  inline casacore::Double getAmpModel(casacore::Int chnk,casacore::Int irel)  { return *(ampModel_[chnk]->data()+irel); };
  inline casacore::Double getAmpCorrMod(casacore::Int chnk,casacore::Int irel)  { return *(ampCorrModel_[chnk]->data()+irel); };
  inline casacore::Double getAmpDataMod(casacore::Int chnk,casacore::Int irel)  { return *(ampDataModel_[chnk]->data()+irel); };
  inline casacore::Double getAmpDataDivMod(casacore::Int chnk,casacore::Int irel)  { return *(ampDataDivModel_[chnk]->data()+irel); };
  inline casacore::Double getAmpCorrDivMod(casacore::Int chnk,casacore::Int irel)  { return *(ampCorrDivModel_[chnk]->data()+irel); };
  inline casacore::Double getAmpFloat(casacore::Int chnk,casacore::Int irel)  { return *(ampFloat_[chnk]->data()+irel); };

  inline casacore::Double getPha(casacore::Int chnk,casacore::Int irel)  { return *(pha_[chnk]->data()+irel); };
  inline casacore::Double getPhaCorr(casacore::Int chnk,casacore::Int irel)  { return *(phaCorr_[chnk]->data()+irel); };
  inline casacore::Double getPhaModel(casacore::Int chnk,casacore::Int irel)  { return *(phaModel_[chnk]->data()+irel); };
  inline casacore::Double getPhaCorrMod(casacore::Int chnk,casacore::Int irel)  { return *(phaCorrModel_[chnk]->data()+irel); };
  inline casacore::Double getPhaDataMod(casacore::Int chnk,casacore::Int irel)  { return *(phaDataModel_[chnk]->data()+irel); };
  inline casacore::Double getPhaDataDivMod(casacore::Int chnk,casacore::Int irel)  { return *(phaDataDivModel_[chnk]->data()+irel); };
  inline casacore::Double getPhaCorrDivMod(casacore::Int chnk,casacore::Int irel)  { return *(phaCorrDivModel_[chnk]->data()+irel); };

  inline casacore::Double getReal(casacore::Int chnk,casacore::Int irel) { return *(real_[chnk]->data()+irel); };
  inline casacore::Double getRealCorr(casacore::Int chnk,casacore::Int irel)  { return *(realCorr_[chnk]->data()+irel); };
  inline casacore::Double getRealModel(casacore::Int chnk,casacore::Int irel)  { return *(realModel_[chnk]->data()+irel); };
  inline casacore::Double getRealCorrMod(casacore::Int chnk,casacore::Int irel)  { return *(realCorrModel_[chnk]->data()+irel); };
  inline casacore::Double getRealDataMod(casacore::Int chnk,casacore::Int irel)  { return *(realDataModel_[chnk]->data()+irel); };
  inline casacore::Double getRealDataDivMod(casacore::Int chnk,casacore::Int irel)  { return *(realDataDivModel_[chnk]->data()+irel); };
  inline casacore::Double getRealCorrDivMod(casacore::Int chnk,casacore::Int irel)  { return *(realCorrDivModel_[chnk]->data()+irel); };

  inline casacore::Double getImag(casacore::Int chnk,casacore::Int irel) { return *(imag_[chnk]->data()+irel); };
  inline casacore::Double getImagCorr(casacore::Int chnk,casacore::Int irel)  { return *(imagCorr_[chnk]->data()+irel); };
  inline casacore::Double getImagModel(casacore::Int chnk,casacore::Int irel)  { return *(imagModel_[chnk]->data()+irel); };
  inline casacore::Double getImagCorrMod(casacore::Int chnk,casacore::Int irel)  { return *(imagCorrModel_[chnk]->data()+irel); };
  inline casacore::Double getImagDataMod(casacore::Int chnk,casacore::Int irel)  { return *(imagDataModel_[chnk]->data()+irel); };
  inline casacore::Double getImagDataDivMod(casacore::Int chnk,casacore::Int irel)  { return *(imagDataDivModel_[chnk]->data()+irel); };
  inline casacore::Double getImagCorrDivMod(casacore::Int chnk,casacore::Int irel)  { return *(imagCorrDivModel_[chnk]->data()+irel); };

  inline casacore::Double getWtxAmp(casacore::Int chnk, casacore::Int irel) { return *(wtxamp_[chnk]->data()+irel); }
  inline casacore::Double getWtxAmpCorr(casacore::Int chnk,casacore::Int irel)  { return *(wtxampCorr_[chnk]->data()+irel); };
  inline casacore::Double getWtxAmpModel(casacore::Int chnk,casacore::Int irel)  { return *(wtxampModel_[chnk]->data()+irel); };
  inline casacore::Double getWtxAmpCorrMod(casacore::Int chnk,casacore::Int irel)  { return *(wtxampCorrModel_[chnk]->data()+irel); };
  inline casacore::Double getWtxAmpDataMod(casacore::Int chnk,casacore::Int irel)  { return *(wtxampDataModel_[chnk]->data()+irel); };
  inline casacore::Double getWtxAmpDataDivMod(casacore::Int chnk,casacore::Int irel)  { return *(wtxampDataDivModel_[chnk]->data()+irel); };
  inline casacore::Double getWtxAmpCorrDivMod(casacore::Int chnk,casacore::Int irel)  { return *(wtxampCorrDivModel_[chnk]->data()+irel); };
  inline casacore::Double getWtxAmpFloat(casacore::Int chnk,casacore::Int irel)  { return *(wtxampFloat_[chnk]->data()+irel); };

  inline casacore::Double getFlag(casacore::Int chnk,casacore::Int irel) { return *(flag_[chnk]->data()+irel); };
  inline casacore::Double getFlagRow(casacore::Int chnk,casacore::Int irel) { return *(flagrow_[chnk]->data()+irel); };

  inline casacore::Double getWt(casacore::Int chnk,casacore::Int irel) { return *(wt_[chnk]->data()+irel); };
  inline casacore::Double getWtSp(casacore::Int chnk,casacore::Int irel) { return *(wtsp_[chnk]->data()+irel); };
  inline casacore::Double getSigma(casacore::Int chnk,casacore::Int irel) { return *(sigma_[chnk]->data()+irel); };
  inline casacore::Double getSigmaSp(casacore::Int chnk,casacore::Int irel) { return *(sigmasp_[chnk]->data()+irel); };

  inline casacore::Double getUVDist(casacore::Int chnk,casacore::Int irel) { return *(uvdist_[chnk]->data()+irel); };
  inline casacore::Double getUVDistL(casacore::Int chnk,casacore::Int irel) { return *(uvdistL_[chnk]->data()+irel); };
  inline casacore::Double getU(casacore::Int chnk,casacore::Int irel) { return *(u_[chnk]->data()+irel); };
  inline casacore::Double getV(casacore::Int chnk,casacore::Int irel) { return *(v_[chnk]->data()+irel); };
  inline casacore::Double getW(casacore::Int chnk,casacore::Int irel) { return *(w_[chnk]->data()+irel); };
  inline casacore::Double getUwave(casacore::Int chnk,casacore::Int irel) { return *(uwave_[chnk]->data()+irel); };
  inline casacore::Double getVwave(casacore::Int chnk,casacore::Int irel) { return *(vwave_[chnk]->data()+irel); };
  inline casacore::Double getWwave(casacore::Int chnk,casacore::Int irel) { return *(wwave_[chnk]->data()+irel); };

  // These are array-global (one value per chunk)
  inline casacore::Double getAz0(casacore::Int chnk,casacore::Int irel) { return az0_(chnk);  (void)irel; };
  inline casacore::Double getEl0(casacore::Int chnk,casacore::Int irel) { return el0_(chnk);  (void)irel; };
  inline casacore::Double getRadialVelocity0(casacore::Int chnk, casacore::Int irel){ return radialVelocity_(chnk); (void)irel;};
  inline casacore::Double getRHO0(casacore::Int chnk, casacore::Int irel){return rho_(chnk); (void)irel; };
  inline casacore::Double getHA0(casacore::Int chnk,casacore::Int irel) { return ha0_(chnk);  (void)irel; };
  inline casacore::Double getPA0(casacore::Int chnk,casacore::Int irel) { return pa0_(chnk);  (void)irel; };

  // These are antenna-based
  inline casacore::Double getAntenna(casacore::Int chnk,casacore::Int irel) { return *(antenna_[chnk]->data()+irel); };
  inline casacore::Double getAz(casacore::Int chnk,casacore::Int irel)      { return *(az_[chnk]->data()+irel); };
  inline casacore::Double getEl(casacore::Int chnk,casacore::Int irel)      { return *(el_[chnk]->data()+irel); };
  inline casacore::Double getParAng(casacore::Int chnk,casacore::Int irel)  { return *(parang_[chnk]->data()+irel); };

  // These support generic non-complex calibration
  inline casacore::Double getPar(casacore::Int chnk,casacore::Int irel)  { return *(par_[chnk]->data()+irel); };
  inline casacore::Double getSnr(casacore::Int chnk,casacore::Int irel)  { return *(snr_[chnk]->data()+irel); };

  // Returns a list of channel numbers that were averaged together in that chunk
  inline casacore::Vector<casacore::Int> getChansPerBin(casacore::Int chnk,casacore::Int irel) { return (*chansPerBin_[chnk])[irel]; };

  casacore::Record locateInfo(int plotIterIndex, const casacore::Vector<PlotRegion>& regions,
      		bool showUnflagged, bool showFlagged, bool selectAll );

  PlotLogMessage* locateRange( int plotIterIndex, const casacore::Vector<PlotRegion> & regions,
     		bool showUnflagged, bool showFlagged);

  PlotLogMessage* flagRange( int plotIterIndex, casa::PlotMSFlagging& flagging,
     		const casacore::Vector<PlotRegion>& regions, bool showFlagged);

  //Return a formatted string for time iteration plots giving the time range.
  casacore::String getTimeBounds( int iterValue );
  // Return the time as doubles 
  pair<casacore::Double,casacore::Double> getTimeBounds() const;
  // Return the axes ranges
  pair<casacore::Double,casacore::Double> getXAxisBounds() const;
  pair<casacore::Double,casacore::Double> getYAxisBounds() const;

protected:
    
  // Forbid copy for now
  PlotMSCacheBase(const PlotMSCacheBase&);

  // Resize storage for the number of chunks
  void setCache(casacore::Int newnChunk, const vector<PMS::Axis>& loadAxes,
    const vector<PMS::DataColumn>& loadData);
  template<typename T> void addArrays(
    casacore::PtrBlock<casacore::Array<T>*>& input);
  template<typename T> void addMatrices(
    casacore::PtrBlock<casacore::Matrix<T>*>& input);
  template<typename T> void addVectors(
    casacore::PtrBlock<casacore::Vector<T>*>& input);

  // Specialized method for loading the cache
  //  (pure virtual: implemented specifically in child classes)
  virtual void loadIt(std::vector<PMS::Axis>& loadAxes,
		      std::vector<PMS::DataColumn>& loadData,
		      /*PlotMSCacheThread**/ThreadCommunication* thread = NULL)=0;

  virtual void flagToDisk(const PlotMSFlagging& flagging,
			  casacore::Vector<casacore::Int>& chunks, 
			  casacore::Vector<casacore::Int>& relids,
			  casacore::Bool flag,
			  PlotMSIndexer* indexer, int dataIndex)=0;
  
  // Clean up the PtrBlocks
  void deleteCache();
  void deleteIndexer();



  virtual bool isEphemeris() {return false;};
  bool isEphemerisAxis( PMS::Axis axis ) const;
  // Set the net axes mask (defines how to collapse flags for the chosen plot axes)
  void setAxesMask(PMS::Axis axis,casacore::Vector<casacore::Bool>& axismask);

  // Return the net axes mask for the currently set plot axes
  casacore::Vector<casacore::Bool> netAxesMask(PMS::Axis xaxis,PMS::Axis yaxis);

  // Derive the plot mask by appropriately collapsing the flags
  void setPlotMask( casacore::Int dataIndex);           // all chunks
  void setPlotMask(casacore::Int dataIndex, casacore::Int chunk);  // per chunk

  // Delete the whole plot mask
  void deletePlotMask();

  // Returns the number of points loaded for the given axis or 0 if not loaded.
  // Only used for PlotMSCacheTab, no longer used in plotms
  //unsigned int nPointsForAxis(PMS::Axis axis) const;
  
  // Convenience methods that call log() with the given method name and the
  // appropriate event type.
  // <group>
  void logInfo(const casacore::String& method, const casacore::String& message) {
      log(method, message, PlotLogger::MSG_INFO); }
  void logDebug(const casacore::String& method, const casacore::String& message) {
      log(method, message, PlotLogger::MSG_DEBUG); }
  void logWarn(const casacore::String& method, const casacore::String& message) {
      log(method, message, PlotLogger::MSG_WARN); }
  void logError(const casacore::String& method, const casacore::String& message) {
      log(method, message, PlotLogger::MSG_ERROR); }
  
  void logLoad(const casacore::String& message) {
      log(PMS::LOG_ORIGIN_LOAD_CACHE, message, PMS::LOG_EVENT_LOAD_CACHE); }
  void logFlag(const casacore::String& message) {
      log(PMS::LOG_ORIGIN_FLAG, message, PMS::LOG_EVENT_FLAG); }
  // </group>
  
  // Logs the given message from the given method name as the given event type
  // (see PlotLogger).
  void log(const casacore::String& method, const casacore::String& message, int eventType);

  //Return the color lookup index for the chunk.
  int findColorIndex( int chunk, bool initialize );



  // Private data
  
  // Parent plotms.
  //  (used only for access to logger, so far)
  PlotMSApp* plotms_;

  // An empty indexer (its an empty PlotData object used for initialization)
  PlotMSIndexer* indexer0_;

  // The indexer into the cache
  std::vector<casacore::PtrBlock<PlotMSIndexer*> > indexer_;
  
  // The number of chunks in the cache
  casacore::Int nChunk_;

  // The reference time for this cache, in seconds
  casacore::Double refTime_p;

  // The number of antennas
  casacore::Int nAnt_;

  // Set frame from VI if not specified by user
  // (for VI2::getFrequencies and axis label)
  casacore::MFrequency::Types freqFrame_;

  // Global min/max
  casacore::Double minX_,maxX_,minY_,maxY_;

  // The fundamental meta-data cache
  casacore::Matrix<casacore::Int> chshapes_;
  casacore::Vector<casacore::Bool> goodChunk_;
  casacore::Vector<casacore::Double> time_, timeIntr_;
  casacore::Vector<casacore::Int> field_, spw_, scan_;
  casacore::PtrBlock<casacore::Vector<casacore::uInt>*> row_;
  casacore::PtrBlock<casacore::Vector<casacore::Int>*> antenna1_, antenna2_, baseline_;
  casacore::PtrBlock<casacore::Vector<casacore::Double>*> uvdist_, u_, v_, w_;
  casacore::PtrBlock<casacore::Matrix<casacore::Double>*> uvdistL_, uwave_, vwave_, wwave_;
  casacore::PtrBlock<casacore::Vector<casacore::Double>*> freq_, vel_;
  casacore::PtrBlock<casacore::Vector<casacore::Int>*> chan_;
  casacore::PtrBlock<casacore::Array<casacore::Int>*> chansPerBin_;
  casacore::PtrBlock<casacore::Vector<casacore::Int>*> corr_;
  casacore::PtrBlock<casacore::Vector<casacore::Int>*> obsid_;
  casacore::PtrBlock<casacore::Vector<casacore::Int>*> intent_;
  casacore::PtrBlock<casacore::Vector<casacore::Int>*> feed1_, feed2_;

  // Optional parts of the cache
  casacore::PtrBlock<casacore::Vector<casacore::Float>*> pa_;

  // casacore::Data (the heavy part)
  casacore::PtrBlock<casacore::Array<casacore::Float>*> amp_, 
      ampCorr_, ampModel_, ampCorrModel_, ampDataModel_, 
      ampDataDivModel_, ampCorrDivModel_, ampFloat_;
  casacore::PtrBlock<casacore::Array<casacore::Float>*> pha_, 
      phaCorr_, phaModel_, phaCorrModel_, phaDataModel_, 
      phaDataDivModel_, phaCorrDivModel_;  // no phase for FLOAT_DATA
  casacore::PtrBlock<casacore::Array<casacore::Float>*> real_, 
      realCorr_, realModel_, realCorrModel_, realDataModel_,
      realDataDivModel_, realCorrDivModel_;  // use real_ for FLOAT_DATA
  casacore::PtrBlock<casacore::Array<casacore::Float>*> imag_,
      imagCorr_, imagModel_, imagCorrModel_, imagDataModel_,
      imagDataDivModel_, imagCorrDivModel_;  // no imag for FLOAT_DATA
  casacore::PtrBlock<casacore::Array<casacore::Float>*> wtxamp_,
      wtxampCorr_, wtxampModel_, wtxampCorrModel_, wtxampDataModel_, 
      wtxampDataDivModel_, wtxampCorrDivModel_, wtxampFloat_;

  casacore::PtrBlock<casacore::Array<casacore::Bool>*> flag_;
  casacore::PtrBlock<casacore::Vector<casacore::Bool>*> flagrow_;
  
  casacore::PtrBlock<casacore::Array<casacore::Float>*> wt_,wtsp_;
  casacore::PtrBlock<casacore::Array<casacore::Float>*> sigma_,sigmasp_;

  casacore::PtrBlock<casacore::Vector<casacore::Float>*> parang_;
  casacore::PtrBlock<casacore::Vector<casacore::Int>*> antenna_;
  casacore::PtrBlock<casacore::Vector<casacore::Double>*> az_,el_;

  casacore::Vector<casacore::Double> radialVelocity_, rho_;
  casacore::Vector<casacore::Double> az0_,el0_,ha0_,pa0_;

  casacore::PtrBlock<casacore::Array<casacore::Float>*> par_, snr_;

  // Current setup/state.
  bool dataLoaded_;
  bool userCanceled_;
  std::vector<PMS::Axis> currentX_;
  std::vector<PMS::Axis> currentY_;
  std::vector<PMS::DataColumn> currentXData_;
  std::vector<PMS::DataColumn> currentYData_;
  map<PMS::Axis, bool> loadedAxes_;
  map<PMS::Axis, std::set<PMS::DataColumn>> loadedAxesData_;
  map<PMS::Axis, bool> pendingLoadAxes_;

  // Global ranges
  casacore::Double xminG_,yminG_,xflminG_,yflminG_,xmaxG_,ymaxG_,xflmaxG_,yflmaxG_;

  // A copy of the casacore::Data parameters 
  casacore::String filename_;
  PlotMSSelection selection_;
  PlotMSAveraging averaging_;
  PlotMSTransformations transformations_;
  PlotMSCalibration calibration_;

  // Axes mask
  std::vector<casacore::Vector<casacore::Bool> > netAxesMask_;

  // collapsed flag mask for plotting
  std::vector<casacore::PtrBlock<casacore::Array<casacore::Bool>* > > plmask_;

  // meta info for locate output
  casacore::Vector<casacore::String> antnames_; 	 
  casacore::Vector<casacore::String> stanames_; 	 
  casacore::Vector<casacore::String> antstanames_; 	 
  casacore::Vector<casacore::String> fldnames_;
  casacore::Vector<casacore::String> intentnames_;
  casacore::Array<casacore::Double> positions_;

  PMS::Axis iterAxis;
  bool ephemerisInitialized;
  ::QVector<double> uniqueTimes;

  // The calibration type (casacore::Table subType)
  casacore::String calType_;
  // polarization selection is ratio ("/")
  bool polnRatio_;

private:
  void _updateAntennaMask( casacore::Int a, casacore::Vector<casacore::Bool>& antMask, const casacore::Vector<casacore::Int> selectedAntennas );
  bool axisIsValid(PMS::Axis axis, const PlotMSAveraging& averaging);

};
typedef casacore::CountedPtr<PlotMSCacheBase> PlotMSCacheBasePtr;


}

#endif /* PLOTMSCACHEBASE_H_ */
