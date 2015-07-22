//# tVisVectorJonesMueller: test program for VisVector, Jones, Mueller classes
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

#include <synthesis/MeasurementComponents/VisVector.h>
#include <synthesis/MeasurementComponents/Jones.h>
#include <synthesis/MeasurementComponents/Mueller.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <casa/namespace.h>

// <summary>
// Test program for VisVector, Jones, Mueller iterating algebra classes
// </summary>

// Control verbosity
#define JONES_VERBOSE True


void testVisVector (Bool verbose=False) {

  {
    Int nCorr(4), nChan(32), nRow(10);

    Cube<Complex> visCube(nCorr,nChan,nRow,Complex(0.0));
    visCube(Slice(0,1,1),Slice(),Slice())=Complex(1.,0);
    visCube(Slice(nCorr-1,1,1),Slice(),Slice())=Complex(1.,0);
    Cube<Bool> flagCube(nCorr,nChan,nRow);
    flagCube.set(False);

    if (verbose) {
      cout << "ntrue(flagCube) = " << ntrue(flagCube) << endl;
      cout << "sum(visCube)    = " << sum(visCube) << endl;
    }

    AlwaysAssert( ntrue(flagCube)==0, AipsError);
    AlwaysAssert( real(sum(visCube))==visCube.nelements()/2, AipsError)

    VisVector V(visType(nCorr),False);
    for (Int irow=0;irow<nRow;++irow) {
      V.sync(visCube(0,0,irow),flagCube(0,0,irow));
      for (Int ich=0;ich<nChan;++ich,++V) 
	V.zero();
    }

    AlwaysAssert( ntrue(flagCube)==flagCube.nelements(), AipsError);
    AlwaysAssert( real(sum(visCube))==0.0, AipsError)

    if (verbose) {
      cout << "ntrue(flagCube) = " << ntrue(flagCube) << endl;
      cout << "sum(visCube)    = " << sum(visCube) << endl;
    }

  }

}

void initVis(Cube<Complex>& v,Float I,Float Q,Float U,Float V) {

  Int nCorr=v.shape()(0);

  Complex RR(I+V,0.0);
  Complex RL(Q,U);
  Complex LR(Q,-U);
  Complex LL(I-V,0.);

  ArrayIterator<Complex> viter(v,1);
  while (!viter.pastEnd()) {
    Vector<Complex> v1(viter.array());
    v1(0)=RR;
    if (nCorr==2)
      v1(1)=LL;
    else if (nCorr==4) {
      v1(1)=RL;
      v1(2)=LR;
      v1(3)=LL;
    }
    viter.next();
  }

}

