//# SolvableVisJones.h: Definition for SolvableVisJones matrices
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
//# $Id$

#ifndef SYNTHESIS_SOLVABLEVISJONES_H
#define SYNTHESIS_SOLVABLEVISJONES_H

#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <synthesis/MeasurementComponents/TimeVarVisJones.h>
#include <casa/OS/File.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forward
class VisEquation;

// <summary> 
// SolvableVisJones: Model solvable, time-variable multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
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
// SolvableVisJones describes an interface for time variable and solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// SolvableVisJones performs caching of values to minimize computation.
//
// Values may be loaded from, saved to, and interpolated from a Table.
//
// The following examples illustrate how an SolvableVisJones can be
// used:
// <ul>
// <li> Solution for diagonal gains: <linkto class="GJones>GJones</linkto>
// <li> Solution for non-diagonal gains: <linkto class="DJones">GJones</linkto>
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
//      // Make an VisJones
//      GJones gj(vs);
//
//      ve.setVisJones(gj);
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

class SolvableVisJones : public TimeVarVisJones {
public:

  enum FilterType {meanFilter, medianFilter,slidingMean, slidingMedian};
  enum InterpolaterType {nearestNeighbour,linear,cubic, spline};

  SolvableVisJones() {}

  // From VisSet alone
  SolvableVisJones(VisSet& vs);

  // Copy
  SolvableVisJones(const SolvableVisJones& other);

  // Destructor
  ~SolvableVisJones();

  // Assignment
  //  virtual SolvableVisJones& operator=(const SolvableVisJones& other);

  // Setup apply context
  virtual void setInterpolation(const Record& interpolation);

  virtual void setInterpolationParam(const String& table,
				     const String& select,
				     const Double& interval);
  
  // Setup solve context
  virtual void setSolver(const Record& solver);
  
  virtual void setSolverParam(const String& table,
			      const Double& interval,
			      const Double preavg=0.0,
			      const Int refant=-1,
			      const Bool phaseonly=False);

  virtual void setAccumulate(const String& table,
			     const String& select="",
			     const Double& interval=0.0,
			     const Int& refant=-1);

  // Is this solveable?
  virtual Bool isSolveable() {return True;};

  // Return the pre-averaging interval for the solution
  virtual Double preavg() {return preavg_;};

  // Return the referent antenna for the solution
  virtual Int refant() {return refant_;};

  virtual VisBuffer& applyGradient(VisBuffer& vb, 
				   const Vector<Int>& antenna, 
				   Int i, Int j, Int pos); 

  // Initialize for gradient search
  virtual void initializeGradients();
  virtual void finalizeGradients();

  // Add to Gradient Chisq
  virtual void addGradients(const VisBuffer& vb, Int row, const Antenna& a,
			    const Vector<Float>& sumwt, 
			    const Vector<Float>& chisq,
			    const Vector<SquareMatrix<Complex,2> >& c, 
			    const Vector<SquareMatrix<Float,2> >& f);

  // Solve
  virtual Bool solve (VisEquation& me);

  // Store the time-variable gains, fits and status as a table.
  void store(const String& file, const Bool& append);

  //
  // Filter this from the other.  timeConstant is the filter time constant,
  // and scanGap is the gap in time between scans.  Both are in seconds.
  //
  virtual void filterFrom(SolvableVisJones& other, 
			  FilterType filterType=meanFilter,
			  Float timeConstant=0.0, 
			  Bool doAmpPhase=True, 
			  Float scanGap=FLT_MAX) {};
  // Interpolate this from the other. 
  virtual void interpolateFrom(SolvableVisJones& other, 
			       InterpolaterType interpolaterType=spline,
			       Bool doAmpPhase=True) {};

