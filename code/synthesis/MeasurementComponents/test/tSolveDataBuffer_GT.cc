//# tSolveDataBuffer.cc: Tests the SolveDataBuffer
//# Copyright (C) 1995,1999,2000,2001
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
//# $Id$

//#include <casa/aips.h>
//#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/Timer.h>

#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/VisCalSolver2.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
//#include <casa/Quanta/MVTime.h>
//#include <casa/iomanip.h>
//#include <casa/namespace.h>


#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

class SolveDataBufferTest : public ::testing::Test {

public:
  SolveDataBufferTest() :
    nAnt(5),
    nCorr(4),
    nChan(2),
    nRow(nAnt*(nAnt-1)/2),
    a1(nRow,-1),
    a2(nRow,-1),
    c0(9,1.0)
 {
   VisBuffer2 *vb0 = VisBuffer2::factory(VbPlain, VbRekeyable);
   
   // The basic shape
   vb0->setShape(nCorr,nChan,nRow);
   
   // Meta info
   vb0->setObservationId(Vector<Int>(nRow,0));    // obsid=0
   vb0->setArrayId(Vector<Int>(nRow,0));          // arrid=0
   vb0->setScan(Vector<Int>(nRow,1));             // scan #1
   vb0->setFieldId(Vector<Int>(nRow,1));          // fieldid=0
   vb0->setTime(Vector<Double>(nRow,1e9));        // time
   vb0->setTimeCentroid(Vector<Double>(nRow,1e9));        // time
   vb0->setDataDescriptionIds(Vector<int>(nRow,0));  // ddid=0
   vb0->setSpectralWindows(Vector<int>(nRow,0));  // spw=0
   Int irow(0);
   for (Int i=0;i<nAnt-1;++i) {
     for (Int j=i+1;j<nAnt;++j,irow++) {
       a1(irow)=i;
       a2(irow)=j;
     }
   }
   vb0->setAntenna1(a1);
   vb0->setAntenna2(a2);
   
   // Data, etc.
   vb0->setFlagRow(Vector<Bool>(nRow,False));  // unflagged
   vb0->setFlagCube(Cube<Bool>(nCorr,nChan,nRow,False));  // unflagged
   vb0->setWeightSpectrum(Cube<Float>(nCorr,nChan,nRow,1.0));  // all wt=1
   vb0->setWeight(Matrix<Float>(nCorr,nRow,1.0));  // all wt=1
   vb0->setSigma(Matrix<Float>(nCorr,nRow,1.0));  // all wt=1

   Cube<Complex> mod(nCorr,nChan,nRow,Complex(1.0));
   mod(Slice(1,2,1),Slice(),Slice())=Complex(0.0); // zero cross-hand model
   vb0->setVisCubeModel(mod);
  
   Cube<Complex> cb(nCorr,nChan,nRow,Complex(0.0));
   cb(Slice(0,1,1),Slice(),Slice())=c0;
   cb(Slice(nCorr-1,1,1),Slice(),Slice())=conj(c0);
   vb0->setVisCubeCorrected(cb);
   
   // Make the SDBs
   sdbs.add(*vb0);

  }

