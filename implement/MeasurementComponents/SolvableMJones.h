//# SolvableMJones.h: Definition for SolvableMJones matrices
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

#ifndef SYNTHESIS_SOLVABLEMJONES_H
#define SYNTHESIS_SOLVABLEMJONES_H
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <synthesis/MeasurementComponents/TimeVarMJones.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <casa/OS/File.h>

#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Model solvable, time-variable multiplicative errors for the VisEquation </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
//   <li> <linkto class="TimeVarMJones">TimeVarMJones</linkto> class
// </prerequisite>
//
// <etymology>
// SolvableMJones describes an interface for time variable and solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// SolvableMJones performs caching of values to minimize computation.
//
// Values may be loaded from, saved to, and interpolated from a Table.
//
// The following examples illustrate how an SolvableMJones can be
// used:
// <ul>
// <li> Solution for diagonal gains: <linkto class="MIfr">MIfr</linkto>
// </ul>
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
//      MIfr ij(vs);
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

class SolvableMJones : public TimeVarMJones {
public:

  SolvableMJones() {}

  // Load from a table
  SolvableMJones(VisSet& vs, String file, const String& select);

  // From VisSet - interval is the solution interval in seconds, deltat
  // is the fuzziness in time comparisons
  SolvableMJones(VisSet& vs, Double interval=DBL_MAX, Double deltat=1.0);

  // Copy
  SolvableMJones(const SolvableMJones& other);

  // Destructor
  ~SolvableMJones();

  // Assignment
  virtual SolvableMJones& operator=(const SolvableMJones& other);

  // Is this solveable?
  virtual Bool isSolveable() {return True;};

  // Initialize for gradient search
  virtual void initializeGradients();
  virtual void finalizeGradients();

  // Add to Gradient Chisq
  virtual void addGradients(const VisBuffer& vb, Int row, 
			    const Antenna& a1, const Antenna& a2,
			    const Vector<Float>& sumwt, 
			    const Vector<Float>& chisq,
			    const Cube<Complex>& c, const Cube<Float>& f);

  // Solve
  virtual Bool solve (VisEquation& me);

  // Store the time-variable gains as a table.
  // Derived classes may need to redefine this 
  virtual void store(const String& file, const Bool& append);

  // Retrieve the time-variable gains from a table
  virtual void load(const String& file, const String& select, 
		    const String& type);

protected:

  void initialize();

  // Update the interferometer gains from the gradients.
  virtual void updateIntGain(Int slot);

  // Get interferometer gains from store. To be defined in derived class.
  virtual void getIntGain(Cube<mjJones4>& ag, const VisBuffer& vb, Int row);

  // Need to keep reference to VisSet
  VisSet* vs_;

  // Statistics of fit
  Float sumwt_;
  Float chisq_;

  // Gradients of chisquared
  Cube< mjJones4 > gS_;
  Cube< mjJones4F > ggS_;

  // Fit status
  Vector<Bool> solutionOK_;
  Vector<Float> fit_;
  Vector<Float> fitwt_;

  LogSink logSink_p;
  virtual LogSink& logSink() {return logSink_p;};

};

// <summary> 
// MIfr: Model solvable, time-variable diagonal multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
//   <li> <linkto class="SolvableMJones">TimeVarMJones</linkto> class
// </prerequisite>
//
// <etymology>
// MIfr describes an interface for diagonal, time variable and solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// MIfr performs caching of values to minimize computation.
//
// Values may be loaded from, saved to, and interpolated from a Table.
//
// The elements of MIfr are constrained to be non-zero only along
// the diagonal.
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
//      MIfr ij(vs);
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

class MIfr : public SolvableMJones {
public:

  MIfr(VisSet& vs, Double interval=DBL_MAX, Double deltat=1.0);
  
  MIfr(VisSet& vs, String file, const String& select);
  
  ~MIfr();

protected:
  // init mask and cache for diagonal matrices
  void init();

  // Update the interferometer gains from the gradients.
  virtual void updateIntGain(Int slot);

};


} //# NAMESPACE CASA - END

#endif
