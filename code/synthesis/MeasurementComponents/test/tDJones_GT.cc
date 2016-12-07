//# tDJones: test polarization terms
//# Copyright (C) 2013
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>

#include <synthesis/MeasurementComponents/DJones.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
//#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
//#include <synthesis/MeasurementComponents/MSMetaInfoForCal.h>
//#include <msvis/MSVis/SimpleSimVi2.h>
//#include <msvis/MSVis/VisBuffer2.h>

#include <gtest/gtest.h>

#include "VisCalTestBase_GT.h"

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

// <summary>
// Test program for KJones-related classes
// </summary>

// Control verbosity
#define DJONES_TEST_VERBOSE false


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


// Ad hoc D-term refant application (TBD: use a DJones method)
void Dref(Cube<Complex>& D,Int refant=0,
	  Complex rval=Complex(0.0f)) {

  Int nChan=D.shape()(1);

  for (Int ich=0;ich<nChan;++ich) {
    const Complex D0=D(0,ich,refant)-rval;
    Array<Complex> Dl(D(Slice(1,1,1),Slice(ich),Slice()));
    Dl+=conj(D0);
    Array<Complex> Dr(D(Slice(0,1,1),Slice(ich),Slice()));
    Dr-=D0;
  }

}



class DJonesUNPOLarizedTest : public VisCalTestBase {

public:

  Cube<Complex> d;

  DJonesUNPOLarizedTest() :
    VisCalTestBase(1,3,1,50,4,1,10,false),   // 3 scans, UNpolarized
    d(2,nChan,nAnt,Complex(0.0))
  {
    for (Int iant=0;iant<nAnt;++iant) {
      d(Slice(0,1,1),Slice(),Slice(iant,1,1))=Complex(iant*0.01,iant*0.02);
      d(Slice(1,1,1),Slice(),Slice(iant,1,1))=Complex(iant*0.02,iant*0.01);
    }
    //summary("DJonesTest");
    //cout << "d = " << d << endl;
  }


};
  

TEST_F(DJonesUNPOLarizedTest, DfllsJonesSolveOneTest) {

  // Apply-able Dflls
  DfJones Dapp(msmc); // "<noms>",nAnt,nSpw);
  Dapp.setApply();

  for (Int ispw=0;ispw<nSpw;++ispw) { 
    Dapp.setMeta(0,1,0.0,
		 ispw,ssvp.freqs(ispw),
		 nChan);
    Dapp.sizeApplyParCurrSpw(nChan);
    
    Dapp.setApplyParCurrSpw(d,true,false);  // corrupt
  }

  DfllsJones Dsol(msmc);
  Record solvePar;
  solvePar.define("table",String("test.Df"));
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));
  Vector<Int> refant(1,0); solvePar.define("refant",refant);
  Dsol.setSolve(solvePar);

  SDBList sdbs;
  for (vi2.originChunks();vi2.moreChunks();vi2.nextChunk()) {
    for (vi2.origin();vi2.more();vi2.next()) {

      Int ispw=vb2->spectralWindows()(0);
      Int obsid(vb2->observationId()(0));
      Int scan(vb2->scan()(0));
      Double timestamp(vb2->time()(0));
      Int fldid(vb2->fieldId()(0));
      Vector<Double> freqs(vb2->getFrequencies(0));
      Vector<Int> a1(vb2->antenna1());
      Vector<Int> a2(vb2->antenna2());

      vb2->resetWeightsUsingSigma();
      vb2->setVisCubeCorrected(vb2->visCube());
      vb2->setFlagCube(vb2->flagCube());

      /*
      cout.precision(16);
      cout << "scan=" << scan 
	   << "  timestamp=" << timestamp 
	   << "  feedPa=" << vb2->feedPa(timestamp)
	   << endl;
      cout << "vCC=" << vb2->visCubeCorrected()(Slice(),Slice(0,1,1),Slice(0,1,1)) << endl;
      */

      Dapp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);
      Dapp.correct2(*vb2,false,false,false);  // (trial?,doWtSp?,dosync?)
      
      sdbs.add(*vb2);

    }
  }

  // Setup meta & sizes for the solve
  Dsol.setMeta(sdbs.aggregateObsId(),
	       sdbs.aggregateScan(),
	       sdbs.aggregateTime(),
	       sdbs.aggregateSpw(),
	       sdbs.freqs(),
	       sdbs.aggregateFld());
  Dsol.sizeSolveParCurrSpw(sdbs.nChannels()); 

  Dsol.selfSolveOne(sdbs);

  // Apply refant (ad hoc)
  Dref(Dsol.solveAllCPar(),Dsol.refant());

  Cube<Float> soldiff=amplitude(Dsol.solveAllCPar()-d);
  
  //  cout << "Dsol.solveAllCPar() = " << Dsol.solveAllCPar() << endl;
  //  cout << "Diff = " << soldiff  << endl;
  
  ASSERT_TRUE(allNearAbs(soldiff,0.0f,1e-7)); 

}

