//# AMueller.cc: Implementation of AMueller
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#include <synthesis/MeasurementComponents/CalCorruptor.h>
#include <synthesis/MeasurementComponents/AMueller.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <scimath/Fitting/LSQFit.h>


namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  AMueller
//


AMueller::AMueller(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  MMueller(vs)            // immediate parent
{
  if (prtlev()>2) cout << "A::A(vs)" << endl;
}

AMueller::AMueller(const Int& nAnt) :
  VisCal(nAnt),
  VisMueller(nAnt),
  MMueller(nAnt)
{
  if (prtlev()>2) cout << "A::A(nAnt)" << endl;
}

AMueller::~AMueller() {
  if (prtlev()>2) cout << "A::~A()" << endl;
}

void AMueller::setSolve(const Record& solvepar) {

  // Call parent
  MMueller::setSolve(solvepar);

  // Override preavg 
  // (solver will fail if we don't average completely in each solint)
  preavg()=DBL_MAX;

}

void AMueller::corrupt(VisBuffer& vb) {

  if (prtlev()>3) cout << "  A::corrupt()" << endl;

  // Initialize model data to zero, so corruption contains
  //  only the AMueller solution
  //  TBD: may wish to make this user togglable.
  vb.setModelVisCube(Complex(0.0));

  // Call general version:
  VisMueller::corrupt(vb);

}

void ANoise::createCorruptor(const VisIter& vi, const Record& simpar, const Int nSim)
{
  if (prtlev()>2) cout << " AN::createCorruptor()" << endl;
  AlwaysAssert((isSimulated()),AipsError);

  acorruptor_p = new ANoiseCorruptor();
  corruptor_p = acorruptor_p;

  // call generic parent to set corr,spw,etc info
  SolvableVisCal::createCorruptor(vi,simpar,nSim);

  Int Seed(1234);
  if (simpar.isDefined("seed")) {    
    Seed=simpar.asInt("seed");
  }

  Float Amp(1.0);
  if (simpar.isDefined("amplitude")) {    
    Amp=simpar.asFloat("amplitude");
  }

  acorruptor_p->initialize(Seed,Amp);

  String Mode("calc"); // calc means multiply by 1/sqrt(dnu dt)
  if (simpar.isDefined("mode")) {    
    Mode=simpar.asString("mode");
  }

  acorruptor_p->mode()=Mode;

  if (prtlev()>2) cout << " ~AN::createCorruptor()" << endl;

}


ANoise::ANoise(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisMueller(vs)  // immediate parent
{
  if (prtlev()>2) cout << "ANoise::ANoise(vs)" << endl;
}

ANoise::ANoise(const Int& nAnt) :
  VisCal(nAnt),
  VisMueller(nAnt),
  SolvableVisMueller(nAnt)
{
  if (prtlev()>2) cout << "ANoise::ANoise(nAnt)" << endl;
}

ANoise::~ANoise() {
  if (prtlev()>2) cout << "ANoise::~ANoise()" << endl;
}



void ANoise::calcOneMueller(Vector<Complex>& mat, Vector<Bool>& mOk,
			    const Vector<Complex>& par, const Vector<Bool>& pOk) {
  
  if (prtlev()>10) cout << "        AN::calcOneMueller()" << endl;

  // If Mueller matrix is trivial, shouldn't get here
  if (trivialMuellerElem()) 
    throw(AipsError("Trivial Mueller Matrix logic error."));
  else {
    Int len=0;
    mat.shape(len);
    for (Int i=0; i<len; i++) {
      mat[i]=acorruptor_p->simPar(); // single complex #
      mOk[i]=True;
    }    
  }
}



} //# NAMESPACE CASA - END
