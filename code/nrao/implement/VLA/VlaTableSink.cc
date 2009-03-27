//# ClassFileName.cc:  this defines ClassName, which ...
//# Copyright (C) 1997,1999,2001
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

#include <nrao/VLA/VlaTableSink.h>
#include <nrao/VLA/VlaDAs.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/IPosition.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <casa/sstream.h>

//#include <math.h>

// Stubbed for now

// Note to myself, the continuum and spectral line CDA write routines are
// are probably broke.  I need to redo them.  Look at Gareth's stuff in
// VlaRecord there's probably something to steal from there.
//

VlaTableSink::VlaTableSink()
  :VlaSink() 
{
}

VlaTableSink::VlaTableSink(const String& a, const String& b)
  :VlaSink()
{
  allocateFlags();
  setConstraints(a);
  rcaNames.resize(10);
  sdaNames.resize(200);
  adaNames.resize(100);
  cdaNames.resize(100);
  rowCount = 0;
  createTable(b);
}

VlaTableSink::~VlaTableSink() {
}

Int VlaTableSink::writeVisibilityRecord(const Char* buffer) {
  Int rStat = 1;
  data.addRow();
  
  writeTable(rcaFlags);
  writeTable(sdaFlags);
  writeTable(adaFlags);
  writeTable(ccdaFlags);
  writeTable(slcdaFlags);

  dataRow.put(rowCount++);
  data.flush();
  cerr << data.nrow() << " ";
  return rStat; 
}

void VlaTableSink::createTable(const String& tableName) {
  TableDesc td("vlaTableDesc", "1", TableDesc::Scratch);
  td.comment() = "VLA Visibility Table";
  addColumns(td, rcaFlags);
  addColumns(td, sdaFlags);
  addColumns(td, adaFlags);
  addColumns(td, ccdaFlags);
  addColumns(td, slcdaFlags);
  
  SetupNewTable nt(tableName, td, Table::New);
  IncrementalStMan ism;
  // StManAipsIO ism;
  nt.bindAll(ism);

  if (adaNames.nelements() > 0) {
    td.defineHypercolumn("ada", adaNames.nelements(), adaNames);
    TiledColumnStMan sm3("ada", IPosition(adaNames.nelements(), 27, 10));
    nt.bindColumn("ada", sm3);
  }

  if(cdaNames.nelements() > 0){
    td.defineHypercolumn("cda", cdaNames.nelements(), cdaNames);
    TiledColumnStMan sm4("cda", IPosition(cdaNames.nelements(), 351,10));
    nt.bindColumn("cda", sm4);
  }
  
  
  data = Table(nt);
    
  Vector<String> colNames = concatenateArray(rcaNames, sdaNames);
  colNames = concatenateArray(colNames, adaNames);
  colNames = concatenateArray(colNames, cdaNames);
  // dataRow = new TableRow(*data, colNames);
  dataRow = TableRow(data);
}

void VlaTableSink::addColumns(TableDesc &td, const RCAFlags* flags){
  Int vecSize = 0;
  if (flags) {
    if (flags->dateMJADFlag) {
      td.addColumn(ScalarColumnDesc<Int> ("MJAD", "Modified Julian Date")); 
      rcaNames(vecSize++) = "MJAD";
    }
    if (flags->timeIATFlag) {
      td.addColumn(ScalarColumnDesc<Int> ("IAT", "IAT since midnight in 19.2Hz")); 
      rcaNames(vecSize++) = "IAT";
    }
    if (flags->numberOfAntennasFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("Nants", "Number of Antennas")); 
      rcaNames(vecSize++) = "Nants";
    }
  }
  rcaNames.resize(vecSize, True);
  return;
}

