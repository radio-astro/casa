//# GBTHoloFiller.cc: a GBTBackendFiller for Holography data
//# Copyright (C) 1999,2000,2001,2002,2003,2004
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

#include <nrao/GBTFillers/GBTHoloFiller.h>
#include <nrao/GBTFillers/GBTGOFiller.h>
#include <nrao/GBTFillers/GBTIFFiller.h>
#include <nrao/GBTFillers/GBTMSFillOptions.h>
#include <nrao/GBTFillers/GBTMSTypeFiller.h>
#include <nrao/GBTFillers/GBTMSTypeDataFiller.h>
#include <nrao/GBTFillers/GBTRcvrCalFiller.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/Constants.h>
#include <measures/Measures/Stokes.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ScaColDesc.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Assert.h>

#include <fits/FITS/CopyRecord.h>

#include <nrao/FITS/GBTBackendTable.h>

#include <casa/sstream.h>

GBTHoloFiller::GBTHoloFiller(const String &msName, const Regex &object)
    : holoTab_p(0), msName_p(msName), object_p(object),
      typeDataTab_p(0), typeTab_p(0), gbtPointingTab_p(0), gbtFocusTab_p(0), 
      ifFiller_p(0), rxCalFiller_p(0), GOFiller_p(0)
{	
    // openMS returns True if msName_p exists and can be opened
    if (openMS(msName_p, False)) {
	// and the associated GBT_type_* tables
	typeDataTab_p = new GBTMSTypeDataFiller(ms().tableName() + "/GBT_HOLOGRAPHY_DATA");
	AlwaysAssert(typeDataTab_p, AipsError);
	typeTab_p = new GBTMSTypeFiller(ms().tableName() + "/GBT_HOLOGRAPHY");
	AlwaysAssert(typeTab_p, AipsError);
	// attach the standard columns
	attachColumns();
	LogIO os(LogOrigin("GBTHoloFiller","GBTHoloFiller(const String &msName, const Regex &object)"));
	os << "Existing MS re-opened MS = " << ms().tableName() << LogIO::POST;
    }
}

GBTHoloFiller::~GBTHoloFiller() 
{
    delete holoTab_p;
    holoTab_p = 0;

    delete typeDataTab_p;
    typeDataTab_p = 0;

    delete typeTab_p;
    typeTab_p = 0;

    delete ifFiller_p;
    ifFiller_p = 0;

    delete rxCalFiller_p;
    rxCalFiller_p = 0;

    delete GOFiller_p;
    GOFiller_p = 0;

    delete gbtPointingTab_p;
    gbtPointingTab_p = 0;

    delete gbtFocusTab_p;
    gbtFocusTab_p = 0;
}

