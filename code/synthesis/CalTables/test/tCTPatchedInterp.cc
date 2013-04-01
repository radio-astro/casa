//# tCTPatchedInterp.cc: Test program for CTPatchedInterp class
//# Copyright (C) 2011
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
//# $Id: tNewCalTable.cc 15602 2011-07-14 00:03:34Z tak.tsutsumi $

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTPatchedInterp.h>
#include <synthesis/CalTables/VisCalEnum.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <casa/OS/Timer.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>
#include <casa/Arrays/ArrayMath.h>

// <summary>
// Test program for CTInterp class.
// </summary>

// Control verbosity
#define CTPATCHEDINTERPTEST_VERBOSE False

void doTest1 (Bool verbose=False) {

  cout << "****----doTest1()----****" << endl;
  
  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(1), nAnt(3), nSpw(2), nObs(1), nScan(1), nTime(100);
  Vector<Int> nChan(nSpw,1);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(verbose);
  NewCalTable tnct("tCTPatchedInterp_test1.ct","Complex",
		   nObs,nScan,nTime,
		   nAnt,nSpw,nChan,
		   nFld,
		   refTime,tint,disk,False);

  // some sanity checks on the test NewCalTable
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << " (should be " << Table::Memory  << ")"
		    << endl;
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << " (should be " << nObs*nScan*nTime*nSpw*nAnt << ")"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nObs*nScan*nTime*nSpw*nAnt), AipsError);
  
  // Make a CTPatchedInterp  (no freq interp yet)
  CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linear","none","");
  if (verbose) ci.state();
  
  Bool newcal1=ci.interpolate(0,0,0,4832568310.0);
  Bool newcal2=ci.interpolate(0,0,1,4832568310.0);

  if (verbose) {
    cout << "new = " << newcal1  << endl;
    cout << "resultF = " << ci.resultF(0,0,0) << endl;
    cout << "new = " << newcal2 << endl;
    cout << "resultF = " << ci.resultF(0,0,1) << endl;
  }

  Double tol(2.e-7);
  Complex cfval0=NewCalTable::NCTtestvalueC(0,0,0,4832568310.0,refTime,tint);
  Complex cfval1=NewCalTable::NCTtestvalueC(0,1,0,4832568310.0,refTime,tint);

  Cube<Complex> rc0(ci.resultC(0,0,0));
  Complex r0(rc0(0,0,0));
  Complex d0=r0-cfval0;
  if (verbose) cout << "diff0=" << d0 << " (abs=" << abs(d0) << ")" << endl;
  AlwaysAssert( nearAbs(abs(d0),0.0f,tol), AipsError);

  Cube<Complex> rc1(ci.resultC(0,0,1));
  Complex r1(rc1(0,0,0));
  Complex d1=r1-cfval1;
  if (verbose) cout << "diff1=" << d1 << " (abs=" << abs(d1) << ")" << endl;
  AlwaysAssert( nearAbs(abs(d1),0.0f,tol), AipsError);

}

void doTest2 (Bool verbose=False) {

  cout << "****----doTest2()----****" << endl;

  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(1), nAnt(1), nSpw(1), nObs(1), nScan(1),  nTime(2);
  Vector<Int> nChan(nSpw,10);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);
  Bool disk(verbose);
  NewCalTable tnct("tCTPatchedInterp_test2.ct","Complex",
		   nObs,nScan,nTime,
		   nAnt,nSpw,nChan,
		   nFld,
		   refTime,tint,disk,False);

  // some sanity checks on the test NewCalTable
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << " (should be " << Table::Memory << ")"
		    << endl;
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << " (should be " << nObs*nScan*nTime*nSpw*nAnt << ")"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nObs*nScan*nTime*nSpw*nAnt), AipsError);
  
  // Make a CTPatchedInterp 
  CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linear","linear","");
  if (verbose) ci.state();

  Vector<Double> f(40); indgen(f);
  f-=3.1;  
  f*=0.5e6;  f+=60.e9; f/=1e9;  // in GHz

  Double t(4832568311.0);

  Bool newcal=ci.interpolate(0,0,0,t,f);

  if (verbose) {
    cout.precision(7);
    cout << "f=" << f << endl;
    cout << "new = " << newcal  << endl;
    cout << "resultF = " << ci.resultF(0,0,0) << endl;
  }
  
  Cube<Complex> rc(ci.resultC(0,0,0));
  Vector<Complex> r(rc(Slice(0,1,1),Slice(),Slice(0,1,1)));
  
  // Invent comparison values at fraction ichans manually (see NCTtestvalueC)
  Vector<Complex> cfval(40,NewCalTable::NCTtestvalueC(0,0,0,4832568060.0,refTime,tint));
  Vector<Double> fichan;
  fichan=f;  // in GHz
  fichan-=60.0;  // relative to 60.0 GHz
  fichan*=1000.0; // in MHz
  fichan-=0.5; // channels are centered at half-MHz
  for (Int ich=0;ich<40;++ich) {
    if (fichan(ich)<0.0) fichan(ich)=0.0;
    if (fichan(ich)>9.0) fichan(ich)=9.0;
    Double a=Double(abs(cfval(ich)))+fichan(ich)/10000.0;
    Double p=Double(arg(cfval(ich)))+fichan(ich)/100.0;
    cfval(ich)=Complex(a*cos(p),a*sin(p));
  }
 
  Vector<Complex> diff=r-cfval;
  if (verbose) cout << "diff = " << diff << endl;
  Double tol(2.e-7);
  AlwaysAssert( allNearAbs(amplitude(diff),0.0f,tol), AipsError);


}