void testJonesApply (Bool verbose=False) {

  {
    Int nAnt(5);
    Int nCorr(4), nChan(1);
    Float I(1.0), Q(0.1), U(0.01);


    Int nRow(nAnt*(nAnt+1)/2);

    Cube<Complex> visCubeMod(nCorr,nChan,nRow,Complex(0.0));
    visCubeMod(Slice(0,1,1),Slice(),Slice())=Complex(I,0);
    visCubeMod(Slice(1,1,1),Slice(),Slice())=Complex(Q,U);
    visCubeMod(Slice(2,1,1),Slice(),Slice())=Complex(Q,-U);
    visCubeMod(Slice(3,1,1),Slice(),Slice())=Complex(I,0);
    Cube<Bool> flagCube(nCorr,nChan,nRow);
    flagCube.set(False);

    Cube<Complex> B(2,nChan,nAnt);
    Cube<Bool> Bok(2,nChan,nAnt);
    Bok.set(True);

    for (Int ia1=0;ia1<nAnt;++ia1) {
      for (Int ich=0;ich<nChan;++ich) {
	Float a=Float(ia1+1) - Float(ich*ich)/Float(nChan*nChan);
	B(Slice(0,1,1),Slice(ich,1,1),Slice(ia1,1,1))=Complex(a,0.0);
	B(Slice(1,1,1),Slice(ich,1,1),Slice(ia1,1,1))=Complex(0.0,a);
      }
    }

    Bok(1,nChan/2,nAnt/2)=False;

    cout << "B = " << B << endl;
    cout << "Bok = " << boolalpha << Bok << endl;


    Cube<Complex> visCubeObs;
    visCubeObs=visCubeMod;

    Int irow=0;
    for (Int ia1=0;ia1<nAnt;++ia1) {
      for (Int ia2=ia1;ia2<nAnt;++ia2) {
	for (Int ich=0;ich<nChan;++ich) {
	  Int icorr=0;
	  for (Int ip1=0;ip1<2;++ip1) {
	    for (Int ip2=0;ip2<2;++ip2) {
	      visCubeObs(icorr,ich,irow)*=B(ip1,ich,ia1);
	      visCubeObs(icorr,ich,irow)*=conj(B(ip2,ich,ia2));
	      ++icorr;
	    }
	  }
	}
	++irow;
      }
    }
 
    if (verbose)
      cout << "visCubeObs = " << visCubeObs << endl;


    Cube<Complex> visCubeCorr;
    visCubeCorr=visCubeObs;

    
    VisVector V(visType(nCorr),False);
    JonesDiag J1, J2;

    // Invert the matrices
    for (Int ia1=0;ia1<nAnt;++ia1) {
      J1.sync(B(0,0,ia1),Bok(0,0,ia1));
      for (Int ich=0;ich<nChan;++ich,++J1)
	J1.invert();
    }

    cout << "Binv = " << B << endl;

    // Apply the corrections
    irow=0;
    for (Int ia1=0;ia1<nAnt;++ia1) {
      for (Int ia2=ia1;ia2<nAnt;++ia2) {
	J1.sync(B(0,0,ia1),Bok(0,0,ia1));
	J2.sync(B(0,0,ia2),Bok(0,0,ia2));

	V.sync(visCubeCorr(0,0,irow),flagCube(0,0,irow));

	for (Int ich=0;ich<nChan;++ich,++J1,++J2,++V) {
	  J1.applyRight(V);
	  J2.applyLeft(V);
	}
	++irow;
      }
    }

    Cube<Complex> visCubeRes;
    visCubeRes= visCubeCorr-visCubeMod;

    if (verbose) {
      cout << "visCubeCorr = " << visCubeCorr << endl;
      cout << "visCubeRes  = " << visCubeRes << endl;
      cout << "flagCube    = " << boolalpha << flagCube << endl;
      cout << "max(amplitude(visCubeRes)(!flagCube)) = " << max(amplitude(visCubeRes)(!flagCube)) << endl;
    }	

    AlwaysAssert( max(amplitude(visCubeRes)(!flagCube))<2*FLT_EPSILON , AipsError);
    
      
    visCubeRes(!flagCube)=Complex(0.0);
    if (verbose)
      cout << "amplitude(visCubeRes)  = " << amplitude(visCubeRes) << endl;
      



    // Invert the matrices
    for (Int ia1=0;ia1<nAnt;++ia1) {
      J1.sync(B(0,0,ia1),Bok(0,0,ia1));
      for (Int ich=0;ich<nChan;++ich,++J1)
	J1.invert();
    }

    cout << "Binvinv = " << B << endl;

  }
}


void testGenLinJones (Bool /*verbose=False*/) {

  {

    Vector<Complex> vis(4);
    vis(0)=vis(3)=Complex(1.);
    vis(1)=vis(2)=Complex(0.);

    Vector<Bool> fl(4);
    fl.set(False);
    
    Vector<Complex> D1(4), D2(4);
    D1(0)=Complex(1.);
    D1(1)=Complex(0.01,0.02);
    D1(2)=Complex(0.03,0.04);
    D1(3)=Complex(1.);
    D2(0)=Complex(1.);
    D2(1)=Complex(0.015,0.025);
    D2(2)=Complex(-0.035,-0.045);
    D2(3)=Complex(1.);
    Vector<Bool> Ok1(4), Ok2(4);
    Ok1.set(True);
    Ok2.set(True);

    // JonesGenLin D
    Vector<Complex> d1(2), d2(2);
    d1(0)=D1(1);
    d1(1)=D1(2);
    d2(0)=D2(1);
    d2(1)=D2(2);
    Vector<Bool> ok1(2), ok2(2);
    ok1.set(True);
    ok2.set(True);
      
    VisVector VV(visType(4),False);
    VV.sync(vis(0),fl(0));

    cout << "VV raw = " << VV << endl;

    {
    Jones J1, J2;

    J1.sync(D1(0),Ok1(0));
    J2.sync(D2(0),Ok2(0));

    J1.applyRight(VV);
    J2.applyLeft(VV);
    }
    cout << "VV corrupted = " << VV << endl;

    //    ok1(0)=False;

    JonesGenLin J1, J2;

    J1.sync(d1(0),ok1(0));
    J2.sync(d2(0),ok2(0));

    J1.invert();
    J2.invert();

    J1.applyRight(VV);
    J2.applyLeft(VV);


    cout << "VV corrected = " << VV << endl;

  }
}



