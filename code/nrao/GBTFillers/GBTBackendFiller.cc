//# GBTBackendFiller.cc: GBTBackendFiller is a base class for GBT backend fillers
//# Copyright (C) 1999,2000,2001,2002,2003
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

#include <nrao/GBTFillers/GBTBackendFiller.h>
#include <nrao/GBTFillers/GBTAntennaDAPFiller.h>
#include <nrao/GBTFillers/GBTLO1DAPFiller.h>
#include <nrao/GBTFillers/GBTMSFillOptions.h>

#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicMath/Math.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSFeedColumns.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <ms/MeasurementSets/MSTableImpl.h>
#include <measures/Measures/MPosition.h>
#include <casa/OS/File.h>
#include <tables/DataMan/IncrementalStMan.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/DataMan/CompressComplex.h>
#include <tables/DataMan/CompressFloat.h>
#include <tables/DataMan/DataManager.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/DataMan/StandardStMan.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/DataMan/TiledShapeStMan.h>
#include <tables/DataMan/TiledStManAccessor.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>
#include <casa/namespace.h>

GBTBackendFiller::GBTBackendFiller()
    : ms_p(0), hasMS_p(False), mscols_p(0), deviceMap_p(-1),
      antennaDAPFiller_p(-1), lo1DAPFiller_p(-1), startTime_p(0.0)
{
    // make an empty MS to start with
    ms_p = new MeasurementSet;
    AlwaysAssert(ms_p, AipsError);
}

GBTBackendFiller::~GBTBackendFiller()
{ 
    delete mscols_p;
    mscols_p = 0;
    delete ms_p;
    ms_p = 0;
    hasMS_p = False;
    for (uInt i=0;i<dapFillers_p.nelements();i++) {
	if (dapFillers_p[i]) {
 	    if (Int(i) == antennaDAPFiller_p) {
 		delete dynamic_cast<GBTAntennaDAPFiller *>(dapFillers_p[i]);
	    } else if (Int(i) == lo1DAPFiller_p) {
		delete dynamic_cast<GBTLO1DAPFiller *>(dapFillers_p[i]);
 	    } else {
 		delete dapFillers_p[i];
 	    }
	    dapFillers_p[i] = 0;
	}
    }
}

Bool GBTBackendFiller::openMS(const String &msName, Bool useFeedDDFiller)
{
    Bool result = False;
    // see if a file of that name already exists
    File msFile(msName);
    if (msFile.exists() && msFile.isWritable()) {
	delete ms_p;
	ms_p = 0;
	ms_p = new MeasurementSet(msFile.path().absoluteName(),
				  tableLock(),
				  Table::Update);
	AlwaysAssert(ms_p, AipsError);
	reattach();
	if (ms_p->nrow()) {
	    Double lastTime = mscols_p->time().asdouble(ms_p->nrow()-1);
	    // that should be MJD seconds
	    startTime_p = MVTime(Quantity(lastTime,"s"));
	}
	initSubTableFillers(useFeedDDFiller);
	hasMS_p = True;
	result = True;
    }
    return result;
}

Bool GBTBackendFiller::createMS(const String &msName,
				const TableDesc &mainTD,
				const GBTMSFillOptions &fillOptions,
				Int nrows,
				Bool useFeedDDFiller)
{
    Vector<String> tmp;
    // because tmp is empty this stman is never used
    StandardStMan stman;
    return createMS(msName, mainTD, stman, tmp, 
		    IPosition(), fillOptions, stman, tmp, nrows,
		    False, useFeedDDFiller);
}

Bool GBTBackendFiller::createMS(const String &msName,
				const TableDesc &mainTD,
				const DataManager &dm,
				const Vector<String> &dmColNames,
				const IPosition &defaultTileShape,
				const GBTMSFillOptions &fillOptions,
				Int nrows,
				Bool spectralLine,
				Bool useFeedDDFiller)
{
    Vector<String> tmp;
    // because tmp is empty this stman is never used
    StandardStMan stman;
    return createMS(msName, mainTD, dm, dmColNames, defaultTileShape,
		    fillOptions, stman, tmp, nrows, spectralLine, useFeedDDFiller);
}

