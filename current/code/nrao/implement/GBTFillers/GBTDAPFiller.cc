//# GBTDAPFiller.h: the generic DAP filler
//# Copyright (C) 1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
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
//# You should have received a copy of the GNU Library General Public License
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
//# $Id$

//# Includes

#include <nrao/GBTFillers/GBTDAPFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <measures/Measures/MEpoch.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/ScaColDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Path.h>
#include <casa/BasicSL/Constants.h>

#include <nrao/FITS/GBTDAPFile.h>

GBTDAPFiller::GBTDAPFiller(const String &device, Table &parent)
    : device_p(device), tab_p(0), daprow_p(1), ndap_p(0)
{
    device_p.upcase();
    String tabName = String("NRAO_GBT_DAP_") + device_p;
    // does this already exist as a subtable of parent?
    Int whichField = parent.keywordSet().fieldNumber(tabName);
    if (whichField >= 0 && parent.keywordSet().type(whichField) == TpTable) {
	// open it as is, get the full table name of the subtable
	// tabName = parent.rwKeywordSet().asTable(whichField).tableName();
	tabName = parent.tableName()+"/"+tabName;
	tab_p = new Table(tabName, GBTBackendFiller::tableLock(),
			  Table::Update);
	AlwaysAssert(tab_p, AipsError);
	attachColumns();
    } else {
	// we need to make a new one
	if (whichField >= 0) parent.rwKeywordSet().removeField(whichField);
	// just the SAMPLER, MANAGER, TIME, and INTERVAL columns to start off with
	TableDesc td;
	td.addColumn(ScalarColumnDesc<String>("SAMPLER"));
	td.addColumn(ScalarColumnDesc<String>("MANAGER"));
	// TIME is an MEpoch column
	td.addColumn(ScalarColumnDesc<Double>("TIME","Modified Julian Day"));
	TableMeasDesc<MEpoch> measCol(TableMeasValueDesc(td,"TIME"),
				      TableMeasRefDesc(MEpoch::DEFAULT));
	measCol.write(td);
	// and change the units to "s" from default of "d"
	TableQuantumDesc timeqd(td,"TIME",Unit("s"));
	timeqd.write(td);
	// INTERVAL is a Quantity
	td.addColumn(ScalarColumnDesc<Double>("INTERVAL","The sampling interval"));
	TableQuantumDesc intqd(td,"INTERVAL",Unit("s"));
	intqd.write(td);
	
	SetupNewTable newtab(parent.tableName()+"/"+tabName,
			    td, Table::New);
	IncrementalStMan stmanIncr;
	newtab.bindAll(stmanIncr);
	tab_p = new Table(newtab, GBTBackendFiller::tableLock());
	AlwaysAssert(tab_p, AipsError);
	parent.rwKeywordSet().defineTable(tabName, *tab_p);
	attachColumns();
    }
}

GBTDAPFiller::~GBTDAPFiller() 
{
    for (Int i=0; i<ndap_p; i++) {
	delete daprow_p[i];
	daprow_p[i] = 0;
    }
    ndap_p = 0;
    delete tab_p;
    tab_p = 0;
}

