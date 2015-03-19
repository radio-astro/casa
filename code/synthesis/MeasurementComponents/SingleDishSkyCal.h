//# SingleDishSkyCal.h: Single dish sky calibration
//# Copyright (C) 2014
//# Associated Universities, Inc. Washington DC, USA.
//# National Astronomical Observatory of Japan, Japan.
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
//#
//# $Id$

//#! Create an include 'guard', containing your class name in the all
//#! upper case format implied below.  This prevents multiple inclusion
//#! of this header file during pre-processing.
//#!
//#! Note that the leading "AIPS_" identifies the package to which your class
//#! belongs.  Other packages include dish, vlbi, nfra, synthesis, atnf...
//#! If you are contributing a new class to one of these packages, be
//#! sure to replace "AIPS_" with (for instance) "DISH_" or "ATNF_".

#ifndef _SYNTHESIS_SINGLEDISH_SKY_CAL_H_
#define _SYNTHESIS_SINGLEDISH_SKY_CAL_H_

//#! Includes go here
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/MeasurementComponents/SkyCal.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations

// <summary>
//#! A one line summary of the class.   This summary (shortened a bit
//#! if necessary so that it fits along with the "ClassFileName.h" in 75
//#! characters) should also appear as the first line of this file.
//#! Be sure to use the word "abstract" here if this class is, indeed,
//#! an abstract base class.
// </summary>

// <use visibility=local>   or   <use visibility=export>
//#! If a class is intended for use by application programmers, or
//#! people writing other libraries, then specify that this class
//#! has an "export" visibility:  it defines an interface that
//#! will be seen outside of its module.  On the other hand, if the
//#! class has a "local" visibility, then it is known and used only
//#! within its module.

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
//#! for example:
//#!  <reviewed reviewer="pshannon@nrao.edu" date="1994/10/10" tests="tMyClass, t1MyClass" demos="dMyClass, d1MyClass">
//#!  This is a well-designed class, without any obvious problems.
//#!  However, the addition of several more demo programs would
//#!  go a *long* way towards making it more usable.
//#!  </reviewed>
//#!
//#! (In time, the documentation extractor will be able handle reviewed
//#! attributes spread over more than one line.)
//#!
//#! see "Coding Standards and Guidelines"  (AIPS++ note 167) and
//#! "AIPS++ Code Review Process" (note 170) for a full explanation
//#! It is up to the class author (the programmer) to fill in these fields:
//#!     tests, demos
//#! The reviewer fills in
//#!     reviewer, date
// </reviewed>

// <prerequisite>
//#! Classes or concepts you should understand before using this class.
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
// </prerequisite>
//
// <etymology>
//#! Except when it is obvious (e.g., "Array") explain how the class name
//#! expresses the role of this class.  Example: IPosition is short for
//#! "Integral Position" - a specialized integer vector for specifying
//#! array dimensions and indices.
// </etymology>
//
// <synopsis>
//#! What does the class do?  How?  For whom?  This should include code
//#! fragments as appropriate to support text.  Code fragments shall be
//#! delimited by <srcblock> </srcblock> tags.  The synopsis section will
//#! usually be dozens of lines long.
// </synopsis>
//
// <example>
//#! One or two concise (~10-20 lines) examples, with a modest amount of
//#! text to support code fragments.  Use <srcblock> and </srcblock> to
//#! delimit example code.
// </example>
//
// <motivation>
//#! Insight into a class is often provided by a description of
//#! the circumstances that led to its conception and design.  Describe
//#! them here.
// </motivation>
//
// <templating arg=T>
//#! A list of member functions that must be provided by classes that
//#! appear as actual template arguments.  For example:  imagine that you
//#! are writing a templated sort class, which does a quicksort on a
//#! list of arbitrary objects.  Anybody who uses your templated class
//#! must make sure that the actual argument class (say, Int or
//#! String or Matrix) has comparison operators defined.
//#! This tag must be repeated for each template formal argument in the
//#! template class definition -- that's why this tag has the "arg" attribute.
//#! (Most templated classes, however, are templated on only a single
//#! argument.)
//    <li>
//    <li>
// </templating>
//
// <thrown>
//#! A list of exceptions thrown if errors are discovered in the function.
//#! This tag will appear in the body of the header file, preceding the
//#! declaration of each function which throws an exception.
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//#! A List of bugs, limitations, extensions or planned refinements.
//#! The programmer should fill in a date in the "asof" field, which
//#! will usually be the date at which the class is submitted for review.
//#! If, during the review, new "todo" items come up, then the "asof"
//#! date should be changed to the end of the review period.
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>

