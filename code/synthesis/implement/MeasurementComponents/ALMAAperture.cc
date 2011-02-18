//# ALMAAperture.cc: Implementation of the ALMAAperture class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//# Copyright (C) 2011 by ESO (in the framework of the ALMA collaboration)
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
//
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/ALMAAperture.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>
#include <synthesis/MeasurementComponents/BeamCalc.h>
#include <synthesis/MeasurementComponents/WTerm.h>
#include <synthesis/MeasurementComponents/ALMACalcIlluminationConvFunc.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>

namespace casa{

  AntennaResponses* ALMAAperture::aR_p = 0;

  ALMAAperture::ALMAAperture(): 
    ATerm(),
    polMap_p(),
    antTypeMap_p(),
    pairTypeToCFKeyMap_p(-1)
  {

    haveCannedResponses_p = True;
    
    if(!aR_p){
      cout << "new antenna responses object" << endl;
      aR_p = new AntennaResponses();
    }

    if(!aR_p->isInit()){ // the shared antenna responses object was not yet initialised

      cout << "initialising antenna responses" << endl;

      String antRespPath;
      if(!MeasTable::AntennaResponsesPath(antRespPath, "ALMA")) {
	// unknown observatory
	logIO() << LogOrigin("ALMAAperture", "ctor")
		<< LogIO::WARN
		<< "We don't have predefined antenna responses for ALMA at any frequency."
		<< endl << "Will try to use raytracing instead."
		<< LogIO::POST;
	haveCannedResponses_p = False;
      }
      else if(!aR_p->init(antRespPath)){
	// init failed
	String mesg="Initialisation of antenna responses for observatory ALMA failed using path "+antRespPath;
	SynthesisError err(mesg);
	throw(err);
      }
      else if(MeasTable::AntennaResponsesPath(antRespPath, "ACA")) {
	// also have responses for ACA
	aR_p->append(antRespPath); // dont't throw if this fails because the ACA antennas
	                          // may already be in the ALMA table
      }
    }
	 
  }
  
  ALMAAperture& ALMAAperture::operator=(const ALMAAperture& other)
  {
    if(this!=&other) 
      {
        // aR_p does not need to be copied because it is static
	haveCannedResponses_p = other.haveCannedResponses_p;
 	polMap_p.assign(other.polMap_p);
// 	Diameter_p=other.Diameter_p;
// 	Nant_p=other.Nant_p;
// 	HPBW=other.HPBW;
// 	sigma=other.sigma;
      }
    return *this;
  }

  Int ALMAAperture::getVisParams(const VisBuffer& vb)
  {
    Vector<String> telescopeNames=vb.msColumns().observation().telescopeName().getColumn();
    for(uInt nt=0;nt<telescopeNames.nelements();nt++){
      if ((telescopeNames(nt) != "ALMA") && 
	  (telescopeNames(nt) != "ACA") && 
	  (telescopeNames(nt) != "OSF")){
	String mesg="Can handle only ALMA, ACA, and OSF antennas.\n";
	mesg += "Erroneous telescope name = " + telescopeNames(nt) + ".";
	SynthesisError err(mesg);
	throw(err);
      }
    }

    MVFrequency FreqQ(vb.msColumns().spectralWindow().refFrequencyQuant()(0));
    Double Freq = FreqQ.getValue();

    Int bandID = -1;
    if(haveCannedResponses_p){
      String bandName;
      if(aR_p->getBandName(bandName, "ALMA", FreqQ)){
	bandID = atoi(bandName.substr(bandName.find("and")+3).c_str()); // band names start with "band" 
      }
      else{
	logIO() << LogOrigin("ALMAAperture", "getVisParams")
		<< LogIO::WARN
		<< "We don't have predefined antenna responses for ALMA at "
		<< Freq << " Hz. Will try to use raytracing instead."
		<< LogIO::POST;
      }      
    }
    return bandID;
  }
  
  
  void ALMAAperture::applySky(ImageInterface<Complex>& outImages,  // the image (cube if there is more than one pol)
			                                           // upon return: the images (cubes) multiplied by the 
			                                           // different rotated, regridded,
			                                           // FTed convolution pairs of AIFs  
			      const VisBuffer& vb, // for the parallactic angle
			      const Bool doSquint,
			      const Int& cfKey)
  {

    if(getVisParams(vb)==-1){ // need to use ray tracing
//       ALMACalcIlluminationConvFunc almaPB;
//       Long cachesize=(HostInfo::memoryTotal(true)/8)*1024;
//       almaPB.setMaximumCacheSize(cachesize);
//       almaPB.applyPB(outImages, vb, doSquint);
      logIO() << LogOrigin("ALMAAperture", "applySky")
	      << LogIO::SEVERE
	      << "Interface to BeamCalc ray tracing is not yet implemented for ALMA."
	      << LogIO::POST;      
    }
    else{ // use canned antenna responses
      // extract matrix from image
      
      //   (issue warning if too coarse => will be in method ATerm::OK(vb, PAtolerance, timetolerance)
      //   identify the right AIF based on antenna, freq band, polarizations
      //   Form all convolution pairs (Efield patterns) of antenna types in the Array
      //   FT the Efield patterns
      //   rotate using par. angle,
      //   then regrid it to the image, 
      //   multiply with image
    }

  }

  void ALMAAperture::applySky(ImageInterface<Float>& outImages,
			      const VisBuffer& vb, 
			      const Bool doSquint,
			      const Int& cfKey)
  {
    if(getVisParams(vb)==-1){ // need to use ray tracing
//       ALMACalcIlluminationConvFunc almaPB;
//       Long cachesize=(HostInfo::memoryTotal(true)/8)*1024;
//       almaPB.setMaximumCacheSize(cachesize);
//       almaPB.applyPB(outImages, vb, doSquint);
    }
    else{ // use canned antenna responses

    }
  }

