//# Calibrater.h: High-level calibrator object; controls calibration overall
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//#
//# $Id: Calibrater.h,v 19.16 2006/02/14 19:46:23 gmoellen Exp $

#ifndef SYNTHESIS_CALIBRATER_H
#define SYNTHESIS_CALIBRATER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MRadialVelocity.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/MeasurementComponents/VisCalGlobals.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <synthesis/MSVis/VisibilityProcessing.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>Controls the solution of calibration components (Jones Matrices)</summary>

class CorrectorVp;

namespace asyncio {
  class PrefetchColumns;
}

class Calibrater 
{

  friend class CorrectorVp;

 public:
  // Default constructor
  Calibrater();

  // Copy constructor and assignment operator
  Calibrater(const Calibrater&);
  Calibrater& operator=(const Calibrater&);

  // Destructor
  ~Calibrater();
  

  // Set uv-data selection via MSSelection
  void selectvis(const String& time="",
		 const String& spw="",
		 const String& scan="",
		 const String& field="",
		 const String& intent="",
		 const String& obsIDs="",
		 const String& baseline="",
		 const String& uvrange="",
		 const String& chanmode="none",
		 const Int& nchan=1,
		 const Int& start=0, 
		 const Int& step=1,
		 const MRadialVelocity& mStart=MRadialVelocity(),
		 const MRadialVelocity& mStep=MRadialVelocity(),
		 const String& msSelect="");

  // Set up to apply calibration (using MSSelection syntax)
  Bool setapply (const String& type, 
		 const Double& t,
		 const String& table,
		 const String& spw,
		 const String& field,
		 const String& interp,
		 const Bool& calwt,
		 const Vector<Int>& spwmap,
		 const Vector<Double>& opacity);

  Bool setapply (const String& type, 
		 const Record& applypar);

  // Set up apply-able calibration via a Cal Library
  Bool setcallib(Record callib);
  Bool validatecallib(Record callib);

  Bool setmodel(const String& modelImage);
  Bool setModel(const Vector<Double>& stokes);

  // Arrange to solve (using MSSelection syntax)
  Bool setsolve (const String& type, 
		 const String& solint,
		 const String& table,
		 const Bool append,
		 const Double preavg, 
		 const String& apmode="AP",
		 const Int minblperant=4,
		 const String& refant="",
		 const Bool solnorm=False,
		 const Float minsnr=0.0f,
		 const String& combine="",
		 const Int fillgaps=0,
		 const String& cfcache="",
		 const Double painc=360.0,
                 const Int fitorder=0);

  // Arrange to solve for BPOLY (using MSSelection syntax)
  Bool setsolvebandpoly(const String& table,
			const Bool& append,
			const String& solint,
			const String& combine,
			const Vector<Int>& degree,
			const Bool& visnorm,
			const Bool& solnorm,
			const Int& maskcenter,
			const Float& maskedge,
			const String& refant);

  // Arrange to solve for GSPLINE (using MSSelection syntax)
  Bool setsolvegainspline(const String& table,
			  const Bool& append,
			  const String& mode,
			  const Double& splinetime,
			  const Double& preavg,
			  const Int& numpoint,
			  const Double& phasewrap,
			  const String& refant);

  Bool setsolve (const String& type, 
		 const Record& solvepar);

  // Unset all (default) or one apply calibration component
  Bool unsetapply(const Int& which=-1);
  // TBD:   Bool unsetapply(const String& type);  // by type?

  // Unset the solved-for calibration component
  Bool unsetsolve();

  // Reset the calibrator object
  Bool reset(const Bool& apply=True, 
	     const Bool& solve=True);

  // Apply all setapply'd calibration components to DATA and
  //  deposit in the CORRECTED_DATA column
  Bool correct(String mode="calflag");

  // Apply all setapply'd calibration components to MODEL_DATA and
  //  deposit in the MODEL_DATA column
  Bool corrupt();

  // Initialize sigma/weight according to bandwidth/exposure
  Bool initWeights();

  // Solve for a given calibration component
  Bool solve();

  // Modelfit
  Vector<Double> modelfit(const Int& iter,
			  const String& stype,
			  const Vector<Double>& par,
			  const Vector<Bool>& vary,
			  const String& file);

  // Fluxscale (using MSSelection syntax for fields)
  void fluxscale(const String& infile, 
		 const String& outfile,
		 const String& refFields, 
		 const Vector<Int>& refSpwMap,
		 const String& tranFields,
		 const Bool& append,
		 SolvableVisCal::fluxScaleStruct& oFluxScaleFactor,
		 Vector<Int>& tranidx,
		 const String& oListFile,
                 const Bool& incremental,
                 const Int& fitorder);

  // Fluxscale (via field indices)
  void fluxscale(const String& infile, 
		 const String& outfile,
		 const Vector<Int>& refField, 
		 const Vector<Int>& refSpwMap,
		 const Vector<Int>& tranField,
		 const Bool& append,
		 SolvableVisCal::fluxScaleStruct& oFluxScaleFactor,
		 const String& oListFile,
		 const Bool& incremental,
                 const Int& fitorder);

