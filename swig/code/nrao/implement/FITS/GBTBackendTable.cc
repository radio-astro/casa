//# GBTBackendTable.cc:  GBTBackend makes a GBT Backend FITS table look like a FITSTabular
//# Copyright (C) 1998,1999,2000,2001,2002,2003
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

#include <nrao/FITS/GBTBackendTable.h>
#include <nrao/FITS/GBTFITSBase.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/File.h>
#include <casa/OS/RegularFile.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableRow.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>

GBTBackendTable::GBTBackendTable()
    : dataTable_p(0), sampler_p(0), actState_p(0),
      port_p(0), unhandledKeys_p(0), unhandledFields_p(0),
      fieldCopier_p(0), isValid_p(False), itsNstate(1),
      itsNchan(1), itsNsamp(1), itsFitsVer("0.0"), itsBaseVer(0), itsDeviceVer(0)
{;}

GBTBackendTable::GBTBackendTable(const String &fileName)
    : dataTable_p(0), sampler_p(0), actState_p(0),
      port_p(0), unhandledKeys_p(0), unhandledFields_p(0),
      fieldCopier_p(0), isValid_p(False), itsNstate(1),
      itsNchan(1), itsNsamp(1), itsFitsVer("0.0"), itsBaseVer(0), itsDeviceVer(0)
{
    isValid_p = reopen(fileName);
}

GBTBackendTable::~GBTBackendTable()
{
    clear_self();
}

