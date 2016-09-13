//# tAsdmStMan.cc: Test program for the AsdmStMan class
//# Copyright (C) 2012
//# Associated Universities, Inc. Washington DC, USA.
//# (c) European Southern Observatory, 2012
//# Copyright by ESO (in the framework of the ALMA collaboration)
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have receied a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: tAsdmStMan.cc 20942 2012-05-15 13:43:52Z diepen $

#include <asdmstman/AsdmStMan.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/DataMan/StandardStMan.h>
#include <tables/DataMan/DataManAccessor.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/OS/RegularFile.h>
#include <casa/IO/RegularFileIO.h>
#include <casa/IO/AipsIO.h>
#include <casa/Containers/BlockIO.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

using namespace casa;

// This program tests the class AsdmStMan and related classes.
// The results are written to stdout. The script executing this program,
// compares the results with the reference output file.


// Write the index file in the same way as read by AsdmStMan.
// The file name should asl obe that expected by AsdmStMan.
void writeIndex (const String& fname,
                 bool asBigEndian,
                 const Block<String>& bdfNames,
                 const vector<AsdmIndex>& index)
{
  AipsIO aio(fname + "asdmindex", ByteIO::New);
  aio.putstart ("AsdmStMan", 1);
  // Write the index info.
  aio << asBigEndian << bdfNames;
  aio.put (index);
  aio.putend();
}

