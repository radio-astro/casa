//# tCTGenericFill.cc: Test program for NewCalTable class
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
//# $Id: tNewCalTable.cc 18164 2012-02-13 00:55:19Z george.moellenbrock $

#include <synthesis/CalTables/NewCalTable.h>
#include <casa/Arrays/Cube.h>
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
  String casapath = getenv("CASAPATH");
  String datapath = casapath.substr(0,casapath.find(" "));
  String msdata = "/data/regression/unittest/clean/point_spw1.ms";

  String msname=datapath+msdata;

  String tableName("tCTGenericFill");
  NewCalTable ct(tableName,VisCalEnum::COMPLEX,"T Jones",msname,True);

  Int nAnt(5);  // from the above ms
  Int nSpw(1);
  Int nTime(10);
  Double interval(60.0);

  Cube<Complex> cpar(1,1,nAnt);
  Cube<Bool>    flag(1,1,nAnt); flag.set(False);
  Cube<Float>   err(1,1,nAnt);  err.set(0.0);
  Cube<Float>   snr(1,1,nAnt);  snr.set(3.0);

  for (Int itime=0;itime<nTime;++itime) {
    Double time=4832568000.0+Double(itime)*interval;;
    for (Int ispw=0;ispw<nSpw;++ispw) {
      // Set the antenna-based cpar
      cpar.set(Complex(0.0));
      for (Int iant=0;iant<nAnt;++iant) {
	Float phase=Float(iant)+0.1*Float(itime);
	cpar(0,0,iant)=Complex(cos(phase),sin(phase));
      }

      // Fill into the table for this itime, ispw
      ct.fillAntBasedMainRows(nAnt,
			      time,
			      interval,
			      0,
			      ispw,
			      -1,
			      Vector<Int>(),
			      -1,
			      cpar,
			      flag,
			      err,
			      snr);
    }
  }

  // Add a history message
  ct.addHistoryMessage("tCTGenericFill","I hope this works");

  AlwaysAssert( (ct.nrow()==nSpw*nTime*nAnt), AipsError );

  // Flush to disk
  ct.writeToDisk(tableName+".disk");


}

int main ()
{
  try {

    cout<<" doTest1..."<<endl;

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
