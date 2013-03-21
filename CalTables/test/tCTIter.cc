//# tNewCalTable.cc: Test program for NewCalTable class
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
#include <synthesis/CalTables/CTMainColumns.h>
#include <synthesis/CalTables/CTIter.h>
#include <casa/Arrays/Cube.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>

// <summary>
// Test program for CTIter class.
// </summary>

// Control verbosity
#define CTITERTEST_VERBOSE True

Bool foundError = False;

void doTest1 (Bool verbose=False) {
  
  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(1), nAnt(10), nSpw(1), nObs(1), nTime(3);
  Vector<Int> nChan(nSpw,1);
  Bool disk(True);
  NewCalTable tnct("test.ct","T",nFld,nAnt,nSpw,nChan,nObs,nTime,disk,verbose);

  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() << endl;
  AlwaysAssert( (tnct.nrow()==nFld*nAnt*nSpw*nTime*nObs), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() << endl;
  
  // Set up iteration
  Block<String> sortcol(2);
  sortcol[0]="SPECTRAL_WINDOW_ID";
  sortcol[1]="ANTENNA1";
  ROCTIter nctiter(tnct,sortcol);

  // Count iterations
  //  TBD: provide this service in CalIter itself!
  uInt niter(0);
  while (!nctiter.pastEnd()) {
    niter+=1;
    nctiter.next();
  }
  AlwaysAssert( (niter==nAnt*nSpw), AipsError)
  if (verbose) cout << "niter = " << niter << endl;
  
  // Test individual iterations
  nctiter.reset();
  Vector<NewCalTable> tablist(niter);
  Int iter=0;
  while (!nctiter.pastEnd()) {
    Int expectSpw(iter/nAnt);
    Int expectAnt(iter%nAnt);    
    Int thisspw(nctiter.thisSpw());
    Int thisant(nctiter.thisAntenna1());

    NewCalTable tab(nctiter.table());
    tablist(iter)=tab;

    if (verbose) {
      cout << iter // << ": name = " << tab.tableName() << " / " << tablist(iter).tableName()
	   << " nrow=" << nctiter.nrow(); // << flush;
      cout << " spw="<<thisspw;
      cout << " ant1="<<thisant;
      //cout << " row #s: " << tab.rowNumbers(); 
      //      cout << " " << tab.rowNumbers(nct); 
      cout << endl;
    }

    AlwaysAssert( (thisspw==expectSpw) , AipsError);
    AlwaysAssert( (thisant==expectAnt) , AipsError);

    nctiter.next();
    ++iter;
  }
  

  // Test recorded reference table contents
  for (uInt i=0;i<niter;++i) {
    Int expectSpw(i/nAnt);
    Int expectAnt(i%nAnt);
    
    CTMainColumns mc(tablist(i));
    
    Vector<Int> thisspw=mc.spwId().getColumn();
    Vector<Int> thisant=mc.antenna1().getColumn();

    if (verbose) {
      cout << i << " "
	//	   << tablist(i).tableName() << " "
	   << "nrow=" << tablist(i).nrow() << " "
	   << "ant1=" << mc.antenna1()(0) << " "
	//	     << " row #s: " << tablist(i).rowNumbers(tablist(i))
	   << "paramshape= " << mc.cparam().shape(0) << " "
	   << "param= " << mc.cparam()(0) << " "
	   << endl;
    }

    // Tests
    AlwaysAssert( (tablist(i).nrow()==nFld*nTime*nObs) , AipsError);
    AlwaysAssert( allEQ(thisspw,expectSpw) , AipsError);
    AlwaysAssert( allEQ(thisant,expectAnt) , AipsError);

    // TBD: Add param value tests here...

  }
}
void doTest2 (Bool verbose=False) {
  
  // Test (the writable) CTIter

  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(1), nAnt(10), nSpw(1), nObs(2), nTime(3);
  Vector<Int> nChan(nSpw,1);
  Double rtime(0.0), tint(60.0);  // trigger default times
  Bool disk(True);
  NewCalTable tnct("test.ct","T",nFld,nAnt,nSpw,nChan,
		   nObs,nTime,rtime,tint,disk,verbose);


  cout << "OBS_ID col ok? " << boolalpha << tnct.tableDesc().isColumn(NCT::fieldName(NCT::OBSERVATION_ID)) << endl;

  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() << endl;
  AlwaysAssert( (tnct.nrow()==nFld*nAnt*nSpw*nTime*nObs), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() << endl;
  
  // Set up iteration
  Block<String> sortcol(2);
  sortcol[0]="SPECTRAL_WINDOW_ID";
  sortcol[1]="ANTENNA1";
  CTIter nctiter(tnct,sortcol);

  // Count iterations
  //  TBD: provide this service in CalIter itself!
  uInt niter(0);
  while (!nctiter.pastEnd()) {
    niter+=1;
    nctiter.next();
  }
  AlwaysAssert( (niter==nAnt*nSpw), AipsError)
  if (verbose) cout << "niter = " << niter << endl;
  
  // Test individual iterations
  nctiter.reset();
  Vector<NewCalTable> tablist(niter);
  Int iter=0;
  while (!nctiter.pastEnd()) {
    Int expectSpw(iter/nAnt);
    Int expectAnt(iter%nAnt);    
    Int thisspw(nctiter.thisSpw());
    Int thisant(nctiter.thisAntenna1());

    Vector<Int> obsv(nctiter.obs());
    Vector<Int> scanv(nctiter.scan());

    NewCalTable tab(nctiter.table());
    tablist(iter)=tab;

    if (verbose) {
      cout << iter // << ": name = " << tab.tableName() << " / " << tablist(iter).tableName()
	   << " nrow=" << nctiter.nrow(); // << flush;
      cout << " spw="<<thisspw;
      cout << " ant1="<<thisant;
      cout << " obs="<<obsv;
      cout << " scan="<<scanv;
      //cout << " row #s: " << tab.rowNumbers(); 
      //      cout << " " << tab.rowNumbers(nct); 
      cout << endl;
    }

    AlwaysAssert( (thisspw==expectSpw) , AipsError);
    AlwaysAssert( (thisant==expectAnt) , AipsError);

    // Play with setable columns
    Cube<Bool> flag(nctiter.flag());
    //    cout << boolalpha << " flag = " << flag << endl;
    if (thisant==Int(nAnt/2))
      flag=True;
    nctiter.setflag(flag);

    nctiter.next();
    ++iter;
  }
  

  // Test recorded reference table contents
  for (uInt i=0;i<niter;++i) {
    Int expectSpw(i/nAnt);
    Int expectAnt(i%nAnt);
    
    CTMainColumns mc(tablist(i));
    
    Vector<Int> thisspw=mc.spwId().getColumn();
    Vector<Int> thisant=mc.antenna1().getColumn();

    if (verbose) {
      cout.precision(15);
      cout << i << " "
	//	   << tablist(i).tableName() << " "
	   << "nrow=" << tablist(i).nrow() << " "
	   << "ant1=" << mc.antenna1()(0) << " "
	   << "time=" << mc.time()(0) << " "
	   << "timeQ=" << mc.timeQuant()(0) << " "
	   << "timeM=" << mc.timeMeas()(0) << " "
	//	     << " row #s: " << tablist(i).rowNumbers(tablist(i))
	   << "paramshape= " << mc.cparam().shape(0) << " "
	   << "param= " << mc.cparam()(0) << " "
	   << endl;
    }


    if (thisant(0)==Int(nAnt/2)) {
      Bool f(Cube<Bool>(mc.flag().getColumn())(0,0,0));
      AlwaysAssert( (f), AipsError);
    }

    // Tests
    AlwaysAssert( (tablist(i).nrow()==nFld*nTime*nObs) , AipsError);
    AlwaysAssert( allEQ(thisspw,expectSpw) , AipsError);
    AlwaysAssert( allEQ(thisant,expectAnt) , AipsError);

    // TBD: Add param value tests here...

  }

}

int main ()
{
  try {

    //    doTest1(CTITERTEST_VERBOSE);
    doTest2(CTITERTEST_VERBOSE);

  } catch (AipsError x) {
    cout << "Unexpected exception: " << x.getMesg() << endl;
    exit(1);
  } catch (...) {
    cout << "Unexpected unknown exception" << endl;
    exit(1);
  }
  if (foundError) {
    exit(1);
  }
  cout << "OK" << endl;
  exit(0);
};
