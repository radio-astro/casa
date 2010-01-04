//# GBTMeanFocusFiller.cc: fill the MEAN_FOCUS table from the ANTENNA file
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

#include <nrao/GBTFillers/GBTMeanFocusFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>
#include <nrao/FITS/GBTAntennaFile.h>

#include <casa/Arrays/ArrayUtil.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Quanta/Quantum.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <casa/Utilities/Assert.h>

GBTMeanFocusFiller::GBTMeanFocusFiller(MeasurementSet &ms) 
    : itsTable(0)
{
    // is there an NRAO_GBT_MEAN_FOCUS table already available?
    String tabName("NRAO_GBT_MEAN_FOCUS");
    Int whichField = ms.keywordSet().fieldNumber(tabName);
    if (whichField >= 0 && ms.keywordSet().type(whichField) == TpTable) {
	// open it as is
	// tabName = ms.rwKeywordSet().asTable(whichField).tableName();
	tabName = ms.tableName() + "/" + tabName;
	itsTable = new Table(tabName, GBTBackendFiller::tableLock(),
			     Table::Update);
	AlwaysAssert(itsTable, AipsError);
    } else {
	// need to create a new table 
	// must have OPTICS_TYPE column
	TableDesc td;
	td.addColumn(ScalarColumnDesc<String>("OPTICS_TYPE", "GBT OPTICS TYPE - FROM EXTNAME KEYWORD"));
	SetupNewTable newtab(ms.tableName() + "/" + tabName, td, Table::New);
	itsTable = new Table(newtab, GBTBackendFiller::tableLock());
	// and add this in as a keyword
	AlwaysAssert(itsTable, AipsError);
	ms.rwKeywordSet().defineTable(tabName, *itsTable);
    }
}

GBTMeanFocusFiller::~GBTMeanFocusFiller()
{
    delete itsTable;
    itsTable = 0;
}

