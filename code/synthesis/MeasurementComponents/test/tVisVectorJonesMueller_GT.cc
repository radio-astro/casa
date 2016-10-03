//# tVisVectorJonesMueller_GT.cc:  Tests low-level vis/matrix algebra
//# Copyright (C) 2016
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

#include <synthesis/MeasurementComponents/VisVector.h>
#include <synthesis/MeasurementComponents/Jones.h>
#include <synthesis/MeasurementComponents/Mueller.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayPartMath.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>

#include <gtest/gtest.h>

using namespace std;
using namespace casacore;
using namespace casa;

#define NCHAN 8
#define NANT 4

class VisVectorJonesMuellerTest : public ::testing::Test {

public:

  virtual void setupV1() {
    v1=Vector<Complex>(1,cOne);
    f1=Vector<Bool>(1,false);
    V1=Cube<Complex>(1,NCHAN,nBsln,cOne);
    F1=Cube<Bool>(1,NCHAN,nBsln,false);
  }
  virtual void setupV2() {
    v2=Vector<Complex>(2,cOne);
    f2=Vector<Bool>(2,false);
    V2=Cube<Complex>(2,NCHAN,nBsln,cOne);
    F2=Cube<Bool>(2,NCHAN,nBsln,false);

  }
  virtual void setupV4() {
    v4=Vector<Complex>(4,cOne);
    f4=Vector<Bool>(4,false);
    V4=Cube<Complex>(4,NCHAN,nBsln,cOne);
    F4=Cube<Bool>(4,NCHAN,nBsln,false);
  }
  virtual void setupV4b() {
    v4=Vector<Complex>(4,cOne);
    v4(1)=v4(2)=Complex(0.0);
    f4=Vector<Bool>(4,false);
    //V4=Cube<Complex>(4,NCHAN,nBsln,cOne);
    //F4=Cube<Bool>(4,NCHAN,nBsln,false);
  }
  virtual void setupScalar() {
    js=Vector<Complex>(1,iTwo);
    jsok=Vector<Bool>(1,true);
  }

  virtual void setupDiag() {
    jd=Vector<Complex>(2);
    jd(0)=iTwo;
    jd(1)=iFour;
    jdok=Vector<Bool>(2,true);
  }

  virtual void setupGenLin() {
    jgl=Vector<Complex>(2);
    jgl(0)=iTwo;
    jgl(1)=iThree;
    jglok=Vector<Bool>(2,true);
  }
  virtual void setupGen() {
    jg=Vector<Complex>(4);
    jg(0)=cOne;
    jg(1)=iTwo;
    jg(2)=iThree;
    jg(3)=cOne;
    jgok=Vector<Bool>(4,true);
  }

  virtual void setupB() {
    B=Cube<Complex>(2,NCHAN,NANT);
    Bok=Cube<Bool>(2,NCHAN,NANT);
    Bok=true;
    for (Int iant=0;iant<NANT;++iant) {
      Float A=1.0+Float(iant+33.0)/100.0;
      Float sg( ((iant+1)%2)*2-1 );
      for (Int ich=0;ich<NCHAN;++ich) {
	Float P=Float(iant+1.0)*(1.0 + Float(ich)*C::pi/8.);
	for (Int ipol=0;ipol<2;++ipol) {
	  Float dP=-sg*Float(iant*ipol+1)*C::pi/7.;
	  Float dA=sg*Float(iant*ipol+1)/1000.0;
	  B(ipol,ich,iant) = Complex(A+dA)*Complex(cos(P+dP),sin(P+dP));
	}
      }
    }
  }
  
  virtual void setupV4iquv(Float I,Float Q,Float U,Float V) {
    V4=Cube<Complex>(4,NCHAN,nBsln,cOne);
    V4(Slice(0,1,1),Slice(),Slice())=Complex(I+V);
    V4(Slice(1,1,1),Slice(),Slice())=Complex(Q,U);
    V4(Slice(2,1,1),Slice(),Slice())=Complex(Q,-U);
    V4(Slice(3,1,1),Slice(),Slice())=Complex(I-V);
    F4=Cube<Bool>(4,NCHAN,nBsln,false);
  }

