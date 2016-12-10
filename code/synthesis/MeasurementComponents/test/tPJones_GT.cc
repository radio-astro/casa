//# tPJones: test parallactic angle term
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

#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
#include <synthesis/MeasurementComponents/MSMetaInfoForCal.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/VisBuffer2.h>

#include <gtest/gtest.h>

#include "VisCalTestBase_GT.h"

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

// <summary>
// Test program for PJones
// </summary>

// TBD:
//  o make nPar public in PJones; use *RPar for parang values?
//  o use hardwired implicit spwmap (reduce calculation by factor nSpw)?
//     (effectiveness here will depend on data sort, though...)
//  o add tests for linear basis (need control in SSVi2)
//  o a real MS test

// Control verbosity
#define PJONES_TEST_VERBOSE false

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class PJonesTest : public VisCalTestBase {

public:

  PJonesTest() :
    VisCalTestBase(1,3,1,4,4,1,10,true)   // 3 scans
  {
    if (PJONES_TEST_VERBOSE)
      summary("PJonesTest");
  }

};

TEST_F(PJonesTest, PJonesApplyTest) {

  // Apply-able parang
  //PJones Papp("<noms>",nAnt,nSpw);
  PJones Papp(msmc);
  Papp.setApply();

  ASSERT_EQ(VisCalEnum::JONES,Papp.matrixType());
  ASSERT_EQ(VisCal::P,Papp.type());
  ASSERT_EQ(String("P Jones"),Papp.typeName());
  //  ASSERT_EQ(1,Papp.nPar());  // not really used by PJones?
  ASSERT_FALSE(Papp.freqDepPar());
  ASSERT_FALSE(Papp.freqDepMat());
  ASSERT_FALSE(Papp.freqDepCalWt());
  ASSERT_FALSE(Papp.timeDepMat());
  ASSERT_TRUE(Papp.isApplied());
  ASSERT_FALSE(Papp.isSolvable());

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

      Papp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);



      Vector<Complex> mVb(partialMeans(vb2->visCubeCorrected(),IPosition(2,1,2)));
      //cout << "mean(vCC) = " << mVb << endl;
      ASSERT_NE(0.04,real(mVb(1)));
      ASSERT_NE(0.03,imag(mVb(1)));

      Papp.correct2(*vb2,false,false,true);  // (trial?,doWtSp?,dosync?)

      Vector<Complex> mVa(partialMeans(vb2->visCubeCorrected(),IPosition(2,1,2)));
      //cout << "mean(vCC) = " << mVa << endl;
      ASSERT_NEAR(0.04,real(mVa(1)),1e-6);
      ASSERT_NEAR(0.03,imag(mVa(1)),1e-6);


    }
  }

  if (PJONES_TEST_VERBOSE)
    Papp.state();


}

