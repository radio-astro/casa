//# tTrVI2.cc: Tests the TransformingVI stuff
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

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Timer.h>
#include <casa/iomanip.h>
//#include <casa/namespace.h>
#include <casa/Arrays/ArrayMath.h>
#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casa::vi;

#define NFLD 2
#define NSCAN 9
#define NSPW 1
#define NANT 5
#define NCORR 4
#define NTIMEPERFIELD 5
#define NCHAN 32
#define FREQ 100.0e9
#define DF 0.5e6
#define DATE0 "2016/01/06/00:00:00.0"
#define DT 1.0
#define STOKES 1.0
#define GAIN 1.0
#define TSYS 1.0
#define POLBASIS "circ"
#define DONOISE True
#define DONORM True
#define DOAC False


class SimpleSimVi2Test : public ::testing::Test {

protected:

  virtual void SetUp() {

    // Setup the non-trivial SimpleSimVi2Parameters
    Vector<Int> ntpf(NFLD);
    indgen(ntpf); ntpf+=1;  ntpf*=NTIMEPERFIELD;
    Vector<Int> nchan(NSPW,NCHAN);
    Vector<Double> freq(NSPW,FREQ), df(NSPW,DF);
    Matrix<Float> stokes(4,NFLD,0.0);
    Vector<Float> s(stokes.row(0));
    indgen(s); s*=-1.0f; s=pow(10.0f,s);   // 10^0, 10^-1, 10^-2, etc
    Matrix<Float> gain(1,1,GAIN);
    Matrix<Float> tsys(1,1,TSYS);

    s1=SimpleSimVi2Parameters(NFLD,NSCAN,NSPW,NANT,NCORR,
			      ntpf,nchan,
			      DATE0,DT,
			      freq, df, 
			      stokes,
			      DONOISE,
			      gain,tsys,
			      DONORM,
			      POLBASIS,DOAC);

  }
  
  SimpleSimVi2Parameters s0,s1,s2;

};

 
TEST_F( SimpleSimVi2Test , SimpleSimVi2Parameters_Trivial ) {

  ASSERT_EQ(1,s0.nField_);
  ASSERT_EQ(1,s0.nScan_);
  ASSERT_EQ(1,s0.nSpw_);
  ASSERT_EQ(4,s0.nAnt_);
  ASSERT_EQ(4,s0.nCorr_);

  ASSERT_EQ(uInt(1),s0.nTimePerField_.nelements());
  ASSERT_EQ(1,s0.nTimePerField_(0));

  ASSERT_EQ(uInt(1),s0.nChan_.nelements());
  ASSERT_EQ(1,s0.nChan_(0));

  //ASSERT_STREQ(s0.date0_,DATE0);
  ASSERT_EQ(1.0,s0.dt_);

  ASSERT_FALSE(s0.doNoise_);

  ASSERT_EQ(uInt(4),s0.stokes_.nelements());
  ASSERT_EQ(1.0f,s0.stokes_(0,0));

  ASSERT_EQ(uInt(8),s0.gain_.nelements());  // 2*4
  ASSERT_TRUE(allEQ(s0.gain_,1.0f));

  ASSERT_EQ(uInt(8),s0.tsys_.nelements());  // 2*4
  ASSERT_TRUE(allEQ(s0.tsys_,1.0f));
  
  ASSERT_FALSE(s0.doNorm_);
  
  //  ASSERT_STREQ(s0.polBasis_,"circ");

  ASSERT_FALSE(s0.doAC_);

}

 
TEST_F( SimpleSimVi2Test , SimpleSimVi2Parameters_NonTrivial1 ) {

  ASSERT_EQ(NFLD,  s1.nField_);
  ASSERT_EQ(NSCAN, s1.nScan_);
  ASSERT_EQ(NSPW,  s1.nSpw_);
  ASSERT_EQ(NANT,  s1.nAnt_);
  ASSERT_EQ(NCORR, s1.nCorr_);

  ASSERT_EQ(uInt(NFLD),    s1.nTimePerField_.nelements());
  for (Int i=0;i<NFLD;++i)
    ASSERT_EQ(NTIMEPERFIELD*(i+1), s1.nTimePerField_(i));

  ASSERT_EQ(uInt(NSPW), s1.nChan_.nelements());
  ASSERT_EQ(NCHAN,      s1.nChan_(0));

  //ASSERT_STREQ(s0.date0_,DATE0);
  ASSERT_EQ(DT,s1.dt_);

  ASSERT_TRUE(DONOISE==s1.doNoise_);

  ASSERT_EQ(uInt(4*NFLD),s1.stokes_.nelements());
  Vector<Float> stokes(s1.stokes_.row(0));
  for (Int i=0;i<NFLD;++i) ASSERT_EQ(Float(exp10(-i)),stokes(i));

  ASSERT_EQ(uInt(2*NANT),s1.gain_.nelements());  // 2*4
  ASSERT_TRUE(allEQ(s1.gain_,Float(GAIN)));

  ASSERT_EQ(uInt(2*NANT),s1.tsys_.nelements());  // 2*4
  ASSERT_TRUE(allEQ(s1.tsys_,Float(TSYS)));
  
  ASSERT_TRUE(DONORM==s1.doNorm_);
  
  //  ASSERT_STREQ(s1.polBasis_,"circ");

  ASSERT_TRUE(DOAC==s1.doAC_);


}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