  virtual void setupV4B(Float I,Float Q,Float U,Float V) {
    setupB();
    setupV4iquv(I,Q,U,V);
   
    Int ibl=0;
    for (Int ia1=0;ia1<NANT-1;++ia1) {
      for (Int ia2=ia1+1;ia2<NANT;++ia2,++ibl) {
	for (Int ich=0;ich<NCHAN;++ich) {
	  for (Int icorr=0;icorr<4;++icorr) {
	    V4(icorr,ich,ibl)*=(B(icorr/2,ich,ia1)*conj(B(icorr%2,ich,ia2)));
	  }
	}
      }
    }
  }

protected:

  virtual void SetUp() {
    nBsln=(NANT*(NANT-1)/2);
    cZero=Complex(0.0);
    cOne=Complex(1.0);
    cSeven=Complex(7.0);
    iTwo=Complex(0.,2.);
    iThree=Complex(0.,3.);
    iFour=Complex(0.,4.);
  }
  
  Int nBsln;
  Complex cZero,cOne,iTwo,iThree,iFour,cSeven;

  Vector<Complex> v1,v2,v4;
  Vector<Bool> f1,f2,f4;

  Cube<Complex> V1,V2,V4;
  Cube<Bool> F1,F2,F4;

  Vector<Complex> js,jd,jgl,jg;
  Vector<Bool> jsok,jdok,jglok,jgok;

  Cube<Complex> B;
  Cube<Bool> Bok;
  
};

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

 
TEST_F( VisVectorJonesMuellerTest, VisVectorTest1 ) {
  
  setupV1();

  // nCorr=1  *******************************************
  VisVector VV1(visType(1),false);
  VV1.sync(V1(0,0,0),F1(0,0,0));
  ASSERT_EQ(1,VV1.type());

  VisVector VV2(visType(2),false);
  ASSERT_EQ(2,VV2.type());
  VV2.setType(visType(1));  // reset the type
  ASSERT_EQ(1,VV2.type());
  VV2.sync(v1(0),f1(0));

  // zero (and flag)
  VV1.zero();
  ASSERT_TRUE(allEQ(V1(Slice(),Slice(0,1,1),Slice(0,1,1)),cZero));
  ASSERT_TRUE(allEQ(F1(Slice(),Slice(0,1,1),Slice(0,1,1)),true));

  // assign
  VV1=VV2;
  ASSERT_TRUE(allEQ(V1(Slice(),Slice(0,1,1),Slice(0,1,1)),cOne));
  ASSERT_TRUE(allEQ(F1(Slice(),Slice(0,1,1),Slice(0,1,1)),false));

  // Advance and zero
  Int adv(NANT*NCHAN/2);
  VV1.advance(adv);
  VV1.zero();
  ASSERT_TRUE(allEQ(V1(Slice(),Slice(adv%NCHAN,1,1),Slice(adv/NCHAN,1,1)),cZero));
  ASSERT_TRUE(allEQ(F1(Slice(),Slice(adv%NCHAN,1,1),Slice(adv/NCHAN,1,1)),true));

}

TEST_F( VisVectorJonesMuellerTest, VisVectorTest2 ) {

  setupV2();

  // nCorr=2  *******************************************
  VisVector VV1(visType(2),false);
  VV1.sync(V2(0,0,0),F2(0,0,0));
  ASSERT_EQ(2,VV1.type());

  VisVector VV2(visType(1),false);
  ASSERT_EQ(1,VV2.type());
  VV2.setType(visType(2));  // reset the type
  ASSERT_EQ(2,VV2.type());
  VV2.sync(v2(0),f2(0));

  // zero (and flag)
  VV1.zero();
  ASSERT_TRUE(allEQ(V2(Slice(),Slice(0,1,1),Slice(0,1,1)),cZero));
  ASSERT_TRUE(allEQ(F2(Slice(),Slice(0,1,1),Slice(0,1,1)),true));

  // assign
  VV1=VV2;
  ASSERT_TRUE(allEQ(V2(Slice(),Slice(0,1,1),Slice(0,1,1)),cOne));
  ASSERT_TRUE(allEQ(F2(Slice(),Slice(0,1,1),Slice(0,1,1)),false));

  // Advance and zero
  Int adv(NANT*NCHAN/2);
  VV1.advance(adv);
  VV1.zero();
  ASSERT_TRUE(allEQ(V2(Slice(),Slice(adv%NCHAN,1,1),Slice(adv/NCHAN,1,1)),cZero));
  ASSERT_TRUE(allEQ(F2(Slice(),Slice(adv%NCHAN,1,1),Slice(adv/NCHAN,1,1)),true));

}

