//# FringeVisJones.h: Definition for Fringe Fitting
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

#ifndef SYNTHESIS_FRINGEVISJONES_H
#define SYNTHESIS_FRINGEVISJONES_H

#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <synthesis/MeasurementComponents/SolvableVisJones.h>
#include <casa/OS/File.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogIO.h>

namespace casa {

//# forward
class VisEquation;

// <summary> 
// GJonesDelayRateSB: Model solvable, time-variable multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
//   <li> <linkto class="TimeVarVisJones">TimeVarVisJones</linkto> class
// </prerequisite>
//
// <etymology>
// GJonesDelayRateSB describes an interface for time variable and solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// GJonesDelayRateSB performs caching of values to minimize computation.
// The elements of GJonesDelayRateSB are only non-zero on the diagonal. The
// two diagonal elements are different. If you need them to be the
// same, use <linkto class="TJones">TJones</linkto>.
//
// </synopsis> 
//
// <motivation>
// The properties of an multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class GJonesDelayRateSB : public GJones {
public:

  GJonesDelayRateSB(VisSet& vs, Double interval=DBL_MAX, 
		    Double preavg=DBL_MAX, Double deltat=1.0, 
		    Int refant=-1);

  //  GJonesDelayRateSB(VisSet& vs, String file, const String& select);

  ~GJonesDelayRateSB();

  // Frequency dependent 
  virtual Bool freqDep() {return True;};

  // Solve
  virtual Bool solve (VisEquation& me);

  // Update the antenna gains from the gradients.
  //  virtual void updateAntGain(Int slot);

protected:
  // init mask and cache for diagonal matrices
  //  void init();

};

} //# end namespace casa

#endif
