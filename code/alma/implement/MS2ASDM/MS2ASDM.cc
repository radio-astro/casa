//# MS2ASDM.cc 
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
#include <alma/MS2ASDM/MS2ASDM.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/RefRows.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayOpsDiffShapes.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slice.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/File.h>
#include <casa/OS/HostInfo.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/System/AppInfo.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableInfo.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <tables/Tables/TiledDataStMan.h>
#include <tables/Tables/TiledStManAccessor.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>

#include <algorithm>
#include <casa/OS/Directory.h>
#include <alma/ASDM/ASDMAll.h>
#include <alma/ASDMBinaries/SDMDataObjectWriter.h>
#include <alma/ASDMBinaries/SDMDataObject.h>

using asdm::ASDM;
using asdm::TagType;
using namespace asdmbinaries;

namespace casa {
  
  MS2ASDM::MS2ASDM(MeasurementSet& ms) :
    ROMSColumns(ms),
    ms_p(ms),
    baseUid_p("uid://X0/X0/X"),
    runningId_p(0),
    currentUid_p("uid://X0/X0/X0"),
    // the Id to Tag maps
    asdmStationId_p(Tag()),
    asdmAntennaId_p(Tag()),
    asdmSpectralWindowId_p(Tag()),
    asdmPolarizationId_p(Tag()),
    asdmProcessorId_p(Tag()),
    asdmFieldId_p(Tag())
  {
    ASDM_p = new ASDM();
  }
  
  MS2ASDM::~MS2ASDM()
  {
    delete ASDM_p;
  }
  
  const String& MS2ASDM::showversion()
  {
    ASDM* myASDM = new ASDM();
    static String rstr;
    rstr = String((myASDM->getEntity()).getEntityVersion());
    delete myASDM;
    return rstr;
  }
  
  void MS2ASDM::setBaseUid(const String& baseuid){ baseUid_p = baseuid; }

  const String& MS2ASDM::getBaseUid(){ return baseUid_p; }

  const std::string& MS2ASDM::getCurrentUid(){ 
    static std::string str;
    str = string(currentUid_p.c_str());
    return str; 
  }

  const std::string& MS2ASDM::getCurrentUidAsFileName(){ 
    static std::string str;
    str = string(currentUid_p.c_str());
    std::replace( str.begin(), str.end(), ':', '_' );
    std::replace( str.begin(), str.end(), '/', '_' );

    // cout << str << endl;

    return str; 
  }

  Bool MS2ASDM::incrementUid()
  {
    static char cstr[20];
    runningId_p++;
    if(sprintf(cstr,"%x", runningId_p) > 0){
      currentUid_p = baseUid_p + String(cstr);
      return True;
    }
    else{
      runningId_p--;
      return False;
    }
  }

  Bool MS2ASDM::writeASDM(const String& asdmfile, const String& datacolumn, 
			  const String& archiveid, const String& rangeid, Bool verbose,
			  Double subscanDuration)
  {
    Bool rstat(True);

    LogIO os(LogOrigin("MS2ASDM", "writeASDM()"));
    os << LogIO::NORMAL << "Converting " << ms_p.tableName() << " to ASDM " << asdmfile
       << LogIO::POST;


    setBaseUid("uid://"+archiveid+"/"+rangeid+"/X");

    cout << "Base uid is " << getBaseUid() << endl;

    if(!incrementUid()){// need to increment before first use
      os << LogIO::SEVERE << "Error generating UID"
	 << LogIO::POST;      
      return False;
    }
      
    cout << " UID is " << getCurrentUid() << endl;

    // write the ASDM tables

    if(!writeStation()){
      return False;
    }

    if(!writeAntenna()){
      return False;
    }

    if(!writeSpectralWindow()){
      return False;
    }

    if(!writePolarization()){
      return False;
    }

    if(!writeConfigDesc()){
      return False;
    }

    // finish writing the ASDM non-binary data
    try{
      ASDM_p->toFile(asdmfile);
    }
    catch(ConversionException x){
      os << LogIO::SEVERE << "Error writing ASDM non-binary data:" << x.getMessage()
	 << LogIO::POST;      
      return False;
    }     

    // now the binary data
    if(!setDirectory(asdmfile)){
       return False;
    }
    
    setSubScanDuration(subscanDuration);
    if(!writeMainBin(datacolumn)){
      rstat = False;
    }

    return rstat;

  }

  Bool MS2ASDM::setDirectory(const String& asdmfile){

    LogIO os(LogOrigin("MS2ASDM", "setDirectory()"));

    // create ASDM output directories
    Directory asdmDir(asdmfile);
    // create if it doesn't exist
    try{
      asdmDir.isEmpty(); // throws if dir doesn't exist
    }
    catch(AipsError x){
      try{
	asdmDir.create(False); // do not overwrite
      }
      catch(AipsError y){
	os << LogIO::SEVERE << "Error creating ASDM top directory: " << y.getMesg()
	   << LogIO::POST;      
	return False;
      }
    }
    // create ASDM binary output directory
    Directory asdmDir2(asdmfile+"/ASDMBinary");
    try{
      asdmDir2.create(False); // do not overwrite
    }
    catch(AipsError x){
      os << LogIO::SEVERE << "Error creating ASDMBinary directory: " << x.getMesg()
	 << LogIO::POST;      
      return False;
    }
    asdmDir_p = asdmfile;
    return True;
  }


