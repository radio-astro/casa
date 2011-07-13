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

#include <calibration/CalTables/NewCalTable.h>
#include <calibration/CalTables/NewCalTableDesc.h>
#include <calibration/CalTables/NewCalMainRecord.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <tables/Tables/ArrayColumn.h>
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
  NewCalTableDesc caltabdesc;
  cout << "*** Creat a new caltable on disk: testNewCal.tab ***"<<endl;
  NewCalTable testcaltab("testNewCal.tab",caltabdesc,Table::New);
  cout << "setMetaInfo(): attach meta data from a parent MS"<<endl;
  String casapath = getenv("CASAPATH");
  String datapath = casapath.substr(0,casapath.find(" "));
  String msdata = "/data/regression/unittest/clean/point_spw1.ms";
  cout<<"MS:"<<datapath<<msdata<<endl;
  testcaltab.setMetaInfo(datapath+msdata);

  cout << "*** newcaltable from table object *** "<<endl;
  Table tab("testNewCal.tab", Table::Update);
  cout<< "table obj from testNewCal.tab: nrow="<<tab.nrow()<<endl;

  //read from table object
  cout<<"create NewCalTable from the table and add 2 rows..."<<endl;
  NewCalTable testcaltab2(tab);
  testcaltab2.addRow(2);

  cout<< "testcaltab2: nrow="<<testcaltab2.nrow()<<endl;
  cout<< "The original table nrow after="<<tab.nrow()<<endl;
  //
  cout<<"*** open from an existing caltable by a file name ***"<<endl;
  cout<<"Create testcaltab3 from testNewCal.tab"<<endl;
  NewCalTable testcaltab3("testNewCal.tab");
  cout<<"add 3 rows to testcaltab3"<<endl;
  testcaltab3.addRow(3);
  cout<< "testcaltab3: nrow="<<testcaltab3.nrow()<<endl;

  cout<<"*** test copy constructor ***"<<endl;
  cout<<" create testcaltab4 using copy constructor with testcaltab3 and add 3 rows"<<endl; 
  NewCalTable testcaltab4(testcaltab3);
  testcaltab4.addRow(3); 
  cerr<< "testcaltab4 nrow="<<testcaltab4.nrow()<<endl;
  cerr<< "testcaltab3 nrow="<<testcaltab3.nrow()<<endl;

  if (NewCalTable::conformant(testcaltab3.tableDesc())) {
    cout << "testcaltab3 is conformant with the new cal table!"<<endl;
  }
  else {
    cout << "Conformance error fo testcaltab3!"<<endl;
  }
}

void doTest2 ()
{
  //create a  memory table
  cout<<"**********************"<<endl;
  cout<<"* memory table test  *"<<endl;
  cout<<"**********************"<<endl;
  cout<<"*** creating caltable in memory from caltable on disk ***"<<endl;
  NewCalTable testmemcaltab("testNewCal.tab",Table::Update, Table::Memory);
  cout<<"Add 5 rows to the memtable "<<endl;
  testmemcaltab.addRow(5,True);
  cout<<"memtable:testmemcaltab nrow="<<testmemcaltab.nrow()<<endl;
  Table tab("testNewCal.tab", Table::Old);
  cout<<"check original caltable on disk : nrow="<<tab.nrow()<<endl;
  cout<<"Write the mem table to SavedfromMemCal.tab on disk..."<<endl;
  testmemcaltab.writeToDisk("SavedfromMemCal.tab"); 
 
  cout<<"*** Creating a new caltable(testmemtab2) in memory ***"<<endl;
  NewCalTableDesc caltabdesc;
  NewCalTable testmemcaltab2("testTempCal.tab",caltabdesc, Table::Scratch, Table::Memory);
  cout<<"Adde 3 rows"<<endl;
  testmemcaltab2.addRow(3);
  cout<<"testmemcaltab2.nrow()="<<testmemcaltab2.nrow()<<endl;
  cout<<"testmemcaltab2.rowNumbers()="<<testmemcaltab2.rowNumbers()<<endl;
}

void doTest3 ()
{
  //row accessor test
  // need to add more ....
  cout<<"*****************"<<endl;
  cout<<"* Accessor test *"<<endl;
  cout<<"*****************"<<endl;
  cout<<"Row accessor test : create a memory table of nrow=1 ***"<<endl;
  NewCalTableDesc caltabdesc;
  NewCalTable testmemcaltab("testTempCal.tab",caltabdesc, Table::Scratch, Table::Memory);
  testmemcaltab.addRow(1);
  cout<<"getRowMain and modify Field_ID to set to 2"<<endl;
  Record rec=testmemcaltab.getRowMain(0);
  RecordFieldPtr<Int> savref (rec, "FIELD_ID");  
  *savref=2;
  Int inrow=0;
  // test NewCalMainRecord accessors
  NewCalMainRecord calrec(rec);
  Int fid;
  calrec.getFieldId(fid);
  cout<<"Field ID="<<fid<<endl;
  cout<<"PutRowMain and print out"<<endl;
  testmemcaltab.putRowMain(inrow,calrec);
  Record rec2 = testmemcaltab.getRowMain(0);
  rec2.print(cout,-1," ");
}

void doTest4 ()
{
  //subtable access
  cout<<"**************************"<<endl;
  cout<<"* Subtable accessor test *"<<endl;
  cout<<"**************************"<<endl;
  cout<<"*** ANTENNA table access ***"<<endl;
  NewCalTableDesc caltabdesc;
  NewCalTable testcaltab("testDiskCal.Tab", caltabdesc);
  testcaltab.addRow(1);
  //attach parent MS info
  String casapath = getenv("CASAPATH");
  String datapath = casapath.substr(0,casapath.find(" "));
  testcaltab.setMetaInfo(datapath+"/data/regression/unittest/clean/point_spw1.ms");
  NewCalTable::CalAntenna calantab = testcaltab.antenna(); 
  ROMSAntennaColumns antcols(calantab);
  ROScalarColumn<String> antnames=antcols.name();

  cout<<"antnames="<<antnames(0)<<endl;
  
  //from disk to mem table
  //NewCalTable testcaltab2("testDiskCal.Tab", Table::Old, Table::Memory);
  //
  //copy constructor 
  //NewCalTable testcaltab2(testcaltab);
  //
  // from table obj
  cout <<"*** subtable access test for new caltable constructed from a table object"<<endl;
  Table tabnew("testDiskCal.Tab");
  NewCalTable testcaltab2(tabnew);
  NewCalTable::CalAntenna calantab2 = testcaltab2.antenna(); 
  //
  ROMSAntennaColumns antcols2(calantab2);
  ROScalarColumn<String> antnames2=antcols2.name();
  cout<<"antnames2="<<antnames2(0)<<endl;
}

void doTest5 ()
{
  //test sorting/save to disk
  cout<<"*****************"<<endl;
  cout<<"* sort/save to disk test *"<<endl;
  cout<<"*****************"<<endl;
  NewCalTableDesc caltabdesc;
  NewCalTable testmemcaltab("testTempCal.tab",caltabdesc, Table::Scratch, Table::Memory);
  testmemcaltab.addRow(10);
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
    Record rec=testmemcaltab.getRowMain(0);
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
    NewCalMainRecord calrec(rec);
    testmemcaltab.putRowMain(i,calrec);
  }
  cerr<<"write to disk"<<endl;
  testmemcaltab.writeToDisk("sortedDisk.tab");
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
