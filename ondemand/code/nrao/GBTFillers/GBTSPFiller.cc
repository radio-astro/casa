//# GBTSPFiller.cc: A GBTBackendFiller for Spectral Processor data
//# Copyright (C) 2000,2001,2002,2003,2004
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

#include <nrao/GBTFillers/GBTSPFiller.h>
#include <nrao/GBTFillers/GBTGOFiller.h>
#include <nrao/GBTFillers/GBTIFFiller.h>
#include <nrao/GBTFillers/GBTMSFillOptions.h>
#include <nrao/GBTFillers/GBTMSTypeFiller.h>
#include <nrao/GBTFillers/GBTMSTypeDataFiller.h>
#include <nrao/GBTFillers/GBTMSTypeRecFiller.h>
#include <nrao/GBTFillers/GBTMSTypeStateFiller.h>
#include <nrao/GBTFillers/GBTRcvrCalFiller.h>

#include <casa/Arrays/ArrayIter.h>
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
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/RefRows.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableAttr.h>
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

GBTSPFiller::GBTSPFiller(const String &msName, const Regex &object)
    : spTab_p(0), msName_p(msName), object_p(object),
      typeDataTab_p(0), typeTab_p(0), typeStateTab_p(0), typeSampTab_p(0), 
      gbtPointingTab_p(0), gbtFocusTab_p(0), ifFiller_p(0), 
      rxCalFiller_p(0), GOFiller_p(0)
{
    // openMS returns True if msName_p exists and can be opened
    if (openMS(msName_p)) {
	// and the associated GBT_type_* tables
	typeDataTab_p = new GBTMSTypeDataFiller(ms().tableName() + "/GBT_SPECTRALPROCESSOR_DATA");
	typeTab_p = new GBTMSTypeFiller(ms().tableName() + "/GBT_SPECTRALPROCESSOR");
	typeStateTab_p = new GBTMSTypeStateFiller(ms().tableName() + "/GBT_SPECTRALPROCESSOR_STATE");
	typeSampTab_p = new GBTMSTypeSampFiller(ms().tableName() + "/GBT_SPECTRALPROCESSOR_SAMPLER");
	AlwaysAssert(typeDataTab_p && typeTab_p && typeStateTab_p && typeSampTab_p, AipsError);

	// mark columns in the SAMPLER table which are handled elsewhere
	handleColumns();

	// attach some standard columns
	attachColumns();

	LogIO os(LogOrigin("GBTSPFiller","GBTSPFiller(const String &msName, const Regex &object)"));
	os << LogIO::NORMAL << WHERE
	   << "Existing MS opened" << LogIO::POST;
    }
}

