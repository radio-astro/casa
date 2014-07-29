//# GBTDCRFiller.cc: A GBTBackendFiller for DCR data
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

#include <nrao/GBTFillers/GBTDCRFiller.h>
#include <nrao/GBTFillers/GBTGOFiller.h>
#include <nrao/GBTFillers/GBTIFFiller.h>
#include <nrao/GBTFillers/GBTMSFillOptions.h>
#include <nrao/GBTFillers/GBTMSTypeFiller.h>
#include <nrao/GBTFillers/GBTMSTypeDataFiller.h>
#include <nrao/GBTFillers/GBTMSTypeRecFiller.h>
#include <nrao/GBTFillers/GBTMSTypeStateFiller.h>
#include <nrao/GBTFillers/GBTRcvrCalFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/RecordDesc.h>
#include <casa/Containers/RecordField.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicMath/Math.h>
#include <measures/Measures/Stokes.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableVector.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QMath.h>
#include <casa/Utilities/Assert.h>
#include <casa/Logging/LogIO.h>

#include <fits/FITS/CopyRecord.h>

#include <nrao/FITS/GBTBackendTable.h>
#include <nrao/FITS/GBTStateTable.h>

#include <casa/sstream.h>

GBTDCRFiller::GBTDCRFiller(const String &msName, const Regex &object)
    : dcrTab_p(0), msName_p(msName), object_p(object),
      typeDataTab_p(0), typeTab_p(0), typeStateTab_p(0), typeSampTab_p(0), 
      gbtPointingTab_p(0), gbtFocusTab_p(0), ifFiller_p(0), 
      rxCalFiller_p(0), GOFiller_p(0)
{
    // openMS returns True if msName_p exists and can be opened
    if (openMS(msName_p)) {
	// and the associated GBT_type_* tables
	typeDataTab_p = new GBTMSTypeDataFiller(ms().tableName() + "/GBT_DCR_DATA");
	typeTab_p = new GBTMSTypeFiller(ms().tableName() + "/GBT_DCR");
	typeStateTab_p = new GBTMSTypeStateFiller(ms().tableName() + "/GBT_DCR_STATE");
	typeSampTab_p = new GBTMSTypeSampFiller(ms().tableName() + "/GBT_DCR_SAMPLER");
	AlwaysAssert(typeDataTab_p && typeTab_p && typeStateTab_p && typeSampTab_p, AipsError);

	// attach some standard columns
	attachColumns();

	LogIO os(LogOrigin("GBTDCRFiller","GBTDCRFiller(const String &msName, const Regex &object)"));
	os << LogIO::NORMAL << WHERE
	   << "Existing MS opened" << LogIO::POST;
    }
}

