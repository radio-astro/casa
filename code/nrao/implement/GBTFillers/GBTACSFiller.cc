//# GBTACSFiller.cc: A GBTBackendFiller for ACS data
//# Copyright (C) 2001,2002,2003,2004
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

#include <nrao/GBTFillers/GBTACSFiller.h>
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
#include <casa/OS/RegularFile.h>
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

#include <nrao/FITS/GBTMultiACSTable.h>
#include <nrao/FITS/GBTStateTable.h>

#include <casa/sstream.h>
#include <casa/namespace.h>

#include <iostream>
#include <fstream>

GBTACSFiller::GBTACSFiller(const String &msName, 
			   const Regex &object)
    : spTab_p(0), msName_p(msName), object_p(object),
      typeDataTab_p(0), typeTab_p(0), typeStateTab_p(0), 
      typeMasterStateTab_p(0), typeSampTab_p(0),
      gbtPointingTab_p(0), gbtFocusTab_p(0), ifFiller_p(0), 
      rxCalFiller_p(0), GOFiller_p(0)
{
    // openMS returns True if msName_p exists and can be opened
    if (openMS(msName_p)) {
	// and the associated GBT_type_* tables
	typeDataTab_p = new GBTMSTypeDataFiller(ms().tableName() + \
						"/GBT_ACS_DATA");
	typeTab_p = new GBTMSTypeFiller(ms().tableName() + 
					"/GBT_ACS");
	typeStateTab_p = new GBTMSTypeStateFiller(ms().tableName() + 
						  "/GBT_ACS_STATE");
	typeMasterStateTab_p = new GBTMSTypeStateFiller(ms().tableName() + 
							"/GBT_ACS_MASTERSTATE");
	typeSampTab_p = new GBTMSTypeSampFiller(ms().tableName() + 
					      "/GBT_ACS_SAMPLER");
	AlwaysAssert(typeDataTab_p && typeTab_p && typeStateTab_p && 
		     typeMasterStateTab_p && typeSampTab_p, AipsError);

	// mark columns in the SAMPLER table which are handled elsewhere
	handleColumns();

	// attach some standard columns
	attachColumns();

	LogIO os(LogOrigin("GBTACSFiller",
	   "GBTACSFiller(const String &msName, const Regex &object)"));
	os << LogIO::NORMAL << WHERE
	   << "Existing MS opened" << LogIO::POST;
    }
}

