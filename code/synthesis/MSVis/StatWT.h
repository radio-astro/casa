//# StatWT.h: Sets WEIGHT and SIGMA for a VisBuffGroup according to the scatter
//#           of its visibilities.
//# Copyright (C) 2011
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

#ifndef MSVIS_STATWT_H
#define MSVIS_STATWT_H

#include <casa/aips.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MSVis/GroupWorker.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisBufferComponents.h>
#include <map>
#include <set>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forward decl
class VisBuffGroup;

//<summary>StatWT sets WEIGHT and SIGMA statistically</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisBuffGroup">VisBuffGroup</linkto>
//   <li> <linkto class="VisibilityIterator">VisibilityIterator</linkto>
//   <li> <linkto class="GroupProcessor">GroupProcessor</linkto>
//   <li> <linkto class="GroupWorker">GroupWorker</linkto>
// </prerequisite>
//
// <etymology>
// StatWT sets WEIGHT and SIGMA statistically.
// </etymology>
//
//<synopsis>
// Set the weights and sigmas according to the scatter of the
// visibilities.
//</synopsis>
//
//<todo>
// <li> 
//</todo>
class StatWT : public GroupWorker
{
public:
  // Construct an object that will set the weights and sigmas of vi's
  // MeasurementSet, as selected by outspw, according to the scatter of the
  // visibilities selected by fitspw.  If dorms is true, assume that the true
  // mean is 0.  Otherwise, use the standard sample variance.
  //
  // For each baseline and correlation, if fitspw does not select at least
  // minsamp unflagged visibilities, it will be flagged and weight and sigma
  // will not be calculated.  minsamp is effectively at least 2.
  //
  StatWT(const ROVisibilityIterator& vi,
         const MS::PredefinedColumns datacol=MS::DATA,
         const String& fitspw="*",
         const String& outspw="*",
         const Bool dorms=false,
         const uInt minsamp=2,
         const vector<uInt> selcorrs=vector<uInt>());

  //// Copy construct
  //StatWT(const StatWT& other) {}

  // Destructor
  virtual ~StatWT();

  //// Assignment
  //virtual StatWT& operator=(const StatWT& gw) {}

  // // Returns which columns need to be prefetched for process to work.
  // virtual asyncio::PrefetchColumns *prefetchColumns() const;

  // This is where all the work gets done!
  virtual Bool process(VisBuffGroup& vbg);

private:
  // Disable null c'tor.
  StatWT();

  Bool update_variances(std::map<uInt, Vector<uInt> >& ns,
                        std::map<uInt, Vector<Complex> >& means,
                        std::map<uInt, Vector<Double> >& variances,
                        const VisBuffer& vb,
                        const Cube<Bool>& chanmaskedflags, const uInt maxAnt);

  // ns and variances are effectively const here, but declaring them that way
  // would take some gymnastics.
  Bool apply_variances(VisBuffer& vb,
                       std::map<uInt, Vector<uInt> >& ns,
                       std::map<uInt, Vector<Double> >& variances,
                       const uInt maxAnt);

  // Compute a baseline (row) index (ant1, ant2).
  // It ASSUMES that ant1 and ant2 are both <= maxAnt.
  uInt hashFunction(const Int ant1, const Int ant2, const Int maxAnt)
  {
    return (maxAnt + 1) * ant1 - (ant1 * (ant1 - 1)) / 2 + ant2 - ant1;
  }

  // Initialized by c'tor:

  // Which of DATA, MODEL_DATA, or CORRECTED_DATA to fit.
  // It will always _write_ to DATA if datacols_p.nelements() == 1, and write
  // to all 3 otherwise.  Thus datacols_p.nelements() should be either 1 or 3.
  // 4 and 2 are right out, and FLOAT_DATA isn't handled by this yet.
  MS::PredefinedColumns datacol_p;

  String fitspw_p;      // Line-free channels used for the fit.  Can include ;
  String outspw_p;      // Channels to write out.  Does not yet support ;.
  Bool   dorms_p;       // If true, assume that the true mean is 0.
  uInt   rowsdone_p;    // How many rows have been written so far.
  std::set<Int> outspws_p;  // Spws to reweight.
                                // Otherwise, use the standard sample variance.
  uInt   minsamp_p;     // Minimum # of unflagged visibilities for calculating
                        // a variance.
  vector<uInt> selcorrs_p;
  // Not initialized by c'tor:
  //std::set<Int> appliedSpWs_p;
  std::map<Int, Vector<Bool>*> fitmask_p;      // spw -> a list of flags by chan
};

} //# NAMESPACE CASA - END

#endif

