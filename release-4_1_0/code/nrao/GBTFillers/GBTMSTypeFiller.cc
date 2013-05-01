//# GBTMSTypeFiller.h: a GBTMSTypeFiller is used to fill the GBT_type table
//# Copyright (C) 2000,2001,2002,2003
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

#include <nrao/GBTFillers/GBTMSTypeFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordDesc.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ColumnsIndex.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRow.h>
#include <casa/Utilities/Assert.h>

GBTMSTypeFiller::GBTMSTypeFiller(const String &tableName) 
    : tab_p(0), tabRow_p(0), index_p(0), typeId_p(-1), type_p("unknown")
{
    // extract the type name
    Int ndelim = tableName.freq("_");
    if (ndelim > 0) {
	String *fields = new String[ndelim+1];
	split(tableName, fields, (ndelim+1), "_");
	type_p = fields[ndelim];
        delete [] fields;
    }
    tab_p = new Table(tableName, GBTBackendFiller::tableLock(), 
    		      Table::Update);
    AlwaysAssert(tab_p, AipsError);
    updateTableRow();
    updateIndex(tab_p->tableDesc().columnNames());
    
}

GBTMSTypeFiller::GBTMSTypeFiller(Table &attachTable, const String &type)
    : tab_p(0), tabRow_p(0), index_p(0), typeId_p(-1), type_p(type)
{
    // make a very bare bones table
    String tableName = "GBT_" + type;
    SetupNewTable newtab(attachTable.tableName()+"/"+tableName, 
			 TableDesc(), Table::New);
    tab_p = new Table(newtab, GBTBackendFiller::tableLock());
    AlwaysAssert(tab_p, AipsError);
    attachTable.rwKeywordSet().defineTable(tableName, *tab_p);
    updateTableRow();
    updateIndex(tab_p->tableDesc().columnNames());
}

void GBTMSTypeFiller::fill(const Record &other)
{
    Record tmp = other;
    checkRecord(tmp);
    Bool addIt = False;
    if (!index_p) {
	addIt = True;
    } else {
	Vector<uInt> rownrs = index_p->getRowNumbers(tmp);
	if (rownrs.nelements() == 0) {
	    addIt = True;
	} else {
	    // use the last one found
	    typeId_p = rownrs(rownrs.nelements()-1);
	}
    }
    if (addIt) {
	typeId_p = tab_p->nrow();
	tab_p->addRow(1);
	tabRow_p->putMatchingFields(typeId_p, tmp);
    }
}

void GBTMSTypeFiller::checkRecord(Record &other)
{
    Bool colsAdded = False;
    Bool indexChanged = False;
    Vector<Bool> typesOK(other.nfields(), True);
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
	typesOK[i] = other.type(i) == tab_p->tableDesc().columnDesc(name).dataType();
    }
    if (colsAdded) updateTableRow();

    if (anyEQ(typesOK, False)) {
        indexChanged = True;
	// this should only happen for Float/Double.  It should be fixed upstream
	// in the FITS classes somewhere.  Floating point keywords should all always
	// return double.
	// iterate backwards so that kw i is still in the same place when we get to it
	for (Int i=(typesOK.nelements()-1);i>=0;i--) {
	    if (!typesOK[i]) {
		String name = other.name(i);
		switch (other.type(i)) {
		case TpFloat:
		    {
			Float tmp = other.asFloat(i);
			Double dtmp = tmp;
			other.removeField(name);
			other.define(name, dtmp);
		    }
                    break;
		case TpDouble:
		    {
			Double dtmp = other.asDouble(i);
			Float tmp = dtmp;
			other.removeField(name);
			other.define(name, tmp);
		    }
                    break;
		default:
		    // do nothing
                    break;
		}
	    }
	}
    }

    Bool eqTypes = False;
    if (indexChanged || 
        (index_p && !other.description().isEqual(index_p->accessKey().description(),eqTypes))) {
	updateIndex(other.description());
    }
}


void GBTMSTypeFiller::updateTableRow()
{
    delete tabRow_p;
    tabRow_p = 0;
    tabRow_p = new TableRow(*tab_p);
    AlwaysAssert(tabRow_p, AipsError);
}

void GBTMSTypeFiller::updateIndex(const RecordDesc &other) 
{
    Vector<String> colNames(other.nfields());
    for (uInt i=0;i<other.nfields();i++) {
	colNames(i) = other.name(i);
    }
    updateIndex(colNames);
}

void GBTMSTypeFiller::updateIndex(const Vector<String> &colnames) 
{
    Bool okToIndex = True;
    for (uInt i=0;i<colnames.nelements();i++) {
	if (!tab_p->tableDesc().columnDesc(colnames(i)).isScalar()) {
	    okToIndex = False;
	    break;
	}
    }
    delete index_p;
    index_p = 0;
    if (okToIndex) {
	index_p = new ColumnsIndex(*tab_p, colnames);
	AlwaysAssert(index_p, AipsError);
    }
}

GBTMSTypeFiller::~GBTMSTypeFiller() {
    delete tabRow_p;
    tabRow_p = 0;
    delete index_p;
    index_p = 0;
    delete tab_p;
    tab_p = 0;
}

