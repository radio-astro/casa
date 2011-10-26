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

#include <calibration/CalTables/NewCalTable.h>
#include <calibration/CalTables/NewCalTableDesc.h>
#include <calibration/CalTables/NewCalMainRecord.h>
#include <calibration/CalTables/NewCalMainColumns.h>
#include <calibration/CalTables/NewCalTabIter.h>
//#include <calibration/CalTables/NewCalColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ScaColDesc.h>
#include <derivedmscal/DerivedMC/DerivedMSCal.h>
#include <casa/Arrays/Cube.h>
#include <casa/Containers/RecordField.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/OS/Timer.h>
#include <casa/namespace.h>
// <summary>
// Test program for NewCalTable class.
// </summary>

Bool foundError = False;

void doTest1 () {

  {

  // Make empty table
  cout << "making new, empty in-Memory NCT" << endl;
  NewCalTableDesc caltabdesc;
  NewCalTable nct("testNCT.tab",caltabdesc,Table::Scratch,Table::Memory);

  Int nPar(2);

  /*   
  // Shapes
  Int nAnt=8;
  Int nFld=3;
  Int nSpw=2;
  Vector<Int> nChan(nSpw,0);
  for (Int i=0;i<nSpw;++i) nChan(i)=((i%2==0) ? 128 : 1024);

  // Invent time-series
  Int nTimes(5);
  Vector<Double> times(nTimes);
  Double time0(100.0);
  Double dtime(10.0);
  */

  // Adopt meta info from an MS
  cout << "setMetaInfo(): attach meta data from a parent MS"<<endl;
  String casapath = getenv("CASAPATH");
  String datapath = casapath.substr(0,casapath.find(" "));
  String msdata = "/data/regression/unittest/clean/point_spw1.ms";
  cout << "casapath =" << casapath << endl;
  cout << "datapath =" << datapath << endl;
  cout << "msdata   =" << msdata << endl;


  nct.setMetaInfo(datapath+msdata);
  // Remember some shapes
  Int nAnt=nct.antenna().nrow();
  Int nFld=nct.field().nrow();
  Int nSpw=nct.spectralWindow().nrow();
  cout << "nAnt=" << nAnt << endl ;
  cout << "nFld=" << nFld << endl ;
  cout << "nSpw=" << nSpw << endl ;



  ROMSSpWindowColumns spwcol(nct.spectralWindow());
  Vector<Int> nChan=spwcol.numChan().getColumn();
  //  cout << "nChan = " << nChan << endl;


  cout << "Generating 'interesting' contents for the table" << endl;
  Int nTimes(5);  
  Double time0(4734187549.8204317);  // from the MS
  Double dtime(1000.0);

  Vector<Int> antlist(nAnt);
  indgen(antlist);
  Int refant(nAnt/2);  // some antenna in the middle

  // Columns (replace with NCTMainCols, TBD)
  NewCalMainColumns ncmc(nct);

  Double thistime(time0);
  Int thisscan(0);
  Int thisfield(0);
  Int nAddRows(0);
  for (Int itime=0;itime<nTimes;++itime) {
    if (itime%10==0) cout << "itime =" << itime << endl;
    if (itime>0) thistime+=dtime;
    thisscan=itime/100 + 1;
    thisfield=thisscan%nFld;
    for (Int thisspw=0;thisspw<nSpw;++thisspw) {
      // add rows
      nAddRows=nAnt;
      RefRows rows(nct.nrow(),nct.nrow()+nAddRows-1,1); 
      nct.addRow(nAddRows);

      // fill columns in new rows
      ncmc.time().putColumnCells(rows,Vector<Double>(nAddRows,thistime));
      ncmc.fieldId().putColumnCells(rows,Vector<Int>(nAddRows,thisfield));
      ncmc.spwId().putColumnCells(rows,Vector<Int>(nAddRows,thisspw));
      ncmc.antenna1().putColumnCells(rows,antlist);
      ncmc.antenna2().putColumnCells(rows,Vector<Int>(nAddRows,refant));
      ncmc.scanNo().putColumnCells(rows,Vector<Int>(nAddRows,thisscan));

      Cube<Float> par(nPar,nChan(thisspw),nAddRows);
      par=1.0;
      ncmc.param().putColumnCells(rows,par);
      ncmc.paramerr().putColumnCells(rows,par);

    }
  }
  
  cout << "total Rows: " << nct.nrow() << endl;

  // Write it out
  String diskname("testNCTdisk.tab");
  cout << "Writing out to disk: "+diskname << endl;
  nct.writeToDisk(diskname);

  }

  {

    Timer timer;

    // Make memoryTable from diskfile
    timer.mark();
    cout << "reading disk table into memory table" << endl;
    NewCalTable nct("testNCTdisk.tab",Table::Old,Table::Memory);
    timer.show();

    /* TBD: virtual columns for geometry calculations
       // probably this will be added to NewCalTable itself
    TableDesc td;
    td.addColumn(ScalarColumnDesc<double>("PA1"));
    DerivedMSCal dM;
    nct.addColumn(td,dM);
    */

    Block<String> sortcol(2);
    sortcol[0]="SPECTRAL_WINDOW_ID";
    sortcol[1]="ANTENNA1";
      
    NewCalTabIter nctiter(nct,sortcol);
    Int iter=0;
    while (!nctiter.pastEnd()) {
      Table tab(nctiter.table());
      //  TBD: from virtual column...
      //      ROScalarColumn<double> pa1(tab,"PA1");
      cout << iter << ": nrow=" << nctiter.nrow(); // << flush;
      cout << " spw="<<nctiter.spw0();
      cout << " ant1="<<nctiter.antenna1();
      //  TBD: from virtual column...
      //      cout << " pa1="<<flush;
      //      cout << pa1.getColumn()*180.0/C::pi;
      cout << " row #s: " << tab.rowNumbers(); 
      cout << " " << tab.rowNumbers(nct); 
      cout << endl;
      nctiter.next();
      ++iter;
    }

  }

}



int main ()
{
  try {

    cout<<"\n doTest1..."<<endl;
    doTest1();

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
