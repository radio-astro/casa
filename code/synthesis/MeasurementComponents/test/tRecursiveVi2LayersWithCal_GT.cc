//# tRecursiveVi2LayersWithCal.cc: Tests Recursive factory for layered VI2s, incl. Cal
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
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
//#include <msvis/MSVis/ViiLayerFactory.h>
#include <synthesis/MeasurementComponents/CalibratingVi2.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/AveragingVi2Factory.h>
#include <mstransform/TVI/ChannelAverageTVI.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/iomanip.h>
#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// A function to test the contents of vb.antenna1 and vb.antenna2
Bool testA1A2(const Vector<Int>& a1, const Vector<Int>& a2, Int nAnt, Bool doAC=False) {
  
  Int nBln=nAnt*(nAnt-1)/2;
  Int da=1;
  if (doAC) {
    nBln+=nAnt;
    da=0;
  }
  Bool ok(True);
  Int k=0;
  for (Int i=0;i<nAnt-da;++i) {
    for (Int j=i+da;j<nAnt;++j) {
      ok &= (i==a1[k]);
      ok &= (j==a2[k]);
      //      cout << k << ": " << i << "-" << j << "   " << a1[k] << "-" << a2[k] << endl;
      ++k;
    }
  }
  return ok;

}

 
TEST( RecursiveVi2LayersWithCal, BasicCalLayerTest ) {
 
  // A very rudimentary test of the Cal layer

  // Data-generating layer
  SimpleSimVi2Parameters s0;
  SimpleSimVi2LayerFactory ssfac(s0);

  // Calibrating layer
  Float calfactor(2.333);
  CalibratingParameters c0(calfactor);
  CalVi2LayerFactory calfac(c0);

  Vector<ViiLayerFactory*> facts(2);
  facts[0]=&ssfac;
  facts[1]=&calfac;

  VisibilityIterator2 vi(facts);
  VisBuffer2 *vb = vi.getImpl()->getVisBuffer();

  cout << "VI Layers: " << vi.ViiType() << endl;

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi.next()) {

      /*      
      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      cout << "fieldId=" << vb->fieldId() << endl;
      cout << "scan=" << vb->scan() << endl;
      cout << "channelNumbers=" << vb->getChannelNumbers(0) << endl;
      cout << "flagCube = " << boolalpha << vb->flagCube() << endl;

      cout << "weightSpectrum = " << vb->weightSpectrum() << endl;
      cout << "obs data = " << vb->visCube() << endl;
      cout << "obs data A= " << amplitude(vb->visCube()) << endl;
      cout << "obs data P= " << phase(vb->visCube()) << endl;
      cout << "cor data = " << vb->visCubeCorrected() << endl;
      cout << "mod data = " << vb->visCubeModel() << endl;

      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */


      ASSERT_EQ(4,vb->nAntennas());
      ASSERT_EQ(6,vb->nRows());
      ASSERT_EQ(1,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(testA1A2(vb->antenna1(),vb->antenna2(),vb->nAntennas(),False));
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),0));
      ASSERT_TRUE(allEQ(vb->flagCube(),False));
      Float cwt(1.0/calfactor/calfactor);
      ASSERT_TRUE(allEQ(vb->weightSpectrum(),cwt));
      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());
      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCube()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(phands)),calfactor));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(1,niter);
}