TEST_F( SimpleSimVi2Test , SimpleSimVi2_Trivial ) {

  SimpleSimVi2Factory s0f(s0);
  VisibilityIterator2 *vi = new VisibilityIterator2(s0f);
  VisBuffer2 *vb = vi->getImpl()->getVisBuffer();

  vi->originChunks();
  vi->origin();
  
  EXPECT_EQ(4,vb->nAntennas());
  Int nBln(4*3/2);  // no ACs
  EXPECT_EQ(nBln,vb->nRows());

  Vector<Int> vba1(vb->antenna1());
  Vector<Int> vba2(vb->antenna2());
  Int k=0;
  for (Int a1=0;a1<3;++a1) {
    for (Int a2=a1+1;a2<4;++a2) {
      EXPECT_EQ(a1,vba1(k));
      EXPECT_EQ(a2,vba2(k));
      ++k;
    }
  }

  EXPECT_EQ(1,vb->scan()(0));
  EXPECT_EQ(0,vb->fieldId()(0));
  EXPECT_EQ(0,vb->spectralWindows()(0));
  EXPECT_EQ(1,vb->nChannels());
  EXPECT_EQ(4,vb->nCorrelations());

  IPosition dsh(3,4,1,4*3/2);
  EXPECT_EQ(dsh,vb->visCube().shape());
  EXPECT_EQ(dsh,vb->visCubeModel().shape());
  EXPECT_EQ(dsh,vb->visCubeCorrected().shape());

  ASSERT_TRUE(allNearAbs(vb->visCube()(Slice(0,2,3),Slice(),Slice()),Complex(1.0f),FLT_EPSILON));
  ASSERT_TRUE(allNearAbs(vb->visCubeModel()(Slice(0,2,3),Slice(),Slice()),Complex(1.0f),FLT_EPSILON));

  ASSERT_TRUE(allNearAbs(vb->visCube()(Slice(1,2,1),Slice(),Slice()),Complex(0.0f),FLT_EPSILON));
  ASSERT_TRUE(allNearAbs(vb->visCubeModel()(Slice(1,2,1),Slice(),Slice()),Complex(0.0f),FLT_EPSILON));

  Int niter(0);
  for (vi->originChunks();vi->moreChunks();vi->nextChunk()) {
    for (vi->origin();vi->more();vi->next()) {
      ++niter;
    }
  }
  ASSERT_EQ(1,niter);
}
 