Bool GBTBackendFiller::createMS(const String &msName, 
				const TableDesc &mainTD,
				const DataManager &dm, 
				const Vector<String> &dmColNames,
				const IPosition &defaultTileShape,
				const GBTMSFillOptions &fillOptions,
				const DataManager &dm2,
				const Vector<String> &dm2ColNames,
				Int nrows,
				Bool spectralLine, 
				Bool useFeedDDFiller)
{
    IncrementalStMan incstman ("IMData");
    StandardStMan stdstman ("StdSMData");

    SetupNewTable newMain(msName, mainTD, Table::New);
    newMain.bindAll(stdstman);
    for (uInt i=0;i<dmColNames.nelements();i++) {
	newMain.bindColumn(dmColNames(i), dm);
    }
    for (uInt i=0;i<dm2ColNames.nelements();i++) {
	newMain.bindColumn(dm2ColNames(i), dm2);
    }
    // many things should probably be bound to the ISM - they don't change often
    // the specific fillers make use of this information, if any of these stop
    // using the ISM, the fillers will also need to be changed
    newMain.bindColumn(MS::columnName(MS::SCAN_NUMBER), incstman);
    newMain.bindColumn(MS::columnName(MS::INTERVAL), incstman);
    newMain.bindColumn(MS::columnName(MS::EXPOSURE), incstman);
    newMain.bindColumn(MS::columnName(MS::TIME), incstman);
    newMain.bindColumn(MS::columnName(MS::TIME_CENTROID), incstman);
    newMain.bindColumn(MS::columnName(MS::FIELD_ID), incstman);
    newMain.bindColumn(MS::columnName(MS::OBSERVATION_ID), incstman);
    newMain.bindColumn(MS::columnName(MS::PROCESSOR_ID), incstman);
    newMain.bindColumn(MS::columnName(MS::ARRAY_ID), incstman);
    newMain.bindColumn(MS::columnName(MS::UVW), incstman);
    newMain.bindColumn(MS::columnName(MS::ANTENNA1), incstman);
    newMain.bindColumn(MS::columnName(MS::ANTENNA2), incstman);
    if (mainTD.isColumn("NRAO_GBT_IF_ID"))
	newMain.bindColumn("NRAO_GBT_IF_ID", incstman);
    if (mainTD.isColumn("NRAO_GBT_MASTERSTATE_IDS"))
	newMain.bindColumn("NRAO_GBT_MASTERSTATE_IDS", incstman);

    delete ms_p;
    ms_p = 0;
    ms_p = new MeasurementSet(newMain, tableLock(), nrows);
    AlwaysAssert(ms_p, AipsError);

    {
      TableInfo &info(ms_p->tableInfo());
      info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
      info.setSubType(String("GBT"));
      info.readmeAddLine(String("This is a MeasurementSet Table holding measurements from the GBT (operated by NRAO)"));
    }

    // the filler version number keyword - start with version 2 - version 1 never had this keyword.
    ms_p->rwKeywordSet().define("NRAO_GBTMSFILLER_VERSION", 3.0);

    // The ANTENNA table
    SetupNewTable newAntenna(ms_p->antennaTableName(),
			     MSAntenna::requiredTableDesc(), Table::New);
    newAntenna.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::ANTENNA),
				     Table(newAntenna, tableLock()));

    // DATA_DESCRIPTION table
    SetupNewTable newDataDesc(ms_p->dataDescriptionTableName(),
			      MSDataDescription::requiredTableDesc(), 
			      Table::New);
    newDataDesc.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::DATA_DESCRIPTION),
				     Table(newDataDesc, tableLock()));

    // don't use DOPPLER just yet, but probably eventually

    // The FEED table
    TableDesc feedtd = MSFeed::requiredTableDesc();
    // add the GBT-specific columns
    feedtd.addColumn(ScalarColumnDesc<String> ("GBT_FEED_NAME"));
    feedtd.addColumn(ScalarColumnDesc<Int> ("GBT_SRFEED_ID"));
    feedtd.addColumn(ScalarColumnDesc<String> ("GBT_TRCKBEAM"));
    feedtd.addColumn(ScalarColumnDesc<String> ("GBT_RECEIVER"));
    feedtd.addColumn(ArrayColumnDesc<String> ("GBT_RECEPTORS", 1));
    SetupNewTable newFeed(ms_p->feedTableName(), feedtd, Table::New);
    // incremental okay for most columns
    newFeed.bindAll(incstman);
    // but use the standard st man for FEED_ID, TIME, INTERVAL, BEAM_OFFSET,
    // POLARIZATION_TYPE, GBT_FEED_NAME, GBT_SRFEED_ID, GBT_TRCKBEAM
    newFeed.bindColumn(MSFeed::columnName(MSFeed::FEED_ID), stdstman);
    newFeed.bindColumn(MSFeed::columnName(MSFeed::TIME), stdstman);
    newFeed.bindColumn(MSFeed::columnName(MSFeed::INTERVAL), stdstman);
    newFeed.bindColumn(MSFeed::columnName(MSFeed::BEAM_OFFSET), stdstman);
    newFeed.bindColumn(MSFeed::columnName(MSFeed::POLARIZATION_TYPE), stdstman);
    newFeed.bindColumn("GBT_FEED_NAME", stdstman);
    newFeed.bindColumn("GBT_SRFEED_ID", stdstman);
    newFeed.bindColumn("GBT_TRCKBEAM", stdstman);
    // the reference code for BEAM_OFFSET is changed to AZEL below
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::FEED),
				     Table(newFeed, tableLock()));

    // The FIELD table

    SetupNewTable newField(ms_p->fieldTableName(),
			   MSField::requiredTableDesc(), Table::New);
    newField.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::FIELD),
				     Table(newField, tableLock()));

    // The FLAG_CMD table
    SetupNewTable newFlagCmd(ms_p->flagCmdTableName(),
			     MSFlagCmd::requiredTableDesc(), Table::New);
    newFlagCmd.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::FLAG_CMD),
				     Table(newFlagCmd, tableLock()));

    // No FREQ_OFFSET table just yet, but probably eventually

    // The HISTORY table
    SetupNewTable newHistory(ms_p->historyTableName(),
			     MSHistory::requiredTableDesc(), Table::New);
    newHistory.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::HISTORY),
				     Table(newHistory, tableLock()));

    // The OBSERVATION table
    SetupNewTable newObs(ms_p->observationTableName(),
			 MSObservation::requiredTableDesc(), Table::New);
    newObs.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::OBSERVATION),
				     Table(newObs, tableLock()));

    // The POINTING table
    TableDesc pointingTd = MSPointing::requiredTableDesc();
    // add the optional POINTING_MODEL_ID column
    MSPointing::addColumnToDesc(pointingTd, MSPointing::POINTING_MODEL_ID);
    SetupNewTable newPointing(ms_p->pointingTableName(), pointingTd, Table::New);
    newPointing.bindAll(stdstman);
    // some columns should use the incremental st man
    newPointing.bindColumn(MSPointing::columnName(MSPointing::ANTENNA_ID), incstman);
    newPointing.bindColumn(MSPointing::columnName(MSPointing::NAME), incstman);
    newPointing.bindColumn(MSPointing::columnName(MSPointing::NUM_POLY), incstman);
    newPointing.bindColumn(MSPointing::columnName(MSPointing::TIME_ORIGIN), incstman);
    newPointing.bindColumn(MSPointing::columnName(MSPointing::TRACKING), incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::POINTING),
				     Table(newPointing, tableLock()));

    // The POLARIZATION table
    SetupNewTable newPol(ms_p->polarizationTableName(),
			 MSPolarization::requiredTableDesc(), Table::New);
    newPol.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::POLARIZATION),
				     Table(newPol, tableLock()));

    // The PROCESSOR table
    SetupNewTable newProc(ms_p->processorTableName(),
			  MSProcessor::requiredTableDesc(), Table::New);
    newProc.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::PROCESSOR),
				     Table(newProc, tableLock()));

    // The SOURCE table
    TableDesc sourceTd = MSSource::requiredTableDesc();
    MSSource::addColumnToDesc(sourceTd, MSSource::POSITION);
    // Add spectral-line based columns
    if (spectralLine) {
	MSSource::addColumnToDesc(sourceTd, MSSource::REST_FREQUENCY);
        MSSource::addColumnToDesc(sourceTd, MSSource::SYSVEL);
    }
    SetupNewTable newSource(ms_p->sourceTableName(), sourceTd, Table::New);
    newSource.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
				     Table(newSource, tableLock()));

    // The SPECTRAL_WINDOW table
    SetupNewTable newSpecWin(ms_p->spectralWindowTableName(),
			     MSSpectralWindow::requiredTableDesc(), Table::New);
    newSpecWin.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::SPECTRAL_WINDOW),
				     Table(newSpecWin, tableLock()));

    // The STATE table
    TableDesc stateTd = MSState::requiredTableDesc();
    // add the NRAO-specific columns
    stateTd.addColumn(ScalarColumnDesc<String> ("NRAO_GBT_PROCNAME"));
    stateTd.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_PROCSIZE"));
    stateTd.addColumn(ScalarColumnDesc<Int> ("NRAO_GBT_PROCSEQN"));
    SetupNewTable newState(ms_p->stateTableName(), stateTd, Table::New);
    newState.bindAll(stdstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::STATE),
				     Table(newState, tableLock()));

    // The SYSCAL table
    TableDesc sysCalTd = MSSysCal::requiredTableDesc();
    // Add TCAL  and TCAL_FLAG columns 
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TCAL);
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TCAL_FLAG);
    // and add TRX and TRX_FLAG
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TRX);
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TRX_FLAG);
    // and add TSYS and TSYS_FLAG
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TSYS);
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TSYS_FLAG);
    // Add spectral-line based columns
    if (spectralLine) {
	MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TCAL_SPECTRUM);
	MSSysCal::addColumnCompression(sysCalTd, MSSysCal::TCAL_SPECTRUM);
    }

    SetupNewTable newSysCal(ms_p->sysCalTableName(), sysCalTd, Table::New);
    newSysCal.bindAll(stdstman);
    // some columns should use the incremental st man
    newSysCal.bindColumn(MSSysCal::columnName(MSSysCal::ANTENNA_ID), incstman);
    newSysCal.bindColumn(MSSysCal::columnName(MSSysCal::INTERVAL), incstman);
    newSysCal.bindColumn(MSSysCal::columnName(MSSysCal::TCAL_FLAG), incstman);
    newSysCal.bindColumn(MSSysCal::columnName(MSSysCal::TRX_FLAG), incstman);
    newSysCal.bindColumn(MSSysCal::columnName(MSSysCal::TSYS_FLAG), incstman);
    // and bind any compress columns - only necessary for spectralLine
    if (spectralLine) {
	MSTableImpl::setupCompression(newSysCal);
    }
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::SYSCAL),
				     Table(newSysCal, tableLock()));

    // The WEATHER table
    SetupNewTable newWeather(ms_p->weatherTableName(),
			     MSWeather::requiredTableDesc(), Table::New);
    newWeather.bindAll(incstman);
    ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::WEATHER),
				     Table(newWeather, tableLock()));

    // reset the MS references
    ms_p->initRefs();

    // spectral line MSs get compressed calibration sets
    Bool compress = spectralLine && fillOptions.compressCalCols();
    addCalSet(defaultTileShape, compress);

    reattach();

    // add the QuantumUnits keyword to FLOAT_DATA or DATA
    if (!mscols_p->floatData().isNull()) {
	mscols_p->floatData().rwKeywordSet().define("QuantumUnits","CNTS");
    }
    if (!mscols_p->data().isNull()) {
	mscols_p->data().rwKeywordSet().define("QuantumUnits","CNTS");
    }

    // set the reference code of FEED::BEAM_OFFSET to MDirection::AZEL
    {
	MSFeedColumns feedCols(ms().feed());
	feedCols.beamOffsetMeas().setDescRefCode(MDirection::AZEL);
    }

    initSubTableFillers(useFeedDDFiller);

    // initial time is zero
    startTime_p = MVTime(0.0);
    hasMS_p = True;
    return True;
}