  Bool MS2ASDM::writeMainBinForOneDDId(const Int theDDId, const String& datacolumn){

    LogIO os(LogOrigin("MS2ASDM", "writeMainBinForOneDDId()"));

    try{

      // loop over MS main table rows
     
      uInt nMainTabRows = ms_p.nrow();

      unsigned int subscanNum = 1;
      Int theScan = 0;
      Bool haveState = True;

      for(uInt mainTabRow=0; mainTabRow<nMainTabRows; mainTabRow++){

	Int DDId = dataDescId()(mainTabRow);
	if(DDId != theDDId){ // skip all other Data Description Ids
	  continue;
	}
	
	if(scanNumber()(mainTabRow) != theScan){ // Scan number has changed
	  theScan = scanNumber()(mainTabRow);
	  // also reset the subscan number
	  subscanNum = 1;
	}

	os << LogIO::NORMAL << "Writing Main table entries for DataDescId " << DDId 
	   << ", Scan number " << theScan << LogIO::POST;

	if(haveState && stateId()(mainTabRow)>=0){
	  Int ssNum = state().subScan()(stateId()(mainTabRow)); // get the subscan number from the state table if available 
	  if(ssNum>=0){
	    //	    subscanNum == ssNum;
	    os << LogIO::NORMAL << "Valid sub scan number available in MS state table (" << ssNum 
	       << ") but use not yet implemented. Will use my own numbering." << LogIO::POST;
	    haveState = False;	    
	  }
	  else{
	    os << LogIO::WARN << "Invalid sub scan number in MS state table (" << ssNum 
	       << "). Will use my own numbering." << LogIO::POST;
	    haveState = False;
	  }
	}

	//////////////////////
	// Construct subscan == SDMDataObject
	//  One subscan consists of all the data taken within subscanDuration seconds starting at the first 
	//  timestamp of the MS main table.
      
	// Assume MS main table is sorted in time. 
	
	// Get first timestamp.
	Double subScanStartTime = time()(mainTabRow);
	Double subScanEndTime = time()(nMainTabRows-1); // by default there is no time limit on subscans  
	if(subscanDuration_p>0.){
	  subScanEndTime = subScanStartTime + subscanDuration_p;
	}

	// determine number of different timestamps in this subscan 
	unsigned int numTimestampsCorr = 0;
	unsigned int numTimestampsAuto = 0;
	uInt i=mainTabRow;
	while(i<nMainTabRows 
	      && scanNumber()(i)==theScan 
	      && time()(i)<=subScanEndTime
	      ){
	  DDId = dataDescId()(i);
	  if(DDId != theDDId){ // skip all other Data Description Ids
	    i++;
	    continue;
	  }

	  if(antenna1()(i)==antenna2()(i)){
	    numTimestampsAuto++;
	  }
	  else{
	    numTimestampsCorr++;
	  }
	  i++;
	} // end while
	subScanEndTime = time()(i-1);
	cout << " subscan number is " << subscanNum << endl;
	cout << "  subscan number of timestamps with crosscorrelations is " << numTimestampsCorr << endl;
	cout << "  subscan number of timestamps with autocorrelations is " << numTimestampsAuto << endl;
	cout << "  subscan end time is " << subScanEndTime << endl;

	// open disk file for subscan
	String subscanFileName = asdmDir_p+"/ASDMBinary/"+String(getCurrentUidAsFileName());
	cout << "  subscan filename is " << subscanFileName << endl;
      
	// make sure that this file doesn't exist yet
	ofstream ofs(subscanFileName.c_str());
      
	// set up SDMDataObjectWriter
      
	SDMDataObjectWriter sdmdow(&ofs, getCurrentUid()); // use default title

	unsigned long long startTime = (unsigned long long) floor(subScanStartTime);
	unsigned int execBlockNum = 1; // constant for all scans
	unsigned int scanNum = theScan; // ASDM scan numbering starts at 1
	if(scanNum == 0){
	  os << LogIO::WARN << "Scan Number is 0. Note that by convention scan numbers in ASDMs should start at 1." << LogIO::POST;
	}

	// determine actual number of baselines and antennas
	// assume that the first timestamp has complete information
	unsigned int numAutoCorrs = 0;
	unsigned int numBaselines = 0;
	unsigned int numAntennas = 0;
	vector<Int> ant;
	{
	  uInt i = mainTabRow;
	  Double thisTStamp = time()(i); 
	  while(i<nMainTabRows && time()(i)== thisTStamp){

	    DDId = dataDescId()(i);
	    if(DDId != theDDId){ // skip all other Data Description Ids
	      i++;
	      continue;
	    }

	    if(antenna1()(i) == antenna2()(i)){
	      numAutoCorrs++;
	    }
	    else{
	      numBaselines++;
	    }

	    Bool found = False;
	    for(uInt j=0;j<ant.size();j++){
	      if(antenna1()(i) == ant[j]){
		found = True;
		break;
	      }
	    }
	    if(!found){
	      ant.push_back(antenna1()(i));
	    }
	    found = False;
	    for(uInt j=0;j<ant.size();j++){
	      if(antenna2()(i) == ant[j]){
		found = True;
		break;
	      }
	    }
	    if(!found){
	      ant.push_back(antenna2()(i));
	    }
	    i++;
	  }
	}
	numAntennas = ant.size();
	if(numAntennas!=antenna().nrow()){
	  os << LogIO::WARN << "    Number of antennas in ANTENNA table (" << antenna().nrow() 
	     << ") is not the same as number of antennas present in MAIN table (" << ant.size()
	     << ")." << LogIO::POST;      
	}
	if(numAntennas!=numAutoCorrs && numAutoCorrs>0){
	  os << LogIO::WARN << "    Number of antennas in ANTENNA table (" << antenna().nrow() 
	     << ") is not the same as number of autocorrelations present in MAIN table (" << ant.size()
	     << ")." << LogIO::POST;      
	}
	else if(numAutoCorrs==0){
	  os << LogIO::NORMAL << "    No autocorrelations present in MAIN table." << LogIO::POST;      
	}
	

	CorrelationMode correlationMode;
	if(numAutoCorrs==0){
	  correlationMode = CorrelationModeMod::CROSS_ONLY;
	}
	else if(numBaselines>0){
	  correlationMode = CorrelationModeMod::CROSS_AND_AUTO;
	}
	else{
	  correlationMode = CorrelationModeMod::AUTO_ONLY;
	}
	// if datacolumn is FLOAT_DATA make correlation mode == AUTO as well!

	OptionalSpectralResolutionType spectralResolution = SpectralResolutionTypeMod::FULL_RESOLUTION; 
	// alternatives: BASEBAND_WIDE, CHANNEL_AVERAGE
	os << LogIO::NORMAL << "    Assuming data is of spectral resolution type \"FULL RESOLUTION\"." << LogIO::POST;      

	vector< AtmPhaseCorrection >  apc;
	vector< SDMDataObject::Baseband > basebands; // ???

	// construct spectral window and basedband vectors

	vector<SDMDataObject::SpectralWindow> spectralWindows;
	
	// for creating a Spectral Window
	vector<StokesParameter> crossPolProducts;
	uInt PolId = dataDescription().polarizationId()(theDDId);
	uInt numStokes = polarization().numCorr()(PolId);
	Array<Int> corrT = polarization().corrType()(PolId);
	// the following will be placed into a separate method
	for(uInt i=0; i<numStokes && i<corrT.nelements(); i++){
	  Stokes::StokesTypes t = Stokes::type(corrT(IPosition(1,i)));
	  StokesParameter s = ASDMStokesParameter(t);
	  crossPolProducts.push_back(s);
	}
	    
	vector<StokesParameter> sdPolProduct;
	float scaleFactor = 1.;
	uInt spwId = dataDescription().spectralWindowId()(theDDId);
	unsigned int numSpectralPoint = spectralWindow().numChan()(spwId);
	unsigned int numBin = 1; // number of switch cycles
	NetSideband sideband = CNetSideband::from_int(spectralWindow().netSideband()(spwId));
	
	SDMDataObject::SpectralWindow spw(crossPolProducts,
					  sdPolProduct,
					  scaleFactor,
					  numSpectralPoint,
					  numBin,
					  sideband);
	
	spectralWindows.push_back(spw);

	
	SDMDataObject::Baseband bband(BasebandNameMod::NOBB, spectralWindows); // how to select name ???? -> Francois
	// note: for ALMA only four basebands exist, not easy to find baseband for a given SPW

	basebands.push_back(bband);

	// construct binary parts for dataStruct
	unsigned int bpFlagsSize = numSpectralPoint * numStokes * (numTimestampsCorr+numTimestampsAuto);
	vector<AxisName> bpFlagsAxes;
	bpFlagsAxes.push_back(AxisNameMod::SPP); /// order: inner part of loop should be last!!!!!!
	bpFlagsAxes.push_back(AxisNameMod::POL);
	SDMDataObject::BinaryPart bpFlags(bpFlagsSize, bpFlagsAxes);
	cout << "FlagsSize " << bpFlagsSize << endl;

	unsigned int bpTimesSize = 0; // only needed for data blanking
	//unsigned int bpTimesSize = numTimestampsCorr+numTimestampsAuto; 
	vector<AxisName> bpTimesAxes;
	bpTimesAxes.push_back(AxisNameMod::TIM);
	SDMDataObject::BinaryPart bpActualTimes(bpTimesSize, bpTimesAxes);
	cout << "TimesSize " << bpTimesSize << endl;

	unsigned int bpDurSize = 0; // only needed for data blanking
	//	unsigned int bpDurSize = numTimestampsCorr+numTimestampsAuto;
	vector<AxisName> bpDurAxes;
	bpDurAxes.push_back(AxisNameMod::TIM);
	SDMDataObject::BinaryPart bpActualDurations(bpDurSize, bpDurAxes);
	cout << "DurSize " << bpDurSize << endl;

	unsigned int bpLagsSize = 0; // not filled for the moment (only useful if LAG_DATA column present) -> Francois 
	vector<AxisName> bpLagsAxes;
	bpLagsAxes.push_back(AxisNameMod::SPP); // ******
	SDMDataObject::ZeroLagsBinaryPart bpZeroLags(bpLagsSize, bpLagsAxes, CorrelatorTypeMod::FXF); // how to determine?
	cout << "LagsSize " << bpLagsSize << endl;

	unsigned int bpCrossSize = numSpectralPoint * numStokes * numTimestampsCorr * 2; // real + imag
	vector<AxisName> bpCrossAxes;
	bpCrossAxes.push_back(AxisNameMod::TIM);
	bpCrossAxes.push_back(AxisNameMod::BAL);
	bpCrossAxes.push_back(AxisNameMod::SPP); 
	bpCrossAxes.push_back(AxisNameMod::POL);
	SDMDataObject::BinaryPart bpCrossData(bpCrossSize, bpCrossAxes);
	cout << "CrossSize " << bpCrossSize << endl;

	unsigned int bpAutoSize = numSpectralPoint * numStokes * numTimestampsAuto;
	vector<AxisName> bpAutoAxes;
	bpAutoAxes.push_back(AxisNameMod::TIM);
	bpAutoAxes.push_back(AxisNameMod::ANT);
	bpAutoAxes.push_back(AxisNameMod::SPP); 
	bpAutoAxes.push_back(AxisNameMod::POL);
	SDMDataObject::AutoDataBinaryPart bpAutoData(bpAutoSize, bpAutoAxes, False); // not normalised
	cout << "AutoSize " << bpAutoSize << endl;
	
	SDMDataObject::DataStruct dataStruct( apc, basebands, bpFlags, bpActualTimes, bpActualDurations, 
					      bpZeroLags, bpCrossData, bpAutoData);	 

	// Parameters of dataStruct:
	//     	apc 	        a vector of AtmPhaseCorrection. If apc is not relevant pass an empty vector.
	//     	basebands 	a vector of Baseband.
	//     	flags 	        a BinaryPart object describing the flags. If flags is not relevant pass an 
	//                      empty BinaryPart object.
	//     	actualTimes 	a BinaryPart object describing the actual times. If actualTimes is not 
        //                      relevant pass an empty BinaryPart object.
	//     	actualDurations a BinaryPart object describing the actual durations. If actualDurations is 
        //                      not relevant pass an empty BinaryPart object.
	//     	zeroLags 	a ZeroLagsBinaryPart object describing the zero lags. If zeroLags is not 
        //                      relevant pass an empty ZeroLagsBinaryPart object.
	//     	crossData 	a BinaryPart object describing the cross data. If crossData is not relevant 
        //                      pass an empty BinaryPart object.
	//     	autoData 	an AutoDataBinaryPart object describing the auto data. If autoData is not 
        //                      relevant pass an empty AutoDataBinaryPart object. 
	
	
	// Write the global header.
	sdmdow.corrDataHeader(startTime,
			      getCurrentUid(), // execBlockUID
			      execBlockNum,
			      scanNum,
			      subscanNum,
			      numAntennas,        
			      correlationMode,    // the correlation mode.
			      spectralResolution, // the spectral resolution.
			      dataStruct);        // the description of the structure of the binary data.
	
	//////////////////////////////////////////////////////
	// write the integrations until timestamp exceeds limit
	unsigned int integrationNum = 1;
	while(mainTabRow < nMainTabRows 
	      && scanNumber()(mainTabRow) == theScan 
	      && time()(mainTabRow)<=subScanEndTime){
	  
	  DDId = dataDescId()(mainTabRow);
	  if(DDId != theDDId){ // skip all other Data Description Ids
	    mainTabRow++;
	    continue;
	  }

	  unsigned long long timev = (unsigned long long) floor((time()(mainTabRow))*1E9); // what units? nanoseconds
	  unsigned long long intervalv = (unsigned long long) floor(interval()(mainTabRow)*1E9);
	  vector< unsigned long > flags;
	  vector< long long > actualTimes;
	  vector< long long > actualDurations;
	  vector< float > zeroLags; // LAG_DATA, optional column, not used for the moment
	  vector< float > crossData;
	  // vector< short > crossData;
	  // vector< int > crossData; // standard case for ALMA
	  vector< float > autoData;	 

	  ////////////////////////////////////////////////////////
	  // fill data, time, and flag vectors for this timestamp
	  Double theTStamp = time()(mainTabRow);

	  // SORT the data by baseline and antenna resp.!!!!!!!!!!!!

	  while(mainTabRow < nMainTabRows 
	      && scanNumber()(mainTabRow) == theScan 
	      && time()(mainTabRow)==theTStamp){

	    DDId = dataDescId()(mainTabRow);
	    if(DDId != theDDId){ // skip all other Data Description Ids
	      mainTabRow++;
	      continue;
	    }

	    Matrix<Complex> dat;
	    Matrix<Bool> flagsm;
	    if(datacolumn == "MODEL"){
	      dat.reference(modelData()(mainTabRow));
	    }
	    else if(datacolumn == "CORRECTED"){
	      dat.reference(correctedData()(mainTabRow));
	    }
	    else{
	      dat.reference(data()(mainTabRow));
	    }
	    flagsm.reference(flag()(mainTabRow));

	    if(antenna1()(mainTabRow) == antenna2()(mainTabRow)){
	      Complex x;  
	      float a;
	      for(uInt i=0; i<numSpectralPoint; i++){
		for(uInt j=0; j<numStokes; j++){
		  x = dat(j,i);
		  a = x.real();
		  autoData.push_back( a );
		}
	      }
	    }
	    else{
	      Complex x;  
	      float a,b;
	      // int a,b;
	      for(uInt i=0; i<numSpectralPoint; i++){
		for(uInt j=0; j<numStokes; j++){
		  x = dat(j,i);
		  a = x.real();
		  b = x.imag();
		  crossData.push_back( a );
		  crossData.push_back( b );
		}
	      }
	    }	
	    unsigned long ul;
	    for(uInt i=0; i<numSpectralPoint; i++){
	      for(uInt j=0; j<numStokes; j++){
		if(flagRow()(mainTabRow)){
		  ul = 1;
		}
		else{
		  ul = flagsm(j,i);
		}
		flags.push_back( ul );
	      }
	    }
	    // the following two lines only needed for data blanking
	    //	    actualTimes.push_back((long long)floor(time()(mainTabRow))*1000.);
	    //	    actualDurations.push_back((long long)floor(interval()(mainTabRow))*1000.);
	    mainTabRow++;
	  }// end while same timestamp
	
// 	  cout << "Sizes: " << endl;
// 	  cout << "   flags " << flags.size() << endl;
// 	  cout << "   actualTimes " << actualTimes.size() << endl;
// 	  cout << "   actualDurations " << actualDurations.size() << endl;
// 	  cout << "   zeroLags " << zeroLags.size() << endl;
// 	  cout << "   crossData " << crossData.size() << endl;
// 	  cout << "   autoData " << autoData.size() << endl;

	  sdmdow.addIntegration(integrationNum,    // integration's index.
				timev,              // midpoint
				intervalv,          // time interval
				flags,             // flags binary data 
				actualTimes,       // actual times binary data      
				actualDurations,   // actual durations binary data          
				zeroLags,          // zero lags binary data                 
				crossData,    // cross data (can be short or int)  
				autoData);         // single dish data.                 
	  integrationNum++;

	  // (Note: subintegrations are used only for channel averaging to gain time res. by sacrificing spec. res.)

	  mainTabRow++;
	} // end while
      
	sdmdow.done();

	ofs.close();
      
	// end write subscan
      
	// next subscan
	subscanNum++;
	incrementUid();
            
      } // end loop over main table rows
      
    }
    catch(asdmbinaries::SDMDataObjectWriterException x){
      os << LogIO::SEVERE << "Error creating ASDM: " << x.getMessage()
	 << LogIO::POST;      
      return False;
    }
    catch(asdmbinaries::SDMDataObjectException x){
      os << LogIO::SEVERE << "Error creating ASDM: " << x.getMessage()
	 << LogIO::POST;      
      return False;
    }
    catch(AipsError y){
      os << LogIO::SEVERE << "Error creating ASDM: " << y.getMesg()
	 << LogIO::POST;      
      return False;
    }      
    catch(std::string z){
      os << LogIO::SEVERE << "Error creating ASDM: " << z
	 << LogIO::POST;      
      return False;
    }      
    catch(std::exception zz){
      os << LogIO::SEVERE << "Error creating ASDM: " << zz.what()
	 << LogIO::POST;      
      return False;
    }      

    return True;
  } // end writeMainBinForOneDDId


