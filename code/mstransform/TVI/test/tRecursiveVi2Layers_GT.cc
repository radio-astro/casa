//# tRecursiveVi2Layers_GT:   test of recursive layering of TVIs
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
#include <msvis/MSVis/ViiLayerFactory.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/AveragingVi2Factory.h>
#include <mstransform/TVI/ChannelAverageTVI.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/iomanip.h>

#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casa::vi;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST( RecursiveVi2LayersTest , ChanAve_Sim ) {
 
  // Test of sim+freqave

  // Data-generating layer  ( V=[1.0,0.0,0.0,1.0] )
  Int nAnt(4);
  Int nchan(128), ntime(4);  // 4 timestamps --> 4 iterations below
  SimpleSimVi2Parameters s1(1,1,1,nAnt,4,  // nfld=1, nscan=1, nspw=1, nant=nAnt, ncorr=4
			    Vector<Int>(1,ntime),     // ntime times/scan
			    Vector<Int>(1,nchan));    // nchan chans/spw
  SimpleSimVi2LayerFactory ssfac(s1);

  // Freq-ave layer
  Int nchanbin(32);  // 4 output channels
  Record config;
  config.define("chanbin",nchanbin);
  ChannelAverageTVILayerFactory chanave(config);

  // Vector of factors for VI2 ctor
  Vector<ViiLayerFactory*> facts(2);
  facts[0]=&ssfac;
  facts[1]=&chanave;

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
      cout << "cor data = " << vb->visCubeCorrected() << endl;
      cout << "mod data = " << vb->visCubeModel() << endl;

      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */

      ASSERT_EQ(nAnt,vb->nAntennas());
      ASSERT_EQ(nAnt*(nAnt-1)/2,vb->nRows());
      ASSERT_EQ(nchan/nchanbin,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));

      Vector<Int> outchans(nchan/nchanbin);
      indgen(outchans);
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),outchans));

      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());

      ASSERT_TRUE(allEQ(vb->flagCube(),False));
      ASSERT_TRUE(allEQ(vb->weightSpectrum(),Float(nchanbin)));
      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCube()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(4,niter); // only one iteration
}


TEST( RecursiveVi2LayersTest , TimeAve_ChanAve_Sim ) {
 
  // Test of sim+freqave+timeave w/ multiple times/freqs

  // Data-generating layer
  Int nAnt(4);
  Int nchan(128), ntime(128);
  SimpleSimVi2Parameters s1(1,1,1,nAnt,4,  // nfld=1, nscan=1, nspw=1, nant=nAnt, ncorr=4
			    Vector<Int>(1,ntime), 
			    Vector<Int>(1,nchan));
  SimpleSimVi2LayerFactory ssfac(s1);

  // Freq-ave layer
  Int nchanbin(32);
  Record config;
  config.define("chanbin",nchanbin);
  ChannelAverageTVILayerFactory chanave(config);

  // Time-ave layer

  Int ntimebin(32);  // ==32s
  AveragingOptions aveopt(AveragingOptions::AverageCorrected|
			  AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT|
			  AveragingOptions::AverageObserved|
			  AveragingOptions::ObservedFlagWeightAvgFromSIGMA|
			  AveragingOptions::AverageModel|
			  AveragingOptions::ModelPlainAvg);
  AveragingParameters avepar(Float(ntimebin)-0.1f,0.0,SortColumns(),aveopt);
  AveragingVi2LayerFactory timeave(avepar);

  Vector<ViiLayerFactory*> facts(3);
  facts[0]=&ssfac;
  facts[1]=&chanave;
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


      cout << "weightSpectrum = " << vb->weightSpectrum() << endl;
      cout << "cor data = " << vb->visCubeCorrected() << endl;
      cout << "mod data = " << vb->visCubeModel() << endl;

      cout << "antenna1=" << vb->antenna1() << endl;
      cout << "antenna2=" << vb->antenna2() << endl;
      */


      ASSERT_EQ(nAnt,vb->nAntennas());
      ASSERT_EQ(nAnt*(nAnt-1)/2,vb->nRows());
      ASSERT_EQ(nchan/nchanbin,vb->nChannels());
      ASSERT_EQ(4,vb->nCorrelations());
      ASSERT_TRUE(allEQ(vb->spectralWindows(),0));
      ASSERT_TRUE(allEQ(vb->fieldId(),0));
      ASSERT_TRUE(allEQ(vb->scan(),1));


      Vector<Int> outchans(nchan/nchanbin);
      indgen(outchans);
      ASSERT_TRUE(allEQ(vb->getChannelNumbers(0),outchans));

      Slicer phands(Slice(0,2,3),Slice(),Slice());
      Slicer xhands(Slice(1,2,1),Slice(),Slice());

      ASSERT_TRUE(allEQ(vb->flagCube(),False));
      ASSERT_TRUE(allEQ(vb->weightSpectrum(),Float(nchanbin*ntimebin)));
      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCube()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCube()),0.0f));
      ASSERT_TRUE(allEQ(vb->visCube()(phands),Complex(1.0f,0.0f)));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeCorrected()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeCorrected()),0.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(phands)),1.0f));
      ASSERT_TRUE(allEQ(amplitude(vb->visCubeModel()(xhands)),0.0f));
      ASSERT_TRUE(allEQ(phase(vb->visCubeModel()),0.0f));

      ++niter;
    }
  }
  ASSERT_EQ(4,niter);
}