TEST_F( VisVectorJonesMuellerTest, VisVectorTest4 ) {

  setupV4();

  // nCorr=4  *******************************************
  VisVector VV1(visType(4),false);
  VV1.sync(V4(0,0,0),F4(0,0,0));
  ASSERT_EQ(4,VV1.type());

  VisVector VV2(visType(2),false);
  ASSERT_EQ(2,VV2.type());
  VV2.setType(visType(4));  // reset the type
  ASSERT_EQ(4,VV2.type());
  VV2.sync(v4(0),f4(0));

  // zero (and flag)
  VV1.zero();
  ASSERT_TRUE(allEQ(V4(Slice(),Slice(0,1,1),Slice(0,1,1)),cZero));
  ASSERT_TRUE(allEQ(F4(Slice(),Slice(0,1,1),Slice(0,1,1)),true));

  // assign
  VV1=VV2;
  ASSERT_TRUE(allEQ(V4(Slice(),Slice(0,1,1),Slice(0,1,1)),cOne));
  ASSERT_TRUE(allEQ(F4(Slice(),Slice(0,1,1),Slice(0,1,1)),false));

  // Advance and zero
  Int adv(NANT*NCHAN/2);
  VV1.advance(adv);
  VV1.zero();

  ASSERT_TRUE(allEQ(V4(Slice(),Slice(adv%NCHAN,1,1),Slice(adv/NCHAN,1,1)),cZero));
  ASSERT_TRUE(allEQ(F4(Slice(),Slice(adv%NCHAN,1,1),Slice(adv/NCHAN,1,1)),true));

}

TEST_F( VisVectorJonesMuellerTest, VisVectorIterTest1 ) {

  setupV1();

  // nCorr=1 *******************************************
  VisVector V(visType(1),false);
  ASSERT_EQ(1,V.type());

  // Iterate, zeroing all
  for (Int ibsln=0;ibsln<nBsln;++ibsln) {
    V.sync(V1(0,0,ibsln),F1(0,0,ibsln));
    for (Int ich=0;ich<NCHAN;++ich,++V) 
      V.zero();
  }

  // All zeroed and flagged
  ASSERT_TRUE(allEQ(V1,cZero));
  ASSERT_TRUE(allEQ(F1,true));

  VisVector v(visType(1),false);
  v.sync(v1(0),f1(0));

  // Iterate, assigining all
  for (Int ibsln=0;ibsln<nBsln;++ibsln) {
    V.sync(V1(0,0,ibsln),F1(0,0,ibsln));
    for (Int ich=0;ich<NCHAN;++ich,++V) 
      V=v;
  }

  ASSERT_TRUE(allEQ(V1,cOne));
  ASSERT_TRUE(allEQ(F1,false));

}

TEST_F( VisVectorJonesMuellerTest, VisVectorIterTest2 ) {

  setupV2();

  // nCorr=2 *******************************************
  VisVector V(visType(2),false);
  ASSERT_EQ(2,V.type());

  // Iterate, zeroing all
  for (Int ibsln=0;ibsln<nBsln;++ibsln) {
    V.sync(V2(0,0,ibsln),F2(0,0,ibsln));
    for (Int ich=0;ich<NCHAN;++ich,++V) 
      V.zero();
  }

  // All zeroed and flagged
  ASSERT_TRUE(allEQ(V2,cZero));
  ASSERT_TRUE(allEQ(F2,true));

  VisVector v(visType(2),false);
  v.sync(v2(0),f2(0));

  // Iterate, assigining all
  for (Int ibsln=0;ibsln<nBsln;++ibsln) {
    V.sync(V2(0,0,ibsln),F2(0,0,ibsln));
    for (Int ich=0;ich<NCHAN;++ich,++V) 
      V=v;
  }

  ASSERT_TRUE(allEQ(V2,cOne));
  ASSERT_TRUE(allEQ(F2,false));

}

