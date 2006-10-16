//# TimeVarMJones.h: Definition for TimeVarMJones matrices
//# Copyright (C) 1996,1997,1998,1999,2000
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
//# $Id$

#ifndef SYNTHESIS_TIMEVARMJONES_H
#define SYNTHESIS_TIMEVARMJONES_H
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <synthesis/MeasurementComponents/MJones.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <casa/OS/File.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

namespace casa { //# NAMESPACE CASA - BEGIN

typedef SquareMatrix<Complex,4> mjJones4;
typedef SquareMatrix<Float,4> mjJones4F;

//forward

// <summary> Model baseline-related multiplicative errors for the VisEquation </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
//   <li> <linkto class="MJones">MJones</linkto> class
// </prerequisite>
//
// <etymology>
// TimeVarMJones describes an interface for time variable but not solvable
// baseline-related, multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// TimeVarMJones performs caching of values to minimize computation.
//
// Values may be saved to a table.
// </synopsis> 
//
// <example>
// <srcblock>
//      // Read the VisSet from disk
//      VisSet vs("3c84.MS");
//
//      VisEquation ve(vs);
//
//      // Make an MJones
//      SMIfr ij(vs);
//
//      ve.setMJones(ij);
//
//      // Correct the visset
//      ve.correct();
// </srcblock>
// </example>
//
// <motivation>
// The properties of an multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class TimeVarMJones : public MJones {
public:

  TimeVarMJones() {}

  // From VisSet - caching interval is in seconds
  TimeVarMJones(VisSet& vs, Double interval=DBL_MAX, Double deltat=1.0);

  // Copy
  TimeVarMJones(const TimeVarMJones& other);

  virtual ~TimeVarMJones();

  // Assignment
  virtual TimeVarMJones& operator=(const TimeVarMJones& other);

  // Apply actual Jones matrix and inverse
  virtual VisBuffer& apply(VisBuffer& vb); 
  virtual VisBuffer& applyInv(VisBuffer& vb);

  // Store the time-variable gains as a table.
  // Derived classes may need to redefine this 
  virtual void store(const String& file, const Bool& append);

  // Retrieve the time-variable gains from a table
  virtual void load(const String& file, const String& select, 
		    const String& type);

protected:

  void initialize(VisSet& vs);

  void initializeCache();

  // Calculate gains ab initio. To be defined in derived class
  virtual void getIntGain(Cube<mjJones4>& ig, const VisBuffer& vb, Int Row) = 0;

  // Is this time in the slot? 
  Bool inSlot(Double start, Double end, Double time);

  // Is the time in the specified slot. slot must be in range
  Bool inSlot(Int slot, Double time);

  // Find the relevant slot. Throw an exception if not found.
  Int findSlot(Double time);

  // check the cache : if necessary find new slot, get new interferometer
  // gains and then refresh values in the cache
  void checkCache(const VisBuffer& vb, Int row);
 
  // (in)validate cache
  void invalidateCache() {cacheValid_=False;}
  void validateCache(Int spectralWindow) {cacheValid_(spectralWindow)=True;}
  Bool cacheValid(Int spectralWindow) {return cacheValid_(spectralWindow);}

  // Number of Slots and current Slot
  Int numberOfSlots_, currentSlot_;

  // Interval of each slot
  Double interval_;

  // Precision of time comparison
  Double deltat_;

  // Start and end times in MJD
  Vector<Double> MJDStart_, MJDStop_;

  // Field and source names, and field id.
  Vector<String> fieldName_, sourceName_;
  Vector<Int> fieldid_;

  // Status of cache
  Vector<Bool> cacheValid_;

  // Number of antennas
  Int numberAnt_;

  // Number for spectral Windows
  Int numberSpw_;

  // Actual caches
  Cube<mjJones4> intGainCache_;
  Cube<mjJones4> intGainInvCache_;

  // Jones matrix for every interferometer and slot.
  // To be resized and initialized with SquareMatrices of correct type 
  // by derived class.
  Array<mjJones4> intGain_;

  LogSink logSink_p;
  LogSink& logSink() {return logSink_p;};

};


} //# NAMESPACE CASA - END

#endif
