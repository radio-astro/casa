//# tCLPatchPanel: test program for CLPatchPanel
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

#include <synthesis/CalTables/CLPatchPanel.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
//#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>

#include <synthesis/CalTables/CTInterface.h>
#include <ms/MSSel/MSSelection.h>
#include <ms/MSSel/MSSelectionTools.h>

// <summary>
// Test program for RIorAParray class.
// </summary>

// Control verbosity
#define CLPATCHPANELTEST_VERBOSE False

void tMSCalPatchKey (Bool verbose=False) {

  Bool& v(verbose);

  cout << "---tMSCalPatchKey--BEGIN--" << endl;

  {  

    MSCalPatchKey cpk0(0,1,-1,3,4);
    MSCalPatchKey cpk1(0,1,-1,3,4);
    MSCalPatchKey cpk2(0,1,-1,3,5);
    MSCalPatchKey cpk3(0,1,-1,2,5);

    if (v) {
      cout << "cpk0: " << cpk0.print() << endl;
      cout << "cpk1: " << cpk1.print() << endl;
      cout << "cpk2: " << cpk2.print() << endl;
      cout << "cpk3: " << cpk3.print() << endl;
      
      cout << boolalpha;
      cout << "cpk1 < cpk0 = " << (cpk1<cpk0) << " (should be false)"<< endl;
      cout << "cpk0 < cpk1 = " << (cpk0<cpk1) << " (should be false)"<< endl;
      
      cout << "cpk2 < cpk0 = " << (cpk2<cpk0) << " (should be false)"<< endl;
      cout << "cpk0 < cpk2 = " << (cpk0<cpk2) << " (should be true)"<< endl;

      cout << "cpk3 < cpk0 = " << (cpk3<cpk0) << " (should be true)"<< endl;
      cout << "cpk0 < cpk3 = " << (cpk0<cpk3) << " (should be false)"<< endl;
    }

    std::map<MSCalPatchKey,String> m;

    m[cpk0] = "<"+cpk0.print()+">";
    m[cpk2] = "<"+cpk2.print()+">";

    if (v) {
      cout << "m.size() = " << m.size() << endl;
      cout << "m.max_size() = " << m.max_size() << endl;

      cout << "m.count(cpk0) = " << m.count(cpk0) << endl;
      cout << "m.count(cpk1) = " << m.count(cpk0) << endl;
      cout << "m.count(cpk2) = " << m.count(cpk0) << endl;
      cout << "m.count(cpk3) = " << m.count(cpk3) << endl;
      
      for (std::map<MSCalPatchKey,String>::iterator it=m.begin(); 
	   it!=m.end(); ++it) {
	cout << it->first.print() << " => " << it->second << endl;
      //      cout << "test: " << it->operator[](cpk0) << endl;
      }

    }

  }

  cout << "---tMSCalPatchKey---END---" << endl << endl;

}

void tMSCalPatchKey2 (Bool verbose=False) {

  Bool& v(verbose);

  cout << "---tMSCalPatchKey2--BEGIN--" << endl;

  {
    MSCalPatchKey cpk0(0,1,-1,3,5);

    std::map<MSCalPatchKey,String> c;
    std::map<MSCalPatchKey,String> m;

    c[cpk0]="Hello";
    m[cpk0]=c[cpk0];

    if (v) {
      cout << "Addresses: c:" << &(c[cpk0]) << " m:" << &(m[cpk0]) << endl;
      cout << "Content 1: " << c[cpk0] << " " << m[cpk0] << endl;
    }

    c[cpk0]="Goodbye";

    if (v) cout << "Content 2: " << c[cpk0] << " " << (m[cpk0]) << endl;

  }
  cout << "---tMSCalPatchKey2---END---" << endl << endl;

}


