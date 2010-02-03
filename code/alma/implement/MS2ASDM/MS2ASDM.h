//# MS2ASDM.h 
//#
//#  ALMA - Atacama Large Millimeter Array
//#  (c) European Southern Observatory, 2002
//#  (c) Associated Universities Inc., 2002
//#  Copyright by ESO (in the framework of the ALMA collaboration),
//#  Copyright by AUI (in the framework of the ALMA collaboration),
//#  All rights reserved.
//#  
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#  
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#  
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <map>
#include <vector>
#include <casa/OS/Directory.h>

#include <alma/ASDM/ASDM.h>
#include <alma/ASDM/Tag.h>
#include <alma/ASDM/Complex.h>
#include <alma/ASDM/Frequency.h>
#include <alma/ASDM/Angle.h>
#include <alma/ASDM/AngularRate.h>
#include <alma/ASDM/Length.h>
#include <alma/ASDM/Temperature.h>
#include <alma/ASDM/ArrayTimeInterval.h>
#include <alma/ASDM/EntityRef.h>
#include <alma/Enumerations/CStokesParameter.h>
#include <alma/Enumerations/CAntennaType.h>
#include <alma/Enumerations/CBasebandName.h>
#include <alma/Enumerations/CNetSideband.h>
#include <alma/Enumerations/CFrequencyReferenceCode.h>
#include <alma/Enumerations/CReceiverBand.h>
#include <alma/Enumerations/CReceiverSideband.h>


#ifndef MSVIS_MS2ASDM_H
namespace casa { //# NAMESPACE CASA - BEGIN

#define MSVIS_MS2ASDM_H

// <summary>
// MS2ASDM provides functionalities to create an ASDM (ALMA science data model)
// from an existing MS
// </summary>

// <visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSet
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>

  class MS2ASDM : public ROMSColumns
{

 public:
  
  // construct from an MS
  MS2ASDM(MeasurementSet& ms);
  
  ~MS2ASDM();
  
  const String& showversion();
  
  void setBaseUid(const String& baseuid);
  
  const String& getBaseUid();
  
  const std::string& getCurrentUid();
  
  // return currentUid_p with all ":" and "/" characters replaced by "_"
  const std::string& getCurrentUidAsFileName();

  // set maximum duration of a subscan in seconds, 0. == no time limit
  void setSubScanDuration(const Double subscanDuration = 24.*3600.){
    subscanDuration_p = subscanDuration; }

  // get maximum duration of a subscan in seconds
  Double getSubScanDuration(){ return subscanDuration_p; }

  // set maximum duration of a Scheduling Block in seconds, 0. == no time limit
  void setSBDuration(const Double sBDuration = 1800.){
    schedBlockDuration_p = sBDuration; }

  // get maximum duration of a Scheduling Block in seconds
  Double getSBDuration(){ return schedBlockDuration_p; }

  void setDataAPCorrected(const Bool isCorrected = True){
    dataIsAPCorrected_p = isCorrected; }

  Bool dataIsAPCorrected(){ return dataIsAPCorrected_p; }

  void setObservatoryName(const String& telName){
    telName_p = telName;
  }

  void getObservatoryName( String& telName ){
    telName = telName_p;
  }

  // convert CASA Stokes to ASDM Stokes
  StokesParameterMod::StokesParameter ASDMStokesParameter( Stokes::StokesTypes s);

  // convert CASA antenna type string to ASDM antenna type enum
  AntennaTypeMod::AntennaType ASDMAntennaType( const String& type ); 

  // convert time in seconds to an array time
  ArrayTime ASDMArrayTime( const Double seconds ){ 
    return ArrayTime((long long) (floor(seconds*ArrayTime::unitsInASecond))); }

  // convert array time to time in seconds
  Double MSTimeSecs( const ArrayTime atime ){ 
    return (Double) atime.get() / (Double)ArrayTime::unitsInASecond; }

  asdm::Interval ASDMInterval( const Double seconds ){ 
    return asdm::Interval((long) (floor(seconds*ArrayTime::unitsInASecond))); }

  // convert a base band converter number to an ASDM base band name
  BasebandNameMod::BasebandName ASDMBBName( const Int bbcNo );

  // convert a MS net sideband no. to an ASDM enum
  NetSidebandMod::NetSideband ASDMNetSideBand( const Int netSideband );

  // set the receiver band and receiver sideband based on a representative frequency
  //   and the previously set observatory name telName_p
  Bool setRecBands( const asdm::Frequency refFreq, 
		    ReceiverBandMod::ReceiverBand& frequencyBand,
		    ReceiverSidebandMod::ReceiverSideband& receiverSideband);

  FrequencyReferenceCodeMod::FrequencyReferenceCode ASDMFreqRefCode( const MFrequency::Types refFrame ); 

  Unit unitASDMFreq(){ return Unit(String(asdm::Frequency::unit())); }

  Unit unitASDMAngle(){ return Unit(String(asdm::Angle::unit())); }

  Unit unitASDMAngularRate(){ return Unit(String(asdm::AngularRate::unit())); }

  Unit unitASDMLength(){ return Unit(String(asdm::Length::unit())); }

  Unit unitASDMTemp(){ return Unit(String(asdm::Temperature::unit())); }

