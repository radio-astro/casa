//# CTTimeInterp1.h: Definition for Single-element Cal Interpolation time
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#ifndef CALTABLES_CTTIMEINTERP1_H
#define CALTABLES_CTTIMEINTERP1_H

#include <casa/aips.h>

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/RIorAParray.h>
#include <casa/Arrays/Cube.h>
#include <scimath/Functionals/Interpolate1D.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


class CTTimeInterp1
{
public:

  // Null ctor 
  //  CTTimeInterp1();  // TBD: make private?

  // From NewCalTable
  CTTimeInterp1(NewCalTable& ct,
		const String& timetype,
		Array<Float>& result,
		Array<Bool>& rflag);

  // Destructor
  virtual ~CTTimeInterp1();

  // Some state info
  Double timeRef() { return timeRef_; };
  String timeType() { return timeType_; };
  Vector<Double> domain() { return domain_; };

  // Set interpolation type
  void setInterpType(String strtype);

  // Interpolate, given timestamp; returns T if new result
  Bool interpolate(Double time);

  // Interpolate, given timestamp and fiducial freq; returns T if new result
  Bool interpolate(Double newtime, Double freq);

  void state(Bool verbose=False);

private:
  
  // Find 
  Bool findTimeRegistration(Int& idx,Bool& exact,Float newtime);

  // 
  void applyPhaseDelay(Double freq);


  // The CalTable 
  //  Should be a single antenna, spw
  NewCalTable ct_;

  // Access to columns
  ROCTMainColumns *mcols_p;

  // Interpolation modes
  String timeType_;

  // Current time, idx
  Double currTime_;
  Int currIdx_;
  Bool lastWasExact_;

  // Time list
  Double timeRef_;
  Vector<Float> timelist_;
  Vector<Double> domain_;

  // Flags list
  Cube<Bool> flaglist_;

  // Pointer to the time interpolator
  Interpolate1D<Float,Array<Float> > *tInterpolator_p;

  // Phase-delay correction info
  Double cfreq_;
  Cube<Float> cycles_;   // [npar,nchan,ntime]
  
  // Arrays in which to deposit results
  //  (ordinarily, these (CASA-Array) reference external Arrays)
  Array<Float> result_;
  Array<Bool> rflag_;

};


} //# NAMESPACE CASA - END

#endif
