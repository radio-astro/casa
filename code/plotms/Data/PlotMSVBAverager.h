//# PlotMSVBAverager.h: class to average VisBuffers for PlotMS
//# Copyright (C) 2000,2002
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
//# $Id: VisBuffAccumulator.h,v 19.6 2004/11/30 17:50:38 ddebonis Exp $

#ifndef PLOTMSVBAVERAGER_H
#define PLOTMSVBAVERAGER_H

#include <casa/aips.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// A class to average VisBuffers for PlotMS
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> VisBuffer2
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// This class averages VisBuffers together for PlotMS
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="2009/04/24">
//   <li> averaging over other indices.
// </todo>

class PlotMSVBAverager
{
public:
  // Construct from the number of antennas, the averaging interval and
  // the pre-normalization flag
  PlotMSVBAverager(casacore::Int nAnt);

  // Null destructor
  ~PlotMSVBAverager();

  // Set up baseline averaging
  inline void setBlnAveraging(casacore::Bool doBln) { 
    blnAve_p = doBln; if (doBln) setAntAveraging(false); };
  // Set up antenna averaging
  inline void setAntAveraging(casacore::Bool doAnt) { 
    antAve_p = doAnt; if (doAnt) setBlnAveraging(false); };
  // Set scalar averaging flag 
  inline void setScalarAve(casacore::Bool doScalar) { 
    //    cout << "Using " << (doScalar ? "SCALAR" : "VECTOR") << " averaging." << endl;
    inCoh_p = doScalar; };
  
  // Control which data column to average
  inline void setNoData() {doVC_p = doMVC_p = doCVC_p = doFC_p = doWC_p = 
				doUVW_p = false;};
  inline void setDoVC()  {doVC_p  = doWC_p = true;};
  inline void setDoMVC() {doMVC_p = doWC_p = true;};
  inline void setDoCVC() {doCVC_p = doWC_p = true;};
  inline void setDoFC()  {doFC_p  = doWC_p = true;};
  inline void setDoUVW() {doUVW_p = true;};

  // Accumulate a VisBuffer
  inline void accumulate (vi::VisBuffer2& vb) { antAve_p ? antAccumulate(vb) : simpAccumulate(vb); };

  // Finalize averaging
  void finalizeAverage();

  // Return a reference to the result
  vi::VisBuffer2& aveVisBuff() { return *avBuf_p; }

private:
  // Prohibit null constructor, copy constructor and assignment for now
  PlotMSVBAverager();
  PlotMSVBAverager& operator= (const PlotMSVBAverager&);
  PlotMSVBAverager (const PlotMSVBAverager&);

  // Diagnostic printing level
  casacore::Int& prtlev() { return prtlev_; };

  // Initialize the next accumulation interval
  void initialize(vi::VisBuffer2& vb);

  // Different accumulate versions
  void simpAccumulate (vi::VisBuffer2& vb);  // ordinary
  void antAccumulate (vi::VisBuffer2& vb);   // antenna-based averaging version

  // Verify zero or two crosshands present (if antAve_p)
  void verifyCrosshands(vi::VisBuffer2& vb);

  // Hash function to return the row offset for an interferometer (ant1, ant2)
  casacore::Int baseline(const casacore::Int& ant1, const casacore::Int& ant2);

  // Convert r/i to a/p
  void convertToAP(casacore::Cube<casacore::Complex>& d);

  // fill vector that is resized larger
  void fillIds(casacore::Int nrows);

  // Number of antennas, correlations, and channels
  casacore::Int nAnt_p, nCorr_p, nChan_p, nBlnMax_p;

  // Weights in input VBs are chan-independent
  casacore::Bool chanIndepWt_p;

  // Validation by baseline (if false, no attempt to accumulate this baseline)
  casacore::Vector<casacore::Bool> blnOK_p;

  // Are we averaging baselines together?
  casacore::Bool blnAve_p;

  // Are we averaging antennas together?
  casacore::Bool antAve_p;

  // Are we incoherently (scalar) averaging?
  casacore::Bool inCoh_p;

  // Accumulation helpers...
  casacore::Double timeRef_p;
  casacore::Double minTime_p;
  casacore::Double maxTime_p;
  casacore::Double aveTime_p;
  casacore::Double aveInterval_p;
  casacore::Vector<casacore::Double> blnWtSum_p;
  casacore::Double vbWtSum_p;
  casacore::Int aveScan_p;

  // Optional averaging triggers
  casacore::Bool doVC_p, doMVC_p, doCVC_p, doFC_p, doUVW_p, doWC_p;

  // Accumulation buffer
  vi::VisBuffer2* avBuf_p;

  // Keep track of initialization state
  casacore::Bool initialized_p;

  // Correlation list for cross-hand swapping
  casacore::Vector<casacore::Int> jcor_p;
  
  // Diagnostic print level
  casacore::Int prtlev_;

    // Mutable arrays, set in avBuf_p when finalized
    casacore::Cube<casacore::Complex> avgVisCube_;
    casacore::Cube<casacore::Complex> avgModelCube_;
    casacore::Cube<casacore::Complex> avgCorrectedCube_;
    casacore::Cube<casacore::Float> avgFloatCube_;
    casacore::Cube<casacore::Bool> avgFlagCube_;
    casacore::Vector<casacore::Bool> avgFlagRow_;
    casacore::Cube<casacore::Float> avgWeight_;
    casacore::Matrix<casacore::Double> avgUvw_;
    casacore::Vector<casacore::Int> avgAntenna1_;
    casacore::Vector<casacore::Int> avgAntenna2_;
    casacore::Vector<casacore::Double> avgTime_;
    casacore::Vector<casacore::Double> avgTimeInterval_;
    casacore::Vector<casacore::Int> avgScan_;
    // Need to resize these in final avBuf
    casacore::Vector<casacore::Int> fieldid_;
    casacore::Vector<casacore::Int> spw_;
    casacore::Vector<casacore::Int> obsid_;
    casacore::Vector<casacore::Int> stateid_;
};


} //# NAMESPACE CASA - END

#endif