  // Accumulate (using MSSelection syntax)
  void accumulate(const String& intab,
		  const String& incrtab,
		  const String& outtab,
		  const String& fields,
		  const String& calFields,
		  const String& interp="linear",
		  const Double& t=-1.0,
		  const Vector<Int>& spwmap=Vector<Int>(1,-1));

  // Generate cal table from specified values
  void specifycal(const String& type,
		  const String& caltable,
		  const String& time,
		  const String& spw,
		  const String& antenna,
		  const String& pol,
		  const Vector<Double>& parameter);

  // Smooth  calibration (using MSSelection syntax
  Bool smooth(const String& infile,
              String& outfile, 
	      const String& smoothtype, 
	      const Double& smoothtime,
	      const String& fields);


  // List a calibration table
  Bool listCal(const String& infile,
	       const String& field,
	       const String& antenna,
	       const String& spw,
	       const String& listfile="",
	       const Int& pagerows=50);

  // Initialize the calibrator object from an input MeasurementSet.
  // Optional compression of the calibration columns (MODEL_DATA,
  // and CORRECTED_DATA) is supported.
  //if addScratch=True, optionally one can just add CORRECTED_DATA by
  //setting addModel to False
  Bool initialize(MeasurementSet& inputMS, 
		  Bool compress=True,
		  Bool addScratch=True, Bool addModel=True);

  // Re-initialize the calibration scratch columns
  Bool initCalSet(const Int& calSet);

  // Report apply/solve state
  Bool state();
  Bool applystate();
  Bool solvestate();

  Record& getActRec() {return actRec_;};

  Bool cleanup();

  // Method to update MS HISTORY Table
  void writeHistory(LogIO& os, 
		    Bool cliCommand=False);

  CorrectorVp * getCorrectorVp ();

  // Handle caltable backward compatibility
  static Bool updateCalTable(const String& caltable);
  
 private:
  // Log functions and variables
  LogIO sink_p;
  LogIO& logSink();

  // Time functions and variables
  String timerString();
  Timer timer_p;

  VisibilityIterator::DataColumn configureForCorrection ();
  Bool correctUsingVpf ();

  // Select on channel using MSSelection
  void selectChannel(const String& spw);

  // Channel mask services
  void initChanMask();

  // Select on channel in the VisSet
  void selectChannel(const String& mode, 
		     const Int& nchan, const Int& start, const Int& step,
		     const MRadialVelocity& mStart,
		     const MRadialVelocity& mStep);
  
  // Interpret refant index
  Vector<Int> getRefantIdxList(const String& refant);
  
  Vector<Int> getAntIdx(const String& antenna);

  // Interpret field indices (MSSelection)
  Vector<Int> getFieldIdx(const String& fields);

  // Interpret spw indices (MSSelection)
  Vector<Int> getSpwIdx(const String& spws);

  // Interpret spw indices (MSSelection)
  Matrix<Int> getChanIdx(const String& spws);
  
  // Query apply types to see if we need to calibrate the weights
  Bool calWt();

  // Returns True if calibrator object is in a valid state
  Bool ok();

  // Given a (supplied) list of uncalibrated spws, determines and returns if there were
  // any, and if so sends them as a warning message to the logger.
  Bool summarize_uncalspws(const Vector<Bool>& uncalspw, const String& origin,
			   Bool strictflag=False);

  // Create a VisSet for raw phase transfer if needed
  void getRawPhaseVisSet(Vector<Int>& spwid); 

  // The standard solving mechanism
  Bool genericGatherAndSolve();

  // Input MeasurementSet and derived selected MeasurementSet
  String msname_p;
  MeasurementSet* ms_p;
  MeasurementSet* mssel_p;
  MeasurementSet* msselRaw_p;

  // VisSet and VisEquation derived from the input MeasurementSet
  VisSet* vs_p;
  VisSet* rawvs_p;
  VisEquation* ve_p;

  // VisCals for applying and solving:
  PtrBlock<VisCal*> vc_p;
  SolvableVisCal* svc_p;

  // MeasurementSet selection parameters
  String dataMode_p;
  Int dataNchan_p, dataStart_p, dataStep_p;
  MRadialVelocity mDataStart_p, mDataStep_p;

  //Spline phase wrapping helper params
  Double phasewrap_p;
  Int splinepoint_p;

  //Used to update the MS HISTORY Table
  Int histLockCounter_p;
  MSHistoryHandler *hist_p;
  Table historytab_p;

  // channel masking 
  PtrBlock<Vector<Bool>*> chanmask_;

  // Activity record
  Record actRec_;

};

class CorrectorVp : public vpf::VisibilityProcessor {

public:

    CorrectorVp (Calibrater * calibrater, const String & name = "Corrector");
    ROVisibilityIterator * getVisibilityIterator ();

    static const String In;
    static const String Out;

protected:

    void chunkStartImpl (const vpf::SubchunkIndex &);
    ProcessingResult
    doProcessingImpl (ProcessingType processingType,
                      vpf::VpData & inputData,
                      const vpf::SubchunkIndex & subChunkIndex);
    void processingStartImpl ();
    void validateImpl ();

private:

    Calibrater * calibrater_p;
    Bool calculateWeights_p;
    Vector<Bool> uncalibratedSpectralWindows_p;
    VisibilityIterator::DataColumn  whichOutputColumn_p;
};




} //# NAMESPACE CASA - END

#endif
