//# tCTCalSelection.cc: Test program for selecting on a NewCalTable
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
#include <synthesis/CalTables/CTColumns.h>
#include <synthesis/CalTables/CTInterface.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <casa/OS/Timer.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>

// <summary>
// Test program for CTInterp class.
// </summary>

// Control verbosity
#define CTSELECTION_VERBOSE True

void doTest1 (Bool verbose=False) {

  cout << "****----doTest1()----****" << endl;
  
  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(20), nAnt(10), nSpw(4), nObs(1), nTime(10);
  Vector<Int> nChan(nSpw,32);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(False);
  NewCalTable tnct("temp.ct","T",nFld,nAnt,nSpw,nChan,
		   nObs,nTime,refTime,tint,disk,False); // verbose);

  if (verbose) 
    cout << "Writing test NewCalTable out to tCTSelection.ct" << endl;
  tnct.writeToDisk("tCTSelection.ct");


  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << "  (should be 1)"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nFld*nAnt*nSpw*nTime), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << "  (should be " << nFld*nAnt*nSpw*nTime << ")"
		    << endl;

  ROCTColumns ctc(tnct);

  // Extract some field names to select
  Vector<String> fldnames=ctc.field().name().getColumn();
  Vector<Int> fldids(3);
  fldids(0)=3; fldids(1)=10; fldids(2)=17;
  String fieldsel("");
  for (uInt i=0;i<fldids.nelements();++i) {
    if (i>0) fieldsel+=",";
    fieldsel+=fldnames(fldids(i));
  }


  // Some spws to select
  Vector<Int> spwids(2);
  spwids(0)=1; spwids(1)=3;
  String spwsel("");
  for (uInt i=0;i<spwids.nelements();++i) {
    if (i>0) spwsel+=",";
    spwsel+=String::toString(spwids(i));
  }


  // Extract some antenna names to select
  Vector<String> antnames=ctc.antenna().name().getColumn();
  Vector<Int> antids(5);
  antids(0)=2; antids(1)=3; antids(2)=6; antids(3)=8; antids(4)=9;
  String antsel("");
  for (uInt i=0;i<antids.nelements();++i) {
    if (i>0) antsel+=",";
    antsel+=antnames(antids(i));
  }

  if (verbose) {
    cout << "Selection strings:" << endl;
    cout << " fieldsel = " << fieldsel << endl;
    cout << " spwsel = " << spwsel << endl;
    cout << " antsel = " << antsel << endl;
  }

  NewCalTable selnct(tnct);
  CTInterface cti(tnct);
  MSSelection mss;
  mss.setFieldExpr(fieldsel);
  mss.setSpwExpr(spwsel);
  mss.setAntennaExpr(antsel);

  TableExprNode ten=mss.toTableExprNode(&cti);

  if (verbose)
    cout << "CalSelection index results (get*List): " << endl;


  if (verbose)
    cout << " Field list: " << mss.getFieldList() << endl; // OK?
  AlwaysAssert( allEQ(mss.getFieldList(),fldids), AipsError );


  if (verbose)
    cout << " Antenna list: " << mss.getAntenna1List() << endl;
  AlwaysAssert( allEQ(mss.getAntenna1List(),antids), AipsError );

  if (verbose) {
    cout << " Spw list: " << mss.getSpwList() << endl;
    //cout << " Chan list: " << mss.getChanList() << endl;
  }
  AlwaysAssert( allEQ(mss.getSpwList(),spwids), AipsError );

  getSelectedTable(selnct,tnct,ten,"");

  if (verbose)
    cout << "selected: nrow=" << selnct.nrow() << endl;
  AlwaysAssert( (selnct.nrow()==antids.nelements()*nTime*spwids.nelements()*fldids.nelements()), AipsError);


  ROCTMainColumns ctmc(selnct);
  Vector<Int> selfieldcol,selantcol,selspwcol;
  ctmc.fieldId().getColumn(selfieldcol);
  ctmc.spwId().getColumn(selspwcol);
  ctmc.antenna1().getColumn(selantcol);

  if (verbose) {
    cout << "ctmc.fieldId().getColumn()  = " << selfieldcol << endl;
    cout << "ctmc.spwId().getColumn()    = " << selspwcol << endl;
    cout << "ctmc.antenna1().getColumn() = " << selantcol << endl;
  }

  Vector<Bool> fldok(selfieldcol.nelements(),False);
  for (uInt i=0;i<fldids.nelements();++i) {
    fldok|=(selfieldcol==fldids(i));
  }
  Vector<Bool> spwok(selspwcol.nelements(),False);
  for (uInt i=0;i<spwids.nelements();++i) {
    spwok|=(selspwcol==spwids(i));
  }
  Vector<Bool> antok(selantcol.nelements(),False);
  for (uInt i=0;i<antids.nelements();++i) {
    antok|=(selantcol==antids(i));
  }

  Bool allfldok=allEQ(fldok,True);
  Bool allspwok=allEQ(spwok,True);
  Bool allantok=allEQ(antok,True);
  if (verbose) {
    cout << boolalpha;
    //    cout << "fldok = " << fldok << endl;
    cout << "allEQ(fldok,True) = " << allfldok  << endl;
    cout << "allEQ(spwok,True) = " << allspwok  << endl;
    cout << "allEQ(antok,True) = " << allantok  << endl;
  }
  AlwaysAssert( allEQ(fldok,True) , AipsError );

}

