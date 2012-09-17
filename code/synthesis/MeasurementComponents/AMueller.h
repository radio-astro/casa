//# AMueller.h:  Additive closure errors
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

#ifndef SYNTHESIS_AMUELLER_H
#define SYNTHESIS_AMUELLER_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/CalCorruptor.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class VisBuffGroupAcc;

class AMueller : public MMueller {
public:

  // Constructor
  AMueller(VisSet& vs);
  AMueller(const Int& nAnt);

  virtual ~AMueller();

  // Return the type enum
  virtual Type type() { return VisCal::A; };

  // Return type name as string
  virtual String typeName()     { return "A Mueller"; };
  virtual String longTypeName() { return "A Mueller (baseline-based)"; };

  // Algebraic type of Mueller matrix 
  //  (this is the key distinguishing characteristic)
  virtual Mueller::MuellerType muellerType() { return Mueller::AddDiag2; };

  // Local setsolve (overrides preavg)
  using SolvableVisCal::setSolve;
  // Parameters particular to this class:
  //    fitorder: Order of the polynomial fit.  If 0, it is just an average.
  virtual void setSolve(const Record& solvepar);

  // AMueller's caltables have polynomial orders where channels would normally
  // go.  setSolve() above sets the number of "channels", but
  // SolvableVisCal::setSolveChannelization() would just reset it to 1 if it
  // was not overrode here.
  virtual void setSolveChannelization(VisSet& vs);

  // Size up the solving arrays, etc.  (supports combine)
  virtual Int sizeUpSolve(VisSet& vs, Vector<Int>& nChunkPerSol);

  // The fitorder = 0 version (in M) skips LinearFitSVD by just averaging.
  virtual Bool useGenericGatherForSolve() {return fitorder_p != 0;}

  // Only called if useGenericGatherForSolve() == True.  If
  // useGenericGatherForSolve() == True, then genericGatherAndSolve() will call
  // AMueller's selfSolveOne().  Otherwise MMueller's selfGatherAndSolve() will
  // do everything.
  virtual Bool useGenericSolveOne() {return False;}

  // Per-solution self-solving inside generic gather.  Flexible enough for
  // fitorder != 0, but overkill otherwise.
  virtual void selfSolveOne(VisBuffGroupAcc& vbga);

  virtual void storeNCT();

  virtual void setApply() {SolvableVisCal::setApply();}
  virtual void setApply(const Record& applypar);

  // Apply this calibration to vb.  AMueller does NOT support trial
  //                                and ignores it!
  virtual void applyCal(VisBuffer& vb, Cube<Complex>& Vout, Bool trial=False);

  // Freq dependence
  virtual Bool freqDepPar() { return False; };
  virtual Bool freqDepMat() {
    return fitorder_p != 0 || nChanPar() > 1; // The latter is for applying.
  }

  // We do not normalize by the model, since we are estimating
  //  directly from the data  (we should optimize here by avoiding 
  //  the model I/O)
  virtual Bool normalizable() {return False;};

  // Specialize corrupt to pre-zero model for corruption
  //using VisMueller::corrupt;
  virtual void corrupt(VisBuffer& vb);
  //virtual void corrupt(VisBuffer& vb, Cube<Complex>& Mout);

  // Set (repeatedly, unfortunately) whether or not subtraction is being done,
  // and IF fitorder == 0, sync matrices for current meta data (VisMueller
  // override).  (Mueller matrices aren't used for fitorder != 0.)
  void syncCalMat(const Bool& doInv)
  {
    doSub_p = doInv;
    if(fitorder_p == 0)
      VisMueller::syncCalMat(doInv);
  }

protected:
  virtual Int nPar() {
    if(nCorr_p < 0)
      hurl("nPar()", "nPar() called before being set.");
    return nCorr_p;
  }

private:
  void init();  // Common code for the c'tors.

  // Logs and throws msg as an exception from origin.
  void hurl(const String& origin, const String& msg);

  // Initialized to 0 in the initialization lists of the c'tors.
  Int fitorder_p;  // Stores the order of the fitted polynomials.

  Bool doSub_p; // For apply, whether or not to subtract or give the continuum
                // estimate.
  Int  nCorr_p; // # of correlations.  -1 if not yet known.

  // Resized and set to impossible values in init().
  Vector<Double> lofreq_p; // Lowest and highest frequencies (Hz) used
  Vector<Double> hifreq_p; // to make the fit.
  Vector<uInt> totnumchan_p; // The total number of input channels that will be
                             // looked at (including masked ones!)
  Vector<Bool> spwApplied_p;  // Just keeps track of which spws have been
                              // applied to.
};

// Additive noise
// In practice, this is not really solvable, but it
//   is a SVM because we need access to general simulation methods
class ANoise : public SolvableVisMueller {
public:

  // Constructor
  ANoise(VisSet& vs);
  ANoise(const Int& nAnt);

  virtual ~ANoise();

  // Return the type enum
  virtual Type type() { return VisCal::ANoise; };

  // Return type name as string
  virtual String typeName()     { return "A Noise"; };
  virtual String longTypeName() { return "A Noise (baseline-based)"; };

  // Algebraic type of Mueller matrix 
  //  (this is the key distinguishing characteristic)
  virtual Mueller::MuellerType muellerType() { return Mueller::AddDiag2; };

  // Overide solvability
  virtual Bool isSolvable() { return False; };

  // this is inherently freqdep:
  virtual Bool freqDepPar() { return True; };

  virtual void createCorruptor(const VisIter& vi, const Record& simpar, const Int nSim);

protected:
  // umm... 2 for each of parallel hands?
  virtual Int nPar() { return 2; };

  // Jones matrix elements are trivial
  virtual Bool trivialMuellerElem() { return (!simOnTheFly()); };

  // override VC default of timeDepMat=F for OTF simulatio:
  virtual Bool timeDepMat() { return simOnTheFly(); };

  // Calculate an ensemble of Mueller matrices (all baselines, channels)
  // overrriding VisCal::calcAllMueller
  virtual void calcAllMueller();

  // Calculate a single Mueller matrix by some means
  // override SolvableVisMueller::calcOneMueller
  virtual void calcOneMueller(Vector<Complex>& mat, Vector<Bool>& mOk,
			      const Vector<Complex>& par, const Vector<Bool>& pOk);

private:
  ANoiseCorruptor *acorruptor_p;

};

} //# NAMESPACE CASA - END

#endif
