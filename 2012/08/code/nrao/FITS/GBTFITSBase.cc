//# GBTFITSBase.cc:  A base class for GBT FITS files - primary keywords.
//# Copyright (C) 2001,2002
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

#include <nrao/FITS/GBTFITSBase.h>

#include <fits/FITS/FITSDateUtil.h>
#include <casa/Quanta/MVEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRow.h>
#include <casa/Utilities/Assert.h>
#include <casa/sstream.h>

GBTFITSBase::GBTFITSBase()
{ init();}

GBTFITSBase::GBTFITSBase(const GBTFITSBase &other)
{
    *this = other;
}

GBTFITSBase &GBTFITSBase::operator=(const GBTFITSBase &other)
{
    if (this != &other) {
	init();
	itsKeys = other.itsKeys;
	itsUsedKeys = other.itsUsedKeys;
	itsOrigin = other.itsOrigin;
	itsInstrume = other.itsInstrume;
	itsMCVersion = other.itsMCVersion;
	itsFitsVer = other.itsFitsVer;
	itsDatebld = other.itsDatebld;
	itsTelescop = other.itsTelescop;
	itsObject = other.itsObject;
	itsProjid = other.itsProjid;
	itsObsid = other.itsObsid;
	itsBaseVer = other.itsBaseVer;
	itsDeviceVer = other.itsDeviceVer;
	itsScan = other.itsScan;
	itsSimulate = other.itsSimulate;
	itsDateObs = other.itsDateObs;
    }
    return *this;
}

Bool GBTFITSBase::setPrimaryKeys(const Record &keys, Bool scanlog)
{
    Bool result = True;

    init(); // reset internals

    itsKeys = keys;
    itsUsedKeys.resize(itsKeys.nfields());
    itsUsedKeys = False;

    // these are always ignored when present (these should always be
    // there, but its probably not a significant cost to check anyway)
    Int n = keys.fieldNumber("SIMPLE");
    if (n>=0) itsUsedKeys[n] = True;
    n = keys.fieldNumber("EXTEND");
    if (n>=0) itsUsedKeys[n] = True;

    // decode FITSVER
    n = keys.fieldNumber("FITSVER");
    // if not there (n<0), old-style file, nothing is expected, default FITSVER
    // and related internals are okay.
    if (n >= 0) {
	itsFitsVer = keys.asString(n);
	itsUsedKeys[n] = True;
	String leading = itsFitsVer.before(".");
	String trailing = itsFitsVer.after(".");
	itsBaseVer = stringToInt(leading);
	itsDeviceVer = stringToInt(trailing);
    }

    // keywords common to all M&C FITS file, including ScanLog

    // ORIGIN
    n = keys.fieldNumber("ORIGIN");
    if (n>=0) {
	itsOrigin = itsKeys.asString(n);
	itsUsedKeys[n] = True;
    } else if (itsBaseVer > 0 && !result) {
	result = False;
    }

    // INSTRUME
    n = keys.fieldNumber("INSTRUME");
    if (n>=0) {
	itsInstrume = itsKeys.asString(n);
	itsUsedKeys[n] = True;
    } else if (itsBaseVer > 0 && !result) {
	result = False;
    }

    // GBTMCVER
    n = keys.fieldNumber("GBTMCVER");
    if (n>=0) {
	itsMCVersion = itsKeys.asString(n);
	itsUsedKeys[n] = True;
    } else if (itsBaseVer > 0 && !result) {
	result = False;
    }

    // DATEBLD
    n = keys.fieldNumber("DATEBLD");
    if (n>=0) {
	itsDatebld = itsKeys.asString(n);
	itsUsedKeys[n] = True;
    } else if (itsBaseVer > 0 && !result) {
	result = False;
    }

    // SIMULATE
    n = keys.fieldNumber("SIMULATE");
    if (n>=0) {
	// watch for correct type
	if (itsKeys.dataType(n) == TpInt) {
	    itsSimulate = itsKeys.asInt(n) != 0;
	} else {
	    itsSimulate = itsKeys.asBool(n);
	}
	itsUsedKeys[n] = True;
    } else if (itsBaseVer > 0 && !result) {
	result = False;
    }

    // TELESCOP
    n = keys.fieldNumber("TELESCOP");
    if (n>=0) {
	itsTelescop = itsKeys.asString(n);
	itsUsedKeys[n] = True;
	// translate NRAO_GBT to just GBT
	if (itsTelescop == "NRAO_GBT") itsTelescop = "GBT";
    } else if (itsBaseVer > 0 && !result) {
	result = False;
    }

    // PROJID
    n = keys.fieldNumber("PROJID");
    if (n>=0) {
	itsProjid = itsKeys.asString(n);
	itsUsedKeys[n] = True;
    } else if (itsBaseVer > 0 && !result) {
	result = False;
    }

    // keywords everything but the ScanLog should have
    if (!scanlog) {
	// DATE-OBS and TIMESYS
	n = keys.fieldNumber("DATE-OBS");
	if (n>=0) {
	    String dateObs = itsKeys.asString(n);
	    itsUsedKeys[n] = True;
	    n = keys.fieldNumber("TIMESYS");
	    String timeSys = "UTC";
	    if (n>=0) {
		timeSys = itsKeys.asString(n);
		itsUsedKeys[n] = True;
	    } else if (itsBaseVer > 0 && !result) {
		result = False;
	    }
	    MVTime time;
	    MEpoch::Types system;
	    if (FITSDateUtil::fromFITS(time, system, dateObs, timeSys)) {
		itsDateObs = MEpoch(MVEpoch(time), MEpoch::Ref(system));
	    } else {
		result = False;
	    }
	} else if (itsBaseVer > 0 && !result) {
	    result = False;
	}
	
	// OBJECT
	n = keys.fieldNumber("OBJECT");
	if (n>=0) {
	    itsObject = itsKeys.asString(n);
	    itsUsedKeys[n] = True;
	} else if (itsBaseVer > 0 && !result) {
	    result = False;
	} else {
	    // might be in there as SOURCE
	    n = keys.fieldNumber("SOURCE");
	    if (n>=0) {
		itsObject = itsKeys.asString(n);
		itsUsedKeys[n] = True;
	    }
	}
	
	// OBSID
	n = keys.fieldNumber("OBSID");
	if (n>=0) {
	    itsObsid = itsKeys.asString(n);
	    itsUsedKeys[n] = True;
	} else if (itsBaseVer > 0 && !result) {
	    result = False;
	}
	
	// SCAN
	n = keys.fieldNumber("SCAN");
	if (n>=0) {
	    // older versions might have this as a String
	    if (itsBaseVer <= 0 && keys.type(n) == TpString) {
		itsScan = stringToInt(itsKeys.asString(n));
	    } else {
		itsScan = itsKeys.asInt(n);
	    }
	    itsUsedKeys[n] = True;
	} else if (itsBaseVer > 0 && !result) {
	    result = False;
	}
    }

    return result;
}

