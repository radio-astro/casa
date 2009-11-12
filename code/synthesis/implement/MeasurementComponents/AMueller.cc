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




Bool ANoise::simPar(VisIter& vi, const Int nChunks){
  if (prtlev()>4) cout << "  AN::simPar()" << endl;  

  AlwaysAssert((isSimulated()),AipsError);

  try {
    
    Vector<Int> a1;
    Vector<Int> a2;
    Matrix<Bool> flags;  // matrix(chan,row) - if want pol send cube(pol,chn,row)
    solveCPar()=Complex(0.0);
    // n good VI elements averaged in each CPar() entry:
    IPosition cparshape=solveCPar().shape();
    Cube<Complex> nGood(cparshape);
    solveParOK()=False;

    Vector<Double> timevec;
    Double starttime,stoptime;
    Vector<int> scntmp;
    Int ibln;
    starttime=vi.time(timevec)[0];
    Vector<uInt> rowids;

    for (Int ichunk=0;ichunk<nChunks;++ichunk) {
      Int spw(vi.spectralWindow());	
      // corruptor_p->currSpw()=svb.spectralWindow(); 

      for (vi.origin(); vi.more(); vi++) {

	// these need to be updated whenever the VI scrolls forward
	vi.antenna1(a1);
	vi.antenna2(a2);
	vi.flag(flags);

	Int scan(vi.scan(scntmp)[0]);
	// not ness right here but for debugging yes
	vi.time(timevec);

	for (Int irow=0;irow<vi.nRow();++irow)		  
	  if (nfalse(flags.column(irow)) > 0 ) {   
	    ibln=blnidx(a1(irow),a2(irow)); // baseline id.

	    vi.rowIds(rowids);
	    if ((irow<1 or irow>1223) and scan<2 and prtlev()>2)
	      cout << "row " << irow << "/" << vi.nRow() << " chunk " << ichunk << "/" << nChunks << " ants:" << a1(irow) << "&" <<a2(irow) << " = " << ibln << " ms rowid " << rowids[0] << " rel time " << timevec[0]-starttime << endl;	   

	    if ( a1(irow)==a2(irow) ) {
	      solveCPar().xyPlane(ibln) = Complex(0.0);
	      nGood.xyPlane(ibln) = nGood.xyPlane(ibln) + Complex(1.);	    
	      solveParOK().xyPlane(ibln) = True;	    
	    } else {
	      // in T, there's a loop to find the corruptor time slot here.
	      solveCPar().xyPlane(ibln) = solveCPar().xyPlane(ibln) + 
		acorruptor_p->noise(solveCPar().nrow(),solveCPar().ncolumn());
	      nGood.xyPlane(ibln) = nGood.xyPlane(ibln) + Complex(1.);	    
	      solveParOK().xyPlane(ibln) = True;	    
	    }
	  }
      }
      if (vi.moreChunks()) vi.nextChunk();
    }
    
    // RI TODO AN::simPar  don't divide by zero  **
    for (Int i=0;i<cparshape(2);i++)
      solveCPar().xyPlane(i) =  solveCPar().xyPlane(i) / nGood.xyPlane(i);

    vi.time(timevec);
    Int tvsize;
    timevec.shape(tvsize);
    stoptime=timevec[tvsize-1];
    refTime() = 0.5*(starttime+stoptime);
    interval() = (stoptime-starttime);
    currField() = vi.fieldId();

  } catch (AipsError x) {
    if (corruptor_p) delete corruptor_p;
    cout << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
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

} //# NAMESPACE CASA - END