void testGenJones (Bool /*verbose=False*/) {

  {

    Vector<Complex> vis(4);
    vis(0)=vis(3)=Complex(1.);
    vis(1)=vis(2)=Complex(0.);

    Vector<Bool> fl(4);
    fl.set(False);
    

    // Jones (general) D
    Vector<Complex> d1(4), d2(4);
    d1(0)=d1(3)=Complex(1.0);
    d1(1)=Complex(0.005,0.015);
    d1(2)=Complex(0.025,0.035);
    d2(0)=d2(3)=Complex(1.0);
    d2(1)=Complex(0.01,0.02);
    d2(2)=Complex(0.03,0.04);
    Vector<Bool> ok1(4), ok2(4);
    ok1.set(True);
    ok2.set(True);
      
    VisVector VV(visType(4),False);
    VV.sync(vis(0),fl(0));

    cout << "VV raw = " << VV << endl;

    Jones J1, J2;

    J1.sync(d1(0),ok1(0));
    J2.sync(d2(0),ok2(0));

    J1.applyRight(VV);
    J2.applyLeft(VV);

    cout << "VV corrupted = " << VV << endl;

    //ok1(2)=False;

    J1.invert();
    J2.invert();

    J1.applyRight(VV);
    J2.applyLeft(VV);


    cout << "VV corrected = " << VV << endl;

  }
}

void testGenJones2 (Bool /*verbose=False*/) {

  {

    Vector<Complex> vis(4);
    vis(0)=vis(3)=Complex(1.);
    vis(1)=vis(2)=Complex(0.);

    Vector<Bool> fl(4);
    fl.set(False);
    

    // Jones (general) D
    Vector<Complex> d1(4), d2(4);
    d1(0)=d1(3)=Complex(1.0);
    d1(1)=Complex(0.005,0.015);
    d1(2)=Complex(0.025,0.035);
    d2(0)=d2(3)=Complex(1.0);
    d2(1)=Complex(0.01,0.02);
    d2(2)=Complex(0.03,0.04);
    Vector<Bool> ok1(4), ok2(4);
    ok1.set(True);
    ok2.set(True);
      
    cout << "d1=" << d1 << endl;
    cout << "d2=" << d2 << endl;

    VisVector VV(visType(4),False);
    VV.sync(vis(0),fl(0));

    cout << "VV raw = " << VV << endl;

    Jones J1, J2;

    J1.sync(d1(0),ok1(0));
    J2.sync(d2(0),ok2(0));

    J1.applyRight(VV);
    J2.applyLeft(VV);

    cout << "VV corrupted = " << VV << endl;


    Complex a;
    a=d1(1);
    d1(1)-=a;
    d2(1)-=a;
    d1(2)+=conj(a);
    d2(2)+=conj(a);

    cout << "d1=" << d1 << endl;
    cout << "d2=" << d2 << endl;

    J1.invert();
    J2.invert();

    J1.applyRight(VV);
    J2.applyLeft(VV);


    cout << "VV corrected = " << VV << endl;

  }
}