TEST( RecursiveVi2LayersWithCal , BasicCalSolveLayerTest ) {
 
  // A very rudimentary test of Cal Solving layer


  // Data-generating layer
  SimpleSimVi2Parameters s0;
  SimpleSimVi2LayerFactory ssfac(s0);

  // Calibrating layer
  Float calfactor(2.333);
  CalibratingParameters c0(calfactor);
  CalSolvingVi2LayerFactory calfac(c0);

  Vector<ViiLayerFactory*> facts(2);
  facts[0]=&ssfac;
  facts[1]=&calfac;

  VisibilityIterator2 vi(facts);
  VisBuffer2 *vb = vi.getImpl()->getVisBuffer();

  cout << "VI Layers: " << vi.ViiType() << endl;

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi.next()) {

      /*      
      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      cout << "fieldId=" << vb->fieldId() << endl;
      cout << "scan=" << vb->scan() << endl;
      cout << "channelNumbers=" << vb->getChannelNumbers(0) << endl;
      cout << "flagCube = " << boolalpha << vb->flagCube() << endl;
      cout << "weightSpectrum = " << vb->weightSpectrum() << endl;
      cout << "obs data = " << vb->visCube() << endl;
      cout << "obs data A= " << amplitude(vb->visCube()) << endl;
      cout << "obs data P= " << phase(vb->visCube()) << endl;
      cout << "cor data = " << vb->visCubeCorrected() << endl;
      cout << "mod data = " << vb->visCubeModel() << endl;

      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */

      ASSERT_EQ(4,vb->nAntennas());
      ASSERT_EQ(6,vb->nRows());
      ASSERT_EQ(1,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(testA1A2(vb->antenna1(),vb->antenna2(),vb->nAntennas(),False));
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),0));
      ASSERT_TRUE(allEQ(vb->flagCube(),False));
      Float cwt(1.0/calfactor/calfactor);
      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());
      ASSERT_TRUE(allEQ(vb->weightSpectrum()(phands),cwt));  // only p-hands model-norm'd

      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCube()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(phands)),calfactor));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(1,niter);
}

/*  Turn this one off for the moment...
TEST( RecursiveVi2LayersWithCal , VECalSolveLayerTest ) {
 
  // A very rudimentary test of Cal Solving layer


  // Data-generating layer
  SimpleSimVi2Parameters s0;
  SimpleSimVi2LayerFactory ssfac(s0);

  // Calibrating layer
  Float calfactor(1.0);
  VisEquation ve;
  GJones G("<noms>",4,1);
  ve.setsolve(G);

  CalSolvingVi2LayerFactoryByVE calfac(&ve);

  Vector<ViiLayerFactory*> facts(2);
  facts[0]=&ssfac;
  facts[1]=&calfac;

  VisibilityIterator2 vi(facts);

  VisBuffer2 *vb = vi.getImpl()->getVisBuffer();

  cout << "VI Layers: " << vi.ViiType() << endl;

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi.next()) {

      /*      
      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      cout << "fieldId=" << vb->fieldId() << endl;
      cout << "scan=" << vb->scan() << endl;
      cout << "channelNumbers=" << vb->getChannelNumbers(0) << endl;
      cout << "flagCube = " << boolalpha << vb->flagCube() << endl;
      cout << "weightSpectrum = " << vb->weightSpectrum() << endl;
      cout << "obs data = " << vb->visCube() << endl;
      cout << "obs data A= " << amplitude(vb->visCube()) << endl;
      cout << "obs data P= " << phase(vb->visCube()) << endl;
      cout << "cor data = " << vb->visCubeCorrected() << endl;
      cout << "mod data = " << vb->visCubeModel() << endl;

      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;

      ASSERT_EQ(4,vb->nAntennas());
      ASSERT_EQ(6,vb->nRows());
      ASSERT_EQ(1,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(testA1A2(vb->antenna1(),vb->antenna2(),vb->nAntennas(),False));
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),0));
      ASSERT_TRUE(allEQ(vb->flagCube(),False));
      Float cwt(1.0/calfactor/calfactor);
      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());
      ASSERT_TRUE(allEQ(vb->weightSpectrum()(phands),cwt));  // only p-hands model-norm'd

      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCube()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(phands)),calfactor));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(1,niter);
}
*/