  //
  // timeConstant and scanGap are in seconds.  
  //
  virtual void filter(Vector<Double>& inX,  Vector<Array<Double> >& inY, Vector<Bool>& inFlags,
		      Vector<Double>& outX, Vector<Array<Double> >& outY,
		      FilterType filterType, Float timeConstant, Float scanGap=300) {};
  virtual void slidingFilter(Vector<Double>& inX,  Vector<Array<Double> >& inY, Vector<Bool>& inFlags,
			     Vector<Double>& outX, Vector<Array<Double> >& outY,
			     FilterType filterType, Float timeConstant, Float scanGap=300) {};


  virtual void fluxscale(const Vector<Int>& refFieldIn, 
			 const Vector<Int>& tranFieldIn,
			 const Vector<Int>& inRefSpwMap,
			 Matrix<Double>& fluxScaleFactor);


  // Accumulate another onto this one
  void accumulate(SolvableVisJones& incr, const Vector<Int>& fields);


protected:

  // Initialize solve cache
  void initSolveCache();

  // Delete solve cache
  void deleteSolveCache();

  // The type string specifies whether to load "general", "diagonal" or 
  // "scalar" 2x2 Jones matrices.
  virtual void load(const String& file, const String& select="", 
		    const String& type="general");

  // Update the antenna gains from the gradients.
  virtual void updateAntGain();

  // Temporary method for copying solved-for matrix into "parameterized" storage
  //  (eventually, we'll use the parameterized storage directly in solve)
  virtual void keep(const Int& spw,const Int& slot);

  // Re-reference the gain solutions
  virtual void reReference(const Int& refAnt);

  // Cal table filename
  String calTableName_;

  // Selecttion on cal table
  String calSelect_;

  // Pre-averaging interval for solution
  Double preavg_;

  // Reference antenna for solution
  Int refant_;

  // Indicates which elements of Jones matrix are required
  Matrix<Bool> required_;

  // Fit status
  Float sumwt_;
  Float chisq_;
  //  Vector<Bool> solutionOK_;
  //  Vector<Float> fit_;
  //  Vector<Float> fitwt_;


  //----------------------------------------------------------
  // New PB versions
  PtrBlock<Matrix<Bool>*> iSolutionOK_;  // [numberSpw_](numberAnt_,numberSlots_)
  PtrBlock<Matrix<Float>*> iFit_;        // [numberSpw_](numberAnt_,numberSlots_)
  PtrBlock<Matrix<Float>*> iFitwt_;      // [numberSpw_](numberAnt_,numberSlots_)
  
  PtrBlock<Vector<Bool>*> solutionOK_;   // [numberSpw_](numberSlots_)
  PtrBlock<Vector<Float>*> fit_;         // [numberSpw_](numberSlots_)
  PtrBlock<Vector<Float>*> fitwt_;       // [numberSpw_](numberSlots_)

  Matrix< mjJones2 > gS_;     // (nSolnChan,numberAnt_)
  Matrix< mjJones2F > ggS_;   // (nSolnChan,numberAnt_)


  LogSink logSink_p;
  virtual LogSink& logSink() {return logSink_p;};

  LogIO logSink2_p;
  LogIO& logSink2() {return logSink2_p;};

};


// ********************************************************************
// ********************************************************************
// ********************************************************************
//
//   Specialized non-solvable types:
//   ******************************
//

// <summary> 
// GJones: Model solvable, time-variable multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
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
// GJones describes an interface for time variable and solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// GJones performs caching of values to minimize computation.
// The elements of GJones are only non-zero on the diagonal. The
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

class GJones : public SolvableVisJones {
public:

  // Null constructor
  GJones() {};

  GJones(VisSet& vs);

  ~GJones();

  virtual Type type() { return G;};

};


// ********************************************************************
// ********************************************************************


class BJones : public GJones {
public:
  // Null constructor
  BJones() {};

  BJones(VisSet& vs);

  ~BJones();

  virtual Bool freqDep() { return True; };

  virtual Type type() { return B;};

};


// ********************************************************************
// ********************************************************************