TEST_F( VisVectorJonesMuellerTest, VisVectorIterTest4 ) {

  setupV4();

  // nCorr=4 *******************************************
  VisVector V(visType(4),false);
  ASSERT_EQ(4,V.type());

  // Iterate, zeroing all
  for (Int ibsln=0;ibsln<nBsln;++ibsln) {
    V.sync(V4(0,0,ibsln),F4(0,0,ibsln));
    for (Int ich=0;ich<NCHAN;++ich,++V) 
      V.zero();
  }

  // All zeroed and flagged
  ASSERT_TRUE(allEQ(V4,cZero));
  ASSERT_TRUE(allEQ(F4,true));

  VisVector v(visType(4),false);
  v.sync(v4(0),f4(0));

  // Iterate, assigining all
  for (Int ibsln=0;ibsln<nBsln;++ibsln) {
    V.sync(V4(0,0,ibsln),F4(0,0,ibsln));
    for (Int ich=0;ich<NCHAN;++ich,++V) 
      V=v;
  }

  ASSERT_TRUE(allEQ(V4,cOne));
  ASSERT_TRUE(allEQ(F4,false));

}


TEST_F( VisVectorJonesMuellerTest, JonesScalTest ) {

  setupScalar();

  JonesScal S;
  S.sync(js(0),jsok(0));

  ASSERT_EQ(S.type(),Jones::Scalar);
  ASSERT_EQ(S.typesize(),1);

  S.invert();
  ASSERT_FLOAT_EQ(abs(js(0)-Float(1.)/iTwo)+1.,1.);

  S.invert();
  ASSERT_FLOAT_EQ(abs(js(0)-iTwo)+1.,1.);

  { // nCorr=1

    setupV1();
    setupScalar();

    VisVector V(visType(1),false);
    V.sync(v1(0),f1(0));
    
    S.applyRight(V);
    ASSERT_TRUE(allEQ(v1,iTwo));
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(v1,iTwo*conj(iTwo)));
    
    S.invert();
    
    S.applyRight(V);
    ASSERT_TRUE(allEQ(v1,conj(iTwo)));
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(v1,cOne));
    
    jsok(0)=false;  // flag the solution
    S.applyRight(V);
    ASSERT_TRUE(allEQ(f1,true));  // now flagged
    f1(0)=false;                  // unflag V
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(f1,true));  // now flagged
    
  }

  { // nCorr=2

    setupV2();
    setupScalar();

    VisVector V(visType(2),false);
    V.sync(v2(0),f2(0));
    
    S.applyRight(V);
    ASSERT_TRUE(allEQ(v2,iTwo));
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(v2,iTwo*conj(iTwo)));
    
    S.invert();
    
    S.applyRight(V);
    ASSERT_TRUE(allEQ(v2,conj(iTwo)));
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(v2,cOne));
    
    jsok(0)=false;  // flag the solution
    S.applyRight(V);
    ASSERT_TRUE(allEQ(f2,true));  // now flagged
    f2(0)=false;                  // unflag V
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(f2,true));  // now flagged
    
  }

  { // nCorr=4

    setupV4();
    setupScalar();

    VisVector V(visType(4),false);
    V.sync(v4(0),f4(0));
    
    S.applyRight(V);
    ASSERT_TRUE(allEQ(v4,iTwo));
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(v4,iTwo*conj(iTwo)));
    
    S.invert();
    
    S.applyRight(V);
    ASSERT_TRUE(allEQ(v4,conj(iTwo)));
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(v4,cOne));
    
    jsok(0)=false;  // flag the solution
    S.applyRight(V);
    ASSERT_TRUE(allEQ(f4,true));  // now flagged
    f4(0)=false;                  // unflag V
    S.applyLeft(V);
    ASSERT_TRUE(allEQ(f4,true));  // now flagged
    
  }

}
  