void GBTBackendFiller::reattach()
{
    delete mscols_p;
    mscols_p = 0;

    mscols_p = new MSMainColumns(*ms_p);
    AlwaysAssert(mscols_p, AipsError);

    attachCalSet();
}

Bool GBTBackendFiller::prepareDAPfillers(const String &dapfile, 
					 const MVTime &startTime)
{
    String device, manager, sampler;
    Bool result = GBTDAPFillerBase::parseName(dapfile, device, manager, sampler);
    if (result) {
	if (device == "Weather") {
	    // give it to the weather filler
	    weatherFiller().prepare(dapfile, manager, sampler, startTime);
	} else {
	    // do we already know of this device
	    Int whichDevice = deviceMap_p(device);
	    if (whichDevice < 0) {
		// we need a new one
		whichDevice = dapFillers_p.nelements();
		dapFillers_p.resize(whichDevice+1);
		readyToFill_p.resize(whichDevice+1);
		readyToFill_p[whichDevice] = False;
		if (device == "Antenna") {
		    dapFillers_p[whichDevice] = new GBTAntennaDAPFiller(device, *ms_p);
		    antennaDAPFiller_p = whichDevice;
		    pointingFiller().
			setAntennaDAPFiller(dynamic_cast<GBTAntennaDAPFiller *>(dapFillers_p[antennaDAPFiller_p]));
		} else if (device == "LO1") {
		    dapFillers_p[whichDevice] = new GBTLO1DAPFiller(device, *ms_p);
		    lo1DAPFiller_p = whichDevice;
 		} else {
 		    dapFillers_p[whichDevice] = new GBTDAPFiller(device, *ms_p);
 		}
		AlwaysAssert(dapFillers_p[whichDevice], AipsError);
		deviceMap_p.define(device, whichDevice);
	    }
	    if (dapFillers_p[whichDevice]->prepare(dapfile, manager, sampler,startTime) >= 0) {
		readyToFill_p[whichDevice] = True;
	    }
	}
    } else {
	LogIO os(LogOrigin("GBTBackendFiller","prepareDAPFillers(const String &dapfile, const MVTime &starttime)"));
	os << LogIO::WARN
	   << "Problem parsing DAP file name : " << dapfile << LogIO::POST;
    }
    return result;
}