void tCalMap1 (Bool verbose=False) {

  Bool& v(verbose);

  cout << "---tCalMap1--BEGIN--" << endl;

  {

    CalMap c;

    AlwaysAssert( c.vmap().nelements()==0, AipsError );
    if (v) cout << "c.vmap() = " << c.vmap() << endl;

    AlwaysAssert( c(3)==3, AipsError );
    if (v) cout << "c(3)     = " << c(3) << endl;


    cout << "NEED TO FIX THESE..." << endl;

    Vector<Int> msids(3);
    indgen(msids);
    msids+=1;
    //    AlwaysAssert( allEQ(c.ctids(msids),msids), AipsError );
    if (v) cout << "c.ctids(msids) = " << c.ctids(msids) << endl;
    //    AlwaysAssert( c.msids(3)(0)==3, AipsError );
    if (v) cout << "c.msids(3)     = " << c.msids(3) << endl;

  }
  cout << "---tCalMap1---END---" << endl << endl;

}

void tCalMap2 (Bool verbose=False) {

  Bool& v(verbose);

  cout << "---tCalMap2--BEGIN--" << endl;

  {

    Vector<Int> map(4,0);
    indgen(map);
    map(2)=1;
    CalMap cmap(map);

    if (v) cout << "cmap.vmap() = " << cmap.vmap() << endl;

    for (Int i=0;i<4;++i) {
      Int shouldbe(map(i));
      AlwaysAssert( cmap(i)==shouldbe, AipsError );
      if (v) 
	cout << "ms " << i << " by ct " << cmap(i) 
	     << "  (should be: " << shouldbe << ")"
	     << endl;
    }

    {
      Vector<Int> msids(2);
      indgen(msids);
      msids+=1;
      
      Vector<Int> shouldbe(1,1);
      AlwaysAssert( allEQ(cmap.ctids(msids),shouldbe), AipsError );
      if (v)
	cout << "ms " <<  msids << " requires ct " << cmap.ctids(msids) 
	     << "  (should be: " << shouldbe << ")"
	     << endl;
    }
    
    {
      Vector<Int> msids(4);
      indgen(msids);

      for (Int i=0;i<4;++i) {
	Vector<Int> shouldbe=msids(cmap.vmap()==i).getCompressedArray();
	AlwaysAssert( allEQ(cmap.msids(i),shouldbe), AipsError );
	if (v)
	  cout << "ct " << i << " cals ms " << cmap.msids(i) 
	       << "  (should be: " << shouldbe << ")"
	       << endl;
      }
    }

  }
  cout << "---tCalMap2---END---" << endl << endl;

}

void tCalLibSlice1 (Bool verbose=False) {

  Bool& v(verbose);

  cout << "---tCalLibSlice1--BEGIN--" << endl;

  {
    
    String obs("");
    String fld("1");
    String ent("");
    String spw("0,1,2");
    String tinterp("linear"),finterp("linear");
    Vector<Int> spwmap(3,0);
    spwmap(1)=spwmap(2)=1;
    Vector<Int> obsmap(1,-1), fldmap(1,-1), antmap(1,-1);

    CalLibSlice cls(obs,fld,ent,spw,tinterp,finterp,
		    obsmap,fldmap,spwmap,antmap);

    if (v)
      cout << cls.state();

    AlwaysAssert( cls.obs==obs, AipsError);
    AlwaysAssert( cls.fld==fld, AipsError);
    AlwaysAssert( cls.ent==ent, AipsError);
    AlwaysAssert( cls.spw==spw, AipsError);
    AlwaysAssert( cls.tinterp==tinterp, AipsError);
    AlwaysAssert( cls.finterp==finterp, AipsError);
    AlwaysAssert( allEQ(cls.obsmap.vmap(),obsmap), AipsError);
    AlwaysAssert( allEQ(cls.fldmap.vmap(),fldmap), AipsError);
    AlwaysAssert( allEQ(cls.spwmap.vmap(),spwmap), AipsError);
    AlwaysAssert( allEQ(cls.antmap.vmap(),antmap), AipsError);

    Record r=cls.asRecord();

    CalLibSlice cls2(r);
    Record r2=cls2.asRecord();

    AlwaysAssert( r.conform(r2), AipsError );
    if (v)
      cout << "r.conform(r2) = " << boolalpha << r.conform(r2) << endl;

    if (v)
      cout << cls2.state();
   
  }
  cout << "---tCalLibSlice1---END---" << endl << endl;

}

