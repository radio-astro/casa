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
#include <ms/MSSel/MSSelection.h>
//#include <synthesis/CalTables/CTSelection.h>
#include <ms/MSSel/MSSelectionTools.h>
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
  uInt nFld(20), nAnt(10), nSpw(4), nObs(1), nScan(20),nTime(10);
  Vector<Int> nChan(nSpw,32);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(60.0);

  Bool disk(verbose);
  NewCalTable tnct("tCTSelection1.ct","Complex",
		   nObs,nScan,nTime,
		   nAnt,nSpw,nChan,
		   nFld,
		   refTime,tint,disk,False);


  if (verbose)
    cout << "Wrote NewCalTable out to tCTSelection1.ct" << endl;

  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << "  (should be 1)"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nObs*nScan*nTime*nSpw*nAnt), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << "  (should be " << nObs*nScan*nTime*nSpw*nAnt << ")"
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
  //  CTSelection mss;
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
    {
      cout << "selected: nrow=" << selnct.nrow()<< " ";
      cout << "(should be " << antids.nelements()*nTime*spwids.nelements()*fldids.nelements() << ")" << endl;
    }
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
  uInt nFld(1), nAnt(10), nSpw(4), nObs(3), nScan=6, nTime(6);
  Vector<Int> nChan(nSpw,1);
  Double refTime(4832568000.0); // 2012 Jan 06 @ noon
  Double tint(10.0);

  Bool disk(verbose);
  NewCalTable tnct("tCTSelection2.ct","Complex",
		   nObs,nScan,nTime,
		   nAnt,nSpw,nChan,
		   nFld,
		   refTime,tint,disk,False);

  if (verbose) 
    cout << "Wrote test NewCalTable out to tCTSelection2.ct" << endl;

  // some sanity checks on the test NewCalTable
  AlwaysAssert( (tnct.tableType() == Table::Memory), AipsError);
  if (verbose) cout << "Table::Type: " << tnct.tableType() 
		    << "  (should be 1)"
		    << endl;
  AlwaysAssert( (tnct.nrow()==nObs*nScan*nTime*nSpw*nAnt), AipsError);
  if (verbose) cout << "nrow = " << tnct.nrow() 
		    << "  (should be " << nObs*nScan*nTime*nSpw*nAnt << ")"
		    << endl;

  ROCTColumns ctc(tnct);

  // And ObsID selection:
  Vector<Int> obsids(1);
  obsids(0)=1;
  String obssel=String::toString(obsids(0));


  // A scan selection:
  Vector<Int> scans(1);
  scans(0)=7;
  String scansel=String::toString(scans(0));


  // A time selection expression:
  //  this should pick 3 timestamps in obsid=1, scan=7
  String timesel("2012/01/06/12:06:15.0~12:06:45");
  Vector<Double> timebounds(2);
  timebounds(0)=375.0+refTime;
  timebounds(1)=405.0+refTime;

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
    cout << " obssel  = " << obssel << endl;
    cout << " scansel = " << scansel << endl;
    cout << " timesel = " << timesel << endl;
    cout << " spwsel  = " << spwsel << endl;
    cout << " antsel  = " << antsel << endl;
  }

  NewCalTable selnct(tnct);
  CTInterface cti(tnct);
  //  CTSelection mss;
  MSSelection mss;
  mss.setObservationExpr(obssel);
  mss.setScanExpr(scansel);
  mss.setTimeExpr(timesel);
  mss.setSpwExpr(spwsel);
  mss.setAntennaExpr(antsel);

  TableExprNode ten=mss.toTableExprNode(&cti);

  if (verbose)
    cout << "CalSelection parsing results (get*List): " << endl;

  if (verbose) {
    cout << " Obs list: " << mss.getObservationList() << endl;
  }
  AlwaysAssert( allEQ(mss.getObservationList(),obsids), AipsError );  

  if (verbose) {
    cout << " Scan list: " << mss.getScanList() << endl;
  }
  AlwaysAssert( allEQ(mss.getScanList(),scans), AipsError );

  if (verbose) {
    cout.precision(15);
    Vector<Double> parsedTimeBounds=Vector<Double>(mss.getTimeList()); 
    cout << " Time bounds: " << parsedTimeBounds
	 << " [" << parsedTimeBounds-refTime << "]" << endl
	 << "  (expecting: " << timebounds << " [" << timebounds-refTime << "] )" << endl
         << "  (diff = " << parsedTimeBounds-timebounds << ")"
	 << endl;
  }
  AlwaysAssert( allNear(Vector<Double>(mss.getTimeList()),timebounds,1.0e-9), AipsError );

  if (verbose) {
    cout << " Spw list: " << mss.getSpwList() 
	 << endl;
    //cout << " Chan list: " << mss.getChanList() << endl;
  }
  AlwaysAssert( allEQ(mss.getSpwList(),spwids), AipsError );

  if (verbose)
    cout << " Antenna list: " << mss.getAntenna1List() << endl;
  AlwaysAssert( allEQ(mss.getAntenna1List(),antids), AipsError );


  getSelectedTable(selnct,tnct,ten,"");

  if (verbose)
    cout << "CalSelection table contents results:" << endl;

  if (verbose)
    cout << "selected: nrow=" << selnct.nrow() 
	 << " (should be " << 3*antids.nelements()*spwids.nelements() << ")"
	 << endl;
  AlwaysAssert( (selnct.nrow()==3*antids.nelements()*spwids.nelements()), AipsError);


  ROCTMainColumns ctmc(selnct);
  Vector<Int> selantcol,selspwcol,selobscol,selscancol;
  Vector<Double> seltimecol;
  ctmc.obsId().getColumn(selobscol);
  ctmc.scanNo().getColumn(selscancol);
  ctmc.time().getColumn(seltimecol);
  ctmc.spwId().getColumn(selspwcol);
  ctmc.antenna1().getColumn(selantcol);

  if (verbose) {
    cout << "ctmc.obs().getColumn()      = " << selobscol << endl;
    cout << "ctmc.scanNo().getColumn()   = " << selscancol << endl;
    cout << "ctmc.time().getColumn()     = " << seltimecol << endl;
    cout << "dtime                       = " << seltimecol-refTime << endl;
    cout << "ctmc.spwId().getColumn()    = " << selspwcol << endl;
    cout << "ctmc.antenna1().getColumn() = " << selantcol << endl;
  }

  Vector<Bool> obsok(selobscol.nelements(),False);
  for (uInt i=0;i<obsids.nelements();++i) {
    obsok|=(selobscol==obsids(i));
  }

  Vector<Bool> scanok(selscancol.nelements(),False);
  for (uInt i=0;i<scans.nelements();++i) {
    scanok|=(selscancol==scans(i));
  }

  Vector<Bool> timeok(seltimecol.nelements(),False);
  timeok|=(seltimecol>timebounds(0));
  timeok|=(seltimecol<timebounds(1));

  Vector<Bool> spwok(selspwcol.nelements(),False);
  for (uInt i=0;i<spwids.nelements();++i) {
    spwok|=(selspwcol==spwids(i));
  }
  Vector<Bool> antok(selantcol.nelements(),False);
  for (uInt i=0;i<antids.nelements();++i) {
    antok|=(selantcol==antids(i));
  }
  


  Bool allobsok=allEQ(obsok,True);
  Bool allscanok=allEQ(scanok,True);
  Bool alltimeok=allEQ(timeok,True);
  Bool allspwok=allEQ(spwok,True);
  Bool allantok=allEQ(antok,True);
  if (verbose) {
    cout << boolalpha;
    cout << "allEQ(obsok,True) = " << allobsok  << endl;
    cout << "allEQ(scanok,True) = " << allscanok  << endl;
    cout << "allEQ(timeok,True) = " << alltimeok  << endl;
    cout << "allEQ(spwok,True) = " << allspwok  << endl;
    cout << "allEQ(antok,True) = " << allantok  << endl;
  }
  AlwaysAssert( allEQ(obsok,True) , AipsError );
  AlwaysAssert( allEQ(scanok,True) , AipsError );
  AlwaysAssert( allEQ(timeok,True) , AipsError );
  AlwaysAssert( allEQ(spwok,True) , AipsError );
  AlwaysAssert( allEQ(antok,True) , AipsError );

}


int main ()
{
  try {

    doTest1(CTSELECTION_VERBOSE);
    doTest2(CTSELECTION_VERBOSE);

  } catch (AipsError x) {
    cout << "Unexpected exception: " << x.getMesg() << endl;
    exit(1);
  } catch (...) {
    cout << "Unexpected unknown exception" << endl;
    exit(1);
  }
  cout << "All OK" << endl;
  exit(0);
};