void VlaTableSink::addColumns(TableDesc& td, const SDAFlags* flags){
  Int vecSize = 0;
  if (flags) {
    if(flags->subarrayIDFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("subarrayID", "Subarray ID")); 
      sdaNames(vecSize++) = "subarrayID";
    } 
    if (flags->sourceNameFlag) {
      td.addColumn(ScalarColumnDesc<String> ("source", "Source Name")); 
      sdaNames(vecSize++) = "source";
    }
    if (flags->sourceNameQualifierFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("qualifier", "Source Name Qualifier")); 
      sdaNames(vecSize++) = "qualifier";
    }
    if (flags->observingProgramIDFlag) {
      td.addColumn(ScalarColumnDesc<String> ("progID", "Program ID")); 
      sdaNames(vecSize++) = "progID";
    }
    if (flags->aipsIDFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("aipsID", "Aips ID")); 
      sdaNames(vecSize++) = "aipsID";
    }
    if (flags->observingModeFlag) {
      td.addColumn(ScalarColumnDesc<String> ("obsMode", "Observing Mode")); 
      sdaNames(vecSize++) = "obsMode";
    }
    if (flags->calibratorCodeFlag) {
      td.addColumn(ScalarColumnDesc<Char> ("calCode", "Calibrator Code")); 
      sdaNames(vecSize++) = "calCode";
    }
    if (flags->submodeFlag) {
      td.addColumn(ScalarColumnDesc<uChar> ("subMode", "Sub Mode")); 
      sdaNames(vecSize++) = "subMode";
    }
    if (flags->arrayStatusInformationFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("arrayStatus", "Array Status")); 
      sdaNames(vecSize++) = "arrayStatus";
    }
    if (flags->numFreqChansPerBaselineFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("chansPerBaseline",
					    "Number of Frequency Channels per Baseline")); 
      sdaNames(vecSize++) = "chansPerBaseline";
    }
    if (flags->integrationTimeFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("integrationTime",
					    "Integration Time")); 
      sdaNames(vecSize++) = "integrationTime";
    }
    if (flags->stopTimeFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("stop", "Stop Time")); 
      sdaNames(vecSize++) = "stop";
    }
    if (flags->startTimeFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("start", "Start Time")); 
      sdaNames(vecSize++) = "start";
    }
    if (flags->raFlag) {
      td.addColumn(ScalarColumnDesc<Double> ("RA", "Right Ascension")); 
      sdaNames(vecSize++) = "RA";
    }
    if (flags->decFlag) {
      td.addColumn(ScalarColumnDesc<Double> ("Dec", "Declination")); 
      sdaNames(vecSize++) = "Dec";
    }
    if (flags->apparentRAFlag) {
      td.addColumn(ScalarColumnDesc<Double> ("apparentRA", "Apparent RA")); 
      sdaNames(vecSize++) = "apparentRA";
    }
    if (flags->apparentDecFlag) {
      td.addColumn(ScalarColumnDesc<Double> ("apparentDec",
                                             "Apparent Declination")); 
      sdaNames(vecSize++) = "apparentDec";
    }
    for (Int i=0; i<4; i++) {
      if (flags->sslosFlag[i]) {
	ostringstream oss; 
	oss << i;
	String tdName(String("sslo_") + String(oss.str()));
	td.addColumn(ScalarColumnDesc<Double> (tdName, "Signed Sum of LOs")); 
	sdaNames(vecSize++) = tdName;
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->skyFreqFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("skyfreq_") + String(oss.str()));
	td.addColumn(ScalarColumnDesc<Double> (tdName, "Sky Frequency")); 
	sdaNames(vecSize++) = tdName;
      }
    }
    if (flags->iatEndOfIntegrationFlag) {
      td.addColumn(ScalarColumnDesc<Double> ("iatEnd",
                                             "IAT at end of integration")); 
      sdaNames(vecSize++) = "iatEnd";
    }
    if (flags->lstEndOfIntegrationFlag) {
      td.addColumn(ScalarColumnDesc<Double> ("lstEnd",
                                             "LST at end of integration")); 
      sdaNames(vecSize++) = "lstEnd";
    }
    if (flags->iat4GeoCalculationsFlag) {
      td.addColumn(ScalarColumnDesc<Double> ("iat4GeoCalcs",
                                             "IAT for Geo Calculations")); 
      sdaNames(vecSize++) = "iat4GeoCalcs";
    }
    if (flags->currentSurfaceRefractivityFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("curSurfRefract",
					    "Current Surface Refractivity")); 
      sdaNames(vecSize++) = "curSurfRefract";
    }
    if (flags->estimatedZenithAtmosPhaseFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("estZenithAtmosPhase",
					    "Estimated Zenith Atmospheric Phase")); 
      sdaNames(vecSize++) = "estZenithAtmosPhase";
    }
    if (flags->sinElevationFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("sinEl", "sin of Elevation")); 
      sdaNames(vecSize++) = "sinEl";
    }
    if (flags->cosElevationFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("cosEl", "cos of Elevation")); 
      sdaNames(vecSize++) = "cosEl";
    }
    if (flags->sinAzimuthFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("sinAz", "sin of Azimuth")); 
      sdaNames(vecSize++) = "sinAz";
    }
    if (flags->cosAzimuthFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("cosAz", "cos of Azimuth")); 
      sdaNames(vecSize++) = "cosAz";
    }
    if (flags->sinParallacticAngleFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("sinPA",
					    "sin of Parallactic Angle")); 
      sdaNames(vecSize++) = "sinPA";
    }
    if (flags->cosParallacticAngleFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("cosPA",
					    "cos of Parallactic Angle")); 
      sdaNames(vecSize++) = "cosPA";
    }
    if (flags->bandwidthCodesFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("bwCodes", "Bandwidth Codes")); 
      sdaNames(vecSize++) = "bwCodes";
    }
    if (flags->frontendFilterCodesFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("feFilterCodes",
					    "Front-end filter codes")); 
      sdaNames(vecSize++) = "feFilterCodes";
    }
    if (flags->zeroSpacingFluxFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("zeroSpcFlux",
					    "Zero spacing flux")); 
      sdaNames(vecSize++) = "zeroSpcFlux";
    }
    if (flags->uvLimitsFlag) {
      td.addColumn(ScalarColumnDesc<Float> ("uvLimits", "UV-Limits")); 
      sdaNames(vecSize++) = "uvLimits";
    }
    if (flags->arrayControlBitsFlag) {
      td.addColumn(ArrayColumnDesc<Float> ("arrayCtrlBits",
					   "Array Control Bits")); 
      sdaNames(vecSize++) = "arrayCtrlBits";
    }
    for (Int i=0; i<5; i++) {
      if(flags->weatherInfoFlag[i]){
	ostringstream oss; oss << i;
	String tdName(String("weather_") + String(oss.str()));
	td.addColumn(ScalarColumnDesc<Float> (tdName, "Weather Info")); 
	sdaNames(vecSize++) = tdName;
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->radialVelocityFlag[i]) {
	ostringstream oss; oss << i;
	String tdName(String("rvel_") + String(oss.str()));
	td.addColumn(ScalarColumnDesc<Double> (tdName, "Radial Velocity")); 
	sdaNames(vecSize++) = tdName;
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->lineRestFreqFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("restFreq_") + String(oss.str()));
	td.addColumn(ScalarColumnDesc<Double> (tdName, "Rest Frequency")); 
	sdaNames(vecSize++) = tdName;
      }
    }
      
    for (Int i=0; i<4; i++) {
      if (flags->velocityReferenceFrameFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("refFrame_") + String(oss.str()));
	td.addColumn(ScalarColumnDesc<String> (tdName,
					       "Velocity Reference Frame")); 
	sdaNames(vecSize++) = tdName;
      }
    }
    if (flags->correlatorModeFlag) {
      td.addColumn(ScalarColumnDesc<String> ("corrMode", "Correlator Mode")); 
      sdaNames(vecSize++) = "corrMode";
    }
    if (flags->arrayProcessorOptionsFlag) {
      td.addColumn(ScalarColumnDesc<String> ("arrayProcessor",
                                             "Array Processor Options")); 
      sdaNames(vecSize++) = "arrayProcessor";
    }
    if (flags->epochYearFlag) {
      td.addColumn(ScalarColumnDesc<Short> ("epochYr", "Epoch Year")); 
      sdaNames(vecSize++) = "epochYr";
    }
    for (int i=0; i<4; i++) {
      if (flags->channelOffsetsFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("chanOffsets_") + String(oss.str()));
	td.addColumn(ScalarColumnDesc<Short> (tdName, "Channel Offsets")); 
	sdaNames(vecSize++) = tdName;
      }
    }
    for (int i=0; i<4; i++) {
      if (flags->channelSepCodesFlag[i]) {
	ostringstream oss; oss << i;
	String tdName(String("chanSepCodes_") + String(oss.str()));
	td.addColumn(ScalarColumnDesc<Short> (tdName,
					      "Channel Separation Codes")); 
	sdaNames(vecSize++) = tdName;
      } 
    }
  }
  sdaNames.resize(vecSize, True);
  return;
}

