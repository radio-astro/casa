//# tKJones: test delay term
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

#include <synthesis/MeasurementComponents/KJones.h>
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
// Test program for KJones-related classes
// </summary>

// Control verbosity
#define KJONES_TEST_VERBOSE false


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


class DelayFFTTest : public ::testing::Test {

public:
  virtual Vector<Complex> appdel(Int nchan,Float f0,Float df,Float tau, Float ftau) {

    Vector<Float> ph(nchan);
    indgen(ph);
    ph*=df;
    ph+=(f0-ftau);
    ph*=tau;
    ph*=Float(C::_2pi);
    Vector<Float> fph(nchan*2);
    fph(Slice(0,nchan,2))=cos(ph);
    fph(Slice(1,nchan,2))=sin(ph);
    Vector<Complex> cph(nchan);
    RealToComplex(cph,fph);
    return cph;
  }

};

TEST_F(DelayFFTTest, BasicDelayFFTTest) {

    Int nchan0(10),nchan1(20);
    Double df0(0.1),df1(0.05);  //,df1(0.05);
    Double rf0(90.0), rf1(91.0);
    Float tau(-0.5333);

    Cube<Complex> Vobs0(1,nchan0,1), Vobs1(1,nchan1,1);
    Vobs0.set(Complex(1.0));
    Vobs1.set(Complex(1.0));

    Vector<Complex> v0(Vobs0.xyPlane(0).row(0));
    v0*=this->appdel(nchan0,rf0,df0,tau,90.0);

    Vector<Complex> v1(Vobs1.xyPlane(0).row(0));
    v1*=this->appdel(nchan1,rf1,df1,tau,90.0);

    //    cout << "phase(Vobs0) = " << phase(Vobs0)*180/C::pi << endl;
    //    cout << "phase(Vobs1) = " << phase(Vobs1)*180/C::pi << endl;

    Double pbw(64.0);

    DelayFFT sum(90.0,df1,pbw,1,1,-1,Complex(0.0));
      
    DelayFFT delfft0(rf0,df0,pbw,Vobs0);
    delfft0.FFT();
    delfft0.shift(90.0);
    sum.add(delfft0);
    delfft0.searchPeak();

    //cout << "A0 = " << amplitude(delfft0.Vpad()) << endl;
    //cout << "ph0 = " << phase(delfft0.Vpad())*(180.0/C::pi) << endl;

    DelayFFT delfft1(rf1,df1,pbw,Vobs1);
    delfft1.FFT();
    delfft1.shift(90.0);
    sum.add(delfft1);
    delfft1.searchPeak();

    sum.searchPeak();

    //cout << "A1 = " << amplitude(delfft1.Vpad()) << endl;
    //cout << "ph1 = " << phase(delfft1.Vpad())*(180.0/C::pi) << endl;

    if (KJONES_TEST_VERBOSE) {
      cout << boolalpha;
      cout << "delay0 = " << delfft0.delay()(0,0) << " delta=" << delfft0.delay()(0,0)-tau << " " 
	   << delfft0.flag()(0,0) << endl;
      cout << "delay1 = " << delfft1.delay()(0,0) << " delta=" << delfft1.delay()(0,0)-tau << " " 
	   << delfft1.flag()(0,0) << endl;
      cout << "sum    = " << sum.delay()(0,0) << " delta=" << sum.delay()(0,0)-tau << " " 
	   << sum.flag()(0,0) << endl;
    }

    ASSERT_TRUE(allNearAbs(delfft0.delay(),tau,1e-5));
    ASSERT_TRUE(allNearAbs(delfft1.delay(),tau,1e-5));
    ASSERT_TRUE(allNearAbs(sum.delay(),tau,1e-5));


}

class KJonesTest : public VisCalTestBase {

public:
  
  Cube<Float> del;

  KJonesTest() :
    VisCalTestBase(1,1,2,3,4,128,1,true),
    del(2,1,nAnt)
  {
    // canned delays
    indgen(del);
    del*=-1.0f;
    del*=Float(1.0/nChan/0.001);
    del/=2.0f;
    del+=0.01f;
    del(0,0,0)=0.0;
    del(1,0,0)=0.0;
    //cout.precision(16);
    //cout << "del=" << del << endl;
  }

  void setCrossHandDelay(Float xdel) {
    del(Slice(0),Slice(),Slice())=xdel;  // ~ on the padded grid
    del*=Float(1.0/nChan/0.001);
    del(Slice(1),Slice(),Slice())=0.0f;  // all in the first pol
  }

};    