  Int ALMAAperture::makePBPolnCoords(const VisBuffer&vb,
				     const Int& convSize,
				     const Int& convSampling,
				     const CoordinateSystem& skyCoord,
				     const Int& skyNx, const Int& skyNy,
				     CoordinateSystem& feedCoord)
  {
    feedCoord = skyCoord;
    //
    // Make a two dimensional image to calculate auto-correlation of
    // the ideal illumination pattern. We want this on a fine grid in
    // the UV plane
    //
    Int directionIndex=skyCoord.findCoordinate(Coordinate::DIRECTION);
    AlwaysAssert(directionIndex>=0, AipsError);
    DirectionCoordinate dc=skyCoord.directionCoordinate(directionIndex);
    Vector<Double> sampling;
    sampling = dc.increment();
    sampling*=Double(convSampling);
    sampling*=Double(skyNx)/Double(convSize);
    dc.setIncrement(sampling);
    
    
    Vector<Double> unitVec(2);
    unitVec=convSize/2;
    dc.setReferencePixel(unitVec);
    
    // Set the reference value to that of the image
    feedCoord.replaceCoordinate(dc, directionIndex);

    //
    // Make an image with linear polarization axis.
    //
    Int NPol=0,M,N=0;
    M=polMap_p.nelements();
    for(Int i=0;i<M;i++) if (polMap_p(i) > -1) NPol++;
    Vector<Int> poln(NPol);
    
    Int index;
    Vector<Int> inStokes;
    index = feedCoord.findCoordinate(Coordinate::STOKES);
    inStokes = feedCoord.stokesCoordinate(index).stokes();
    N = 0;
    try
      {
	for(Int i=0;i<M;i++) if (polMap_p(i) > -1) {poln(N) = vb.corrType()(i);N++;}
	StokesCoordinate polnCoord(poln);
	Int StokesIndex = feedCoord.findCoordinate(Coordinate::STOKES);
	feedCoord.replaceCoordinate(polnCoord,StokesIndex);
      }
    catch(AipsError& x)
      {
	throw(SynthesisFTMachineError("Likely cause: Discrepancy between the poln. "
				      "axis of the data and the image specifications."));
      }
    
    return NPol;
  }


  Vector<Int> ALMAAperture::vbRow2CFKeyMap(const VisBuffer& vb, Int& nUnique) 
  {
    // return index to outputImages for each row in vb
    Vector<Int> map; 
    map.resize(vb.nRow()); 

    cout << "vb rows: " << vb.nRow() << endl;

    if(haveCannedResponses_p){
      // distinguish different antenna types
      if(antTypeMap_p.nelements()!=(uInt)vb.numberAnt()){
	antTypeMap(antTypeMap_p, vb);
	cout << "initialising antTypeMap to " << antTypeMap_p << endl;
      }

      pairTypeToCFKeyMap_p.clear();
      Int cfKeyCount = 0;
      for(uInt i=0; i<(uInt)vb.nRow(); i++){
	Int pairType = antennaPairTypeCode(antTypeMap_p(vb.antenna1()(i)),
				       antTypeMap_p(vb.antenna2()(i)));
	map(i) = pairTypeToCFKeyMap_p(pairType);
	if(map(i)<0){ // new pair type
	  cout << "new pair type " << pairType << " gets key :" << cfKeyCount << endl;
	  pairTypeToCFKeyMap_p.define(pairType, cfKeyCount);
	  map(i) = cfKeyCount;
	  cfKeyCount++;
	}
      }  
      nUnique = cfKeyCount;
    }
    else{ // raytracing knows only one antenna type
      map=0; 
      nUnique=1; 
    }
    return map;
  }

  void ALMAAperture::antTypeMap(Vector<ALMAAntennaType>& map, const VisBuffer& vb){
    map.resize(vb.numberAnt());
    for(uInt i=0; i<map.nelements(); i++){
      map(i) = antTypeFromName(vb.msColumns().antenna().name()(i));
      cout << vb.msColumns().antenna().name()(i) << " " << map(i) << endl;
      if(map(i)==ALMA_INVALID){
	logIO() << LogOrigin("ALMAAperture", "antTypeMap")
		<< LogIO::WARN 
		<< "Unrecognised antenna type for antenna \"" 
		<< vb.msColumns().antenna().name()(i) << "\""  
		<< LogIO::POST;
      }
    }
  }

  ALMAAntennaType ALMAAperture::antTypeFromName(const String& name){
    ALMAAntennaType rval = ALMA_INVALID;
    String typeN = name.substr(0,2);
    if(typeN=="DV") rval=ALMA_DV;
    else if(typeN=="DA") rval=ALMA_DA;
    else if(typeN=="PM") rval=ALMA_PM;
    else if(typeN=="CM") rval=ALMA_CM;
    return rval;
  }

  Int ALMAAperture::antennaPairTypeCode(const ALMAAntennaType aT1, const ALMAAntennaType aT2){
    return min((Int)aT1+1, (Int)aT2+1) + 10000*max((Int)aT1+1, (Int)aT2+1); // order doesn't matter, convolution commutes
  }

  void ALMAAperture::antennaTypesFromPairType(ALMAAntennaType& aT1, ALMAAntennaType& aT2,
					      const Int& antennaPairType){
    Int t1 = (Int) floor(antennaPairType/10000.);
    Int t2 = antennaPairType - 10000*t1 - 1;
    t1--;
    aT1 = (ALMAAntennaType) min(t1,t2);
    aT2 = (ALMAAntennaType) max(t1,t2);
  }

};