void VlaTableSink::addColumns(TableDesc& td, const ADAFlags* flags){
  Int vecSize = 0;
  if (flags) {
    Int i = 0;
    if (flags->antennaIDFlag) {
      td.addColumn(ArrayColumnDesc<Int> ("antID", "Antenna ID")); 
      adaNames(vecSize++) = "antID";
    }
    if (flags->dcsAddressFlag) {
      td.addColumn(ArrayColumnDesc<Int> ("dcsAddress", "DCS Address")); 
      adaNames(vecSize++) = "dcsAddress";
    }
    if (flags->antennaControlBitsFlag) {
      td.addColumn(ArrayColumnDesc<Int> ("antCtrlBits", "Antenna Control Bits")); 
      adaNames(vecSize++) = "antCtrolBits";
    }
    if (flags->ifStatusFlag) {
      td.addColumn(ArrayColumnDesc<Int> ("ifStatus", "IF Status")); 
      adaNames(vecSize++) = "ifStatus";
    }
    if (flags->nominalSensitivityFlag) {
      for (i=0; i<4; i++) {
	ostringstream oss;
	oss << i;
	String tdName(String("nomSensitivity_") + String(oss.str()));
	td.addColumn(ArrayColumnDesc<Float> (tdName, "Nominal Sensitivity")); 
	adaNames(vecSize++) = tdName;
      }
    }
    if (flags->peculiarDelayFlag) {
      for (i=0; i<4; i++) {
	ostringstream oss;
	oss << i;
	String tdName(String("peculiarDelay_") + String(oss.str()));
	td.addColumn(ArrayColumnDesc<Float> (tdName, "Peculiar Delay")); 
	adaNames(vecSize++) = tdName;
      }
    }
    if (flags->peculiarPhaseFlag) {
      td.addColumn(ArrayColumnDesc<Double> ("peculiarPhase", "Peculiar Phase"));
      adaNames(vecSize++) = "peculiarPhase";
    }
    if (flags->totalDelayAtEpochFlag) {
      td.addColumn(ArrayColumnDesc<Double> ("ttlDelay","Total Delay at Epoch"));
      adaNames(vecSize++) = "ttlDelay";
    }
    if (flags->uFlag) {
      td.addColumn(ArrayColumnDesc<Float> ("u", "U")); 
      adaNames(vecSize++) = "u";
    }
    if (flags->vFlag) {
      td.addColumn(ArrayColumnDesc<Float> ("v", "V")); 
      adaNames(vecSize++) = "v";
    }
    if (flags->wFlag) {
      td.addColumn(ArrayColumnDesc<Float> ("w", "W")); 
      adaNames(vecSize++) = "w";
    }
    if (flags->bxFlag) {
      td.addColumn(ArrayColumnDesc<Double> ("bx", "BX")); 
      adaNames(vecSize++) = "bx";
    }
    if (flags->byFlag) {
      td.addColumn(ArrayColumnDesc<Double> ("by", "BY")); 
      adaNames(vecSize++) = "by";
    }
    if (flags->bzFlag) {
      td.addColumn(ArrayColumnDesc<Double> ("bz", "BZ")); 
      adaNames(vecSize++) = "bz";
    }
    if (flags->baFlag) {
      td.addColumn(ArrayColumnDesc<Float> ("ba", "BA")); 
      adaNames(vecSize++) = "ba";
    }
    if (flags->frontendTsysFlag) {
      for (i=0; i<4; i++) {
	ostringstream oss;
	oss << i;
	String tdName(String("feTsys_") + String(oss.str()));
	td.addColumn(ArrayColumnDesc<Float> (tdName, "Frontend Tsys")); 
	adaNames(vecSize++) = tdName;
      }
    }
    if (flags->backendTsysFlag) {
      for (i=0; i<4; i++) {
	ostringstream oss;
	oss << i;
	String tdName(String("beTsys_") + String(oss.str()));
	td.addColumn(ArrayColumnDesc<Float> (tdName, "Backend TSys")); 
	adaNames(vecSize++) = tdName;
      }
    }
  }
  
  adaNames.resize(vecSize, True);
  return;
}