Int GBTDAPFiller::prepare(const String &fileName, 
			  const String &manager, const String &sampler,
			  const MVTime &startTime)
{
    Int result = GBTDAPFillerBase::prepare(fileName, manager, sampler, startTime);
    if (result >= ndap_p) {
	// try and make a new TableRow
	if (Int(daprow_p.nelements()) <= result) {
	    daprow_p.resize(daprow_p.nelements()*2);
	}
	// add any column here, first, get the tds
	// this assumes that the columns in a given manager/sampler
	// do not change from file to file
	TableDesc daptd = tableDesc(result);
	TableDesc tabtd = tab_p->tableDesc();
	TableDesc unhandledTD = tableDesc(result);
	// daptd is all columns, but we only want the unhandled ones in the dapRecord for this id
	RecordDesc dapRecordDesc(dapRecord(result).description());
	// now remove any existing columns and any columns not in dapRecordDesc
	Vector<String> cols = daptd.columnNames();
	Int colCount = 0;
	for (uInt i=0;i<cols.nelements();i++) {
	    if (tabtd.isColumn(cols(i)) ||
		dapRecordDesc.fieldNumber(cols(i)) < 0) {
		daptd.removeColumn(cols(i));
	    } else {
		colCount++;
	    }
	    // remove any unhandled columns from unhandledTD
	    if (dapRecordDesc.fieldNumber(cols(i)) < 0) {
		unhandledTD.removeColumn(cols(i));
	    }
	}
	if (colCount > 0) {
	    IncrementalStMan stmanIncr;
	    tab_p->addColumn(daptd, stmanIncr);
	}
	// is the keyword storing the column names for this manager+sampler up to date
	String colkwName = manager + "_" + sampler + "_COLUMNS";
	if (tab_p->rwKeywordSet().fieldNumber(colkwName) < 0) {
	    // it doesn't yet exist, initially consists of the standard columns
	    // TIME and INTERVAL and then everthing in unhandledTD (don't put SAMPLER and
	    // MANAGER there as that seems redundant)
	    uInt ncols = unhandledTD.columnNames().nelements();
	    Vector<String> kwCols(2+ncols);
	    kwCols(0) = "TIME";
	    kwCols(1) = "INTERVAL";
	    kwCols(Slice(2,ncols)) = unhandledTD.columnNames();
	    tab_p->rwKeywordSet().define(colkwName, kwCols);
	} else {
	    // it is defined, need to verify that there aren't any changes
	    Vector<String> currCols = tab_p->rwKeywordSet().asArrayString(colkwName);
	    // ignore the first two values
	    if (!allEQ(currCols(Slice(2,unhandledTD.columnNames().nelements())), unhandledTD.columnNames())) {
		// there has been a change, merge the two lists
		Vector<String> newCols(unhandledTD.columnNames());
		Vector<Bool> newColsInList(newCols.nelements(),True);
		uInt nnew = 0;
		for (uInt i=0;i<newCols.nelements();i++) {
		    if (anyEQ(currCols, newCols(i))) {
			// already known
			newColsInList(i) = False;
		    } else {
			nnew++;
		    }
		}
		uInt ncurrCols = currCols.nelements();
		Vector<String> kwCols(nnew+ncurrCols);
		kwCols(Slice(0,ncurrCols)) = currCols;
		nnew = 0;
		for (uInt i=0;i<newCols.nelements();i++) {
		    if (newColsInList(i)) {
			kwCols(ncurrCols+nnew) = newCols(i);
			nnew++;
		    }
		}
		tab_p->rwKeywordSet().define(colkwName, currCols);
	    }
	}
	// and create the appropriate table row for this DAP
	// the daprow_p is whatever is in the dapRecordDesc
	Vector<String> dapRecordCols(dapRecordDesc.nfields());
	for (uInt i=0;i<dapRecordDesc.nfields();i++) {
	    dapRecordCols(i) = dapRecordDesc.name(i);
	}
	daprow_p[result] = new TableRow(*tab_p, dapRecordCols);
	AlwaysAssert(daprow_p[result], AipsError);
	ndap_p = result + 1;

    } 
    // both of the other possibilities leave nothing left to be done here
    // if result is >= 0, then it should already be known and nothing to do
    // if result < 0, then there was a problem and we should report that.
    return result;
}

void GBTDAPFiller::fill()
{
    // add in the number of rows to fill
    if (rowsToFill() > 0) {
	// add the rows, remember where we start from
	uInt thisrow = tab_p->nrow();
	uInt endrow = thisrow + rowsToFill();
	tab_p->addRow(rowsToFill(), True);
	while (thisrow < endrow) {
	    Int id = currentId();
	    // first put the sampler, manager, and time values
	    samplerCol_p.putScalar(thisrow, currentSampler());
	    managerCol_p.putScalar(thisrow, currentManager());
	    timeCol_p.putScalar(thisrow, currentTime().second());
	    intervalCol_p.putScalar(thisrow, currentInterval());
	    // now, copy the row values
	    daprow_p[id]->put(thisrow, currentRow());
	    next();
	    thisrow++;
	}
    }
}

void GBTDAPFiller::attachColumns() 
{
    // it is assumed that tab_p exists and is locked
    samplerCol_p.attach(*tab_p, "SAMPLER");
    managerCol_p.attach(*tab_p, "MANAGER");
    timeCol_p.attach(*tab_p, "TIME");
    intervalCol_p.attach(*tab_p, "INTERVAL");
}
	