void GBTMeanFocusFiller::fill(Double time, Double interval, 
			      GBTAntennaFile &antennaFile)
{
    if (antennaFile.opticsType() == "ANTPOSPF") {
	Quantity focus, rotation, x;
	// don't worry about the return value, if we get to this point,
	// antennaFile must be attached and if the return value falls off
	// the end then the values set here are the best we'll do anyway.
	// It might be nice to emit a warning, I suppose.
	antennaFile.getPrimeFocus(time, interval, focus, rotation, x);
	if (itsPFfocus.isNull()) {
	    // check to see if the necessary columns exist and add them
	    // in as ScalarQuant columns as necessary
	    TableDesc td;
	    if (!itsTable->tableDesc().isColumn("OPTICS_TYPE")) {
		td.addColumn(ScalarColumnDesc<String>("OPTICS_TYPE",
						      "The EXTNAME keyword value"));
	    }
	    if (!itsTable->tableDesc().isColumn("PF_FOCUS")) {
		td.addColumn(ScalarColumnDesc<Double>("PF_FOCUS"));
		TableQuantumDesc tqd(td, "PF_FOCUS", focus.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    if (!itsTable->tableDesc().isColumn("PF_ROTATION")) {
		td.addColumn(ScalarColumnDesc<Double>("PF_ROTATION"));
		TableQuantumDesc tqd(td, "PF_ROTATION", rotation.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    if (!itsTable->tableDesc().isColumn("PF_X")) {
		td.addColumn(ScalarColumnDesc<Double>("PF_X"));
		TableQuantumDesc tqd(td, "PF_X", x.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    for (uInt i=0;i<td.ncolumn();i++) {
		itsTable->addColumn(td[i]);
	    }
	    if (itsOpticsType.isNull()) 
		itsOpticsType.attach(*itsTable, "OPTICS_TYPE");
	    itsPFfocus.attach(*itsTable, "PF_FOCUS");
	    itsPFrotation.attach(*itsTable, "PF_ROTATION");
	    itsPFx.attach(*itsTable, "PF_X");
	}
	// can the last row be reused
	Int rownr = itsTable->nrow()-1;
	if (rownr < 0 || 
	    itsOpticsType(rownr) != "ANTPOSPF" ||
	    itsPFfocus(rownr) != focus.getValue() ||
	    itsPFrotation(rownr) != rotation.getValue() ||
	    itsPFx(rownr) == x.getValue()) {
	    // need a new row
	    rownr = itsTable->nrow();
	    itsTable->addRow();
	    itsOpticsType.put(rownr, "ANTPOSPF");
	    itsPFfocus.put(rownr, focus.getValue());
	    itsPFrotation.put(rownr, rotation.getValue());
	    itsPFx.put(rownr, x.getValue());
	}
    } else if (antennaFile.opticsType() == "ANTPOSGR") {
	Quantity xp, yp, zp, xt, yt, zt;
	// don't worry about the return value, if we get to this point,
	// antennaFile must be attached and if the return value falls off
	// the end then the values set here are the best we'll do anyway.
	// It might be nice to emit a warning, I suppose.
	antennaFile.getGregorianFocus(time, interval, xp, yp, zp, xt, yt, zt);
	if (itsSRxp.isNull()) {
	    // check to see if the necessary columns exist and add them
	    // in as ScalarQuant columns as necessary
	    TableDesc td;
	    if (!itsTable->tableDesc().isColumn("OPTICS_TYPE")) {
		td.addColumn(ScalarColumnDesc<String>("OPTICS_TYPE",
						      "The EXTNAME keyword value"));
	    }
	    if (!itsTable->tableDesc().isColumn("SR_XP")) {
		td.addColumn(ScalarColumnDesc<Double>("SR_XP"));
		TableQuantumDesc tqd(td, "SR_XP", xp.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    if (!itsTable->tableDesc().isColumn("SR_YP")) {
		td.addColumn(ScalarColumnDesc<Double>("SR_YP"));
		TableQuantumDesc tqd(td, "SR_YP", yp.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    if (!itsTable->tableDesc().isColumn("SR_ZP")) {
		td.addColumn(ScalarColumnDesc<Double>("SR_ZP"));
		TableQuantumDesc tqd(td, "SR_ZP", zp.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    if (!itsTable->tableDesc().isColumn("SR_XT")) {
		td.addColumn(ScalarColumnDesc<Double>("SR_XT"));
		TableQuantumDesc tqd(td, "SR_XT", xt.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    if (!itsTable->tableDesc().isColumn("SR_YT")) {
		td.addColumn(ScalarColumnDesc<Double>("SR_YT"));
		TableQuantumDesc tqd(td, "SR_YT", yt.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    if (!itsTable->tableDesc().isColumn("SR_ZT")) {
		td.addColumn(ScalarColumnDesc<Double>("SR_ZT"));
		TableQuantumDesc tqd(td, "SR_ZT", zt.getFullUnit());
		// Its not clear if this step is necessary or not
		tqd.write(td);
	    }
	    for (uInt i=0;i<td.ncolumn();i++) {
		itsTable->addColumn(td[i]);
	    }
	    if (itsOpticsType.isNull()) 
		itsOpticsType.attach(*itsTable, "OPTICS_TYPE");
	    itsSRxp.attach(*itsTable, "SR_XP");
	    itsSRyp.attach(*itsTable, "SR_YP");
	    itsSRzp.attach(*itsTable, "SR_ZP");
	    itsSRxt.attach(*itsTable, "SR_XT");
	    itsSRyt.attach(*itsTable, "SR_YT");
	    itsSRzt.attach(*itsTable, "SR_ZT");
	}
	// can the last row be reused
	Int rownr = itsTable->nrow()-1;;
	if (rownr < 0 ||
	    itsOpticsType(rownr) != "ANTPOSGR" ||
	    itsSRxp(rownr) != xp.getValue() ||
	    itsSRyp(rownr) != yp.getValue() ||
	    itsSRzp(rownr) != zp.getValue() ||
	    itsSRxt(rownr) != xt.getValue() ||
	    itsSRyt(rownr) != yt.getValue() ||
	    itsSRzt(rownr) != zt.getValue()) {
	    // need a new row
	    rownr = itsTable->nrow();
	    itsTable->addRow();
	    itsOpticsType.put(rownr, "ANTPOSGR");
	    itsSRxp.put(rownr, xp.getValue());
	    itsSRyp.put(rownr, yp.getValue());
	    itsSRzp.put(rownr, zp.getValue());
	    itsSRxt.put(rownr, xt.getValue());
	    itsSRyt.put(rownr, yt.getValue());
	    itsSRzt.put(rownr, zt.getValue());
	}
    } // otherwise, its a stow optics (ANTPOSST) and there is nothing to do
}