  Bool MS2ASDM::writeMainBin(const String& datacolumn){

    LogIO os(LogOrigin("MS2ASDM", "writeMainBin()"));

    Bool rstat = True;

    // check if datacolumn exists

    if( (datacolumn == "CORRECTED" && !ms_p.tableDesc().isColumn("CORRECTED_DATA"))
	|| (datacolumn == "MODEL" && !ms_p.tableDesc().isColumn("MODEL_DATA"))
	|| (datacolumn == "DATA" && !ms_p.tableDesc().isColumn("DATA"))
	){
      os << LogIO::SEVERE << "Error: column " << datacolumn << " does not exist in "
	 << ms_p.tableName() << LogIO::POST;      
      return False;
    }
    else{
      os << LogIO::NORMAL << "Using column " << datacolumn << "." << LogIO::POST;      
    }      

    for(uInt ddId=0; ddId<dataDescription().nrow(); ddId++){
      if(!writeMainBinForOneDDId(ddId, datacolumn)){
	rstat = False;
	break;
      }
    }

    return rstat;
  } // end writeMainBin

  /////////////////////////////////////
  //   // Pattern for wrting a non-binary table:
  //   Bool MS2ASDM::writeAntenna(){ // create asdm antenna table and fill antenna id map
  //     LogIO os(LogOrigin("MS2ASDM", "writeAntenna()"));
  