TEST_F( VisVectorJonesMuellerTest, JonesDiagTest ) {

  setupDiag();

  JonesDiag D;
  D.sync(jd(0),jdok(0));

  ASSERT_EQ(D.type(),Jones::Diagonal);
  ASSERT_EQ(D.typesize(),2);

  D.invert();
  ASSERT_FLOAT_EQ(abs(jd(0)-Float(1.)/iTwo)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jd(1)-Float(1.)/iFour)+1.,1.);

  D.invert();
  ASSERT_FLOAT_EQ(abs(jd(0)-iTwo)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jd(1)-iFour)+1.,1.);

  // NB: nCorr=1 not applicable to JonesDiag?

  { // nCorr=2

    setupV2();
    setupDiag();

    VisVector V(visType(2),false);
    V.sync(v2(0),f2(0));
    
    D.applyRight(V);
    ASSERT_FLOAT_EQ(abs(v2(0)-iTwo)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v2(1)-iFour)+1.,1.);
    D.applyLeft(V);
    ASSERT_FLOAT_EQ(abs(v2(0)-iTwo*conj(iTwo))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v2(1)-iFour*conj(iFour))+1.,1.);
    
    D.invert();
    
    D.applyRight(V);
    ASSERT_FLOAT_EQ(abs(v2(0)-conj(iTwo))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v2(1)-conj(iFour))+1.,1.);
    D.applyLeft(V);
    ASSERT_TRUE(allEQ(v2,cOne));

    setupDiag();
    setupV2();
    jdok(0)=false;        // flag first solution
    D.applyRight(V);
    ASSERT_TRUE(f2(0));   // now flagged
    ASSERT_FALSE(f2(1));  // not flagged
    jdok(1)=false;        // flag second solution
    D.applyRight(V);
    ASSERT_TRUE(f2(1));   // now flagged

    setupDiag();
    setupV2();
    jdok(0)=false;        // flag first solution
    D.applyLeft(V);
    ASSERT_TRUE(f2(0));   // now flagged
    ASSERT_FALSE(f2(1));  // not flagged
    jdok(1)=false;        // flag second solution
    D.applyLeft(V);
    ASSERT_TRUE(f2(1));   // now flagged
  }

  { // nCorr=4

    setupV4();
    setupDiag();

    VisVector V(visType(4),false);
    V.sync(v4(0),f4(0));
    
    D.applyRight(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-iTwo)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-iTwo)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-iFour)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-iFour)+1.,1.);
    D.applyLeft(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-iTwo*conj(iTwo))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-iTwo*conj(iFour))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-iFour*conj(iTwo))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-iFour*conj(iFour))+1.,1.);
    
    D.invert();
    
    D.applyRight(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-conj(iTwo))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-conj(iFour))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-conj(iTwo))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-conj(iFour))+1.,1.);
    D.applyLeft(V);
    ASSERT_TRUE(allEQ(v4,cOne));

    setupDiag();
    setupV4();
    jdok(0)=false;        // flag first solution
    D.applyRight(V);
    ASSERT_TRUE(f4(0));   // now flagged
    ASSERT_TRUE(f4(1));   // now flagged
    ASSERT_FALSE(f4(2));  // not flagged
    ASSERT_FALSE(f4(3));  // not flagged
    jdok(1)=false;        // flag second solution
    D.applyRight(V);
    ASSERT_TRUE(f4(2));   // now flagged
    ASSERT_TRUE(f4(3));   // now flagged

    setupDiag();
    setupV4();
    jdok(0)=false;        // flag first solution
    D.applyLeft(V);
    ASSERT_TRUE(f4(0));   // now flagged
    ASSERT_FALSE(f4(1));  // not flagged
    ASSERT_TRUE(f4(2));   // now flagged
    ASSERT_FALSE(f4(3));  // not flagged
    jdok(1)=false;        // flag second solution
    D.applyLeft(V);
    ASSERT_TRUE(f4(1));   // now flagged
    ASSERT_TRUE(f4(3));   // now flagged


  }

}
  