Bool GBTFITSBase::markAsUsed(const String &keywordName) {
    Int n = itsKeys.fieldNumber(keywordName);
    Bool result = n>=0;
    if (result) itsUsedKeys[n] = True;
    return result;
}

Record GBTFITSBase::unusedPrimaryKeys() const
{
    // start with everything
    Record result = itsKeys;
    // and remove what is used
    for (uInt i=0; i<itsUsedKeys.nelements(); i++) {
	if (itsUsedKeys[i]) result.removeField(itsKeys.name(i));
    }
    return result;
}

void GBTFITSBase::init()
{
    itsKeys = Record();
    itsUsedKeys.resize(0);

    itsOrigin = itsInstrume = itsMCVersion = itsDatebld = itsObject = 
	itsProjid = itsObsid = "";

    itsFitsVer = "0.0";
    itsTelescop = "GBT";
    itsBaseVer = itsDeviceVer = 0;
    itsScan = -1;
    itsSimulate = False;
    itsDateObs = MEpoch(MVEpoch(0.0), MEpoch::Ref(MEpoch::UTC));
}

Int GBTFITSBase::stringToInt(const String &str)
{
    Int result;
    stringstream buf;
    buf << str;
    buf >> result;
    return result;
}

Double GBTFITSBase::stringToDouble(const String &str)
{
    Double result;
    stringstream buf;
    buf << str;
    buf >> result;
    return result;
}

Table *GBTFITSBase::tableFromFITS(FITSTable &fits)
{
    Table *tab = 0;

    if (fits.isValid()) {
	TableDesc td(FITSTabular::tableDesc(fits));
	SetupNewTable newtab("", td, Table::Scratch);
	uInt nrow = fits.nrow();
	tab = new Table(newtab, TableLock::PermanentLocking);
	AlwaysAssert(tab, AipsError);
	tab->addRow(nrow);
	TableRow row(*tab);
	uInt rownr = 0;

	while (rownr < fits.nrow()) {
	    row.putMatchingFields(rownr, TableRecord(fits.currentRow()));
	    fits.next();
	    rownr++;
	}
    }

    return tab;
}