TEST( RecursiveVi2LayersWithCal , FreqAvedCalSolveTest ) {

  // Test of sim+cal+freqave w/ multiple times/freqs

  // Full FREQ averaging only

  // Data-generating layer
  Int nAnt(4);
  Int nchan(128), ntime(128);
  SimpleSimVi2Parameters s1(1,1,1,nAnt,4,  // nfld=1, nscan=1, nspw=1, nant=nAnt, ncorr=4
			    Vector<Int>(1,ntime), 
			    Vector<Int>(1,nchan));
  SimpleSimVi2LayerFactory ssfac(s1);

  // Calibrating layer
  Float calfactor(2.333f);
  CalibratingParameters c0(calfactor);
  CalSolvingVi2LayerFactory calfac(c0);

  // Freq-ave layer
  Record config;
  config.define("chanbin",nchan); 
  ChannelAverageTVILayerFactory chanave(config);

  Vector<ViiLayerFactory*> facts(3);
  facts[0]=&ssfac;
  facts[1]=&calfac;
  facts[2]=&chanave;

  VisibilityIterator2 vi(facts);
  VisBuffer2 *vb = vi.getImpl()->getVisBuffer();

  cout << "VI Layers: " << vi.ViiType() << endl;

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi.next()) {

      /*      
      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      cout << "fieldId=" << vb->fieldId() << endl;
      cout << "scan=" << vb->scan() << endl;
      cout << "channelNumbers=" << vb->getChannelNumbers(0) << endl;
      cout << "flagCube = " << boolalpha << vb->flagCube() << endl;
      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */

      ASSERT_EQ(nAnt,vb->nAntennas());
      ASSERT_EQ(nAnt*(nAnt-1)/2,vb->nRows());
      ASSERT_EQ(1,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(testA1A2(vb->antenna1(),vb->antenna2(),nAnt,False));
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),0));
      ASSERT_TRUE(allEQ(vb->flagCube(),False));

      Float cwt(1.0f/calfactor/calfactor);
      cwt*=(nchan);  // effect of averaging
      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());

      Float tol(2e-6);  // Needs to be larger for larger averaging!
      /*
      cout << "tol=" << tol << endl;
      cout.precision(30);
      cout << boolalpha;
      cout << "vb->weightSpectrum()(phands)/cwt - 1.0 = " << vb->weightSpectrum()(phands)/cwt - 1.0f << endl;
      cout << "allNear(vb->weightSpectrum()(phands),cwt,tol) = " << allNear(vb->weightSpectrum()(phands),cwt,tol)  << endl;
      cout << "amplitude(vb->visCubeCorrected()(phands))/calfactor -1.0  = " << amplitude(vb->visCubeCorrected()(phands))/calfactor - 1.0f << endl;
      cout << "allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) = " << allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) << endl;
      */

      ASSERT_TRUE(allNear(vb->weightSpectrum()(phands),cwt,tol));

      ASSERT_TRUE(allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));

      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(ntime,niter);
 

}
 

TEST( RecursiveVi2LayersWithCal , TimeAvedCalSolveTest ) {

  // Test of sim+cal+timeave w/ multiple times/freqs

  // Full TIME averaging only

  // Data-generating layer
  Int nAnt(4);
  Int nchan(128), ntime(128);
  SimpleSimVi2Parameters s1(1,1,1,nAnt,4,  // nfld=1, nscan=1, nspw=1, nant=nAnt, ncorr=4
			    Vector<Int>(1,ntime), 
			    Vector<Int>(1,nchan));
  SimpleSimVi2LayerFactory ssfac(s1);

  // Calibrating layer
  Float calfactor(2.333f);
  CalibratingParameters c0(calfactor);
  CalSolvingVi2LayerFactory calfac(c0);

  // Time-ave layer
  AveragingOptions aveopt(AveragingOptions::AverageCorrected|
			  AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT|
			  AveragingOptions::AverageModel|
			  AveragingOptions::ModelPlainAvg);
  AveragingParameters avepar(Float(ntime),0.0,SortColumns(),aveopt);
  AveragingVi2LayerFactory timeave(avepar);

  Vector<ViiLayerFactory*> facts(3);
  facts[0]=&ssfac;
  facts[1]=&calfac;
  facts[2]=&timeave;

  VisibilityIterator2 vi(facts);
  VisBuffer2 *vb = vi.getImpl()->getVisBuffer();

  cout << "VI Layers: " << vi.ViiType() << endl;

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi.next()) {

      /*      
      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      cout << "fieldId=" << vb->fieldId() << endl;
      cout << "scan=" << vb->scan() << endl;
      cout << "channelNumbers=" << vb->getChannelNumbers(0) << endl;
      cout << "flagCube = " << boolalpha << vb->flagCube() << endl;
      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */

      ASSERT_EQ(nAnt,vb->nAntennas());
      ASSERT_EQ(nAnt*(nAnt-1)/2,vb->nRows());
      ASSERT_EQ(nchan,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(testA1A2(vb->antenna1(),vb->antenna2(),nAnt,False));
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));
      Vector<Int> chans(nchan);
      indgen(chans);
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),chans));
      ASSERT_TRUE(allEQ(vb->flagCube(),False));

      /*
      Vector<Double> f(vb->getFrequencies(0));
      Double df=f(1)-f(0);
      f-=f(0);
      //f/=df;
      cout << "f=" << f << endl;
      cout << "df=" << df << endl;
      */

      Float cwt(1.0f/calfactor/calfactor);
      cwt*=(ntime);  // effect of averaging
      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());

      Float tol(2e-6);  // Needs to be larger for larger averaging!
      /*
      cout << "tol=" << tol << endl;
      cout.precision(30);
      cout << boolalpha;
      cout << "vb->weightSpectrum()(phands)/cwt - 1.0 = " << vb->weightSpectrum()(phands)/cwt - 1.0f << endl;
      cout << "allNear(vb->weightSpectrum()(phands),cwt,tol) = " << allNear(vb->weightSpectrum()(phands),cwt,tol)  << endl;
      cout << "amplitude(vb->visCubeCorrected()(phands))/calfactor -1.0  = " << amplitude(vb->visCubeCorrected()(phands))/calfactor - 1.0f << endl;
      cout << "allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) = " << allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) << endl;
      */

      ASSERT_TRUE(allNear(vb->weightSpectrum()(phands),cwt,tol));

      ASSERT_TRUE(allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));

      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(1,niter);
}
 
