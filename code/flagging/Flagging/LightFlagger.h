//# LightFlagger.h: A lighter flagger - for autoflag
//# Copyright (C) 2000,2001
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
//# $Jan 28 2011 rurvashi Id$
#ifndef FLAGGING_LIGHTFLAGGER_H
#define FLAGGING_LIGHTFLAGGER_H

#include <flagging/Flagging/FlagVersion.h>

#include <flagging/Flagging/LFBase.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/Quantum.h>
#include <synthesis/MSVis/VisSet.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Fitting.h>
#include <scimath/Fitting/LinearFit.h>
#include <scimath/Fitting/GenericL2Fit.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// LightFlagger: Tool for manual and automatic flagging
// </summary>

// <use visibility=global>
// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> Flagger/LFBase
//   <li> MS/MSSelection
//   <li> casadbus/PlotServerProxy and FlagPlotServerProxy
// </prerequisite>
//
// <etymology>
// A lighter version of the existing flagger. 
// This should eventually replace the Flagger.
// </etymology>
//
// <synopsis>
// LightFlagger performs manual and automated flagging operations on a measurement set.
//  -- Attaches to an MS, with subsets decided via MSSelection. 
//  -- Contains one or more flagging methods of types derived from LFBase
//  -- Iterates through the MS using a VisibilityIterator
//  -- For each visbuffer, send each flagmethod the data and flags (by reference).
//  -- Each flagmethod modifies the flags (in sequence, and in place).
//  -- A FlagDisplay method displays data and flags as the visbuffers iterate by
//  -- A record of summary statistics is returned after all the data has been traversed.
// 
// 
// </synopsis>
//
// <example>
//        // Construct MS
//        MeasurementSet ms("test.MS2",Table::Update);
//
//        // Construct LightFlagger
//        LightFlagger flagger(ms);
//
//        // Build record of global flagging options
//        Record gen_options();
//        gen_options.define("showplots",True);
//        gen_options.define("writeflags",False);
//
//        // Build record of flagging-agent options (can read default params first)
//        Record algo_options( flagger.getparameters("tfcrop") );
//        algo_options.define("freq_amp_cutoff",3.0);
//
//        // Send flagging-agent parameters to the flagger
//        flagger.setparameters("tfcrop",algo_options);
//
//        // Run it.
//        flagger.run(gen_options);
//
//
// </example>
//
// <motivation>
// The existing Flagger infrastructure needs clean-up.
// </motivation>
//
// <todo asof="2011/05/23">
//   <li> Add a new LFManualFlag class for manual flagging and 'quacking'.
//   <li> Add data-expression functionality for the user to control what to flag on.
//   <li> Add a new LFClip for simple amplitude-threshold clipping
//   <li> Add a new LFFilters for 1D sliding-window statistical filters
//   <li> Add flag statistics counts to LFFlagDisplay
//   <li> Allow extensions across polarization, baseline, time,freq in LFExtendFlags
//   <li> Add support for saving/reading parameters to/from Flag_Cmd strings
//   <li> Use async visbuffer I/O (the dominant cost is I/O).
//   <li> Create a task interface to replace flagdata/flagdata2
// </todo>
 

  class LightFlagger 
  {
  public:  
    // default constructor 
    LightFlagger  ();
    // default destructor
    ~LightFlagger ();

    // Attach to a measurement se
    Bool attach(String msname);    

    // Set the data selection parameters
    Bool setdata(String field="", String spw="", String array="", String feed="", String scan="",
		    String baseline="", String uvrange="", String time="",
		    String correlation="");
    
    // Set autoflag params
    Bool setparameters(String algorithm, Record &parameters);
    
    // Get default autoflag params
    Record getparameters(String algorithm);

    // Run all the algorithms
    Record run(Record &parameters);    
    
    // flag version support.
    Bool  saveFlagVersion(String versionname, String comment, String merge);
    Bool  restoreFlagVersion(Vector<String> versionname, String merge);
    Bool  deleteFlagVersion(Vector<String> versionname);
    Bool  getFlagVersionList( Vector<String> &verlist);
    
  private:

    Record defaultParameters();
    void initParameters(Record &parameters);

    Bool setupIterator();
    Bool readVisAndFlags(VisBuffer &vb, uInt timecnt);
    Bool writeFlags(VisibilityIterator &vi, VisBuffer &vb, uInt timecnt);

     void Ants(uInt bs, uInt *a1, uInt *a2);
     uInt BaselineIndex(uInt row, uInt a1, uInt a2);

  void AllocateMemory();  

  void runAllMethods(Int threadindex, Int nmethods, Int nthreads, 
		     uInt numT, uInt numAnt, uInt numB, uInt numC, uInt nPol);

    MeasurementSet   ms_p,  mssel_p;
    MSSelection mss_p;
    VisSet *vs_p;

    // List of flagging agents
    Vector<CountedPtr<LFBase> >  flagmethods_p;
    Int maxnumthreads_p;  

    // Generic input parameters
    Bool ShowPlots, WriteFlagsToMS, StopAndExit, FlagZeros,UsePreFlags;
    Int NumTime, FlagLevel;
    String Expr, Column;

    Vector<Int> selCorr;
    Vector<Bool> baselineFlag;
 
    // Shapes per chunk
  uInt NumT; // Number of timestamps in one block
  uInt NumB; // Number of baselines.
  uInt NumAnt; // Number of antennas.
  uInt NumC; // Number of channels
  uInt NumP; // Number of polarizations
    uInt Nrow; // Number of rows in current chunk
    uInt Ncorr; // Number of correlations
    IPosition VisCubeShp;

    // Data Mapper
    //    Vector<Bool> dmCorrs;
    // Bool SetupDataMapper();

    // Storage arrays
  Cube<Float> visc; 	// visCube
  Cube<Bool> flagc; 	// flagCube
    Cube<Bool> preflagc; //flagCube before flagging.

   Vector<Int> ant1,ant2;
    //   LogIO os;
    Bool dbg;
    
  };
  
  
} //# NAMESPACE CASA - END

#endif