Bool GBTBackendTable::reopen(const String &fileName, Bool resync)
{
    Int currRow = -1;
    if (dataTable_p && resync && isValid() && fileName == dataTable_p->name()) {
	// remember where we are
	// if we are past the end, this needs to be set appropriately
	if (dataTable_p->pastEnd()) {
	    currRow = dataTable_p->nrow() - 1;
	} else {
	    currRow = dataTable_p->rownr();
	}
    } else {
	resync = False;
    }

    clear_self(resync);

    // verify that fileName exists and is readable
    File thefile(fileName);
    if (thefile.exists() && thefile.isReadable()) {
	// open the file and initialize the tables, it should
	// have at least one table, start by assuming its the
	// data table ...
	Bool opening = True;
	uInt whichHDU=0;
	while(opening) {
	    whichHDU++;

	    // don't open if its empty
	    RegularFile newfile(fileName);
	    Bool ok = newfile.size() != 0;
	    if (ok) {
		dataTable_p = new FITSTable(fileName, whichHDU);
		AlwaysAssert(dataTable_p, AipsError);
		ok = dataTable_p->isValid();
	    }
	    if (!ok) {
		clear_self();
		return False;
	    }

	    if (dataTable_p->keywords().isDefined("EXTNAME")) {
		String extname(dataTable_p->keywords().asString("EXTNAME"));
		if (extname == "DATA") {
		    // stop here
		    opening = False;
		} else if (extname == "SAMPLER" || extname == "RECEIVER") {
		    // make the sampler table if not resyncing
		    if (!resync) {
			sampler_p = tableFromFITS(*dataTable_p);
			AlwaysAssert(sampler_p, AipsError);
		    }
		    delete dataTable_p;
		    dataTable_p = 0;
		} else if (extname == "STATE" || extname == "PHASE") {
		    // make the state table if not resyncing
		    if (!resync) {
			state_p.reattach(*dataTable_p);
		    }
		    delete dataTable_p;
		    dataTable_p = 0;
		} else if (extname == "ACT_STATE") {
		    // make the act_phase table if not resyncing
		    if (!resync) {
			actState_p = tableFromFITS(*dataTable_p);
			AlwaysAssert(actState_p, AipsError);
		    }
		    delete dataTable_p;
		    dataTable_p = 0;
		} else if (extname == "PORT") {
		    // make the port table if not resyncing
		    if (!resync) {
			port_p = tableFromFITS(*dataTable_p);
			AlwaysAssert(port_p, AipsError);
		    }
		    delete dataTable_p;
		    dataTable_p = 0;
		} else {
		    // this is invalid
		    opening = False;
		    clear_self();
		}
	    }
	}
	if (dataTable_p) {
	    if (!sampler_p) {
		TableDesc td;
		SetupNewTable setup("", td, Table::Scratch);
		sampler_p = new Table(setup);
		AlwaysAssert(sampler_p, AipsError);
	    }
	    if (!actState_p) {
		TableDesc td;
		SetupNewTable setup("", td, Table::Scratch);
		actState_p = new Table(setup);
		AlwaysAssert(actState_p, AipsError);
	    }
	    if (!port_p) {
		TableDesc td;
		SetupNewTable setup("", td, Table::Scratch);
		port_p = new Table(setup);
		AlwaysAssert(port_p, AipsError);
	    }
	    isValid_p = dataTable_p->isValid();
	    if (isValid_p) {
		// merge primary HDU keywords and main table keywords
		// the data table keywords take precedence
		keywords_p = dataTable_p->keywords();
		keywords_p.merge(dataTable_p->primaryKeywords(),
				 RecordInterface::SkipDuplicates);
		// set the type of this backend
		String instrume = "UNKNOWN";
		if (keywords_p.fieldNumber("INSTRUME") >= 0) {
		    instrume = keywords_p.asString("INSTRUME");
		} else if (keywords_p.fieldNumber("BACKEND") >= 0) {
		    instrume = keywords_p.asString("BACKEND");
		}
		// watch for SP
		if (instrume.matches(Regex("^SP[AB]*"))) {
		    instrume = "SP";
		}
		itsType = GBTScanLogReader::type(instrume);
		// could warn here of unknown type?

		// try and deduce size of data from the shape here
		// also set bw, centerIF, and increases for this scan
		// they can all be resized here
		uInt nsamp = sampler().nrow();
		itsBW.resize(nsamp);
		itsCenterIF.resize(nsamp);
		itsIncreases.resize(nsamp);
		itsIncreases = True;
		itsCenterIF = -1;
		itsBW = -1;

		IPosition dataShape;
		if (dataTable_p->currentRow().fieldNumber("DATA") >= 0) {
		    dataShape = dataTable_p->currentRow().shape("DATA");
		} // otherwise it must be holography data - no shape
		switch (itsType) {
		case GBTScanLogReader::DCR:
		    // nchan already set to 1
		    itsNstate = state_p.sigref().nelements();
		    break;
		case GBTScanLogReader::ACS:
		    // use TDESC3 keyword
		    {
			Int stateAxis, chanAxis, sampAxis;
			stateAxis = chanAxis = sampAxis = -1;
			if (keywords_p.fieldNumber("TDESC3") >= 0) {
			    String tdesc = keywords_p.asString("TDESC3");
			    Vector<String> axisType = stringToVector(tdesc);
			    for (uInt i=0; i<axisType.nelements(); i++) {
				if (axisType[i] == "ACT_STATE") {
				    stateAxis = i;
				} else if (axisType[i] == "LAG") {
				    chanAxis = i;
				} else if (axisType[i] == "SAMPLER") {
				    sampAxis = i;
				}
			    }
			} else {
			    // this is old data, it should look like this
			    stateAxis = 2;
			    sampAxis = 1;
			    chanAxis = 0;
			}
			if (stateAxis >= 0 && uInt(stateAxis) < dataShape.nelements()) {
			    itsNstate = dataShape(stateAxis);
			} else {
			    itsNstate = state_p.sigref().nelements();
			}
			if (chanAxis >= 0 && uInt(chanAxis) < dataShape.nelements()) {
			    itsNchan = dataShape(chanAxis);
			}
			if (sampAxis >= 0 && uInt(sampAxis) < dataShape.nelements()) {
			    itsNsamp = dataShape(sampAxis);
			} else {
			    itsNsamp = sampler_p->nrow();
			}
			
			if (port().tableDesc().isColumn("BANDWDTH")) {
			    ROScalarColumn<String> bankACol;
			    if (sampler().tableDesc().isColumn("BANK_A")) {
				bankACol.attach(sampler(), "BANK_A");
			    } else {
				bankACol.attach(sampler(), "Bank_A");
			    }
			    ROScalarColumn<Short> portACol(sampler(), "PORT_A");
			    ROScalarColumn<Short> portCol(port(), "PORT");
			    ROScalarColumn<String> bankCol(port(), "BANK");
			    ROScalarColumn<Double> bwcol(port(), "BANDWDTH");
			    ROScalarColumn<Double> fstartCol;
			    if (port().tableDesc().isColumn("FSTART")) {
				fstartCol.attach(port(), "FSTART");
			    }
			    for (uInt i=0;i<nsamp;i++) {
				String thisBankA = bankACol(i);
				Short thisPortA = portACol(i);
				for (uInt j=0;j<port().nrow();j++) {
				    if (thisBankA == bankCol(j) &&
					thisPortA == portCol(j)) {
					itsBW[i] = bwcol(j);
					if (!fstartCol.isNull()) {
					    itsCenterIF = fstartCol(j);
					} else {
					    if (itsBW[i] == 800e6) {
						itsCenterIF = 1600e6;
					    } else if (itsBW[i] == 200e6) {
						itsCenterIF = 800e6;
					    } else if (itsBW[i] == 50e6) {
						itsCenterIF = 100e6;
					    } else {
						itsCenterIF = 25e6;
					    }
					}
					if (itsBW[i] == 50e6 || itsBW[i] == 800e6) itsIncreases = False;
					// adjust centerIF by one channel width - this centerIF refers to 
					// the zero channel, which is going into the VIDEO_POINT.  The
					// spectra starts at channel 1.
					Double deltaF = itsBW[i]/itsNchan;
					if (!itsIncreases[0]) deltaF = -deltaF;
					itsCenterIF += deltaF;
					break;
				    }
				}
			    }
			} else {
			    // old style
			    ROScalarColumn<Double> bwcol(sampler(), "BANDWIDT");
			    itsBW = bwcol.getColumn();
			    itsCenterIF = 0.0;
			}
		    }
		break;
		case GBTScanLogReader::SP:
		    {
			Int stateAxis, chanAxis, sampAxis;
			stateAxis = chanAxis =sampAxis = -1;
			// CTYPEx keywords
			Vector<String> ctypes(3);
			ctypes[0] = "CTYPE1";
			ctypes[1] = "CTYPE2";
			ctypes[2] = "CTYPE3";
			for (uInt i=0;i<3;i++) {
			    String thisType = keywords_p.asString(ctypes[i]);
			    if (thisType == "STATE" || thisType == "PHASE") {
				stateAxis = i;
			    } else if (thisType == "FREQUENCY") {
				chanAxis = i;
			    } else if (thisType == "RECEIVER") {
				sampAxis = i;
			    }
			}
			if (stateAxis >= 0 && uInt(stateAxis) < dataShape.nelements()) {
			    itsNstate = dataShape(stateAxis);
			} else {
			    itsNstate = state_p.sigref().nelements();
			}
			if (chanAxis >= 0 && uInt(chanAxis) < dataShape.nelements()) {
			    itsNchan = dataShape(chanAxis);
			}
			if (sampAxis >= 0 && uInt(sampAxis) < dataShape.nelements()) {
			    itsNsamp = dataShape(sampAxis);
			} else {
			    itsNsamp = sampler_p->nrow();
			}
			ROTableColumn bandwd(sampler(), "BANDWD");
			for (uInt i=0;i<nsamp;i++) {
			    itsBW[i] = bandwd.asdouble(i);
			}
		    }
		break;
		default:
		    // do nothing
		    break;
		   
		}
	    }
	}
    }
    Bool resyncFailed = False;
    if (isValid_p) {
	// if its a resync and there aren't at least currRow + 1 rows
	// then this can't be the same file and the resync has failed
	if (resync && currRow >= Int(dataTable_p->nrow())) {
	    resyncFailed = True;
	} else {
	    if (resync) {
		// move to the appropriate starting row number, make sure
		// we don't ask to move past the end
		Int startRow = currRow + 1;
		if (startRow >= Int(dataTable_p->nrow())) 
		    startRow = dataTable_p->nrow()-1;
		dataTable_p->move(startRow);
	    }
	    keyMap_p.resize(keywords().nfields());
	    keyMap_p = 0;
	    fieldMap_p.resize(currentRow().nfields());
	    fieldMap_p = 0;

	    // these keywords are never needed, hence they are always handled
	    handleKeyword("EXTNAME");
	    handleKeyword("SIMPLE");
	    handleKeyword("EXTEND");

	    // extract FITSVER from the primary keywords
	    Int n = dataTable_p->primaryKeywords().fieldNumber("FITSVER");
	    if (n >= 0) {
		itsFitsVer = dataTable_p->primaryKeywords().asString(n);
		String leading = itsFitsVer.before(".");
		String trailing = itsFitsVer.after(".");
		itsBaseVer = GBTFITSBase::stringToInt(leading);
		itsDeviceVer = GBTFITSBase::stringToInt(trailing);
	    } else {
		// otherwise no FITSVER, default FITSVER etal is appropriate
		itsFitsVer = "0.0";
		itsBaseVer = itsDeviceVer = 0;
	    }
	}
    }
    // if the resync has failed, try a non resynced reopen
    if (resyncFailed) reopen(fileName, False);
    return isValid_p;
}