TEST( RecursiveVi2LayersWithCal , TimeFreqAvedCalSolveTest ) {
 
  // Test of sim+cal+freqave+timeave w/ multiple times/freqs

  // Full TIME and FREQ averaging only

  // Data-generating layer
  Int nAnt(4);
  Int nchan(128), ntime(128);
  SimpleSimVi2Parameters s1(1,1,1,nAnt,4,  // nfld=1, nscan=1, nspw=1, nant=nAnt, ncorr=4
			    Vector<Int>(1,ntime), 
			    Vector<Int>(1,nchan));
  SimpleSimVi2LayerFactory ssfac(s1);

  // Calibrating layer
  Float calfactor(2.333f);
  CalibratingParameters c0(calfactor);
  CalSolvingVi2LayerFactory calfac(c0);

  // Freq-ave layer
  Record config;
  config.define("chanbin",nchan); 
  ChannelAverageTVILayerFactory chanave(config);

  // Time-ave layer
  AveragingOptions aveopt(AveragingOptions::AverageCorrected|
			  AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT|
			  AveragingOptions::AverageModel|
			  AveragingOptions::ModelPlainAvg);
  AveragingParameters avepar(Float(ntime),0.0,SortColumns(),aveopt);
  AveragingVi2LayerFactory timeave(avepar);

  Vector<ViiLayerFactory*> facts(4);
  facts[0]=&ssfac;
  facts[1]=&calfac;
  facts[2]=&chanave;
  facts[3]=&timeave;

  VisibilityIterator2 vi(facts);
  VisBuffer2 *vb = vi.getImpl()->getVisBuffer();

  cout << "VI Layers: " << vi.ViiType() << endl;

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi.next()) {

      /*      
      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      cout << "fieldId=" << vb->fieldId() << endl;
      cout << "scan=" << vb->scan() << endl;
      cout << "channelNumbers=" << vb->getChannelNumbers(0) << endl;
      cout << "flagCube = " << boolalpha << vb->flagCube() << endl;
      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */

      ASSERT_EQ(nAnt,vb->nAntennas());
      ASSERT_EQ(nAnt*(nAnt-1)/2,vb->nRows());
      ASSERT_EQ(1,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(testA1A2(vb->antenna1(),vb->antenna2(),nAnt,False));
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),0));
      ASSERT_TRUE(allEQ(vb->flagCube(),False));

      Float cwt(1.0f/calfactor/calfactor);
      cwt*=(nchan*ntime);  // effect of averaging
      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());

      Float tol(2e-6);  // Needs to be larger for larger averaging!
      /*
      cout << "tol=" << tol << endl;
      cout.precision(30);
      cout << boolalpha;
      cout << "vb->weightSpectrum()(phands)/cwt - 1.0 = " << vb->weightSpectrum()(phands)/cwt - 1.0f << endl;
      cout << "allNear(vb->weightSpectrum()(phands),cwt,tol) = " << allNear(vb->weightSpectrum()(phands),cwt,tol)  << endl;
      cout << "amplitude(vb->visCubeCorrected()(phands))/calfactor -1.0  = " << amplitude(vb->visCubeCorrected()(phands))/calfactor - 1.0f << endl;
      cout << "allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) = " << allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) << endl;
      */

      ASSERT_TRUE(allNear(vb->weightSpectrum()(phands),cwt,tol));

      ASSERT_TRUE(allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));

      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(1,niter);
}

