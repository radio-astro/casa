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
    respImage_p()
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
		<< "We don't have any precalculated antenna responses for ALMA."
		<< endl << "Will try to generate them using ray tracing instead."
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
  
  ALMAAperture::~ALMAAperture(){ 
    for(uInt i=0;i<respImage_p.nelements();i++){
      if(respImage_p(i)){
	delete respImage_p(i);
      } 
    }
  };


  ALMAAperture& ALMAAperture::operator=(const ALMAAperture& other)
  {
    if(this!=&other) 
      {
        // aR_p does not need to be copied because it is static
	haveCannedResponses_p = other.haveCannedResponses_p;
 	polMap_p.assign(other.polMap_p);
	antTypeMap_p.assign(other.antTypeMap_p);
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
  
  
  void ALMAAperture::applySky(ImageInterface<Complex>& outImage,  // the image (cube if there is more than one pol)
			                                           // upon return: FTed convolution pair of AIFs given by the cfKey  
			                                           // rotated by the PA and regridded to the image
			      const VisBuffer& vb, // for the parallactic angle
			      const Bool doSquint,
			      const Int& cfKey)
  {

    if(getVisParams(vb)==-1){ // need to use ray tracing, cfKey is ignored
      ALMACalcIlluminationConvFunc almaPB;
      Long cachesize=(HostInfo::memoryTotal(true)/8)*1024;
      almaPB.setMaximumCacheSize(cachesize);
      almaPB.applyPB(outImage, vb, doSquint);
    }
    else{ // use canned antenna responses

      //   (issue warning if image too coarse => will be in method ATerm::OK(vb, PAtolerance, timetolerance)
      
      //   identify the right response image based on antenna, freq band, polarizations
      Vector<ALMAAntennaType> aTypes = antennaTypesFromCFKey(cfKey);
      uInt nAntTypes = aTypes.nelements();
      Int spwId = vb.spectralWindow();

      Vector<String> respImageName(nAntTypes);
      Vector<uInt> respImageChannel(nAntTypes);
      Vector<MFrequency> respImageNomFreq(nAntTypes);
      Vector<AntennaResponses::FuncTypes> respImageFType(nAntTypes);
      Vector<Vector<Array<Complex> > > respByPol(nAntTypes); 

      MFrequency refFreq = vb.msColumns().spectralWindow().refFrequencyMeas()(spwId);
      MEpoch obsTime = vb.msColumns().observation().timeRangeMeas()(vb.observationId()(0))(IPosition(1,0)); // start time
      MDirection obsDir = vb.msColumns().field().phaseDirMeas(vb.fieldId()); 

      for(uInt i=0; i<nAntTypes; i++){
	cout << "Testing for ant type \"" << antTypeStrFromType(aTypes(i)) << "\", freq " 
	     << refFreq.get(Unit("Hz")).getValue() << " Hz" << endl;
	if(!aR_p->getImageName(respImageName(i),
			       respImageChannel(i),
			       respImageNomFreq(i),
			       respImageFType(i),
			       "ALMA",
			       obsTime,
			       refFreq,
			       AntennaResponses::ANY, // accept all at this stage
			       antTypeStrFromType(aTypes(i)),
			       obsDir,
			       "", // receiver type
			       0 // beam number
			       )){ // no matching response found
	  ostringstream oss;
	  oss << "No matching antenna response found for frequency "
	      << refFreq.get(Unit("Hz")).getValue() << " Hz, and antenna type "
	      << antTypeStrFromType(aTypes(i));
	  throw(SynthesisError(oss.str()));
	}
      }

      // load all necessary images (max. two)
      for(uInt i=0; i<nAntTypes; i++){
	if(respImageFType(i)!=AntennaResponses::EFP){
	  throw(SynthesisError(String("Can only process antenna responses of type EFP.")));
	}
      }
      respImage_p.resize(nAntTypes);
      for(uInt i=0; i<nAntTypes; i++){
	cout << "Loading " << respImageName(i) << endl;
   	
	try{
	  respImage_p(i) = new PagedImage<Complex>(respImageName(i));
	}
	catch(std::exception x){
	  ostringstream oss;
	  oss << "Error reading antenna response image from path \""
	      << respImageName(i) << "\": " << x.what();
	  respImage_p.resize(i,True);
	  throw(SynthesisError(oss.str()));
	} 
      }
      cout << "Loaded " << nAntTypes << " images." << endl;
      
      // identify polarisations in the response image and fill into respByPol(i)
      Int rIndex = respImage_p(0)->coordinates().findCoordinate(Coordinate::STOKES);
      Int rAxis = rIndex+1;
      Vector<Int> rStokes = respImage_p(0)->coordinates().stokesCoordinate(rIndex).stokes();
      IPosition rShape = respImage_p(0)->shape();
      cout << "Resp. image shape " << rShape << endl;
      uInt rNDim = rShape.size();
      IPosition rSkyShape(rNDim,1);
      rSkyShape(0) = rShape(0);
      rSkyShape(1) = rShape(1);
      uInt nRPol = rStokes.nelements();
      if(!(nRPol==4 && 
	   rStokes(0)==Stokes::XX && rStokes(1)==Stokes::XY && rStokes(2)==Stokes::YX && rStokes(3)==Stokes::YY)){
	  ostringstream oss;
	  oss << "Error: Antenna response image from path \""
	      << respImageName(0) 
	      << "\" does not contain the necessary polarisation products or products are in the wrong order.\n"
	      << "Order should be XX, XY, YX, YY.";
	  throw(SynthesisError(oss.str()));
      }	
      for(uInt i=0;i<nAntTypes; i++){ 
	respByPol(i).resize(nRPol); 
      }
      for(uInt i=0; i<nAntTypes; i++){
	for(uInt iPol=0; iPol<nRPol; iPol++){
	  IPosition start(rNDim,0);
	  start(rAxis) = iPol;
// 	  IPosition end(rNDim,0);
// 	  end(0) = rShape(0)-1;
// 	  end(1) = rShape(1)-1;
// 	  end(rAxis) = iPol;
// 	  Slicer s(start,end);
	  respImage_p(i)->getSlice(respByPol(i)(iPol), start, rSkyShape); 
	}
      }

      // identify polarizations in the input image, put them into polToDoIndex()
      Vector<Int> inStokes;
      Int pIndex;
      pIndex = outImage.coordinates().findCoordinate(Coordinate::STOKES);
      inStokes = outImage.coordinates().stokesCoordinate(pIndex).stokes();
      uInt nPol = inStokes.nelements();
      Vector<uInt> polToDoIndex(nPol);
      for(uInt i=0; i<nPol; i++){
	uInt ival=-1;
	switch(inStokes(i)){
	case Stokes::XX:
	  ival = 0;
	  break;
	case Stokes::XY:
	  ival = 1;
	  break;
	case Stokes::YX:
	  ival = 2;
	  break;
	case Stokes::YY:
	  ival = 3;
	  break;
	default:
	  ostringstream oss;
	  oss << "Error processing input image: polarization not valid for ALMA: " 
	      << Stokes::name(Stokes::type(inStokes(i)));
	  throw(SynthesisError(oss.str()));
	  break;
	}
	polToDoIndex(i) = ival;
      }

      // Calculate the primary beam for the given baseline type for each polarization
      ImageConvolver<Complex> iC;
      LogIO os(LogOrigin("ALMAAperture", "applySky", WHERE));
      
      CoordinateSystem dCoord = respImage_p(0)->coordinates(); // assume both response images have same coordsys
      IPosition dShape = respImage_p(0)->shape();
      dShape(rAxis) = nPol; // set the number of stokes pixels to that of the output image
      TempImage<Complex> nearFinal(dShape, dCoord);

      DirectionCoordinate dC = dCoord.directionCoordinate(0);
      CoordinateSystem tempCoord;
      tempCoord.addCoordinate(dC);

      for(uInt iPol=0; iPol<nPol; iPol++){

	Array<Complex> pB(respByPol(0)(polToDoIndex(iPol)).shape());

	// multiply EFPs equivalent to convolution of AIFs, get primary beam
	pB = respByPol(0)(polToDoIndex(iPol)) * respByPol(nAntTypes-1)(polToDoIndex(iPol));

	//   rotate using par. angle
	Array<Complex> rotPB;
	Double dAngleRad = getPA(vb);
	SynthesisUtils::rotateComplexArray(os, pB, tempCoord, rotPB, 
					   dAngleRad, "LINEAR");
	// now have the primary beam for polarization iPol in rotPB
      
	// combine all PBs into one image
	IPosition pos(rNDim,0);
	pos(rAxis) = iPol;
	nearFinal.putSlice(rotPB, pos);  

      }

      // then regrid it to the image

      // the following mess is necessary since ImageRegrid does not work for Complex images

      Array<Complex> nearFinalArray = nearFinal.get();
      Array<Complex> outArray = outImage.get();

      CoordinateSystem outCS(outImage.coordinates());
      Vector<Int> pixAxes=outCS.pixelAxes(outCS.findCoordinate(Coordinate::DIRECTION));
      IPosition whichOutPixelAxes(pixAxes);

      TempImage<Float> inImage(nearFinalArray.shape(),dCoord);
      TempImage<Float> tOutImage(outArray.shape(), outCS);
      ImageRegrid<Float> iR;

      inImage.copyData(LatticeExpr<Float>(real(ArrayLattice<Complex>(nearFinalArray))));
      tOutImage.set(0.0);

      iR.regrid(tOutImage, Interpolate2D::LINEAR, whichOutPixelAxes, inImage);
      setReal(outArray,tOutImage.get());

      inImage.copyData(LatticeExpr<Float>(imag(ArrayLattice<Complex>(nearFinalArray))));
      tOutImage.set(0.0);

      iR.regrid(tOutImage, Interpolate2D::LINEAR, whichOutPixelAxes, inImage);
      setImag(outArray,tOutImage.get());

      outImage.put(outArray);

      // tidy up
      for(uInt i=0; i<nAntTypes; i++){
	delete respImage_p(i);
      }
      respImage_p.resize(0);	  

    }
  }

  void ALMAAperture::applySky(ImageInterface<Float>& outImages,
			      const VisBuffer& vb, 
			      const Bool doSquint,
			      const Int& cfKey)
  {
    if(getVisParams(vb)==-1){ // need to use ray tracing
      ALMACalcIlluminationConvFunc almaPB;
      Long cachesize=(HostInfo::memoryTotal(true)/8)*1024;
      almaPB.setMaximumCacheSize(cachesize);
      almaPB.applyPB(outImages, vb, doSquint);
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
    for(Int i=0;i<M;i++){
      if (polMap_p(i) > -1) NPol++;
    }
    Vector<Int> poln(NPol);
    
    Int index;
    Vector<Int> inStokes;
    index = feedCoord.findCoordinate(Coordinate::STOKES);
    inStokes = feedCoord.stokesCoordinate(index).stokes();
    N = 0;
    try{
      for(Int i=0;i<M;i++) if (polMap_p(i) > -1) {poln(N) = vb.corrType()(i);N++;}
      StokesCoordinate polnCoord(poln);
      Int StokesIndex = feedCoord.findCoordinate(Coordinate::STOKES);
      feedCoord.replaceCoordinate(polnCoord,StokesIndex);
    }
    catch(AipsError& x){
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
	antTypeMap_p.assign(antTypeMap(vb));
	cout << "initialising antTypeMap to " << antTypeMap_p << endl;
      }

      SimpleOrderedMap<Int, Int > cFKeysEncountered(-1); 
      Int cfKeyCount = 0;
      for(uInt i=0; i<(uInt)vb.nRow(); i++){
	Int cfKey = cFKeyFromAntennaTypes(antTypeMap_p(vb.antenna1()(i)),
					  antTypeMap_p(vb.antenna2()(i)));
	map(i) = cfKey;
	if(cFKeysEncountered(cfKey)<0){ // new cFKey
	  cFKeysEncountered.define(cfKey, cfKeyCount);
	  cfKeyCount++;
	}
      }  
      nUnique = cfKeyCount;
    }
    else{ // raytracing knows only one antenna type for the moment
      map=0; 
      nUnique=1; 
    }
    return map;
  }
  
  Vector<ALMAAntennaType> ALMAAperture::antTypeMap(const VisBuffer& vb){
    Vector<ALMAAntennaType> map(vb.numberAnt(),ALMA_INVALID);
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
    return map;
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

  String ALMAAperture::antTypeStrFromType(const ALMAAntennaType& aType){
    String tS;
    switch(aType){
    case ALMA_DV:
      tS = "DV";
      break;
    case ALMA_DA:
      tS = "DA";
      break;
    case ALMA_PM:
      tS = "PM";
      break;
    case ALMA_CM:
      tS = "CM";
      break;
    case ALMA_INVALID:
    default:
      tS = "";
      break;
    }
    return tS;
  }


  Int ALMAAperture::cFKeyFromAntennaTypes(const ALMAAntennaType aT1, const ALMAAntennaType aT2){
    return min((Int)aT1+1, (Int)aT2+1) + 10000*max((Int)aT1+1, (Int)aT2+1); // order doesn't matter, convolution commutes
  }

  Vector<ALMAAntennaType> ALMAAperture::antennaTypesFromCFKey(const Int& cFKey){
    Int t1 = (Int) floor(cFKey/10000.);
    Int t2 = cFKey - 10000*t1 - 1;
    t1--;
    Vector<ALMAAntennaType> rval(1);
    if(t1==t2){
      rval(0) = (ALMAAntennaType) t1;
    }
    else{
      rval.resize(2);
      rval(0) = (ALMAAntennaType) min(t1,t2);
      rval(1) = (ALMAAntennaType) max(t1,t2);
    }
    return rval;
  }

  Vector<ALMAAntennaType> ALMAAperture::antTypeList(const VisBuffer& vb){
    Vector<ALMAAntennaType> aTypeMap = antTypeMap(vb);
    Vector<Bool> encountered(ALMA_numAntTypes, False);
    uInt typeCount = 0;
    for(uInt i=0; i<aTypeMap.nelements(); i++){
      uInt index = (uInt)aTypeMap(i);
      if(!encountered(index)){
	encountered(index) = True;
	typeCount++;
      }
    }
    Vector<ALMAAntennaType> aTypeList(typeCount);
    uInt index = 0;
    for(uInt i=0; i<ALMA_numAntTypes; i++){
      if(encountered(i)){
	aTypeList(index) = (ALMAAntennaType)i;
	index++;
      }
    }
    return aTypeList;
  }


};