TEST_F( SimpleSimVi2Test , SimpleSimVi2_NonTrivial1 ) {
  SimpleSimVi2Factory s1f(s1);
  VisibilityIterator2 *vi = new VisibilityIterator2(s1f);
  VisBuffer2 *vb = vi->getImpl()->getVisBuffer();

  vi->originChunks();
  vi->origin();

  EXPECT_EQ(NANT,vb->nAntennas());
  Int nBln(NANT*(NANT-1)/2);  // no ACs
  Vector<Int> vba1(vb->antenna1());
  Vector<Int> vba2(vb->antenna2());
  EXPECT_EQ(nBln,vb->nRows());
  Int k=0;
  for (Int a1=0;a1<(NANT-1);++a1) {
    for (Int a2=a1+1;a2<NANT;++a2) {
      EXPECT_EQ(a1,vba1(k));
      EXPECT_EQ(a2,vba2(k));
      ++k;
    }
  }
  ASSERT_EQ(nBln,k);

  ASSERT_EQ(FREQ+DF/2,vb->getFrequencies(0)(0));  // row=0, chan=0
  
  Int nchunk(0),niter(0),eniter(0);
  for (vi->originChunks();vi->moreChunks();vi->nextChunk()) {
    
    vi->origin();
    Int fldid(vb->fieldId()(0));
    eniter+=(NTIMEPERFIELD*(fldid+1));
    
    for (vi->origin();vi->more();vi->next()) {

      ASSERT_EQ(nBln,vb->nRows());

      EXPECT_EQ(nchunk+1,vb->scan()(0));
      EXPECT_EQ((nchunk/NSPW)%NFLD,fldid);  
      EXPECT_EQ((nchunk%NSPW),vb->spectralWindows()(0));   
      EXPECT_EQ(NCHAN,vb->nChannels());
      EXPECT_EQ(NCORR,vb->nCorrelations());
      EXPECT_TRUE(allEQ(vb->exposure(),DT));
      
      IPosition dsh(3,NCORR,NCHAN,nBln);
      EXPECT_EQ(dsh,vb->visCube().shape());
      EXPECT_EQ(dsh,vb->visCubeModel().shape());
      EXPECT_EQ(dsh,vb->visCubeCorrected().shape());

      // Model is fldid-dependent
      ASSERT_TRUE(allNearAbs(vb->visCubeModel()(Slice(0,2,3),Slice(),Slice()),Complex(Float(exp10(-fldid))),FLT_EPSILON));
      ASSERT_TRUE(allNearAbs(vb->visCubeModel()(Slice(1,2,1),Slice(),Slice()),Complex(0.0f),FLT_EPSILON));

      // Test stats of residual (implicitly tests Stokes I level)
      Cube<Complex> resid(vb->visCube()-vb->visCubeModel());

      Float rmean=mean(real(resid)), imean=mean(imag(resid));
      Float rerr=stddev(real(resid)), ierr=stddev(imag(resid));
      Float ewt=2*DF*DT;
      Float eerr=1.f/sqrt(ewt);  // expected error per sample
      Float eerrN=eerr/sqrt(resid.nelements());  // expected rms for mean over all corrs,chans,baselines

      rerr=abs(rerr-eerr)/eerr;  // abs err as fraction of expected
      ierr=abs(ierr-eerr)/eerr;

      ASSERT_GT(3.0f,rmean/eerrN);  // 3 sigma
      ASSERT_GT(3.0f,imean/eerrN);  // 3 sigma
      ASSERT_GT(0.1,rerr);  // 10%
      ASSERT_GT(0.1,ierr);  // 10%

      // Formal weights
      Cube<Float> wtsp(vb->weightSpectrum());
      ASSERT_TRUE(allEQ(wtsp,ewt));

      ++niter;
    }
    ++nchunk;
  }
  ASSERT_EQ(NSCAN*NSPW,nchunk);
  ASSERT_EQ(eniter,niter);

}