TEST( RecursiveVi2LayersWithCal , FreqTimeAvedCalSolveTest ) {
 
  // Test of sim+cal+freqave+timeave w/ multiple times/freqs

  // Full TIME and FREQ averaging only

  // Data-generating layer
  Int nAnt(4);
  Int nchan(128), ntime(128);
  SimpleSimVi2Parameters s1(1,1,1,nAnt,4,  // nfld=1, nscan=1, nspw=1, nant=nAnt, ncorr=4
			    Vector<Int>(1,ntime), 
			    Vector<Int>(1,nchan));
  SimpleSimVi2LayerFactory ssfac(s1);

  // Calibrating layer
  Float calfactor(2.333f);
  CalibratingParameters c0(calfactor);
  CalSolvingVi2LayerFactory calfac(c0);

  // Freq-ave layer
  Record config;
  config.define("chanbin",nchan); 
  ChannelAverageTVILayerFactory chanave(config);

  // Time-ave layer
  AveragingOptions aveopt(AveragingOptions::AverageCorrected|
			  AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT|
			  AveragingOptions::AverageModel|
			  AveragingOptions::ModelPlainAvg);
  AveragingParameters avepar(Float(ntime),0.0,SortColumns(),aveopt);
  AveragingVi2LayerFactory timeave(avepar);

  Vector<ViiLayerFactory*> facts(4);
  facts[0]=&ssfac;
  facts[1]=&calfac;
  facts[2]=&timeave;
  facts[3]=&chanave;

  VisibilityIterator2 vi(facts);
  VisBuffer2 *vb = vi.getImpl()->getVisBuffer();

  cout << "VI Layers: " << vi.ViiType() << endl;

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi.next()) {

      /*      
      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      cout << "fieldId=" << vb->fieldId() << endl;
      cout << "scan=" << vb->scan() << endl;
      cout << "channelNumbers=" << vb->getChannelNumbers(0) << endl;
      cout << "flagCube = " << boolalpha << vb->flagCube() << endl;
      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */

      ASSERT_EQ(nAnt,vb->nAntennas());
      ASSERT_EQ(nAnt*(nAnt-1)/2,vb->nRows());
      ASSERT_EQ(1,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(testA1A2(vb->antenna1(),vb->antenna2(),nAnt,False));
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),0));
      ASSERT_TRUE(allEQ(vb->flagCube(),False));

      Float cwt(1.0f/calfactor/calfactor);
      cwt*=(nchan*ntime);  // effect of averaging
      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());

      Float tol(2e-6);  // Needs to be larger for larger averaging!
      /*
      cout << "tol=" << tol << endl;
      cout.precision(30);
      cout << boolalpha;
      cout << "vb->weightSpectrum()(phands)/cwt - 1.0 = " << vb->weightSpectrum()(phands)/cwt - 1.0f << endl;
      cout << "allNear(vb->weightSpectrum()(phands),cwt,tol) = " << allNear(vb->weightSpectrum()(phands),cwt,tol)  << endl;
      cout << "amplitude(vb->visCubeCorrected()(phands))/calfactor -1.0  = " << amplitude(vb->visCubeCorrected()(phands))/calfactor - 1.0f << endl;
      cout << "allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) = " << allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) << endl;
      */

      ASSERT_TRUE(allNear(vb->weightSpectrum()(phands),cwt,tol));

      ASSERT_TRUE(allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));

      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(1,niter);
}


