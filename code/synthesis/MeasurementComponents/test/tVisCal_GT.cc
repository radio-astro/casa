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

#define SHOWSTATE false
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




TEST_F(VisCalTest, GJonesApplyState) {
  
  VisCal *G = new GJones(msmc);
  G->setApply();

  G->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  G->sizeApplyParCurrSpw(ss.nChan_(0));
  G->setDefApplyParCurrSpw(true,true);  // sync, w/ doInv=T

  if (SHOWSTATE)
    G->state();

  ASSERT_EQ(VisCalEnum::JONES,G->matrixType());
  ASSERT_EQ(VisCal::G,G->type());
  ASSERT_EQ(String("G Jones"),G->typeName());
  ASSERT_EQ(2,G->nPar());
  ASSERT_FALSE(G->freqDepPar());
  ASSERT_FALSE(G->freqDepMat());
  ASSERT_FALSE(G->freqDepCalWt());
  ASSERT_FALSE(G->timeDepMat());
  ASSERT_TRUE(G->isApplied());
  ASSERT_TRUE(G->isSolvable());

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
  G->setDefSolveParCurrSpw(true);

  if (SHOWSTATE)
    G->state();

  ASSERT_EQ(VisCalEnum::JONES,G->matrixType());
  ASSERT_EQ(VisCal::G,G->type());
  ASSERT_EQ(String("G Jones"),G->typeName());
  ASSERT_EQ(2,G->nPar());
  ASSERT_FALSE(G->freqDepPar());
  ASSERT_FALSE(G->freqDepMat());
  ASSERT_FALSE(G->freqDepCalWt());
  ASSERT_FALSE(G->timeDepMat());
  ASSERT_FALSE(G->isApplied());
  ASSERT_TRUE(G->isSolved());
  ASSERT_TRUE(G->isSolvable());
  
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
  B->setDefApplyParCurrSpw(true,true);  // sync, w/ doInv=T

  if (SHOWSTATE)
    B->state();

  ASSERT_EQ(VisCalEnum::JONES,B->matrixType());
  ASSERT_EQ(VisCal::B,B->type());
  ASSERT_EQ(String("B Jones"),B->typeName());
  ASSERT_EQ(2,B->nPar());
  ASSERT_TRUE(B->freqDepPar());
  ASSERT_TRUE(B->freqDepMat());
  ASSERT_FALSE(B->freqDepCalWt());
  ASSERT_FALSE(B->timeDepMat());
  ASSERT_TRUE(B->isApplied());
  ASSERT_TRUE(B->isSolvable());

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
  B->setDefSolveParCurrSpw(true);
  
  if (SHOWSTATE)
    B->state();

  ASSERT_EQ(VisCalEnum::JONES,B->matrixType());
  ASSERT_EQ(VisCal::B,B->type());
  ASSERT_EQ(String("B Jones"),B->typeName());
  ASSERT_EQ(2,B->nPar());
  ASSERT_TRUE(B->freqDepPar());
  ASSERT_TRUE(B->freqDepMat());
  ASSERT_FALSE(B->freqDepCalWt());
  ASSERT_FALSE(B->timeDepMat());
  ASSERT_FALSE(B->isApplied());
  ASSERT_TRUE(B->isSolved());
  ASSERT_TRUE(B->isSolvable());
  
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
  T->setDefApplyParCurrSpw(true,true);  // sync, w/ doInv=T

  if (SHOWSTATE)
    T->state();

  ASSERT_EQ(VisCalEnum::JONES,T->matrixType());
  ASSERT_EQ(VisCal::T,T->type());
  ASSERT_EQ(String("T Jones"),T->typeName());
  ASSERT_EQ(1,T->nPar());
  ASSERT_FALSE(T->freqDepPar());
  ASSERT_FALSE(T->freqDepMat());
  ASSERT_FALSE(T->freqDepCalWt());
  ASSERT_FALSE(T->timeDepMat());
  ASSERT_TRUE(T->isApplied());
  ASSERT_TRUE(T->isSolvable());

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
  T->setDefSolveParCurrSpw(true);  // sync

  if (SHOWSTATE)
    T->state();

  ASSERT_EQ(VisCalEnum::JONES,T->matrixType());
  ASSERT_EQ(VisCal::T,T->type());
  ASSERT_EQ(String("T Jones"),T->typeName());
  ASSERT_EQ(1,T->nPar());
  ASSERT_FALSE(T->freqDepPar());
  ASSERT_FALSE(T->freqDepMat());
  ASSERT_FALSE(T->freqDepCalWt());
  ASSERT_FALSE(T->timeDepMat());
  ASSERT_FALSE(T->isApplied());
  ASSERT_TRUE(T->isSolvable());
  ASSERT_TRUE(T->isSolved());

  ASSERT_EQ(caltablename,T->calTableName());
  ASSERT_EQ(solint,T->solint());
  ASSERT_EQ(refantlist[0],T->refant());
  ASSERT_TRUE(allEQ(refantlist,T->refantlist()));


  delete T;
}