void VlaTableSink::addColumns(TableDesc& td, const ContinuumCDAFlags* flags) {
  Int vecSize = 0;
  // Need to handle cross-correlation terms also.
  if (flags) {
    for (Int vlaIF=1; vlaIF<3; vlaIF++) {
      if (flags->headerFlag) {
	ostringstream oss;
	oss << "Header_IF_" << vlaIF;
	td.addColumn(ArrayColumnDesc<Short> (oss.str(), "header")); 
	cdaNames(vecSize++) = oss.str();
      }
      if (flags->AwAFlag) {
	ostringstream oss;
	oss << "AwA_IF_" << vlaIF;
	td.addColumn(ArrayColumnDesc<Float> (oss.str(), "A with A")); 
	cdaNames(vecSize++) = oss.str();
      }
      if (flags->CwCFlag) {
	ostringstream oss;
	oss << "CwC_IF_" << vlaIF;
	td.addColumn(ArrayColumnDesc<Float> (oss.str(), "C with C")); 
	cdaNames(vecSize++) = oss.str();
      }
   
      if (flags->AwCFlag) {
	ostringstream oss;
	oss << "AwC_IF_" << vlaIF;
	td.addColumn(ArrayColumnDesc<Float> (oss.str(), "A with C")); 
	cdaNames(vecSize++) = oss.str();
      }
      if (flags->CwAFlag) {
	ostringstream oss;
	oss << "CwA_IF_" << vlaIF;
	td.addColumn(ArrayColumnDesc<Float> (oss.str(), "C with A")); 
	cdaNames(vecSize++) = oss.str();
      }
    }
  }
  cdaNames.resize(vecSize, True);
  return;
}

