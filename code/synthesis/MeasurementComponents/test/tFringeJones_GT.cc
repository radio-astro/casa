//# tFringeJones_GT.cc: Tests the FringeJones
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
#include <synthesis/MeasurementComponents/FringeJones.h>

#include <gtest/gtest.h>

#include "VisCalTestBase_GT.h"

// <summary>
// Test program for FringeJones
// </summary>


using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class FringeJonesTest : public VisCalTestBase {

public:

  // test values for solutions
  Cube<Float> fpar;

  FringeJonesTest() :
    VisCalTestBase(1,  // nfield
		   1,  // nscan
		   1,  // nspw
		   27, // nant
		   4,  // ncorr
		   64, // nchan per spw
		   16, // ntime per scan
		   false),  // unpolarized
    fpar(6,1,VisCalTestBase::nAnt,0.0f)  // 6 pars per antenna
  {

    // Add FringeJonesTest specific init
    //  e.g., fill fpar with interesting values

    // uncomment to see data shape summary from
    //VisCalTestBase::summary("FringeJonesTest");  

  }


};



TEST_F(FringeJonesTest, FringeJonesApplyState) {

  FringeJones ff(VisCalTestBase::msmc);  
  ff.setApply();

  ASSERT_EQ(VisCalEnum::JONES,ff.matrixType());
  ASSERT_EQ(VisCal::K,ff.type());
  ASSERT_EQ(String("Fringe Jones"),ff.typeName());
  //ASSERT_EQ(6,ff.nPar());
  ASSERT_FALSE(ff.freqDepPar());
  ASSERT_TRUE(ff.freqDepMat());
  ASSERT_FALSE(ff.freqDepCalWt());
  ASSERT_EQ(True,ff.timeDepMat());
  ASSERT_TRUE(ff.isApplied());
  ASSERT_TRUE(ff.isSolvable());
  ASSERT_TRUE(ff.useGenericGatherForSolve());
  ASSERT_FALSE(ff.useGenericSolveOne());
  
}



TEST_F(FringeJonesTest, FringeJones_selfSolveOneTest) {

  // Apply-able FringeJones
  FringeJones FJapp(msmc); // "<noms>",nAnt,nSpw);
  FJapp.setApply();

  // Fill FJapp with actual parameters
  for (Int ispw=0;ispw<nSpw;++ispw) {
    FJapp.setMeta(0,1,0.0,
                 ispw,ssvp.freqs(ispw),
                 nChan);
    FJapp.sizeApplyParCurrSpw(nChan);

    // Disable until phase model implemented...
    //FJapp.setApplyParCurrSpw(fpar,true,false);  // don't invert
  }

  FringeJones FJsol(VisCalTestBase::msmc);  
  Record solvePar;
  solvePar.define("table",String("test.Fringe"));  // not used
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));
  Vector<Int> refant(1,0); solvePar.define("refant",refant);
  FJsol.setSolve(solvePar);

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
      FJapp.setMeta(obsid,scan,timestamp,
		    ispw,freqs,
		    fldid);
      // Disabled until phase model implemented
      //FJapp.correct2(*vb2,false,false,false); // (trial?, doWtSp?, dosync)

      // Add vb2 to the SDBList
      sdbs.add(*vb2);

    }
  }

  // Setup meta & sizes for the solve
  FJsol.setMeta(sdbs.aggregateObsId(),
		sdbs.aggregateScan(),
		sdbs.aggregateTime(),
		sdbs.aggregateSpw(),
		sdbs.freqs(),
		sdbs.aggregateFld());
  FJsol.sizeSolveParCurrSpw(sdbs.nChannels()); 

  FJsol.selfSolveOne(sdbs);

  // Add comparison tests here

  
}