  Int nAnt, nCorr, nChan, nRow;
  Vector<Int> a1,a2;
  Complex c0;
  SDBList sdbs;

};



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST_F(SolveDataBufferTest, SolveDataBufferState) {
  
  //cout << "sdbs.nSDB() = " << sdbs.nSDB() << endl;
  ASSERT_EQ(1,sdbs.nSDB());

  // Refer to first SDB in the SDBList
  SolveDataBuffer& sdb(sdbs(0));
  sdbs.enforceAPonData("AP");
  sdbs.enforceSolveWeights(true); // p-hands only

  //cout << "sdb.nRows() = " << sdb.nRows() << endl;
  ASSERT_EQ(nRow,sdb.nRows());

  //  cout << "sdb.arrayId() = " << sdb.arrayId() << endl;
  ASSERT_TRUE(allEQ(sdb.arrayId(),0));

  //cout << "sdb.antenna1() = " << sdb.antenna1() << endl;
  //cout << "sdb.antenna2() = " << sdb.antenna2() << endl;
  ASSERT_TRUE(allEQ(sdb.antenna1(),a1));
  ASSERT_TRUE(allEQ(sdb.antenna2(),a2));
  
  //cout << "sdb.dataDescriptionIds() = " << sdb.dataDescriptionIds() << endl;
  ASSERT_TRUE(allEQ(sdb.dataDescriptionIds(),0));

  //cout << "sdb.spectralWindow() = " << sdb.spectralWindow() << endl;
  ASSERT_TRUE(allEQ(sdb.spectralWindow(),0));

  //cout << "sdb.time() = " << sdb.time() << endl;
  ASSERT_TRUE(allEQ(sdb.time(),1e9));
  //cout << "sdb.fieldId() = " << sdb.fieldId() << endl;
  ASSERT_TRUE(allEQ(sdb.fieldId(),1));
  //cout << "sdb.scan() = " << sdb.scan() << endl;
  ASSERT_TRUE(allEQ(sdb.scan(),1));

  //cout << "sdb.nChannels() = " << sdb.nChannels() << endl;
  ASSERT_EQ(nChan,sdb.nChannels());
  //cout << "sdb.nCorrelations() = " << sdb.nCorrelations() << endl;
  ASSERT_EQ(nCorr,sdb.nCorrelations());
  
  //cout << boolalpha;
  //cout << "sdb.flagRow() = " << sdb.flagRow() << endl;
  ASSERT_EQ(IPosition(1,nRow),sdb.flagRow().shape());
  ASSERT_EQ(0,sum(sdb.flagRow()));

  //cout << "sdb.flagCube() = " << sdb.flagCube() << endl;
  ASSERT_EQ(IPosition(3,nCorr,nChan,nRow),sdb.flagCube().shape());
  ASSERT_EQ(0,sum(sdb.flagCube()));

  //cout << "sdb.weightSpectrum() = " << sdb.weightSpectrum() << endl;
  ASSERT_EQ(IPosition(3,nCorr,nChan,nRow),sdb.weightSpectrum().shape());
  ASSERT_TRUE(allEQ(sdb.weightSpectrum()(Slice(0,2,3),Slice(),Slice()),1.0f));
  ASSERT_TRUE(allEQ(sdb.weightSpectrum()(Slice(1,2,1),Slice(),Slice()),0.0f));

  //  cout << "sdb.visCubeModel() = " << sdb.visCubeModel() << endl;
  ASSERT_EQ(IPosition(3,nCorr,nChan,nRow),sdb.visCubeModel().shape());
  ASSERT_TRUE(allEQ(sdb.visCubeModel()(Slice(0,2,3),Slice(),Slice()),Complex(1.0)));
  ASSERT_TRUE(allEQ(sdb.visCubeModel()(Slice(1,2,1),Slice(),Slice()),Complex(0.0)));

  //cout << "sdb.visCubeCorrected() = " << sdb.visCubeCorrected() << endl;
  ASSERT_EQ(IPosition(3,nCorr,nChan,nRow),sdb.visCubeCorrected().shape());
  ASSERT_TRUE(allEQ(sdb.visCubeCorrected()(Slice(0,1,1),Slice(),Slice()),c0));
  ASSERT_TRUE(allEQ(sdb.visCubeCorrected()(Slice(3,1,1),Slice(),Slice()),conj(c0)));
  ASSERT_TRUE(allEQ(sdb.visCubeCorrected()(Slice(1,2,1),Slice(),Slice()),Complex(0)));


  // Play with zeroth channel...
  Int focusChan0(0);
  sdb.setFocusChan(focusChan0);
  //  cout << "sdb.infocusVisCube().contiguousStorage() = " 
  //       << boolalpha << sdb.infocusVisCube().contiguousStorage() << endl;

  // infocus* shapes...

  //cout << "sdb.infocusFlagCube() = " << sdb.infocusFlagCube() << endl;
  ASSERT_EQ(IPosition(3,nCorr,1,nRow),sdb.infocusFlagCube().shape());
  //cout << "sdb.infocusVisCube() = " << sdb.infocusVisCube() << endl;
  ASSERT_EQ(IPosition(3,nCorr,1,nRow),sdb.infocusVisCube().shape());
  //cout << "sdb.infocusModelVisCube() = " << sdb.infocusModelVisCube() << endl;
  ASSERT_EQ(IPosition(3,nCorr,1,nRow),sdb.infocusModelVisCube().shape());

  // infocus* contents
  //cout << "sdb.infocusFlagCube() = " << sdb.infocusFlagCube() << endl;
  ASSERT_EQ(uInt(0),ntrue(sdb.infocusFlagCube()));

  ASSERT_TRUE(allEQ(sdb.infocusVisCube()(Slice(0,1,1),Slice(),Slice()),c0));
  ASSERT_TRUE(allEQ(sdb.infocusVisCube()(Slice(3,1,1),Slice(),Slice()),conj(c0)));
  ASSERT_TRUE(allEQ(sdb.infocusVisCube()(Slice(1,2,1),Slice(),Slice()),Complex(0)));
  ASSERT_TRUE(allEQ(sdb.infocusModelVisCube()(Slice(0,2,3),Slice(),Slice()),Complex(1.0)));
  ASSERT_TRUE(allEQ(sdb.infocusModelVisCube()(Slice(1,2,1),Slice(),Slice()),Complex(0.0)));

  // Verify ref'd infocus* setting

  // Flag
  sdb.infocusFlagCube().set(True);
  //cout << "ntrue(sdb.infocusFlagCube()) = " << ntrue(sdb.infocusFlagCube()) << endl;
  ASSERT_EQ(uInt(nCorr*1*nRow),ntrue(sdb.infocusFlagCube()));
  //cout << "ntrue(sdb.flagCube()(slice)) = " << ntrue(sdb.flagCube()(Slice(),Slice(focusChan0,1,1),Slice())) << endl;
  ASSERT_EQ(uInt(nCorr*1*nRow),ntrue(sdb.flagCube()(Slice(),Slice(focusChan0,1,1),Slice())));

  sdb.infocusVisCube().set(Complex(999.0));
  ASSERT_TRUE(allEQ(sdb.infocusVisCube(),Complex(999.0)));
  ASSERT_TRUE(allEQ(sdb.visCubeCorrected()(Slice(),Slice(focusChan0,1,1),Slice()),Complex(999.0)));
  sdb.infocusModelVisCube().set(Complex(998.0));
  ASSERT_TRUE(allEQ(sdb.infocusModelVisCube(),Complex(998.0)));
  ASSERT_TRUE(allEQ(sdb.visCubeModel()(Slice(),Slice(focusChan0,1,1),Slice()),Complex(998.0)));
  

  Int focusChan1(1);

  sdb.setFocusChan(focusChan1);

  // verify _this_ focusChan contents still ok

  // infocus* contents
  //cout << "sdb.infocusFlagCube() = " << sdb.infocusFlagCube() << endl;
  ASSERT_EQ(uInt(0),ntrue(sdb.infocusFlagCube()));

  ASSERT_TRUE(allEQ(sdb.infocusVisCube()(Slice(0,1,1),Slice(),Slice()),c0));
  ASSERT_TRUE(allEQ(sdb.infocusVisCube()(Slice(3,1,1),Slice(),Slice()),conj(c0)));
  ASSERT_TRUE(allEQ(sdb.infocusVisCube()(Slice(1,2,1),Slice(),Slice()),Complex(0)));
  ASSERT_TRUE(allEQ(sdb.infocusModelVisCube()(Slice(0,2,3),Slice(),Slice()),Complex(1.0)));
  ASSERT_TRUE(allEQ(sdb.infocusModelVisCube()(Slice(1,2,1),Slice(),Slice()),Complex(0.0)));

  sdbs.sizeResiduals(2,2);   //  nPar=2, nDiff=2
  sdbs.initResidWithModel();
  sdb.finalizeResiduals();
  
  //cout << "sdb.residFlagCube().shape() = " << sdb.residFlagCube().shape() << endl;
  ASSERT_EQ(IPosition(3,nCorr,1,nRow),sdb.residFlagCube().shape());
  //cout << "sdb.residFlagCube() = " << sdb.residFlagCube() << endl;
  ASSERT_EQ(uInt(0),ntrue(sdb.residFlagCube()));

  //cout << "sdb.residuals().shape() = " << sdb.residuals().shape() << endl;
  ASSERT_EQ(IPosition(3,nCorr,1,nRow),sdb.residuals().shape());
  //cout << "sdb.residuals() = " << sdb.residuals() << endl;
  //cout << "sdb.residuals() = " << sdb.residuals()(Slice(0,1,1),Slice(),Slice()) << endl;
  ASSERT_TRUE(allEQ(sdb.residuals()(Slice(0,1,1),Slice(),Slice()),Complex(1.)-c0));
  ASSERT_TRUE(allEQ(sdb.residuals()(Slice(3,1,1),Slice(),Slice()),Complex(1.)-conj(c0)));
  ASSERT_TRUE(allEQ(sdb.residuals()(Slice(1,2,1),Slice(),Slice()),Complex(0.0)));

}