// <summary>
// EVisJones: Model Gain Curve corrections for the <linkto class="VisEquation">VisEquation</linkto>
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
// EVisJones describes an interface for describing gain curves.  E is the
// generic Jones type for forward gain, and this is the VisJones (i.e.,
// not instantaneously direction dependent) component. Note that
// the gain curve is direction-dependent as an antenna slews, and is
// therfore time-dependent.
// </etymology>
//
// <synopsis>
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The elements of EVisJones are only non-zero on the diagonal.
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

class EVisJones : public SolvableVisJones {
public:
  // Null constructor
  EVisJones() {gaincoeff_p=0;};

  EVisJones(VisSet& vs);
  
  ~EVisJones();

  virtual Type type() { return E;};

  virtual void setInterpolation(const Record& interpolation);

  // Get gain curve by calculation
  virtual void calcAntGainPar(const VisBuffer& vb);
  virtual void calcJonesPar(const Int& spw,const Double& time,const Int& field=-1);

private:

  Array<Float>* gaincoeff_p;

};

// ********************************************************************
// ********************************************************************


// <summary> 
// DJones: Model solvable, time-variable multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
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
// DJones describes an interface for time variable and solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// DJones performs caching of values to minimize computation.
// The elements of DJones are potentially all If you need only the diagonal
// use <linkto class="TJones">TJones</linkto> or use <linkto class="GJones">GJones</linkto>.
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
class DJones : public SolvableVisJones {
public:

  DJones(VisSet& vs);

  ~DJones();

  virtual Type type() { return D;};

  virtual void calcAntGainMat(mjJones2& mat, Vector<Complex>& par);
  virtual void calcJonesMat(mjJones2& mat, Vector<Complex>& par);

  // Temporary method for copying solved-for matrix into "parameterized" storage
  //  (eventually, we'll use the parameterized storage directly in solve)
  virtual void keep(const Int& spw,const Int& slot);


};

// ********************************************************************
// ********************************************************************

// <summary> 
// TJones: Model solvable, time-variable multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
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
// TJones describes an interface for time variable and solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// TJones performs caching of values to minimize computation.
// The elements of TJones are only non-zero on the diagonal and
// are the same. If you need them to be 
// different, use <linkto class="GJones">GJones</linkto>.
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

class TJones : public SolvableVisJones {
public:
  // Null constructor
  TJones() {};

  TJones(VisSet& vs);

  ~TJones();

  virtual Type type() { return T;};

  // Update the antenna gains from the gradients.
  virtual void updateAntGain();

};

// ********************************************************************
// ********************************************************************

// <summary>
// TOpac: Model solvable, time-variable multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
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
// TOpac describes an interface for TJones parameterized by one or more
// opacity values for
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis>
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The elements of TOpac are only non-zero on the diagonal and
// are the same.
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

class TOpac : public TJones {
public:
  // Null constructor
  TOpac() {};

  TOpac(VisSet& vs);

  ~TOpac();

  virtual Type type() { return T;};

  virtual void setInterpolation(const Record& interpolation);

  // Get opacity gain by calculation
  virtual void calcAntGainPar(const VisBuffer& vb);
  virtual void calcJonesPar(const Int& spw,const Double& time,const Int& field=-1);

private:
  Float opacity_p;

};

// ********************************************************************
// ********************************************************************

// <summary> 
// FVisJones: Model solvable, time-variable multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
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
// FVisJones describes an interface for time variable and solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// FVisJones performs caching of values to minimize computation.
//
// </synopsis> 
//
// <motivation>
// The properties of an multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="2004/05/04">
//   Implement proper F algebra!
// </todo>

class FVisJones : public SolvableVisJones {
public:
  // Null constructor
  FVisJones() {};

  FVisJones(VisSet& vs);

  ~FVisJones();

  virtual Type type() { return F;};

  // Update the antenna gains from the gradients.
  virtual void updateAntGain();

};



} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <synthesis/MeasurementComponents/SolvableVisJones.cc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