void doTest2 (Bool verbose=False) {

  cout << "****----doTest2()----****" << endl;
  
  // Make a testing NewCalTable (Table::Memory)
  uInt nFld(20), nAnt(10), nSpw(4), nObs(1), nTime(10);
  Vector<Int> nChan(nSpw,32);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(False);
  NewCalTable tnct("temp.ct","T",nFld,nAnt,nSpw,nChan,
		   nObs,nTime,refTime,tint,disk,False); // verbose);

  if (verbose) 
    cout << "Writing test NewCalTable out to tCTSelection.ct" << endl;
  tnct.writeToDisk("tCTSelection.ct");


  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << "  (should be 1)"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nFld*nAnt*nSpw*nTime), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << "  (should be " << nFld*nAnt*nSpw*nTime << ")"
		    << endl;

  ROCTColumns ctc(tnct);

  // Extract some field names to select
  Vector<String> fldnames=ctc.field().name().getColumn();
  Vector<Int> fldids(3);
  fldids(0)=3; fldids(1)=10; fldids(2)=17;
  String fieldsel("");
  for (uInt i=0;i<fldids.nelements();++i) {
    if (i>0) fieldsel+=",";
    fieldsel+=fldnames(fldids(i));
  }


  // Some spws to select
  Vector<Int> spwids(2);
  spwids(0)=1; spwids(1)=3;
  String spwsel("");
  for (uInt i=0;i<spwids.nelements();++i) {
    if (i>0) spwsel+=",";
    spwsel+=String::toString(spwids(i));
  }


  // Extract some antenna names to select
  Vector<String> antnames=ctc.antenna().name().getColumn();
  Vector<Int> antids(5);
  antids(0)=2; antids(1)=3; antids(2)=6; antids(3)=8; antids(4)=9;
  String antsel("");
  for (uInt i=0;i<antids.nelements();++i) {
    if (i>0) antsel+=",";
    antsel+=antnames(antids(i));
  }

  // A time selection expression:
  //  this should pick 2 timestamps
  String timesel("2012/01/06/12:02:30.0~12:04:30");
  Vector<Double> timevals(2);
  timevals(0)=150.0+refTime;
  timevals(1)=270.0+refTime;


  if (verbose) {
    cout << "Selection strings:" << endl;
    cout << " fieldsel = " << fieldsel << endl;
    cout << " spwsel = " << spwsel << endl;
    cout << " antsel = " << antsel << endl;
    cout << " timesel = " << timesel << endl;
  }

  NewCalTable selnct(tnct);
  CTInterface cti(tnct);
  MSSelection mss;
  mss.setFieldExpr(fieldsel);
  mss.setSpwExpr(spwsel);
  mss.setAntennaExpr(antsel);
  mss.setTimeExpr(timesel);

  TableExprNode ten=mss.toTableExprNode(&cti);

  if (verbose)
    cout << "CalSelection index results (get*List): " << endl;


  if (verbose)
    cout << " Field list: " << mss.getFieldList() << endl; // OK?
  AlwaysAssert( allEQ(mss.getFieldList(),fldids), AipsError );


  if (verbose)
    cout << " Antenna list: " << mss.getAntenna1List() << endl;
  AlwaysAssert( allEQ(mss.getAntenna1List(),antids), AipsError );

  if (verbose) {
    cout << " Spw list: " << mss.getSpwList() 
	 << endl;
    //cout << " Chan list: " << mss.getChanList() << endl;
  }
  AlwaysAssert( allEQ(mss.getSpwList(),spwids), AipsError );

  if (verbose) {
    cout.precision(15);
    cout << " Time list: " << mss.getTimeList() 
	 << "  (expecting: " << timevals << ")" 
         << "  (diff = " << Vector<Double>(mss.getTimeList())-timevals << ")"
	 << endl;
    AlwaysAssert( allEQ(mss.getTimeList(),timevals), AipsError );
  }


  getSelectedTable(selnct,tnct,ten,"");

  if (verbose)
    cout << "selected: nrow=" << selnct.nrow() << endl;
  AlwaysAssert( (selnct.nrow()==antids.nelements()*2*spwids.nelements()*fldids.nelements()), AipsError);
  AlwaysAssert( (selnct.nrow()==antids.nelements()*nTime*spwids.nelements()*fldids.nelements()), AipsError);


  ROCTMainColumns ctmc(selnct);
  Vector<Int> selfieldcol,selantcol,selspwcol;
  ctmc.fieldId().getColumn(selfieldcol);
  ctmc.spwId().getColumn(selspwcol);
  ctmc.antenna1().getColumn(selantcol);

  if (verbose) {
    cout << "ctmc.fieldId().getColumn()  = " << selfieldcol << endl;
    cout << "ctmc.spwId().getColumn()    = " << selspwcol << endl;
    cout << "ctmc.antenna1().getColumn() = " << selantcol << endl;
  }

  Vector<Bool> fldok(selfieldcol.nelements(),False);
  for (uInt i=0;i<fldids.nelements();++i) {
    fldok|=(selfieldcol==fldids(i));
  }
  Vector<Bool> spwok(selspwcol.nelements(),False);
  for (uInt i=0;i<spwids.nelements();++i) {
    spwok|=(selspwcol==spwids(i));
  }
  Vector<Bool> antok(selantcol.nelements(),False);
  for (uInt i=0;i<antids.nelements();++i) {
    antok|=(selantcol==antids(i));
  }

  Bool allfldok=allEQ(fldok,True);
  Bool allspwok=allEQ(spwok,True);
  Bool allantok=allEQ(antok,True);
  if (verbose) {
    cout << boolalpha;
    //    cout << "fldok = " << fldok << endl;
    cout << "allEQ(fldok,True) = " << allfldok  << endl;
    cout << "allEQ(spwok,True) = " << allspwok  << endl;
    cout << "allEQ(antok,True) = " << allantok  << endl;
  }
  AlwaysAssert( allEQ(fldok,True) , AipsError );

}


int main ()
{
  try {

    doTest1(CTSELECTION_VERBOSE);
    //doTest2(CTSELECTION_VERBOSE);

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
