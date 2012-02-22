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
//# $Id$

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTDesc.h>
#include <synthesis/CalTables/CTMainRecord.h>
#include <synthesis/CalTables/CTMainColumns.h>
#include <synthesis/CalTables/CTColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableCache.h>
#include <casa/Containers/RecordField.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>
// <summary>
// Test program for NewCalTable class.
// </summary>

Bool foundError = False;

void doTest1 () 
{
  // ctors test
  // Create a NewCalTable object
  
  cout<<"********************"<<endl;
  cout<<"* Basic ctor test  *"<<endl;
  cout<<"********************"<<endl;
  CTDesc caltabdesc;
  cout << "*** Creat a new caltable on disk: testNewCal.tab ***"<<endl;
  NewCalTable caltab("testDiskCal1.tab",caltabdesc,Table::New);
  cout << "setMetaInfo(): attach meta data from a parent MS"<<endl;
  String casapath = getenv("CASAPATH");
  String datapath = casapath.substr(0,casapath.find(" "));
  String msdata = "/data/regression/unittest/clean/point_spw1.ms";
  cout<<"MS:"<<datapath<<msdata<<endl;
  caltab.setMetaInfo(datapath+msdata);
  
  cout << "\n*** Create a newcaltable from table object *** "<<endl;
  Table tab("testDiskCal1.tab", Table::Update);
  cout<< "table obj from testDiskCal1.tab: nrow="<<tab.nrow()<<endl;

  //read from table object
  cout<<"create NewCalTable from the table and add 2 rows..."<<endl;
  NewCalTable caltab2(tab);
  caltab2.addRow(2);

  cout<< "caltab2: nrow="<<caltab2.nrow()<<endl;
  cout<< "The original table nrow after="<<tab.nrow()<<endl;
  //
  cout<<"*** open from an existing caltable on disk by a file name ***"<<endl;
  cout<<"Create caltab3 from testDiskCal1.tab"<<endl;
  NewCalTable caltab3("testDiskCal1.tab");
  cout<<"add 3 rows to testcaltab3"<<endl;
  caltab3.addRow(3);
  cout<< "caltab3: nrow="<<caltab3.nrow()<<endl;

  cout<<"*** test copy constructor ***"<<endl;
  cout<<" create caltab4 using copy constructor with caltab3 and add 3 rows"<<endl; 
  NewCalTable caltab4(caltab3);
  caltab4.addRow(3); 
  cerr<<"check column access..."<<endl;
  cerr<<"calling CalAntenna"<<endl;
  CTAntenna calantab1 = caltab.antenna(); 
  cerr<<"calling MSAntennaColumns"<<endl;
  ROCTAntennaColumns antcols1(calantab1);
  ROScalarColumn<String> antnames1=antcols1.name();
  cerr<<"Anatenna name ========>"<<antnames1(0)<<endl;

  if (NewCalTable::conformant(caltab3.tableDesc())) {
    cout << "caltab3 is conformant with the new cal table!"<<endl;
  }
  else {
    cout << "Conformance error fo caltab3!"<<endl;
  }
}

void doTest2 ()
{
  //create a  memory table
  cout<<"**********************"<<endl;
  cout<<"* memory table test  *"<<endl;
  cout<<"**********************"<<endl;
  cout<<"*** creating caltable in memory from caltable on disk ***"<<endl;
  NewCalTable memcaltab("testDiskCal1.tab",Table::Update, Table::Memory);
  cout<<"Add 5 rows to the memtable "<<endl;
  memcaltab.addRow(5,True);
  cout<<"memtable:memcaltab nrow="<<memcaltab.nrow()<<endl;
  Table tab("testDiskCal1.tab", Table::Old);
  cout<<"check original caltable on disk : nrow="<<tab.nrow()<<endl;
  cout<<"Write the mem table to SavedfromMemCal.tab on disk..."<<endl;
  memcaltab.writeToDisk("SavedfromMemCal.tab"); 
 
  cout<<"*** Creating a new caltable(memtab2) in memory ***"<<endl;
  CTDesc caltabdesc;
  NewCalTable memcaltab2("testTempCal1.tab",caltabdesc, Table::Scratch, Table::Memory);
  cout<<"Adde 3 rows"<<endl;
  memcaltab2.addRow(3);
  cout<<"memcaltab2.nrow()="<<memcaltab2.nrow()<<endl;
  cout<<"memcaltab2.rowNumbers()="<<memcaltab2.rowNumbers()<<endl;
}

void doTest3 ()
{
  //row accessor test
  // need to add more ....
  cout<<"*****************"<<endl;
  cout<<"* Accessor test *"<<endl;
  cout<<"*****************"<<endl;
  cout<<"Row accessor test : create a memory table of nrow=1 ***"<<endl;
  CTDesc caltabdesc;
  NewCalTable memcaltab("testTempCal2.tab",caltabdesc, Table::Scratch, Table::Memory);
  memcaltab.addRow(1);
  cout<<"getRowMain and modify Field_ID to set to 2"<<endl;
  Record rec=memcaltab.getRowMain(0);
  RecordFieldPtr<Int> savref (rec, "FIELD_ID");  
  *savref=2;
  Int inrow=0;
  // test CTMainRecord accessors
  CTMainRecord calrec(rec);
  Int fid;
  calrec.getFieldId(fid);
  cout<<"Field ID="<<fid<<endl;
  cout<<"PutRowMain and print out"<<endl;
  memcaltab.putRowMain(inrow,calrec);
  Record rec2 = memcaltab.getRowMain(0);
  rec2.print(cout,-1," ");
}