void GBTBackendTable::clear_self(Bool resync) {
    delete dataTable_p;
    dataTable_p = 0;
    if (!resync) {
	delete sampler_p;
	sampler_p = 0;
	delete actState_p;
	actState_p = 0;
	delete port_p;
	port_p = 0;
    }
    delete unhandledKeys_p;
    unhandledKeys_p = 0;
    delete unhandledFields_p;
    unhandledFields_p = 0;
    delete fieldCopier_p;
    fieldCopier_p = 0;
    isValid_p = False;
}

Table *GBTBackendTable::tableFromFITS(FITSTable &fits)
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

void GBTBackendTable::handleKeyword(const String& kwname)
{
    Int whichField = keywords().fieldNumber(kwname);
    if (whichField >= 0) {
	keyMap_p(whichField) = -1;
    }
}

void GBTBackendTable::handleField(const String& fieldname)
{
    Int whichField = currentRow().fieldNumber(fieldname);
    if (whichField >= 0) {
	fieldMap_p(whichField) = -1;
    }
}

void GBTBackendTable::initUnhandledKeys()
{
    delete unhandledKeys_p;
    unhandledKeys_p = 0;
    unhandledKeys_p = new Record(keywords());
    AlwaysAssert(unhandledKeys_p, AipsError);
    for (Int i=0;i<Int(keyMap_p.nelements());i++) {
	if (keyMap_p(i) < 0) {
	    unhandledKeys_p->removeField(keywords().name(i));
	}
    }
}

void GBTBackendTable::initUnhandledFields()
{
    delete unhandledFields_p;
    unhandledFields_p = 0;
    delete fieldCopier_p;
    fieldCopier_p = 0;

    RecordDesc rd(currentRow().description());
    for (Int i=0;i<Int(fieldMap_p.nelements());i++) {
	if (fieldMap_p(i) < 0) {
	    rd.removeField(rd.fieldNumber(currentRow().name(i)));
	}
    }

    unhandledFields_p = new Record(rd);
    AlwaysAssert(unhandledFields_p, AipsError);

    for (Int i=0;i<Int(fieldMap_p.nelements());i++) {
	if (fieldMap_p(i) >= 0) {
	    fieldMap_p(i) = rd.fieldNumber(currentRow().name(i));
	}
    }

    fieldCopier_p = new CopyRecordToRecord(*unhandledFields_p,
					   currentRow(),
					   fieldMap_p);
    AlwaysAssert(fieldCopier_p, AipsError);
    fieldCopier_p->copy();
}

