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

//#include <casa/aips.h>
//#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <msvis/MSVis/VisBuffer2.h>
//#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/Timer.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/DJones.h>
#include <synthesis/MeasurementComponents/KJones.h>

#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casa::vi;

class VisCalTest : public ::testing::Test {

public:

  virtual void SetUp() {
    nAnt=5;
    nSpw=1;
  }

  Int nAnt,nSpw;

};


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST_F(VisCalTest, PJones) {

  VisCal *P = new PJones("<noms>",nAnt,nSpw);
  P->setApply();

  ASSERT_EQ(VisCalEnum::JONES,P->matrixType());
  ASSERT_EQ(VisCal::P,P->type());
  ASSERT_EQ(String("P Jones"),P->typeName());
  ASSERT_EQ(1,P->nPar());
  ASSERT_EQ(False,P->freqDepPar());
  ASSERT_EQ(False,P->freqDepMat());
  ASSERT_EQ(False,P->freqDepCalWt());
  ASSERT_EQ(False,P->timeDepMat());
  ASSERT_EQ(True,P->isApplied());
  ASSERT_EQ(False,P->isSolvable());

  delete P;
}



TEST_F(VisCalTest, GJones) {
  
  VisCal *G = new GJones("<noms>",nAnt,nSpw);
  G->setApply();

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

  delete G;
}

TEST_F(VisCalTest, BJones) {
  
  VisCal *B = new BJones("<noms>",nAnt,nSpw);
  B->setApply();

  ASSERT_EQ(VisCalEnum::JONES,B->matrixType());
  ASSERT_EQ(VisCal::B,B->type());
  ASSERT_EQ(String("B Jones"),B->typeName());
  ASSERT_EQ(2,B->nPar());
  ASSERT_EQ(True,B->freqDepPar());
  ASSERT_EQ(True,B->freqDepMat());
  ASSERT_EQ(False,B->freqDepCalWt());
  ASSERT_EQ(False,B->timeDepMat());
  ASSERT_EQ(True,B->isApplied());
  ASSERT_EQ(True,B->isSolvable());

  delete B;
}

TEST_F(VisCalTest, TJones) {

  VisCal *T = new TJones("<noms>",nAnt,nSpw);
  T->setApply();

  ASSERT_EQ(VisCalEnum::JONES,T->matrixType());
  ASSERT_EQ(VisCal::T,T->type());
  ASSERT_EQ(String("T Jones"),T->typeName());
  ASSERT_EQ(1,T->nPar());
  ASSERT_EQ(False,T->freqDepPar());
  ASSERT_EQ(False,T->freqDepMat());
  ASSERT_EQ(False,T->freqDepCalWt());
  ASSERT_EQ(False,T->timeDepMat());
  ASSERT_EQ(True,T->isApplied());
  ASSERT_EQ(True,T->isSolvable());

  delete T;
}


TEST_F(VisCalTest, DJones) {

  VisCal *D = new DJones("<noms>",nAnt,nSpw);
  D->setApply();

  ASSERT_EQ(VisCalEnum::JONES,D->matrixType());
  ASSERT_EQ(VisCal::D,D->type());
  ASSERT_EQ(String("Dgen Jones"),D->typeName());
  ASSERT_EQ(2,D->nPar());
  ASSERT_EQ(False,D->freqDepPar());
  ASSERT_EQ(False,D->freqDepMat());
  ASSERT_EQ(False,D->freqDepCalWt());
  ASSERT_EQ(False,D->timeDepMat());
  ASSERT_EQ(True,D->isApplied());
  ASSERT_EQ(True,D->isSolvable());

  delete D;
}

TEST_F(VisCalTest, KJones) {

  VisCal *K = new KJones("<noms>",nAnt,nSpw);
  K->setApply();

  ASSERT_EQ(VisCalEnum::JONES,K->matrixType());
  ASSERT_EQ(VisCal::K,K->type());
  ASSERT_EQ(String("K Jones"),K->typeName());
  ASSERT_EQ(2,K->nPar());
  ASSERT_EQ(False,K->freqDepPar());
  ASSERT_EQ(True,K->freqDepMat());
  ASSERT_EQ(False,K->freqDepCalWt());
  ASSERT_EQ(False,K->timeDepMat());
  ASSERT_EQ(True,K->isApplied());
  ASSERT_EQ(True,K->isSolvable());

  delete K;
}


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
  vb0->setFlagRow(Vector<Bool>(nRow,False));  // unflagged
  vb0->setFlagCube(Cube<Bool>(nCorr,nChan,nRow,False));  // unflagged
  vb0->setWeightSpectrum(Cube<Float>(nCorr,nChan,nRow,1.0));  // all wt=1
  vb0->setVisCubeModel(Cube<Complex>(nCorr,nChan,nRow,Complex(1.0)));
  
  Cube<Complex> cb(nCorr,nChan,nRow,Complex(0.0));

  cb(Slice(0,1,1),Slice(),Slice())=Complex(9.0,1.0);
  cb(Slice(nCorr-1,1,1),Slice(),Slice())=Complex(9.0,-1.0);
  vb0->setVisCubeCorrected(cb);
  */  

  