class SingleDishSkyCal : public SolvableVisCal
{
public:

  // Constructor
  SingleDishSkyCal(VisSet& vs);
  SingleDishSkyCal(const Int& nAnt);

  // Destructor
  virtual ~SingleDishSkyCal();

  // Return the type of this calibration matrix (actual type of derived class).
  //    (Must be implemented in specializations!)
  //
  // 2015/03/19 (gmoellen): Use VisCal::M here, so that it is sorted 
  //    correctly in the VisEquation relative to "B TSYS"
  virtual Type type() { return VisCal::M; }
  
  // Return type name as string (ditto)
  virtual String typeName()     { return "SD SKY"; };
  virtual String longTypeName() { return "SD SKY (sky spectra)"; };

  // Return Matrix type
  // single dish calibration is antenna-based
  virtual VisCalEnum::MatrixType matrixType() { return VisCalEnum::JONES; };

  // Mueller matrix type (must be implemented in Mueller specializations!)
  virtual Mueller::MuellerType muellerType() { return Mueller::AddDiag2; };  

  // Return the parameter type
  // so far single dish calibration is real
  virtual VisCalEnum::VCParType parType() { return VisCalEnum::REAL; };

  // Number of pars per ant/bln
  // TODO: nPar should refer numCorr in input MS
  virtual Int nPar() { return 2; }

  // Total number of (complex) parameters per solve
  //  (specialize to jive with ant- or bln-basedness, etc.)
  virtual Int nTotalPar() { return nPar(); }

  // Does normalization by MODEL_DATA commute with this VisCal?
  //   (if so, permits pre-solve time-averaging)
  virtual Bool normalizable() { return False; }

  // Hazard a guess at the parameters (solveCPar) given the data
  virtual void guessPar(VisBuffer& vb);

  // Differentiate VB model w.r.t. Cal  parameters (no 2nd derivative yet)
  virtual void differentiate(CalVisBuffer& cvb);
  virtual void differentiate(VisBuffer& vb,        
        		     Cube<Complex>& V,     
        		     Array<Complex>& dV,
        		     Matrix<Bool>& Vflg);

  // Differentiate VB model w.r.t. Source parameters
  virtual void diffSrc(VisBuffer& vb,        
  		       Array<Complex>& dV);

  // Apply refant (implemented in SVJ)
  virtual void reReference() {};

  // Accumulate another VisCal onto this one
  virtual void accumulate(SolvableVisCal* incr,
			  const Vector<Int>& fields);

  // Determine and apply flux density scaling
  virtual void fluxscale(const String& outfile,
                         const Vector<Int>& refFieldIn,
			 const Vector<Int>& tranFieldIn,
			 const Vector<Int>& inRefSpwMap,
			 const Vector<String>& fldNames,
                         const Float& inGainThres,
                         const String& antSel,
                         const String& timerangeSel,
                         const String& scanSel,
			 fluxScaleStruct& oFluxScaleStruct,
			 const String& oListFile,
                         const Bool& incremental,
                         const Int& fitorder,
                         const Bool& display);

  // Use generic data gathering mechanism for solve
  virtual Bool useGenericGatherForSolve() { return False; };

  // Report state:
  virtual void listCal(const Vector<Int> ufldids, const Vector<Int> uantids,
		       const Matrix<Int> uchanids,  //const Int& spw, const Int& chan,
		       const String& listfile="",const Int& pagerows=50);


  // Local setApply
  virtual void setApply(const Record& apply);

  // In general, we are freq-dep
  virtual Bool freqDepPar() { return True; };
  
  // New CalTable handling
  virtual void keepNCT();

  // Self- gather and/or solve prototypes
  //  (triggered by useGenericGatherForSolve=F or useGenericSolveOne=F)
  virtual void selfGatherAndSolve(VisSet& vs, VisEquation& ve);

protected:

  // The number of sets of parameters under consideration
  // This states size of third axis of SolveAllRPar
  virtual Int& nElem() { return nAnt(); }

  // Number of Calibration matrices on ant/bln axis
  virtual Int nCalMat() { return nAnt(); }

  // Are the parameters the matrix elements? 
  //   (or is a non-trivial calculation required?)
  //    (Must be implemented in specializations!)
  virtual Bool trivialMuellerElem() { return False; };

  // Initialize solve parameters (shape)
  virtual void initSolvePar();