void GBTBackendFiller::fillDAP()
{
    // if the ANTENNA DAP isn't ready to fill, prepare it with an empty name
    // to clear the internal tables
    if (antennaDAPFiller_p >= 0 && !readyToFill_p[antennaDAPFiller_p]) 
 	dapFillers_p[antennaDAPFiller_p]->prepare("","","",MVTime(0.0));
    // if the LO1 DAP isn't ready to fill, prepare it with an empty name
    if (lo1DAPFiller_p >= 0 && !readyToFill_p[lo1DAPFiller_p])
	dapFillers_p[lo1DAPFiller_p]->prepare("","","",MVTime(0.0));

    for (uInt i=0;i<dapFillers_p.nelements();i++) {
	if (readyToFill_p[i]) {
	    dapFillers_p[i]->fill();
	    readyToFill_p[i] = False;
	}
    }
}

void GBTBackendFiller::initSubTableFillers(Bool useFeedDDFiller)
{
    // set up the sub-table fillers
     antennaFiller().attach(ms().antenna());
     if (useFeedDDFiller) {
	 feedDDFiller().attach(ms());
     } else {
	 dataDescFiller().attach(ms().dataDescription());
	 feedFiller().attach(ms().feed());
	 polarizationFiller().attach(ms().polarization());
	 specWinFiller().attach(ms().spectralWindow());
     }
     fieldFiller().attach(ms().field());
     historyFiller().attach(ms().history());
     observationFiller().attach(ms().observation());
     processorFiller().attach(ms().processor());
     pointingFiller().attach(ms().pointing());
     sourceFiller().attach(ms().source());
     stateFiller().attach(ms().state());
     sysCalFiller().attach(ms().sysCal());
     weatherFiller().attach(ms().weather());
}