TEST_F( VisVectorJonesMuellerTest, JonesGenLinTest ) {

  setupGenLin();

  JonesGenLin GL;
  GL.sync(jgl(0),jglok(0));

  ASSERT_EQ(GL.type(),Jones::GenLinear);
  ASSERT_EQ(GL.typesize(),2);

  GL.invert();
  ASSERT_FLOAT_EQ(abs(jgl(0)+iTwo)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jgl(1)+iThree)+1.,1.);
  
  GL.invert();
  ASSERT_FLOAT_EQ(abs(jgl(0)-iTwo)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jgl(1)-iThree)+1.,1.);
  
  // NB: nCorr=1 or 2 not applicable to JonesGenLin?

  { // nCorr=4

    setupV4b();
    setupGenLin();

    VisVector V(visType(4),false);
    V.sync(v4(0),f4(0));
    
    GL.applyRight(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-cOne)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-iTwo)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-iThree)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-cOne)+1.,1.);
    

    GL.applyLeft(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-cOne)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-(iTwo+conj(iThree)))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-(iThree+conj(iTwo)))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-cOne)+1.,1.);
    
    GL.invert();
    
    GL.applyRight(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-cOne)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-conj(iThree))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-conj(iTwo))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-cOne)+1.,1.);
    
    GL.applyLeft(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-cOne)+1.,1.); 
    ASSERT_FLOAT_EQ(abs(v4(1)-cZero)+1.,1.); 
    ASSERT_FLOAT_EQ(abs(v4(2)-cZero)+1.,1.); 
    ASSERT_FLOAT_EQ(abs(v4(3)-cOne)+1.,1.); 
    

    setupGenLin();
    setupV4b();
    jglok(0)=false;        // flag first solution
    GL.applyRight(V);
    ASSERT_FALSE(f4(0));  // not flagged
    ASSERT_TRUE(f4(1));   // now flagged
    ASSERT_FALSE(f4(2));  // not flagged
    ASSERT_FALSE(f4(3));  // not flagged
    jglok(1)=false;        // flag second solution
    GL.applyRight(V);
    ASSERT_FALSE(f4(0));  // not flagged
    ASSERT_TRUE(f4(1));   // now flagged
    ASSERT_TRUE(f4(2));   // now flagged
    ASSERT_FALSE(f4(3));  // not flagged

    setupGenLin();
    setupV4b();
    jglok(0)=false;        // flag first solution
    GL.applyLeft(V);
    ASSERT_FALSE(f4(0));  // not flagged
    ASSERT_FALSE(f4(1));  // not flagged
    ASSERT_TRUE(f4(2));   // now flagged
    ASSERT_FALSE(f4(3));  // not flagged
    jglok(1)=false;       // flag second solution
    GL.applyLeft(V);
    ASSERT_FALSE(f4(0));  // not flagged
    ASSERT_TRUE(f4(1));   // now flagged
    ASSERT_TRUE(f4(2));   // now flagged
    ASSERT_FALSE(f4(3));  // not flagged


  }
}
  
TEST_F( VisVectorJonesMuellerTest, JonesGenTest ) {

  setupGen();

  Jones J;
  J.sync(jg(0),jgok(0));

  ASSERT_EQ(J.type(),Jones::General);
  ASSERT_EQ(J.typesize(),4);

  J.invert();
  ASSERT_FLOAT_EQ(abs(jg(0)-cOne/cSeven)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jg(1)+iTwo/cSeven)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jg(2)+iThree/cSeven)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jg(3)-cOne/cSeven)+1.,1.);
  
  J.invert();
  ASSERT_FLOAT_EQ(abs(jg(0)-cOne)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jg(1)-iTwo)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jg(2)-iThree)+1.,1.);
  ASSERT_FLOAT_EQ(abs(jg(3)-cOne)+1.,1.);
  
  // NB: nCorr=1 or 2 not applicable to JonesGen?

  { // nCorr=4

    setupV4b();
    setupGenLin();

    VisVector V(visType(4),false);
    V.sync(v4(0),f4(0));
    
    J.applyRight(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-cOne)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-iTwo)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-iThree)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-cOne)+1.,1.);

    J.applyLeft(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-(cOne+iTwo*conj(iTwo)))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-(iTwo+conj(iThree)))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-(iThree+conj(iTwo)))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-(cOne+iThree*conj(iThree)))+1.,1.);

    J.invert();
    
    J.applyRight(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-cOne)+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(1)-conj(iThree))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(2)-conj(iTwo))+1.,1.);
    ASSERT_FLOAT_EQ(abs(v4(3)-cOne)+1.,1.);

    J.applyLeft(V);
    ASSERT_FLOAT_EQ(abs(v4(0)-cOne)+1.,1.); 
    ASSERT_FLOAT_EQ(abs(v4(1)-cZero)+1.,1.); 
    ASSERT_FLOAT_EQ(abs(v4(2)-cZero)+1.,1.); 
    ASSERT_FLOAT_EQ(abs(v4(3)-cOne)+1.,1.); 

    setupGen();
    setupV4b();
    jgok(0)=false;        // flag first diag solution
    J.applyRight(V);
    ASSERT_TRUE(f4(0)); 
    ASSERT_TRUE(f4(1)); 
    ASSERT_FALSE(f4(2));
    ASSERT_FALSE(f4(3));

    setupGen();
    setupV4b();
    jgok(1)=false;        // flag first off-diag solution
    J.applyRight(V);
    ASSERT_TRUE(f4(0)); 
    ASSERT_TRUE(f4(1)); 
    ASSERT_FALSE(f4(2));
    ASSERT_FALSE(f4(3));

    setupGen();
    setupV4b();
    jgok(2)=false;        // flag second off-diag solution
    J.applyRight(V);
    ASSERT_FALSE(f4(0));
    ASSERT_FALSE(f4(1));
    ASSERT_TRUE(f4(2)); 
    ASSERT_TRUE(f4(3)); 

    setupGen();
    setupV4b();
    jgok(3)=false;        // flag second diag solution
    J.applyRight(V);
    ASSERT_FALSE(f4(0));
    ASSERT_FALSE(f4(1));
    ASSERT_TRUE(f4(2)); 
    ASSERT_TRUE(f4(3)); 


    setupGen();
    setupV4b();
    jgok(0)=false;        // flag first diag solution
    J.applyLeft(V);
    ASSERT_TRUE(f4(0));   
    ASSERT_FALSE(f4(1));  
    ASSERT_TRUE(f4(2));   
    ASSERT_FALSE(f4(3));  

    setupGen();
    setupV4b();
    jgok(1)=false;        // flag first off-diag solution
    J.applyLeft(V);
    ASSERT_TRUE(f4(0));   
    ASSERT_FALSE(f4(1));  
    ASSERT_TRUE(f4(2));  
    ASSERT_FALSE(f4(3));  

    setupGen();
    setupV4b();
    jgok(2)=false;        // flag second off-diag solution
    J.applyLeft(V);
    ASSERT_FALSE(f4(0)); 
    ASSERT_TRUE(f4(1));  
    ASSERT_FALSE(f4(2)); 
    ASSERT_TRUE(f4(3));  

    setupGen();
    setupV4b();
    jgok(3)=false;        // flag second diag solution
    J.applyLeft(V);
    ASSERT_FALSE(f4(0)); 
    ASSERT_TRUE(f4(1));  
    ASSERT_FALSE(f4(2)); 
    ASSERT_TRUE(f4(3));  


  }

}
  