void VlaTableSink::addColumns(TableDesc& td, 
			      const SpectralLineCDAFlags* flags) {
  Int vecSize = 0;
  if (flags) {
    for (Int vlaIF=0; vlaIF<4; vlaIF++) {
      if (flags->ifFlag[vlaIF]){
	if (flags->allChannelsFlag) {
	  ostringstream oss;
	  oss << "ChannelData_IF_" << vlaIF+1;
	  cdaNames(vecSize++) = oss.str();
	  td.addColumn(ArrayColumnDesc<Complex> (oss.str(), "Channel Data")); 
	} else {
	  for (Int k=0; k<flags->channelCount; k++) {
	    ostringstream oss;
	    oss << "IF_" << vlaIF << "_ChanData_" 
		<< flags->channelID[k];
	    cdaNames(vecSize++) = oss.str();
	    td.addColumn(ArrayColumnDesc<Complex> (oss.str(), "Channel Data")); 
	  }
	}
      }
    }
  }
  cdaNames.resize(vecSize, True);
  return;
}

void VlaTableSink::writeTable(const RCAFlags* flags) {
  TableRecord& dataRecord = dataRow.record();
  if (flags) {
    if (flags->dateMJADFlag) {
      dataRecord.define("MJAD", rca.dateMJAD);
    }
    if (flags->timeIATFlag) {
      dataRecord.define("IAT", rca.timeIAT);
    }
    if (flags->numberOfAntennasFlag) {
      dataRecord.define("Nants", rca.numberOfAntennas);
    }
  }
  cerr << rca.dateMJAD << " " << rca.timeIAT << " ";
  return;
}