void testCLPPResult (Bool verbose=True) {

  Bool& v(verbose);

  cout << "---tCLPPResult1--BEGIN--" << endl;

  {

    IPosition ipos(3,2,1,10);

    CLPPResult clpp(ipos);
    clpp(3).set(3.0);

    if (v)
      cout << "clpp.result_ = " << clpp.result_ << endl;

    Matrix<Float> m2;
    m2.reference(clpp(5));
    m2.set(5.0);

    if (v) {
      cout << "clpp.result_ = " << clpp.result_ << endl;
      cout << "clpp.result_.nrefs() = " << clpp.result_.nrefs() << endl;
    }

    CTCalPatchKey cpk0(0,1,-1,3,-1);
    std::map<CTCalPatchKey,CLPPResult> c;
    c[cpk0]=clpp;  

    if (v) {
      cout << "c[cpk0].result(5) = " << c[cpk0].result(5) << endl;
      cout << "c[cpk0].result_.data() = " << c[cpk0].result_.data() << endl;
      cout << "clpp.result_.data()    = " << clpp.result_.data() << endl;
    }

    clpp(5).set(999.0);
    if (v)
      cout << "c[cpk0].result(5) = " << c[cpk0].result(5) << endl;

    {
      if (v) cout << "Via pointers..." << endl;
      std::map<CTCalPatchKey,CLPPResult*> cp;
      cp[cpk0]=&clpp;
      if (v) {
	cout << "cp[cpk0]->result(5) = " << cp[cpk0]->result(5) << endl;
	cout << "cp[cpk0]->result_.data() = " << cp[cpk0]->result_.data() << endl;
	cout << "clpp.result_.data()      = " << clpp.result_.data() << endl;
      }
    }


    {
      if (v) cout << "Via new..." << endl;
      std::map<CTCalPatchKey,CLPPResult*> cp;
      cp[cpk0]=new CLPPResult(ipos);
      if (v) {
	cout << "cp[cpk0]->result_.data()  = " << cp[cpk0]->result_.data() << endl;
	cout << "cp[cpk0]->result_.nrefs() = " << cp[cpk0]->result_.nrefs() << endl;
      }
      delete cp[cpk0];
    }


  }
  cout << "---tCLPPResult1--END--" << endl;

}