TEST_F(SolveDataBufferTest, SolveDataBufferSolve) {

  sdbs.enforceSolveWeights(True); // p-hands only  (zeros cross-hand weights)
    
  VisEquation ve;
  GJones G("<noms>",nAnt,1);
  G.setSolve();
  ve.setsolve(G);
  
  {
    
    Vector<Double> f(2); indgen(f); f*=1e6; f+=10.0e9;
    G.syncSolveMeta(sdbs);
    G.sizeSolveParCurrSpw(nChan);

    //    G.state();

    G.guessPar(sdbs);

    /*  original...
    G.setSolve();
    G.simpleSizeUpSolveForTesting();
    G.syncSolveMeta(sdbs);
    G.focusChan()=1;
    G.state();
    G.guessPar(sdbs);
    */

    /*
    cout << "Guess: G.solveCPar() = " << G.solveCPar() << endl;
    cout << "G.minblperant() = " << G.minblperant() << endl;
    */

    VisCalSolver2 vcs;
    vcs.solve(ve,G,sdbs);
  
    /*
    cout << "Model=" << sdbs(0).infocusModelVisCube() << endl;
    cout << "Corrd=" << sdbs(0).infocusVisCube() << endl;
    cout << "Wts=" << sdbs(0).infocusWtSpec() << endl;

    cout << "Resid=" << amplitude(sdbs(0).residuals()) << endl;
    cout << "Resid=" << phase(sdbs(0).residuals())*(180./C::pi) << endl;

    cout << "Solution: G.solveCPar() = " << G.solveCPar() << endl;
    cout << "Solution: Amp = " << amplitude(G.solveCPar()) << endl;
    cout << "Solution: Phase = " << phase(G.solveCPar())*180.0/C::pi << endl;
    */

    Float ph0=arg(G.solveCPar()(0,0,1));
    Cube<Float> phtest(phase(G.solveCPar())/ph0);
    //cout << "Solution: Phase frac = " << phtest << endl;

    phtest(0,0,0)+=1.0f;
    phtest(0,0,2)/=2.0f;
    phtest(0,0,3)/=3.0f;
    phtest(0,0,4)/=4.0f;
    phtest(1,0,0)-=1.0f;
    phtest(1,0,2)/=2.0f;
    phtest(1,0,3)/=3.0f;
    phtest(1,0,4)/=4.0f;
    Array<Float> a;
    a.reference(phtest(Slice(0,1,1),Slice(),Slice()));
    a-=1.0f;
    a.reference(phtest(Slice(1,1,1),Slice(),Slice()));
    a+=1.0f;
    phtest=abs(phtest);
    //    cout << "phtest = " << phtest << endl;

    ASSERT_TRUE(allLT(phtest,Float(3e-5)));  // could this be better?
    // it is better if uniform phase offest is smaller....

  }

}