void testDiagJones (Bool /*verbose=False*/) {

  {

    Vector<Complex> vis(4);
    vis(0)=vis(3)=Complex(1.);
    vis(1)=vis(2)=Complex(0.);

    Vector<Bool> fl(4);
    fl.set(False);
    

    // Jones (general) D
    Vector<Complex> g1(2), g2(2);
    g1(0)=Complex(0.005,0.015);
    g1(1)=Complex(0.025,0.035);
    g2(0)=Complex(0.01,0.02);
    g2(1)=Complex(0.03,0.04);
    Vector<Bool> ok1(2), ok2(2);
    ok1.set(True);
    ok2.set(True);
      
    VisVector VV(visType(4),False);
    VV.sync(vis(0),fl(0));

    cout << "VV raw = " << VV << endl;

    JonesDiag J1, J2;

    J1.sync(g1(0),ok1(0));
    J2.sync(g2(0),ok2(0));

    J1.applyRight(VV);
    J2.applyLeft(VV);

    cout << "VV corrupted = " << VV << endl;

    ok2(1)=False;

    J1.invert();
    J2.invert();

    J1.applyRight(VV);
    J2.applyLeft(VV);


    cout << "VV corrected = " << VV << endl;

  }
}

#define complextype Complex
#define floattype Float 