TEST_F(KJonesTest, SBDSolveTest) {

  // Apply-able K
  KMBDJones Kapp(msmc);
  Kapp.setApply();

  for (Int ispw=0;ispw<nSpw;++ispw) { 
    Kapp.setMeta(0,1,0.0,
		 ispw,ssvp.freqs(ispw),
		 nChan);
    Kapp.sizeApplyParCurrSpw(nChan);
    
    Kapp.setApplyParCurrSpw(del,true,false);  // corrupt
  }


  KJones K(msmc);
  Record solvePar;
  solvePar.define("table",String("test.K"));
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));
  Vector<Int> refant(1,0); solvePar.define("refant",refant);
  K.setSolve(solvePar);

  for (vi2.originChunks();vi2.moreChunks();vi2.nextChunk()) {
    for (vi2.origin();vi2.more();vi2.next()) {

      Int ispw=vb2->spectralWindows()(0);
      Int obsid(vb2->observationId()(0));
      Int scan(vb2->scan()(0));
      Double timestamp(vb2->time()(0));
      Int fldid(vb2->fieldId()(0));
      Vector<Double> freqs(vb2->getFrequencies(0));

      vb2->resetWeightsUsingSigma();
      vb2->setVisCubeCorrected(vb2->visCube());
      vb2->setFlagCube(vb2->flagCube());

      Kapp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);
      Kapp.correct2(*vb2,false,false,false);  // (trial?,doWtSp?,dosync?)
      
      Cube<Float> ph=phase(vb2->visCubeCorrected());
      Int hichan(1); // nChan-1);
      Vector<Int> a1(vb2->antenna1());
      Vector<Int> a2(vb2->antenna2());

      for (Int irow=0;irow<vb2->nRows();++irow) {
	for (Int icor=0;icor<nCorr;icor+=3) {
	  Float dph=(ph(icor,hichan,irow)-ph(icor,0,irow))/2.0f/C::pi;  // cycles
	  if (dph>0.5f) dph-=1.0f;
	  if (dph<-0.5f) dph+=1.0f;
	  Float adel(dph/(hichan*0.001));
	  Float edel(del(icor/2,0,a1(irow))-del(icor%2,0,a2(irow)));
	  Float diff(abs(adel-edel));
	  /*
	  cout << "a1=" << a1(irow) << " a2=" << a2(irow)
	       << " dph= " << dph*360.0 << "deg "
	       << " app delay=" << adel
	       << " cal delay=" << edel
	       << " diff=" << diff
	     << endl;
	  */
	  ASSERT_TRUE(diff<1.0e-4);
	}
      }

      K.setMeta(obsid,scan,timestamp,
		ispw,freqs,
		fldid);
      K.sizeSolveParCurrSpw(nChan); 
  
      SDBList sdbs;
      sdbs.add(*vb2);

      K.selfSolveOne(sdbs);


      Cube<Float> soldiff=abs(K.solveRPar()-del);

      //cout << "K.solveRPar() = " << K.solveRPar() << endl;
      //cout << "Diff = " << soldiff  << endl;

      ASSERT_TRUE(allNearAbs(soldiff,0.0f,1e-3));  // at available resoln


    }
  }
}


TEST_F(KJonesTest, MBDSolveTest) {

  // Apply-able K
  KMBDJones Kapp(msmc);
  Kapp.setApply();

  for (Int ispw=0;ispw<nSpw;++ispw) { 
    Kapp.setMeta(0,1,0.0,
		 ispw,ssvp.freqs(ispw),
		 nChan);
    Kapp.sizeApplyParCurrSpw(nChan);
    
    Kapp.setApplyParCurrSpw(del,true,false);  // corrupt
  }

  KJones K(msmc);
  Record solvePar;
  solvePar.define("table",String("test.K"));
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));
  Vector<Int> refant(1,0); solvePar.define("refant",refant);
  K.setSolve(solvePar);

  SDBList sdbs;

  for (vi2.originChunks();vi2.moreChunks();vi2.nextChunk()) {
    for (vi2.origin();vi2.more();vi2.next()) {
      Int ispw=vb2->spectralWindows()(0);
      Int obsid(vb2->observationId()(0));
      Int scan(vb2->scan()(0));
      Double timestamp(vb2->time()(0));
      Int fldid(vb2->fieldId()(0));
      Vector<Double> freqs(vb2->getFrequencies(0));

      vb2->resetWeightsUsingSigma();
      vb2->setVisCubeCorrected(vb2->visCube());
      vb2->setFlagCube(vb2->flagCube());

      Kapp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);
      Kapp.correct2(*vb2,false,false,false);  // (trial?,doWtSp?,dosync?)
      
      Cube<Float> ph=phase(vb2->visCubeCorrected());
      Int hichan(1); // nChan-1);
      Vector<Int> a1(vb2->antenna1());
      Vector<Int> a2(vb2->antenna2());

      for (Int irow=0;irow<vb2->nRows();++irow) {
	for (Int icor=0;icor<nCorr;icor+=3) {
	  Float dph=(ph(icor,hichan,irow)-ph(icor,0,irow))/2.0f/C::pi;  // cycles
	  if (dph>0.5f) dph-=1.0f;
	  if (dph<-0.5f) dph+=1.0f;
	  Float adel(dph/(hichan*0.001));
	  Float edel(del(icor/2,0,a1(irow))-del(icor%2,0,a2(irow)));
	  Float diff(abs(adel-edel));
	  /*
	  cout << "a1=" << a1(irow) << " a2=" << a2(irow)
	       << " dph= " << dph*360.0 << "deg "
	       << " app delay=" << adel
	       << " cal delay=" << edel
	       << " diff=" << diff
	     << endl;
	  */
	  ASSERT_TRUE(diff<1.0e-4);
	}
      }

      sdbs.add(*vb2);

    }
  }

  ASSERT_EQ(nSpw,sdbs.nSDB());
  
  K.setMeta(sdbs.aggregateObsId(),sdbs.aggregateScan(),sdbs.aggregateTime(),
	    sdbs.aggregateSpw(),sdbs(0).freqs(),  // freqs don't matter here, really
	    sdbs.aggregateFld());
  K.sizeSolveParCurrSpw(nChan); 
  
  K.selfSolveOne(sdbs);
  
  Cube<Float> soldiff=abs(K.solveRPar()-del);
  
  //cout << "K.solveRPar() = " << K.solveRPar() << endl;
  //cout << "Diff = " << soldiff  << endl;
  
  ASSERT_TRUE(allNearAbs(soldiff,0.0f,1e-4));  // at available resoln

}

