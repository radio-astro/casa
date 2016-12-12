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

#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/SolveDataBuffer.h>

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


class MMuellerTest : public VisCalTestBase {

public:

  Cube<Complex> m;

  MMuellerTest() :
    VisCalTestBase(1,1,1,4,4,2,1,false), 
    m()
  {
    Int nBsln=nAnt*(nAnt+1)/2; // ,Complex(0.0));
    m.resize(2,1,nBsln);
    m.set(Complex(0.0));
    Int ibln=0;
    for (Int i=0;i<nAnt;++i) {
      for (Int j=i;j<nAnt;++j) {
	if (i!=j) {
	  m(Slice(0),Slice(),Slice(ibln,1,1))=Complex(1.0+ibln*0.001,ibln*0.004);
	  m(Slice(1),Slice(),Slice(ibln,1,1))=Complex(1.0+ibln*0.002,ibln*0.003);
	}
	++ibln;
      }
    }
    //summary("MMuellerTest");
    //    cout << "m = " << m << endl;
  }


};
  

TEST_F(MMuellerTest, MMuellerSolveOneTest) {

  // Apply-able MMueller (single chan)
  MMueller Mapp(msmc);
  Mapp.setApply();

  for (Int ispw=0;ispw<nSpw;++ispw) { 
    Mapp.setMeta(0,1,0.0,
		 ispw,ssvp.freqs(ispw),
		 nChan);
    Mapp.sizeApplyParCurrSpw(nChan);
    
    Mapp.setApplyParCurrSpw(m,true,false);  // corrupt
  }

  MfMueller Msol(msmc);
  Record solvePar;
  solvePar.define("table",String("test.M"));
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));
  Msol.setSolve(solvePar);

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


      Mapp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);

      cout << vb2->visCubeCorrected() << endl;

      Mapp.correct2(*vb2,false,false,false);  // (trial?,doWtSp?,dosync?)
      
      sdbs.add(*vb2);

    }
  }

  // Setup meta & sizes for the solve
  Msol.setMeta(sdbs.aggregateObsId(),
	       sdbs.aggregateScan(),
	       sdbs.aggregateTime(),
	       sdbs.aggregateSpw(),
	       sdbs.freqs(),
	       sdbs.aggregateFld());
  Msol.sizeSolveParCurrSpw(sdbs.nChannels()); 

  Msol.selfSolveOne(sdbs);

  //cout << "Dsol.solveAllCPar() = " << Msol.solveAllCPar() << endl;

  for (Int ich=0;ich<nChan;++ich) {
    Cube<Float> soldiff=amplitude(Msol.solveAllCPar()(Slice(),Slice(ich),Slice())-m);
    //cout << "Diff = " << soldiff  << endl;
    ASSERT_TRUE(allNearAbs(soldiff,0.0f,1e-7)); 
  }

}
