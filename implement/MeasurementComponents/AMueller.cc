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

void AMueller::corrupt(VisBuffer& vb) {

  if (prtlev()>3) cout << "  A::corrupt()" << endl;

  // Initialize model data to zero, so corruption contains
  //  only the AMueller solution
  //  TBD: may wish to make this user togglable.
  vb.setModelVisCube(Complex(0.0));

  // Call general version:
  VisMueller::corrupt(vb);

}

Int ANoise::setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes)
{
  prtlev()=4; // debug

  if (prtlev()>2) cout << "   AN::setupSim()" << endl;

  // This method only called in simulate context!
  AlwaysAssert((isSimulated()),AipsError);

  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);

  if (prtlev()>3) cout << " sized for Sim." << endl;

  acorruptor_p = new ANoiseCorruptor();
  corruptor_p = acorruptor_p;

  if (prtlev()>3) cout << " ANCorruptor created." << endl;

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

  return nSim;
}




Bool ANoise::simPar(VisBuffGroupAcc& vbga) {

  if (prtlev()>2) cout << "  AN::simPar()" << endl;  

  // This method only called in simulate context!
  // does AMueller have isSimulated?  
  AlwaysAssert((isSimulated()),AipsError);

  try {
    
    for (Int ivb=0;ivb<vbga.nBuf();++ivb) {
      VisBuffer& svb(vbga(ivb));   

      Vector<Int>& a1(svb.antenna1());
      Vector<Int>& a2(svb.antenna2());

      // corruptor_p->currSpw()=svb.spectralWindow(); // same for entire vbga right?
      solveCPar()=Complex(1.0);
      solveParOK()=False;

      for (Int irow=0;irow<svb.nRow();++irow) {
	
	if ( !svb.flagRow()(irow) &&
             svb.antenna1()(irow)!=svb.antenna2()(irow) &&
             nfalse(svb.flag().column(irow))> 0 ) {  
	  
	  // in T, there's a loop to find the corruptor time slot here. 
	  // since this is baseline-based random noise, it doesn't 
	  // need a fixed sequence in corruptor, but just the noise params 
	  // and can draw from the distribution every time. 

	  // could have corruptor return different Tsys according to different 
	  // antennas, e.g. for a heterogeneous array
	  // corruptor_p->currAnt1()=a1(irow);
	  // corruptor_p->currAnt2()=a2(irow);

	  // figure out shape of viscube - from MM::selfSolve: 
	  // Insist that channel,row shapes match
	  IPosition visshape(svb.visCube().shape());
	  AlwaysAssert(solveCPar().shape().getLast(2)==visshape.getLast(2),AipsError);
	  
	  // Zero flagged data
	  IPosition vblc(3,0,0,0);
	  IPosition vtrc(visshape);  vtrc-=1;      
	  Int nCorr(visshape(0));
	  for (Int i=0;i<nCorr;++i) {
	    vblc(0)=vtrc(0)=i;
	    svb.visCube()(vblc,vtrc).reform(visshape.getLast(2))(svb.flag())=Complex(1.0);
	  }
	  	  
	  // initSolvePar: solveCPar().resize(nPar(),nChanPar(),nBln());

//	  Complex factor(1.0);
//	  if (acorruptor_p->mode() == "calc") {
//	    // live dangerously: assume all vis have the same tint
//	    Double tint = svb.msColumns().exposure()(0);  
//	    Int iSpW = svb.spectralWindow();
//	    Float deltaNu = svb.msColumns().spectralWindow().totalBandwidth()(iSpW) / 
//	      Float(svb.msColumns().spectralWindow().numChan()(iSpW));	    
//	    factor = Complex(1./sqrt( deltaNu * tint ));
//	  }
	  // 20090922 moved dnu dt to the MMueller part of adding noise.
//	  solveCPar().xyPlane(irow) = 
//	    ( acorruptor_p->noise(solveCPar().nrow(),solveCPar().ncolumn()) )* factor;

	  // RI TODO do we need a switch (nCorr)?
	  solveCPar().xyPlane(irow) = 
	    acorruptor_p->noise(solveCPar().nrow(),solveCPar().ncolumn());

	  solveParOK().xyPlane(irow) = True;
	  //	  cout << irow << ": " << a1(irow) << "&" << a2(irow) << solveCPar().xyPlane(irow)[0,0] << endl;
	  
        }
      }
    }
  } catch (AipsError x) {
    if (corruptor_p) delete corruptor_p;
    cout << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}


Array<Complex> ANoiseCorruptor::noise(const Int nrow,const Int ncol) {
  Matrix<Complex> foo(nrow,ncol);
  for (Int j=0;j<ncol;j++)
    for (Int i=0;i<nrow;i++) {
      //foo(i,j,k) = Complex((*nDist_p)(),(*nDist_p)())/amp();
      foo(i,j).real() = (*nDist_p)()*amp();
      foo(i,j).imag() = (*nDist_p)()*amp();
    }
  return foo;
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

ANoiseCorruptor::ANoiseCorruptor(): CalCorruptor(0) {};

ANoiseCorruptor::~ANoiseCorruptor() {};

} //# NAMESPACE CASA - END