TEST_F( VisVectorJonesMuellerTest, JonesDiagIterTest ) {

  Float I(1.0),Q(0.03),U(0.04),sV(0.0);

  setupV4B(I,Q,U,sV);

  VisVector V(visType(4),false);
  JonesDiag B1,B2;

  // Invert the Bs
  B1.sync(B(0,0,0),Bok(0,0,0));
  for (Int ia1=0;ia1<NANT;++ia1) {
    for (Int ich=0;ich<NCHAN;++ich,++B1) {
      B1.invert();
    }
  }

  // Correct the visibilites
  Int ibl=0;
  for (Int ia1=0;ia1<NANT-1;++ia1) {
    B1.sync(B(0,0,ia1),Bok(0,0,ia1));
    for (Int ia2=ia1+1;ia2<NANT;++ia2,++ibl) {
      B2.sync(B(0,0,ia2),Bok(0,0,ia2));
      V.sync(V4(0,0,ibl),F4(0,0,ibl));
      B1.origin();
      B2.origin();
      V.origin();
      for (Int ich=0;
	   ich<NCHAN;
	   ++ich,++B1,++B2,++V) {
	B1.applyRight(V);
	B2.applyLeft(V);
      }
    }
  }

  // Subtract the "truth"
  Array<Complex> RR(V4(Slice(0,1,1),Slice(),Slice()));
  RR-=Complex(I+sV);
  Array<Complex> RL(V4(Slice(1,1,1),Slice(),Slice()));
  RL-=Complex(Q,U);
  Array<Complex> LR(V4(Slice(2,1,1),Slice(),Slice()));
  LR-=Complex(Q,-U);
  Array<Complex> LL(V4(Slice(3,1,1),Slice(),Slice()));
  LL-=Complex(I-sV);

  Vector<Complex> mV4=partialMeans(V4,IPosition(2,1,2));
  
  // Expect ~zero
  ASSERT_TRUE(allNearAbs(amplitude(mV4),0.0f,1e-6));
  ASSERT_TRUE(allNearAbs(amplitude(V4),0.0f,1e-6));

}