void doTest3 (Bool verbose=False) {

  cout << "****----doTest3()----****" << endl;

  
  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(1), nAnt(1), nSpw(1), nObs(3),nScan(1),nTime(1);
  Vector<Int> nChan(nSpw,1);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(verbose);
  NewCalTable tnct("tCTPatchedInterp_test3.ct","Complex",
		   nObs,nScan,nTime,
		   nAnt,nSpw,nChan,
		   nFld,
		   refTime,tint,disk,False); 
		   
  // some sanity checks on the test NewCalTable
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << " (should be " << Table::Memory << ")"
		    << endl;
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << " (should be " << nObs*nScan*nTime*nSpw*nAnt << ")"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nObs*nScan*nTime*nSpw*nAnt), AipsError);
  if (verbose) cout << "Complex = " << tnct.isComplex() 
		    << " (should be True)"
		    << endl;
  AlwaysAssert( (tnct.isComplex()), AipsError);
  
  {
    if (verbose)
      cout << endl
	   << "Testing 'linear', which should ignore obsid and behave as linear within time bounds:" << endl;

    // Make a CTPatchedInterp  
    //  'linear' in time, ignoring obsid
    CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linear","none","");
    if (verbose) ci.state();
    
    Cube<Complex> r;
    cout.precision(10);
    Double thistime=refTime-30.0;
    Double tol(2.e-7);
    for (uInt iobs=0;iobs<nObs;++iobs) {
      for (Int itime=0;itime<10;++itime) {
	thistime+=Double(6.0);
	//    cout << itime << " " << thistime << endl;
	ci.interpolate(iobs,0,0,thistime);
	r.reference(ci.resultC(iobs,0,0));

	Float reltime=thistime-refTime;   // precise reltime
	reltime=max(reltime,0.0);
	reltime=min(reltime,(nObs-1)*tint);
	Complex cfval=NewCalTable::NCTtestvalueC(0,0,0,reltime,0.0,tint);
	Complex diff=r(0,0,0)-cfval;

	if (verbose)
	  cout << "iobs="<< iobs << " t=" << thistime-refTime 
	       << " A=" << abs(r(0,0,0)) << " P=" << arg(r(0,0,0)) 
	       << " (should be"
	       << " A=" << abs(cfval) << " P=" << arg(cfval)
	       << " diff= " << diff
	       << endl;

	AlwaysAssert( nearAbs(abs(diff),0.0f,tol), AipsError);
      }
    }
  }

  {

    if (verbose)
      cout << endl
	   << "Testing 'linearperobs', which should behave as nearest in each obsid:" << endl;

    // Make a CTPatchedInterp  (no freq interp yet)
    CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linearperobs","none","");
    if (verbose) ci.state();
    
    Cube<Complex> r;
    cout.precision(10);
    Double thistime=refTime-30.0;
    Double tol(2.e-7); // float precision
    for (uInt iobs=0;iobs<nObs;++iobs) {
      for (Int itime=0;itime<10;++itime) {
	thistime+=Double(6.0);
	//    cout << itime << " " << thistime << endl;
	ci.interpolate(iobs,0,0,thistime);
	r.reference(ci.resultC(iobs,0,0));

	Float reltime=iobs*tint;  // effective time is caltable timestamps
	Complex cfval=NewCalTable::NCTtestvalueC(0,0,0,reltime,0.0,tint);
	Complex diff=r(0,0,0)-cfval;

	if (verbose)
	  cout << "iobs="<< iobs << " t=" << thistime-refTime 
	       << " A=" << abs(r(0,0,0)) << " P=" << arg(r(0,0,0)) 
	       << " (should be"
	       << " A=" << abs(cfval) << " P=" << arg(cfval)
	       << " diff= " << diff
	       << endl;

	AlwaysAssert( nearAbs(abs(diff),0.0f,tol), AipsError);
      }
    }
  }

}

