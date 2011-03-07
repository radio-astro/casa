//# tSDRecord.cc: This program tests the SDRecord class
//# Copyright (C) 1996,1997,1999,2000,2002
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

#include <dish/SDIterators/SDRecord.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
int main() 
{
    try {
	// create an empty SDRecord
	SDRecord trec;
	// display it
	cout << trec;

	// is it Valid?
	cout << "SDRecord::isValid(trec) : " << SDRecord::isValid(trec) << endl;
	cout << "trec.isValid() : " << trec.isValid() << endl;
	cout << "trec.arrType() : " << trec.arrType() << endl;
	
	// the parts
	SDRecord::showRecord(cout, trec.data());
	SDRecord::showRecord(cout, trec.desc());
	SDRecord::showRecord(cout, trec.header());
	SDRecord::showRecord(cout, trec.other());
	cout << "trec.hist() : " << trec.hist() << endl;
	
	cout << "trec.isFixed() : " << trec.isFixed() << endl;
	cout << "trec.data().isFixed() : " << trec.data().isFixed() << endl;
	cout << "trec.desc().isFixed() : " << trec.desc().isFixed() << endl;
	cout << "trec.header().isFixed() : " << trec.header().isFixed() << endl;
	cout << "trec.other().isFixed() : " << trec.other().isFixed() << endl;

	// show the RW fields
	cout << "tsys : " << *(trec.tsys()) << endl;
	cout << "duration : " << *(trec.duration()) << endl;
	cout << "exposure : " << *(trec.exposure()) << endl;
	cout << "restfrequency : " << *(trec.restfrequency()) << endl;
	cout << "veldef : " << *(trec.veldef()) << endl;
	cout << "refframe : " << *(trec.refframe()) << endl;
	cout << "chan_freq : " << endl;
	SDRecord::showRecord(cout, *(trec.chan_freq()));
	cout << "reffrequency : " << *(trec.reffrequency()) << endl;
	cout << "flag : " << *(trec.flag()) << endl;
	cout << "weight : " << *(trec.weight()) << endl;
	cout << "sigma : " << *(trec.sigma()) << endl;
	cout << "arr : " << *(trec.farr()) << endl;

	// set them and show their RO counterparts
	cout << "duration set to 1.0" << endl;
	*(trec.duration()) = 1.0;
	cout << "exposure set to 2.0" << endl;
	*(trec.exposure()) = 2.0;
	cout << "restfrequency set to 3.0" << endl;
	*(trec.restfrequency()) = 3.0;
	cout << "veldef set to \"stuff\"" << endl;
	*(trec.veldef()) = "stuff";
	cout << "refframe set to \"otherstuff\"" << endl;
	*(trec.refframe()) = "otherstuff";
	cout << "reffrequency set to 4.0" << endl;
	*(trec.reffrequency()) = 4.0;

	cout << "tsys : " << *(trec.rotsys()) << endl;
	cout << "duration : " << *(trec.roduration()) << endl;
	cout << "exposure : " << *(trec.roexposure()) << endl;
	cout << "restfrequency : " << *(trec.rorestfrequency()) << endl;
	cout << "veldef : " << *(trec.roveldef()) << endl;
	cout << "refframe : " << *(trec.rorefframe()) << endl;
	cout << "chan_freq : " << endl;
	SDRecord::showRecord(cout, *(trec.rochan_freq()));
	cout << "reffrequency : " << *(trec.roreffrequency()) << endl;
	cout << "flag : " << *(trec.roflag()) << endl;
	cout << "weight : " << *(trec.roweight()) << endl;
	cout << "sigma : " << *(trec.rosigma()) << endl;
	cout << "arr : " << *(trec.rofarr()) << endl;

	// Make a Record from trec
	Record rec(trec.description(), RecordInterface::Variable);
	// remove a field
	rec.removeField("header");
	SDRecord::showRecord(cout, rec);
	cout << "SDRecord::isValid(rec) : " << SDRecord::isValid(rec) << endl;

	// can we construct a SDRecord from this invalid record
	SDRecord nrec(rec);
	cout << "nrec.isValid() : " << nrec.isValid() << endl;

	// a Complex SDRecord
	cout << "Compelx SDRecord" << endl;
	SDRecord crec(TpComplex);
	// a few simple tests
	cout << "SDRecord::isValid(crec) : " << SDRecord::isValid(crec) << endl;
	cout << "crec.isValid() : " << crec.isValid() << endl;
	cout << "crec.arrType() : " << crec.arrType() << endl;

	// show the RO fields
	cout << "tsys : " << *(crec.rotsys()) << endl;
	cout << "duration : " << *(crec.roduration()) << endl;
	cout << "exposure : " << *(crec.roexposure()) << endl;
	cout << "restfrequency : " << *(crec.rorestfrequency()) << endl;
	cout << "veldef : " << *(crec.roveldef()) << endl;
	cout << "refframe : " << *(crec.rorefframe()) << endl;
	cout << "chan_freq : " << endl;
	SDRecord::showRecord(cout, *(crec.rochan_freq()));
	cout << "reffrequency : " << *(crec.roreffrequency()) << endl;
	cout << "flag : " << *(crec.roflag()) << endl;
	cout << "weight : " << *(crec.roweight()) << endl;
	cout << "sigma : " << *(crec.rosigma()) << endl;
	cout << "arr : " << *(crec.rocarr()) << endl;
	
	// and show it
	cout << crec << endl;
    } catch (AipsError x) {
	cout << x.getMesg() << endl;
    }

    return 0;
}
