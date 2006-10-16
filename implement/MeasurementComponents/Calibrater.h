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
//# $Id$

#ifndef SYNTHESIS_CALIBRATER_H
#define SYNTHESIS_CALIBRATER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MRadialVelocity.h>
#include <synthesis/MeasurementEquations.h>
#include <synthesis/MeasurementComponents.h>
#include <synthesis/MeasurementComponents/SolvableVisJones.h>
#include <synthesis/MeasurementComponents/GJonesPoly.h>
#include <synthesis/MeasurementComponents/BJonesPoly.h>
#include <synthesis/MeasurementComponents/SolvableMJones.h>


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
  
  // Initialize the calibrator object from an input MeasurementSet.
  // Optional compression of the calibration columns (MODEL_DATA,
  // CORRECTED_DATA and IMAGING_WEIGHT) is supported.
  Bool initialize(MeasurementSet& inputMS, Bool compress=True);

  // Set uv-data selection criteria
  void setdata(const String& mode, const Int& nchan,
		       const Int& start, const Int& step,
		       const MRadialVelocity& mStart,
		       const MRadialVelocity& mStep,
		       const String& msSelect);

  // Flush the underlying uv-data to disk
  Bool write();

  // Initialize calibration components to be solved for or applied
  Bool setApply(const String& typeComp, const Record& interpolation, 
		Vector<Int> rawspw= Vector<Int>(0));
  Bool setSolve(const String& typeComp, const Record& solver);

  // Unset a calibration component
  Bool unset(String typeComp);

  // Unset the solved-for calibration component
  Bool unsetSolve();

  // Apply all set calibration components to update the CORRECTED_DATA column
  Bool correct();

  // Solve for a given calibration component (Jones matrix)
  Bool solve(String typeComp);

  // Modelfit
  Vector<Double> modelfit(const Int& iter,
			  const String& stype,
			  const Vector<Double>& par,
			  const Vector<Bool>& vary,
			  const String& file);

  // Smooth and Interpolate calibration
  Bool smooth(const String& infile,
              const String& outfile, const Bool& append,
              const String& select,
              const String& smoothtype, const Double& smoothtime,
              const String& interptype, const Double& interptime);

  // Flush calibration solutions to disk
  Bool put(String typeComp, String tableName, Bool append);

  // Fluxscale
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
		  const Vector<Int>& fields,
		  const Vector<Int>& calFields,
		  const String& interp="linear",
		  const Double& t=-1.0);

  // Reset the calibrator object
  Bool reset();

  // Re-initialize the calibration scratch columns
  Bool initCalSet(const Int& calSet);

  // Set parameters for 360 jump de-wrapping in spline solver
  void setPhaseSplineParam(const Int& npoi, const Double& phaseWrap);

  // Method to update MS HISTORY Table
  void writeHistory(LogIO& os, Bool cliCommand=False);

  
 private:
  // Log functions and variables
  LogIO sink_p;
  LogIO& logSink();

  // Time functions and variables
  String timerString();
  Timer timer_p;

  // Returns True if calibrator object is in a valid state
  Bool ok();

  // Create a VisSet for raw phase transfer if needed
  void getRawPhaseVisSet(Vector<Int>& spwid); 

  // Input MeasurementSet and derived selected MeasurementSet
  String msname_p;
  MeasurementSet* ms_p;
  MeasurementSet* mssel_p;
  MeasurementSet* msselRaw_p;

  // VisSet and VisEquation derived from the input MeasurementSet
  VisSet* vs_p;
  VisSet* rawvs_p;
  VisEquation* ve_p;

  // Jones matrices used in the Measurement Equation.
  BJones* bj_p;
  GJones* gj_p;
  DJones* dj_p;
  CJones* cj_p;
  EVisJones* ej_p;
  PJones* pj_p;
  TJones* tj_p;
  MIfr *mj_p;

  KMueller* km_p;
  MMueller* mm_p;
  MfMueller* mfm_p;

  // The type to be solved for
  SolvableVisJones* svj_p;
  Bool delsvj_;
  SolvableVisCal* svc_p;
  Bool delsvc_;

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
