//# tVisCal.cc: Tests the VisCal framework
//# Copyright (C) 1995,1999,2000,2001,2016
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

#include <casa/aips.h>
//#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/SimpleSimVi2.h>
//#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/Timer.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/DJones.h>
#include <synthesis/MeasurementComponents/KJones.h>
#include <synthesis/MeasurementComponents/MSMetaInfoForCal.h>

#include <gtest/gtest.h>

#define SHOWSTATE False
using namespace casacore;
using namespace casa;
using namespace casa::vi;

class VisCalTest : public ::testing::Test {

public:
  
  VisCalTest() :
    nFld(1),
    nScan(1),
    nSpw(1),
    nAnt(5),
    nCorr(4),
    nChan(1,32),
    ss(nFld,nScan,nSpw,nAnt,nCorr,Vector<Int>(1,1),nChan),
    msmc(ss)
  {}

  Int nFld,nScan,nSpw,nAnt,nCorr;
  Vector<Int> nChan;
  SimpleSimVi2Parameters ss;
  MSMetaInfoForCal msmc;

};

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



TEST_F(VisCalTest, PJones) {

  VisCal *P = new PJones(msmc);
  P->setApply();

  if (SHOWSTATE)
    P->state();

  ASSERT_EQ(VisCalEnum::JONES,P->matrixType());
  ASSERT_EQ(VisCal::P,P->type());
  ASSERT_EQ(String("P Jones"),P->typeName());
  ASSERT_EQ(1,P->nPar());
  ASSERT_EQ(false,P->freqDepPar());
  ASSERT_EQ(false,P->freqDepMat());
  ASSERT_EQ(false,P->freqDepCalWt());
  ASSERT_EQ(false,P->timeDepMat());
  ASSERT_EQ(true,P->isApplied());
  ASSERT_EQ(false,P->isSolvable());

  delete P;
}


TEST_F(VisCalTest, GJonesApplyState) {
  
  VisCal *G = new GJones(msmc);
  G->setApply();

  G->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  G->sizeApplyParCurrSpw(ss.nChan_(0));
  G->setDefApplyParCurrSpw(True,True);  // sync, w/ doInv=T

  if (SHOWSTATE)
    G->state();

  ASSERT_EQ(VisCalEnum::JONES,G->matrixType());
  ASSERT_EQ(VisCal::G,G->type());
  ASSERT_EQ(String("G Jones"),G->typeName());
  ASSERT_EQ(2,G->nPar());
  ASSERT_EQ(False,G->freqDepPar());
  ASSERT_EQ(False,G->freqDepMat());
  ASSERT_EQ(False,G->freqDepCalWt());
  ASSERT_EQ(False,G->timeDepMat());
  ASSERT_EQ(True,G->isApplied());
  ASSERT_EQ(True,G->isSolvable());

  /*
  IPosition sh(3,2,1,nAnt);  // nChan=1 for G
  ASSERT_TRUE(G->currCPar().shape()==sh);
  ASSERT_TRUE(G->currParOK().shape()==sh);
  ASSERT_TRUE(G->currJElem().shape()==sh);
  ASSERT_TRUE(G->currJElemOK().shape()==sh);
  ASSERT_EQ(G->currParOK().data(),G->currJElemOK().data()); // ok addr equal
  */

  delete G;
}


TEST_F(VisCalTest, GJonesSolveState) {

  //  MSMetaInfoForCal msmc(ss);
  SolvableVisCal *G = new GJones(msmc);

  Record solvePar;
  String caltablename("test.G"); solvePar.define("table",caltablename);
  String solint("int");          solvePar.define("solint",solint);
  Vector<Int> refantlist(1,3);   solvePar.define("refant",refantlist);

  G->setSolve(solvePar);

  G->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  G->sizeSolveParCurrSpw(ss.nChan_(0));
  G->setDefSolveParCurrSpw(True);

  if (SHOWSTATE)
    G->state();

  ASSERT_EQ(VisCalEnum::JONES,G->matrixType());
  ASSERT_EQ(VisCal::G,G->type());
  ASSERT_EQ(String("G Jones"),G->typeName());
  ASSERT_EQ(2,G->nPar());
  ASSERT_EQ(False,G->freqDepPar());
  ASSERT_EQ(False,G->freqDepMat());
  ASSERT_EQ(False,G->freqDepCalWt());
  ASSERT_EQ(False,G->timeDepMat());
  ASSERT_EQ(False,G->isApplied());
  ASSERT_EQ(True,G->isSolved());
  ASSERT_EQ(True,G->isSolvable());
  
  ASSERT_EQ(caltablename,G->calTableName());
  ASSERT_EQ(solint,G->solint());
  ASSERT_EQ(refantlist[0],G->refant());
  ASSERT_TRUE(allEQ(refantlist,G->refantlist()));
  
  //  cout << "G->solveAllCPar().shape() = " << G->solveAllCPar().shape() << endl;

  delete G;
}