GBTACSFiller::~GBTACSFiller() 
{
    delete spTab_p;
    spTab_p = 0;

    delete typeDataTab_p;
    typeDataTab_p = 0;
    delete typeTab_p;
    typeTab_p = 0;
    delete typeStateTab_p;
    typeStateTab_p = 0;
    delete typeMasterStateTab_p;
    typeMasterStateTab_p = 0;
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

Bool GBTACSFiller::fill(const Vector<String> &backendFiles,
			const Block<String> &dapFiles,
			const String &ifManagerFile,
			const Block<String> &rxCalInfoFiles,
			const GBTGOFile &GOFile,
			const String &antennaFile,
			const GBTStateTable &masterState,
			const GBTLO1File &LO1A,
			const GBTLO1File &LO1B,
			const GBTMSFillOptions &fillOptions)
{
    LogIO os(LogOrigin("GBTACSFiller",
		       "fill(const String &backendFiles, const Block<String> &dapFiles)"));
    Int rownr = 0;
    if (spTab_p) {
	// reopen it
	spTab_p->reopen(backendFiles);
    } else {
	String fixLagsLog("");
	if (fillOptions.fixbadlags()) {
	    // prepare output log file
	    fixLagsLog = msName_p+".fixed_lags";
	    // if the MS does not exist and this file already exists, remove it first
	    if (!File(msName_p).exists()) {
		RegularFile logFile(fixLagsLog);
		if (logFile.exists()) logFile.remove();
	    }
	    ofstream badLagsLog(fixLagsLog.c_str(),ofstream::app);
	    if (!badLagsLog) { // open failed
		os << LogIO::WARN << WHERE
		   << "Cannot open fixed logs file for output." << LogIO::POST;
		os << LogIO::WARN
		   << "Fixed lags log messages will appear within this message stream instead." 
		   << LogIO::POST;
		fixLagsLog = "";
	    } else {
	        if (Int(badLagsLog.tellp())==0) { 
		    badLagsLog << 
                        "# timestamp scan sampler int phaseid channels" 
                           << endl;
                }
	    }
	}
	spTab_p = new GBTMultiACSTable(backendFiles,
				       fillOptions.vanVleckCorr(),
				       fillOptions.smoothing(),
				       fillOptions.vvSize(),
				       fillOptions.useBias(),
				       fillOptions.dcbias(),
				       fillOptions.minbiasfactor(),
				       fillOptions.fixbadlags(),
				       fixLagsLog,
				       fillOptions.sigmaFactor(),
				       fillOptions.spikeStart());
	AlwaysAssert(spTab_p, AipsError);
    }
    if (!spTab_p->isValid()) {
	os << LogIO::WARN << WHERE
	   << "Invalid backend FITS files: " << backendFiles << 
	    " - skipping" << LogIO::POST;
	return False;
    }
	
    // new or old style
    Bool isNew = True;
    if (spTab_p->port().nrow() == 0) isNew = False;

    // mark the DATA column as being handled
    spTab_p->handleField("DATA");
    // for now we return if this is an empty table, filling nothing
    // but we'll return True for now so as not to signal an error
    if (spTab_p->nrow() == 0) {
	os << LogIO::NORMAL << WHERE
	   << "ACS scan " << spTab_p->keywords().asInt("SCAN") << " contains no data" << endl;
	return True;
    }

    // sanity check
    if (spTab_p->keywords().fieldNumber("INSTRUME") < 0 ||
	!spTab_p->keywords().asString("INSTRUME").
	matches(Regex("^Spectrometer$"))) {
	os << LogIO::SEVERE << WHERE;
	if (spTab_p->keywords().fieldNumber("INSTRUME") >= 0) {
	    os << "Unexpected INSTRUME value : "
	       << spTab_p->keywords().asString("INSTRUME") << LogIO::POST;
	} else {
	    os << "No INSTRUME keyword found in data file." << LogIO::POST;
	}
	return False;
    }
    // mark it as handled
    spTab_p->handleKeyword("INSTRUME");

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
	project = spTab_p->keywords().asString("PROJID");
    }
    // either way, mark this one as handled
    spTab_p->handleKeyword("PROJID");

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

    // we use TIME-MID to get the midpoint times, ignore this
    spTab_p->handleKeyword("DATE-OBS");
    
    Double timeFieldOffset = 0.0;
    Double tdur = 0.0;
    Double swper = 0.0;
    Bool acsIsMaster = spTab_p->state().master() == "Spectrometer" && 
	spTab_p->state().isAttached();
    if (acsIsMaster || !masterState.isAttached()) {
	swper = spTab_p->state().swperiod();
    } else {
	if (masterState.isAttached()) {
	    swper = masterState.swperiod();
	}
    }
    Record kw = spTab_p->keywords();
    Bool useDMJD = True;
    if (isNew) {
	if (kw.fieldNumber("SWPERINT") >= 0) {
	    tdur = kw.asDouble("SWPERINT") * swper;
	    spTab_p->handleKeyword("SWPERINT");
	}
	timeFieldOffset = tdur/2.0;
    } else {
	if (kw.fieldNumber("INTSWPER") >= 0) {
	    tdur = kw.asDouble("INTSWPER") * swper;
	    spTab_p->handleKeyword("INTSWPER");
	}
	useDMJD = False;
	// Use, instead timeMid() : dmjd at mid-point of current integration
    }

    // these are redundant in spTab_p, marked as handled here
    spTab_p->handleKeyword("ORIGIN");
    spTab_p->handleKeyword("TELESCOP");
    spTab_p->handleKeyword("TIMESYS");

    // decipher the row shape et al.
    Int stateAxis, sampAxis, freqAxis;
    stateAxis = sampAxis = freqAxis = -1;
    uInt nTrueState = 0;
    if (isNew) {
	// axes described by TDESC3 for DATA and TDESC2 for INTEGRAT
	String tdesc = kw.asString("TDESC3");
	Vector<String> axisType = stringToVector(tdesc);
	for (uInt i=0; i<axisType.nelements();i++) {
	    String thisType = axisType[i];
	    if (thisType == "ACT_STATE") {
		stateAxis = i;
	    } else if (thisType == "SAMPLER") {
		sampAxis = i;
	    } else if (thisType == "LAG") {
		freqAxis = i;
	    } else {
		throw(AipsError("Unrecognized DATA axis in ACS FITS file"));
	    }
	}
	spTab_p->handleKeyword("TDESC3");
	String tintdesc = kw.asString("TDESC2");
    } else {
	Vector<String> ctypes(3);
	ctypes[0] = "CTYPE1";
	ctypes[1] = "CTYPE2";
	ctypes[2] = "CTYPE3";
	// axes described by CTYPE keywords
	for (uInt i=0;i<3;i++) {
	    String thisType = kw.asString(ctypes[i]);
	    if (thisType == "ACT_STATE") {
		stateAxis = i;
	    } else if (thisType == "SAMPLER") {
		sampAxis = i;
	    } else if (thisType == "LAGS") {
		freqAxis = i;
	    } else {
		throw(AipsError("Unrecognized DATA axis in ACS FITS file"));
	    }
	    // we've handled this keyword
	    spTab_p->handleKeyword(ctypes[i]);
	}
    }
    AlwaysAssert(stateAxis != -1 && sampAxis != -1 && freqAxis != -1, AipsError);

    MVTime startTime(0.0);

    if (!hasMS()) {
	// this can only happen if the MS needs to be created
	TableDesc mainTD(MS::requiredTableDesc());
	// add the DATA column
	MS::addColumnToDesc(mainTD, MS::FLOAT_DATA, 2);
	// add the VIDEO_POINT column
	MS::addColumnToDesc(mainTD, MS::VIDEO_POINT, 1);
	// add the LAG_DATA column if asked to do so
	if (fillOptions.fillLags()) {
	    MS::addColumnToDesc(mainTD, MS::LAG_DATA, 2);
	}
	// add in the non-standard index columns
	mainTD.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_STATE_ID"));
	mainTD.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_IF_ID"));
	mainTD.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_SAMPLER_ID"));
	mainTD.addColumn(ArrayColumnDesc<Int> ("NRAO_GBT_MASTERSTATE_IDS", 1));
	mainTD.addColumn(ScalarColumnDesc<String> ("GBT_SRFEED_BANK"));
	mainTD.addColumn(ScalarColumnDesc<String> ("GBT_BANK"));
	// define the hyper column for the data-shaped columns
	Vector<String> dmCols(2);
	dmCols(0) = MS::columnName(MS::FLOAT_DATA);
	dmCols(1) = MS::columnName(MS::FLAG);
	mainTD.defineHypercolumn("HYPERDATA", 3, dmCols,
				 stringToVector(",,"));
	IPosition defaultTileShape(3, 4, 1034, 32);
	TiledShapeStMan stman("HYPERDATA", defaultTileShape);

	// second hypercolumn for the lags, if necessary
	if (!fillOptions.fillLags()) {
	    AlwaysAssert(GBTBackendFiller::createMS(msName_p, mainTD, 
						    stman, dmCols, defaultTileShape,
						    fillOptions, 0, True),
			 AipsError);
 	} else {
 	    // needs a second hypercolumn for the LAG_DATA
	    Vector<String> dm2Cols(1);
	    dm2Cols(0) = MS::columnName(MS::LAG_DATA);
	    mainTD.defineHypercolumn("LAGSHYPERDATA", 3, dm2Cols,
				     stringToVector(",,"));
 	    TiledShapeStMan lagssm("LAGSHYPERDATA", defaultTileShape);
   	    AlwaysAssert(GBTBackendFiller::createMS(msName_p, mainTD,
   						    stman, dmCols, defaultTileShape, 
						    fillOptions, lagssm, dm2Cols, 
						    0, True),
   			 AipsError);
   	}

	os << LogIO::NORMAL << WHERE
	   << "Created a new MS" << LogIO::POST;
	// now add in the associated GBT_type_* tables
	typeDataTab_p = new GBTMSTypeDataFiller(ms(), "ACS");
	typeTab_p = new GBTMSTypeFiller(ms(), "ACS");
	typeStateTab_p = new GBTMSTypeStateFiller(ms(), "ACS");
	typeMasterStateTab_p = new GBTMSTypeStateFiller(ms(), "ACS", True);
	typeSampTab_p = new GBTMSTypeSampFiller(ms(), "ACS");
	AlwaysAssert(typeDataTab_p && typeTab_p && typeStateTab_p && 
		     typeMasterStateTab_p && typeSampTab_p, AipsError);

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

    if (fillOptions.fillLags() && cols().lagData().isNull()) {
	if (!ms().isColumn(MS::LAG_DATA)) {
	    // attach the lags column
	    TableDesc td;
	    MS::addColumnToDesc(td, MS::LAG_DATA, 2);
	    // needs a second hypercolumn for the LAG_DATA
	    td.defineHypercolumn("LAGSHYPERDATA", 3, 
				 stringToVector(MS::columnName(MS::LAG_DATA)),
				 stringToVector(",,"));
	    TiledShapeStMan lagsSM("LAGSHYPERDATA", IPosition(3,4,1024,8));
	    ms().addColumn(td[MS::columnName(MS::LAG_DATA)], lagsSM);
	}
	// and reattach it
	reattach();
    }

    if (!ifFiller_p) {
	ifFiller_p = new GBTIFFiller(ms(), "Spectrometer");
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
    
    Double thisTime = timeFieldOffset;
    if (useDMJD) {
	thisTime += spTab_p->dmjd()*C::day;
    } else {
	thisTime += spTab_p->timeMid()*C::day;
    }
    if (rownr > 0) {
	Double lastTime = cols().time().asdouble(rownr-1);
	Int nSPTabRows = (spTab_p->nrow() - spTab_p->rownr());
	Double rowTime = thisTime-tdur/2.0; // start time of this row
	while (((rowTime-lastTime) <= 0 || nearAbs(rowTime,lastTime)) 
	       && nSPTabRows > 0) {
	    nSPTabRows--;
	    spTab_p->next();
	    thisTime = timeFieldOffset;
	    if (useDMJD) {
		thisTime += spTab_p->dmjd()*C::day;
	    } else {
		thisTime += spTab_p->timeMid()*C::day;
	    }
	    rowTime = thisTime-tdur/2.0; // start time of this row
	}
	startTime = MVTime(lastTime/C::day);

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
    ostr << "Filling ACS scan " << scanNr;
    if (spTab_p->banks().length() > 1) {
	ostr << " banks ";
    } else {
	ostr << " bank ";
    }
    ostr << spTab_p->banks();
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
		    gbtFocusTab_p = new Table(tabName,tableLock(),
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
    // converted to J2000.  initialize it with the antenna position
    MeasFrame frame(antennaFile_p.position());
      
    // fill the feed, pol, sw, and data_desc subtables
    feedDDFiller().fill(*spTab_p, *ifFiller_p, antennaFile_p,
			LO1A, LO1B, GOFile.swtchsig(), False, False);


    // calculate nrows given how many rows are left in the table
    // spTab_p->rownr() may be non-zero if this file has been reopened
    Int ncorr = feedDDFiller().ncorr();
    Int nActState = feedDDFiller().nstates();

    // master state vector - default is a null vector
    // uses ISM so only need to put this in the first row being filled
    // For the ACS, the state will only be okay IF the ACS
    // is the master.
    Bool canNotReorder = False;
    if (!acsIsMaster) {
	canNotReorder = True;
	os << LogIO::SEVERE << WHERE
	   << "The ACS is not the master of the switching states for this scan."
	   << LogIO::POST;
    }
    // digest the ACT_STATE
    // State map gives the row in the STATE table corresponding the
    // each row in the ACT_STATE table.
    Vector<Int> stateMap(nActState);
    Bool mixedSignals, multipleCal, multipleSigref;
    mixedSignals = multipleCal = multipleSigref = False;
    // If one IF is freq switched, they all must be
    feedDDFiller().origin();
    Bool isFreqSwitched = anyNE(feedDDFiller().currIF().offsets(), 0.0);
    if (!acsIsMaster || 
	!spTab_p->state().digestActState(spTab_p->actState(), stateMap, 
					 mixedSignals, multipleCal, multipleSigref,
					 !spTab_p->switchOK())) {
	// something went wrong
	canNotReorder = True;
	if (acsIsMaster) {
	    os << LogIO::SEVERE << WHERE;
	    if (multipleCal || multipleSigref) {
		os << "There is a mixture of internal and external signals in this scan."
		   << endl;
	    } else {
		os << "There is an ambiguity matching the ACT_STATE row to rows in the STATE table."
		   << endl;
	    }
	    if (isFreqSwitched) {
		os << "The frequency offsets in LO1 for this frequency switched data can not."
		   << "\nbe correctly assigned to the appropriate ACS state."
		   << "\nA zero offset will be assumed for all ACS states in this scan."
		   << endl;
	    }
	    os << LogIO::POST;
	}
    } 
    if (acsIsMaster && mixedSignals) {
	os << LogIO::WARN << WHERE
	   << "There is an unexpected mixture of internal and external signals in this scan."
	   << "\nThe filler is able to reorder this data to the temporal order implied in the STATE"
	   << "\ntable, however that resorting may not be correct."
	   << "\nUse caution when interpreting this data."
	   << LogIO::POST;
    }
    if (canNotReorder) {
	// set the state map to their natural order - i.e. same order as in ACT_STATE
	indgen(stateMap);
	os << LogIO::SEVERE << WHERE;
	os << "The data can not be reordered to match the temporal order of the STATE table."
	   << "\nThe STATE information associated with this scan may be wrong."
	   << "\nAutomatic processing of this scan may fail or produce wrong results."
	   << LogIO::POST;
    }

    // count the number of true states
    for (uInt i=0;i<stateMap.nelements();i++) {
	if (stateMap(i) >= 0) nTrueState++;
    }

    uInt nrowPerRow = ncorr * nTrueState;
    Int nrows = nrowsleft * nrowPerRow;

    // extend the MS by the required number of rows
    if (nrows > 0) {
	ms().addRow(nrows);
    } 

    // fill the state and sampler sub tables
    typeSampTab_p->fill(spTab_p->sampler());
    typeStateTab_p->fill(spTab_p->actState());
    if (acsIsMaster || !masterState.isAttached()) {
	typeMasterStateTab_p->fill(spTab_p->state().table());
	masterStateIds_p.put(rownr,typeMasterStateTab_p->stateIds());
    } else if (masterState.isAttached()) {
	typeMasterStateTab_p->fill(masterState.table());
    } else {
	masterStateIds_p.put(rownr, Vector<Int>());
    }
   
    // the vector of TIME, one for each row
    // of the backend table, do this in possibly smaller chunks?
    // these values are filled in as the data is processed
    Vector<Double> time(nrowsleft);

    // fill the MS subtables
    antennaFiller().fill(antennaFile_p);

    // WEATHER
    weatherFiller().fill();

    // fill the state table - these GO functions are OK to use even
    // if goIsOK is False
    if (acsIsMaster || !masterState.isAttached()) {
	stateFiller().fill(spTab_p->state().sigref(),
			   spTab_p->state().cal(),
			   GOFile.procname(), 
			   GOFile.procsize(), GOFile.procseqn(),
			   GOFile.swstate(), GOFile.swtchsig());
    } else {
	stateFiller().fill(masterState.sigref(), masterState.cal(), 
			   GOFile.procname(), 
			   GOFile.procsize(), GOFile.procseqn(),
			   GOFile.swstate(), GOFile.swtchsig());
    } 

    // Remember the spwids for each corr and state
    // These two are filled in to the MAIN table in the appropriate rows
    // in order - all corrs for each state in turn
    // So, index k in each vector = ncorr*(state) + corr
    // FEED1 == FEED2
    Vector<Int> feedIds(nrowPerRow);
    Vector<String> srfeedBank(nrowPerRow);
    Vector<String> feedBank(nrowPerRow);
    // DATA_DESCRIPTION_ID
    Vector<Int> ddIds(nrowPerRow);
    // IDs for each state - but filled for each corr as well
    Vector<Int> stateIds(nrowPerRow);
    Vector<Int> stateSlice(stateIds(Slice(0,nTrueState,ncorr)));
    // IDs for each GBT state - but filled for each corr as well
    Vector<Int> gbtStateIds(nrowPerRow);
    Vector<Int> gbtStateSlice(gbtStateIds(Slice(0,nTrueState,ncorr)));
    // Next the STATE ids and GBT STATE Ids
    // generate the first sequence
    Vector<Int> theseStateIds = stateFiller().stateIds();
    // which stateSlice elements have SIG=True
    Vector<Bool> sigs = stateFiller().sigs();
    Vector<Int> gbtIds(typeStateTab_p->stateIds());

    if (theseStateIds.nelements() != nTrueState) {
	// I believe that this can only happen if STATE is larger than ACT_STATE
	// assign each element by hand.
	Int stateCount = 0;
	Vector<Int> usedStateIds(nTrueState);
	Vector<Int> newStateMap(stateMap.nelements());
	Vector<Bool> newSigs(nTrueState);
	Vector<Int> newGbtIds(nTrueState);
	Vector<Bool> found(stateMap.nelements(), False);
	for (Int i=0;i<Int(theseStateIds.nelements());i++) {
	    for (Int j=0;j<Int(stateMap.nelements());j++) {
		if (!found[j] && stateMap[j] == i) {
		    usedStateIds[stateCount] = theseStateIds[i];
		    newStateMap[j] = stateCount;
		    newSigs[stateCount] = sigs[i];
		    newGbtIds[stateCount] = gbtIds[i];
		    stateCount++;
		    found[j] = True;
		}
	    }
	}
	theseStateIds.resize(nTrueState);
	theseStateIds = usedStateIds;
	sigs.resize(nTrueState);
	sigs = newSigs;
	stateMap = newStateMap;
	gbtIds.resize(nTrueState);
	gbtIds = newGbtIds;
    } 
    stateSlice = theseStateIds;
    gbtStateSlice = gbtIds;

    // SIGMA for each row in an integration
    Vector<Float> sigmas(nrowPerRow);
    // set the initial time in the frame (defaults to UTC - i.e. okay)
    frame.set(MEpoch(Quantity(thisTime,"s")));

    // the LO1 state is OK if ACS is the master or if there is
    // no frequency switching. Check the each feed.  Only emit FS not master
    // error message once.

    // remember the unique spectral window IDs for each unique feed id
    // Encode the unique pair as spwid*1000+feedid - hopefully there won't be
    // more than 1000 feed in a scan.  This is necessary because when
    // we fill he SYSCAL table - its not one row per row of the main table
    // but rather, for each time, one row for each unique spectral window and
    // feed id.  Since the set of unique pairs doesn't change with time for
    // a given scan, it makes sense to determine what that set is up front.
    // We also only prefill the table for the unique pairs.
    SimpleOrderedMap<Int, Int> uniqueFeedSpws(-1);

    // k2 for sigma calculation below.  Depends only on the smoothing mode.
    // == 2 for hanning, 1.21 for uniform. Need to work out what it is for
    // other smoothings - go with 1.21 for now. The only case this will
    // be invalid for currently is Hamming.
    Double k2 = 1.21;
    if (fillOptions.smoothing() == GBTACSTable::Hanning) k2 = 2.0;

    Bool beamSwitched = feedDDFiller().hasBeamSwitched();

    feedDDFiller().origin();
    for (Int i=0;i<ncorr;i++) {
	// state IDs
	if (i != 0) {
	    stateIds(Slice(i,nTrueState,ncorr)) = stateSlice;
	    gbtStateIds(Slice(i,nTrueState,ncorr)) = gbtStateSlice;
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

 	String bank = feedDDFiller().currCorr().bank();
	// GBT_SRFEED_BANK is empty if there is no switching going on
	String srbank = feedDDFiller().currCorr().srbank();
	if (srbank.length() == 0) srbank = bank;
	
	// assumes receiver name is the same for all ifrows here
	String rxname;
	if (ifrow >= 0 && !ifFiller_p->receiver().isNull()) {
	    rxname = ifFiller_p->receiver()(ifrow);
	}

	// state dependent part - skip the non-true states
	uInt stateCount = 0;
 	for (uInt j=0;j<uInt(nTrueState);j++) {
	    Int thisState = stateMap[j];
	    if (thisState >= 0) {
		const GBTFeed *trueFeed = thisFeed;
		if (beamSwitched && !sigs[stateCount] && 
		    thisFeed->srFeedPtr()) {
		    trueFeed = thisFeed->srFeedPtr();
		}
		uInt indx = ncorr*stateCount + i;
		Int spwid = feedDDFiller().currIF().spwId(j);
		if (trueFeed->feedId() >=1000) {
		    os << LogIO::SEVERE << WHERE;
		    os << "Unexpected FEED_ID >= 1000.  This scan can not be filled."
		       << LogIO::POST;
		    return False;
		}
		
		ddIds[indx] = feedDDFiller().currCorr().dataDescId(j);
		feedIds[indx] = trueFeed->feedId();
		srfeedBank[indx] = srbank;
		feedBank[indx] = bank;

		// SOURCE 
		if (goIsOkay) {
		    // use this rest frequency even if the scan number is bad, no where else to get it
		    sourceFiller().fill(Vector<Int>(1,spwid), object,
					GOFile.restfrq(), GOFile.velocity().getValue());
		} else {
		    sourceFiller().fill(Vector<Int>(1,spwid), object);
		}
		// SIGMA - assumes TSYS == 1.0, will be scaled later in calibration step
		//       - and uses the exposure for this state as texp - can also be
		//         scaled during the calibration step.
		//   This is scaled by k1 below - which is not known until the
		//   data is fetched - may vary by sampler.  Will also
		//   scale by 1.0/sqrt(texp) for this feed later.
		sigmas[indx] = 1.0 / sqrt(k2 * abs(feedDDFiller().currIF().deltaFreq()));

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
		stateCount++;
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
	gbtStateId_p.putColumnRange(rowSlice, stateIds);
	gbtSRBank_p.putColumnRange(rowSlice, srfeedBank);
	gbtBank_p.putColumnRange(rowSlice, feedBank);
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
    observationFiller().fill(project, observer,
			     MVTime(Quantity(thisTime,"s")), tdur*nrowsleft,
			     antennaFile_p.telescop());

    // History
    // need a new row for each new observation_id
    if (obsId != observationFiller().observationId()) {
 	historyFiller().fill(observationFiller().observationId(), 
			     "Filling ACS data", "NORMAL",
			     "GBTACSFiller", MVTime(0.0));
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

    Int row0 = rownr;

    // set the shape of columns stored in TSM - just FLOAT_DATA 
    for (uInt i=0;i<nrowsleft;i++) {
	feedDDFiller().origin();
	for (Int k=0;k<ncorr;k++) {
	    // shape is independent of state
	    IPosition thisShape = feedDDFiller().currCorr().shape();
	    uInt thisrow = rownr + i*ncorr*nTrueState + k;
	    for (uInt j=0;j<uInt(nTrueState);j++) {
		cols().floatData().setShape(thisrow, thisShape);
		if (fillOptions.fillLags()) {
		    cols().lagData().setShape(thisrow, thisShape);
		}
		modelData().setShape(thisrow, thisShape);
		correctedData().setShape(thisrow, thisShape);
		thisrow += ncorr;
	    }
	    feedDDFiller().next();
	}
    }

    uInt rowcount = 0;
    while (rowcount < nrowsleft) {
	thisTime = timeFieldOffset;
	if (useDMJD) {
	    thisTime += spTab_p->dmjd()*C::day;
	} else {
	    thisTime += spTab_p->timeMid()*C::day;
	}
	
	Vector<uInt> rows(nTrueState);
	Vector<Bool> flagRows(nTrueState, False);
	indgen(rows, uInt(rownr), uInt(ncorr));
	Matrix<Double> texp(ncorr, nTrueState);
	
	feedDDFiller().origin();
	for (Int icorr=0;icorr<ncorr;icorr++) {

	    IPosition dataShape = cols().floatData().shape(rows[0]);
	    Array<Float> outdata(IPosition(3,dataShape(0),dataShape(1),
					   nTrueState));
	    Array<Bool> flags(outdata.shape(),False);
	    Array<Complex> outVideo(IPosition(2,dataShape(0), nTrueState));
	    IPosition outEnd = outdata.endPosition();
	    IPosition outStart = IPosition(outEnd.nelements(),0);
	    IPosition outVideoEnd = outVideo.endPosition();
	    IPosition outVideoStart = IPosition(outVideoEnd.nelements(), 0);
	    
	    Array<Complex> outlags;
	    
	    if (fillOptions.fillLags()) {
		outlags.resize(IPosition(3,dataShape(0),dataShape(1),
					 nTrueState));
	    }
	    // and which sampler rows do we want
	    Vector<Int> sampRows = feedDDFiller().currCorr().samplerRows();
	    Float k1 = 1.235;
	    // both samplers here have the same level
	    if (spTab_p->nlevels(sampRows[0]) == 9) k1 = 1.032;
	    for (uInt isamp=0;isamp<sampRows.nelements();isamp++) {
		// get the raw data as necessary
		uInt thisSamp = sampRows[isamp];
		outStart[0] = outEnd[0] = isamp;
		outVideoStart[0] = outVideoEnd[0] = isamp;
		Matrix<Float> lagData;
		if (fillOptions.fillLags()) {
		    lagData = spTab_p->rawdata(thisSamp);
		}
		// the converted data - it is a reference.
		const Matrix<Float> data(spTab_p->data(thisSamp));
		const Vector<Float> zeroChans(spTab_p->zeroChannel(thisSamp));
		// and the associated zero-channel values - one for each state
		Vector<Bool> badData(spTab_p->badData(thisSamp));

		// loop through states - use the stateMap vector values
		for (uInt istate=0;istate<nTrueState;istate++) {
		    Int thisState = stateMap(istate);
		    // negative states are ignored
		    if (thisState >= 0) {
			outStart[2] = outEnd[2] = thisState;
			outdata(outStart, outEnd).nonDegenerate() = data.column(istate);

			if (fillOptions.fillLags()) {
			    Vector<Complex> clag(feedDDFiller().currIF().nchan());
			    convertArray(clag, lagData.column(istate));
			    outlags(outStart, outEnd).nonDegenerate() = clag;
			}
			outVideoStart[1] = outVideoEnd[1] = thisState;
			outVideo(outVideoStart, outVideoEnd).nonDegenerate() = zeroChans[istate];
			if (isamp==0) {
			    // need to do this only once per correlation
			    Double thisExp = 0.0;
			    for (uInt thisRow=0;thisRow<sampRows.nelements();thisRow++) {
				thisExp = max(thisExp, spTab_p->integrat(sampRows[thisRow],istate));
			    }
			    texp(icorr,thisState) = thisExp;
			}
			if (badData[istate]) {
			    flags(outStart, outEnd).nonDegenerate() = True;
			}
		    }
		}
	    }
	    
	    RefRows rrows(rows);
	    // put the data column values
	    cols().floatData().putColumnCells(rrows, outdata);
	    if (fillOptions.fillLags()) {
		cols().lagData().putColumnCells(rrows, outlags);
	    }
	    cols().videoPoint().putColumnCells(rrows, outVideo);
	    
	    Array<Complex> modData(outdata.shape().getFirst(2),1.0);
	    Array<Complex> cdata(modData.shape());
	    Array<Float> sigma(outdata.shape().getFirst(1), 1.0);
	    Array<Float> weight(sigma.shape(), 1.0);
	    ArrayIterator<Float> dataIter(outdata,2);
	    
	    for (uInt istate=0;istate<rows.nelements();istate++) {
		// each row corresponds to state istate for ncorr icorr
		uInt indx = ncorr*istate + icorr;
		uInt thisRow = rows[istate];
		sigma = k1*sigmas[indx]/sqrt(texp(icorr, istate));
		weight = Float(1.0)/(sigma*sigma);
		cols().sigma().put(thisRow, sigma);
		cols().weight().put(thisRow, weight);
		modelData().put(thisRow, modData);
		convertArray(cdata,dataIter.array());
		correctedData().put(thisRow, cdata);
		dataIter.next();
	    }
	    
	    // FLAG - all False
	    cols().flag().putColumnCells(rrows, flags);

	    // FLAG_ROW
	    cols().flagRow().putColumnCells(rrows, flagRows);

	    // increment the rows
	    rows += uInt(1);
	    feedDDFiller().next();
	}
	// it would probably help if all of the following could also
	// be cached as the data column is
	// and put the unhandled fields of the current row in to the 
	// backend subtable
	// the first time we do this, we add a merging of the
	// unhandled keywords record with the unhandled fields
	if (rowcount == 0) {
	    typeTab_p->fill(spTab_p->unhandledKeywords());
	    processorFiller().fill(typeTab_p->type(), typeTab_p->typeId());
	    cols().processorId().put(rownr, processorFiller().processorId());
	} 
	// and the exposures
	
	cols().exposure().
	    putColumnRange(Slicer(IPosition(1,rownr),
				  IPosition(1, texp.nelements()),
				  IPosition(1,1)), 
			   texp.reform(IPosition(1,texp.nelements())));
	rownr += nrowPerRow;
	typeDataTab_p->fill(spTab_p->unhandledFields(),thisTime,tdur,typeTab_p->typeId());
	
	// remember the time - convert from days to seconds
	time(rowcount) = thisTime;
	spTab_p->next();
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
    if (acsIsMaster || !masterState.isAttached()) typeMasterStateTab_p->flush();
    typeSampTab_p->flush();
    
    // these may not exist
    if (gbtPointingTab_p) gbtPointingTab_p->flush();
    if (gbtFocusTab_p) gbtFocusTab_p->flush();
    if (ifFiller_p) ifFiller_p->flush();
    if (rxCalFiller_p) rxCalFiller_p->flush();
    if (GOFiller_p) GOFiller_p->flush();
    
    return True;
}


void GBTACSFiller::attachColumns() 
{
    ifId_p.attach(ms(), "NRAO_GBT_IF_ID");
    gbtStateId_p.attach(ms(), "NRAO_GBT_STATE_ID");
    samplerId_p.attach(ms(), "NRAO_GBT_SAMPLER_ID");
    masterStateIds_p.attach(ms(), "NRAO_GBT_MASTERSTATE_IDS");
    gbtSRBank_p.attach(ms(), "GBT_SRFEED_BANK");
    gbtBank_p.attach(ms(), "GBT_BANK");
}

void GBTACSFiller::handleColumns()
{
    typeSampTab_p->handleColumn("TCAL");
}
