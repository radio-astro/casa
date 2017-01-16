//# SingleDishSkyCal_GTest.cc: Tests the SingleDishSkyCal family
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
#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
#include <synthesis/MeasurementComponents/SingleDishSkyCal.h>

#include <gtest/gtest.h>

#include "VisCalTestBase_GT.h"

// <summary>
// Test program for SingleDishSkyCal family
// </summary>

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

namespace {
template<class SkyCalImpl>
void ApplyStateTestTemplate(SkyCalImpl &cal, String const &calTypeName) {
//  SkyCalImpl cal(VisCalTestBase::msmc);
  cal.setApply();

  ASSERT_EQ(VisCalEnum::JONES, cal.matrixType());
  ASSERT_EQ(VisCal::M, cal.type());
  ASSERT_EQ(calTypeName, cal.typeName());
  //ASSERT_EQ(6,ff.nPar());
  ASSERT_TRUE(cal.freqDepPar());
  ASSERT_TRUE(cal.freqDepMat());
  ASSERT_FALSE(cal.freqDepCalWt());
  ASSERT_FALSE(cal.timeDepMat());
  ASSERT_TRUE(cal.isApplied());
  ASSERT_TRUE(cal.isSolvable());
  // TODO: should use genericGatherAndSolve
  ASSERT_FALSE(cal.useGenericGatherForSolve());
  ASSERT_FALSE(cal.useGenericSolveOne());

}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class SingleDishSkyCalTest: public VisCalTestBase {

public:

  // test values for solutions
  Cube<Float> fpar;

  SingleDishSkyCalTest() :
      VisCalTestBase(1,  // nfield
          1,  // nscan
          1,  // nspw
          27, // nant
          4,  // ncorr
          64, // nchan per spw
          16, // ntime per scan
          false),  // unpolarized
      fpar(2, 64, VisCalTestBase::nAnt, 0.0f)  // 6 pars per antenna
  {

    // Add SingleDishSkyCalTest specific init
    //  e.g., fill fpar with interesting values

    // uncomment to see data shape summary from
    //VisCalTestBase::summary("SingleDishSkyCalTest");

  }

};

TEST_F(SingleDishSkyCalTest, PositionSwitchCalApplyState) {

  SingleDishPositionSwitchCal cal(VisCalTestBase::msmc);
  ApplyStateTestTemplate(cal, String("SDSKY_PS"));
}

TEST_F(SingleDishSkyCalTest, RasterCalApplyState) {

  SingleDishRasterCal cal(VisCalTestBase::msmc);
  ApplyStateTestTemplate(cal, String("SDSKY_RASTER"));
}

TEST_F(SingleDishSkyCalTest, OtfCalApplyState) {

// TODO: SingleDishOtfCal should have a constructor that accepts
//       MSMetaInfoForCal object
//  SingleDishOtfCal cal(VisCalTestBase::msmc);
//  ApplyStateTestTemplate(cal, String("SDSKY_OTF"));

  cout
      << "TODO: SingleDishOtfCal should have a constructor that accepts MSMetaInfoForCal object"
      << endl;
  SUCCEED();
}

// selfSolveOneTest for SDSKY_PS
TEST_F(SingleDishSkyCalTest, PositionSwitchCal_selfSolveOneTest) {

  // Apply-able SingleDishPositionSwitchCal
  SingleDishPositionSwitchCal PSapp(msmc); // "<noms>",nAnt,nSpw);
  PSapp.setApply();

  // Fill PSapp with actual parameters
  for (Int ispw=0;ispw<nSpw;++ispw) {
    PSapp.setMeta(0,1,0.0,
                 ispw,ssvp.freqs(ispw),
                 nChan);
    PSapp.sizeApplyParCurrSpw(nChan);

    // Disable until phase model implemented...
    PSapp.setApplyParCurrSpw(fpar,true,false);  // don't invert
  }

  SingleDishPositionSwitchCal PSsol(VisCalTestBase::msmc);
  Record solvePar;
  solvePar.define("table",String("test.pscal"));  // not used
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));
  Vector<Int> refant(1,0); solvePar.define("refant",refant);
  PSsol.setSolve(solvePar);

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

      // Corrupt with FJapp
      PSapp.setMeta(obsid,scan,timestamp,
        ispw,freqs,
        fldid);
      // Disabled until phase model implemented
      //PSapp.correct2(*vb2,false,false,false); // (trial?, doWtSp?, dosync)

      // Add vb2 to the SDBList
      sdbs.add(*vb2);

    }
  }

  // Setup meta & sizes for the solve
  PSsol.setMeta(sdbs.aggregateObsId(),
    sdbs.aggregateScan(),
    sdbs.aggregateTime(),
    sdbs.aggregateSpw(),
    sdbs.freqs(),
    sdbs.aggregateFld());
  PSsol.sizeSolveParCurrSpw(sdbs.nChannels());
//
//  PSsol.selfSolveOne(sdbs);
//
//  // Add comparison tests here


}