TEST_F(VisCalTest, BJonesApplyState) {
  
  VisCal *B = new BJones(msmc);
  B->setApply();

  B->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  B->sizeApplyParCurrSpw(ss.nChan_(0));
  B->setDefApplyParCurrSpw(True,True);  // sync, w/ doInv=T

  if (SHOWSTATE)
    B->state();

  ASSERT_EQ(VisCalEnum::JONES,B->matrixType());
  ASSERT_EQ(VisCal::B,B->type());
  ASSERT_EQ(String("B Jones"),B->typeName());
  ASSERT_EQ(2,B->nPar());
  ASSERT_EQ(true,B->freqDepPar());
  ASSERT_EQ(true,B->freqDepMat());
  ASSERT_EQ(false,B->freqDepCalWt());
  ASSERT_EQ(false,B->timeDepMat());
  ASSERT_EQ(true,B->isApplied());
  ASSERT_EQ(true,B->isSolvable());

  delete B;
}

TEST_F(VisCalTest, BJonesSolveState) {
  
  MSMetaInfoForCal msmc("<noms>");
  SolvableVisCal *B = new BJones(msmc);

  Record solvePar;
  String caltablename("test.B"); solvePar.define("table",caltablename);
  String solint("int");          solvePar.define("solint",solint);
  Vector<Int> refantlist(1,3);   solvePar.define("refant",refantlist);

  B->setSolve(solvePar);

  B->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  B->sizeSolveParCurrSpw(ss.nChan_(0));
  B->setDefSolveParCurrSpw(True);
  
  if (SHOWSTATE)
    B->state();

  ASSERT_EQ(VisCalEnum::JONES,B->matrixType());
  ASSERT_EQ(VisCal::B,B->type());
  ASSERT_EQ(String("B Jones"),B->typeName());
  ASSERT_EQ(2,B->nPar());
  ASSERT_EQ(True,B->freqDepPar());
  ASSERT_EQ(True,B->freqDepMat());
  ASSERT_EQ(False,B->freqDepCalWt());
  ASSERT_EQ(False,B->timeDepMat());
  ASSERT_EQ(False,B->isApplied());
  ASSERT_EQ(True,B->isSolved());
  ASSERT_EQ(True,B->isSolvable());
  
  ASSERT_EQ(caltablename,B->calTableName());
  ASSERT_EQ(solint,B->solint());
  ASSERT_EQ(refantlist[0],B->refant());
  ASSERT_TRUE(allEQ(refantlist,B->refantlist()));
  
  //cout << "B->solveAllCPar().shape() = " << B->solveAllCPar().shape() << endl;
  

  delete B;
}

TEST_F(VisCalTest, TJonesApplyState) {

  VisCal *T = new TJones(msmc);
  T->setApply();

  T->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  T->sizeApplyParCurrSpw(ss.nChan_(0));
  T->setDefApplyParCurrSpw(True,True);  // sync, w/ doInv=T

  if (SHOWSTATE)
    T->state();

  ASSERT_EQ(VisCalEnum::JONES,T->matrixType());
  ASSERT_EQ(VisCal::T,T->type());
  ASSERT_EQ(String("T Jones"),T->typeName());
  ASSERT_EQ(1,T->nPar());
  ASSERT_EQ(false,T->freqDepPar());
  ASSERT_EQ(false,T->freqDepMat());
  ASSERT_EQ(false,T->freqDepCalWt());
  ASSERT_EQ(false,T->timeDepMat());
  ASSERT_EQ(true,T->isApplied());
  ASSERT_EQ(true,T->isSolvable());

  delete T;
}

TEST_F(VisCalTest, TJonesSolveState) {

  SolvableVisCal *T = new TJones(msmc);

  Record solvePar;
  String caltablename("test.T"); solvePar.define("table",caltablename);
  String solint("int");          solvePar.define("solint",solint);
  Vector<Int> refantlist(1,3);   solvePar.define("refant",refantlist);
  T->setSolve(solvePar);

  T->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  T->sizeSolveParCurrSpw(ss.nChan_(0));
  T->setDefSolveParCurrSpw(True);  // sync

  if (SHOWSTATE)
    T->state();

  ASSERT_EQ(VisCalEnum::JONES,T->matrixType());
  ASSERT_EQ(VisCal::T,T->type());
  ASSERT_EQ(String("T Jones"),T->typeName());
  ASSERT_EQ(1,T->nPar());
  ASSERT_EQ(False,T->freqDepPar());
  ASSERT_EQ(False,T->freqDepMat());
  ASSERT_EQ(False,T->freqDepCalWt());
  ASSERT_EQ(False,T->timeDepMat());
  ASSERT_EQ(False,T->isApplied());
  ASSERT_EQ(True,T->isSolvable());
  ASSERT_EQ(True,T->isSolved());

  ASSERT_EQ(caltablename,T->calTableName());
  ASSERT_EQ(solint,T->solint());
  ASSERT_EQ(refantlist[0],T->refant());
  ASSERT_TRUE(allEQ(refantlist,T->refantlist()));


  delete T;
}

