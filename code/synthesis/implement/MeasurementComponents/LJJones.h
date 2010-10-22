//# LJJones.h: Declaration of LJJones (Solvable)VisCal type
//# Copyright (C) 1996,1997,2000,2001,2002,2003
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

#ifndef SYNTHESIS_LJJONES_H
#define SYNTHESIS_LJJONES_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
//#include <synthesis/MeasurementComponents/nPBWProjectFT.h>
#include <synthesis/MeasurementComponents/GridFT.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageInfo.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/VisSet.h>
#include <casa/OS/Timer.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declaration
class VisEquation;
class nPBWProjectFT;

// **********************************************************
//  LJJones (pointing errors)
//

class LJJones : public SolvableVisJones {
public:

  // Constructor
  LJJones(VisSet& vs);
	 //  {throw(AipsError("Use the constructor LJJones(VisSet&, MeasurementSet&) instead"));};
  LJJones(VisSet& vs, MeasurementSet& ms);
  //   LJJones(const Int& nAnt);  // NYI

  virtual ~LJJones();

  // Return the type enum
  virtual Type type() { return VisCal::E; };

  // Return type name as string
  virtual String typeName()     { return "LJ Jones"; };
  virtual String longTypeName() { return "LeakyJ Jones"; };
  // Frequency-dependent Parameters?  Nominally not.
  virtual Bool freqDepPar() { return False; };

  // Type of Jones matrix according to nPar()
  Jones::JonesType jonesType() { return Jones::General; };
  
  virtual VisCalEnum::VCParType parType() { return setParType(VisCalEnum::COMPLEX);};

  // Specialized access to pointing parameters (no chan axis)
  Cube<Complex>& loadPar();

  virtual void setModel(const String& modelImage);
  // Set the solving parameters
  virtual void setSolve();
  virtual void setSolve(const Record& solve);
  virtual void setNiter(const Int& niter) {niter_p=niter;}
  virtual void setTolerance(const Float& tol) {tolerance_p = tol;}
  virtual void setGain(const Float& gain) {gain_p = gain;}

  // Arrange to apply (corrupt only)
  using SolvableVisCal::setApply;
  virtual void setApply(const Record& applypar);

  // Apply calibration to a VisBuffer 
  virtual void applyCal(VisBuffer& vb, 
			Cube<Complex>& Mout);

  // Differentiate a VisBuffer w.r.t. pointng parameters
  //
  // These effectively compute residuals and derivatives for
  // a time-averaged VisBuffer
  //
  virtual void differentiate(VisBuffer& vb,
			     Cube<Complex>& Mout,
			     Array<Complex>& dMout,
			     Matrix<Bool>& Mflg);
  // Differentiate a VisBuffer w.r.t. pointng parameters
  virtual void differentiate(VisBuffer& vb,
			     VisBuffer& dvb0,
			     VisBuffer& dvb1,
			     Matrix<Bool>& Mflg);
  //
  // This one averages the residuals and the derivatives in time.
  //
  virtual void diffResiduals(VisIter& vi, VisEquation& ve,
			    VisBuffer& residuals, 
			    VisBuffer& dVr1, 
			    VisBuffer& dVr2, 
			    Matrix<Bool>& flags);  
  // Guess (throws error because we don't yet solve for this)
  virtual void guessPar(VisBuffer& vb);
  virtual void guessPar() {pointPar_=0;}
  
  virtual Cube<Complex>& solvePar() {return pointPar_;}
  virtual void setRPar(Cube<Complex>& val) {pointPar_.resize(val.shape());pointPar_=val;}
  virtual void setRPar(Double val) {pointPar_=val;}
  
  //  virtual void keep(const Int& slot);

  virtual Bool normalizable() { return False; };

  //  virtual BaseCalSet& cs() {return *cs_;};
  virtual void keep(const Int& slot);
  
  inline virtual CalSet<Complex>& cs() {return *cs_;};

  virtual void inflate(const Vector<Int>& nChan,
		       const Vector<Int>& startChan,
		       const Vector<Int>& nSlot);
  void initSolve(VisSet& vs);
  void initSolvePar();
  void store();
  void store(const String& table,const Bool& append);
  Bool verifyForSolve(VisBuffer& vb);
  virtual void postSolveMassage(const VisBuffer&);
  virtual void selfSolve(VisSet& vs,VisEquation& ve);
  virtual Bool standardSolve() {return False;};
  virtual Float printFraction(const Int& nSlots) {return 0.1;};
  Array<Complex> getOffsets(const Int& spw) {return cs().par(spw);}
  Array<Double> getTime(const Int& spw) {return cs().time(spw);}
  void nearest(const Double time, Array<Complex>& vals);
  void printRPar();  

  void setByPassMode(Int& b) {byPass_p = b;}
  void getAvgVB(VisIter& vi, VisEquation& ve, VisBuffer& vb);
protected:

  // EP has a pair of real parameters per feed
  virtual Int nPar() { return 4; };

  // Jones matrix elements are NOT trivial
  virtual Bool trivialJonesElem() { return False; };

  // Fill-in a complex grid with the image values in prepration for 
  // computing it's FT
  virtual void makeComplexGrid(TempImage<Complex>& Grid, 
					PagedImage<Float>& ModelImage,
					VisBuffer& vb);
  void printActivity(const Int slotNo, const Int fieldId, const Int spw, const Int nSolutions);
  //  inline virtual CalSet<Float> cs() {return *cs_;}

  void reformVisibilities(const VisBuffer& vb, const Int whichChan, const Int whichPol,
			  Array<Complex>& visArray, Array<Float>& visWts);

  Int getRefAnt(const Array<Float>& visWts);
private:

  // Local Matrix for referencing pointing pars in a convenient way
  Cube<Complex>  pointPar_;
  GridFT *pbwp_p;
  MeasurementSet *ms_p;
  VisSet *vs_p;
  //  Array<Float> azOff, elOff;
  TempImage<Complex> targetVisModel_;
  CalSet<Complex> *cs_;
  Double maxTimePerSolution, minTimePerSolution, avgTimePerSolution;
  Timer timer;
  Vector<Int> polMap_p;
  Float tolerance_p, gain_p;
  Int niter_p,byPass_p;
  String modelImageName_p;
};





} //# NAMESPACE CASA - END

#endif

