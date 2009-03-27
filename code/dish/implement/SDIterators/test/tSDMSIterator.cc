//# tSDMSIterator.cc: This program tests the SDMSIterator class
//# Copyright (C) 1996,1997,2000,2001,2002
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

//# Includes

#include <dish/SDIterators/SDMSIterator.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/OS/File.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <casa/OS/Directory.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
int main(int argc, char **argv) 
{
    try {
	if (argc<2) {
	    cout << "Usage: " << argv[0] << " ms_name" << endl; 
	    return 1;
	}
	String msName(argv[1]);
	cout << "MS Name = " << msName << endl;

	// start up an iterator
	Timer timer;
	timer.mark();
	Record empty;
	SDMSIterator iter(msName,empty);
	timer.show("Construction : ");

	cout << "iter size : " << iter.nrecords() << endl;

	// selection
	timer.mark();
	Record sel;
	sel.defineRecord("header",Record());
	Vector<Double> scans(2);
	scans[0] = 5.0;
	scans[1] = 10.0;
	sel.rwSubRecord("header").define("scan_number",scans);
	SDMSIterator iter2(iter, sel);
	cout << "selected size : " << iter2.nrecords() << endl;
	timer.show("Selection : ");
	
	// get the first one and show it
	timer.mark();
	SDRecord::showRecord(cout,iter.get());
	timer.show("get and show : ");
	// advance one and show it
	timer.mark();
	iter++;
	timer.show("++ : ");
	timer.mark();
	SDRecord::showRecord(cout,iter.get());
	timer.show("get and show : ");
	// advance to end and show it
	// time  this step
    
	iter.origin();
	timer.mark();
	while (iter.more()) iter++;
	timer.show("iter to end : ");
	SDRecord::showRecord(cout,iter.get());

	// reset to origin and iterator plus get
	Record rec(iter.get().description());
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    rec = iter.get();
	    iter++;
	}
	timer.show("iter to end plus get with more");

	// reset to origin and iterator plus getData
	Record data(iter.getData().description());
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    data = iter.getData();
	    iter++;
	}
	timer.show("iter to end plus geData with more");

	// reset to origin and iterator plus getHeader
	Record header(iter.getHeader().description());
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    header = iter.getHeader();
	    cout << header.asArrayFloat("tcal") << endl;
	    iter++;
	}
	timer.show("iter to end plus getHeader with more");

	// reset to origin and iterator plus getOther
	Record other(iter.getOther().description());
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    other = iter.getOther();
	    iter++;
	}
	timer.show("iter to end plus getOther with more");

	// reset to origin and iterator plus getHist
	Array<String> hist;
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    hist.resize(IPosition());
	    hist = iter.getHist();
	    iter++;
	}
	timer.show("iter to end plus getHist with more");

	cout << "done" << endl;

    
    } catch (AipsError x) {
	cerr << "Exception : " << x.getMesg() << endl;
    } 
 
    return 0;
}