void VlaTableSink::writeTable(const SDAFlags* flags) {
  TableRecord& dataRecord = dataRow.record();
  if (flags) {
    if (flags->subarrayIDFlag) {
      dataRecord.define("SubarrayID", sda.subarrayID);
    }
    if (flags->sourceNameFlag) {
      dataRecord.define("source", sda.sourceName);
    }
    cerr << sda.sourceName << endl;
    if (flags->sourceNameQualifierFlag) {
      dataRecord.define("qualifier", sda.sourceNameQualifier);
    }
    if (flags->observingProgramIDFlag) {
      dataRecord.define("protID", sda.observingProgramID);
    }
    if (flags->aipsIDFlag) {
      dataRecord.define("aipsID", sda.aipsID);
    }
    if (flags->observingModeFlag) {
      dataRecord.define("obsMode", sda.observingMode);
    }
    if (flags->calibratorCodeFlag) {
      dataRecord.define("calCode", sda.calibratorCode);
    }
    if (flags->submodeFlag) {
      dataRecord.define("subMode", sda.submode);
    }
    if (flags->arrayStatusInformationFlag) {
      dataRecord.define("arrayStatus", sda.arrayStatusInformation);
    }
    if (flags->numFreqChansPerBaselineFlag) {
      // oss << " " << sda.NumFreqChannelsPerBaseline;
      Vector<Short> numChans(4);
      uShort mask = 0x000f;
      numChans(3) = sda.numFreqChansPerBaseline & mask;             // Channel D is least significant
      numChans(2) = (sda.numFreqChansPerBaseline >> 4) & mask;
      numChans(1) = (sda.numFreqChansPerBaseline >> 8) & mask;
      numChans(0) = (sda.numFreqChansPerBaseline >> 12) & mask;     // Channel A is most significant
      dataRecord.define("chansPerBaseline", numChans);
    }
    if (flags->integrationTimeFlag) {
      dataRecord.define("integrationTime", sda.integrationTime);
    }
    if (flags->stopTimeFlag) {
      dataRecord.define("stop", sda.stopTime);
    }
    if (flags->startTimeFlag) {
      dataRecord.define("start", sda.startTime);
    }
    if (flags->raFlag) {
      dataRecord.define("RA", sda.ra);
    }
    if (flags->decFlag) {
      dataRecord.define("Dec", sda.dec);
    }
    if (flags->apparentRAFlag) {
      dataRecord.define("apparentRA", sda.apparentRA);
    }
    if (flags->apparentDecFlag) {
      dataRecord.define("apparentDec", sda.apparentDec);
    }
    for (Int i=0; i<4; i++) {
      if (flags->sslosFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("sslo_") + String(oss.str()));
	dataRecord.define(tdName, sda.sslos[i]);
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->skyFreqFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("skyfreq_") + String(oss.str()));
	dataRecord.define(tdName, sda.skyFreq[i]);
      }
    }
    if (flags->iatEndOfIntegrationFlag) {
      dataRecord.define("iatEnd", sda.iatEndOfIntegration);
    }
    if (flags->lstEndOfIntegrationFlag) {
      dataRecord.define("lstEnd", sda.lstEndOfIntegration);
    }
    if (flags->iat4GeoCalculationsFlag) {
      dataRecord.define("iat4GeoCalcs", sda.iat4GeoCalculations);
    }
    if (flags->currentSurfaceRefractivityFlag) {
      dataRecord.define("curSurfRefract", sda.currentSurfaceRefractivity);
    }
    if (flags->estimatedZenithAtmosPhaseFlag) {
      dataRecord.define("estZenithAtmosPhase", sda.estimatedZenithAtmosPhase);
    }
    if (flags->sinElevationFlag) {
      dataRecord.define("sinEl", sda.sinElevation);
    }
    if (flags->cosElevationFlag) {
      dataRecord.define("cosEl", sda.cosElevation);
    }
    if (flags->sinAzimuthFlag) {
      dataRecord.define("sinAz", sda.sinAzimuth);
    }
    if (flags->cosAzimuthFlag) {
      dataRecord.define("cosAz", sda.cosAzimuth);
    }
    if (flags->sinParallacticAngleFlag) {
      dataRecord.define("sinPA", sda.sinParallacticAngle);
    }
    if (flags->cosParallacticAngleFlag) {
      dataRecord.define("cosPA", sda.cosParallacticAngle);
    }
    if (flags->bandwidthCodesFlag) {
      dataRecord.define("bwCodes", sda.bandwidthCodes);
    }
    if (flags->frontendFilterCodesFlag) {
      dataRecord.define("feFilterCodes", sda.frontendFilterCodes);
    }
    if (flags->zeroSpacingFluxFlag) {
      dataRecord.define("zeroSpcFlux", sda.zeroSpacingFlux);
    }
    if (flags->uvLimitsFlag) {
      dataRecord.define("uvLimits", Vector<Float>(IPosition(2),
						  sda.uvLimits));
    }
    if (flags->arrayControlBitsFlag) {
      dataRecord.define("arrayCtrlBits", sda.arrayControlBits);
    }
    for (Int i=0; i<5; i++) {
      if (flags->weatherInfoFlag[i]) {
	ostringstream oss; oss << i;
	String tdName(String("weather_") + String(oss.str()));
	dataRecord.define(tdName, sda.weatherInfo[i]);
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->radialVelocityFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("rvel_") + String(oss.str()));
	dataRecord.define(tdName, sda.radialVelocity[i]);
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->lineRestFreqFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("restFreq_") + String(oss.str()));
	dataRecord.define(tdName, sda.lineRestFreq[i]);
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->velocityReferenceFrameFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("refFrame_") + String(oss.str()));
	dataRecord.define("source", sda.velocityReferenceFrame[i]);
      }
    }
    if (flags->correlatorModeFlag) {
      dataRecord.define("corrMode", sda.correlatorMode);
    }
    if (flags->arrayProcessorOptionsFlag) {
      dataRecord.define("arrayProcessor", sda.arrayProcessorOptions);
    }
    if (flags->epochYearFlag) {
      dataRecord.define("epochYr", sda.epochYear);
    }
    for (Int i=0; i<4; i++) {
      if (flags->channelOffsetsFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("chanOffsets_") + String(oss.str()));
	dataRecord.define(tdName, sda.channelOffsets[i]);
      }
    }
    for (Int i=0; i<4; i++) {
      if (flags->channelSepCodesFlag[i]) {
	ostringstream oss;
	oss << i;
	String tdName(String("chanSepCodes_") + String(oss.str()));
	dataRecord.define(tdName, sda.channelSepCodes[i]);
      }
    }
  } 
  return;
} 

