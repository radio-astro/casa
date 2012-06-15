//# GBTPointModelFiller.cc: fill the pointingModel from the ANTENNA file
//# Copyright (C) 2001,2003
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

#include <nrao/GBTFillers/GBTPointModelFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Vector.h>
#include <nrao/FITS/GBTAntennaFile.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableRow.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>

GBTPointModelFiller::GBTPointModelFiller(MeasurementSet &ms) 
    : itsTable(0), itsTableRow(0)
{
    // is there an NRAO_GBT_POINTING_MDOEL table already available?
    String tabName("NRAO_GBT_POINTING_MODEL");
    Int whichField = ms.keywordSet().fieldNumber(tabName);
    if (whichField >= 0 && ms.keywordSet().type(whichField) == TpTable) {
	// open it as is
	// tabName = ms.rwKeywordSet().asTable(whichField).tableName();
	tabName = ms.tableName() + "/" + tabName;
	itsTable = new Table(tabName, GBTBackendFiller::tableLock(),
			     Table::Update);
	AlwaysAssert(itsTable, AipsError);
    } else {
	// need to create a new table - make it empty
	SetupNewTable newtab(ms.tableName() + "/" + tabName,
			     TableDesc(), Table::New);
	itsTable = new Table(newtab, GBTBackendFiller::tableLock());
	AlwaysAssert(itsTable, AipsError);
	// and add this in as a keyword
	ms.rwKeywordSet().defineTable(tabName, *itsTable);
    }
}

GBTPointModelFiller::~GBTPointModelFiller()
{
    delete itsTable;
    itsTable = 0;

    delete itsTableRow;
    itsTableRow = 0;
}

void GBTPointModelFiller::fill(const GBTAntennaFile &antennaFile)
{
    // always add a new row if there are no rows yet
    Int rownr = pointingModelId();
    
    // do all of the necessary columns exist in itsTable
    Vector<String> colNames(antennaFile.pointingModel().nfields());

    uInt validCols = 0;
    Bool colsAdded = False;

    for (uInt i=0;i<antennaFile.pointingModel().nfields();i++) {
	// assumes that the type doesn't ever change for the same name
	String fieldName = antennaFile.pointingModel().name(i);
	colNames(validCols) = fieldName;
	validCols++;
	if (!itsTable->tableDesc().isColumn(fieldName)) {
	    switch (antennaFile.pointingModel().dataType(i)) {
		// only strings, floats, and doubles should happen here
	    case TpString:
		itsTable->addColumn(ScalarColumnDesc<String>(fieldName));
		colsAdded = True;
		break;
	    case TpFloat:
		itsTable->addColumn(ScalarColumnDesc<Float>(fieldName));
		colsAdded = True;
		break;
	    case TpDouble:
		itsTable->addColumn(ScalarColumnDesc<Double>(fieldName));
		colsAdded = True;
		break;
	    default:
		// just ignore this
		validCols--;
		break;
	    }
	}
    }
    colNames.resize(validCols, True);
    if (!colsAdded && itsTableRow) {
	colsAdded = (validCols != itsTableRow->columnNames().nelements()) ||
	    anyNE(colNames, itsTableRow->columnNames());
    }
    if (colsAdded || !itsTableRow) {
	if (itsTableRow) {
	    delete itsTableRow;
	    itsTableRow = 0;
	}
	itsTableRow = new TableRow(*itsTable, colNames);
	AlwaysAssert(itsTableRow, AipsError);
    }

    Bool newRow = True;
    if (rownr >= 0) {
	// try the last filled row - can it be reused
	itsTableRow->get(rownr);
	if (antennaFile.equalPointingModel(itsTableRow->record())) newRow = False;
    }
    if (newRow) {
	rownr = itsTable->nrow();
	itsTable->addRow();
	itsTableRow->put(uInt(rownr), TableRecord(antennaFile.pointingModel()));
    }
}