TEST_F(KJonesTest, KCrossSolveTest) {

  setCrossHandDelay(1.251f);

  // Apply-able K
  KMBDJones Kapp(msmc);
  Kapp.setApply();

  for (Int ispw=0;ispw<nSpw;++ispw) { 
    Kapp.setMeta(0,1,0.0,
		 ispw,ssvp.freqs(ispw),
		 nChan);
    Kapp.sizeApplyParCurrSpw(nChan);
    
    Kapp.setApplyParCurrSpw(del,true,false);  // corrupt
  }


  KcrossJones K(msmc);
  Record solvePar;
  solvePar.define("table",String("test.KCROSS"));
  solvePar.define("solint",String("inf"));
  solvePar.define("combine",String(""));
  Vector<Int> refant(1,0); solvePar.define("refant",refant);
  K.setSolve(solvePar);

  for (vi2.originChunks();vi2.moreChunks();vi2.nextChunk()) {
    for (vi2.origin();vi2.more();vi2.next()) {

      Int ispw=vb2->spectralWindows()(0);
      Int obsid(vb2->observationId()(0));
      Int scan(vb2->scan()(0));
      Double timestamp(vb2->time()(0));
      Int fldid(vb2->fieldId()(0));
      Vector<Double> freqs(vb2->getFrequencies(0));

      vb2->resetWeightsUsingSigma();

      Cube<Complex> vC(vb2->visCube());
      vb2->setVisCubeCorrected(vC);
      vb2->setFlagCube(vb2->flagCube());

      Kapp.setMeta(obsid,scan,timestamp,
		   ispw,freqs,
		   fldid);
      Kapp.correct2(*vb2,false,false,false);  // (trial?,doWtSp?,dosync?)


      Cube<Float> ph=phase(vb2->visCubeCorrected());
      Int hichan(1); // nChan-1);
      Vector<Int> a1(vb2->antenna1());
      Vector<Int> a2(vb2->antenna2());

      for (Int irow=0;irow<vb2->nRows();++irow) {
	for (Int icor=1;icor<3;icor+=1) {
	  Float dph=(ph(icor,hichan,irow)-ph(icor,0,irow))/2.0f/C::pi;  // cycles
	  if (dph>0.5f) dph-=1.0f;
	  if (dph<-0.5f) dph+=1.0f;
	  Float adel(dph/(hichan*0.001));
	  Float edel(del(icor/2,0,a1(irow))-del(icor%2,0,a2(irow)));
	  Float diff(abs(adel-edel));
	  /*
	  cout << "a1=" << a1(irow) << " a2=" << a2(irow)
	       << " dph= " << dph*360.0 << "deg "
	       << " app delay=" << adel
	       << " cal delay=" << edel
	       << " diff=" << diff
	     << endl;
	  */
	  ASSERT_TRUE(diff<1.0e-4);
	}
      }

      K.setMeta(obsid,scan,timestamp,
		ispw,freqs,
		fldid);
      K.sizeSolveParCurrSpw(nChan); 
  
      SDBList sdbs;
      sdbs.add(*vb2);

      K.selfSolveOne(sdbs);
      Cube<Float> soldiff=abs(K.solveRPar()-del);

      //cout << "K.solveRPar() = " << K.solveRPar() << endl;
      //cout << "Diff = " << soldiff  << endl;
      ASSERT_TRUE(allNearAbs(soldiff,0.0f,1e-4));  // at available resoln


    }
  }
}