void GBTBackendFiller::flush() {
    if (hasMS()) {
	antennaFiller().flush();
	if (feedDDFiller().isAttached()) {
	    feedDDFiller().flush();
	} else {
	    dataDescFiller().flush();
	    feedFiller().flush();
	    polarizationFiller().flush();
	    specWinFiller().flush();
	}
	fieldFiller().flush();
	historyFiller().flush();
	observationFiller().flush();
	pointingFiller().flush();
	processorFiller().flush();
	sourceFiller().flush();
	// stateFiller().flush();
	sysCalFiller().flush();
	weatherFiller().flush();
	if (lo1Filler()) lo1Filler()->flush();
	ms().flush();
	for (uInt i=0;i<dapFillers_p.nelements();i++) {
	    if (dapFillers_p[i]) dapFillers_p[i]->flush();
	}
    }
}

void GBTBackendFiller::addCalSet(const IPosition &defaultTileShape,
				 Bool compress)
{
    // Add a calibration set (comprising a set of CORRECTED_DATA, 
    // and MODEL_DATA columns) to the MeasurementSet.
  
    // Define a column accessor to the observed data
    ROTableColumn* data;
    if (ms_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA))) {
	data = new ROArrayColumn<Float> (*ms_p, MS::columnName(MS::FLOAT_DATA));
    } else if (ms_p->tableDesc().isColumn(MS::columnName(MS::DATA))) {
	data = new ROArrayColumn<Complex> (*ms_p, MS::columnName(MS::DATA));
    } else {
	// Holography data - nothing to do
	return;
    }

    // Check if the data column is tiled and, if so, the
    // smallest tile shape used.
    String dataManType = data->columnDesc().dataManagerType();
    String dataManGroup = data->columnDesc().dataManagerGroup();
    IPosition dataTileShape;
    Bool tiled = (dataManType.contains("Tiled"));
    Bool simpleTiling = False;

    if (tiled) {
	ROTiledStManAccessor tsm(*ms_p, dataManGroup);
	uInt nHyper = tsm.nhypercubes();
	// Find smallest tile shape
	Int lowestProduct = 0;
	Int lowestId = 0;
	Bool firstFound = False;
	for (uInt id=0; id < nHyper; id++) {
	    Int product = tsm.getTileShape(id).product();
	    if (product > 0 && (!firstFound || product < lowestProduct)) {
		lowestProduct = product;
		lowestId = id;
		if (!firstFound) firstFound = True;
	    };
	};
	if (!firstFound) {
	    dataTileShape = defaultTileShape;
	} else {
	    dataTileShape = tsm.getTileShape(lowestId);
	}
	simpleTiling = (dataTileShape.nelements() == 3);
    };

    if (!tiled || !simpleTiling) {
	// Untiled, or tiled at a higher than expected dimensionality
	// first, try default tile shape
	if (defaultTileShape.product() <= 0) {
	    dataTileShape = defaultTileShape;
	} else {
	    // Use a canonical tile shape of 128 kB size

	    Int maxNchan = 1024;
	    Int nCorr = 2;
	    // try to infer nCorr from table, if there is something there
	    if (data->nrow() > 0 && data->isDefined(0)) {
		nCorr = data->shape(0)(0);
		dataTileShape = IPosition(3, nCorr, maxNchan, max(1, 131072/nCorr/maxNchan));
	    }
	}
    };
  
    // Add the MODEL_DATA column
    Vector<String> coordColNames(0), idColNames(0);
    TableDesc tdModel, tdModelComp, tdModelScale;
    CompressComplexSD* ccModel=NULL;
    String colModel=MS::columnName(MS::MODEL_DATA);

    tdModel.addColumn(ArrayColumnDesc<Complex>(colModel,"model data", 2));
    IPosition modelTileShape = dataTileShape;
    if (compress) {
	tdModelScale.addColumn(ScalarColumnDesc<Float>(colModel+"_SCALE"));
	tdModelScale.addColumn(ScalarColumnDesc<Float>(colModel+"_OFFSET"));
	tdModelComp.addColumn(ArrayColumnDesc<Int>(colModel+"_COMPRESSED",
						   "model data compressed",2));
	StandardStMan modelScaleStMan("ModelScaleOffset");
	ms_p->addColumn(tdModelScale, modelScaleStMan);
	
	tdModelComp.defineHypercolumn("TiledShape-ModelComp",3,
				      stringToVector(colModel+"_COMPRESSED"),
				      coordColNames,idColNames);
	ccModel = new CompressComplexSD(colModel, colModel+"_COMPRESSED",
					colModel+"_SCALE", colModel+"_OFFSET", True);
	
	TiledShapeStMan modelCompStMan("TiledShape-ModelComp", modelTileShape);
	ms_p->addColumn(tdModelComp, modelCompStMan);
	ms_p->addColumn(tdModel, *ccModel);
	
    } else {
	tdModel.defineHypercolumn("TiledShape-Model",3,
				  stringToVector(colModel), coordColNames,
				  idColNames);
	TiledShapeStMan modelStMan("TiledShape-Model", modelTileShape);
	ms_p->addColumn(tdModel, modelStMan);
    };
    
    // Add the CORRECTED_DATA column
    TableDesc tdCorr, tdCorrComp, tdCorrScale;
    CompressComplexSD* ccCorr=NULL;
    String colCorr=MS::columnName(MS::CORRECTED_DATA);
    
    tdCorr.addColumn(ArrayColumnDesc<Complex>(colCorr,"corrected data", 2));
    IPosition corrTileShape = dataTileShape;
    if (compress) {
	tdCorrScale.addColumn(ScalarColumnDesc<Float>(colCorr+"_SCALE"));
	tdCorrScale.addColumn(ScalarColumnDesc<Float>(colCorr+"_OFFSET"));
	StandardStMan corrScaleStMan("CorrScaleOffset");
	ms_p->addColumn(tdCorrScale, corrScaleStMan);
	
	tdCorrComp.addColumn(ArrayColumnDesc<Int>(colCorr+"_COMPRESSED",
						  "corrected data compressed",2));
	tdCorrComp.defineHypercolumn("TiledShape-CorrComp",3,
				     stringToVector(colCorr+"_COMPRESSED"),
				     coordColNames,idColNames);
	ccCorr = new CompressComplexSD(colCorr, colCorr+"_COMPRESSED",
				       colCorr+"_SCALE", colCorr+"_OFFSET", True);
	
	TiledShapeStMan corrCompStMan("TiledShape-CorrComp", corrTileShape);
	ms_p->addColumn(tdCorrComp, corrCompStMan);
	ms_p->addColumn(tdCorr, *ccCorr);
	
    } else {
	tdCorr.defineHypercolumn("TiledShape-Corr",3,
				 stringToVector(colCorr), coordColNames,
				 idColNames);
	TiledShapeStMan corrStMan("TiledShape-Corr", corrTileShape);
	ms_p->addColumn(tdCorr, corrStMan);
    };
    // set the QuantumUnits keyword value here
    TableColumn cd(*ms_p,colCorr);
    // get the value from data column, if there
    String qu("CNTS");
    if (data->keywordSet().fieldNumber("QuantumUnits") >= 0) {
	qu = data->keywordSet().asString("QuantumUnits)");
    }
    cd.rwKeywordSet().define("QuantumUnits",qu);
        
    if (ccModel) delete ccModel;
    if (ccCorr) delete ccCorr;
    delete data;
}

