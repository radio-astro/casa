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
  PlotMSVBAverager(Int nAnt);

  // Null destructor
  ~PlotMSVBAverager();

  // Set up baseline averaging
  inline void setBlnAveraging(Bool doBln) { 
    blnAve_p = doBln; if (doBln) setAntAveraging(False); };
  // Set up antenna averaging
  inline void setAntAveraging(Bool doAnt) { 
    antAve_p = doAnt; if (doAnt) setBlnAveraging(False); };
  // Set scalar averaging flag 
  inline void setScalarAve(Bool doScalar) { 
    //    cout << "Using " << (doScalar ? "SCALAR" : "VECTOR") << " averaging." << endl;
    inCoh_p = doScalar; };
  
  // Control which data column to average
  inline void setNoData() {doVC_p = doMVC_p = doCVC_p = doFC_p = doWC_p = 
				doUVW_p = False;};
  inline void setDoVC()  {doVC_p  = doWC_p = True;};
  inline void setDoMVC() {doMVC_p = doWC_p = True;};
  inline void setDoCVC() {doCVC_p = doWC_p = True;};
  inline void setDoFC()  {doFC_p  = doWC_p = True;};
  inline void setDoUVW() {doUVW_p = True;};

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
  Int& prtlev() { return prtlev_; };

  // Initialize the next accumulation interval
  void initialize(vi::VisBuffer2& vb);

  // Different accumulate versions
  void simpAccumulate (vi::VisBuffer2& vb);  // ordinary
  void antAccumulate (vi::VisBuffer2& vb);   // antenna-based averaging version

  // Verify zero or two crosshands present (if antAve_p)
  void verifyCrosshands(vi::VisBuffer2& vb);

  // Hash function to return the row offset for an interferometer (ant1, ant2)
  Int baseline(const Int& ant1, const Int& ant2);

  // Convert r/i to a/p
  void convertToAP(Cube<Complex>& d);

  // Number of antennas, correlations, and channels
  Int nAnt_p, nCorr_p, nChan_p, nBlnMax_p;

  // Weights in input VBs are chan-independent
  Bool chanIndepWt_p;

  // Validation by baseline (if False, no attempt to accumulate this baseline)
  Vector<Bool> blnOK_p;

  // Are we averaging baselines together?
  Bool blnAve_p;

  // Are we averaging antennas together?
  Bool antAve_p;

  // Are we incoherently (scalar) averaging?
  Bool inCoh_p;

  // Accumulation helpers...
  Double timeRef_p;
  Double minTime_p;
  Double maxTime_p;
  Double aveTime_p;
  Double aveInterval_p;
  Vector<Double> blnWtSum_p;
  Double vbWtSum_p;
  Int aveScan_p;

  // Optional averaging triggers
  Bool doVC_p, doMVC_p, doCVC_p, doFC_p, doUVW_p, doWC_p;

  // Accumulation buffer
  vi::VisBuffer2* avBuf_p;

  // Keep track of initialization state
  Bool initialized_p;

  // Correlation list for cross-hand swapping
  Vector<Int> jcor_p;
  
  // Diagnostic print level
  Int prtlev_;

    // Mutable arrays, set in avBuf_p when finalized
    Cube<Complex> avgVisCube_;
    Cube<Complex> avgModelCube_;
    Cube<Complex> avgCorrectedCube_;
    Cube<Float> avgFloatCube_;
    Cube<Bool> avgFlagCube_;
    Vector<Bool> avgFlagRow_;
    Cube<Float> avgWeight_;
    Matrix<Double> avgUvw_;
    Vector<Int> avgAntenna1_;
    Vector<Int> avgAntenna2_;
    Vector<Double> avgTime_;
    Vector<Double> avgTimeInterval_;
    Vector<Int> avgScan_;
};


} //# NAMESPACE CASA - END

#endif


