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

// <summary>
// Test program for CTInterp class.
// </summary>

// Control verbosity
#define CTPATCHEDINTERPTEST_VERBOSE True

void doTest1 (Bool verbose=False) {

  cout << "****----doTest1()----****" << endl;
  
  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(1), nAnt(3), nSpw(2), nObs(1), nTime(100);
  Vector<Int> nChan(nSpw,1);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(True);
  NewCalTable tnct("tCTPatchedInterp_test1.ct","T",nFld,nAnt,nSpw,nChan,
		   nObs,nTime,refTime,tint,disk,False); // verbose);

  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() << endl;
  AlwaysAssert( (tnct.nrow()==nFld*nAnt*nSpw*nTime), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() << endl;
  
  // Make a CTPatchedInterp  (no freq interp yet)
  CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linear","none",nSpw);
  ci.state();
  
  cout << "new = " << ci.interpolate(0,0,4832568310.0) << endl;
  cout << "resultF = " << ci.resultF(0,0) << endl;
  cout << "new = " << ci.interpolate(0,1,4832568310.0) << endl;
  cout << "resultF = " << ci.resultF(0,1) << endl;


  /*


  // Call time interpolation
  sci.interpolate(4832568240.0);
  if (verbose)
    cout << "result = " << result << endl;
  sci.interpolate(4832568245.0);
  if (verbose)
    cout << "result = " << result << endl;
  sci.interpolate(4832568271.0);
  if (verbose)
    cout << "result = " << result << endl;
  sci.interpolate(4832568271.0);
  if (verbose)
    cout << "result = " << result << endl;
  
  sci.setInterpType("linear");
  sci.state();

  sci.interpolate(4832568271.0);
  if (verbose)
    cout << "result = " << result << endl;
  sci.interpolate(4832568275.0);
  if (verbose)
    cout << "result = " << result << endl;

  sci.interpolate(4832568310.0);
  if (verbose)
    cout << "result = " << result << endl;

  //  sci.setInterpType("aipslin");
  */

  cout << "****----end of doTest1()----****" << endl;

}

void doTest2 (Bool verbose=False) {
  

  cout << "****----doTest2()----****" << endl;

  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(1), nAnt(1), nSpw(1), nObs(1), nTime(2);
  Vector<Int> nChan(nSpw,10);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);
  Bool disk(True);
  NewCalTable tnct("tCTPatchedInterp_test2.ct","T",nFld,nAnt,nSpw,nChan,
		   nObs,nTime,refTime,tint,disk,False); // verbose);

  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() << endl;
  AlwaysAssert( (tnct.nrow()==nFld*nAnt*nSpw*nTime), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() << endl;
  
  // Make a CTPatchedInterp 
  CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linear","linear",nSpw);
  ci.state();

  /*  
  Double itime=4832568060.0;
  cout << "new = " << ci.interpolate(0,0,itime) << endl;
  cout << "resultF = " << ci.resultF(0,0) << endl;
  cout << "resultC = " << ci.resultC(0,0) << endl;
  cout << "ch5 = "<< NewCalTable::NCTtestvalueC(0,0,5,itime,refTime,tint) << endl;
  */

  Vector<Double> f(40); indgen(f);
  f-=3.1;  
  f*=0.5e6;  f+=60.e9; f/=1e9;  // in GHz
  cout.precision(7);
  cout << "f=" << f << endl;
  cout << "new = " << ci.interpolate(0,0,4832568311.0,f) << endl;
  cout << "resultF = " << ci.resultF(0,0) << endl;
  //  cout << "tresultF = " << ci.tresultF(0,0) << endl;

  cout << "****----end of doTest2()----****" << endl;


}

void doTest3 (Bool verbose=False) {

  cout << "****----doTest3()----****" << endl;

  Timer timer;

  timer.mark();
  
  // Make a testing NewCalTable (Table::Memory)
  Int nhour=6;
  uInt nFld(1), nAnt(27), nSpw(16), nObs(1),nTime(61*nhour);
  Vector<Int> nChan(nSpw,128);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(True);
  NewCalTable tnct("tCTPatchedInterp_test3.ct","T",nFld,nAnt,nSpw,nChan,
		   nObs,nTime,refTime,tint,disk,False); // verbose);
		   
  cout << "Made NewCalTable in " << timer.real() << endl;

  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() << endl;
  AlwaysAssert( (tnct.nrow()==nFld*nAnt*nSpw*nTime), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() << endl;
  
  timer.mark();

  // Make a CTPatchedInterp  (no freq interp yet)
  CTPatchedInterp ci(tnct,VisCalEnum::JONES,1,"linear","none",nSpw);
  //  ci.state();

  Int N=60*60*nhour;
  Cube<Complex> r;
  cout.precision(10);

  Double thistime=refTime;
  for (uInt ispw=0;ispw<nSpw;++ispw) {
    for (Int itime=0;itime<N;++itime) {
      thistime=refTime+Double(itime);
      //    cout << itime << " " << thistime << endl;
      ci.interpolate(0,ispw,thistime);
      r.reference(ci.resultC(0,ispw));
    }
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
    //    doTest3(CTPATCHEDINTERPTEST_VERBOSE);

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