void GBTBackendFiller::initCalSet()
{
    // Define a column accessor to the observed data
    ROTableColumn* data;
    Bool floatData = True;
    if (mscols_p->data().isNull()) {
	// must be FLOAT_DATA
	data = new ROArrayColumn<Float>(*ms_p, MS::columnName(MS::FLOAT_DATA));
    } else {
	floatData = False;
	data = new ROArrayColumn<Complex> (*ms_p, MS::columnName(MS::DATA));
    }

    // Set the shapes for each row and put in the default values
    // corrected data is a copy of data, and model data is all 1s
    for (uInt row=0; row < ms_p->nrow(); row++) {
	// only do this if there is something in the data row
	if (data->isDefined(row)) {
	    IPosition rowShape=data->shape(row);
	    modelData().setShape(row,rowShape);
	    correctedData().setShape(row,rowShape);

	    modelData().basePut(row,Array<Complex>(rowShape,1.0));
	    Array<Complex> cdata(rowShape);
	    if (floatData) {
		convertArray(cdata, mscols_p->floatData()(row));
	    } else {
		cdata = mscols_p->data()(row);
	    }
	    correctedData().basePut(row,cdata);
	}
    }
    delete data;
}

void GBTBackendFiller::attachCalSet()
{
    // don't bother with this if there is no data column - must be holography data
    if (!mscols_p->data().isNull() || !mscols_p->floatData().isNull()) {
	// make sure they exist - assume if one does, they all do
	if (!ms_p->tableDesc().isColumn("CORRECTED_DATA")) {
	    // add them in as compressed unless this is continuum data
	    // - only works if there is already data there, which
	    // there should be if we get to this step since the fillers should
	    // add these most of the time.  Hence this only happens on old data
	    // or, when we add this feature, the user has choosen not to add
	    // calibration iformation during the initial fill.
	    Bool compress = True;
	    if (ms_p->nrow() > 0) {
		if (!mscols_p->floatData().isNull()) {
		    compress = mscols_p->floatData().shape(0).product() > 10;
		} else {
		    // must be a data column then
		    compress = mscols_p->data().shape(0).product() > 10;
		}
	    }
	    addCalSet(IPosition(), compress);
	    initCalSet();
	}
	modelData_p.attach(*ms_p,"MODEL_DATA");
	correctedData_p.attach(*ms_p, "CORRECTED_DATA");
    }
}