  // Invalidate diff cal matrices generically 
  inline virtual void invalidateDiffCalMat() {};

  // overwride syncMeta2
  virtual void syncMeta2(const vi::VisBuffer2& vb);

  // Sync matrices generically for current meta data 
  virtual void syncCalMat(const Bool& doInv=False);

  // Synchronize the differentiated calibration 
  virtual void syncDiffMat();

  // Synchronize weight scale factors
  virtual void syncWtScale();

  // Perform weight scale calculation (specializable)
  template<class ScalingScheme>
  void calcWtScale();

  // Normalize a (complex) solution array (generic)
  virtual Float calcPowerNorm(Array<Float>& amp, const Array<Bool>& ok);

  // Invalidate cal matrices generically 
  virtual void invalidateCalMat() {};

  // Row-by-row apply to a Cube<Complex> (generic)
  virtual void applyCal(VisBuffer& vb, Cube<Complex>& Vout,Bool trial=False);
  virtual void applyCal2(vi::VisBuffer2& vb, 
        		 Cube<Complex>& Vout,Cube<Float>& Wout,
        		 Bool trial=False);

  // Fill caltable by traversing MS
  // Accessor is responsible for accessing data stored either
  // in DATA or FLOAT_DATA
  template<class Accessor>
  inline void traverseMS(MeasurementSet const &ms);

  // access to current calibration data
  inline Cube<Complex> &currentSky() { return (*currentSky_[currSpw()]); };
  inline Cube<Bool> &currentSkyOK() { return (*currentSkyOK_[currSpw()]); };
  inline SkyCal<Complex, Complex> &engineC() { return (*engineC_[currSpw()]); };
  inline SkyCal<Float, Float> &engineF() { return (*engineF_[currSpw()]); };

  // arrange data selection according to calibration mode
  virtual String configureSelection() = 0;

  // current antenna
  Int currAnt_;
  Vector<Double> interval_;

  // Single Dish Calibration algebra wrapper (per Spw)
  PtrBlock<SkyCal<Complex, Complex> *> engineC_;
  PtrBlock<SkyCal<Float, Float> *> engineF_;

  // Current Sky spectra
  PtrBlock<Cube<Complex> *> currentSky_; // [nSpw]([1,2],nChanMat,nAnt)
  PtrBlock<Cube<Bool> *> currentSkyOK_;  // [nSpw]([1,2],nChanMat,nAnt)

private:
  void initializeSky();
  void finalizeSky();
  void updateWt2(Matrix<Float> &weight, const Int& antenna1);
};

class SingleDishPositionSwitchCal : public SingleDishSkyCal 
{
public:

  // Constructor
  SingleDishPositionSwitchCal(VisSet& vs);
  SingleDishPositionSwitchCal(const Int& nAnt);

  // Destructor
  virtual ~SingleDishPositionSwitchCal();

  // Return type name as string (ditto)
  virtual String typeName()     { return "SDSKY_PS"; };
  virtual String longTypeName() { return "SDSKY_PS (position switch sky subtraction)"; };

  // data selection for position switch calibration
  virtual String configureSelection();

};
  
class SingleDishRasterCal : public SingleDishSkyCal 
{
public:

  // Constructor
  SingleDishRasterCal(VisSet& vs);
  SingleDishRasterCal(const Int& nAnt);

  // Destructor
  virtual ~SingleDishRasterCal();

  // Return type name as string (ditto)
  virtual String typeName()     { return "SDSKY_RASTER"; };
  virtual String longTypeName() { return "SDSKY_RASTER (position switch sky subtraction specific to OTF raster observation)"; };

  // local setSolve
  virtual void setSolve(const Record& solve);
  
  // data selection specific to otfraster mode
  virtual String configureSelection();

private:
  // edge detection parameter for otfraster mode
  Float fraction_;
  Int numEdge_;
};

class SingleDishOtfCal : public SingleDishSkyCal 
{
public:

  // Constructor
  SingleDishOtfCal(VisSet& vs);
  SingleDishOtfCal(const Int& nAnt);

  // Destructor
  virtual ~SingleDishOtfCal();

  // Return type name as string (ditto)
  virtual String typeName()     { return "SDSKY_OTF"; };
  virtual String longTypeName() { return "SDSKY_OTF (position switch sky subtraction specific to OTF fast scan)"; };

  // data selection specific to otf mode
  virtual String configureSelection();
};

} //# NAMESPACE CASA - END

#endif /* _SYNTHESIS_SINGLEDISH_SKY_CAL_H_ */