void testMueller (Bool verbose=False) {

  {

    Double tol(10*FLT_EPSILON);
    
    if (verbose) {
      cout << boolalpha;
      cout << "tol = " << tol << endl;
    }

    Float I(10.0),Q(0.1),U(0.0),V(0.0);

    // vis4o,fl4o
    Cube<complextype> vis4o(4,1,16);
    initVis(vis4o,I,Q,U,V);
    Cube<Bool> fl4o(4,1,16,False);  
    fl4o(Slice(0,1,1),Slice(),Slice(8,8,1))=True;
    fl4o(Slice(1,1,1),Slice(),Slice(4,4,1))=True;
    fl4o(Slice(1,1,1),Slice(),Slice(12,4,1))=True;
    fl4o(Slice(2,1,1),Slice(),Slice(2,4,4))=True;
    fl4o(Slice(2,1,1),Slice(),Slice(3,4,4))=True;
    fl4o(Slice(3,1,1),Slice(),Slice(1,8,2))=True;

    // vis2o,fl2o
    Cube<complextype> vis2o(2,1,3); 
    initVis(vis2o,I,Q,U,V);
    Cube<Bool> fl2o(2,1,3,False);
    fl2o(0,0,1)=True;
    fl2o(1,0,2)=True;

    // M4, all ok
    Vector<complextype> m4o(4);
    m4o(0)=m4o(3)=Complex(Q);
    m4o(1)=m4o(2)=Complex(I);
    Vector<Bool> ok4o(4,True);
    Vector<complextype> m4;
    m4.assign(m4o);
    Vector<Bool> ok4;
    ok4.assign(ok4o);
    MuellerDiag M4;
    M4.sync(m4(0),ok4(0));

    // M2, all ok
    Vector<complextype> m2o(2);
    m2o(0)=m2o(1)=Complex(Q);
    Vector<Bool> ok2o(2,True);
    Vector<complextype> m2;
    m2.assign(m2o);
    Vector<Bool> ok2;
    ok2.assign(ok2o);
    MuellerDiag2 M2;
    M2.sync(m2(0),ok2(0));


    //***************************************************
    // 4-correlation tests
    {
      cout << "M4(V4) tests.............." << endl;

      Cube<complextype> vis4(4,1,16);
      Cube<Bool> fl4(4,1,16);
      VisVector V4(visType(4),False);

      // M4(V4) tests
      {

	// init vis4,fl4
	vis4.assign(vis4o);
	fl4.assign(fl4o);

	// init m4
	m4.assign(m4o);
	ok4.assign(ok4o);
	M4.sync(m4(0),ok4(0));

	V4.sync(vis4(0,0,0),fl4(0,0,0));
	for (Int i=0;i<16;++i,V4++) {
	  //cout << i << ": " << V4 << "->";
	  M4.apply(V4);
	  //cout << V4 << endl;
	}
	
	Cube<Bool> tfl(4,1,16,False);
	tfl.assign(fl4o);
	for (Int i=0;i<4;++i) {
	  if (!ok4o(i))
	    tfl(Slice(i,1,1),Slice(),Slice())=True;
	}
	
	if (verbose) {
	  cout << " allNearAbs(vis4,Complex(1.0),tol)  = " << allNearAbs(vis4,Complex(1.0),tol) << endl;
	  cout << " allEQ(fl4,tfl)                    = " << allEQ(fl4,tfl) << endl;
	}
	
	AlwaysAssert(allNearAbs(vis4,Complex(1.0),tol),AipsError);  // should be all Complex(1.0)
	AlwaysAssert(allEQ(fl4,tfl),AipsError);  // should be no change
	
	// Invert the Mueller
	M4.invert();
      
	if (verbose) {
	  cout << " allNearAbs(Minv*M,1.0,tol)        = " << allNearAbs(m4*m4o,Complex(1.0),tol) << endl;
	}
	AlwaysAssert(allNearAbs(m4*m4o,Complex(1.0),tol),AipsError); // should be all Complex(1.0)
	
	
	V4.sync(vis4(0,0,0),fl4(0,0,0));
	for (Int i=0;i<16;++i,V4++) {
	  //cout << i << ": " << V4 << "->";
	  M4.apply(V4);
	  //cout << V4 << endl;
	}
	
	Array<floattype> A=amplitude(vis4(!fl4).getCompressedArray()-vis4o(!fl4).getCompressedArray());
	
	if (verbose) {
	  cout << " allNearAbs(A,floattype(0.0),tol)  = " << allNearAbs(A,floattype(0.0),tol) << endl;
	  cout << " allNearAbs(vis4(!fl4),vis4o(!fl4),tol) = " 
	       << allNearAbs(vis4(!fl4).getCompressedArray(),
			     vis4o(!fl4).getCompressedArray(),tol) << endl;
	}
	AlwaysAssert(allNearAbs(A,floattype(0.0),tol),AipsError);
	
	
	// Invert back
	M4.invert();
	
	// Flag 3rd and 4th element of Mueller
	ok4(2)=ok4(3)=False;
	
	V4.sync(vis4(0,0,0),fl4(0,0,0));
	for (Int i=0;i<16;++i,V4++) {
	  //cout << i << ": " << V4 << "->";
	  M4.apply(V4);
	  //cout << V4 << endl;
	}

	// Check each correlation for correctness
	if (verbose) {
	  cout << " allEQ(fl4(Slice(0,1,1),Slice(),Slice()),fl4o(Slice(0,1,1),Slice(),Slice())) = " 
	       << allEQ(fl4(Slice(0,1,1),Slice(),Slice()),fl4o(Slice(0,1,1),Slice(),Slice())) << endl;
	  cout << " allEQ(fl4(Slice(1,1,1),Slice(),Slice()),fl4o(Slice(1,1,1),Slice(),Slice())) = " 
	       << allEQ(fl4(Slice(1,1,1),Slice(),Slice()),fl4o(Slice(1,1,1),Slice(),Slice())) << endl;
	  cout << " allEQ(fl4(Slice(2,1,1),Slice(),Slice()),True) = " << allEQ(fl4(Slice(2,1,1),Slice(),Slice()),True) << endl;
	  cout << " allEQ(fl4(Slice(3,1,1),Slice(),Slice()),True) = " << allEQ(fl4(Slice(3,1,1),Slice(),Slice()),True) << endl;
	}
	AlwaysAssert(allEQ(fl4(Slice(0,1,1),Slice(),Slice()),fl4o(Slice(0,1,1),Slice(),Slice())),AipsError);
	AlwaysAssert(allEQ(fl4(Slice(1,1,1),Slice(),Slice()),fl4o(Slice(1,1,1),Slice(),Slice())),AipsError);
	AlwaysAssert(allEQ(fl4(Slice(2,1,1),Slice(),Slice()),True),AipsError);
	AlwaysAssert(allEQ(fl4(Slice(3,1,1),Slice(),Slice()),True),AipsError);
      } // M4(V4) tests


      // M2(V4)
      {
	cout << "M2(V4) tests............" << endl;

	// Re-init vis4,fl4
	vis4.assign(vis4o);
	fl4.assign(fl4o);

	V4.sync(vis4(0,0,0),fl4(0,0,0));
	for (Int i=0;i<16;++i,V4++) {
	  //cout << i << ": " << V4 << "->";
	  M2.apply(V4);
	  //cout << V4 << endl;
	}

	if (verbose) {
	  cout << " allNearAbs(vis4(p-hands),Complex(1.0),tol)  = " << allNearAbs(vis4(Slice(0,2,3),Slice(),Slice()),Complex(1.0),tol) << endl;
	  cout << " allNearAbs(vis4(x-hands),Complex(0.0),tol)  = " << allNearAbs(vis4(Slice(1,2,1),Slice(),Slice()),Complex(0.0),tol) << endl;
	  cout << " allEQ(fl4,fl4o)                             = " << allEQ(fl4,fl4o) << endl;
	}
	
	AlwaysAssert(allNearAbs(vis4(Slice(0,2,3),Slice(),Slice()),Complex(1.0),tol),AipsError);  // should be all Complex(1.0)
	AlwaysAssert(allNearAbs(vis4(Slice(1,2,1),Slice(),Slice()),Complex(0.0),tol),AipsError);  // should be all Complex(0.0)
	AlwaysAssert(allEQ(fl4,fl4o),AipsError);  // should be no change
	
	// Invert the Mueller
	M2.invert();
      
	if (verbose) {
	  cout << " allNearAbs(Minv*M,1.0,tol)        = " << allNearAbs(m2*m2o,Complex(1.0),tol) << endl;
	}
	AlwaysAssert(allNearAbs(m2*m2o,Complex(1.0),tol),AipsError); // should be all Complex(1.0)
	
	
	V4.sync(vis4(0,0,0),fl4(0,0,0));
	for (Int i=0;i<16;++i,V4++) {
	  //cout << i << ": " << V4 << "->";
	  M2.apply(V4);
	  //cout << V4 << endl;
	}
	
	Array<floattype> A=amplitude(vis4(Slice(0,2,3),Slice(),Slice())-vis4o(Slice(0,2,3),Slice(),Slice()));
	
	if (verbose) {
	  cout << " allNearAbs(A,floattype(0.0),tol)  = " << allNearAbs(A,floattype(0.0),tol) << endl;
	  cout << " allNearAbs(vis4(!fl4),vis4o(!fl4),tol) = " 
	       << allNearAbs(vis4(Slice(0,2,3),Slice(),Slice()),
			     vis4o(Slice(0,2,3),Slice(),Slice()),tol) << endl;
	}
	AlwaysAssert(allNearAbs(A,floattype(0.0),tol),AipsError);
	
	
	// Invert back
	M2.invert();
	
	// Flag 2nd Mueller element
	ok2(1)=False;
	
	V4.sync(vis4(0,0,0),fl4(0,0,0));
	for (Int i=0;i<16;++i,V4++) {
	  //cout << i << ": " << V4 << "->";
	  M2.apply(V4);
	  //cout << V4 << endl;
	}

	// Check each correlation for correctness
	if (verbose) {
	  cout << " allEQ(fl4(Slice(0,1,1),Slice(),Slice()),fl4o(Slice(0,1,1),Slice(),Slice())) = " 
	       << allEQ(fl4(Slice(0,1,1),Slice(),Slice()),fl4o(Slice(0,1,1),Slice(),Slice())) << endl;
	  cout << " allEQ(fl4(Slice(3,1,1),Slice(),Slice()),True) = " << allEQ(fl4(Slice(3,1,1),Slice(),Slice()),True) << endl;
	}
	AlwaysAssert(allEQ(fl4(Slice(0,1,1),Slice(),Slice()),fl4o(Slice(0,1,1),Slice(),Slice())),AipsError);
	AlwaysAssert(allEQ(fl4(Slice(3,1,1),Slice(),Slice()),True),AipsError);
      }
    }


    //***************************************************
    // 2-correlation tests
    {
      cout << "M4(V2) tests.............." << endl;

      Cube<complextype> vis2(2,1,3);
      Cube<Bool> fl2(2,1,3);
      VisVector V2(visType(2),False);

      // M4(V2) tests
      {
	// init vis2,fl2
	vis2.assign(vis2o);
	fl2.assign(fl2o);

	// init m4
	m4.assign(m4o);
	ok4.assign(ok4o);
	M4.sync(m4(0),ok4(0));

	V2.sync(vis2(0,0,0),fl2(0,0,0));
	for (Int i=0;i<3;++i,V2++) {
	  //cout << i << ": " << V2 << "->";
	  M4.apply(V2);
	  //cout << V2 << endl;
	}
	
	if (verbose) {
	  cout << " allNearAbs(vis2,Complex(1.0),tol)  = " << allNearAbs(vis2,Complex(1.0),tol) << endl;
	  cout << " allEQ(fl2,fl2o)                     = " << allEQ(fl2,fl2o) << endl;
	}
	
	AlwaysAssert(allNearAbs(vis2,Complex(1.0),tol),AipsError);  // should be all Complex(1.0)
	AlwaysAssert(allEQ(fl2,fl2o),AipsError);  // should be no change
	
	// Invert the Mueller
	M4.invert();
      
	if (verbose) {
	  cout << " allNearAbs(Minv*M,1.0,tol)        = " << allNearAbs(m4*m4o,Complex(1.0),tol) << endl;
	}
	AlwaysAssert(allNearAbs(m4*m4o,Complex(1.0),tol),AipsError); // should be all Complex(1.0)
	
	
	V2.sync(vis2(0,0,0),fl2(0,0,0));
	for (Int i=0;i<3;++i,V2++) {
	  //cout << i << ": " << V2 << "->";
	  M4.apply(V2);
	  //cout << V2 << endl;
	}
	
	Array<floattype> A=amplitude(vis2(!fl2).getCompressedArray()-vis2o(!fl2).getCompressedArray());
	
	if (verbose) {
	  cout << " allNearAbs(A,floattype(0.0),tol)  = " << allNearAbs(A,floattype(0.0),tol) << endl;
	  cout << " allNearAbs(vis2(!fl2),vis4o(!fl2),tol) = " 
	       << allNearAbs(vis2(!fl2).getCompressedArray(),
			     vis2o(!fl2).getCompressedArray(),tol) << endl;
	}
	AlwaysAssert(allNearAbs(A,floattype(0.0),tol),AipsError);
	
	
	// Invert back
	M4.invert();
	
	// Flag 3rd and 4th element of Mueller
	ok4(2)=ok4(3)=False;
	
	V2.sync(vis2(0,0,0),fl2(0,0,0));
	for (Int i=0;i<3;++i,V2++) {
	  //cout << i << ": " << V2 << "->";
	  M4.apply(V2);
	  //cout << V2 << endl;
	}

	// Check each correlation for correctness
	if (verbose) {
	  cout << " allEQ(fl2(Slice(0,1,1),Slice(),Slice()),fl2o(Slice(0,1,1),Slice(),Slice())) = " 
	       << allEQ(fl2(Slice(0,1,1),Slice(),Slice()),fl2o(Slice(0,1,1),Slice(),Slice())) << endl;
	  cout << " allEQ(fl2(Slice(1,1,1),Slice(),Slice()),True) = " << allEQ(fl2(Slice(1,1,1),Slice(),Slice()),True) << endl;
	}
	AlwaysAssert(allEQ(fl2(Slice(0,1,1),Slice(),Slice()),fl2o(Slice(0,1,1),Slice(),Slice())),AipsError);
	AlwaysAssert(allEQ(fl2(Slice(1,1,1),Slice(),Slice()),True),AipsError);
      } // M4(V2) tests


      // M2(V2)
      {
	cout << "M2(V2) tests............" << endl;

	// Re-init vis2,fl2
	vis2.assign(vis2o);
	fl2.assign(fl2o);

	// Init m2,ok2
	m2.assign(m2o);
	ok2.assign(ok2o);
	M2.sync(m2(0),ok2(0));

	V2.sync(vis2(0,0,0),fl2(0,0,0));
	for (Int i=0;i<3;++i,V2++) {
	  //cout << i << ": " << V2 << "->";
	  M2.apply(V2);
	  //cout << V2 << endl;
	}

	if (verbose) {
	  cout << " allNearAbs(vis2(p-hands),Complex(1.0),tol)  = " << allNearAbs(vis2(Slice(0,2,1),Slice(),Slice()),Complex(1.0),tol) << endl;
	  cout << " allEQ(fl2,fl2o)                             = " << allEQ(fl2,fl2o) << endl;
	}
	
	AlwaysAssert(allNearAbs(vis2(Slice(0,2,1),Slice(),Slice()),Complex(1.0),tol),AipsError);  // should be all Complex(1.0)
	AlwaysAssert(allEQ(fl2,fl2o),AipsError);  // should be no change
	
	// Invert the Mueller
	M2.invert();
      
	if (verbose) {
	  cout << " allNearAbs(Minv*M,1.0,tol)        = " << allNearAbs(m2*m2o,Complex(1.0),tol) << endl;
	}
	AlwaysAssert(allNearAbs(m2*m2o,Complex(1.0),tol),AipsError); // should be all Complex(1.0)
	
	
	V2.sync(vis2(0,0,0),fl2(0,0,0));
	for (Int i=0;i<3;++i,V2++) {
	  //cout << i << ": " << V2 << "->";
	  M2.apply(V2);
	  //cout << V2 << endl;
	}
	
	Array<floattype> A=amplitude(vis2(Slice(0,2,1),Slice(),Slice())-vis2o(Slice(0,2,1),Slice(),Slice()));
	
	if (verbose) {
	  cout << " allNearAbs(A,floattype(0.0),tol)  = " << allNearAbs(A,floattype(0.0),tol) << endl;
	  cout << " allNearAbs(vis2(!fl2),vis2o(!fl2),tol) = " 
	       << allNearAbs(vis2(Slice(0,2,1),Slice(),Slice()),
			     vis2o(Slice(0,2,1),Slice(),Slice()),tol) << endl;
	}
	AlwaysAssert(allNearAbs(A,floattype(0.0),tol),AipsError);
	
	
	// Invert back
	M2.invert();
	
	// Flag 2nd Mueller element
	ok2(1)=False;
	
	V2.sync(vis2(0,0,0),fl2(0,0,0));
	for (Int i=0;i<3;++i,V2++) {
	  //cout << i << ": " << V2 << "->";
	  M2.apply(V2);
	  //cout << V2 << endl;
	}

	// Check each correlation for correctness
	if (verbose) {
	  cout << " allEQ(fl2(Slice(0,1,1),Slice(),Slice()),fl2o(Slice(0,1,1),Slice(),Slice())) = " 
	       << allEQ(fl2(Slice(0,1,1),Slice(),Slice()),fl2o(Slice(0,1,1),Slice(),Slice())) << endl;
	  cout << " allEQ(fl2(Slice(1,1,1),Slice(),Slice()),True) = " << allEQ(fl2(Slice(1,1,1),Slice(),Slice()),True) << endl;
	}
	AlwaysAssert(allEQ(fl2(Slice(0,1,1),Slice(),Slice()),fl2o(Slice(0,1,1),Slice(),Slice())),AipsError);
	AlwaysAssert(allEQ(fl2(Slice(1,1,1),Slice(),Slice()),True),AipsError);
      } // M2(V2)

    } // V2 tests

  } // testMueller

}


int main ()
{
  try {

    //    /*
    testVisVector(JONES_VERBOSE);
    testJonesApply(JONES_VERBOSE);
    testGenJones(JONES_VERBOSE);
    testGenJones2(JONES_VERBOSE);
    testGenLinJones(JONES_VERBOSE);
    testDiagJones(JONES_VERBOSE);
    //    */
    testMueller(JONES_VERBOSE);


  } catch (AipsError x) {
    cout << "Unexpected exception: " << x.getMesg() << endl;
    exit(1);
  } catch (...) {
    cout << "Unexpected unknown exception" << endl;
    exit(1);
  }
  cout << "OK" << endl;
  exit(0);
};