GBTSPFiller::~GBTSPFiller() 
{
    delete spTab_p;
    spTab_p = 0;

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

Bool GBTSPFiller::fill(const String &backendFile,
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
    LogIO os(LogOrigin("GBTSPFiller","fill(const String &backendFile, const Block<String> &dapFiles)"));
    Int rownr = 0;
    if (spTab_p) {
	// reopen it
	spTab_p->reopen(backendFile);
    } else {
	spTab_p = new GBTBackendTable(backendFile);
	AlwaysAssert(spTab_p, AipsError);
    }
    if (!spTab_p->isValid()) {
	os << LogIO::WARN << WHERE
	   << "Invalid backend FITS file: " << backendFile << " - skipping" << LogIO::POST;
	return False;
    }
    // mark the DATA column as being handled
    spTab_p->handleField("DATA");
    // for now we return if this is an empty table, filling nothing
    // but we'll return True for now so as not to signal an error
    if (spTab_p->nrow() == 0) {
	os << LogIO::NORMAL << WHERE
	   << "SP scan " << spTab_p->keywords().asInt("SCAN") << " contains no data" << endl;
	return True;
    }

    // sanity check
    if (spTab_p->keywords().fieldNumber("BACKEND") < 0 ||
	!spTab_p->keywords().asString("BACKEND").matches(Regex("^SP(A|B|AB).*$"))) {
	os << LogIO::SEVERE << WHERE;
	if (spTab_p->keywords().fieldNumber("BACKEND") >= 0) {
	    os << "Unexpected BACKEND value : "
	       << spTab_p->keywords().asString("BACKEND") << LogIO::POST;
	} else {
	    os << "No BACKEND keyword found in data file." << LogIO::POST;
	}
	return False;
    }
    // mark it as handled
    spTab_p->handleKeyword("BACKEND");

    // extract the scan number, assume it can be found
    Int scanNr = spTab_p->keywords().asInt("SCAN");
    // mark it as handled
    spTab_p->handleKeyword("SCAN");

    Bool goIsOkay = 
	GOFile.isValid() && GOFile.isAttached();
    Bool goBadScan = !goIsOkay || GOFile.scan() != scanNr;

    // project
    // use any value in the GO file and fall back to what is the table
    String project;
    if (goIsOkay && !goBadScan) {
	project = GOFile.projid();
    } else {
	project = spTab_p->keywords().asString("PROJECT");
    }
    spTab_p->handleKeyword("PROJECT");

    // object
    // use SOURCE name from GO file if available
    String object;
    if (goIsOkay && !goBadScan) {
	object = GOFile.object();
    } else {
	// otherwise fall back on OBJECT value
	object = spTab_p->keywords().asString("OBJECT");
    }
    // mark this as handled in any case
    spTab_p->handleKeyword("OBJECT");

    // are we supposed to fill this object
    if (!object.matches(object_p)) {
	return False;
    }

    // and the time stuff
    // we use the UTCDATE and UTCSTART found in the columns in the data table
    // mark them as handled
    spTab_p->handleKeyword("UTDATE");
    spTab_p->handleKeyword("UTCSTART");
    // the UTDATE and UTCSTART fields
    RORecordFieldPtr<Int> utdateField(spTab_p->currentRow(), "UTDATE");
    RORecordFieldPtr<Double> utcstartField(spTab_p->currentRow(), "UTCSTART");
    // mark the fields as being handled
    spTab_p->handleField("UTDATE");
    spTab_p->handleField("UTCSTART");

    // we never use these, and they are redundant
    spTab_p->handleKeyword("UTCSTOP");
    spTab_p->handleKeyword("DATE-OBS");

    // duration - seconds
    Double tdur = spTab_p->keywords().asDouble("INTTIME");
    spTab_p->handleKeyword("INTTIME");

    // get the timestamp for this row
    MVTime utcTime = rowTime(utdateField, utcstartField, tdur);

    // the remaining keywords and columns are used, but leave them
    // unhandled for possible later use as is
    // PHASETIM column from the state table
    ROArrayColumn<Float> tstate(spTab_p->state().table(), "PHASETIM");
    Matrix<Float> mstate(tstate.getColumn());
    // BLANKTIM
    ROArrayColumn<Float> tblank(spTab_p->state().table(), "BLANKTIM");
    Matrix<Float> mblank(tblank.getColumn());
    // infer the number of cycles - state and blank times are 
    // the same for all samplers at a given state
    Int ncycles = Int(tdur/sum(mstate.row(0)) + sum(mblank.row(0)) + 0.5);
    // construct the vector of exposures 
    Vector<Float> ftexp = (mstate.row(0) + mblank.row(0)) * Double(ncycles);
    Vector<Double> texp(ftexp.nelements());
    convertArray(texp, ftexp);

    // decipher the row shape et al.
    RORecordFieldPtr<Array<Float> > data(spTab_p->currentRow(), "DATA");
    Record kw = spTab_p->keywords();
    Int stateAxis, sampAxis, freqAxis;
    stateAxis = sampAxis = freqAxis = -1;
    Vector<String> ctypes(3);
    ctypes[0] = "CTYPE1";
    ctypes[1] = "CTYPE2";
    ctypes[2] = "CTYPE3";
    for (uInt i=0;i<3;i++) {
	String thisType = kw.asString(ctypes[i]);
	if (thisType == "STATE" || thisType == "PHASE") {
	    stateAxis = i;
	} else if (thisType == "RECEIVER") {
	    sampAxis = i;
	} else if (thisType == "FREQUENCY") {
	    freqAxis = i;
	} else {
	    throw(AipsError("Unrecognized DATA axis in Spectral Processor FITS file"));
	}
	// we've handled this keyword
	spTab_p->handleKeyword(ctypes[i]);
    }
    AlwaysAssert(stateAxis != -1 && sampAxis != -1 && freqAxis != -1, AipsError);

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
	IPosition defaultTileShape(3, 4, 1023, 32);
	TiledShapeStMan stman("HYPERDATA", defaultTileShape);

	// create the MS, make sure the hypercolumns get set to use
	// this data manager
	Vector<String> dmCols(1);
	dmCols(0) = MS::columnName(MS::FLOAT_DATA);
	AlwaysAssert(GBTBackendFiller::createMS(msName_p, mainTD, 
						stman, dmCols, defaultTileShape,
						fillOptions, 0, True),
		     AipsError);;
	os << LogIO::NORMAL << WHERE
	   << "Created a new MS" << LogIO::POST;
	// now add in the associated GBT_type_* tables
	typeDataTab_p = new GBTMSTypeDataFiller(ms(), "SPECTRALPROCESSOR");
	typeTab_p = new GBTMSTypeFiller(ms(), "SPECTRALPROCESSOR");
	typeStateTab_p = new GBTMSTypeStateFiller(ms(), "SPECTRALPROCESSOR");
	typeSampTab_p = new GBTMSTypeSampFiller(ms(), "SPECTRALPROCESSOR");
	AlwaysAssert(typeDataTab_p && typeTab_p && typeStateTab_p && typeSampTab_p, AipsError);

	// mark columns in the SAMPLER table which are handled elsewhere
	handleColumns();

	// Fill the subtables which never change
	// No need to fill antenna - it should be filled at creation
	// notify the weather filler what the ANTENNA_ID is
	weatherFiller().setAntennaId(antennaFiller().antennaId());

	// attach some standard columns
	attachColumns();

	os << LogIO::NORMAL << WHERE
	   << "Constructed MS = " << ms().tableName() << LogIO::POST;
    }

    // attach and fill the IFManager and LO1 manager so that they can be used
    if (!ifFiller_p) {
	ifFiller_p = new GBTIFFiller(ms(), "SpectralProcessor");
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

    // at this point we may need to advance the sp table until the
    // time is after the current max time in the ms in case this is
    // a restart of some scan that has been filled before up to some time
	    
    if (rownr > 0) {
	Double lastTime = cols().time().asdouble(rownr-1);
	startTime = MVTime(lastTime/C::day);
	Int nSPTabRows = (spTab_p->nrow() - spTab_p->rownr());
	while (((utcTime.second()-startTime.second()) <= 0 ||
		nearAbs(utcTime.second(),startTime.second())) 
	       && nSPTabRows > 0) {
	    nSPTabRows--;
	    spTab_p->next();
	    utcTime = rowTime(utdateField, utcstartField, tdur);
	}
	// notify the weather filler what the ANTENNA_ID is
	weatherFiller().setAntennaId(antennaFiller().antennaId());

    }

    // number of rows left in the backend table
    uInt nrowsleft = (spTab_p->nrow() - spTab_p->rownr());;
    if (nrowsleft == 0) {
	// no need to go any further
	return True;
    }
    
    // finally start filling

    ostringstream ostr;
    ostr << "Filling SP scan " << scanNr;
    os << LogIO::NORMAL << WHERE
       << String(ostr) << LogIO::POST;

    // DAPs first, even if there aren't any rows since this may be a restart and they may be
    // the only files which need to be updated
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
		    String tabName = ms().rwKeywordSet().tableAttributes(whichField).name();
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
		    String tabName = ms().rwKeywordSet().tableAttributes(whichField).name();
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

    // MeasFrame - used in the Field filler when the position needs to be 
    // converted to J2000. initialize it with the antenna position
    MeasFrame frame(antennaFile_p.position());

    // fill the feed, pol, sw, and data_desc subtables
    feedDDFiller().fill(*spTab_p, *ifFiller_p, antennaFile_p,
			LO1A, LO1B, GOFile.swtchsig());

    // calculate nrows given how many rows are left in the table
    Int ncorr = feedDDFiller().ncorr();
    Int nstate = feedDDFiller().nstates();
    Int nrowPerRow = ncorr * nstate;
    Int nrows = nrowsleft * nrowPerRow;

    // extend the MS by the required number of rows
    if (nrows > 0) {
	ms().addRow(nrows);
    } 

    // fill the state and sampler sub tables
    typeSampTab_p->fill(spTab_p->sampler());
    typeStateTab_p->fill(spTab_p->state().table());

    // the vector of TIME, one for each row
    // of the backend table, do this in possibly smaller chunks?
    Vector<Double> time(nrowsleft);
    // if we don't have a timetag, use tdur
    indgen(time, utcTime.second(), tdur);

    // fill the MS subtables
    antennaFiller().fill(antennaFile_p);

    // WEATHER
    weatherFiller().fill();

    // fill the state table - these GO functions are OK to use even
    // if goIsOK is False
    stateFiller().fill(spTab_p->state().sigref(), 
		       spTab_p->state().cal(), 
		       GOFile.procname(), 
		       GOFile.procsize(), GOFile.procseqn(),
		       GOFile.swstate(), GOFile.swtchsig());

    // Remember the spwids for each corr and state
    // These two are filled in to the MAIN table in the appropriate rows
    // in order - all corrs for each state in turn
    // So, index k in each vector = ncorr*(state) + corr
    // FEED1 == FEED2
    Vector<Int> feedIds(nrowPerRow);
    // DATA_DESCRIPTION_ID
    Vector<Int> ddIds(nrowPerRow);
    // IDs for each state - but filled for each corr as well
    Vector<Int> stateIds(nrowPerRow);
    Vector<Int> stateSlice(stateIds(Slice(0,nstate,ncorr)));
    // IDs for each GBT state - but filled for each corr as well
    Vector<Int> gbtStateIds(nrowPerRow);
    Vector<Int> gbtStateSlice(gbtStateIds(Slice(0,nstate,ncorr)));
    // Next the STATE ids and GBT STATE Ids
    // generate the first sequence
    stateSlice = stateFiller().stateIds();
    // which stateSlice elements have SIG=True
    Vector<Bool> sigs = stateFiller().sigs();

    gbtStateSlice = typeStateTab_p->stateIds();
    // SIGMA for each row in an integration
    Vector<Float> sigmas(nrowPerRow);
    // set the initial time in the frame (defaults to UTC - i.e. okay)
    frame.set(MEpoch(Quantity(time(0),"s")));

    // remember the unique spectral window IDs for each unique feed id
    // Encode the unique pair as spwid*1000+feedid - hopefully there won't be
    // more than 1000 feed in a scan.  This is necessary because when
    // we fill he SYSCAL table - its not one row per row of the main table
    // but rather, for each time, one row for each unique spectral window and
    // feed id.  Since the set of unique pairs doesn't change with time for
    // a given scan, it makes sense to determine what that set is up front.
    // We also only prefill the table for the unique pairs.
    SimpleOrderedMap<Int, Int> uniqueFeedSpws(-1);

    Bool beamSwitched = feedDDFiller().hasBeamSwitched();

    feedDDFiller().origin();
    for (Int i=0;i<ncorr;i++) {
	// state IDs
	if (i != 0) {
	    stateIds(Slice(i,nstate,ncorr)) = stateSlice;
	    gbtStateIds(Slice(i,nstate,ncorr)) = gbtStateSlice;
	}

	const GBTFeed *thisFeed = &(feedDDFiller().currFeed());

	Int ifrow = -1;
	if (feedDDFiller().currIF().ifrows().nelements() > 0) {
	    ifrow = feedDDFiller().currIF().ifrows()[0];
	}
	Int ifloHighCal = -1;
	if (ifrow >= 0 && !ifFiller_p->highCal().isNull()) {
	    ifloHighCal = ifFiller_p->highCal()(ifrow);
	}
	Bool highCal = (ifloHighCal == -1) ? 
	    fillOptions.useHighCal() : (ifloHighCal > 0);

	// assumes receiver name is the same for all ifrows here
	String rxname;
	if (ifrow >= 0 && !ifFiller_p->receiver().isNull()) {
	    rxname = ifFiller_p->receiver()(ifrow);
	}

	// state dependent part
 	for (uInt j=0;j<uInt(nstate);j++) {
	    const GBTFeed *trueFeed = thisFeed;
	    if (beamSwitched && !sigs[j] && thisFeed->srFeedPtr()) {
		trueFeed = thisFeed->srFeedPtr();
	    }
	    uInt indx = ncorr*j + i;
	    Int spwid = feedDDFiller().currIF().spwId(j);
	    if (trueFeed->feedId() >=1000) {
		os << LogIO::SEVERE << WHERE;
		os << "Unexpected FEED_ID >= 1000.  This scan can not be filled."
		   << LogIO::POST;
		return False;
	    }
		
	    ddIds[indx] = feedDDFiller().currCorr().dataDescId(j);
	    feedIds[indx] = trueFeed->feedId();

	    // SOURCE 
	    if (goIsOkay) {
	        // use this rest frequency even if the scan number is bad, no where else to get it
		sourceFiller().fill(Vector<Int>(1,spwid), object,
				    GOFile.restfrq().getValue(), GOFile.velocity().getValue());
	    } else {
		sourceFiller().fill(Vector<Int>(1,spwid), object);
	    }

	    // SIGMA - assuming hanning smoothing => k2 == 2.0
	    //       - also assumes TSYS == 1.0, will be scaled later in calibration step
	    //       - and uses the exposure for this state as texp - can also be
	    //         scaled during the calibration step
	    sigmas[indx] = 1.3 / sqrt(2.0 * ftexp[j] * 
				      abs(feedDDFiller().currIF().deltaFreq()));

	    // has this spwid, feedid combination been seen yet
	    Int pair = feedIds[indx] + spwid*1000;
	    if (!uniqueFeedSpws.isDefined(pair)) {
		uniqueFeedSpws(pair) = uniqueFeedSpws.ndefined();
		// and get TCAL and TRX and prefill the SYSCAL table
		IPosition tcalShape(2, trueFeed->polType().nelements(), feedDDFiller().currIF().nchan());
		Matrix<Float> tcal(tcalShape), trx(tcalShape);
		tcal = trx = 1.0;
		String testDate = "";
		if (rxCalFiller_p) {
		    // need to construct the frequency axis here
		    Vector<Double> freqs(feedDDFiller().currIF().nchan());
		    indgen(freqs);
		    freqs -= feedDDFiller().currIF().refChan();
		    freqs *= feedDDFiller().currIF().deltaFreq();
		    freqs += (feedDDFiller().currIF().refFreq() + 
			      feedDDFiller().currIF().offsets()[j]);
		    rxCalFiller_p->interpolate(rxname,
					       trueFeed->receptorNames(),
					       trueFeed->polType(), 
					       freqs, testDate,
					       tcal, trx,
					       highCal);
		}
		// and fill these into the syscal table
		sysCalFiller().prefill(tcal, trx, antennaFiller().antennaId(), 
				       feedIds[indx], spwid);
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

    // and fill FEED1, FEED, and DD_ID in the MAIN table in one chunk, 
    // they repeat every nrowPerRow of the main table
    IPosition length(1,nrowPerRow);
    IPosition start(1,rownr);
    for (uInt i=0;i<nrowsleft;i++) {
	Slicer rowSlice(start, length);
	// FEED1 == FEED2
	cols().feed1().putColumnRange(rowSlice, feedIds);
	cols().feed2().putColumnRange(rowSlice, feedIds);
	cols().dataDescId().putColumnRange(rowSlice, ddIds);
	cols().stateId().putColumnRange(rowSlice, stateIds);
	// and the non-standard index column
	gbtStateId_p.putColumnRange(rowSlice, gbtStateIds);
	start += nrowPerRow;
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
 	historyFiller().fill(observationFiller().observationId(), 
			     "Filling SpectralProcessor data", "NORMAL",
			     "GBTSPFiller", MVTime(0.0));
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

    // ANTENNA1==ANTENNA2 and they never change
    cols().antenna1().put(rownr, antennaFiller().antennaId());
    cols().antenna2().put(rownr, antennaFiller().antennaId());

    // only bother with this for the first row
    if (rownr == 0) {
 	// there are no separate subarrays here
	cols().arrayId().put(rownr, 0);

	// UVW is always 0.0
	cols().uvw().put(rownr, Vector<Double>(3,0.0));
    }

    uInt rowcount = 0;
    Int row0 = rownr;

    // set the shape of columns stored in TSM - just FLOAT_DATA 
    feedDDFiller().origin();
    for (Int j=0;j<ncorr;j++) {
	// shape just depends on which correlation this is
	IPosition thisShape = feedDDFiller().currCorr().shape();
	rownr = row0 + j;
	for (uInt i=0;i<nrowsleft;i++) {
	    Vector<uInt> rows(nstate);
	    indgen(rows, uInt(rownr), uInt(ncorr));
	    for (uInt k=0;k<uInt(nstate);k++) {
		Int thisRow = rows[k];
		cols().floatData().setShape(thisRow, thisShape);
		modelData().setShape(thisRow, thisShape);
		correctedData().setShape(thisRow, thisShape);
	    }
	    rownr += nrowPerRow;
	}
	feedDDFiller().next();
    }
    rownr = row0;

    Bool outDeleteIt, inDeleteIt;
    Float *outptr;
    const Float *inptr;

    rowcount = 0;
    while (rowcount < nrowsleft) {
	// fill the data from each feed
	Vector<uInt> rows(nstate);
	indgen(rows, uInt(rownr), uInt(ncorr));
	feedDDFiller().origin();

	// get actual shape of the DATA in this row.
	IPosition rowDataShape = spTab_p->currentRow().shape("DATA");
	Int nsamp = rowDataShape(sampAxis);
	Int nfreq = rowDataShape(freqAxis);
	Int rowNstate = rowDataShape(stateAxis);

	for (Int icorr=0;icorr<ncorr;icorr++) {

	    // data shape was already set, use it
	    IPosition dataShape = cols().floatData().shape(rows[0]);

	    Array<Float> outdata(IPosition(3,dataShape(0),dataShape(1),nstate));
	    inptr = (*data).getStorage(inDeleteIt);
	    outptr = outdata.getStorage(outDeleteIt);
	    // move the data from input to output
	    // these things are used to keep track of what goes from where to where
	    uInt fincr, rincr, pincr, fcount, rcount, pcount, outcount;
	    fincr=rincr=pincr=1;
	    fcount=rcount=pcount=outcount=0;
	    if (freqAxis == 0) {
		if (sampAxis == 1) {
		    rincr = nfreq; pincr=nfreq*nsamp;
		} else {
		    rincr = nfreq*rowNstate; pincr=nfreq;
		}
	    } else if (freqAxis == 1) {
		if (sampAxis == 0) {
		    fincr = nsamp; pincr=nsamp*nfreq;
		} else {
		    fincr = rowNstate; rincr=rowNstate*nfreq;
		}
	    } else {
		fincr = rowNstate*nsamp;
		if (sampAxis == 0) {
		    pincr=nsamp;
		} else {
		    rincr=rowNstate;
		}
	    }
	    outcount = pcount = 0;
	    // and which sampler rows do we want
	    Vector<Int> sampRows = feedDDFiller().currCorr().samplerRows();
	    // finally, do the move
	    for (uInt istate=0;istate<uInt(nstate);istate++) {
		fcount = 0;
		for (uInt ifreq=0; ifreq<uInt(nfreq); ifreq++) {
		    for (uInt isampRow=0; isampRow<sampRows.nelements(); isampRow++) {
			rcount = sampRows[isampRow]*rincr;
			outptr[outcount++] = inptr[fcount + rcount + pcount];
		    }
		    fcount += fincr;
		}
		pcount += pincr;
	    }

	    // free the storage
	    (*data).freeStorage(inptr, inDeleteIt);
	    outdata.putStorage(outptr, outDeleteIt);

	    RefRows rrows(rows);
	    // put the data column values
	    cols().floatData().putColumnCells(rrows, outdata);

	    Array<Complex> modData(outdata.shape().getFirst(2), 1.0);
	    Array<Complex> cdata(modData.shape());
	    Array<Float> sigma(outdata.shape().getFirst(1), 1.0);
	    Array<Float> weight(sigma.shape(), 1.0);
	    ArrayIterator<Float> dataIter(outdata,2);

	    for (uInt istate=0;istate<rows.nelements();istate++) {
		// each row corresponds to state istate for corr for icorr
		uInt thisRow = rows[istate];
		uInt indx = ncorr*istate + icorr;
		sigma = sigmas[indx];
		weight = Float(1.0)/(sigma*sigma);
		cols().sigma().put(thisRow, sigma);
		cols().weight().put(thisRow, weight);
		modelData().put(thisRow, modData);
		convertArray(cdata,dataIter.array());
		correctedData().put(thisRow, cdata);
		dataIter.next();
	    }

	    // some way should be found to cache these so that they
	    // can all be written out in a single call
	    // FLAG - all False
	    cols().flag().putColumnCells(rrows, 
					 Array<Bool>(outdata.shape(),False));
	    // FLAG_ROW - all False
	    cols().flagRow().putColumnCells(rrows,
					    Vector<Bool>(rrows.nrows(),False));
	    // FLAG_CATEGORY is required, and I guess this is an appropriate
	    // initial value when there are no categories
	    // I don't see any other way to do this given that there are
	    // no categories yet.
	    //	    for (uInt j=0;j<rows.nelements();j++) {
	    //		cols().flagCategory().put(rows(j),
	    //					  Cube<Bool>(outdata.shape()(0),
	    //						     outdata.shape()(1),
							 //						     0));
	    //	    }
	    // increment the rows
	    rows += uInt(1);
	    feedDDFiller().next();
	}
    
	// it would probably help if all of the following could also
	// be cached as the data column is
	// and put the unhandled fields of the current row in to the 
	// backend subtable
	// the first time we do this, we add a merging of the
	// unhandled keywords sampord with the unhandled fields
	if (rowcount == 0) {
	    typeTab_p->fill(spTab_p->unhandledKeywords());
	    processorFiller().fill(typeTab_p->type(), typeTab_p->typeId());
	    cols().processorId().put(rownr, processorFiller().processorId());
	} 
	// and the exposures
	cols().exposure().
	  putColumnRange(Slicer(IPosition(1,rownr),
				IPosition(1, texp.nelements()),
				IPosition(1,ncorr)), 
			 texp);
	rownr += nrowPerRow;
	typeDataTab_p->fill(spTab_p->unhandledFields(),time(rowcount),tdur,typeTab_p->typeId());

	spTab_p->next();
	utcTime = rowTime(utdateField, utcstartField, tdur);
	// loop through each state and get the LO1 values
	rowcount++;
    }
    // this relies on the TIME column being an IncrStMan column so that only
    // the changes - every nrowPerRow rows need to be written to.
    cols().time().putColumnRange(Slicer(IPosition(1,row0),
					IPosition(1, time.nelements()),
					IPosition(1,nrowPerRow)), 
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
						IPosition(1,nrowPerRow)), 
					 time);
    // now that we know the time, we can fill the POINTING subtable
    pointingFiller().fill(ms(), antennaFiller().antennaId(), time, tdur);

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


void GBTSPFiller::attachColumns() 
{
    ifId_p.attach(ms(), "NRAO_GBT_IF_ID");
    gbtStateId_p.attach(ms(), "NRAO_GBT_STATE_ID");
    samplerId_p.attach(ms(), "NRAO_GBT_SAMPLER_ID");
}

MVTime GBTSPFiller::rowTime(const RORecordFieldPtr<Int> &utdateField,
			    const RORecordFieldPtr<Double> &utcstartField,
			    Double tdur) {
    // Use the columns to get the values at this row
    // The time stamp is the mid point of the scacn which has total length of tdur
    // seconds, starts at utcstart (also in seconds) on utdate (in days)
    Quantum<Double> utdate(*utdateField, "d");
    Quantum<Double> utcstart(*utcstartField + tdur/2, "s");
    MVTime time(utdate);
    time = time + MVTime(utcstart);
    return time;
}

void GBTSPFiller::handleColumns()
{
    typeSampTab_p->handleColumn("OBSFREQ");
    typeSampTab_p->handleColumn("FREQRES");
    typeSampTab_p->handleColumn("BANDWD");
    typeSampTab_p->handleColumn("TCAL");
}