TEST_F(VisCalTest, DJonesApplyState) {

  VisCal *D = new DJones(msmc);
  D->setApply();

  D->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  D->sizeApplyParCurrSpw(ss.nChan_(0));
  D->setDefApplyParCurrSpw(true,true);  // sync, w/ doInv=T

  if (SHOWSTATE)
    D->state();

  // NB:  why is ntrue(currJElemOK())=0???

  ASSERT_EQ(VisCalEnum::JONES,D->matrixType());
  ASSERT_EQ(VisCal::D,D->type());
  ASSERT_EQ(String("Dgen Jones"),D->typeName());
  ASSERT_EQ(2,D->nPar());
  ASSERT_FALSE(D->freqDepPar());
  ASSERT_FALSE(D->freqDepMat());
  ASSERT_FALSE(D->freqDepCalWt());
  ASSERT_FALSE(D->timeDepMat());
  ASSERT_TRUE(D->isApplied());
  ASSERT_TRUE(D->isSolvable());

  delete D;
}


TEST_F(VisCalTest, DJonesSolveState) {

  SolvableVisCal *D = new DJones(msmc);

  Record solvePar;
  String caltablename("test.D"); solvePar.define("table",caltablename);
  String solint("int");          solvePar.define("solint",solint);
  Vector<Int> refantlist(1,3);   solvePar.define("refant",refantlist);
  D->setSolve(solvePar);

  D->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  D->sizeSolveParCurrSpw(ss.nChan_(0));
  D->setDefSolveParCurrSpw(true);  // sync

  if (SHOWSTATE)
    D->state();

  ASSERT_EQ(VisCalEnum::JONES,D->matrixType());
  ASSERT_EQ(VisCal::D,D->type());
  ASSERT_EQ(String("Dgen Jones"),D->typeName());
  ASSERT_EQ(2,D->nPar());
  ASSERT_FALSE(D->freqDepPar());
  ASSERT_FALSE(D->freqDepMat());
  ASSERT_FALSE(D->freqDepCalWt());
  ASSERT_FALSE(D->timeDepMat());
  ASSERT_FALSE(D->isApplied());
  ASSERT_TRUE(D->isSolvable());
  ASSERT_TRUE(D->isSolved());

  ASSERT_EQ(caltablename,D->calTableName());
  ASSERT_EQ(solint,D->solint());
  ASSERT_EQ(refantlist[0],D->refant());
  ASSERT_TRUE(allEQ(refantlist,D->refantlist()));

  delete D;
}


TEST_F(VisCalTest, KJonesApplyState) {

  VisCal *K = new KJones(msmc);
  K->setApply();

  K->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  K->sizeApplyParCurrSpw(ss.nChan_(0));
  K->setDefApplyParCurrSpw(true,true);  // sync, w/ doInv=T

  if (SHOWSTATE)
    K->state();

  // NB:  why is ntrue(currJElemOK())=0???

  ASSERT_EQ(VisCalEnum::JONES,K->matrixType());
  ASSERT_EQ(VisCal::K,K->type());
  ASSERT_EQ(String("K Jones"),K->typeName());
  ASSERT_EQ(2,K->nPar());
  ASSERT_FALSE(K->freqDepPar());
  ASSERT_TRUE(K->freqDepMat());
  ASSERT_FALSE(K->freqDepCalWt());
  ASSERT_FALSE(K->timeDepMat());
  ASSERT_TRUE(K->isApplied());
  ASSERT_TRUE(K->isSolvable());

  delete K;
}


TEST_F(VisCalTest, KJonesSolveState) {

  SolvableVisCal *K = new KJones(msmc);

  Record solvePar;
  String caltablename("test.K"); solvePar.define("table",caltablename);
  String solint("int");          solvePar.define("solint",solint);
  Vector<Int> refantlist(1,3);   solvePar.define("refant",refantlist);
  K->setSolve(solvePar);

  K->setMeta(0,0,0.0,
	     0,ss.freqs(0),
	     0);
  K->sizeSolveParCurrSpw(ss.nChan_(0));
  K->setDefSolveParCurrSpw(false);  // no sync  (KJones doesn't use diffJElem, etc.)

  if (SHOWSTATE)
    K->state();

  ASSERT_EQ(VisCalEnum::JONES,K->matrixType());
  ASSERT_EQ(VisCal::K,K->type());
  ASSERT_EQ(String("K Jones"),K->typeName());
  ASSERT_EQ(2,K->nPar());
  ASSERT_FALSE(K->freqDepPar());
  ASSERT_TRUE(K->freqDepMat());
  ASSERT_FALSE(K->freqDepCalWt());
  ASSERT_FALSE(K->timeDepMat());
  ASSERT_FALSE(K->isApplied());
  ASSERT_TRUE(K->isSolvable());
  ASSERT_TRUE(K->isSolved());

  ASSERT_EQ(caltablename,K->calTableName());
  ASSERT_EQ(solint,K->solint());
  ASSERT_EQ(refantlist[0],K->refant());
  ASSERT_TRUE(allEQ(refantlist,K->refantlist()));

  delete K;
}

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