void tCLPatchPanel1 (Bool verbose=True) {

  Bool& v(verbose);

  cout << "---tCLPatchPanel1--BEGIN--" << endl;


  { // inner scope
    // Make a testing NewCalTable (Table::Memory)
    uInt nFld(3), nAnt(4), nSpw(4), nObs(2), nScan(4),nTime(10);
    Vector<Int> nChan(nSpw,4);
    Double refTime(4832568000.0); // 2012 Jan 06 @ noon
    Double tint(60.0);
    
    Bool disk(True); // verbose);
    NewCalTable tnct("tCTSelection1.ct","Complex",
		     nObs,nScan,nTime,
		     nAnt,nSpw,nChan,
		     nFld,
		     refTime,tint,disk,False);

    cout << "Created " << tnct.tableName() << endl;


    String obs("0,1");
    String ent("");
    String spw("");
    String tinterp("linear"),finterp("linear");
    Vector<Int> obsmap(1,-1), fldmap(1,-1),antmap(1,-1);

    String fld0("0,1"); 
    Vector<Int> spwmap0(nSpw,0); indgen(spwmap0); spwmap0(1)=spwmap0(2)=1;
    Vector<Int> fldmap0(nFld,1); //fldmap0(2)=-1;
    CalLibSlice cls0(obs,fld0,ent,spw,tinterp,finterp,
		     obsmap,fldmap0,spwmap0,antmap);

    String fld1("2");
    Vector<Int> obsmap1(nObs,0); indgen(obsmap1);
    Vector<Int> spwmap1(nSpw,2); indgen(spwmap1); spwmap1(0)=spwmap1(1)=0;
    Vector<Int> antmap1(nAnt,0); indgen(antmap1); antmap1(0)=2; antmap1(1)=1;antmap1(2)=0;
    Vector<Int> fldmap1(nFld,0);
    CalLibSlice cls1(obs,fld1,ent,spw,tinterp,finterp,
		     obsmap1,fldmap1,spwmap1,antmap1);

    Record cl;
    cl.defineRecord("*0",cls0.asRecord());
    cl.defineRecord("*1",cls1.asRecord());

    {


      CLPatchPanel ctpp("tCTSelection1.ct",cl,VisCalEnum::JONES,1);
      ctpp.listmappings();

      // Simple time-dep-only interp
      Int ispw(1);
      Cube<Complex> res;
      Cube<Bool> resfl;
      Double t=refTime+27.0;
      Bool newcal(False);
      newcal=ctpp.interpolate(res,resfl,0,2,0,ispw,t,-1.0);
      
      if (v) {
	cout << "newcal = " << boolalpha << newcal << endl;
	cout << "res=" << res << endl;
	cout << "resfl=" << boolalpha << resfl << endl;
	cout << "amplitude(res) = " << amplitude(res) << endl;
	cout << "phase(res)     = " << phase(res) << endl;
      }
      // Subtract manually-calculated result, expecting ZERO
      for (uInt iant=0;iant<nAnt;++iant) {
	for (Int ich=0;ich<nChan(ispw);++ich) {
	  res(0,ich,iant)-=NewCalTable::NCTtestvalueC(antmap1(iant),spwmap1(ispw),ich,t,refTime,tint);
	}
      }
      
      if (v)
	cout << "zeroed res = " << res << endl;
      
      Double maxres=max(amplitude(res));
      cout << "Max res residual: " << maxres << " " << maxres/FLT_EPSILON << endl;
      
      // res should be all ~zero
      AlwaysAssert( allNearAbs(res,Complex(0.0f),5.*FLT_EPSILON), AipsError );
    }

    {

      CLPatchPanel ctpp("tCTSelection1.ct",cl,VisCalEnum::JONES,1);
      ctpp.listmappings();

      // Non-trivially resampled freq interp

      Int ispw(0);
      Cube<Complex> res;
      Cube<Bool> resfl;
      Double t=refTime+27.0;
      Bool newcal(False);
      Int nChan1(7);
      Vector<Double> freq(nChan1,60.0005e9);

      for (Int ich=1;ich<nChan1;++ich) 
	freq[ich]+=Double(ich)*0.5e6;

      freq/=1.e9;  // in GHz

      newcal=ctpp.interpolate(res,resfl,0,2,0,ispw,t,freq);
      
      if (v||True) {
	cout << "newcal = " << boolalpha << newcal << endl;
	cout << "res=" << res << endl;
	cout << "resfl=" << boolalpha << resfl << endl;
	cout << "amplitude(res) = " << amplitude(res) << endl;
	cout << "phase(res)     = " << phase(res) << endl;
      }
      // Subtract manually-calculated result, expecting ZERO
      for (uInt iant=0;iant<nAnt;++iant) {
	for (Int ich=0;ich<nChan1;++ich) {
	  Double chan=Double(ich)/2.0;
	  cout << res(0,ich,iant) << " " << NewCalTable::NCTtestvalueC(antmap1(iant),spwmap1(ispw),chan,t,refTime,tint) << endl;
	  res(0,ich,iant)-=NewCalTable::NCTtestvalueC(antmap1(iant),spwmap1(ispw),chan,t,refTime,tint);
	}
      }
      
      if (v)
	cout << "zeroed res = " << res << endl;
      
      Double maxres=max(amplitude(res));
      cout << "Max res residual: " << maxres << " " << maxres/FLT_EPSILON << endl;
      
      // res should be all ~zero
      AlwaysAssert( allNearAbs(res,Complex(0.0f),5.*FLT_EPSILON), AipsError );
    }



  }
  cout << "---tCLPatchPanel1--END--" << endl;

}



int main ()
{
  try {


    tMSCalPatchKey(CLPATCHPANELTEST_VERBOSE);
    tMSCalPatchKey2(CLPATCHPANELTEST_VERBOSE);
    tCalMap1(CLPATCHPANELTEST_VERBOSE);
    tCalMap2(CLPATCHPANELTEST_VERBOSE);
    tCalLibSlice1(CLPATCHPANELTEST_VERBOSE);
    testCLPPResult(CLPATCHPANELTEST_VERBOSE);
    tCLPatchPanel1(CLPATCHPANELTEST_VERBOSE);

    
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
