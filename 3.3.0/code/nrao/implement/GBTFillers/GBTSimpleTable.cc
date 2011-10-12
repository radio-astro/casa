//# GBTSimpleTable.h: a GBTSimpleTable is used to concatenate simple GBT tables
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

#include <nrao/GBTFillers/GBTSimpleTable.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Containers/RecordInterface.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <casa/Arrays/IPosition.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/IncrementalStMan.h>

GBTSimpleTable::GBTSimpleTable(const String &tableName, 
			       const String &indexColumn) 
    : tab_p(0), tabRow_p(0), index_p(-1)
{
    tab_p = new Table(tableName, GBTBackendFiller::tableLock(), 
    		      Table::Update);
    AlwaysAssert(tab_p, AipsError);
    updateTableRow();
    attachIndexCol(indexColumn);
    
}

GBTSimpleTable::GBTSimpleTable(Table &attachTable, 
			       const String &attachKeywordName,
			       const String &tableName,
			       const String &indexColumn)
    : tab_p(0), tabRow_p(0), index_p(-1)
{
    // make a very bare bones table

    SetupNewTable newtab(attachTable.tableName()+"/"+tableName, 
			 TableDesc(), Table::New);
    tab_p = new Table(newtab, GBTBackendFiller::tableLock());
    AlwaysAssert(tab_p, AipsError);
    attachTable.rwKeywordSet().defineTable(attachKeywordName, *tab_p);
    updateTableRow();
    attachIndexCol(indexColumn);
}

void GBTSimpleTable::add(const Table &other)
{
    if (other.nrow() == 0) return;

    ROTableRow inTabRow(other);
    checkRecord(inTabRow.get(0));
    // add the appropriate number of rows, remember where we start from
    Int startRow = tab_p->nrow();
    tab_p->addRow(other.nrow());
    for (uInt i=0; i<other.nrow(); i++) {
	tabRow_p->putMatchingFields(i+startRow, inTabRow.get(i));
    }
    setIndex(startRow);
}

void GBTSimpleTable::add(const RecordInterface &other, Bool nextIndex)
{
    checkRecord(other);
    uInt thisRow = tab_p->nrow();
    tab_p->addRow(1);
    tabRow_p->putMatchingFields(thisRow, other);
    setIndex(thisRow, nextIndex);
}

void GBTSimpleTable::checkRecord(const RecordInterface &other)
{
    Bool colsAdded = False;
    for (uInt i=0; i<other.nfields(); i++) {
	String name(other.name(i));
	if (!tab_p->tableDesc().isColumn(name)) {
	    colsAdded = True;
	    IPosition shape(other.shape(i));
	    String comment(other.comment(i));
	    switch (other.type(i)) {
	    case TpBool:
		tab_p->addColumn(ScalarColumnDesc<Bool>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpChar:				      
		tab_p->addColumn(ScalarColumnDesc<Char>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpUChar:				      
		tab_p->addColumn(ScalarColumnDesc<uChar>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpShort:				      
		tab_p->addColumn(ScalarColumnDesc<Short>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpUShort:				      
		tab_p->addColumn(ScalarColumnDesc<uShort>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpInt:					      
		tab_p->addColumn(ScalarColumnDesc<Int>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpUInt:				      
		tab_p->addColumn(ScalarColumnDesc<uInt>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpFloat:				      
		tab_p->addColumn(ScalarColumnDesc<Float>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpDouble:				      
		tab_p->addColumn(ScalarColumnDesc<Double>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpComplex:				      
		tab_p->addColumn(ScalarColumnDesc<Complex>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpDComplex:				      
		tab_p->addColumn(ScalarColumnDesc<DComplex>(name, comment), 
				 "IncrementalStMan", False);
		break;					      
	    case TpString:				      
		tab_p->addColumn(ScalarColumnDesc<String>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayBool:
		tab_p->addColumn(ArrayColumnDesc<Bool>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayChar:
		tab_p->addColumn(ArrayColumnDesc<Char>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayUChar:
		tab_p->addColumn(ArrayColumnDesc<uChar>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayShort:
		tab_p->addColumn(ArrayColumnDesc<Short>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayUShort:
		tab_p->addColumn(ArrayColumnDesc<uShort>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayInt:
		tab_p->addColumn(ArrayColumnDesc<Int>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayUInt:
		tab_p->addColumn(ArrayColumnDesc<uInt>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayFloat:
		tab_p->addColumn(ArrayColumnDesc<Float>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayDouble:
		tab_p->addColumn(ArrayColumnDesc<Double>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayComplex:
		tab_p->addColumn(ArrayColumnDesc<Complex>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayDComplex:
		tab_p->addColumn(ArrayColumnDesc<DComplex>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    case TpArrayString:
		tab_p->addColumn(ArrayColumnDesc<String>(name, comment), 
				 "IncrementalStMan", False);
		break;
	    default:
		// ignore these
		break;
	    }
	}
    }
    if (colsAdded) updateTableRow();
}


void GBTSimpleTable::updateTableRow()
{
    delete tabRow_p;
    tabRow_p = 0;
    tabRow_p = new TableRow(*tab_p);
    AlwaysAssert(tabRow_p, AipsError);
}

void GBTSimpleTable::attachIndexCol(const String &indexColumn) {
    if (indexColumn.length() > 0) {
	if (!tab_p->tableDesc().isColumn(indexColumn)) {
	    // add it in
	    tab_p->addColumn(ScalarColumnDesc<Int>(indexColumn), 
			     "IncrementalStMan", False);
	}
	// attach it
	indexCol_p.attach(*tab_p, indexColumn);
	// get the most recently stored value, if any
	if (indexCol_p.nrow() > 0) 
	    index_p = indexCol_p(indexCol_p.nrow()-1);
    }
}

void GBTSimpleTable::setIndex(Int rownr, Bool nextIndex)
{
    if (!indexCol_p.isNull()) {
	if (nextIndex) index_p++;
	indexCol_p.put(rownr, index_p);
    }
}

GBTSimpleTable::~GBTSimpleTable() {
    delete tabRow_p;
    tabRow_p = 0;
    delete tab_p;
    tab_p = 0;
}

