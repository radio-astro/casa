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


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>Controls the solution of calibration components (Jones Matrices)</summary>


class Calibrater 
{
 public:
  // Default constructor
  Calibrater();

  // Copy constructor and assignment operator
  Calibrater(const Calibrater&);
  Calibrater& operator=(const Calibrater&);

  // Destructor
  ~Calibrater();
  
  // Set uv-data selection criteria
  void setdata(const String& mode="none", 
	       const Int& nchan=1,
	       const Int& start=0, 
	       const Int& step=1,
	       const MRadialVelocity& mStart=MRadialVelocity(),
	       const MRadialVelocity& mStep=MRadialVelocity(),
	       const String& msSelect="");


  // Set uv-data selection via MSSelection
  void selectvis(const String& time="",
		 const String& spw="",
		 const String& scan="",
		 const String& field="",
		 const String& baseline="",
		 const String& uvrange="",
		 const String& chanmode="none",
		 const Int& nchan=1,
		 const Int& start=0, 
		 const Int& step=1,
		 const MRadialVelocity& mStart=MRadialVelocity(),
		 const MRadialVelocity& mStep=MRadialVelocity(),
		 const String& msSelect="");

  // Initialize calibration components to be apply or solved for
  //  Bool setapply(const String& type, const Record& applypar);
  Bool setapply (const String& type, 
		 const Double& t,
		 const String& table,
		 const String& interp,
		 const String& select,
		 const Bool& calwt,
		 const Vector<Int>& spwmap,
		 const Float& opacity);

  // Set up to apply calibration (using MSSelection syntax)
  Bool setapply (const String& type, 
		 const Double& t,
		 const String& table,
		 const String& spw,
		 const String& field,
		 const String& interp,
		 const Bool& calwt,
		 const Vector<Int>& spwmap,
		 const Float& opacity);

  Bool setapply (const String& type, 
		 const Record& applypar);

  //  Arrange to solve
  Bool setsolve (const String& type, 
		 const Double& t,
		 const Double& preavg, 
		 const Bool& phaseonly,
		 const Int& refant, 
		 const String& table,
		 const Bool& append);

  // Arrange to solve (using MSSelection syntax)
  Bool setsolve (const String& type, 
		 const Double& t,
		 const String& table,
		 const Bool& append,
		 const Double& preavg, 
		 const Bool& phaseonly,
		 const String& refant="");

  // Arrange to solve for BPOLY
  Bool setsolvebandpoly(const String& table,
			const Bool& append,
			const Vector<Int>& degree,
			const Bool& visnorm,
			const Bool& bpnorm,
			const Int& maskcenter,
			const Float& maskedge,
			const Int& refant);

  // Arrange to solve for BPOLY (using MSSelection syntax)
  Bool setsolvebandpoly(const String& table,
			const Bool& append,
			const Vector<Int>& degree,
			const Bool& visnorm,
			const Bool& bpnorm,
			const Int& maskcenter,
			const Float& maskedge,
			const String& refant);

  // Arrange to solve for GSPLINE 
  Bool setsolvegainspline(const String& table,
			  const Bool& append,
			  const String& mode,
			  const Double& splinetime,
			  const Double& preavg,
			  const Int& refant,
			  const Int& numpoint,
			  const Double& phasewrap);

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

  // Apply all setapply'd calibration components forming the CORRECTED_DATA column
  Bool correct();

  // Solve for a given calibration component
  Bool solve();

  // Modelfit
  Vector<Double> modelfit(const Int& iter,
			  const String& stype,
			  const Vector<Double>& par,
			  const Vector<Bool>& vary,
			  const String& file);

  // Fluxscale
  void fluxscale(const String& infile, 
		 const String& outfile,
		 const Vector<String>& refFields, 
		 const Vector<Int>& refSpwMap,
		 const Vector<String>& tranFields,
		 const Bool& append,
		 Matrix<Double>& fluxScaleFactor);

  // Fluxscale (using MSSelection syntax for fields)
  void fluxscale(const String& infile, 
		 const String& outfile,
		 const String& refFields, 
		 const Vector<Int>& refSpwMap,
		 const String& tranFields,
		 const Bool& append,
		 Matrix<Double>& fluxScaleFactor);

  // Fluxscale (via field indices)
  void fluxscale(const String& infile, 
		 const String& outfile,
		 const Vector<Int>& refField, 
		 const Vector<Int>& refSpwMap,
		 const Vector<Int>& tranField,
		 const Bool& append,
		 Matrix<Double>& fluxScaleFactor);

  // Accumulate (incremental)
  void accumulate(const String& intab,
		  const String& incrtab,
		  const String& outtab,
		  const Vector<String>& fields,
		  const Vector<String>& calFields,
		  const String& interp="linear",
		  const Double& t=-1.0,
		  const Vector<Int>& spwmap=Vector<Int>(1,-1));

  // Accumulate (using MSSelection syntax)
  void accumulate(const String& intab,
		  const String& incrtab,
		  const String& outtab,
		  const String& fields,
		  const String& calFields,
		  const String& interp="linear",
		  const Double& t=-1.0,
		  const Vector<Int>& spwmap=Vector<Int>(1,-1));

  // Accumululate (via field indices)
  void accumulate(const String& intab,
		  const String& incrtab,
		  const String& outtab,
		  const Vector<Int>& fields,
		  const Vector<Int>& calFields,
		  const String& interp="linear",
		  const Double& t=-1.0,
		  const Vector<Int>& spwmap=Vector<Int>(1,-1));

  // Smooth  calibration
  Bool smooth(const String& infile,
              String& outfile, 
	      const String& smoothtype, 
	      const Double& smoothtime,
	      const Vector<String>& fields);

  // Smooth  calibration (using MSSelection syntax
  Bool smooth(const String& infile,
              String& outfile, 
	      const String& smoothtype, 
	      const Double& smoothtime,
	      const String& fields);


  // Initialize the calibrator object from an input MeasurementSet.
  // Optional compression of the calibration columns (MODEL_DATA,
  // CORRECTED_DATA and IMAGING_WEIGHT) is supported.
  Bool initialize(MeasurementSet& inputMS, 
		  Bool compress=True);

  // Re-initialize the calibration scratch columns
  Bool initCalSet(const Int& calSet);

  // Report apply/solve state
  Bool state();
  Bool applystate();
  Bool solvestate();

  Bool cleanup();

  // Method to update MS HISTORY Table
  void writeHistory(LogIO& os, 
		    Bool cliCommand=False);

  
 private:
  // Log functions and variables
  LogIO sink_p;
  LogIO& logSink();

  // Time functions and variables
  String timerString();
  Timer timer_p;
  
  // Select on channel in the VisSet
  void selectChannel(const String& mode, 
		     const Int& nchan, const Int& start, const Int& step,
		     const MRadialVelocity& mStart,
		     const MRadialVelocity& mStep);
  
  // Interpret refant index
  Int getRefantIdx(const String& refant);
  
  // Interpret field indices (MSSelection)
  Vector<Int> getFieldIdx(const String& fields);

  // Interpret spw indices (MSSelection)
  Vector<Int> getSpwIdx(const String& spws);
  
  // Query apply types to see if we need to calibrate the weights
  Bool calWt();

  // Returns True if calibrator object is in a valid state
  Bool ok();

  // Create a VisSet for raw phase transfer if needed
  void getRawPhaseVisSet(Vector<Int>& spwid); 

  // The standard solving mechanism
  Bool standardSolve();

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

};


} //# NAMESPACE CASA - END

#endif