// Create the table and fill the columns TIME, DATA_DESC_ID, ANTENNA1,
// ANTENNA2, and DATA.
// The DATA column is not really filled. Instead the data are written into
// the BDFs as expected by AsdmStMan. While writing, the index is built up.
void createTable (uInt ntime, uInt nant,
                  uInt crossnspw, uInt crossnchan, uInt crossnpol,
                  uInt autonspw, uInt autonchan, uInt autonpol)
{
  // Build the table description.
  TableDesc td;
  td.comment() = "A test of class Table";
  td.addColumn (ScalarColumnDesc<Double>("TIME"));
  td.addColumn (ScalarColumnDesc<Int>("DATA_DESC_ID"));
  td.addColumn (ScalarColumnDesc<Int>("ANTENNA1"));
  td.addColumn (ScalarColumnDesc<Int>("ANTENNA2"));
  td.addColumn (ArrayColumnDesc<Complex>("DATA"));
  td.addColumn (ArrayColumnDesc<Bool>("FLAG"));
  td.addColumn (ArrayColumnDesc<Float>("WEIGHT"));
  td.addColumn (ArrayColumnDesc<Float>("SIGMA"));
  // Now create a new table from the description.
  SetupNewTable newtab("tAsdmStMan_tmp.data", td, Table::New);
  // Create the storage managers; bind DATA to AsdmStMan.
  StandardStMan sm1;
  newtab.bindAll (sm1);
  AsdmStMan sm2;
  newtab.bindColumn ("DATA", sm2);
  newtab.bindColumn ("FLAG", sm2);
  newtab.bindColumn ("WEIGHT", sm2);
  newtab.bindColumn ("SIGMA", sm2);
  // Finally create the table. The destructor closes it.
  uInt nbl = nant*(nant-1)/2;
  uInt nrow = (nbl*crossnspw + nant*autonspw) * ntime;
  Table tab(newtab, nrow);
  // DATA should not be writable. BUT in order to be compatible
  // with MSMainColumns, we let it _appear_ writable anyway. 
  // The alternative solutions would cause too much code duplication.
  AlwaysAssertExit (tab.isColumnWritable("DATA"));
  AlwaysAssertExit (tab.isColumnWritable("FLAG"));
  AlwaysAssertExit (tab.isColumnWritable("WEIGHT"));
  AlwaysAssertExit (tab.isColumnWritable("SIGMA"));

  AlwaysAssertExit (tab.isColumnWritable("ANTENNA1")); // this is really writable

  // Write data into the columns.
  ScalarColumn<Double> timeCol (tab, "TIME");
  ScalarColumn<Int> ddidCol (tab, "DATA_DESC_ID");
  ScalarColumn<Int> ant1Col (tab, "ANTENNA1");
  ScalarColumn<Int> ant2Col (tab, "ANTENNA2");
  // Create the complex cross-correlation data (scaled to shorts).
  Array<Short> crossData(IPosition(4, 2*crossnpol, crossnchan,
                                   crossnspw, nbl));  // real,imag
  indgen (crossData);
  // Create the real auto-correlation data.
  AlwaysAssert (autonpol<3, AipsError);
  Array<Float> autoData(IPosition(4, autonpol, autonchan, autonspw, nant));
  indgen (autoData);
  // Create the index object.
  vector<AsdmIndex> index;
  index.reserve (ntime);
  AsdmIndex ix;
  // Create 2 BDFs.
  Block<String> bdfNames(2);
  bdfNames[0] = tab.tableName() + "/bdf1";
  bdfNames[1] = tab.tableName() + "/bdf2";
  RegularFileIO bdf1(bdfNames[0], ByteIO::New);
  RegularFileIO bdf2(bdfNames[1], ByteIO::New);
  PtrBlock<RegularFileIO*> bdfs(2);
  bdfs[0] = &bdf1;
  bdfs[1] = &bdf2;
  Int64 fileSize[] = {0,0};
  // Now write the columns and data.
  uInt row = 0;
  for (uInt i=0; i<ntime; ++i) {
    ix.fileNr = i%2;
    ix.row = row;
    // First all cross-correlations of a time slot.
    for (uInt j=0; j<crossnspw; ++j) {
      for (uInt a1=0; a1<nant; ++a1) {
        for (uInt a2=a1+1; a2<nant; ++a2) {
          timeCol.put (row, i);
          ddidCol.put (row, j);
          ant1Col.put (row, a1);
          ant2Col.put (row, a2);
          row++;
        }
      }
    }
    // Then all auto-correlations.
    for (uInt j=0; j<autonspw; ++j) {
      for (uInt a1=0; a1<nant; ++a1) {
        timeCol.put (row, i);
        ddidCol.put (row, j);
        ant1Col.put (row, a1);
        ant2Col.put (row, a1);
        row++;
      }
    }
    // Write the data (alternately in one of the BDFs).
    uInt crossSize = crossData.size() * sizeof(Short);
    uInt autoSize  = autoData.size()  * sizeof(Float);
    bdfs[ix.fileNr]->write (crossSize, crossData.data());
    bdfs[ix.fileNr]->write (autoSize, autoData.data());
    // Set the correct values in the index entry and add it to the index vector.
    ix.dataType = 0;   // short
    ix.nBl = nbl;
    ix.nSpw = crossnspw;
    ix.nChan = crossnchan;
    ix.nPol = crossnpol;
    ix.stepBl = crossnpol*crossnchan;
    ix.stepSpw = crossnpol*crossnchan*nbl;
    ix.fileOffset = fileSize[ix.fileNr];
    ix.scaleFactors.resize(crossnspw);
    for (uInt i=0; i<crossnspw; ++i) {
      ix.scaleFactors[i] = 1;  //i+1;
    }
    AlwaysAssert (ix.dataSize() == crossSize, AipsError);
    index.push_back (ix);
    // The same for the auto data.
    ix.row += crossnspw*nbl;
    ix.dataType = 10;   // auto
    ix.nBl = nant;
    ix.nSpw = autonspw;
    ix.nChan = autonchan;
    ix.nPol = autonpol;
    ix.stepBl = autonpol*autonchan;
    ix.stepSpw = autonpol*autonchan*nant;
    ix.fileOffset = fileSize[ix.fileNr] + crossSize;
    ix.scaleFactors.resize(0);
    AlwaysAssert (ix.dataSize() == autoSize, AipsError);
    index.push_back (ix);
    fileSize[ix.fileNr] += crossSize + autoSize;
    // Use different values for the next data block.
    crossData += Short(1);
    autoData  += Float(2);
  }
  // Create the BDF index file (version 1).
  // The name of the file is table.f<i>index, where <i> is the seqnr
  // of the data manager.
  ostringstream oss;
  oss << RODataManAccessor(tab, "DATA", True).dataManagerSeqNr();
  writeIndex (tab.tableName() + "/table.f" + String(oss.str()),
              False, bdfNames, index);
}