class DJonesPOLarizedTest : public VisCalTestBase {

public:

  Cube<Complex> d;

  DJonesPOLarizedTest() :
    VisCalTestBase(1,10,1,27,4,1,1,true),   // 10 scans, each with 1 times
    d(2,nChan,nAnt,Complex(0.0))
  {
    for (Int iant=0;iant<nAnt;++iant) {
      Int ii=iant+1;
      d(Slice(0,1,1),Slice(),Slice(iant,1,1))=Complex(ii*0.001,ii*0.004);
      d(Slice(1,1,1),Slice(),Slice(iant,1,1))=Complex(ii*0.002,ii*0.003);
    }
    //summary("DJonesTest");
    //cout << "d = " << d << endl;
  }


};
  


TEST_F(DJonesPOLarizedTest, DfllsJonesQUTest) {

  // Apply-able parang
  PJones Papp(msmc);
  Papp.setApply();

  // Apply-able Dflls
  DfJones Dapp(msmc);
  Dapp.setApply();

  for (Int ispw=0;ispw<nSpw;++ispw) { 
    Dapp.setMeta(0,1,0.0,
		 ispw,ssvp.freqs(ispw),
		 nChan);
    Dapp.sizeApplyParCurrSpw(nChan);
    
    Dapp.setApplyParCurrSpw(d,true,false);  // corrupt
  }

  DfllsJones Dsol(msmc);
  Record solvePar;
  solvePar.define("type",String("Df+QU"));
  solvePar.define("table",String("test.Df"));
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));

  // NB: Changing to a higher-indexed refant makes rel error test below fail... WHY?
  Vector<Int> refant(1,0); solvePar.define("refant",refant);
  Dsol.setSolve(solvePar);

  SDBList sdbs;
  for (vi2.originChunks();vi2.moreChunks();vi2.nextChunk()) {
    for (vi2.origin();vi2.more();vi2.next()) {

      Int ispw=vb2->spectralWindows()(0);
      Int obsid(vb2->observationId()(0));
      Int scan(vb2->scan()(0));
      Double timestamp(vb2->time()(0));
      Int fldid(vb2->fieldId()(0));
      Vector<Double> freqs(vb2->getFrequencies(0));
      Vector<Int> a1(vb2->antenna1());
      Vector<Int> a2(vb2->antenna2());

      vb2->resetWeightsUsingSigma();
      vb2->setVisCubeCorrected(vb2->visCube());
      vb2->setFlagCube(vb2->flagCube());

      /*
      cout.precision(16);
      cout << "scan=" << scan 
	   << "  timestamp=" << timestamp 
	   << "  feedPa=" << vb2->feedPa(timestamp)
	   << endl;
      cout << "vCC=" << vb2->visCubeCorrected()(Slice(),Slice(0,1,1),Slice(0,1,1)) << endl;
      */

      Dsol.setUpForPolSolve(*vb2);

      Dapp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);
      Dapp.correct2(*vb2,false,false,false);  // (trial?,doWtSp?,dosync?)
      

      Papp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);
      Papp.corrupt2(*vb2);
  
      sdbs.add(*vb2);

    }
  }

  // Setup meta & sizes for the solve
  Dsol.setMeta(sdbs.aggregateObsId(),
	       sdbs.aggregateScan(),
	       sdbs.aggregateTime(),
	       sdbs.aggregateSpw(),
	       sdbs.freqs(),
	       sdbs.aggregateFld());
  Dsol.sizeSolveParCurrSpw(sdbs.nChannels()); 

  if (DJONES_TEST_VERBOSE)
    Dsol.state();

  Dsol.guessPar(sdbs);

  Dsol.reportSolvedQU();
  
  if (DJONES_TEST_VERBOSE)
    cout << "srcPolPar() = " << real(Dsol.srcPolPar()) << endl;
  ASSERT_NEAR(0.1,real(Dsol.srcPolPar()(0)),1e-3);
  ASSERT_NEAR(0.0,real(Dsol.srcPolPar()(1)),1e-3);

  /*  // explore time-dep QU signal...
  for (Int i=0;i<sdbs.nSDB();++i) {
    const Complex& RL(sdbs(i).visCubeModel()(1,0,0));
    cout << i << " " << RL << " " << abs(RL) << " " << arg(RL)*180.0/C::pi << endl;
  }
  cout << endl << endl;
  */

  // Now solve for the Ds
  Dsol.selfSolveOne(sdbs);

  // Apply refant (ad hoc)
  Dref(Dsol.solveAllCPar(),Dsol.refant(),d(0,0,Dsol.refant()));


  Cube<Float> soldiff=amplitude(Dsol.solveAllCPar()-d);
  Cube<Float> reldiff=soldiff/amplitude(d);

  /*
  cout << "Dsol.refant() = " << Dsol.refant() << endl;
  cout << "d = " << d << endl;
  cout << "Dsol.solveAllCPar() = " << Dsol.solveAllCPar() << endl;
  cout << "Diff = " << soldiff  << endl;
  cout << "Diff2 = " << reldiff  << endl;
  */

  // NB: abs D-term error scales w/ D-term amplitude
  ASSERT_TRUE(allNearAbs(soldiff,0.0f,1e-3));  // abs D-terms good to 0.001 
  ASSERT_TRUE(allNearAbs(reldiff,0.0f,5e-3));  // REL D-terms goood to 0.5% (of their own amp)

}