/*
TEST_F(VisCalTest, DJones) {

  VisCal *D = new DJones("<noms>",nAnt,nSpw);
  D->setApply();

  ASSERT_EQ(VisCalEnum::JONES,D->matrixType());
  ASSERT_EQ(VisCal::D,D->type());
  ASSERT_EQ(String("Dgen Jones"),D->typeName());
  ASSERT_EQ(2,D->nPar());
  ASSERT_EQ(false,D->freqDepPar());
  ASSERT_EQ(false,D->freqDepMat());
  ASSERT_EQ(false,D->freqDepCalWt());
  ASSERT_EQ(false,D->timeDepMat());
  ASSERT_EQ(true,D->isApplied());
  ASSERT_EQ(true,D->isSolvable());

  delete D;
}

TEST_F(VisCalTest, KJones) {

  VisCal *K = new KJones("<noms>",nAnt,nSpw);
  K->setApply();

  ASSERT_EQ(VisCalEnum::JONES,K->matrixType());
  ASSERT_EQ(VisCal::K,K->type());
  ASSERT_EQ(String("K Jones"),K->typeName());
  ASSERT_EQ(2,K->nPar());
  ASSERT_EQ(false,K->freqDepPar());
  ASSERT_EQ(true,K->freqDepMat());
  ASSERT_EQ(false,K->freqDepCalWt());
  ASSERT_EQ(false,K->timeDepMat());
  ASSERT_EQ(true,K->isApplied());
  ASSERT_EQ(true,K->isSolvable());

  delete K;
}
*/



  /*
  VisBuffer2* vb0 = VisBuffer2::factory(VbPlain, VbRekeyable);
  
  // The basic shape
  vb0->setShape(nCorr,nChan,nRow);
  
  // Meta info
  vb0->setArrayId(Vector<Int>(nRow,0));          // arrid=0
  vb0->setScan(Vector<Int>(nRow,1));             // scan #1
  vb0->setFieldId(Vector<Int>(nRow,1));          // fieldid=0
  vb0->setTime(Vector<Double>(nRow,1e9));        // time
  vb0->setDataDescriptionIds(Vector<int>(nRow,0));  // ddid=0
  vb0->setSpectralWindows(Vector<int>(nRow,0));  // spw=0
  Vector<Int> a1(nRow,-1), a2(nRow,-1);
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
  vb0->setFlagRow(Vector<Bool>(nRow,false));  // unflagged
  vb0->setFlagCube(Cube<Bool>(nCorr,nChan,nRow,false));  // unflagged
  vb0->setWeightSpectrum(Cube<Float>(nCorr,nChan,nRow,1.0));  // all wt=1
  vb0->setVisCubeModel(Cube<Complex>(nCorr,nChan,nRow,Complex(1.0)));
  
  Cube<Complex> cb(nCorr,nChan,nRow,Complex(0.0));

  cb(Slice(0,1,1),Slice(),Slice())=Complex(9.0,1.0);
  cb(Slice(nCorr-1,1,1),Slice(),Slice())=Complex(9.0,-1.0);
  vb0->setVisCubeCorrected(cb);
  */  

  
/*
TEST(MISC, MISC0) {

  Int n=6;

  Double df(31.25e6/32);   // chan width
  Double lo(100.0e9);   // total LO  (100GHz)

  Vector<Double> F(n);
  indgen(F);  //  [0,1,2,3,...]
  F*=df;     
  F+=lo;

  Vector<Float> f(n);
  // Convert:  Float <- Double
  convertArray(f,F);

  Vector<Float> f0(n);
  // Convert:  Float <- Double w/ offset
  convertArray(f0,F-F(0));

  cout.precision(16);
  cout << "F=" << F << endl;
  cout.precision(8);
  cout << "f=" << f << endl;

  cout << "F-F(0)=" << F-F(0) << " " << (F-F(0))/df << endl;
  cout << "f-f(0)=" << f-f(0) << " " << (f-f(0))/Float(df) << endl;
  cout << "f0    =" << f0     << " " << f0/Float(df) << endl;




}
*/