// Read back all data and check if they are as expected.
void readTable (uInt ntime, uInt nant,
                uInt crossnspw, uInt crossnchan, uInt crossnpol,
                uInt autonspw, uInt autonchan, uInt autonpol)
{
  // Open the table and check if #rows is as expected.
  Table tab("tAsdmStMan_tmp.data");
  uInt nbl = nant*(nant-1)/2;
  uInt nrow = (nbl*crossnspw +nant*autonspw) * ntime;
  AlwaysAssertExit (nrow = tab.nrow());
  AlwaysAssertExit (tab.canAddRow());
  AlwaysAssertExit (!tab.canRemoveRow());
  AlwaysAssertExit (tab.canRemoveColumn(Vector<String>(1, "DATA")));
  // Create objects for the columns.
  ROArrayColumn<Complex> dataCol(tab, "DATA");
  ROArrayColumn<Bool> flagCol(tab, "FLAG");
  ROArrayColumn<Float> weightCol(tab, "WEIGHT");
  ROArrayColumn<Float> sigmaCol(tab, "SIGMA");
  ROScalarColumn<Double> timeCol(tab, "TIME");
  ROScalarColumn<Int> ddidCol(tab, "DATA_DESC_ID");
  ROScalarColumn<Int> ant1Col(tab, "ANTENNA1");
  ROScalarColumn<Int> ant2Col(tab, "ANTENNA2");
  // Loop through all rows in the table and check the data.
  Matrix<Complex> expCross(crossnpol, crossnchan);
  Matrix<Complex> expAuto(autonpol, autonchan);
  indgen (expCross, Complex(0,1), Complex(2,2));
  indgen (expAuto, Complex(0,0), Complex(1,0));
  Matrix<Bool> expCrossFlag(crossnpol, crossnchan, False);
  Matrix<Bool> expAutoFlag(autonpol, autonchan, False);
  Vector<Float> expCrossWS(crossnpol, 1.);
  Vector<Float> expAutoWS(autonpol, 1.);
  uInt rownr = 0;
  for (uInt i=0; i<ntime; ++i) {
    Matrix<Complex> expc (expCross.copy());
    Matrix<Complex> expa (expAuto.copy());
    for (uInt j=0; j<crossnspw; ++j) {
      for (uInt a1=0; a1<nant; ++a1) {
        for (uInt a2=a1+1; a2<nant; ++a2) {
          AlwaysAssertExit (dataCol.isDefined (rownr));
          AlwaysAssertExit (near (timeCol(rownr), Double(i)));
          AlwaysAssertExit (ddidCol(rownr) == Int(j));
          AlwaysAssertExit (ant1Col(rownr) == Int(a1));
          AlwaysAssertExit (ant2Col(rownr) == Int(a2));
          AlwaysAssertExit (allEQ (flagCol(rownr), expCrossFlag));
          AlwaysAssertExit (allEQ (weightCol(rownr), expCrossWS));
          AlwaysAssertExit (allEQ (sigmaCol(rownr), expCrossWS));
          //cout << expc;
          //cout << dataCol(rownr);
          //AlwaysAssertExit (allNear (dataCol(rownr), expc, 1e-5));
          //expc += Complex(2*crossnpol*crossnchan, 2*crossnpol*crossnchan);
          rownr++;
        }
      }
    }
    for (uInt j=0; j<autonspw; ++j) {
      for (uInt k=0; k<nant; ++k) {
        AlwaysAssertExit (dataCol.isDefined (rownr));
        AlwaysAssertExit (near (timeCol(rownr), Double(i)));
        AlwaysAssertExit (ddidCol(rownr) == Int(j));
        AlwaysAssertExit (ant1Col(rownr) == Int(k));
        AlwaysAssertExit (ant2Col(rownr) == Int(k));
        AlwaysAssertExit (allEQ (flagCol(rownr), expAutoFlag));
        AlwaysAssertExit (allEQ (weightCol(rownr), expAutoWS));
        AlwaysAssertExit (allEQ (sigmaCol(rownr), expAutoWS));
        //cout << expa;
        //cout << dataCol(rownr);
        //AlwaysAssertExit (allNear (dataCol(rownr), expa, 1e-5));
        //expa += Complex(autonpol*autonchan, 0);
        rownr++;
      }
    }
    expCross += Complex(1,1);
    expAuto  += Complex(2,0);
  }
}

int main (int argc, char* argv[])
{
  try {
    // Register AsdmStMan to be able to read it back.
    AsdmStMan::registerClass();
    // Get nrow from argv.
    uInt nrow=4;
    if (argc > 1) {
      istringstream istr(argv[1]);
      istr >> nrow;
    }
    // Do not use autoNpol>2, because it'll fail.
    createTable (2, 2, 1, 2, 2, 1, 2, 2);
    readTable   (2, 2, 1, 2, 2, 1, 2, 2);
    createTable (5, 4, 2, 8, 4, 2, 8, 1);
    readTable   (5, 4, 2, 8, 4, 2, 8, 1);
    createTable (5, 4, 6, 8, 4, 2, 1, 2);
    readTable   (5, 4, 6, 8, 4, 2, 1, 2);
  } catch (AipsError& x) {
    cout << "Caught an exception: " << x.getMesg() << endl;
    return 1;
  } 
  return 0;                           // exit with success status
}

