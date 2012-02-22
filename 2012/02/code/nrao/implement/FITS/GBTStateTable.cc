//# GBTStateTable.cc: Class for dealing with the STATE table in GBT FITS files
//# Copyright (C) 2002,2003
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

#include <nrao/FITS/GBTStateTable.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <nrao/FITS/GBTFITSBase.h>
#include <fits/FITS/FITSTable.h>
#include <casa/Logging/LogIO.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/Table.h>
#include <casa/Utilities/Assert.h>

GBTStateTable::GBTStateTable()
    : itsTab(0), itsAttached(False), itsMaster("unset"), itsNumphase(-1),
      itsSwperiod(0.0), itsBaseVer(0), itsDeviceVer(0)
{}

GBTStateTable::GBTStateTable(FITSTable &fitstab)
    : itsTab(0), itsAttached(False), itsMaster("unset"), itsNumphase(-1),
      itsSwperiod(0.0), itsBaseVer(0), itsDeviceVer(0)
{reattach(fitstab);}

GBTStateTable::GBTStateTable(const GBTStateTable &other)
    : itsTab(0), itsAttached(False), itsMaster("unset"), itsNumphase(-1),
      itsSwperiod(0.0), itsBaseVer(0), itsDeviceVer(0)
{
    *this = other;
}

GBTStateTable::~GBTStateTable()
{
    detach();
}

GBTStateTable &GBTStateTable::operator=(const GBTStateTable &other)
{
    if (this != &other) {
	if (isAttached()) detach();
	if (other.isAttached()) {
	    itsTab = new Table(*other.itsTab);
	    AlwaysAssert(itsTab, AipsError);
	    itsAttached = True;
	    itsMaster = other.itsMaster;
	    itsNumphase = other.itsNumphase;
	    itsSwperiod = other.itsSwperiod;
	    itsSigref.resize(other.itsSigref.nelements());
	    itsSigref = other.itsSigref;
	    itsCal.resize(other.itsCal.nelements());
	    itsCal = other.itsCal;
	    itsBaseVer = other.itsBaseVer;
	    itsDeviceVer = other.itsDeviceVer;
	}
    }
    return *this;
}