void VlaTableSink::writeTable(const ADAFlags* flags){
  TableRecord& dataRecord = dataRow.record();
  if (flags) {
    Int i = 0;
    
// Define the data in a way the table system can handle it
//  Yes this is very kludgy, hey but it should work.
    uInt nants = rca.numberOfAntennas;
    uInt nIfs = 4;

    Matrix<Float>  beTsys(nants, nIfs);
    Matrix<Float>  feTsys(nants, nIfs);
    Vector<Float>  ba(nants);
    Vector<Double> bz(rca.numberOfAntennas);
    Vector<Double> by(rca.numberOfAntennas);
    Vector<Double> bx(rca.numberOfAntennas);
    Vector<Float>  w(rca.numberOfAntennas);
    Vector<Float>  v(rca.numberOfAntennas);
    Vector<Float>  u(rca.numberOfAntennas);
    Vector<Double> ttDelay(rca.numberOfAntennas);
    Vector<Double> peculiarPhase(rca.numberOfAntennas);
    Matrix<Float>  peculiarDelay(nants, nIfs);
    Matrix<Float>  nomSense(nants, nIfs);
    Vector<Int>    antennaControlBits(nants);
    Vector<Int>    dcsAddress(rca.numberOfAntennas);
    Vector<Int>    antennaID(rca.numberOfAntennas);
    Matrix<Int>    ifStatus(nants, uInt(2));
//
    for (i=0; i<rca.numberOfAntennas; i++) {
      for (Int j=0; j<4; j++) {
	beTsys(i,j) = ada[i].backendTsys[j];
	feTsys(i,j) = ada[i].frontendTsys[j];
	peculiarDelay(i,j) = ada[i].peculiarDelay[j];
	nomSense(i,j) = ada[i].nominalSensitivity[j];
      }
      for(Int j=0;j<2;j++) {
	ifStatus(i,j) = ada[i].ifStatus[j];
      }
      ba(i) = ada[i].ba;
      bz(i) = ada[i].bz;
      by(i) = ada[i].by;
      bx(i) = ada[i].bx;
      w(i) = ada[i].w;
      v(i) = ada[i].v;
      u(i) = ada[i].u;
      ttDelay(i) = ada[i].totalDelayAtEpoch;
      peculiarPhase(i) = ada[i].peculiarPhase;
      dcsAddress(i) = ada[i].dcsAddress;
      antennaID(i) = ada[i].dcsAddress;
      antennaControlBits(i) = ada[i].antennaControlBits;
    }

//  Now write stuff out
    if (flags->antennaIDFlag) {
      dataRecord.define("antID", antennaID);
    }
    if (flags->dcsAddressFlag) {
      dataRecord.define("dcsAddress", dcsAddress);
    }
    if (flags->antennaControlBitsFlag) {
      dataRecord.define("antCtrlBits", antennaControlBits);
    }
    if (flags->ifStatusFlag) {
      dataRecord.define("ifStatus", ifStatus);
    }
    if (flags->nominalSensitivityFlag) {
      for (i=0; i<4; i++) {
	ostringstream oss;
	oss << i;
	String tdName(String("nomSensitivity_") + String(oss.str()));
	dataRecord.define(tdName, nomSense.column(i));
      }
    }
    if (flags->peculiarDelayFlag) {
      for(i=0; i<4; i++) {
	ostringstream oss;
	oss << i;
	String tdName(String("peculiarDelay_") + String(oss.str()));
	dataRecord.define(tdName, peculiarDelay.column(i));
      }
    }
    if (flags->peculiarPhaseFlag) {
      dataRecord.define("peculiarPhase", peculiarPhase);
    }
    if (flags->totalDelayAtEpochFlag) {
      dataRecord.define("ttlDelay", ttDelay);
    }
    if (flags->uFlag) {
      dataRecord.define("u", u);
    }
    if (flags->vFlag) {
      dataRecord.define("v", v);
    }
    if (flags->wFlag) {
      dataRecord.define("w", w);
    }
    if (flags->bxFlag) {
      dataRecord.define("bx", bx);
    }
    if (flags->byFlag) {
      dataRecord.define("by", by);
    }
    if (flags->bzFlag) {
      dataRecord.define("bz", bz);
    }
    if (flags->baFlag) {
      dataRecord.define("ba", ba);
    }
    if (flags->frontendTsysFlag) {
      for(i=0; i<4; i++) {
	ostringstream oss;
	oss << i;
	String tdName(String("feTsys_") + String(oss.str()));
	dataRecord.define(tdName, feTsys.column(i));
      }
    }
    if (flags->backendTsysFlag) {
      for (i=0; i<4; i++){
	ostringstream oss;
	oss << i;
	String tdName(String("beTsys_") + String(oss.str()));
	dataRecord.define(tdName, beTsys.column(i));
      }
    }
  }
  return;
}