void doTest4 (Bool verbose=False) {

  cout << "****----doTest4()----****" << endl;

  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(1), nAnt(1), nSpw(1), nObs(3),nScan(1),nTime(1);
  Vector<Int> nChan(nSpw,1);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(verbose);
  NewCalTable tnct("tCTPatchedInterp_test4.ct","Float",
		   nObs,nScan,nTime,
		   nAnt,nSpw,nChan,
		   nFld,
		   refTime,tint,disk,False); 
		   
  // some sanity checks on the test NewCalTable
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << " (should be " << Table::Memory << ")"
		    << endl;
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << " (should be " << nObs*nScan*nTime*nSpw*nAnt << ")"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nObs*nScan*nTime*nSpw*nAnt), AipsError);
  if (verbose) cout << "Complex = " << tnct.isComplex() 
		    << " (should be False)"
		    << endl;
  AlwaysAssert( (!tnct.isComplex()), AipsError);
  
  {
    if (verbose)
      cout << endl
	   << "Testing 'linear', which should ignore obsid and behave as linear within time bounds:" << endl;

    // Make a CTPatchedInterp  
    //  'linear' in time, ignoring obsid
    CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linear","none","");
    //  ci.state();
    
    Cube<Float> r;
    cout.precision(10);
    Double thistime=refTime-30.0;
    Double tol(2.e-7);
    for (uInt iobs=0;iobs<nObs;++iobs) {
      for (Int itime=0;itime<10;++itime) {
	thistime+=Double(6.0);
	//    cout << itime << " " << thistime << endl;
	ci.interpolate(iobs,0,0,thistime);
	r.reference(ci.resultF(iobs,0,0));

	Float reltime=thistime-refTime;   // precise reltime
	reltime=max(reltime,0.0);
	reltime=min(reltime,(nObs-1)*tint);
	Float cfval=NewCalTable::NCTtestvalueF(0,0,0,reltime,0.0,tint);
	Float diff=r(0,0,0)-cfval;

	if (verbose)
	  cout << "iobs="<< iobs << " t=" << thistime-refTime 
	       << " V=" << r(0,0,0)
	       << " (should be"
	       << " V=" << cfval
	       << " diff= " << diff
	       << endl;

	AlwaysAssert( nearAbs(diff,0.0f,tol), AipsError);
      }
    }
  }

  {

    if (verbose)
      cout << endl
	   << "Testing 'linearperobs', which should behave as nearest in each obsid:" << endl;

    // Make a CTPatchedInterp  (no freq interp yet)
    CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linearperobs","none","");
    //  ci.state();
    
    Cube<Float> r;
    cout.precision(10);
    Double thistime=refTime-30.0;
    Double tol(2.e-7); // float precision
    for (uInt iobs=0;iobs<nObs;++iobs) {
      for (Int itime=0;itime<10;++itime) {
	thistime+=Double(6.0);
	//    cout << itime << " " << thistime << endl;
	ci.interpolate(iobs,0,0,thistime);
	r.reference(ci.resultF(iobs,0,0));

	Float reltime=iobs*tint;  // effective time is caltable timestamps
	Float cfval=NewCalTable::NCTtestvalueF(0,0,0,reltime,0.0,tint);
	Float diff=r(0,0,0)-cfval;

	if (verbose)
	  cout << "iobs="<< iobs << " t=" << thistime-refTime 
	       << " V=" << r(0,0,0)
	       << " (should be"
	       << " V=" << cfval
	       << " diff= " << diff
	       << endl;

	AlwaysAssert( nearAbs(diff,0.0f,tol), AipsError);
      }
    }
  }

}

void doTest5 (Bool verbose=False) {

  cout << "****----doTest5()----**** (performance)" << endl;

  Timer timer;

  timer.mark();
  
  // Make a testing NewCalTable (Table::Memory)
  Int nhour=6;
  uInt nFld(1), nAnt(62), nSpw(8), nObs(1),nScan(nhour*10),nTime(6);
  Vector<Int> nChan(nSpw,1);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(verbose);
  NewCalTable tnct("tCTPatchedInterp_test5.ct","Complex",
		   nObs,nScan,nTime,
		   nAnt,nSpw,nChan,
		   nFld,
		   refTime,tint,disk,False); 
		
  if (verbose)
    cout << "Made NewCalTable in " << timer.real() << endl;

  // some sanity checks on the test NewCalTable
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << " (should be " << Table::Memory << ")"
		    << endl;
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << " (should be " << nObs*nScan*nTime*nSpw*nAnt << ")"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nObs*nScan*nTime*nSpw*nAnt), AipsError);
  
  timer.mark();

  // Make a CTPatchedInterp  (no freq interp yet)
  CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linear","none","");

  Int N=60*60*nhour;
  Cube<Complex> r;
  cout.precision(10);

  Double thistime=refTime;
  for (uInt ispw=0;ispw<nSpw;++ispw) {
    for (Int itime=0;itime<N;++itime) {
      thistime=refTime+Double(itime);
      ci.interpolate(0,0,ispw,thistime);
      r.reference(ci.resultC(0,0,ispw));
    }
    if (verbose)
      cout << ispw << " time =" << timer.real() << " (dt="<<thistime-refTime<<")"<< endl;
  }
  Double t=timer.real();
  Int nsamp(N*nSpw*nAnt);

  cout << "Interpolated " << nsamp << " samples in " << t << " ("<< Double(nsamp)/t<<"/s)" << endl;

}


int main ()
{
  try {

    doTest1(CTPATCHEDINTERPTEST_VERBOSE);
    doTest2(CTPATCHEDINTERPTEST_VERBOSE);
    doTest3(CTPATCHEDINTERPTEST_VERBOSE);
    doTest4(CTPATCHEDINTERPTEST_VERBOSE);
    //doTest5(CTPATCHEDINTERPTEST_VERBOSE);

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