Bool GBTStateTable::reattach(FITSTable &fitstab)
{
    detach();

    if (fitstab.isValid() && fitstab.keywords().fieldNumber("EXTNAME")>=0 &&
	(fitstab.keywords().asString("EXTNAME") == "STATE" ||
	 fitstab.keywords().asString("EXTNAME") == "PHASE")) {

	// convert the FITSTable to a Table
	itsTab = GBTFITSBase::tableFromFITS(fitstab);
	AlwaysAssert(itsTab, AipsError);

	// extract keyword values
	if (fitstab.keywords().fieldNumber("MASTER") >= 0) {
	    itsMaster = fitstab.keywords().asString("MASTER");
	} else {
	    // this must be the spectral processor
	    itsMaster = "SP";
	}
	if (fitstab.keywords().fieldNumber("NUMPHASE") >= 0) {
	    itsNumphase= fitstab.keywords().asInt("NUMPHASE");
	} else {
	    // just cound rows
	    itsNumphase = itsTab->nrow();
	}
	if (fitstab.keywords().fieldNumber("SWPERIOD") >= 0) {
	    itsSwperiod = fitstab.keywords().asDouble("SWPERIOD");
	} else {
	    // add up PHASETIM and BLANKTIM for all rows
	    // for one rcvr (assumes all rcvrs are the same, which
	    // as far as I know has always been true).
	    // array vs scalar - only do this for the array case
	    if (itsTab->actualTableDesc().isColumn("BLANKTIM") &&
		itsTab->actualTableDesc().isColumn("PHASETIM")) {
		ROTableColumn btCol(*itsTab,"BLANKTIM");
		if (btCol.columnDesc().isArray() &&
		    btCol.columnDesc().dataType() == TpFloat) {
		    ROArrayColumn<Float> blanktim(*itsTab,"BLANKTIM");
		    ROArrayColumn<Float> phasetim(*itsTab,"PHASETIM");
		    Vector<Float> thisVal;
		    itsSwperiod = 0.0;
		    for (uInt i=0;i<itsTab->nrow();i++) {
			blanktim.get(i,thisVal);
			itsSwperiod += thisVal[0];
			phasetim.get(i,thisVal);
			itsSwperiod += thisVal[0];
		    }	    
		}
	    }
	}
	// decipher FITSVER from primary keywords
	if (fitstab.primaryKeywords().fieldNumber("FITSVER") >= 0) {
	    String fitsver = fitstab.primaryKeywords().asString("FITSVER");
	    String leading = fitsver.before(".");
	    String trailing = fitsver.after(".");
	    itsBaseVer = GBTFITSBase::stringToInt(leading);
	    itsDeviceVer = GBTFITSBase::stringToInt(trailing);
	} else {
	    itsBaseVer = itsDeviceVer = 0;
	}

	itsSigref.resize(itsTab->nrow());
	itsSigref = True;
	itsCal.resize(itsTab->nrow());
	itsCal = False;
	// and get the values from the two columns
	// These may be Ints or Bytes - if the latter, may be an array (SP)
	if (itsTab->actualTableDesc().isColumn("SIGREF")) {
	    ROTableColumn srCol(*itsTab,"SIGREF");
	    if (srCol.columnDesc().dataType() == TpInt) {
		ROScalarColumn<Int> srInt(srCol);
		// itsSigref is True where the column values are 0
		itsSigref = srInt.getColumn() == 0;
	    } else if (srCol.columnDesc().dataType() == TpUChar) {
		if (srCol.columnDesc().isArray()) {
		    // The SP is the only backend that looks like this.
		    // Don't need to worry about FITSVER here since early files
		    // (FITSVER == 0.0) all had SIGREF=0 for all rows
		    ROArrayColumn<uChar> srByte(srCol);
		    // one row at a time, get the first cell in each row
		    // should be constant within a row - don't check
		    // shape should be constant from row to row
		    IPosition firstCell(srByte(0).shape());
		    firstCell = 0;
		    for (uInt i=0;i<itsTab->nrow();i++) {
			itsSigref[i] = Int(srByte(i)(firstCell)) == 0;
		    }
		} else {
		    ROScalarColumn<uChar> srByte(srCol);
		    itsSigref = srByte.getColumn() == uChar(0);
		}
	    } else if (srCol.columnDesc().dataType() == TpShort) {
		// early ACS state tables used shorts
		ROScalarColumn<Short> srShort(srCol);
		itsSigref = srShort.getColumn() == Short(0);
	    } else {
		LogIO os(LogOrigin("GBTStateTable","reattach"));
		os << LogIO::SEVERE << WHERE
		   << "Unrecognized data type for the SIGREF column in a STATE table: "
		   << srCol.columnDesc().dataType() << LogIO::POST;
	    }
	}
	if (itsTab->actualTableDesc().isColumn("CAL")) {
	    ROTableColumn calCol(*itsTab,"CAL");
	    if (calCol.columnDesc().dataType() == TpInt) {
		ROScalarColumn<Int> calInt(calCol);
		// CAL is true where the column values are > 0
		itsCal = calInt.getColumn() > 0;
	    } else if (calCol.columnDesc().dataType() == TpUChar) {
		if (calCol.columnDesc().isArray()) {
		    ROArrayColumn<uChar> calByte(calCol);
		    // The SpectralProcessor is the only backend that
		    // looks like this. 
		    // For FITSVER=0.0 for the SP, CAL==0 is ON, else off.
		    // For any other FITSVER, CAL=1 is ON, else off.

		    Int onCal = 1;
		    if (itsBaseVer == 0 && itsDeviceVer == 0) onCal = 0;

		    // one row at a time, get the first cell in each row
		    // should be constant within a row - don't check
		    // shape should be constant from row to row.
		    IPosition firstCell(calByte(0).shape());
		    firstCell = 0;
		    for (uInt i=0;i<itsTab->nrow();i++) {
			itsCal[i] = Int(calByte(i)(firstCell)) == onCal;
		    }
		} else {
		    ROScalarColumn<uChar> calByte(calCol);
		    itsCal = calByte.getColumn() > uChar(0);
		}
	    } else if (calCol.columnDesc().dataType() == TpShort) {
		// early ACS state tables used shorts
		ROScalarColumn<Short> calShort(calCol);
		itsCal = calShort.getColumn() > Short(0);
	    } else {
		LogIO os(LogOrigin("GBTStateTable","reattach"));
		os << LogIO::SEVERE << WHERE
		   << "Unrecognized data type for the SIGREF column in a STATE table: "
		   << calCol.columnDesc().dataType() << LogIO::POST;
	    }
	}

	itsAttached = True;
    }
    return itsAttached;
}