// the fillLags argument is ignored
Bool GBTHoloFiller::fill(const String &backendFile,
			 const Block<String> &dapFiles,
			 const String &ifManagerFile,
			 const Block<String> &rxCalInfoFiles,
			 const GBTGOFile &GOFile,
			 const String &antennaFile,
			 const GBTStateTable &/*masterState*/,
			 const GBTLO1File &/*LO1A*/,
                         const GBTLO1File &/*LO1B*/,
			 const GBTMSFillOptions &fillOptions)
{
    Int rownr = 0;
    LogIO os(LogOrigin("GBTHoloFiller","fill(const String &backendFile, const Block<String> &dapFiles)"));
    if (holoTab_p) {
	// reopen it
	holoTab_p->reopen(backendFile);
    } else {
	holoTab_p = new GBTBackendTable(backendFile);
	AlwaysAssert(holoTab_p, AipsError);
    }
    if (!holoTab_p->isValid()) {
	os << LogIO::WARN << WHERE
	   << "Invalid backend FITS file : " << backendFile << " - skipping" << LogIO::POST;
	return False;
    }

    // for now we return if this is an empty table, filling nothing
    // but we'll return True for now so as not to signal an error
    if (holoTab_p->nrow() == 0) return True;

    // sanity check
    if (holoTab_p->keywords().fieldNumber("BACKEND") < 0 ||
	!holoTab_p->keywords().asString("BACKEND").matches(Regex("Holo.*"))) {
	os << LogIO::SEVERE << WHERE
	   << "Unexpected BACKEND value : " 
	   << holoTab_p->keywords().asString("BACKEND") << LogIO::POST;
	return False;
    }
    // mark it as handled
    holoTab_p->handleKeyword("BACKEND");

    // extract the scan number, assume it can be found
    Int scanNr = holoTab_p->keywords().asInt("SCAN");
    // mark it as handled
    holoTab_p->handleKeyword("SCAN");

    Bool goIsOkay = 
	GOFile.isValid() && GOFile.isAttached();
    Bool goBadScan = !goIsOkay || GOFile.scan() != scanNr;

    // object
    // use SOURCE name from GO file if available
    String object;
    if (goIsOkay && !goBadScan) {
	object = GOFile.object();
    } else {
	// otherwise fall back on OBJECT value
	object = holoTab_p->keywords().asString("OBJECT");
    }
    // mark this as handled in any case
    holoTab_p->handleKeyword("OBJECT");

    // are we supposed to fill this object
    if (!object.matches(object_p)) {
	return False;
    }

    // project
    // use any value in the GO file and fall back to what is the table
    String project;
    if (goIsOkay && !goBadScan) {
	project = GOFile.projid();
    } else {
	project = holoTab_p->keywords().asString("PROJECT");
    }
    holoTab_p->handleKeyword("PROJECT");

    // and the time stuff, UTDATE and UTCSTART can be ignored here
    // because the holography backend currently has a DMJD column
    // but leave this code here, but commented out, since I suspect
    // that in the long run holography files will look like the other
    // ones and this will win.

    // starting time in seconds
    // Use DMJD column, UTDATE, UTCSTART and DATE-OBS are not used here, ignore them
    holoTab_p->handleKeyword("UTDATE");
    holoTab_p->handleKeyword("UTCSTART");
    holoTab_p->handleKeyword("DATE-OBS");

    // duration
    Double tdur = holoTab_p->keywords().asDouble("DURATION");
    holoTab_p->handleKeyword("DURATION");

    // the DMJD column contains the time, in MJD, for the start of each row
    RORecordFieldPtr<Double> dmjd;
    dmjd.attachToRecord(holoTab_p->currentRow(), "DMJD");
    holoTab_p->handleField("DMJD");
    
    // how many rows are really left to fill
    Int nrows = (holoTab_p->nrow() - holoTab_p->rownr());
    MVTime startTime(0.0);
    if (!hasMS()) {
	// this can only happened if the MS needs to be created
	TableDesc mainTD(MS::requiredTableDesc());
	// and the IF index column
	mainTD.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_IF_ID"));

	// create the MS
	AlwaysAssert(GBTBackendFiller::createMS(msName_p, mainTD, fillOptions, nrows,
						False),
		     AipsError);;
	os << LogIO::NORMAL << WHERE
	   << "Created a new MS" << LogIO::POST;
	// now add in the associated GBT_type_* tables
	typeDataTab_p = new GBTMSTypeDataFiller(ms(), "HOLOGRAPHY");
	AlwaysAssert(typeDataTab_p, AipsError);
	typeTab_p = new GBTMSTypeFiller(ms(), "HOLOGRAPHY");
	AlwaysAssert(typeTab_p, AipsError);
	// The MS subtables which never change and can be filled in now
	
	// no need to do antenna, it is always filled at creation
	// notify the weather filler what the ANTENNA_ID is
	weatherFiller().setAntennaId(antennaFiller().antennaId());

	// attach the standard columns
	attachColumns();
	os << LogIO::NORMAL << WHERE
	   << "Constructed MS = " << ms().tableName() << LogIO::POST;
    } else {
	// remember where we are
	rownr = ms().nrow();
	// in this case, make sure we step the backend table ahead until
	// the dmjd value is >= the time in the last row
	if (rownr > 0) {
	    Double lastTime = cols().time().asdouble(ms().nrow()-1);
	    // the 
	    // that should be MJD seconds, convert it to days for comparison
	    lastTime /= C::day;
	    startTime = MVTime(lastTime);
	    while (*dmjd < lastTime && nrows > 0) {
		nrows--;
		holoTab_p->next();
	    }
	}
	// notify the weather filler what the ANTENNA_ID is
	weatherFiller().setAntennaId(antennaFiller().antennaId());

	// add the appropriate number of rows
	if (nrows > 0) {
	    ms().addRow(nrows);
	}
    }

    // DAPs first, even if there aren't any rows since this may be a restart and they may be
    // the only files which need to be updated
    for (uInt i=0;i<dapFiles.nelements();i++) {
	prepareDAPfillers(dapFiles[i], startTime);
    }
    fillDAP();

    if (ifManagerFile != "") {
	if (!ifFiller_p) {
	    ifFiller_p = new GBTIFFiller(ms(), "HOLOGRAPHY");
	    AlwaysAssert(ifFiller_p, AipsError);
	    // an existing table or a new one?
	}
	ifFiller_p->fill(ifManagerFile);
    }

    if (rxCalInfoFiles.nelements() > 0) {
	if (!rxCalFiller_p) {
	    rxCalFiller_p = new GBTRcvrCalFiller(ms());
	    AlwaysAssert(rxCalFiller_p, AipsError);
	}
	for (uInt i=0;i<rxCalInfoFiles.nelements();i++) {
	    rxCalFiller_p->fill(rxCalInfoFiles[i]);
	}
    }

    if (antennaFile != "") {
	antennaFile_p.attach(antennaFile);
	pointingFiller().setAntennaFile(&antennaFile_p);
	if (fillOptions.fillRawPointing()) {
	    if (!gbtPointingTab_p) {
		// does one already exist or is a new one necessary
		Int whichField = ms().keywordSet().fieldNumber("NRAO_GBT_POINTING");
		if (whichField >= 0 && ms().keywordSet().type(whichField) == TpTable) {
		    // open it as is
		    // String tabName = ms().rwKeywordSet().asTable(whichField).tableName();
		    String tabName = ms().tableName() + "/NRAO_GBT_POINTING";
		    gbtPointingTab_p = new Table(tabName, tableLock(),
						 Table::Update);
		    AlwaysAssert(gbtPointingTab_p, AipsError);
		} else {
		    // create it as an empty table, fillPointingTable() adds any columns
		    SetupNewTable newtab(ms().tableName() + "/NRAO_GBT_POINTING", TableDesc(), Table::New);
		    gbtPointingTab_p = new Table(newtab, tableLock());
		    AlwaysAssert(gbtPointingTab_p, AipsError);
		    ms().rwKeywordSet().defineTable("NRAO_GBT_POINTING", *gbtPointingTab_p);
		}
	    }
	    antennaFile_p.fillPointingTable(*gbtPointingTab_p);
	}
	if (fillOptions.fillRawFocus()) {
	    if (!gbtFocusTab_p) {
		// does one already exist or is a new one necessary
		Int whichField = ms().keywordSet().fieldNumber("NRAO_GBT_FOCUS");
		if (whichField >= 0 && ms().keywordSet().type(whichField) == TpTable) {
		    // open it as is
		    // String tabName = ms().rwKeywordSet().asTable(whichField).tableName();
		    String tabName = ms().tableName() + "/NRAO_GBT_FOCUS";
		    gbtFocusTab_p = new Table(tabName, tableLock(),
					      Table::Update);
		    AlwaysAssert(gbtFocusTab_p, AipsError);
		} else {
		    // create it as an empty table, fillFocusTable() adds any columns
		    SetupNewTable newtab(ms().tableName() + "/NRAO_GBT_FOCUS", TableDesc(), Table::New);
		    gbtFocusTab_p = new Table(newtab, tableLock());
		    AlwaysAssert(gbtFocusTab_p, AipsError);
		    ms().rwKeywordSet().defineTable("NRAO_GBT_FOCUS", *gbtFocusTab_p);
		}
	    }
	    antennaFile_p.fillFocusTable(*gbtFocusTab_p);
	}
    }
      
    if (nrows == 0) {
	// nothing to fill, just return
	return True;
    }

    // MeasFrame - used in the Field filler when the position needs 
    // to be converted to J2000
    // initialize it with the antenna position
    MeasFrame frame(antennaFile_p.position());

    // fill the MS subtables
    antennaFiller().fill(antennaFile_p);

    // WEATHER
    weatherFiller().fill();

    // POLARIZATION
    // The Holography backend, this is still very much a fudge
    polarizationFiller().fill(1,Vector<Int>(1,Stokes::Undefined),
			      Matrix<Int>(2,1,0));

    // SPECTRAL_WINDOW - for the DCR without frequency and polarization
    //    information, nothing currently changes, make sure row 0 is set
    //    appropriately for the DCR with NCHAN=0
    //    Int oldSpid = specWinFiller().spectralWindowId(0);
    specWinFiller().fill(0, 0);

    // DATA_DESCRIPTION
    // Use what has just gone on before
    dataDescFiller().fill(polarizationFiller().polarizationId(),
			  specWinFiller().spectralWindowIds());

    // SOURCE - need a new row is sp win id is new, which can't
    // happen here yet, but do this code anyway so that its ready
    // also need a new row for each new object
    sourceFiller().fill(specWinFiller().spectralWindowIds(), object);

    // FIELD table
    if (goIsOkay) {
	// use this reference direction even if the go scan number is bad, no where else to get it
	fieldFiller().fill(sourceFiller().sourceId(), object, GOFile.refdir());
    } else {
	fieldFiller().fill(sourceFiller().sourceId(), object, MDirection());
    }

    // observation - remember the current OBSERVATION_ID in case it changes
    Int obsId = observationFiller().observationId();
    // grab the observers name if available
    String observer;
    if (goIsOkay) {
	// use the observer here even if go scan number is bad, no where else to get it
	observer = GOFile.observer();
    }
    observationFiller().fill(project, observer, *dmjd*C::day, tdur*nrows,
			     antennaFile_p.telescop());

    // History
    // need a new row for each new observation_id
    if (obsId != observationFiller().observationId()) {
	historyFiller().fill(observationFiller().observationId(), 
			     "Filling Holography data", "NORMAL", 
			     "GBTHoloFiller", MVTime(0.0));
    }

    // feeds
    feedFiller().fill();

    // syscal
    // The Holography backend has no TCAL info, so don't fill this

    // the IF table id
    if (ifFiller_p && ifManagerFile!="" && !ifFiller_p->currentTable().isNull()) {
	ifId_p.put(rownr, ifFiller_p->index());
    } else {
	ifId_p.put(rownr, -1);
    }

    // these columns all use the ISM and so only the first row needs to be put
    // since these are all constant during a scan
    // fill the scan number
    cols().scanNumber().put(rownr, scanNr);

    // duration
    cols().interval().put(rownr, tdur);

    // feed1==feed2 == 0
    cols().feed1().put(rownr, 0);
    cols().feed2().put(rownr, 0);

    // data_desc_id
    cols().dataDescId().put(rownr, dataDescFiller().dataDescriptionId(0));

    // field ID
    cols().fieldId().put(rownr, fieldFiller().fieldId());

    cols().observationId().put(rownr, observationFiller().observationId());

    // not doing anything yet with STATE table
    cols().stateId().put(rownr, -1);

    // ANTENNA1==ATTENNA2
    cols().antenna1().put(rownr, antennaFiller().antennaId());
    cols().antenna2().put(rownr, antennaFiller().antennaId());

    // only bother with these for the first row
    // When we how to fill the FLOAT_DATA or DATA column with holography data,
    // SIGMA and FLAG may need to be done more often
    if (rownr == 0) {
	// there are no separate subarrays here
	cols().arrayId().put(rownr, 0);

	// UVW is always 0.0
	cols().uvw().put(rownr, Vector<Double>(3,0.0));

    }
    // these can't use the ISM because they may be changed downstream during
    // flagging or imaging, etc.  Hence, they need values for each row.
    // flagRow
    cols().flagRow().putColumnRange(Slicer(IPosition(1,rownr), IPosition(1,nrows)),
				    Vector<Bool>(nrows,False));
    
    // Because these are empty, they need to be put in 1 row at a time
    Matrix<Bool> nullFlag(0,0);
    Vector<Float> nullWeight(0);
    Cube<Bool> nullFlagCat(0,0,0);
    for (Int i=0;i<nrows;i++) {
	// flag - there is no DATA column so ncorr and nchan are both zero
	cols().flag().put(rownr+i,nullFlag);
    
	// weight - there is no DATA column so nchan is zero
	cols().weight().put(rownr+i,nullWeight);
    
	// FLAG_CATEGORY seems to be required, but I have no idea how
	// to fill this initially when there are no known categories yet.
	//	cols().flagCategory().put(rownr+i,nullFlagCat);
    
	// sigma - there is no DATA column so nchan is zero
	cols().sigma().put(rownr+i,nullWeight);
    }

    ostringstream ostr;
    ostr << "Filling holography scan " << scanNr;
    os << LogIO::NORMAL << WHERE
       << String(ostr) << LogIO::POST;
    Double minTime, maxTime;
    minTime = maxTime = 0.0;
    for (Int i=0;i<nrows;i++) {
	uInt thisrow = i + rownr;
	// set the time
	Double thisTime = *dmjd*C::day + tdur/2.0;
	if (i==0) minTime = thisTime;
	if (i==(nrows-1)) maxTime = thisTime;
	cols().time().put(thisrow, thisTime);
	// I'm not sure if the TIME_CENTROID is the same as TIME
	cols().timeCentroid().put(thisrow, thisTime);
	// fill the POINTING table for this time
	pointingFiller().fill(ms(), antennaFiller().antennaId(), Vector<Double>(1,thisTime), tdur);
	// eventually we will treat this as two antennas using 
	// the DATA column in the main table....
	// put the unhandled fields of the current row in to the GBT_type_DATA subtable
	// the first time we do this, put the unhandled keywords into GBT_type subtable
	if (i == 0) {
	    // update the exposure, same as duration
	    cols().exposure().put(thisrow, tdur);
	    typeTab_p->fill(holoTab_p->unhandledKeywords());
	    processorFiller().fill(typeTab_p->type(),typeTab_p->typeId());
	    cols().processorId().put(thisrow, processorFiller().processorId());
	    
	}
	typeDataTab_p->fill(holoTab_p->unhandledFields(),thisTime,tdur,typeTab_p->typeId());
	holoTab_p->next();
    }
    // the GO file can be filled now, if necessary
    if (goIsOkay) {
	if (!GOFiller_p) {
	    GOFiller_p = new GBTGOFiller(ms());
	    AlwaysAssert(GOFiller_p, AipsError);
	}
	// the scan number will be "fixed" here if its bad
	GOFiller_p->fill(GOFile, (minTime+maxTime)/2.0, (maxTime-minTime)+tdur, scanNr);	
    } else {
	os << LogIO::WARN << WHERE
	   << "There is no GO file for this scan (or the GO file present has problems)"
	   << LogIO::POST;	
    }    

    // flush things
    // this gets the standard subtables and the DAP subtables of all kinds
    flush();
    // a few specialized subtables remain
    // these first 2 are always present
    typeDataTab_p->flush();
    typeTab_p->flush();

    // these may not exist
    if (gbtPointingTab_p) gbtPointingTab_p->flush();
    if (gbtFocusTab_p) gbtFocusTab_p->flush();
    if (ifFiller_p) ifFiller_p->flush();
    if (rxCalFiller_p) rxCalFiller_p->flush();
    if (GOFiller_p) GOFiller_p->flush();

    return True;
}

void GBTHoloFiller::attachColumns() {
    ifId_p.attach(ms(), "NRAO_GBT_IF_ID");
}