void VlaTableSink::writeTable(const ContinuumCDAFlags* flags){
  TableRecord& dataRecord = dataRow.record();
   if (flags) {
     for (Int vlaIF=0; vlaIF<2; vlaIF++) {
       if (flags->headerFlag) {
	 ostringstream oss;
	 oss << "Header_IF_" << vlaIF+1;
	 dataRecord.define(String(oss.str()),
			   cda[vlaIF]->correlatorData.continuum->header);
       }
       if (flags->AwAFlag) {
	 ostringstream oss;
	 oss << "AwA_IF_" << vlaIF+1;
	 dataRecord.define(String(oss.str()),
			   cda[vlaIF]->correlatorData.continuum->dataAwA);
       }
       if (flags->CwCFlag) {
	 ostringstream oss;
	 oss << "CwC_IF_" << vlaIF+1;
	 dataRecord.define(String(oss.str()),
			   cda[vlaIF]->correlatorData.continuum->dataAwA);
       }
       if (flags->AwCFlag) {
	 ostringstream oss;
	 oss << "AwC_IF_" << vlaIF+1;
	 dataRecord.define(String(oss.str()),
			   cda[vlaIF]->correlatorData.continuum->dataAwA);
       }
       if (flags->CwAFlag) {
	 ostringstream oss;
	 oss << "CwA_IF_" << vlaIF+1;
	 dataRecord.define(String(oss.str()),
			   cda[vlaIF]->correlatorData.continuum->dataAwA);
       }
     }
   }
   return;
}

void VlaTableSink::writeTable(const SpectralLineCDAFlags* flags) {
  TableRecord &dataRecord = dataRow.record();
  for (Int vlaIF=0; vlaIF<4; vlaIF++) {
    if (flags->ifFlag[vlaIF]) {
      if (flags->allChannelsFlag) {
	ostringstream oss;
	oss << "ChannelData_IF_" << vlaIF+1;
	dataRecord.define(String(oss.str()), cda[vlaIF]->correlatorData.spectralLine->data);
      } else {
	for (Int k=0; k<flags->channelCount; k++) {
	  ostringstream oss;
	  oss << "IF_" << vlaIF << "_ChanData_" 
	      << flags->channelID[k];
	  dataRecord.define(String(oss.str()), cda[vlaIF]->correlatorData.spectralLine->data);
	}
      }
    }
  }
  return;
}
// Local Variables: 
// compile-command: "gmake OPTLIB=1 VlaTableSink"
// End: 
