//# GBTMSTypeStateFiller.h: used to fill the GBT_type_[MASTER]STATE table
//# Copyright (C) 2000,2001,2003
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

#include <nrao/GBTFillers/GBTMSTypeStateFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ColumnsIndex.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRow.h>
#include <casa/Utilities/Assert.h>

GBTMSTypeStateFiller::GBTMSTypeStateFiller(const String &tableName) 
    : tab_p(0), tabRow_p(0), index_p(0), type_p("unknown"),
      typesDiffer_p(False)
{
    // extract the type name
    Int ndelim = tableName.freq("_");
    if (ndelim > 1) {
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

GBTMSTypeStateFiller::GBTMSTypeStateFiller(Table &attachTable, const String &type,
					   Bool masterState)
    : tab_p(0), tabRow_p(0), index_p(0), type_p(type), typesDiffer_p(False)
{
    // make a very bare bones table
    String tableName = "GBT_" + type + "_";
    if (masterState) tableName = tableName + "MASTER";
    tableName = tableName + "STATE";
    SetupNewTable newtab(attachTable.tableName()+"/"+tableName, 
			 TableDesc(), Table::New);
    tab_p = new Table(newtab, GBTBackendFiller::tableLock());
    AlwaysAssert(tab_p, AipsError);
    attachTable.rwKeywordSet().defineTable(tableName, *tab_p);
    updateTableRow();
    updateIndex(tab_p->tableDesc().columnNames());
}

void GBTMSTypeStateFiller::fill(const Table &other)
{
    stateIds_p.resize(other.nrow());
    if (other.nrow() == 0) return;
    
    ROTableRow inTabRow(other);
    checkRecord(inTabRow.get(0),other.tableDesc().columnNames());
    
    Record thisRec;
    for (uInt i=0; i< other.nrow(); i++) {
	Bool addIt = False;
	if (!typesDiffer_p) {
	    // okay as is
	    thisRec = inTabRow.get(i);
	} else {
	    // need to copy fields in order, watching for the different types
	    Record rowRec = inTabRow.get(i);
	    // initialize with index fields for first row
	    if (i == 0) {
		if (index_p) {
		    thisRec = index_p->accessKey();
		} else {
		    thisRec = rowRec;
		}
	    }
	    // I believe all types except uChar and Short should be 
	    // simple copies
	    for (uInt k=0;k<thisRec.nfields();k++) {
		String fieldName = thisRec.name(k);
		switch (thisRec.dataType(k)) {
		    // only need to worry about scalar types found in 
		    // binary tables
		case TpBool:
		    thisRec.define(k, rowRec.asBool(fieldName));
		    break;
		case TpUChar:
		    if (rowRec.dataType(fieldName) != thisRec.dataType(k)) {
			// get the current value, converted to a uChar
			thisRec.define(k, uChar(rowRec.asShort(fieldName)));
		    }
		    break;
		case TpShort:
		    if (rowRec.dataType(fieldName) != thisRec.dataType(k)) {
			// get the current value, convert to a Short
			thisRec.define(k, Short(rowRec.asuChar(fieldName)));
		    }
		    break;
		case TpInt:
		    thisRec.define(k, rowRec.asInt(fieldName));
		    break;
		case TpFloat:
		    thisRec.define(k, rowRec.asFloat(fieldName));
		    break;
		case TpDouble:
		    thisRec.define(k, rowRec.asDouble(fieldName));
		    break;
		case TpComplex:
		    thisRec.define(k, rowRec.asComplex(fieldName));
		    break;
		case TpString:
		    thisRec.define(k, rowRec.asString(fieldName));
		    break;
		default:
		    break;
		}
	    }
	}
	if (!index_p) {
	    addIt = True;
	} else {
	    Vector<uInt> rownrs = index_p->getRowNumbers(thisRec);
	    if (rownrs.nelements() == 0) {
		addIt = True;
	    } else {
		// use the last one found
		stateIds_p(i) = rownrs(rownrs.nelements()-1);
	    }
	}
	if (addIt) {
	    stateIds_p(i) = tab_p->nrow();
	    tab_p->addRow(1);
	    tabRow_p->putMatchingFields((tab_p->nrow()-1), thisRec);
	}
    }
}

void GBTMSTypeStateFiller::checkRecord(const Record &other, const Vector<String> &fieldNames)
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
    Bool colsRemoved = False;
    typesDiffer_p = False;
    Vector<String> thisColumnNames(tab_p->tableDesc().columnNames());
    uInt i=0;
    while (i<thisColumnNames.nelements() && 
	   (!colsRemoved || !typesDiffer_p)) {
	String thisName(thisColumnNames(i));
	Int fieldNr = other.fieldNumber(thisName);
	colsRemoved = colsRemoved || fieldNr < 0;
	if ((fieldNr >= 0) && !typesDiffer_p) {
	    typesDiffer_p = other.dataType(fieldNr) != 
	      tab_p->tableDesc()[i].trueDataType();
	}
	i++;
    }
    if (colsAdded || colsRemoved) {
	updateTableRow();
	updateIndex(fieldNames);
    }
}


void GBTMSTypeStateFiller::updateTableRow()
{
    delete tabRow_p;
    tabRow_p = 0;
    tabRow_p = new TableRow(*tab_p);
    AlwaysAssert(tabRow_p, AipsError);
}

void GBTMSTypeStateFiller::updateIndex(const Vector<String> &colNames) 
{
    // don't do the index if any columns are non-scalar
    Bool okToIndex = True;
    for (uInt i=0;i<colNames.nelements();i++) {
	if (!tab_p->tableDesc().columnDesc(i).isScalar()) {
	    okToIndex = False;
	    break;
	}
    }
    delete index_p;
    index_p = 0;
    if (okToIndex) {
	index_p = new ColumnsIndex(*tab_p, colNames);
	AlwaysAssert(index_p, AipsError);
    }
}

GBTMSTypeStateFiller::~GBTMSTypeStateFiller() {
    delete tabRow_p;
    tabRow_p = 0;
    delete index_p;
    index_p = 0;
    delete tab_p;
    tab_p = 0;
}