class XJonesTest : public VisCalTestBase {

public:

  Cube<Complex> x;

  XJonesTest() :
    VisCalTestBase(1,5,1,4,4,64,1,true),
    x(1,nChan,nAnt,Complex(0.0))
  {
    for (Int ich=0;ich<nChan;++ich) {
      Float a((C::pi/4)*cos(ich*C::pi/90.0));
      x(Slice(),Slice(ich,1,1),Slice())=Complex(cos(a),sin(a));
    }

    //summary("XJonesTest");
    //cout << "x = " << phase(x)*180/C::pi << endl;
  }


};
  

TEST_F(XJonesTest, XfJonesTest) {

  // Apply-able parang
  PJones Papp(msmc);
  Papp.setApply();

  // Apply-able Xf
  XfJones Xapp(msmc);
  Xapp.setApply();

  for (Int ispw=0;ispw<nSpw;++ispw) { 
    Xapp.setMeta(0,1,0.0,
		 ispw,ssvp.freqs(ispw),
		 nChan);
    Xapp.sizeApplyParCurrSpw(nChan);
    
    Xapp.setApplyParCurrSpw(x,true,false);  // correct below will corrupt
  }

  XfJones Xsol(msmc);
  Record solvePar;
  solvePar.define("table",String("test.Xf"));
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));
  Vector<Int> refant(1,0); solvePar.define("refant",refant);
  Xsol.setSolve(solvePar);

  SDBList sdbs;
  for (vi2.originChunks();vi2.moreChunks();vi2.nextChunk()) {
    for (vi2.origin();vi2.more();vi2.next()) {

      Int ispw=vb2->spectralWindows()(0);
      Int obsid(vb2->observationId()(0));
      Int scan(vb2->scan()(0));
      Double timestamp(vb2->time()(0));
      Int fldid(vb2->fieldId()(0));
      Vector<Double> freqs(vb2->getFrequencies(0));
      Vector<Int> a1(vb2->antenna1());
      Vector<Int> a2(vb2->antenna2());

      vb2->resetWeightsUsingSigma();

      Cube<Complex> vC(vb2->visCube());
      vb2->setVisCubeCorrected(vC);
      vb2->setFlagCube(vb2->flagCube());

      Xapp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);
      Xapp.correct2(*vb2,false,false,false);  // (trial?,doWtSp?,dosync?)

      /*
      cout << "vCC=" << Vector<Complex>(vb2->visCubeCorrected()(Slice(),Slice(nChan/2),Slice(0))) << endl;
      cout << "vCM=" << Vector<Complex>(vb2->visCubeModel()(Slice(),Slice(nChan/2),Slice(0))) << endl;
      */

      Papp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);
      Papp.corrupt2(*vb2);
      //cout << "vCM=" << Vector<Complex>(vb2->visCubeModel()(Slice(),Slice(nChan/2),Slice(0))) << endl;

      // Add hoc divideCorrByModel (normally done by VisEquation)
      // NB: formally, should also do weights, but doesn't matter here (point source)
      Cube<Complex> vCC(vb2->visCubeCorrected());
      Cube<Complex> vCM(vb2->visCubeModel());
      vCC/=vCM;
      vCM.set(Complex(1.0));

      /*
      cout << "vCC=" << Vector<Complex>(vb2->visCubeCorrected()(Slice(),Slice(nChan/2),Slice(0))) 
	   << Vector<Float>(phase(vb2->visCubeCorrected()(Slice(),Slice(nChan/2),Slice(0)))*180.0f/C::pi)
	   << endl;
      cout << "vCM=" << Vector<Complex>(vb2->visCubeModel()(Slice(),Slice(nChan/2),Slice(0))) << endl;
      */

      sdbs.add(*vb2);

      //cout  << endl << endl;
    }
  }

  // Setup meta & sizes for the solve
  Xsol.setMeta(sdbs.aggregateObsId(),
	       sdbs.aggregateScan(),
	       sdbs.aggregateTime(),
	       sdbs.aggregateSpw(),
	       sdbs.freqs(),
	       sdbs.aggregateFld());
  Xsol.sizeSolveParCurrSpw(sdbs.nChannels()); 

  // Call the specialized solver
  Xsol.selfSolveOne(sdbs);
  
  Cube<Float> soldiff=amplitude(Xsol.solveAllCPar()-x);
  
  //cout << "Xsol.solveCPar() = " << phase(Xsol.solveAllCPar())*180/C::pi << endl;
  //cout << "Diff = " << soldiff  << endl;
  
  ASSERT_TRUE(allNearAbs(soldiff,0.0f,1e-7)); 



}
