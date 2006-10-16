//# SimVisJones.h: Definition for SimVisJones matrices
//# Copyright (C) 1996,1997,1999,2000
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

#ifndef SYNTHESIS_SIMVISJONES_H
#define SYNTHESIS_SIMVISJONES_H
#include <synthesis/MeasurementComponents/SolvableVisJones.h>
#include <casa/BasicMath/Random.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Model multiplicative gain errors for the VisEquation </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
// </prerequisite>
//
// <etymology>
// SimVisJones describes random multiplicative gains to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. 
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation. See <linkto class="VisJones">VisJones</linkto>
// for how SimVisJones is to be used.
// </synopsis> 
//
// <motivation>
// The properties of an additive component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class SimVisJones : public SolvableVisJones {
public:

  // Two supported distributions for the simulated gain errors
  enum Distribution { 
    // the normal distribution
    normal, 
    // the uniform distribution
    uniform 
  };

  SimVisJones() : SolvableVisJones() {}

  // From VisSet - caching interval is in seconds.
  SimVisJones(VisSet& vs, Int seed=0,  
	      Double interval=DBL_MAX, Double deltat=1.0);

  // Copy
  SimVisJones(const SimVisJones& other);

  // Assignment
  virtual SimVisJones& operator=(const SimVisJones& other);

  // make Local VisSet
  virtual void makeLocalVisSet();
 

protected:

  // random number generator
  MLCG rndGen_p;
};

// <summary> 
// Model multiplicative gain errors for the VisEquation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimVisJones">SimVisJones</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> module
//   <li> <linkto class="VisEquation">VisEquation</linkto> module
// </prerequisite>
//
// <etymology>
// SimGJones describes random multiplicative gains to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. 
// The Jones matrices are diagonal.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation. See <linkto class="VisJones">VisJones</linkto>
// for how SimVisJones is to be used.
// </synopsis> 
//
// <motivation>
// The properties of a multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class SimGJones : public SimVisJones {
public:

    // Construct from VisSet. Supply seed for random number generator.
    // the Width parameters represent rms for Normal and
    // total range for Uniform distributions. Phase is in radians.
    // Interval specifies the time variability of the gains, every
    // interval has a new gain.
    SimGJones(VisSet& vs, Int seed, Distribution ampDist=normal,
	      Double ampMean=1.0, Double ampWidth=0.1,
	      Distribution phaseDist=normal,
	      Double phaseMean=0.0, Double phaseWidth=0.1,
	      Double interval=DBL_MAX, Double deltat=1.0);
    ~SimGJones();

    virtual Type type() { return G;};

protected:
    void init();

    Random* ampDist_p;
    Random* phaseDist_p;
};

// <summary> 
// Model multiplicative gain errors for the VisEquation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimVisJones">SimVisJones</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> module
//   <li> <linkto class="VisEquation">VisEquation</linkto> module
// </prerequisite>
//
// <etymology>
// SimGJones describes random multiplicative gains to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. 
// The Jones matrices are those appropriate for polarization leakage terms.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation. See <linkto class="VisJones">VisJones</linkto>
// for how SimDJones is to be used.
// </synopsis> 
//
// <motivation>
// The properties of a multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class SimDJones : public SimVisJones {
public:

    SimDJones(VisSet& vs, Int seed, Distribution dist=normal,
	      Double mean=0.0, Double Width=0.1,
	      Double interval=DBL_MAX, Double deltat=1.0);
    ~SimDJones();

    virtual Type type() { return D;};

protected:
    void init();

    Random* leakageDist_p;
};


} //# NAMESPACE CASA - END

#endif