  asdm::Complex ASDMComplex( casa::Complex x ){ return asdm::Complex(x.real(), x.imag()); }

  // write the entire ASDM from scratch
  Bool writeASDM(const String& asdmfile="", 
		 const String& datacolumn="data", 
		 const String& archiveid="S0", 
		 const String& rangeid="X1", 
		 const Bool verbose=True,
		 const Double maxSubscanDuration = 24.*3600.,
		 const Bool msDataIsAPCorrected=True
		 );

 private:
  // *** Private member functions ***

  Bool incrementUid(); // returns true if successful

  Bool setDirectory(const String& asdmfile);


  Bool writeStation();

  Bool writeAntenna();

  Bool writeSpectralWindow();

  Bool writeSource();

  Bool writePolarization();

  Bool writeCorrelatorMode(); // not called directly but optionally called by writeProcessor()
  Bool writeAlmaRadiometer(); // optionally called by writeProcessor()
  Bool writeHolography(); // optionally called by writeProcessor()

  Bool writeProcessor();

  Bool writeField();

  Bool writeReceiver();

  Bool writeFeed();

  Bool writeDataDescription();

  Bool writeSwitchCycle(); // not yet fully implemented

  Bool writeState();

  Bool writeSysCal();

  Bool writeConfigDescription();

  // Scheme
  // 1) We regard one MS Observation as a set of ASDM ExecBlocks modelled on 
  //    a single ASDM Scheduling Block
  // 2) ALMA ExecBlocks are at most 30 minutes long.
  //    If an MS Observation is more than 30 Minutes long, it is split up into 
  //    several ASDM ExecBlocks each referring to the same Scheduling Block.
  // 3) Each ASDM ExecBlock contains one or more ASDM Scans based on the MS scans 
  // 4) Each ASDM Scan contains one or more ASDM Subscans
  // 5) Each ASDM Subscan is at most subscanduration long. (external parameter)
  // 6) If an MS Scan is longer than subscanduration, it is split up into 
  //    several ASDM subscans.

  Bool writeSBSummaryAndExecBlockStubs(); // "stubs" because these tables will be completed later
                                          //  with information from the APDM
  Bool writeMainAndScanAndSubScan(const String& datacolumn);

  // write the Main binary data for one DataDescId/FieldId pair and one SubScan
  // (return number of integrations written and set the last three parameters in the list)
  Int writeMainBinSubScanForOneDDIdFIdPair(const Int theDDId, const Int theFieldId, 
					   const String& datacolumn, 
					   const uInt theScan, const uInt theSubScan,
					   const uInt startRow, const uInt endRow,
					   const asdm::Tag eBlockId,
					   int& datasize, asdm::EntityRef& dataOid, 
					   vector< asdm::Tag >& stateId);
    
  // *** Aux. methods ***

  // check if vector corrT already contains a stokes type equivalent to st
  Bool stokesTypePresent( const Vector< Int > corrT, const Stokes::StokesTypes st );

  // *** Member variables ***

  // Initialized* by ctors.  (Maintain order both here and in ctors.)
  MeasurementSet ms_p; // the measurement set from which the ASDM is filled

  asdm::ASDM* ASDM_p; // the new ASDM

  string asdmVersion_p; // the version of the new ASDM

  String baseUid_p;  // the part of the UID which is common to all elements of the ASDM,
                    // i.e. typically "uid://archiveid/rangeid/"

  uInt runningId_p; // counter for the tables written; starts at 1!
                    // used to construct the UIDs: uid = baseUid_p + (runningId_p converted to unpadded hex string)

  String currentUid_p; // the last used uid

  String telName_p; // the name of the observatory from first row of MS observation table

  Double subscanDuration_p; // maximum duration of a subscan in seconds

  Double schedBlockDuration_p; // maximum duration of a scheduling or exec block in seconds

  Bool dataIsAPCorrected_p; // true if the data in the selected MS data column is 
                            // AtmPhaseCorrectionMod::AP_CORRECTED, false if it is
                            // AtmPhaseCorrectionMod::AP_UNCORRECTED

  String asdmDir_p; // ASDM output directory name

  SimpleOrderedMap <String, asdm::Tag> asdmStationId_p;  
  SimpleOrderedMap <Int, asdm::Tag> asdmAntennaId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmSpectralWindowId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmPolarizationId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmProcessorId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmFieldId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmEphemerisId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmDataDescriptionId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmStateId_p;
  SimpleOrderedMap <uInt, asdm::Tag> asdmConfigDescriptionId_p; // maps from MS Main rows
  SimpleOrderedMap <Int, asdm::Tag> asdmSBSummaryId_p; // maps from MS Observation Id + 10000*SpwId
  SimpleOrderedMap <Double, asdm::Tag> asdmExecBlockId_p; // maps from MS Main timestamps 
  SimpleOrderedMap <Int, int> asdmFeedId_p; // ASDM feed id is not a Tag
  SimpleOrderedMap <Int, int> asdmSourceId_p; // neither is the source id

  vector< vector< Bool > > skipCorr_p; // skipCorr_p[j][PolId] indicates that correlation 
                                       // product j for POLARIZATION_ID PolId should not 
                                       // be written in the ASDM

};


} //# NAMESPACE CASA - END

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <alma/MS2ASDM/MS2ASDM.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC

#endif

