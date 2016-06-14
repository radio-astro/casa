//# FringeJones.cc: Implementation of FringeJones
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2011
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
//#                        Charlottesville, VA 22903-2475 USA
//#

#include <synthesis/MeasurementComponents/FringeJones.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MeasurementEquations/VisEquation.h>  // *
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/LatticeMath/LatticeFFT.h>
#include <scimath/Mathematics/FFTServer.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/System/Aipsrc.h>

#include <casa/sstream.h>

#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>



namespace casa { //# NAMESPACE CASA - BEGIN

// **********************************************************
//  FringeJones Implementations
//

FringeJones::FringeJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs)       // immediate parent
{
  if (prtlev()>2) cout << "FringeJones::FringeJones(vs)" << endl;
}

FringeJones::FringeJones(String msname,Int MSnAnt,Int MSnSpw) :
  VisCal(msname,MSnAnt,MSnSpw),             // virtual base
  VisMueller(msname,MSnAnt,MSnSpw),         // virtual base
  SolvableVisJones(msname,MSnAnt,MSnSpw)    // immediate parent
{
  if (prtlev()>2) cout << "FringeJones::FringeJones(msname,MSnAnt,MSnSpw)" << endl;
}

FringeJones::FringeJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt)
{
  if (prtlev()>2) cout << "FringeJones::FringeJones(nAnt)" << endl;
}

FringeJones::~FringeJones() {
  if (prtlev()>2) cout << "FringeJones::~FringeJones()" << endl;
}

void FringeJones::setApply(const Record& apply) {

  // Call parent to do conventional things
  SolvableVisJones::setApply(apply);

  if (calWt()) 
    logSink() << " (" << this->typeName() << ": Enforcing calWt()=False for phase/delay-like terms)" << LogIO::POST;

  // Enforce calWt() = False for delays
  calWt()=False;

  // Extract per-spw ref Freq for phase(delay) calculation
  //  from the CalTable
  // TBD:  revise as per refFreq decisions
  MSSpectralWindow msSpw(ct_->spectralWindow());
  ROMSSpWindowColumns msCol(msSpw);
  msCol.refFrequency().getColumn(KrefFreqs_,True);
  KrefFreqs_/=1.0e9;  // in GHz

  /// Re-assign KrefFreq_ according spwmap (if any)
  if (spwMap().nelements()>0) {
    Vector<Double> tmpfreqs;
    tmpfreqs.assign(KrefFreqs_);
    for (uInt ispw=0;ispw<spwMap().nelements();++ispw)
      if (spwMap()(ispw)>-1)
	KrefFreqs_(ispw)=tmpfreqs(spwMap()(ispw));
  }

    
}

void FringeJones::setCallib(const Record& callib,
			    const MeasurementSet& selms) {

  // Call parent to do conventional things
  SolvableVisCal::setCallib(callib,selms);

  if (calWt()) 
    logSink() << " (" << this->typeName() << ": Enforcing calWt()=False for phase/delay-like terms)" << LogIO::POST;

  // Enforce calWt() = False for delays
  calWt()=False;

  // Extract per-spw ref Freq for phase(delay) calculation
  //  from the CalTable
  KrefFreqs_.assign(cpp_->refFreqIn());
  KrefFreqs_/=1.0e9;  // in GHz

  // Re-assign KrefFreq_ according spwmap (if any)
  if (spwMap().nelements()>0) {
    Vector<Double> tmpfreqs;
    tmpfreqs.assign(KrefFreqs_);
    for (uInt ispw=0;ispw<spwMap().nelements();++ispw)
      if (spwMap()(ispw)>-1)
	KrefFreqs_(ispw)=tmpfreqs(spwMap()(ispw));
  }

    
}

void FringeJones::setSolve(const Record& solve) {

  // Call parent to do conventional things
  SolvableVisJones::setSolve(solve);

  // Trap unspecified refant:
  if (refant()<0)
    throw(AipsError("Please specify a good reference antenna (refant) explicitly."));

}

void FringeJones::calcOneJones(Vector<Complex>& /*mat*/, Vector<Bool>& /*mOk*/, 
			       const Vector<Complex>& /*par*/, const Vector<Bool>& /*pOk*/ ) {

  if (prtlev()>6) cout << "       FringeJones::calcOneJones()" << endl;

  //  TBD!!!    It will look be something like this:
  /*
      // This is the KJones version, for which nPar()=2 delays
      //   are changed to the appropriate complex numbers in
      //   the two(=nPar()) diagonal Jones matrix elements
      //   (the ipar loop here will not be appropriate for the
      //    more general fringe phase calculation, obviously)

      for (Int ipar=0;ipar<nPar();++ipar) {
	if (onePOK(ipar)) { 
	  phase=2.0*C::pi*onePar(ipar)*(currFreq()(ich)-KrefFreqs_(currSpw()));
	  oneJones(ipar)=Complex(cos(phase),sin(phase));
	  oneJOK(ipar)=True;
	}
      }
  */
  
}

void FringeJones::selfSolveOne(VisBuffGroupAcc& vbga) {

  // NB: The following meta info may be incomplete w.r.t.
  //   the contents of the VisBufferGroupAcc (esp. when 
  //   vbga.nBuf()>1); this will be remedied in the 
  //   forthcoming SolveDataBuffer version...
  
  Int nVB=vbga.nBuf();

  cout << "Scan=" << currScan()
       << ((combscan() && (nVB>1)) ? "+" : "")
       << " Time=" << ((nVB>1)?"~":"")
       << MVTime(refTime()/C::day).string(MVTime::YMD,7)
       << " Field=" << currField()
       << ((combfld() && (nVB>1)) ? "+" : "")
       << " Spw=" << currSpw()
       << ((combspw() && (nVB>1)) ? "+" : "")
       << " nVB=" << nVB
       << ": I don't know how to solve yet!" << endl;

  // Actual solve TBD!!!!!

}

} //# NAMESPACE CASA - END