void GBTStateTable::detach()
{
    delete itsTab;
    itsTab = 0;

    itsAttached = False;
    itsMaster = "unset";
    itsNumphase = -1;
    itsSwperiod = 0.0;
}

Bool GBTStateTable::digestActState(const Table &actState, Vector<Int> &states,
				   Bool &mixedSignals, Bool &multipleCal,
				   Bool &multipleSigref, Bool oldCalState) const
{
    Bool result;

    states.resize(actState.nrow());
    states = -1;
    result = False;

    if (!isAttached()) return result;

    if (states.nelements() == 1) {
	// it must match with that one row
	// there have been problem here, this may not be the best solution
	states[0] = 0;
	mixedSignals = False;
	multipleCal = False;
	multipleSigref = False;
	oldCalState = False;
	return True;
    }

    Bool ecalUsed, icalUsed, esigrefUsed, isigrefUsed;
    Vector<Int> actStateValue(states.nelements(),0);
    esigrefUsed = examineColumn(actState, "ESIGREF", actStateValue, 2, oldCalState);
    ecalUsed = examineColumn(actState, "ECAL", actStateValue, 1, oldCalState);
    isigrefUsed = examineColumn(actState, "ISIGREF", actStateValue, 2, oldCalState);
    icalUsed = examineColumn(actState, "ICAL", actStateValue, 1, oldCalState);

    multipleCal = ecalUsed && icalUsed;
    multipleSigref = esigrefUsed && isigrefUsed;
    mixedSignals = (ecalUsed || esigrefUsed) && (icalUsed || isigrefUsed);

    if (multipleCal || multipleSigref) return result;

    Vector<Int> stateValue(itsTab->nrow(),0), intSigref(itsSigref.nelements()),
	intCal(itsCal.nelements());
    Vector<Bool> stateFound(stateValue.nelements(), False);

    // translate Bool itsSigref to integer - reversing sign to match
    // that in act state
    for (uInt i=0;i<intSigref.nelements();i++) {
	intSigref[i] = itsSigref[i] ? 0 : 1;
    }
    convertArray(intCal, itsCal);
    stateValue = intSigref*2;
    stateValue += intCal;

    // find the matches
    uInt actRow, stateRow;
    actRow = stateRow = 0;
    result = True;
    while (result && actRow < actStateValue.nelements()) {
	stateRow = 0;
	while (result && stateRow < stateValue.nelements()) {
	    if (stateValue[stateRow] == actStateValue[actRow]) {
		if (stateFound[actRow]) {
		    // this is a duplicate match
		    result = False;
		    states = -1;
		} else {
		    stateFound[actRow] = True;
		    states[stateRow] = actRow;
		}
	    }
	    stateRow++;
	}
	actRow++;
    }
    return result;
}

Bool GBTStateTable::examineColumn(const Table &actState, const String &colName,
				  Vector<Int> &colValue, Int factor,
				  Bool oldCalState) const
{
    // the column must exist
    Bool result = actState.actualTableDesc().isColumn(colName);
    if (result) {
	ROTableColumn tabcol(actState, colName);
	// must be scalar and an integer column or a short column
	result = tabcol.columnDesc().isScalar();
	Vector<Int> thiscol;
	if (result && tabcol.columnDesc().dataType() == TpInt) {
	    ROScalarColumn<Int> intcol(tabcol);
	    intcol.getColumn(thiscol);
	} else if (result && tabcol.columnDesc().dataType() == TpShort) {
	    ROScalarColumn<Short> intcol(tabcol);
	    Vector<Short> thiscolShort;
	    intcol.getColumn(thiscolShort);
	    thiscol.resize(thiscolShort.nelements());
	    convertArray(thiscol, thiscolShort);
	} else {
	    result = False;
	}
	if (result) {
	    // are there any non-zero values here
	    if (anyNE(thiscol,0)) {
		// if oldCalState is True, reverse the sense here
		// this ONLY needs to be done if there are non-zero states here
		if (oldCalState) {
		    for (uInt i=0;i<thiscol.nelements();i++) {
			if (thiscol[i] == 0) {
			    thiscol[i] = 1;
			} else {
			    thiscol[i] = 0;
			}
		    }
		}
		result = True;
		colValue += thiscol*factor;
	    } else {
		result = False;
	    }
	}
    }
    return result;
}
