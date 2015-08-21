//# MSCache.h: MS-specific Data cache for plotms.
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
#ifndef MSCACHE_H_
#define MSCACHE_H_

#include <plotms/Data/PlotMSCacheBase.h>

#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/PlotMS/PlotMSFlagging.h>
//#include <plotms/Threads/PlotMSCacheThread.qo.h>
#include <plotms/Threads/ThreadCommunication.h>
#include <plotms/Data/PlotMSVBAverager.h>
#include <plotms/Data/MSCacheVolMeter.h>

#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MFrequency.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBufferUtil.h>
#include <msvis/MSVis/ViFrequencySelection.h>

namespace casa {

//# Forward declarations.
class PlotMSApp;
class PlotMSIndexer;

class MSCache : public PlotMSCacheBase {
    
  // Friend class declarations.
  friend class PlotMSIndexer;

public:    
  
  // Constructor which takes parent PlotMS.
  MSCache(PlotMSApp* parent);
  
  // Destructor
  virtual ~MSCache();

  // Identify myself
  PlotMSCacheBase::Type cacheType() const { return PlotMSCacheBase::MS; };

  // ...not yet MS-specific... (or ever?)
  // Set up indexing for the plot
  //  void setUpIndexer(PMS::Axis iteraxis=PMS::SCAN,
  //		    Bool globalXRange=False, Bool globalYRange=False);

  virtual String polname(Int ipol);

protected:

  // MS-specific loadIt method
  virtual void loadIt(vector<PMS::Axis>& loadAxes,
		      vector<PMS::DataColumn>& loadData,
		      /*PlotMSCacheThread**/ThreadCommunication* thread = NULL);

  //Returns whether or not the ephemeris data has been
  //attached to a field - radial velocity and rho.
  virtual bool isEphemeris();
private:
    
  // Forbid copy for now
  MSCache(const MSCache&);

  // Set up
  String checkDataColumn(vector<PMS::Axis>& loadAxes, 
			 vector<PMS::DataColumn>& loadData);
  String getDataColumn(PMS::DataColumn dataCol);
  String checkAxesDatacolumns();
  void getNamesFromMS(MeasurementSet& ms);
  void setUpVisIter(PlotMSSelection& selection,
		    PlotMSCalibration& calibration,
		    String dataColumn,
            Bool interactive=False,
		    Bool estimateMemory=False);
  vi::VisibilityIterator2* setUpVisIter(MeasurementSet& selectedMS,
	Vector<Vector<Slice> > chansel, Vector<Vector<Slice> > corrsel);
  void setUpFrequencySelectionChannels(vi::FrequencySelectionUsingChannels fs,
	Vector<Vector<Slice> > chansel);

  // clean up
  void deleteVi();
  void deleteVm();
  void loadError(String mesg);

  // Count the chunks with averaging
  void countChunks(vi::VisibilityIterator2& vi, Vector<Int>& nIterPerAve, 
                   /*PlotMSCacheThread**/ThreadCommunication* thread);

  // Trap attempt to use to much memory (too many points)
  void trapExcessVolume(map<PMS::Axis,Bool> pendingLoadAxes);
  std::vector<IPosition> visBufferShapes_;

  // Loop over VisIter, filling the cache
  void loadChunks(vi::VisibilityIterator2& vi,
		  const vector<PMS::Axis> loadAxes,
		  const vector<PMS::DataColumn> loadData,
		  /*PlotMSCacheThread**/ThreadCommunication* thread);
  void loadChunks(vi::VisibilityIterator2& vi,
		  const PlotMSAveraging& averaging,
		  const Vector<Int>& nIterPerAve,
		  const vector<PMS::Axis> loadAxes,
		  const vector<PMS::DataColumn> loadData,
		  /*PlotMSCacheThread**/ThreadCommunication* thread);

  // Force read on vb for requested axes 
  //   (so pre-cache averaging treats all data it should)
  void forceVBread(vi::VisBuffer2* vb,
		   vector<PMS::Axis> loadAxes,
		   vector<PMS::DataColumn> loadData);

  // Tell time averager which data column to read
  void discernData(vector<PMS::Axis> loadAxes,
		   vector<PMS::DataColumn> loadData,
		   PlotMSVBAverager& vba);

  // Loads the specific axis/metadata into the cache using the given VisBuffer.
  void loadAxis(vi::VisBuffer2* vb, Int vbnum, PMS::Axis axis,
		PMS::DataColumn data = PMS::DEFAULT_DATACOLUMN);

  // Set flags in the MS
  virtual void flagToDisk(const PlotMSFlagging& flagging,
			  Vector<Int>& chunks, 
			  Vector<Int>& relids,
			  Bool setFlag,
			  PlotMSIndexer* indexer, int dataIndex);

  // Create map of intent names to "intent ids" 
  // since state ids can duplicate intents.
  // Then use map to assign intent ids to replace state ids
  // (stateId -> intent string -> intentId)
  map<String, Int> intentIds_; 
  void mapIntentNamesToIds();   // create map
  // Use map to assign intent ids
  Vector<Int> assignIntentIds(Vector<Int>& stateIds);

  // Provisional flagging helpers
  Vector<Int> nVBPerAve_;

  // VisIterator pointer
  vi::VisibilityIterator2* vi_p;

  // Volume meter for volume calculation
  MSCacheVolMeter* vm_;

  // Set frame from VI if not specified by user (for VI2::getFrequencies)
  MFrequency::Types freqFrame_;

  Vector<Double> calcVelocity(vi::VisBuffer2* vb); 

  // For averaging done by PlotMSVBAverager;
  // Some axes need to come from VB2 attached to VI2
  bool useAveragedVisBuffer(PMS::Axis axis);

  bool ephemerisAvailable;
};
typedef CountedPtr<MSCache> MSCachePtr;


}

#endif /* MSCACHE_H_ */