void doTest4 ()
{
  //subtable access
  cout<<"**************************"<<endl;
  cout<<"* Subtable accessor test *"<<endl;
  cout<<"**************************"<<endl;
  cout<<"*** ANTENNA table access ***"<<endl;
  CTDesc caltabdesc;
  NewCalTable caltab("testDiskCal2.tab", caltabdesc);
  caltab.addRow(1);

  //attach parent MS info
  String casapath = getenv("CASAPATH");
  String datapath = casapath.substr(0,casapath.find(" "));
  cerr<<"setMetainfo"<<endl;
  caltab.setMetaInfo(datapath+"/data/regression/unittest/clean/point_spw1.ms");
  CTAntenna calantab = caltab.antenna(); 
  ROCTAntennaColumns antcols(calantab);
  ROScalarColumn<String> antnames=antcols.name();
  cout<<"antnames="<<antnames(0)<<endl;
   
  cout<<"Try in another copied caltable obj..."<<endl;
  NewCalTable caltab2(caltab);
  CTAntenna calantab2 (caltab.antenna());
  ROCTAntennaColumns antcols2(calantab2);
  ROScalarColumn<String> antnames2=antcols2.name();
  cout<<"antnames(from copied caltable)="<<antnames2(0)<<endl;
  
  // from table obj
  cout<<' '<<endl;
  cout <<"*** subtable access test for new caltable constructed from a table object"<<endl;
  //Table tabnew("testDiskCal.Tab");
  //NewCalTable testcaltab2(tabnew);

  // this works ....
  //CTDesc caltabdesc2;
  //NewCalTable testcaltab2("testDiskCal2.Tab", caltabdesc2);
  //testcaltab2.setMetaInfo(datapath+"/data/regression/unittest/clean/point_spw1.ms");
  //testcaltab2.addRow(2);
  // ---------------------------------------------
  // This works too...
  // create a memory table from scratch 
  //CTDesc caltabdesc2;
  NewCalTable caltab3("testTempCal2.tab",caltabdesc, Table::Scratch, Table::Memory);
  caltab3.setMetaInfo(datapath+"/data/regression/unittest/clean/point_spw1.ms");
  caltab3.addRow(2);
  //-------------------------------------------------
  NewCalTable caltab4(caltab3);
  //
  //
  //NewCalTable testcaltab3(testcaltab2);
  cerr <<"now assign nctc to NewCalColumns"<<endl;

  //CTColumns nctc(caltab3);
  CTColumns nctc(caltab4);
  cerr <<"nctc CTColumns done"<<endl; 
  //cout<<"antnames2="<<antnames2(0)<<endl;
  cout<<"antnames2="<<nctc.antenna().name()(0)<<endl;

  //test main col access
  //CTMainColumns newcalmcals(testcopiedcaltab2);
  //ScalarColumn<Double> outtime(newcalmcals.time());
  //cerr<<"outtime(0)="<<outtime(0)<<endl;
  cerr<<"outtime(0)="<<nctc.time()(0)<<endl;
  
}

void doTest5 ()
{
  //test sorting/save to disk
  cout<<"*****************"<<endl;
  cout<<"* sort/save to disk test *"<<endl;
  cout<<"*****************"<<endl;
  CTDesc caltabdesc;
  NewCalTable memcaltab("testTempCal.tab",caltabdesc, Table::Scratch, Table::Memory);
  memcaltab.addRow(10);
  Vector<Int> fldids(10);
  for (Int j =0; j < 10; j++) {
    if (j % 2 == 0){ 
      fldids(j) = 0;
    }
    else {
      fldids(j) = 1;
    }
  } 

  for (Int i =0; i < 10; i++) {
    Record rec=memcaltab.getRowMain(0);
    RecordFieldPtr<Double> tref (rec, "TIME");
    RecordFieldPtr<Int> spwref (rec, "SPECTRAL_WINDOW_ID");
    RecordFieldPtr<Int> fldref (rec, "FIELD_ID");
    RecordFieldPtr<Int> ant1ref (rec, "ANTENNA1");
    *ant1ref = 0;
    if (i == 0 || i == 3 || i == 6 || i == 9) {
      *spwref = 2;
      if (i == 3) {
       *ant1ref = 1;
      }
      else {
       *ant1ref = 2;
      }
    }
    else if (i==1 || i == 7) {
      *spwref = 0;
    }
    else {
      *spwref = 1;
    }
    *fldref = fldids(i);
    *tref=10.0-1.0*i;
    CTMainRecord calrec(rec);
    memcaltab.putRowMain(i,calrec);
  }
  cerr<<"write to disk"<<endl;
  memcaltab.writeToDisk("sortedDisk.tab");
}

int main ()
{
  try {

    //ctors on disk
    cout<<" doTest1..."<<endl;
    doTest1();   

    //memory table
    cout<<"\n doTest2..."<<endl;
    doTest2();

    //row access 
    cout<<"\n doTest3..."<<endl;
    doTest3();

    //subtable access
    cout<<"\n doTest4..."<<endl;
    doTest4();

    //
    cout<<"\n doTest5..."<<endl;
    doTest5();

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
