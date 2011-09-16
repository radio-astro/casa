//# tSDTableIterator.cc: This program tests the SDTableIterator class
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

#include <dish/SDIterators/SDTableIterator.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/File.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <casa/OS/Directory.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordDesc.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
int main(int argc, char **argv) 
{
    try {
	if (argc<2) {
	    cout << "Usage: " << argv[0] << " table_name" << endl; 
	    return 1;
	}
	String tableName(argv[1]);
	cout << "Table Name = " << tableName << endl;

	// start up an iterator
	Timer timer;
	timer.mark();
	Record empty;
	SDTableIterator iter(tableName,empty);
	timer.show("Construction : ");

	// get_vectors
	Record vecs;
	vecs.defineRecord("header", Record());
	vecs.rwSubRecord("header").define("ut",0.0);
	vecs.rwSubRecord("header").define("date","");
	vecs.rwSubRecord("header").define("scan_number",0);
	vecs = iter.get_vectors(vecs);
	cout << "UT : " << vecs.subRecord("header").asArrayDouble("ut") << endl;
	cout << "DATE : " << vecs.subRecord("header").asArrayString("date") << endl;
	cout << "SCAN_NUMBER : " << vecs.subRecord("header").asArrayInt("scan_number") << endl;

	// selection
	timer.mark();
	Record sel;
	sel.defineRecord("header",Record());
	sel.rwSubRecord("header").define("veldef","RADIO");
	SDTableIterator iter2(iter, sel);
	timer.show("Selection : ");

	// and a different one, involving UT
	timer.mark();
	Record sel2;
	sel2.defineRecord("header",Record());
	Matrix<Double> uts(2,1);
	uts(0,0) = 335.0;
	uts(1,0) = 340.0;
	sel2.rwSubRecord("header").define("ut",uts);
	SDTableIterator iter3(iter, sel2);
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
	Record rec;
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    rec = iter.get();
	    iter++;
	}
	timer.show("iter to end plus get with more");

	// reset to origin and iterator plus getData
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    iter.getData();
	    iter++;
	}
	timer.show("iter to end plus geData with more");

	// reset to origin and iterator plus getHeader
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    iter.getHeader();
	    iter++;
	}
	timer.show("iter to end plus getHeader with more");

	// reset to origin and iterator plus getOther
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    iter.getOther();
	    iter++;
	}
	timer.show("iter to end plus getOther with more");

	// reset to origin and iterator plus getHist
	iter.origin();
	timer.mark();
	while (iter.more()) {
	    iter.getHist();
	    iter++;
	}
	timer.show("iter to end plus getHist with more");


	String dummyName = "tSDTableIterator.dummy";
	cout << "Making a new sditerator from scratch - named " << dummyName << endl;
	File tmp(dummyName);
	if (tmp.exists()) {
	    cout << "Existing " << dummyName << " file is being removed" << endl;
	    if (tmp.isSymLink()) {
		SymLink stmp(tmp);
		stmp.remove();
	    } else if (tmp.isRegular()) {
		RegularFile rtmp(tmp);
		rtmp.remove();
	    } else if (tmp.isDirectory()) {
		Directory dtmp(tmp);
		dtmp.removeRecursive();
	    } else {
		cerr << "Existing " << dummyName << " file can not be removed" << endl;
		exit(1);
	    }
	}
	cout << "creating a new iterator from scratch" << endl;
	SDTableIterator newsdit(dummyName, empty, Table::New);
	cout << "created" << endl;
	SDRecord sdrec = iter.get();
	RecordDesc tother;
	tother.addField("TCAL_VECTOR",TpArrayFloat);
	sdrec.rwSubRecord("other").defineRecord("NRAO_GBT_CAL",Record(tother));
	Matrix<Float> tcal(2,512);
	indgen(tcal);
	sdrec.rwSubRecord("other").rwSubRecord("NRAO_GBT_CAL").define("TCAL_VECTOR",tcal);
	if (!newsdit.appendRec(sdrec)) {
	    cout << "appendRec return False" << endl;
	    exit(1);
	} 
	cout << "done" << endl;

    
    } catch (AipsError x) {
	cerr << "Exception : " << x.getMesg() << endl;
    } 
 
    return 0;
}