TEST( RecursiveVi2LayersWithCal , PartialTimeFreqAvedCalSolveTest ) {
 
  // Test of sim+cal+freqave+timeave w/ multiple times/freqs

  // Partial (/4) TIME and FREQ averaging only

  // Data-generating layer
  Int nAnt(4);
  Int nchan(128), ntime(128);
  SimpleSimVi2Parameters s1(1,1,1,nAnt,4,  // nfld=1, nscan=1, nspw=1, nant=nAnt, ncorr=4
			    Vector<Int>(1,ntime), 
			    Vector<Int>(1,nchan));
  SimpleSimVi2LayerFactory ssfac(s1);

  Int pAveFactor(4);

  // Calibrating layer
  Float calfactor(2.333f);
  CalibratingParameters c0(calfactor);
  CalSolvingVi2LayerFactory calfac(c0);

  // Freq-ave layer
  Record config;
  config.define("chanbin",nchan/pAveFactor); 
  ChannelAverageTVILayerFactory chanave(config);

  // Time-ave layer
  AveragingOptions aveopt(AveragingOptions::AverageCorrected|
			  AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT|
			  AveragingOptions::AverageModel|
			  AveragingOptions::ModelPlainAvg);
  AveragingParameters avepar(Float(ntime/pAveFactor),0.0,SortColumns(),aveopt);
  AveragingVi2LayerFactory timeave(avepar);

  Vector<ViiLayerFactory*> facts(4);
  facts[0]=&ssfac;
  facts[1]=&calfac;
  facts[2]=&chanave;
  facts[3]=&timeave;

  VisibilityIterator2 vi(facts);
  VisBuffer2 *vb = vi.getImpl()->getVisBuffer();

  cout << "VI Layers: " << vi.ViiType() << endl;

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin();vi.more();vi.next()) {

      /*      
      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      cout << "fieldId=" << vb->fieldId() << endl;
      cout << "scan=" << vb->scan() << endl;
      cout << "channelNumbers=" << vb->getChannelNumbers(0) << endl;
      cout << "flagCube = " << boolalpha << vb->flagCube() << endl;
      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */

      ASSERT_EQ(nAnt,vb->nAntennas());
      ASSERT_EQ(nAnt*(nAnt-1)/2,vb->nRows());
      ASSERT_EQ(pAveFactor,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(testA1A2(vb->antenna1(),vb->antenna2(),nAnt,False));
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));
      Vector<Int> chans(pAveFactor);
      indgen(chans);

      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),chans));
      ASSERT_TRUE(allEQ(vb->flagCube(),False));

      ASSERT_TRUE(vb->visCubeCorrected().shape()==IPosition(3,4,pAveFactor,nAnt*(nAnt-1)/2));

      /*
      Vector<Double> f(vb->getFrequencies(0));
      Double df=f(1)-f(0);
      f-=f(0);
      //f/=df;
      cout << "f=" << f << endl;
      cout << "df=" << df << endl;
      */

      Float cwt(1.0f/calfactor/calfactor);
      cwt*=(nchan*ntime/pAveFactor/pAveFactor);  // effect of averaging
      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());

      Float tol(2e-6);  // Needs to be larger for larger averaging!
      /*
      cout << "tol=" << tol << endl;
      cout.precision(30);
      cout << boolalpha;
      cout << "vb->weightSpectrum()(phands)/cwt - 1.0 = " << vb->weightSpectrum()(phands)/cwt - 1.0f << endl;
      cout << "allNear(vb->weightSpectrum()(phands),cwt,tol) = " << allNear(vb->weightSpectrum()(phands),cwt,tol)  << endl;
      cout << "amplitude(vb->visCubeCorrected()(phands))/calfactor -1.0  = " << amplitude(vb->visCubeCorrected()(phands))/calfactor - 1.0f << endl;
      cout << "allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) = " << allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol) << endl;
      */

      ASSERT_TRUE(allNear(vb->weightSpectrum()(phands),cwt,tol));

      ASSERT_TRUE(allNear(amplitude(vb->visCubeCorrected()(phands)),calfactor,tol));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));

      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(pAveFactor,niter);
}
 
