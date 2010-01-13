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
#include <alma/ASDM/Length.h>
#include <alma/Enumerations/CStokesParameter.h>
#include <alma/Enumerations/CAntennaType.h>
#include <alma/Enumerations/CBasebandName.h>
#include <alma/Enumerations/CNetSideband.h>
#include <alma/Enumerations/CFrequencyReferenceCode.h>


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

  // convert CASA Stokes to ASDM Stokes
  StokesParameterMod::StokesParameter ASDMStokesParameter( Stokes::StokesTypes s);

  // convert CASA antenna type string to ASDM antenna type enum
  AntennaTypeMod::AntennaType ASDMAntennaType( const String& type ); 

  // convert time in seconds to an array time
  ArrayTime ASDMArrayTime( const Double seconds ){ 
    return ArrayTime((long long) (floor(seconds*ArrayTime::unitsInASecond))); }

  asdm::Interval ASDMInterval( const Double seconds ){ 
    return asdm::Interval((long) (floor(seconds*ArrayTime::unitsInASecond))); }

  // convert a base band converter number to an ASDM base band name
  BasebandNameMod::BasebandName ASDMBBName( const Int bbcNo );

  // convert a MS net sideband no. to an ASDM enum
  NetSidebandMod::NetSideband ASDMNetSideBand( const Int netSideband );

  FrequencyReferenceCodeMod::FrequencyReferenceCode ASDMFreqRefCode( const MFrequency::Types refFrame ); 

  Unit ASDMFUnit(){ 
    if(asdm::Frequency::unit()=="hz"){ // correct for the bad capitalization
      return Unit("Hz");
    }
    else{
      return Unit(String(asdm::Frequency::unit()));
    }
  }

  Unit ASDMAUnit(){ return Unit(String(asdm::Angle::unit())); }

  Unit ASDMLUnit(){ return Unit(String(asdm::Length::unit())); }

  asdm::Complex ASDMComplex( casa::Complex x ){ return asdm::Complex(x.real(), x.imag()); }

  // write the entire ASDM from scratch
  Bool writeASDM(const String& asdmfile="", 
		 const String& datacolumn="data", 
		 const String& archiveid="S0", 
		 const String& rangeid="X1", 
		 const Bool verbose=True,
		 const Double subscanDuration = 24.*3600. 
		 );

 private:
  // *** Private member functions ***

  Bool incrementUid(); // returns true if successful

  Bool setDirectory(const String& asdmfile);


  Bool writeStation();

  Bool writeAntenna();

  Bool writeSpectralWindow();

  Bool writePolarization();

  Bool writeCorrelatorMode(); // not called directly but optionally called by writeProcessor()
  Bool writeAlmaRadiometer(); // optionally called by writeProcessor()
  Bool writeHolography(); // optionally called by writeProcessor()

  Bool writeProcessor();

  Bool writeField();

  Bool writeReceiver();

  Bool writeFeed();

  Bool writeDataDescription();

  Bool writeSwitchCycleDummy(); // write a trivial table with numStep==1

  Bool writeConfigDescription();

  Bool writeConfigDesc(); // obsolete

  Bool writeMain();

  // write the binary part of the ASDM main table
  Bool writeMainBin(const String& datacolumn);

  Bool writeMainBinForOneDDId(const Int dataDescId,
			   const String& datacolumn);

  // *** Aux. methods ***

  // check if vector corrT already contains a stokes type equivalent to st
  Bool stokesTypePresent( const Vector< Int > corrT, const Stokes::StokesTypes st );

  // *** Member variables ***

  // Initialized* by ctors.  (Maintain order both here and in ctors.)
  MeasurementSet ms_p; // the measurement set from which the ASDM is filled

  asdm::ASDM* ASDM_p; // the new ASDM

  String baseUid_p;  // the part of the UID which is common to all elements of the ASDM,
                    // i.e. typically "uid://archiveid/rangeid/"

  uInt runningId_p; // counter for the tables written; starts at 1!
                    // used to construct the UIDs: uid = baseUid_p + (runningId_p converted to unpadded hex string)

  String currentUid_p; // the last used uid

  Double subscanDuration_p; // maximum duration of a subscan in seconds

  String asdmDir_p; // ASDM output directory name

  String telName_p; // the name of the observatory from first row of MS observation table

  SimpleOrderedMap <String, asdm::Tag> asdmStationId_p;  
  SimpleOrderedMap <Int, asdm::Tag> asdmAntennaId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmSpectralWindowId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmPolarizationId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmProcessorId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmFieldId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmEphemerisId_p;
  SimpleOrderedMap <Int, asdm::Tag> asdmDataDescriptionId_p;

  SimpleOrderedMap <Int, int> asdmFeedId_p;

};


} //# NAMESPACE CASA - END

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <alma/MS2ASDM/MS2ASDM.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC

#endif