GBTDCRFiller::~GBTDCRFiller() 
{
    delete dcrTab_p;
    dcrTab_p = 0;

    delete typeDataTab_p;
    typeDataTab_p = 0;
    delete typeTab_p;
    typeTab_p = 0;
    delete typeStateTab_p;
    typeStateTab_p = 0;
    delete typeSampTab_p;
    typeSampTab_p = 0;
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

Bool GBTDCRFiller::fill(const String &backendFile,
			const Block<String> &dapFiles,
			const String &ifManagerFile,
			const Block<String> &rxCalInfoFiles,
			const GBTGOFile &GOFile,
			const String &antennaFile,
			const GBTStateTable &/*masterState*/,
			const GBTLO1File &LO1A,
			const GBTLO1File &LO1B,
			const GBTMSFillOptions &fillOptions)
{
    LogIO os(LogOrigin("GBTDCRFiller","fill(const String &backendFile, const Block<String> &dapFiles)"));
    Int rownr = 0;
    if (dcrTab_p) {
	// reopen it
	dcrTab_p->reopen(backendFile);
    } else {
	dcrTab_p = new GBTBackendTable(backendFile);
	AlwaysAssert(dcrTab_p, AipsError);
    }
    if (!dcrTab_p->isValid()) {
	os << LogIO::WARN << WHERE
	   << "Invalid backend FITS file: " << backendFile << " - skipping" << LogIO::POST;
	return False;
    }
    // mark the DATA column as being handled
    dcrTab_p->handleField("DATA");
    // for now we return if this is an empty table, filling nothing
    // but we'll return True for now so as not to signal an error
    if (dcrTab_p->nrow() == 0) {
	os << LogIO::NORMAL << WHERE
	   << "DCR scan " << dcrTab_p->keywords().asInt("SCAN") << " contains no data" << endl;
	return True;
    }

    // sanity check
    if (dcrTab_p->keywords().fieldNumber("BACKEND") < 0 ||
	dcrTab_p->keywords().asString("BACKEND") != "DCR") {
	os << LogIO::SEVERE << WHERE;
	if (dcrTab_p->keywords().fieldNumber("BACKEND") >= 0) {
	    os << "Unexpected BACKEND value : " 
	       << dcrTab_p->keywords().asString("BACKEND") << LogIO::POST;
	} else {
	    os << "No BACKEND keyword found in data file." << LogIO::POST;
	}
	return False;
    }
    // mark it as handled
    dcrTab_p->handleKeyword("BACKEND");

    // extract the scan number, assume it can be found
    Int scanNr = dcrTab_p->keywords().asInt("SCAN");
    // mark it as handled
    dcrTab_p->handleKeyword("SCAN");

    Bool goIsOkay = 
      GOFile.isValid() && GOFile.isAttached();
    Bool goBadScan = !goIsOkay || GOFile.scan() != scanNr;

    // project
    // use any value in the GO file and fall back to what is the table
    String project;
    if (goIsOkay && !goBadScan) {
	project = GOFile.projid();
    } else {
	project = dcrTab_p->keywords().asString("PROJECT");
    }
    dcrTab_p->handleKeyword("PROJECT");

    // object
    // use SOURCE name from GO file if available
    String object;
    if (goIsOkay && !goBadScan) {
	object = GOFile.object();
    } else {
	// otherwise fall back on OBJECT value
	object = dcrTab_p->keywords().asString("OBJECT");
    }
    // mark this as handled in any case
    dcrTab_p->handleKeyword("OBJECT");

    // are we supposed to fill this object
    if (!object.matches(object_p)) {
	return False;
    }

    // The time stuff
    // starting time in seconds from UTCDATE and UCTSTART keywords
    Quantum<Double> utdate(dcrTab_p->keywords().asDouble("UTDATE"), "d");
    Quantum<Double> utcstart(dcrTab_p->keywords().asDouble("UTCSTART"), "s");
    MVTime utcTime(utdate);
    utcTime = utcTime + MVTime(utcstart);
    // mark them as handled
    dcrTab_p->handleKeyword("UTDATE");
    dcrTab_p->handleKeyword("UTCSTART");

    // duration - seconds
    Double tdur = dcrTab_p->keywords().asDouble("DURATION");
    dcrTab_p->handleKeyword("DURATION");

    // offset utcTime to this backend table row
    if (dcrTab_p->rownr() > 0) {
      utcTime = utcTime + MVTime(Quantum<Double>(tdur*dcrTab_p->rownr(),"s"));
    }

    // the remaining keywords and columns are used, but leave them
    // unhandled for possible later use as is
    // number of cycles
    Int ncycles = dcrTab_p->keywords().asInt("CYCLES");
    // PHASETIM column from the state table
    ROTableVector<Double> tstate(dcrTab_p->state().table(), "PHASETIM");
    // BLANKTIM
    ROTableVector<Double> tblank(dcrTab_p->state().table(), "BLANKTIM");
    // construct the vector of exposures
    Vector<Double> texp = (tstate.makeVector() - 
			   tblank.makeVector()) * Double(ncycles);

    // we may be able to get the time since the start of the scan
    // from the TIMETAG column
    RORecordFieldPtr<Double> timetag;
    if (dcrTab_p->currentRow().fieldNumber("TIMETAG") >= 0) {
	timetag.attachToRecord(dcrTab_p->currentRow(), "TIMETAG");
	dcrTab_p->handleField("TIMETAG");
    }
    
    // decipher the row shape et al.
    RORecordFieldPtr<Array<Int> > data(dcrTab_p->currentRow(), "DATA");

    Record kw = dcrTab_p->keywords();
    Int stateAxis, sampAxis;
    stateAxis = sampAxis = -1;
    Vector<String> ctypes(2);
    ctypes[0] = "CTYPE1";
    ctypes[1] = "CTYPE2";
    for (uInt i=0;i<2;i++) {
        String thisType = kw.asString(ctypes[i]);
        if (thisType == "STATE" || thisType == "PHASE") {
            stateAxis = i;
        } else if (thisType == "RECEIVER") {
            sampAxis = i;
        } else {
            throw(AipsError("Unrecognized DATA axis in DCR fits file"));
        }
        // either way, we've handled this keyword
        dcrTab_p->handleKeyword(ctypes[i]);
    }
    AlwaysAssert(stateAxis != -1 && sampAxis != -1, AipsError);
    MVTime startTime(0.0);

    if (!hasMS()) {
	// this can only happen if the MS needs to be created
	TableDesc mainTD(MS::requiredTableDesc());
	// add the DATA column
	MS::addColumnToDesc(mainTD, MS::FLOAT_DATA, 2);
	// add in the non-standard index columns
	mainTD.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_STATE_ID"));
	mainTD.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_IF_ID"));
	mainTD.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_SAMPLER_ID"));
	// define the hyper column
	mainTD.defineHypercolumn("HYPERDATA", 3,
				 stringToVector(MS::columnName(MS::FLOAT_DATA)),
				 stringToVector(",,"));
	IPosition defaultTileShape(3, 4, 2, 16384);
	TiledShapeStMan stman("HYPERDATA", defaultTileShape);

	// create the MS, make sure the hypercolumns get set to use
	// this data manager
	Vector<String> dmCols(1);
	dmCols(0) = MS::columnName(MS::FLOAT_DATA);
	AlwaysAssert(GBTBackendFiller::createMS(msName_p, mainTD, 
						stman, dmCols, defaultTileShape,
						fillOptions, 0),
		     AipsError);;
	os << LogIO::NORMAL << WHERE
	   << "Created a new MS" << LogIO::POST;
	// now add in the associated GBT_type_* tables
	typeDataTab_p = new GBTMSTypeDataFiller(ms(), "DCR");
	typeTab_p = new GBTMSTypeFiller(ms(), "DCR");
	typeStateTab_p = new GBTMSTypeStateFiller(ms(), "DCR");
	typeSampTab_p = new GBTMSTypeSampFiller(ms(), "DCR");
	AlwaysAssert(typeDataTab_p && typeTab_p && typeStateTab_p && typeSampTab_p, AipsError);

	// Fill the subtables which never change
 	// No need to fillantenna - it should be filled at creation
	// notify the weather filler what the ANTENNA_ID is
	weatherFiller().setAntennaId(antennaFiller().antennaId());

	// attach some standard columns
	attachColumns();

	os << LogIO::NORMAL << WHERE
	   << "Constructed MS = " << ms().tableName() << LogIO::POST;
    }

    // attach and fill the IFManager and LO1 manager so that they can be used
    if (!ifFiller_p) {
	ifFiller_p = new GBTIFFiller(ms(), "DCR");
	AlwaysAssert(ifFiller_p, AipsError);
    }

    if (ifManagerFile != "") {
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

    // remember where we are
    rownr = ms().nrow();

    // at this point we may need to advance the dcr table until the
    // time is after the current max time in the ms in case this is
    // a restart of some scan that has been filled before up to some time
    
    if (rownr > 0) {
	Double lastTime = cols().time().asdouble(rownr-1);
	startTime = MVTime(lastTime/C::day);
	Double newUtcTime = utcTime.second();
	Int nDCRTabRows = (dcrTab_p->nrow() - dcrTab_p->rownr());
	while (((newUtcTime+tdur/2-startTime.second()) <= 0 ||
		nearAbs(newUtcTime+tdur/2,startTime.second())) 
	       && nDCRTabRows > 0) {
	    nDCRTabRows--;
	    if (timetag.isAttached()) {
		newUtcTime = *timetag*C::day;
	    } else {
		newUtcTime += tdur;
	    }
	    dcrTab_p->next();
	}
	utcTime = MVTime(Quantum<Double>(newUtcTime,"s"));

	// notify the weather filler what the ANTENNA_ID is
	weatherFiller().setAntennaId(antennaFiller().antennaId());
    }

    // number of rows left in the backend table
    uInt nrowsleft = (dcrTab_p->nrow() - dcrTab_p->rownr());;
    if (nrowsleft == 0) {
	// no need to go any further
	return True;
    }
    // finally start filling

    ostringstream ostr;
    ostr << "Filling DCR scan " << scanNr;
    os << LogIO::NORMAL << WHERE
       << String(ostr) << LogIO::POST;

    // DAPs first
    for (uInt i=0;i<dapFiles.nelements();i++) {
	prepareDAPfillers(dapFiles[i], startTime);
    }
    fillDAP();

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
    
    
    // MeasFrame - used in the Field filler when the position 
    // needs to be converted to J2000
    // initialize it with the antenna position
    MeasFrame frame(antennaFile_p.position());
      
    // fill the feed, pol, sw, and data_desc subtables
    feedDDFiller().fill(*dcrTab_p, *ifFiller_p, antennaFile_p,
			LO1A, LO1B, GOFile.swtchsig(), True);

    // calculate nrows given how many rows are left in the table
    Int ncorr = feedDDFiller().ncorr();
    Int nstate = feedDDFiller().nstates();
    Int nrowPerRow =  ncorr * nstate;
    Int nrows = nrowsleft * nrowPerRow;

    // extend the MS by the required number of rows
    if (nrows > 0) {
	ms().addRow(nrows);
    }


    // fill the gbt state and sampler sub tables
    typeSampTab_p->fill(dcrTab_p->sampler());
    typeStateTab_p->fill(dcrTab_p->state().table());

    // the vector of TIME, one for each row
    // of the backend table, do this in possibly smaller chunks
    Vector<Double> time(nrowsleft);
    // if we do have a timetag, initialize them to 0.0
    if (timetag.isAttached()) {
	time = 0.0;
    } else {
	// if we don't have a timetag, use tdur
        indgen(time, utcTime.second(), tdur);
    }
    // offset to middle of integration
    time += tdur/2.0;

    // fill the MS subtables
    antennaFiller().fill(antennaFile_p);

    // WEATHER
    weatherFiller().fill();

    // fill the state table - these GO functions are OK to use even
    // if goIsOK is False
    stateFiller().fill(dcrTab_p->state().sigref(), 
		       dcrTab_p->state().cal(), 
		       GOFile.procname(), 
		       GOFile.procsize(), GOFile.procseqn(),
		       GOFile.swstate(), GOFile.swtchsig());

    // FEED and DATA_DESCRIPTION_IDs
    // Index in each vector = ncorr*state + corr
    // FEED == FEED2
    Vector<Int> feedIds(nrowPerRow);
    // DATA_DESCRIPTION_ID
    Vector<Int> ddIds(nrowPerRow);
    // remember the unique spectral window IDs for each unique feed id
    // Encode the unique pair as spwid*1000+feedid - hopefully there won't be
    // more than 1000 feeds in a scan.  This is necessary because when
    // we fill the SYSCAL table - its not one row per row of the main table
    // but rather, for each time, one row for each unique spectral window and
    // feed id.  Since the set of unique pairs doesn't change with time for
    // a given scan, it makes sense to determine what that set is up front.
    // We also only prefill the table for the unique pairs.
    SimpleOrderedMap<Int, Int> uniqueFeedSpws(-1);

    // which states have True SIG
    Vector<Bool> sigs = stateFiller().sigs();
    Bool beamSwitched = feedDDFiller().hasBeamSwitched();

    feedDDFiller().origin();
    // count sampler rows
    uInt ntotsamp = 0;
    for (Int i=0;i<ncorr;i++) {
	// there is no state dependence on spwid for continuum data
	// SOURCE
	Int spwid = feedDDFiller().currIF().spwId(0);
	sourceFiller().fill(Vector<Int>(1,spwid), object);

	ntotsamp += feedDDFiller().currCorr().samplerRows().nelements();

	// load up the main table IDs appropriate to each state
	const GBTFeed *thisFeed = &(feedDDFiller().currFeed());
	for (Int j=0;j<nstate;j++) {
	    const GBTFeed *trueFeed = thisFeed;
	    if (beamSwitched &&! sigs[j] && thisFeed->srFeedPtr()) {
		trueFeed = thisFeed->srFeedPtr();
	    }
	    Int indx = j*ncorr+i;
	    if (trueFeed->feedId() >=1000) {
		os << LogIO::SEVERE << WHERE;
		os << "Unexpected FEED_ID >= 1000.  This scan can not be filled."
		   << LogIO::POST;
		return False;
	    }

	    feedIds[indx] = trueFeed->feedId();
	    ddIds[indx] = feedDDFiller().currCorr().dataDescId(j);

	    // has this spwid, feedid combination been seen yet
	    Int pair = trueFeed->feedId() + spwid*1000;
	    if (!uniqueFeedSpws.isDefined(pair)) {
		uniqueFeedSpws(pair) = uniqueFeedSpws.ndefined();

		// get TCAL and TRX and prefill the SYSCAL table
		IPosition tcalShape(2, trueFeed->polType().nelements(), feedDDFiller().currIF().nchan());
		Matrix<Float> tcal(tcalShape), trx(tcalShape);
		tcal = trx = 1.0;
		String testDate = "";
		if (rxCalFiller_p) {
		    Vector<Double> freq(1, feedDDFiller().currIF().refFreq());
		    if (feedDDFiller().currIF().ifrows().nelements() > 0) {
			Vector<Int> ifrows = feedDDFiller().currIF().ifrows();
			if (ifFiller_p && allGE(ifrows, 0) && !ifFiller_p->receiver().isNull()) {
			    // all ifrows exist and we can know the receiver 
			    // HIGH_CAL - also assume its a constant
			    Int ifloHighCal = -1;
			    if (!ifFiller_p->highCal().isNull()) {
				ifloHighCal = ifFiller_p->highCal()(ifrows[0]);
			    }
			    Bool highCal = (ifloHighCal == -1) ? 
				fillOptions.useHighCal() : (ifloHighCal > 0);
			    // the receiver name should be constant - but that
			    // isn't checked here
			    rxCalFiller_p->interpolate(ifFiller_p->receiver()(ifrows[0]),
						       trueFeed->receptorNames(),
						       trueFeed->polType(), 
						       freq, 
						       testDate,
						       tcal, trx,
						       highCal);
			}
		    }
		}
		// and fill these into the syscal table
		sysCalFiller().prefill(tcal, trx, antennaFiller().antennaId(),
				       trueFeed->feedId(), spwid);
	    }
	}
	feedDDFiller().next();
    }

    // now decode uniqueFeedSpws to a more useful form
    Vector<Int> uniqueFeeds(uniqueFeedSpws.ndefined()), 
	uniqueSpws(uniqueFeedSpws.ndefined());
    for (uInt i=0;i<uniqueFeeds.nelements();i++) {
	Int pair = uniqueFeedSpws.getKey(i);
	uniqueSpws[i] = pair/1000;
	uniqueFeeds[i] = pair % 1000;
    }

    // and fill FEED1, FEED, and DD_ID in the MAIN table in one chunk
    // they repeat every nrowPerRow of the main tabl
    IPosition lengthPos(1,nrowPerRow);
    IPosition startPos(1,rownr);
    for (uInt i=0;i<nrowsleft;i++) {
	Slicer rowSlice(startPos, lengthPos);
	// FEED1 == FEED2
	cols().feed1().putColumnRange(rowSlice, feedIds);
	cols().feed2().putColumnRange(rowSlice, feedIds);
	cols().dataDescId().putColumnRange(rowSlice, ddIds);
	startPos += nrowPerRow;
    }

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
    observationFiller().fill(project, observer, utcTime, tdur*nrowsleft,
			     antennaFile_p.telescop());

    // History
    // need a new row for each new observation_id
    if (obsId != observationFiller().observationId()) {
 	historyFiller().fill(observationFiller().observationId(), "Filling DCR data", 
			     "NORMAL", "GBTDCRFiller", MVTime(0.0));
    }

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

    // field ID
    cols().fieldId().put(rownr, fieldFiller().fieldId());

    // observation ID
    cols().observationId().put(rownr, observationFiller().observationId());

    // ANTENNA1==ATTENNA2
    cols().antenna1().put(rownr, antennaFiller().antennaId());
    cols().antenna2().put(rownr, antennaFiller().antennaId());

    // only bother with these for the first row
    // When we know how to put the data into the more than 1 CORR, then 
    // this may need to be done more often for flag and sigma
    if (rownr == 0) {
	// there are no separate subarrays here
	cols().arrayId().put(rownr, 0);

	// UVW is always 0.0
	cols().uvw().put(rownr, Vector<Double>(3,0.0));
    }

    // Slicer to fill all the SSM columns in the main table which
    // do not depend on row (can't use ISM because these might be
    // changed later in the imaging process
    Slicer rowSlicer(IPosition(1,rownr), IPosition(1,nrows));

    // FLAG - all rows being filled are not flagged
    cols().flag().putColumnRange(rowSlicer,
				 Cube<Bool>(1,1,nrows,False));

    // weight = 1 
    cols().weight().putColumnRange(rowSlicer,
				   Matrix<Float> (1,nrows, 1.0));

    // FLAG_CATEGORY seems to be required, but I have no idea how
    // to fill this initially when there are no known categories yet.
    // zero-length arrays need to be put in one at a time.
//     Cube<Bool> flagCat(1,1,0);
//     for (Int i=0;i<nrows;i++) {
// 	cols().flagCategory().put(rownr+i, flagCat);
//     }

    // flagRow
    cols().flagRow().putColumnRange(rowSlicer,
				    Vector<Bool>(nrows,False));
    
    // sigma - just give it all 1.0 for now, I'm not sure what is really best here
    cols().sigma().putColumnRange(rowSlicer,
				  Matrix<Float>(1,nrows,1.0));


    // the DATA_DESCRIPTION - unique one for each spectral window, polarization, and sampler ID
    // correlations, then states, then backends
    // these are the GBT STATE id column values, various slices for convenience
    Vector<Int> gbtStateIds(nrows);
    Vector<Int> gbtStateIdSlice(gbtStateIds(Slice(0,nstate,ncorr)));
    Vector<Int> gbtStateIdRowSlice(gbtStateIds(Slice(0,nrowPerRow,1)));
    // these are the STATE id column values, various slices for convenience
    Vector<Int> stateIds(nrows);
    Vector<Int> stateIdSlice(stateIds(Slice(0,nstate,ncorr)));
    Vector<Int> stateIdRowSlice(stateIds(Slice(0,nrowPerRow,1)));
    // these are the correlation id column values, various slices for convenience
    Vector<Int> recIds(nrows);
    Vector<Int> recIdSlice(recIds(Slice(0,ncorr,1)));
    Vector<Int> recIdRowSlice(recIds(Slice(0,nrowPerRow,1)));

    // Next the STATE ids and GBT STATE ids
    // generate the first sequence
    stateIdSlice = stateFiller().stateIds();
    gbtStateIdSlice = typeStateTab_p->stateIds();
    // repeat it ncorr-1 times
    Int start = 1;
    for (uInt i=1;i<uInt(ncorr);i++) {
	stateIds(Slice(start,nstate,ncorr)) = stateIdSlice;
	gbtStateIds(Slice(start,nstate,ncorr)) = gbtStateIdSlice;
	start += 1;
    }
    // and do that entire sequence, nrowsleft-1 times
    start = nrowPerRow;
    for (uInt i=1;i<nrowsleft;i++) {
	stateIds(Slice(start,nrowPerRow,1)) = stateIdRowSlice;
	gbtStateIds(Slice(start,nrowPerRow,1)) = gbtStateIdRowSlice;
	start += nrowPerRow;
    }
    // and save the result
    gbtStateId_p.putColumnRange(rowSlicer,
				gbtStateIds);
    cols().stateId().putColumnRange(rowSlicer,
				    stateIds);

    // and finally the corr Ids
    // generate the first sequence
    Vector<Int> sampIds = typeSampTab_p->samplerIds();
    if (sampIds.nelements() == ntotsamp) {
	// need to reorder these to match the output order
	feedDDFiller().origin();
	// count sampler rows
	uInt ntotsamp = 0;
	for (Int i=0;i<ncorr;i++) {
	    Vector<Int> theseSamps = feedDDFiller().currCorr().samplerRows();
	    for (uInt j=0;j<theseSamps.nelements();j++) {
		recIdSlice[ntotsamp++] = theseSamps[j];
	    }
	    feedDDFiller().next();
	}
    } else {
	// hope this is okay
	recIdSlice = sampIds;
    }
    // repeat it nstate-1 times
    start = ncorr;
    for (uInt i=1;i<uInt(nstate);i++) {
	recIds(Slice(start,ncorr,1)) = recIdSlice;
	start += ncorr;
    }
    // and do that entire sequence, nrowsleft-1 times
    start = nrowPerRow;
    for (uInt i=1;i<nrowsleft;i++) {
	recIds(Slice(start,nrowPerRow,1)) = recIdRowSlice;
	start += nrowPerRow;
    }
    // and save the result
    samplerId_p.putColumnRange(rowSlicer,
			       recIds);

    // How many nrowsleft to write out at a time?
    // at most 32768 elements (arbitrary)
    Int nout = 32768/(ncorr*nstate);
    if (nout <= 0) nout = 1;
    if (uInt(nout) > nrowsleft) nout = nrowsleft;

    Array<Float> outdata(IPosition(3,1,1,nout*ncorr*nstate));
    
    uInt rowcount = 0;
    Int row0 = rownr;
    Int offset = 0;
    Bool outDeleteIt, timeDeleteIt;
    Float *outptr;
    Double *timeptr;
    if (timetag.isAttached()) {
	timeptr = time.getStorage(timeDeleteIt);
    } else {
	timeptr = 0;
    }

    while (rowcount < nrowsleft) {
	const Int *inptr;
	Bool inDeleteIt;
	inptr = (*data).getStorage(inDeleteIt);
	if (offset == 0) {
	    outptr = outdata.getStorage(outDeleteIt);
	}
	
	// go through each correlation in order
	feedDDFiller().origin();
	Int sampcount = 0;
	Int ocount = 0;
	for (Int icorr=0;icorr<ncorr;icorr++) {
	    Vector<Int> sampRows(feedDDFiller().currCorr().samplerRows());
	    for (uInt isamp=0;isamp<sampRows.nelements();isamp++) {
		for (Int istate=0;istate<nstate;istate++) {
		    if (sampAxis != 0) {
			// need to reorder the input data
			// input is [nstate, ncorr]
			outptr[offset + sampcount + istate*ntotsamp]
			    = inptr[istate + sampRows[isamp]*nstate];
		    } else {
			// need to reorder the input data
			// input is [ncorr, nstate]
			outptr[offset + sampcount * istate*ntotsamp] = 
			    inptr[sampRows[isamp] + istate*ncorr];
		    }
		    ocount++;
		}
		sampcount++;
	    }
	    feedDDFiller().next();
	}

	(*data).freeStorage(inptr, inDeleteIt);
	offset = offset + ocount;
	// do we need to write it out yet, remember this row for below
	Int thisRow = rownr;
	if (offset >= Int(outdata.nelements())) {
	    outdata.putStorage(outptr, outDeleteIt);
	    // now, finally, put the data column values
	    Slicer columnSlicer(IPosition(1,rownr),
				IPosition(1,outdata.nelements()));
	    cols().floatData().putColumnRange(columnSlicer,outdata);
	    // set the output shape and value in the accompanying calibration columns
	    IPosition thisShape = IPosition(2,1,1);
	    // shapes can't be set in bulk, unfortunately.
	    for (uInt whichRow=rownr;whichRow<(rownr+outdata.nelements());whichRow++) {
		modelData().setShape(whichRow, thisShape);
		correctedData().setShape(whichRow, thisShape);
	    }

	    Array<Complex> modData(outdata.shape(), 1.0);
	    Array<Complex> cdata(outdata.shape());
	    convertArray(cdata,outdata);
	    modelData().putColumnRange(columnSlicer,modData);
	    correctedData().putColumnRange(columnSlicer,cdata);

	    rownr += outdata.nelements();
	    offset = 0;
	}
	// it would probably help if all of the following could also
	// be cached as the data column is
	// and put the unhandled fiels of the current row in to the 
	// backend subtable
	// the first time we do this, we add a merging of the
	// unhandled keywords record with the unhandled fields
	if (rowcount == 0) {
	    typeTab_p->fill(dcrTab_p->unhandledKeywords());
	    processorFiller().fill(typeTab_p->type(), typeTab_p->typeId());
	    cols().processorId().put(thisRow, processorFiller().processorId());
	}

	// and the exposures
	cols().exposure().
	  putColumnRange(Slicer(IPosition(1,thisRow),
				IPosition(1, texp.nelements()),
				IPosition(1,ncorr)), 
			 texp);
	// remember this timetag, offset to center of integration
	Double thisTime;
	if (timeptr) {
	    timeptr[rowcount] += *timetag*C::day;
	    thisTime = timeptr[rowcount];
	} else {
	    thisTime = time(rowcount);
	}
	typeDataTab_p->fill(dcrTab_p->unhandledFields(),thisTime,tdur,typeTab_p->typeId());
	dcrTab_p->next();
	rowcount++;
    }
    if (timeptr) {
	time.putStorage(timeptr, timeDeleteIt);
    }
    cols().time().putColumnRange(Slicer(IPosition(1,row0),
					IPosition(1, time.nelements()),
					IPosition(1,(ncorr*nstate))), 
				 time);
    // Fill SYSCAL table for each time and unique feed and spw
    for (uInt i=0;i<time.nelements();i++) {
	for (uInt j=0;j<uniqueFeeds.nelements();j++) {
	  sysCalFiller().fill(time[i],tdur,
			      antennaFiller().antennaId(),
			      uniqueFeeds[j],uniqueSpws[j]);
	}
    }
    // the GO file can be filled now, if necessary
    Double minTime = time(0);
    Double maxTime = time(time.nelements()-1);
    Double scanMidTime = (minTime+maxTime)/2.0;
    Double scanInterval = (maxTime-minTime)+tdur;
    if (goIsOkay) {
	if (!GOFiller_p) {
	    GOFiller_p = new GBTGOFiller(ms());
	    AlwaysAssert(GOFiller_p, AipsError);
	}
	// the scan number will be "fixed" here if its bad
	GOFiller_p->fill(GOFile, scanMidTime, scanInterval, scanNr);
    } else {
	os << LogIO::WARN << WHERE
	   << "There is no GO file for this scan (or the GO file present has problems)"
	   << LogIO::POST;	
    }
    // can also now set the time in the FEED table
    feedDDFiller().setTime(scanMidTime, scanInterval);

    // I'm not sure if the TIME_CENTROID is the same as TIME
    cols().timeCentroid().putColumnRange(Slicer(IPosition(1,row0),
						IPosition(1, time.nelements()),
						IPosition(1,(ncorr*nstate))), 
					 time);
    // pointing subtable can only be filled after the times are known
    pointingFiller().fill(ms(), antennaFiller().antennaId(), time, tdur);

    // see if there is anything to be written out at this point
    if (offset>0) {
	outdata.putStorage(outptr, outDeleteIt);
	// It must be from the current row to the end of the table
	Int nleft = ms().nrow() - rownr;
	IPosition blc(3,0,0,0);
	IPosition trc(3,0,0,(nleft-1));
	Slicer columnSlicer(IPosition(1,rownr),
			    IPosition(1,nleft));
	cols().floatData().putColumnRange(columnSlicer, outdata(blc, trc));
	// set the output shape in the accompanying calibration columns
	IPosition thisShape = IPosition(2,1,1);
	// shapes can't be set in bulk, unfortunately.
	for (Int whichRow=rownr;whichRow<(rownr+nleft);whichRow++) {
	    modelData().setShape(whichRow, thisShape);
	    correctedData().setShape(whichRow, thisShape);
	}

	IPosition outShape = outdata(blc,trc).shape();
	Array<Complex> modData(outShape, 1.0);
	Array<Complex> cdata(outShape);
	convertArray(cdata,outdata(blc,trc));
	modelData().putColumnRange(columnSlicer,modData);
	correctedData().putColumnRange(columnSlicer,cdata);
    }

    // set CHANNEL_SELECTION on MODEL_DATA
    Matrix<Int> chanSel(2,ms().spectralWindow().nrow());
    MSSpWindowColumns msSpw(ms().spectralWindow());
    chanSel.row(0) = 0;
    chanSel.row(1) = msSpw.numChan().getColumn();
    Bool setChanSel = True;
    if (modelData().keywordSet().isDefined("CHANNEL_SELECTION")) {
	Matrix<Int> storedSel;
	modelData().keywordSet().get("CHANNEL_SELECTION",storedSel);
	if (storedSel.shape() == chanSel.shape() && 
	    allEQ(storedSel, chanSel)) {
	    setChanSel = False;
	}
    }
    if (setChanSel) {
	modelData().rwKeywordSet().define("CHANNEL_SELECTION",chanSel);
    }

    // flush things
    // this gets the standard subtables and the DAP subtables of all kinds
    flush();
    // a few specialized subtables remain
    // these first 4 are always present
    typeDataTab_p->flush();
    typeTab_p->flush();
    typeStateTab_p->flush();
    typeSampTab_p->flush();

    // these may not exist
    if (gbtPointingTab_p) gbtPointingTab_p->flush();
    if (gbtFocusTab_p) gbtFocusTab_p->flush();
    if (ifFiller_p) ifFiller_p->flush();
    if (rxCalFiller_p) rxCalFiller_p->flush();
    if (GOFiller_p) GOFiller_p->flush();

    return True;
}


void GBTDCRFiller::attachColumns() 
{
    ifId_p.attach(ms(), "NRAO_GBT_IF_ID");
    gbtStateId_p.attach(ms(), "NRAO_GBT_STATE_ID");
    samplerId_p.attach(ms(), "NRAO_GBT_SAMPLER_ID");
}
