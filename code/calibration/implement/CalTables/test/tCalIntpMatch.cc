//# tCalIntpMatch.cc: Test program for the CalIntpMatch class
//# Copyright (C) 2001,2002,2003
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

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Utilities/Assert.h>
#include <calibration/CalTables/CalIntpMatch.h>
#include <msvis/MSVis/MSCalEnums.h>
#include <calibration/CalTables/CalMainBuffer.h>
#include <msvis/MSVis/VisBuffer.h>

#include <casa/namespace.h>
// <summary>
// Test program for class CalIntpMatch.
// </summary>

Bool foundError = False;


void doTest1(const Int& nAnt=10)
{
  // Do test #1: check antenna and spectral window id. match
  // Input:
  //    nAnt        Int       Number of antennas
  // Output to global data:
  //    foundError  Bool      True if any test fails
  //

  // Construct and fill a visibility buffer (detached from a 
  // visibility iterator)
  VisBuffer vb;
  Int nBasl = nAnt * (nAnt + 1) / 2;
  vb.antenna1().resize(nBasl);
  vb.antenna2().resize(nBasl);
  vb.spectralWindow() = 3;
  vb.nRow() = nBasl;

  Int row = 0;
  for (Int i=0; i < nAnt; i++) {
    for (Int j=i; j < nAnt; j++) {
      vb.antenna1()(row) = i;
      vb.antenna2()(row) = j;
      row++;
    };
  };

  // Construct and fill an associated calibration buffer
  Vector<Int> antIndex(1, MSC::ANTENNA1);
  Vector<Int> antValues(10);
  indgen(antValues);
  Block<Vector<Int> > indexValues(1);
  indexValues[0] = antValues;

  CalMainBuffer cb(antIndex, indexValues);
  cb.calDescId() = 0;
  CalDescBuffer& cdb = cb.calDescBuffer();
  cdb.numSpw().resize(1);
  cdb.numSpw()(0) = 1;
  cdb.spwId().resize(IPosition(2,1,1));
  cdb.spwId() = 3;

  // Construct a calibration interpolation match
  CalIntpMatch calMatch(CalIntpMatch::MATCHING_ANT_SPW, 2);
  calMatch.setVisBuffer(vb);
  calMatch.setCalBuffer(cb);

  // Match (and verify) each visibility buffer row
  row = 0;
  for (row=0; row < nBasl; row++) {
    for (Int ipos=0; ipos < 2; ipos++) {
      Int index = calMatch.matchIndex(row, ipos);
      LogicalArray calRowMask;
      Vector<Int> matchingRows = calMatch.calRows(index, calRowMask);
      Int expectVal = (ipos == 0) ? vb.antenna1()(row) : vb.antenna2()(row);
      if ((matchingRows.nelements() != 1) || (matchingRows(0) != expectVal)) {
	foundError = True;
      };
    };
  };
}


int main()
{
  try {
    // Test antenna and spectral window id. match for an
    // array of ten antennas
    doTest1(10);
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
}