  //     Bool rstat = True;
  
  //     asdm::AntennaTable& tT = ASDM_p->getAntenna();
  
  //     asdm::AntennaRow* tR = 0;
  
  //     //    tR = tT.newRow();
  
  //     tT.add(tR);
  
  //     EntityId theUid(getCurrentUid());
  //     Entity ent = tT.getEntity();
  //     ent.setEntityId(theUid);
  //     tT.setEntity(ent);
  //     os << LogIO::NORMAL << "Filled Station table " << getCurrentUid() << " ... " << LogIO::POST;
  //     incrementUid();
  
  //     return rstat;
  //   }

  Bool MS2ASDM::writeStation(){ // create asdm station table and fill station id map
    LogIO os(LogOrigin("MS2ASDM", "writeStation()"));

    Bool rstat = True;

    asdm::StationTable& tT = ASDM_p->getStation();

    asdm::StationRow* tR = 0;

    // MS has no station table; get information from the antenna table

    for(uInt irow=0; irow<antenna().nrow(); irow++){

      // parameters for the new station row
      StationTypeMod::StationType type = StationTypeMod::ANTENNA_PAD; 
      // (alternatives are MAINTENANCE_PAD and WEATHER_STATION)

      // parameters of the new row
      string name = antenna().station()(irow).c_str();

      vector<Length > position; // take the antenna position (!), (later set the ASDM antenna postion to 0,0,0)
      Vector<Double > v; // aux. vector
      v.reference(antenna().position()(irow));
      for(uInt i=0; i<v.size(); i++){
	position.push_back(Length(v[i]));
      }

      Tag stationId;
      String sId = antenna().station()(irow);
      tR = tT.newRow(name, position, type);
      
      asdm::StationRow* tR2 = 0;

      tR2 = tT.add(tR);
      if(tR2 == tR){ // adding this row caused a new tag to be defined
	// enter tag into the map
	asdmStationId_p.define(String(name), tR->getStationId());
      }
      else{
	os << LogIO::SEVERE << "Duplicate station in MS Antenna table: " << irow << LogIO::POST;
	return False;
      }

    } // end for irow

    EntityId theUid(getCurrentUid());
    Entity ent = tT.getEntity();
    ent.setEntityId(theUid);
    tT.setEntity(ent);
    os << LogIO::NORMAL << "Filled Station table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }

  Bool MS2ASDM::writeAntenna(){ // create asdm antenna table and fill antenna id map
    LogIO os(LogOrigin("MS2ASDM", "writeAntenna()"));

    Bool rstat = True;

    asdm::AntennaTable& tT = ASDM_p->getAntenna();

    asdm::AntennaRow* tR = 0;

    for(uInt irow=0; irow<antenna().nrow(); irow++){

      // parameters of the new row
      string name = antenna().name()(irow).c_str();
      AntennaMakeMod::AntennaMake antennaMake = AntennaMakeMod::UNDEFINED; // no info in MS (presently)
      AntennaTypeMod::AntennaType antennaType = ASDMAntennaType( antenna().type()(irow) );
      Length dishDiameter = Length( antenna().dishDiameter()(irow) );

      vector<Length > position;
      for(uInt i=0; i<3; i++){
	position.push_back(Length(0.)); // the antenna position in the ASDM is the sum of station pos and antenna pos.
      }

      vector<Length > offset;
      Vector<Double > v; // aux. vector
      v.reference(antenna().offset()(irow));
      for(uInt i=0; i<v.size(); i++){
	offset.push_back(Length(v[i]));
      }

      ArrayTime atime = ASDMArrayTime(time()(0)); // use start time of MS 

      Tag stationId;
      String sId = antenna().station()(irow); // the station name
      if(asdmStationId_p.isDefined(sId)){
	stationId = asdmStationId_p(sId);
      }
      else{
	os << LogIO::SEVERE << "Internal error: No station ID defined for station name " << sId << LogIO::POST;
	return False;
      }
      
      tR = tT.newRow(name, antennaMake, antennaType, dishDiameter, position, offset, atime, stationId);
      
      asdm::AntennaRow* tR2 = 0;

      tR2 = tT.add(tR);
      if(tR2 == tR){ // adding this row caused a new tag to be defined
	// enter tag into the map
	asdmAntennaId_p.define(irow, tR->getAntennaId());
      }
      else{
	os << LogIO::WARN << "Duplicate row in MS Antenna table :" << irow << LogIO::POST;
      }
    }

    EntityId theUid(getCurrentUid());
    Entity ent = tT.getEntity();
    ent.setEntityId(theUid);
    tT.setEntity(ent);
    os << LogIO::NORMAL << "Filled Antenna table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }

  Bool MS2ASDM::writeSpectralWindow(){
    LogIO os(LogOrigin("MS2ASDM", "writeSpectralWindow()"));

    Bool rstat = True;

    asdm::SpectralWindowTable& tT = ASDM_p->getSpectralWindow();

    asdm::SpectralWindowRow* tR = 0;

    for(uInt irow=0; irow<spectralWindow().nrow(); irow++){

      // parameters of the new row
      BasebandNameMod::BasebandName basebandName = BasebandNameMod::NOBB;
      if(!spectralWindow().bbcNo().isNull()){ // BBC_NO is an optional MS column
	basebandName = ASDMBBName( spectralWindow().bbcNo()(irow) );
      }
      NetSidebandMod::NetSideband netSideband = ASDMNetSideBand( spectralWindow().netSideband()(irow) );
      int numChan = spectralWindow().numChan()(irow);

      Unit funit("Hz"); // ASDM frequency values need to be in Hz

      Frequency refFreq  = Frequency( spectralWindow().refFrequencyQuant()(irow).get(funit).getValue() );
      SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode = SidebandProcessingModeMod::NONE;
      Frequency totBandwidth = Frequency( spectralWindow().totalBandwidthQuant()(irow).get(funit).getValue() );
      WindowFunctionMod::WindowFunction windowFunction = WindowFunctionMod::UNIFORM;

      tR = tT.newRow(basebandName, netSideband, numChan, refFreq, sidebandProcessingMode, totBandwidth, windowFunction);

      Vector< Quantum<Double> > v;
      vector< Frequency > chanFreqArray;
      v.reference( spectralWindow().chanFreqQuant()(irow) );
      for(uInt i=0; i<v.nelements(); i++){ 
	chanFreqArray.push_back( Frequency( v[i].get(funit).getValue() ) );
      }
      tR->setChanFreqArray(chanFreqArray);

      vector< Frequency > chanWidthArray;
      v.reference( spectralWindow().chanWidthQuant()(irow) );
      for(uInt i=0; i<v.nelements(); i++){ 
	chanWidthArray.push_back( Frequency( v[i].get(funit).getValue() ) );
      }
      tR->setChanWidthArray(chanWidthArray);

      vector< Frequency > effectiveBwArray;
      v.reference( spectralWindow().effectiveBWQuant()(irow) );
      for(uInt i=0; i<v.nelements(); i++){ 
	effectiveBwArray.push_back( Frequency( v[i].get(funit).getValue() ) );
      }
      tR->setEffectiveBwArray(effectiveBwArray);

      vector< Frequency > resolutionArray;
      v.reference( spectralWindow().resolutionQuant()(irow) );
      for(uInt i=0; i<v.nelements(); i++){ 
	resolutionArray.push_back( Frequency( v[i].get(funit).getValue() ) );
      }      
      tR->setResolutionArray( resolutionArray );

      MFrequency::Types refFrame = MFrequency::castType(spectralWindow().measFreqRef()(irow));
      FrequencyReferenceCodeMod::FrequencyReferenceCode measFreqRef = ASDMFreqRefCode( refFrame );
      tR->setMeasFreqRef( measFreqRef );

      tR->setFreqGroup( spectralWindow().freqGroup()(irow) );

      tR->setFreqGroupName( string( spectralWindow().freqGroupName()(irow).c_str() ) );	

      if(!spectralWindow().dopplerId().isNull()){ // DOPPLER_ID is an optional MS column
	tR->setDopplerId( spectralWindow().dopplerId()(irow) );
      }

      // add the row to the table
      asdm::SpectralWindowRow* tR2 = 0;
      tR2 = tT.add(tR);

      if(tR2 == tR){ // adding this row caused a new tag to be defined
	// enter tag into the map
	asdmSpectralWindowId_p.define(irow, tR->getSpectralWindowId());
      }
      else{
	os << LogIO::WARN << "Duplicate row in MS Spectral Window table :" << irow << LogIO::POST;
      }
    } // end loop over MS SPW table

    EntityId theUid(getCurrentUid());
    Entity ent = tT.getEntity();
    ent.setEntityId(theUid);
    tT.setEntity(ent);
    os << LogIO::NORMAL << "Filled SpectralWindow table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }

  Bool MS2ASDM::writePolarization(){
    LogIO os(LogOrigin("MS2ASDM", "writePolarization()"));

    Bool rstat = True;
    asdm::PolarizationTable& tT = ASDM_p->getPolarization();

    asdm::PolarizationRow* tR = 0;

    for(uInt irow=0; irow<polarization().nrow(); irow++){

      // parameters of the new row

      vector< StokesParameterMod::StokesParameter > corrTypeV;
      vector< vector< PolarizationTypeMod::PolarizationType > > corrProduct;
      
      Vector< Int > v; // aux. vector
      v.reference( polarization().corrType()(irow) );
      for(uInt i=0; i<v.nelements(); i++){
	Stokes::StokesTypes st = static_cast<Stokes::StokesTypes>(v[i]);
	if(st == Stokes::LR){ // only add if RL is not also present
	  if(!stokesTypePresent(v, Stokes::RL)){
	    corrTypeV.push_back( ASDMStokesParameter( st ) );
	  }
	}
	else if(st == Stokes::YX){ // only add if XY is not also present
	  if(!stokesTypePresent(v, Stokes::XY)){
	    corrTypeV.push_back( ASDMStokesParameter( st ) );
	  }
	}
	else{
	  corrTypeV.push_back( ASDMStokesParameter( st ) );
	}
      }
      int numCorr = corrTypeV.size(); 

      for(uInt i=0; i<corrTypeV.size(); i++){
	vector< PolarizationTypeMod::PolarizationType > w;
	switch(corrTypeV[i]){
	case StokesParameterMod::RR:
	  w.push_back(PolarizationTypeMod::R);
	  w.push_back(PolarizationTypeMod::R);
	  break;
	case StokesParameterMod::RL:
	  w.push_back(PolarizationTypeMod::R);
	  w.push_back(PolarizationTypeMod::L);
	  break;
	case StokesParameterMod::LR:
	  w.push_back(PolarizationTypeMod::L);
	  w.push_back(PolarizationTypeMod::R);
	  break;
	case StokesParameterMod::LL:
	  w.push_back(PolarizationTypeMod::L);
	  w.push_back(PolarizationTypeMod::L);
	  break;
	case StokesParameterMod::XX:
	  w.push_back(PolarizationTypeMod::X);
	  w.push_back(PolarizationTypeMod::X);
	  break;
	case StokesParameterMod::XY:
	  w.push_back(PolarizationTypeMod::X);
	  w.push_back(PolarizationTypeMod::Y);
	  break;
	case StokesParameterMod::YX:
	  w.push_back(PolarizationTypeMod::Y);
	  w.push_back(PolarizationTypeMod::X);
	  break;
	case StokesParameterMod::YY:
	  w.push_back(PolarizationTypeMod::Y);
	  w.push_back(PolarizationTypeMod::Y);
	  break;
	case StokesParameterMod::RX:
	  w.push_back(PolarizationTypeMod::R);
	  w.push_back(PolarizationTypeMod::X);
	  break;
	case StokesParameterMod::RY:
	  w.push_back(PolarizationTypeMod::R);
	  w.push_back(PolarizationTypeMod::Y);
	  break;
	case StokesParameterMod::LX:
	  w.push_back(PolarizationTypeMod::L);
	  w.push_back(PolarizationTypeMod::X);
	  break;
	case StokesParameterMod::LY:
	  w.push_back(PolarizationTypeMod::L);
	  w.push_back(PolarizationTypeMod::Y);
	  break;
	case StokesParameterMod::XR:
	  w.push_back(PolarizationTypeMod::X);
	  w.push_back(PolarizationTypeMod::R);
	  break;
	case StokesParameterMod::XL:
	  w.push_back(PolarizationTypeMod::X);
	  w.push_back(PolarizationTypeMod::L);
	  break;
	case StokesParameterMod::YR:
	  w.push_back(PolarizationTypeMod::Y);
	  w.push_back(PolarizationTypeMod::R);
	  break;
	case StokesParameterMod::YL:
	  w.push_back(PolarizationTypeMod::Y);
	  w.push_back(PolarizationTypeMod::L);
	  break;
	default:
	  os << LogIO::SEVERE << "Cannot store correlation product for stokes parameter " << CStokesParameter::name(corrTypeV[i]) << LogIO::POST;
	  rstat = False;
	  break;
	}	  
	corrProduct.push_back(w);
      } // end loop over corrTypeV

      tR = tT.newRow(numCorr, corrTypeV, corrProduct);

      bool flagRow = polarization().flagRow()(irow);
      tR->setFlagRow(flagRow);

      // add the row to the table
      asdm::PolarizationRow* tR2 = 0;
      tR2 = tT.add(tR);

      if(tR2 == tR){ // adding this row caused a new tag to be defined
	// enter tag into the map
	asdmPolarizationId_p.define(irow, tR->getPolarizationId());
      }
      else{
	os << LogIO::WARN << "Duplicate row in MS Polarization table :" << irow << LogIO::POST;
      }
    } // end loop over MS Pol table

    EntityId theUid(getCurrentUid());
    Entity ent = tT.getEntity();
    ent.setEntityId(theUid);
    tT.setEntity(ent);
    os << LogIO::NORMAL << "Filled Polarization table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }

  Bool MS2ASDM::writeProcessor(){
    LogIO os(LogOrigin("MS2ASDM", "writeProcessor()"));

    Bool rstat = True;
    asdm::ProcessorTable& tT = ASDM_p->getProcessor();

    asdm::ProcessorRow* tR = 0;

    //    tR = tT.newRow();

    tT.add(tR);

    EntityId theUid(getCurrentUid());
    Entity ent = tT.getEntity();
    ent.setEntityId(theUid);
    tT.setEntity(ent);
    os << LogIO::NORMAL << "Filled Processor table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }

  Bool MS2ASDM::writeField(){
    LogIO os(LogOrigin("MS2ASDM", "writeField()"));

    Bool rstat = True;
    asdm::FieldTable& tT = ASDM_p->getField();

    asdm::FieldRow* tR = 0;

    //    tR = tT.newRow();

    tT.add(tR);

    EntityId theUid(getCurrentUid());
    Entity ent = tT.getEntity();
    ent.setEntityId(theUid);
    tT.setEntity(ent);
    os << LogIO::NORMAL << "Filled Field table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }


  Bool MS2ASDM::writeFeed(){
    LogIO os(LogOrigin("MS2ASDM", "writeFeed()"));

    Bool rstat = True;
    asdm::FeedTable& tT = ASDM_p->getFeed();

    asdm::FeedRow* tR = 0;

    //    tR = tT.newRow();

    tT.add(tR);

    EntityId theUid(getCurrentUid());
    Entity ent = tT.getEntity();
    ent.setEntityId(theUid);
    tT.setEntity(ent);
    os << LogIO::NORMAL << "Filled Feed table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }


  Bool MS2ASDM::writeDataDesc(){
    LogIO os(LogOrigin("MS2ASDM", "writeDataDesc()"));

    Bool rstat = True;
    asdm::DataDescriptionTable& tT = ASDM_p->getDataDescription();

    asdm::DataDescriptionRow* tR = 0;

    //    tR = tT.newRow();

    tT.add(tR);

    EntityId theUid(getCurrentUid());
    Entity ent = tT.getEntity();
    ent.setEntityId(theUid);
    tT.setEntity(ent);
    os << LogIO::NORMAL << "Filled DataDescription table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }


  Bool MS2ASDM::writeConfigDesc(){

    LogIO os(LogOrigin("MS2ASDM", "writeConfigDesc()"));

    Bool rstat = False;

    asdm::ConfigDescriptionTable& cdT = ASDM_p->getConfigDescription();

    uInt nProcTabRows =  processor().nrow();
    if(nProcTabRows<1){ // processor table not filled, all data will have proc id == -1
      os <<  LogIO::WARN << "MS contains empty Processor table. Will assume processor type is CORRELATOR." << LogIO::POST;
      nProcTabRows = 1;
    }

    // loop over MS processor table
    for(uInt uprocId=0; uprocId<nProcTabRows; uprocId++){

      Int procId = uprocId;

      if(processor().nrow()<1){
	procId  = -1;
      }

      os << LogIO::NORMAL << "Processor Id: " << procId << LogIO::POST;

      asdm::ConfigDescriptionRow* cdR = 0;

      Tag procIdTag(uprocId, TagType::Processor);

      // determine processor type
      ProcessorTypeMod::ProcessorType processorType;
      if(procId>=0){
	String procType = processor().type()(procId);
	if(procType == "CORRELATOR"){
	  processorType = ProcessorTypeMod::CORRELATOR;
	}
	else if(procType == "SPECTROMETER"){
	  processorType = ProcessorTypeMod::SPECTROMETER;
	}
	else if(procType == "RADIOMETER"){
	  processorType = ProcessorTypeMod::RADIOMETER;
	}
	else{
	  os <<  LogIO::SEVERE << "Error: unsuported processor type: " << procType << LogIO::POST;
	  return False;
	}
      }
      else{
	processorType = ProcessorTypeMod::CORRELATOR;
      }

      SpectralResolutionTypeMod::SpectralResolutionType spectralType = SpectralResolutionTypeMod::FULL_RESOLUTION; 
      // alternatives: BASEBAND_WIDE, CHANNEL_AVERAGE
      os << LogIO::NORMAL << "Assuming data is of spectral resolution type \"FULL RESOLUTION\"." << LogIO::POST;       
  
      vector<Tag> antennaId;
      vector<Tag> dataDId;
      vector<int> feedId;
      vector<Tag> switchCycleId;
      vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> atmPhaseCorrection;
      int numAntenna = 0;
      int numFeed = 0;
      CorrelationModeMod::CorrelationMode correlationMode;

      // loop over MS main table and find for this proc id
      //  a) all used antennas
      //  b) all used DD IDs
      //  c) all used feed IDs
      uInt numAutoCorrs = 0;
      uInt numBaselines = 0;
      uInt nMainTabRows = ms_p.nrow();
      for(uInt mainTabRow=0; mainTabRow<nMainTabRows; mainTabRow++){
	if(processorId()(mainTabRow)==procId){

	  cout << "proc id " << procId << " used at main table row " << mainTabRow << endl;

	  uInt i = mainTabRow;
	  Double thisTStamp = time()(i); 
	  while(time()(i)== thisTStamp && i<nMainTabRows){
	    
	    // for the later determination of the correlation mode
	    if(antenna1()(i) == antenna2()(i)){
	      numAutoCorrs++;
	    }
	    else{
	      numBaselines++;
	    }
	    
	    // antenna ids
	    Bool found = False;
	    for(uInt j=0;j<antennaId.size();j++){
	      if(antenna1()(i) == (Int)antennaId[j].getTagValue()){
		found = True;
		break;
	      }
	    }
	    if(!found){
	      antennaId.push_back(Tag(antenna1()(i), TagType::Antenna));
	    }
	    found = False;
	    for(uInt j=0;j<antennaId.size();j++){
	      if(antenna2()(i) == (Int)antennaId[j].getTagValue()){
		found = True;
		break;
	      }
	    }
	    if(!found){
	      antennaId.push_back(Tag(antenna2()(i), TagType::Antenna));
	    }
	    
	    // DD IDs
	    found = False;
	    for(uInt j=0;j<dataDId.size();j++){
	      if(dataDescId()(i) == (Int)dataDId[j].getTagValue()){
		found = True;
		break;
	      }
	    }
	    if(!found){
	      dataDId.push_back(Tag(dataDescId()(i), TagType::DataDescription));
	    }
	    
	    // feed ids
	    found = False;
	    for(uInt j=0;j<feedId.size();j++){
	      if(feed1()(i) == feedId[j]){
		found = True;
		break;
	      }
	    }
	    if(!found){
	      feedId.push_back(feed1()(i));
	    }
	    found = False;
	    for(uInt j=0;j<feedId.size();j++){
	      if(feed2()(i) == feedId[j]){
		found = True;
		break;
	      }
	    }
	    if(!found){
	      feedId.push_back(feed2()(i));
	    }
	    
	    i++;
	  }
	
	  numAntenna = antennaId.size();
	  if(numAntenna!=(Int)antenna().nrow()){
	    os << LogIO::WARN << "Number of antennas in ANTENNA table (" << antenna().nrow() 
	       << ") is not the same as number of antennas for Processor " << procId << " in MAIN table (" << numAntenna
	       << ")." << LogIO::POST;      
	  }
	  if(numAntenna!=(Int)numAutoCorrs && numAutoCorrs>0){
	    os << LogIO::WARN << "Number of antennas in ANTENNA table (" << antenna().nrow() 
	       << ") is not the same as number of autocorrelations for Processor " << procId << " in MAIN table (" << numAntenna
	       << ")." << LogIO::POST;      
	  }
	  else if(numAutoCorrs==0){
	    os << LogIO::NORMAL << "No autocorrelations for Processor " << procId << " in MAIN table." << LogIO::POST;      
	  }
	
	  if(numAutoCorrs==0){
	    correlationMode = CorrelationModeMod::CROSS_ONLY;
	  }
	  else if(numBaselines>0){
	    correlationMode = CorrelationModeMod::CROSS_AND_AUTO;
	  }
	  else{
	    correlationMode = CorrelationModeMod::AUTO_ONLY;
	  }
	  
	  numFeed = feedId.size();
	  
	  switchCycleId.push_back(Tag(0, TagType::SwitchCycle)); // switch cycle table will only be dummy ? -> Francois
	  // dummy if PHASE_ID column doesn't exist in MS main
          // PHASE_ID identifies bin in switch cycle
	  //   otherwise, e.g. PHASE_ID = 0 and 1 => numStep == 2

	  atmPhaseCorrection.push_back(AtmPhaseCorrectionMod::AP_CORRECTED); // hardwired for the moment !!!
	  os << LogIO::NORMAL << "Assuming atm. phase correction type for data from processor " << procId 
	     << " is AP_CORRECTED." << LogIO::POST;      
    
	  // create a new row with its mandatory attributes.
	  cdR = cdT.newRow (antennaId.size(),
			    dataDId.size(),
			    feedId.size(),
			    correlationMode, 
			    atmPhaseCorrection.size(),
			    atmPhaseCorrection, 
			    processorType, 
			    spectralType, 
			    antennaId, 
			    feedId, 
			    switchCycleId, 
			    dataDId, 
			    procIdTag);
	  
	  // optional attributes.
	  //vector<Tag> assocConfigDescriptionId(1);
	  //assocConfigDescriptionId[0] = Tag(1, TagType::ConfigDescription);
	  //vector<int> phasedArrayList(2);
	  //phasedArrayList[0] = 0;
	  //phasedArrayList[1] = 1;
	  //vector<bool> flagAnt(2, false);
	  //	  vector<SpectralResolutionTypeMod::SpectralResolutionType> assocNature(1);
	  //assocNature[0] = SpectralResolutionTypeMod::FULL_RESOLUTION;
	  //cdR->setAssocConfigDescriptionId(assocConfigDescriptionId);
	  //cdR->setPhasedArrayList(phasedArrayList);
	  //cdR->setFlagAnt(flagAnt);
	  //cdR->setAssocNature(assocNature);
	  
	  
	  // add this row to its table.
	  cdT.add(cdR);
	  rstat = True;
	  break; // quit looking further for this proc ID in main table
	}
      } // end loop over MS main table

    } // end loop over MS processor table

    cout << "The ConfigDescription table has now " << cdT.size() << " elements" << endl;

    EntityId theUid(getCurrentUid());
    Entity ent = cdT.getEntity();
    ent.setEntityId(theUid);
    cdT.setEntity(ent);
    os << LogIO::NORMAL << "Filled ConfigDescription table " << getCurrentUid() << " ... " << LogIO::POST;
    incrementUid();

    return rstat;
  }

  Bool MS2ASDM::writeMain(){
    LogIO os(LogOrigin("MS2ASDM", "writeMain()"));

    Bool rstat = True;

    return rstat;
  }


  StokesParameterMod::StokesParameter MS2ASDM::ASDMStokesParameter( Stokes::StokesTypes s) {
    switch (s) {
    case Stokes::I:     return StokesParameterMod::I;         
    case Stokes::Q: 	return StokesParameterMod::Q;        
    case Stokes::U: 	return StokesParameterMod::U;        
    case Stokes::V: 	return StokesParameterMod::V;        
    case Stokes::RR: 	return StokesParameterMod::RR;       
    case Stokes::RL: 	return StokesParameterMod::RL;       
    case Stokes::LR: 	return StokesParameterMod::LR;       
    case Stokes::LL: 	return StokesParameterMod::LL;       
    case Stokes::XX: 	return StokesParameterMod::XX;       
    case Stokes::XY: 	return StokesParameterMod::XY;       
    case Stokes::YX: 	return StokesParameterMod::YX;       
    case Stokes::YY: 	return StokesParameterMod::YY;       
    case Stokes::RX: 	return StokesParameterMod::RX;       
    case Stokes::RY: 	return StokesParameterMod::RY;       
    case Stokes::LX: 	return StokesParameterMod::LX;       
    case Stokes::LY: 	return StokesParameterMod::LY;       
    case Stokes::XR: 	return StokesParameterMod::XR;       
    case Stokes::XL: 	return StokesParameterMod::XL;       
    case Stokes::YR: 	return StokesParameterMod::YR;       
    case Stokes::YL: 	return StokesParameterMod::YL;       
    case Stokes::PP: 	return StokesParameterMod::PP;       
    case Stokes::PQ: 	return StokesParameterMod::PQ;       
    case Stokes::QP: 	return StokesParameterMod::QP;       
    case Stokes::QQ: 	return StokesParameterMod::QQ;       
    case Stokes::RCircular: return StokesParameterMod::RCIRCULAR;
    case Stokes::LCircular: return StokesParameterMod::LCIRCULAR;
    case Stokes::Linear: 	return StokesParameterMod::LINEAR;   
    case Stokes::Ptotal: 	return StokesParameterMod::PTOTAL;   
    case Stokes::Plinear: 	return StokesParameterMod::PLINEAR;  
    case Stokes::PFtotal: 	return StokesParameterMod::PFTOTAL;  
    case Stokes::PFlinear: 	return StokesParameterMod::PFLINEAR; 
    case Stokes::Pangle:  	return StokesParameterMod::PANGLE; 
    case Stokes::Undefined:
    default:    
      throw(AipsError("Undefined stokes parameter."+String((Int)s)) );
    }
  }

  AntennaTypeMod::AntennaType MS2ASDM::ASDMAntennaType( const String& type ){
    LogIO os(LogOrigin("MS2ASDM", "(ASDMAntennaType)"));
    if(type == "GROUND-BASED"){
      return AntennaTypeMod::GROUND_BASED;
    }
    else if(type == "SPACE-BASED"){
      return AntennaTypeMod::SPACE_BASED;
    }      
    else{
      os << LogIO::WARN << "Unknown antenna type " << type << " assuming GROUND-BASED." << LogIO::POST;
      return AntennaTypeMod::GROUND_BASED;
    }      
  } 

  BasebandNameMod::BasebandName MS2ASDM::ASDMBBName( Int BBCNo ){
    switch(BBCNo){
    case 1: return BasebandNameMod::BB_1;
    case 2: return BasebandNameMod::BB_2;
    case 3: return BasebandNameMod::BB_3;
    case 4: return BasebandNameMod::BB_4;
    case 5: return BasebandNameMod::BB_5;
    case 6: return BasebandNameMod::BB_6;
    case 7: return BasebandNameMod::BB_7;
    case 8: return BasebandNameMod::BB_8;
    case 0: 
    default:
      return BasebandNameMod::NOBB;
    }
  }

  NetSidebandMod::NetSideband MS2ASDM::ASDMNetSideBand( Int netSideB ){
    switch(netSideB){
    case 1: return NetSidebandMod::LSB;
    case 2: return NetSidebandMod::USB;
    case 3: return NetSidebandMod::DSB;
    case 0:
    default:
      return NetSidebandMod::NOSB;
    }
  }

  FrequencyReferenceCodeMod::FrequencyReferenceCode MS2ASDM::ASDMFreqRefCode( const MFrequency::Types refFrame ){
    LogIO os(LogOrigin("MS2ASDM", "(ASDMFreqRefCode)"));
    switch( refFrame ){
    case MFrequency::LSRD: return FrequencyReferenceCodeMod::LSRD;
    case MFrequency::LSRK: return FrequencyReferenceCodeMod::LSRK;
    case MFrequency::BARY: return FrequencyReferenceCodeMod::BARY;
    case MFrequency::REST: return FrequencyReferenceCodeMod::REST;
    case MFrequency::GEO: return FrequencyReferenceCodeMod::GEO;
    case MFrequency::GALACTO: return FrequencyReferenceCodeMod::GALACTO;
    case MFrequency::TOPO: return FrequencyReferenceCodeMod::TOPO;
    default:
      os << LogIO::SEVERE << "Unsupported CASA reference frame " << MFrequency::showType(refFrame) 
	 << ", assuming TOPO." << LogIO::POST;
      return FrequencyReferenceCodeMod::TOPO; 
    }
  }

  Bool MS2ASDM::stokesTypePresent( const Vector< Int > corrT, 
				   const Stokes::StokesTypes st ){
    Bool rval = False;
    for(uInt j=0; j<corrT.size(); j++){
      if(corrT[j]==static_cast<Int>(st)){
	 rval = True;
      }
    }
    return rval;
  }

} //#End casa namespace
