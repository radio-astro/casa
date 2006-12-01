// -*- C++ -*-
//# PBWProjectFT.cc: Implementation of PBWProjectFT class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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

#include <msvis/MSVis/VisibilityIterator.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/UnitVal.h>
#include <measures/Measures/Stokes.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>
#include <synthesis/MeasurementComponents/PBWProjectFT.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixIter.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <casa/Utilities/CompositeNumber.h>
#include <casa/OS/Timer.h>
#include <casa/OS/HostInfo.h>
#include <casa/sstream.h>
#include <fstream.h>

#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/IlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/Exp.h>
#include <synthesis/MeasurementComponents/CExp.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>

#define USETABLES 1           // If equal to 1, use tabulated exp() and
			      // complex exp() functions.
#define MAXPOINTINGERROR 250.0 // Max. pointing error in arcsec used to
// determine the resolution of the
// tabulated exp() function.
namespace casa { //# NAMESPACE CASA - BEGIN
  
#define NEED_UNDERSCORES
  extern "C" 
  {
    //
    // The Gridding Convolution Function (GCF) used by the underlying
    // gridder written in FORTRAN.
    //
    // The arguments must all be pointers and the value of the GCF at
    // the given (u,v) point is returned in the weight variable.  Making
    // this a function which returns a complex value (namely the weight)
    // has problems when called in FORTRAN - I (SB) don't understand
    // why.
    //
#if defined(NEED_UNDERSCORES)
#define wcppeij wcppeij_
#endif
    //
    //---------------------------------------------------------------
    //
    IlluminationConvFunc wEij;
    
    void wcppeij(Double *griduvw, Double *area,
		 Double *raoff1, Double *decoff1,
		 Double *raoff2, Double *decoff2, 
		 Int *doGrad,
		 Complex *weight,
		 Complex *dweight1,
		 Complex *dweight2,
		 Double *currentCFPA)
    {
      Complex w,d1,d2;
      wEij.getValue(griduvw, raoff1, raoff2, decoff1, decoff2,
		    area,doGrad,w,d1,d2,*currentCFPA);
      *weight   = w;
      *dweight1 = d1;
      *dweight2 = d2;
    }
  }
  //
  //---------------------------------------------------------------
  //
#define FUNC(a)  ((sqrt(a)))
  PBWProjectFT::PBWProjectFT(MeasurementSet& ms, EPJones* epjPtr,
			     //			     VPSkyJones *vpsj,
			     Int nWPlanes, Long icachesize, 
			     String& cfCacheDirName,
			     Bool applyPointingOffset,
			     Bool doPBCorr,
			     Int itilesize, 
			     Float paSteps,
			     Float pbLimit,
			     Bool usezero)
    : FTMachine(), padding_p(1.0), ms_p(&ms), nWPlanes_p(nWPlanes),
      imageCache(0), cachesize(icachesize), tilesize(itilesize),
      gridder(0), isTiled(False), arrayLattice(0), lattice(0), 
      maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
      mspc(0), msac(0), pointingToImage(0), usezero_p(usezero),
      //      vpSJ(vpsj),avgPB(), 
      epJ(epjPtr),doPBCorrection(doPBCorr),
      Second("s"),Radian("rad"),Day("d"), noOfPASteps(0),
      pbNormalized(False),resetPBs(True),avgPBSaved(False),cfCache(), paChangeDetector()
  {
    convSize=0;
    tangentSpecified_p=False;
    lastIndex_p=0;
    paChangeDetector.reset();
    pbLimit_p=pbLimit;
    //
    // Get various parameters from the visibilities.  
    //
    bandID_p = getVisParams();
    if (applyPointingOffset) doPointing=1; else doPointing=0;

    convFuncCacheReady=False;
    PAIndex = -1;
    maxConvSupport=-1;  
    //
    // Set up the Conv. Func. disk cache manager object.
    //
    cfCache.setCacheDir(cfCacheDirName.data());
    cfCache.initCache();
    convSampling=50;
    convSize=1024*2;
  }
  //
  //---------------------------------------------------------------
  //
  PBWProjectFT::PBWProjectFT(const RecordInterface& stateRec)
    : FTMachine(),Second("s"),Radian("rad"),Day("d")
  {
    // Construct from the input state record
    String error;
    
    if (!fromRecord(error, stateRec)) {
      throw (AipsError("Failed to create PBWProjectFT: " + error));
    };
    bandID_p = getVisParams();
    PAIndex = -1;
    maxConvSupport=-1;
    convSampling=50;
    convSize=1024*2;
  }
  //
  //---------------------------------------------------------------
  //
  PBWProjectFT& PBWProjectFT::operator=(const PBWProjectFT& other)
  {
    if(this!=&other) {
      padding_p=other.padding_p;
      ms_p=other.ms_p;
      nWPlanes_p=other.nWPlanes_p;
      imageCache=other.imageCache;
      cachesize=other.cachesize;
      tilesize=other.tilesize;
      gridder=other.gridder;
      isTiled=other.isTiled;
      lattice=other.lattice;
      arrayLattice=other.arrayLattice;
      maxAbsData=other.maxAbsData;
      centerLoc=other.centerLoc;
      offsetLoc=other.offsetLoc;
      mspc=other.mspc;
      msac=other.msac;
      pointingToImage=other.pointingToImage;
      usezero_p=other.usezero_p;
      doPBCorrection = other.doPBCorrection;
      maxConvSupport= other.maxConvSupport;
    };
    return *this;
  };
  //
  //---------------------------------------------------------------------
  //
  int PBWProjectFT::getVisParams()
  {
    Double Freq;
    mspc=new MSPointingColumns(ms_p->pointing());
    msac=new MSAntennaColumns(ms_p->antenna());
    MSObservationColumns msoc(ms_p->observation());
    Vector<String> telescopeNames=msoc.telescopeName().getColumn();
    for(Int nt=0;nt<telescopeNames.nelements();nt++)
      {
	if (telescopeNames(nt) != "VLA")
	  {
	    String mesg="pbwproject algorithm can handle only VLA antennas for now.\n";
	    mesg += "Erroneous telescope name = " + telescopeNames(nt) + ".";
	    SynthesisError err(mesg);
	    throw(err);
	  }
	if (telescopeNames(nt) != telescopeNames(0))
	  {
	    String mesg="pbwproject algorithm does not (yet) handle inhomogeneous arrays!\n";
	    mesg += "Not yet a \"priority\"!!";
	    SynthesisError err(mesg);
	    throw(err);
	  }
      }
    ROMSSpWindowColumns mssp(ms_p->spectralWindow());
    Freq = mssp.refFrequency()(0);
    Diameter_p = msac->dishDiameter()(0);
    Nant_p     = msac->nrow();
    Double Lambda=C::c/Freq;
    HPBW = Lambda/(Diameter_p*sqrt(log(2.0)));
    sigma = 1.0/(HPBW*HPBW);
    Int bandID = getVLABandID(Freq,telescopeNames(0));
    return bandID;
  }
  //
  //----------------------------------------------------------------------
  //
  PBWProjectFT::PBWProjectFT(const PBWProjectFT& other)
  {
    operator=(other);
  }
  //
  //----------------------------------------------------------------------
  //
  void PBWProjectFT::init() 
  {
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
    
    if(image->shape().product()>cachesize) 
      isTiled=True;
    else 
      isTiled=False;
    
    sumWeight.resize(npol, nchan);
    
    wConvSize=max(1, nWPlanes_p);
    if (!convFuncCacheReady) 
      {
	convSupport.resize(wConvSize,1,1,True);
	convSupport=0;
      }
    
    uvScale.resize(3);
    uvScale=0.0;
    uvScale(0)=Float(nx)*image->coordinates().increment()(0); 
    uvScale(1)=Float(ny)*image->coordinates().increment()(1); 
    uvScale(2)=Float(wConvSize)*abs(image->coordinates().increment()(0));
    
    uvOffset.resize(3);
    uvOffset(0)=nx/2;
    uvOffset(1)=ny/2;
    uvOffset(2)=0;
    
    if(gridder) delete gridder; gridder=0;
    gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						   uvScale, uvOffset,
						   "SF");
    
    // Set up image cache needed for gridding. 
    if(imageCache) delete imageCache;   imageCache=0;
    
    // The tile size should be large enough that the
    // extended convolution function can fit easily
    if(isTiled) 
      {
	Float tileOverlap=0.5;
	tilesize=min(256,tilesize);
	IPosition tileShape=IPosition(4,tilesize,tilesize,npol,nchan);
	Vector<Float> tileOverlapVec(4);
	tileOverlapVec=0.0;
	tileOverlapVec(0)=tileOverlap;
	tileOverlapVec(1)=tileOverlap;
	Int tmpCacheVal=static_cast<Int>(cachesize);
	imageCache=new LatticeCache <Complex> (*image, tmpCacheVal, tileShape, 
					       tileOverlapVec,
					       (tileOverlap>0.0));
      }
    
#if(USETABLES)
    Double StepSize;
    Int N=500000;
    StepSize = abs((((2*nx)/uvScale(0))/sigma + 
		    MAXPOINTINGERROR*1.745329E-02*sigma/3600.0))/N;
    if (!wEij.isReady())
      {
	logIO() << LogOrigin("PBWProjectFT","init")
		<< "Making lookup table for exp function with a resolution of " 
		<< StepSize << " radians.  "
		<< "Memory used: " << sizeof(Float)*N/(1024.0*1024.0)<< " MB." 
		<< LogIO::NORMAL 
		<<LogIO::POST;
	
	wEij.setSigma(sigma);
	wEij.initExpTable(N,StepSize);
	//    ExpTab.build(N,StepSize);
	
	logIO() << LogOrigin("PBWProjectFT","init")
		<< "Making lookup table for complex exp function with a resolution of " 
		<< 2*M_PI/N << " radians.  "
		<< "Memory used: " << 2*sizeof(Float)*N/(1024.0*1024.0) << " MB." 
		<< LogIO::NORMAL
		<< LogIO::POST;
	wEij.initCExpTable(N);
	//    CExpTab.build(N);
      }
#endif
    //    vpSJ->reset();
    paChangeDetector.reset();
    makingPSF = False;
  }
  //
  //---------------------------------------------------------------
  //
  // This is nasty, we should use CountedPointers here.
  PBWProjectFT::~PBWProjectFT() {
    /*
      if(imageCache) delete imageCache; imageCache=0;
      if(arrayLattice) delete arrayLattice; arrayLattice=0;
      if(gridder) delete gridder; gridder=0;
      
      Int NCF=convFuncCache.nelements();
      for(Int i=0;i<NCF;i++) delete convFuncCache[i];
    */
  }
  //
  //---------------------------------------------------------------
  //
  Int PBWProjectFT::makePBPolnCoords(CoordinateSystem& squintCoord)
  {
    //
    // Make an image with circular polarization axis.
    //
    Int NPol=0,M,N=0;
    M=polMap.nelements();
    for(Int i=0;i<M;i++) if (polMap(i) > -1) NPol++;
    
    Vector<Int> poln(NPol);
    //
    // RR,LL...etc. are NOT STOKES names!  Confusing misuse of
    // scientific names.
    //
    // If there is only one polarization plane defined in the polMap,
    // set the single pixel along the poln. axis to be same the one in
    // the MS.
    // 
    if (M==1) 
      {
	MSPolarizationColumns mspol(ms_p->polarization());
	poln(0) = mspol.corrType()(0)(IPosition(1,0));
      }
    else
      {
	for(Int i=0;i<M;i++)
	  {
	    if ((i==0) && (polMap(i) > -1)) poln(N++) = Stokes::LL;
	    if ((i==1) && (polMap(i) > -1)) poln(N++) = Stokes::RL;
	    if ((i==2) && (polMap(i) > -1)) poln(N++) = Stokes::LR;
	    if ((i==3) && (polMap(i) > -1)) poln(N++) = Stokes::RR;
	  }
      }
    
    Vector<Int> inStokes;
    Int index;
    index = squintCoord.findCoordinate(Coordinate::STOKES);
    inStokes = squintCoord.stokesCoordinate(index).stokes();
    N = 0;
    try
      {
	for(Int i=0;i<M;i++) if (polMap(i) > -1) poln(polMap(i)) = inStokes(N++);
	StokesCoordinate polnCoord(poln);
	Int StokesIndex = squintCoord.findCoordinate(Coordinate::STOKES);
	squintCoord.replaceCoordinate(polnCoord,StokesIndex);
      }
    catch(AipsError& x)
      {
	throw(SynthesisFTMachineError("Likely cause: Discrepancy between the poln. "
				      "axis of the data and the image specifications."));
      }
    
    return NPol;
  }
  //
  //---------------------------------------------------------------
  //
  MDirection::Convert PBWProjectFT::makeCoordinateMachine(const VisBuffer& vb,
							  const MDirection::Types& From,
							  const MDirection::Types& To,
							  MEpoch& last)
  {
    Double time = getCurrentTimeStamp(vb);
    
    MEpoch epoch(Quantity(time,Second),MEpoch::TAI);
    //  epoch = MEpoch(Quantity(time,Second),MEpoch::TAI);
    //
    // ...now make an object to hold the observatory position info...
    //
    MPosition pos;
    MSObservationColumns msoc(ms_p->observation());
    String ObsName=msoc.telescopeName()(vb.arrayId());
    
    if (!MeasTable::Observatory(pos,ObsName))
      throw(AipsError("Observatory position for "+ ObsName + " not found"));
    //
    // ...now make a Frame object out of the observatory position and
    // time objects...
    //
    MeasFrame frame(epoch,pos);
    //
    // ...finally make the convert machine.
    //
    MDirection::Convert mac(MDirection::Ref(From,frame),
			    MDirection::Ref(To,frame));
    
    MEpoch::Convert toLAST = MEpoch::Convert(MEpoch::Ref(MEpoch::TAI,frame),
					     MEpoch::Ref(MEpoch::LAST,frame));
    last = toLAST(epoch);
    
    return mac;
  }
  //
  //---------------------------------------------------------------
  //
  int PBWProjectFT::findPointingOffsets(const VisBuffer& vb, 
					Array<Float> &l_off,
					Array<Float> &m_off,
					Bool Evaluate)
  {

    throw(AipsError("PBWProject::findPointingOffsets temporarily disabled. (gmoellen 06Nov10)"));

    Cube<Float> pointingOffsets;
    Int NAnt = 0;

    // TBD: adapt the following to VisCal mechanism:
    pointingOffsets = Cube<Float>(2,1,1); //epJ->getPar(vb);
    pointingOffsets = 0.0;
    MEpoch LAST;
    
    NAnt=pointingOffsets.shape()(2)-1;
    l_off  = pointingOffsets(IPosition(3,0,0,0),IPosition(3,0,0,NAnt));
    m_off = pointingOffsets(IPosition(3,1,0,0),IPosition(3,1,0,NAnt));

    /*
    IPosition shp(pointingOffsets.shape());
    IPosition shp1(l_off.shape()),shp2(m_off.shape());
    for(Int ii=0;ii<NAnt;ii++)
      {
	IPosition ndx(3,0,0,0);
	ndx(2)=ii;
	cout << "Pointing Offsets: " << ii << " " 
	     << l_off(ndx)*57.295*60.0 << " " 
	     << m_off(ndx)*57.295*60.0 << endl;
      }
    */
    return NAnt;
    if (!Evaluate) return NAnt;
    
    //  cout << "AzEl Offsets: " << pointingOffsets << endl;
    //
    // Make a Coordinate Conversion Machine to go from (Az,El) to
    // (HA,Dec).
    //
    MDirection::Convert toAzEl = makeCoordinateMachine(vb,MDirection::HADEC,
						       MDirection::AZEL,
						       LAST);
    MDirection::Convert toHADec = makeCoordinateMachine(vb,MDirection::AZEL,
							MDirection::HADEC,
							LAST);
    //
    // ...and now hope that it all works and works correctly!!!
    //
    Quantity dAz(0,Radian),dEl(0,Radian);
    MSFieldColumns msfc(ms_p->field());
    //
    // An array of shape [2,1,1]!
    //
    Array<Double> phaseDir = msfc.phaseDir().getColumn();
    Double RA0   = phaseDir(IPosition(3,0,0,0));
    Double Dec0  = phaseDir(IPosition(3,1,0,0));
    //  
    // Compute reference (HA,Dec)
    //
    Double LST   = LAST.get(Day).getValue();
    Double SDec0 = sin(Dec0), CDec0=cos(Dec0);
    LST -= floor(LST); // Extract the fractional day
    LST *= 2*C::pi;// Convert to Raidan
    
    Double HA0;
    HA0 = LST - RA0;
    Quantity QHA0(HA0,Radian), QDEC0(Dec0,Radian);
    //
    // Convert reference (HA,Dec) to reference (Az,El)
    //
    MDirection PhaseCenter(QHA0, QDEC0,MDirection::Ref(MDirection::HADEC));
    MDirection AzEl0 = toAzEl(PhaseCenter);
    
    MDirection tmpHADec = toHADec(AzEl0);
    
    Double Az0_Rad = AzEl0.getAngle(Radian).getValue()(0);
    Double El0_Rad = AzEl0.getAngle(Radian).getValue()(1);
    
    //
    // Convert the antenna pointing offsets from (Az,El)-->(RA,Dec)-->(l,m) 
    //
    
    for(IPosition n(3,0,0,0);n(2)<=NAnt;n(2)++)
      {
	//
	// From (Az,El) -> (HA,Dec)
	//
	// Add (Az,El) offsets to the reference (Az,El)
	//
	dAz.setValue(l_off(n)+Az0_Rad);  dEl.setValue(m_off(n)+El0_Rad);
	//      dAz.setValue(0.0+Az0_Rad);  dEl.setValue(0.0+El0_Rad);
	MDirection AzEl(dAz,dEl,MDirection::Ref(MDirection::AZEL));
	//
	// Convert offsetted (Az,El) to (HA,Dec) and then to (RA,Dec)
	//
	MDirection HADec = toHADec(AzEl);
	Double HA,Dec,RA, dRA;
	HA  = HADec.getAngle(Radian).getValue()(0);
	Dec = HADec.getAngle(Radian).getValue()(1);
	RA  = LST - HA;
	dRA = RA - RA0;
	cout << n << " " << l_off(n) << " " << m_off(n) << " "
	     << HA << " " << Dec << endl;
	//
	// Convert offsetted (RA,Dec) -> (l,m)
	//
	l_off(n)  = sin(dRA)*cos(Dec);
	m_off(n) = sin(Dec)*CDec0-cos(Dec)*SDec0*cos(dRA);
	//      cout << "FindOff: " << n(2) << " " << l_offsets(n) << " " << m_offsets(n) << endl;
	
	//       cout << l_off(n) << " " << m_off(n) << " "
	// 	   << " " << HA << " " << Dec
	// 	   << " " << LST << " " << RA0 << " " << Dec0 
	// 	   << " " << RA << " " << Dec
	// 	   << endl;
      }
    
    return NAnt+1;
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::normalizeAvgPB()
  {
    if (!pbNormalized)
      {
	pbPeaks.resize(avgPB.shape()(2),True);
	if (makingPSF) pbPeaks = 1.0;
	else pbPeaks /= (Float)noOfPASteps;
	pbPeaks = 1.0;
	logIO() << LogOrigin("PBWProjectFT", "normalizeAvgPB")  
		<< "Normalizing the average PBs to " << 1.0
		<< LogIO::NORMAL
		<< LogIO::POST;
	
	IPosition avgPBShape(avgPB.shape()),ndx(4,0,0,0,0);
	Vector<Float> peak(avgPBShape(2));
	
	Bool isRefF;
	Array<Float> avgPBBuf;
	isRefF=avgPB.get(avgPBBuf);
	
	for(ndx(3)=0;ndx(3)<avgPBShape(3);ndx(3)++)
	  for(ndx(2)=0;ndx(2)<avgPBShape(2);ndx(2)++)
	    {
	      peak(ndx(2)) = 0;
	      for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
		for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
		  if (abs(avgPBBuf(ndx)) > peak(ndx(2)))
		    peak(ndx(2)) = avgPBBuf(ndx);
	      
	      for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
		for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
		  avgPBBuf(ndx) *= (pbPeaks(ndx(2))/peak(ndx(2)));
	    }
	
	if (isRefF) avgPB.put(avgPBBuf);
      }
    pbNormalized = True;
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::makeAveragePB(const VisBuffer& vb, 
				   const ImageInterface<Complex>& image,
				   Int& polInUse,
				   //TempImage<Float>& thesquintPB,
				   TempImage<Float>& theavgPB)
  {
    TempImage<Float> localPB;
    
    logIO() << LogOrigin("PBWProjecFT","makeAveragePB")
	    << LogIO::NORMAL;
    
    localPB.resize(image.shape()); localPB.setCoordinateInfo(image.coordinates());
    //
    // If this is the first time, resize the average PB
    //
    if (resetPBs)
      {
	logIO() << LogOrigin("PBWProjectFT", "makeAveragePB")  
		<< "Initializing the average PBs"
		<< LogIO::NORMAL
		<< LogIO::POST;
	theavgPB.resize(localPB.shape()); 
	theavgPB.setCoordinateInfo(localPB.coordinates());
	theavgPB.set(0.0);
	noOfPASteps = 0;
	pbPeaks.resize(theavgPB.shape()(2));
	pbPeaks.set(0.0);
	resetPBs=False;
      }
    //
    // Make the Stokes PB
    //
    localPB.set(1.0);
    //   vpSJ->applySquare(localPB,localPB,vb,1);  // This is Stokes PB (not squinted)
    {
      VLACalcIlluminationConvFunc vlaPB;
      vlaPB.applyPB(localPB, vb, bandID_p);
    }
    
    IPosition twoDPBShape(localPB.shape());
    TempImage<Complex> localTwoDPB(twoDPBShape,localPB.coordinates());
    
    //   Array<Float> raoff,decoff;
    Float peak=0;
    Int NAnt;
    //    NAnt=findPointingOffsets(vb,l_offsets,m_offsets,True);
    if (doPointing)
      NAnt=findPointingOffsets(vb,l_offsets,m_offsets,False);
    else
      NAnt = 1;
    
    logIO() << LogOrigin("PBWProjectFT", "makeAveragePB")  
	    << "Updating average PB"
	    << LogIO::NORMAL
	    << LogIO::POST;
    
    noOfPASteps++;
    if (!doPointing) NAnt=1;
    NAnt=1;
    
    for(Int ant=0;ant<NAnt;ant++)
      { //Ant loop
	{
	  IPosition ndx(4,0,0,0,0);
	  for(ndx(0)=0; ndx(0)<twoDPBShape(0); ndx(0)++)
	    for(ndx(1)=0; ndx(1)<twoDPBShape(1); ndx(1)++)
	      //	     for(ndx(2)=0; ndx(2)<polInUse; ndx(2)++)
	      for(ndx(2)=0; ndx(2)<twoDPBShape(2); ndx(2)++)
		localTwoDPB.putAt(Complex((localPB(ndx)),0.0),ndx);
	}
	//
	// If antenna pointing errors are not applied, no shifting
	// (which can be expensive) is required.
	//
	//
	// Accumulate the shifted PBs
	//
	{
	  Bool isRefF,isRefC;
	  Array<Float> fbuf;
	  Array<Complex> cbuf;
	  isRefF=theavgPB.get(fbuf);
	  isRefC=localTwoDPB.get(cbuf);
	  
	  Vector<Float> tmpPeaks(pbPeaks.nelements());
	  IPosition cs(cbuf.shape()),fs(fbuf.shape()),peakLoc(4,0,0,0,0);;
	  //	 if (makingPSF)
	  {
	    IPosition ndx(4,0,0,0,0),avgNDX(4,0,0,0,0);
	    for(ndx(2)=0,avgNDX(2)=0;ndx(2)<twoDPBShape(2);ndx(2)++,avgNDX(2)++)
	      {
		for(ndx(0)=0,avgNDX(0)=0;ndx(0)<fs(0);ndx(0)++,avgNDX(0)++)
		  for(ndx(1)=0,avgNDX(1)=0;ndx(1)<fs(1);ndx(1)++,avgNDX(1)++)
		    {
		      Float val;
		      val = real(cbuf(ndx));
		      fbuf(avgNDX) += val;
		      if (fbuf(avgNDX) > peak) peak=fbuf(avgNDX);
		    }
	      }
	  }
	  if (!isRefF) theavgPB.put(fbuf);
	  pbPeaks += tmpPeaks;
	}
      }
  }
  //
  //---------------------------------------------------------------
  //
  // Locate a convlution function in either mem. or disk cache.  
  // Return 1 if found in the disk cache.
  //        2 if found in the mem. cache.
  //       <0 if not found in either cache.  In this case, absolute of
  //          the return value corresponds to the index in the list of
  //          conv. funcs. where this conv. func. should be filled
  //
  Int PBWProjectFT::locateConvFunction(Int Nw, Int polInUse,  
				       const VisBuffer& vb, Float &pa)
  {
    //    Int i=cfCache.searchConvFunction(vb,*vpSJ,pa);
    Int i; Bool found;
    //    found = cfCache.searchConvFunction(vb,*vpSJ,i,pa);
    found = cfCache.searchConvFunction(vb,paChangeDetector,i,pa);
    //    if (i >= 0)
    if (found)
      {
	Vector<Float> sampling;
	PAIndex=i;
	if (cfCache.loadConvFunction(i,Nw,convFuncCache,convSupport,sampling))
	  {
	    convSampling = (Int)sampling[0];
	    convFunc.reference(*convFuncCache[PAIndex]);
	    if (PAIndex < convFuncCache.nelements())
	      logIO() << "Loaded from disk cache: Conv. func. # "
		      << PAIndex << LogIO::POST;
	    return 1;
	  }
	convFunc.reference(*convFuncCache[PAIndex]);
	return 2;
      }
    return i;
  }
  //
  //---------------------------------------------------------------
  //
  // Given a polMap (mapping of which Visibility polarization is
  // gridded onto which grid plane), make a map of the conjugate
  // planes of the grid E.g, for Stokes-I and -V imaging, the two
  // planes of the uv-grid are [LL,RR].  For input VisBuffer
  // visibilites in order [RR,RL,LR,LL], polMap = [1,-1,-1,0].  The
  // conjugate map will be [0,-1,-1,1].
  //
  Vector<Int> PBWProjectFT::makeConjPolMap(const VisBuffer& vb)
  {
    //
    // All the Natak (Drama) below with slicers etc. is to extract the
    // Poln. info. for the first IF only (not much "information
    // hiding" for the code to slice arrays in a general fashion).
    //
    // Extract the shape of the array to be sliced.
    Array<Int> stokesForAllIFs = vb.msColumns().polarization().corrType().getColumn();
    IPosition stokesShape(stokesForAllIFs.shape());
    IPosition firstIFStart(stokesShape),firstIFLength(stokesShape);
    // Set up the start and length IPositions to extract only the
    // first column of the array.  The following is required since the
    // array could have only one column as well.
    firstIFStart(0)=0;firstIFLength(0)=stokesShape(0);
    for(uInt i=1;i<stokesShape.nelements();i++) {firstIFStart(i)=0;firstIFLength(i)=1;}
    // Construct the slicer and produce the slice.  .nonDegenerate
    // required to ensure the result of slice is a pure vector.
    Vector<Int> visStokes = stokesForAllIFs(Slicer(firstIFStart,firstIFLength)).nonDegenerate();

    Vector<Int> conjPolMap;
    conjPolMap = polMap;
    
    Int i,j,N = polMap.nelements();
    for(i=0;i<N;i++)
      if (polMap[i] > -1)
	if      (visStokes[i] == Stokes::RR) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::LL) break; 
	    conjPolMap[i]=polMap[j];
	  }
	else if (visStokes[i] == Stokes::LL) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::RR) break; 
	    conjPolMap[i]=polMap[j];
	  }
	else if (visStokes[i] == Stokes::LR) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::RL) break; 
	    conjPolMap[i]=polMap[j];
	  }
	else if (visStokes[i] == Stokes::RL) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::LR) break; 
	    conjPolMap[i]=polMap[j];
	  }
    return conjPolMap;
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::findConvFunction(const ImageInterface<Complex>& image,
				      const VisBuffer& vb)
  {
    //    if (!vpSJ->changed(vb,1)) return;
    if (!paChangeDetector.changed(vb,0)) return;
    //    cout << "PA changed" << endl;

    logIO() << LogOrigin("PBWProjectFT", "findConvFunction")  << LogIO::NORMAL;
    
    ok();
    
    
    CoordinateSystem coords(image.coordinates());
    
    //
    // Make a two dimensional image to calculate auto-correlation of
    // the ideal illumination pattern. We want this on a fine grid in
    // the UV plane
    //
    Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
    AlwaysAssert(directionIndex>=0, AipsError);
    DirectionCoordinate dc=coords.directionCoordinate(directionIndex);
    directionCoord=coords.directionCoordinate(directionIndex);
    Vector<Double> sampling;
    sampling = dc.increment();
    sampling*=Double(convSampling);
    sampling*=Double(nx)/Double(convSize);
    dc.setIncrement(sampling);
    
    Vector<Double> unitVec(2);
    unitVec=convSize/2;
    dc.setReferencePixel(unitVec);
    
    // Set the reference value to that of the image
    coords.replaceCoordinate(dc, directionIndex);
    
    //
    // Make an image with circular polarization axis.  Return the
    // no. of vis. poln. planes that will be used in making the user
    // defined Stokes image.
    //
    polInUse=makePBPolnCoords(coords);
    
    Float pa;
    Int cfSource=locateConvFunction(wConvSize, polInUse, vb, pa);
    currentCFPA = pa;
    
    if (cfSource==1) // CF found and loaded from the disk cache
      {
	polInUse = convFunc.shape()(3);
	wConvSize = convFunc.shape()(2);
	try
	  {
	    cfCache.loadAvgPB(avgPB);
	  }
	catch (AipsError& err)
	  {
	    logIO() << err.getMesg() << LogIO::EXCEPTION;
	    throw(err);
	  }
      }
    else if (cfSource==2)  // CF found in the mem. cache
      {
      }
    else                     // CF not found in either cache
      {
	//
	// Make the CF, update the average PB and update the CF and
	// the avgPB disk cache
	//
	PAIndex = abs(cfSource);
        //
        // Load the average PB from the disk since it's going to be
        // updated in memory and on the disk.  Without loading it from
        // the disk (from a potentially more complete existing cache),
        // the average PB can get inconsistant with the rest of the
        // cache.
        //
	/*
	try
	  {
	    cfCache.loadAvgPB(avgPB);
	    resetPBs = False;
	  }
	catch(AipsError &err) // A more speciefic exception should be
			      // caught here. I (SB) just don't know
			      // which one!
	  {
	    logIO() << LogOrigin("PBWProjectFT::findConvFunction()","") 
		    << "Average PB does not exist in the cache - "
		    << "fresh one will be created in the disk cache" 
		    << LogIO::NORMAL 
		    << LogIO::POST;
	  }
	*/
	makeConvFunction(image,vb,pa);
	pbNormalized=False; normalizeAvgPB();
	cfCache.cacheConvFunction(PAIndex, pa, convFunc, coords, convSize,
				  convSupport,convSampling);
	cfCache.finalize(avgPB);
      }
    //    if (vpSJ->changed(vb,1)) vpSJ->update(vb,1);
    if (paChangeDetector.changed(vb,0)) paChangeDetector.update(vb,0);
    
    //
    // If mem. cache not yet ready and the latest CF was loaded from
    // the disk cache, compute and give some user useful info.
    //
    if ((!convFuncCacheReady) && (cfSource != 2))
      {
	//
	// Compute the aggregate memory used by the cached convolution
	// functions.
	//
	Int maxMemoryMB=HostInfo::memoryTotal()/1024;
	Float memoryKB=0;
	String unit(" KB");
	for(Int iPA=0;iPA<convFuncCache.nelements();iPA++)
	  {
	    Int volume=1;
	    if (convFuncCache[iPA] != NULL)
	      {
		IPosition shape=(*convFuncCache[iPA]).shape();
		volume=1;
		for(uInt i=0;i<shape.nelements();i++)
		  volume*=shape(i);
		memoryKB += volume;
	      }
	  }
	
	memoryKB = Int(memoryKB*sizeof(Complex)*2/1024.0+0.5);
	if (memoryKB > 1024) {memoryKB /=1024; unit=" MB";}
	
	logIO() << "Memory used in gridding functions = "
		<< (Int)(memoryKB+0.5) << unit << " out of a maximum of "
		<< maxMemoryMB << " MB" << LogIO::POST;
	//
	// Write out FT of screen as an image
	//
	//        convSampling=6;
	//        convSize=1024*2;
	//       cfCache.cacheConvFunction(PAIndex, pa, convFunc, coords, convSupport,convSampling);
	
	Int lastPASlot = PAIndex;
	//
	// Show the list of support sizes along the w-axis for the current PA.
	//
	IPosition sliceStart(3,0,0,lastPASlot),
	  sliceLength(3,wConvSize,1,1);
	logIO() << "Convolution support [CF#= " << lastPASlot 
		<< ", PA=" << pa*180.0/M_PI << "deg"
		<<"] = "
		<< convSupport(Slicer(sliceStart,sliceLength)).nonDegenerate()
		<< " pixels in Fourier plane"
		<< LogIO::POST;
      }
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::makeConvFunction(const ImageInterface<Complex>& image,
				      const VisBuffer& vb,Float pa)
  {
    //  PAIndex++;
    //  PAIndex = convFuncCache.nelements();
    Int NNN=0;
    if(wConvSize>NNN) {
      logIO() << "Using " << wConvSize << " planes for W-projection" << LogIO::POST;
      Double maxUVW;
      maxUVW=0.25/abs(image.coordinates().increment()(0));
      logIO() << "Estimating maximum possible W = " << maxUVW
	      << " (wavelengths)" << LogIO::POST;
      
      Double invLambdaC=vb.frequency()(0)/C::c;
      logIO() << "Typical wavelength = " << 1.0/invLambdaC
	      << " (m)" << LogIO::POST;
      
      if (wConvSize > 1)
	{
	  uvScale(2)=Float((wConvSize-1)*(wConvSize-1))/maxUVW;
	  logIO() << "Scaling in W (at maximum W) = " << 1.0/uvScale(2)
		  << " wavelengths per pixel" << LogIO::POST;
	}
    }
    //  
    // Get the coordinate system
    //
    CoordinateSystem coords(image.coordinates());
    
    //
    // Set up the convolution function. 
    //
    if(wConvSize>NNN) 
      {
	if(wConvSize>256) 
	  {
	    convSampling=4;
	    convSize=min(nx,512);
	  }
	else 
	  {
	    convSampling=4;
	    convSize=min(nx,2048);
	  }
      }
    else 
      {
	convSampling=4;
	convSize=nx;
      }
    convSampling=50;
    convSize=1024*2;
    //
    // Make a two dimensional image to calculate auto-correlation of
    // the ideal illumination pattern. We want this on a fine grid in
    // the UV plane
    //
    Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
    AlwaysAssert(directionIndex>=0, AipsError);
    DirectionCoordinate dc=coords.directionCoordinate(directionIndex);
    directionCoord=coords.directionCoordinate(directionIndex);
    Vector<Double> sampling;
    sampling = dc.increment();

    sampling*=Double(convSampling);
    sampling*=Double(nx)/Double(convSize);
    dc.setIncrement(sampling);
    
    Vector<Double> unitVec(2);
    unitVec=convSize/2;
    dc.setReferencePixel(unitVec);
    
    // Set the reference value to that of the image
    coords.replaceCoordinate(dc, directionIndex);
    
    //
    // Make an image with circular polarization axis.  Return the
    // no. of vis. poln. planes that will be used in making the user
    // defined Stokes image.
    //
    polInUse=makePBPolnCoords(coords);
    //  convSupport.resize(wConvSize,polInUse,4,True);
    convSupport.resize(wConvSize,polInUse,PAIndex+1,True);
    Int N=convFuncCache.nelements();
    convFuncCache.resize(PAIndex+1,True);
    for(Int i=N;i<PAIndex;i++) convFuncCache[i]=NULL;
    //------------------------------------------------------------------
    //
    // Make the sky Stokes PB.  This will be used in the gridding
    // correction
    //
    //------------------------------------------------------------------
    IPosition pbShape(4, convSize, convSize, polInUse, 1);
    TempImage<Complex> twoDPB(pbShape, coords);
    twoDPB.set(Complex(1.0,0.0));
    //
    // Accumulate the various terms that constitute the gridding
    // convolution function.
    //
    //------------------------------------------------------------------
    Bool writeResults;
    writeResults=False;
    
    Int inner=convSize/convSampling;
    inner = convSize/2;
    //  inner = convSize;
    ConvolveGridder<Double, Complex>
      ggridder(IPosition(2, inner, inner), uvScale, uvOffset, "SF");
    
    convFuncCache[PAIndex] = new Array<Complex>(IPosition(4,convSize/2,convSize/2,
							  wConvSize,polInUse));
    convFunc.reference(*convFuncCache[PAIndex]);
    //  convFunc.resize(IPosition(4,convSize/2-1, convSize/2-1, wConvSize, polInUse));
    convFunc=0.0;
    
    
    IPosition start(4, 0, 0, 0, 0);
    IPosition pbSlice(4, convSize, convSize, 1, 1);
    
    Matrix<Complex> screen(convSize, convSize);
    //    if (vpSJ->changed(vb,1)) vpSJ->update(vb,1);
    if (paChangeDetector.changed(vb,0)) paChangeDetector.update(vb,0);
    VLACalcIlluminationConvFunc vlaPB;

    for (Int iw=0;iw<wConvSize;iw++) 
      {
	screen=0.0;
	//
	// First the spheroidal function
	//
	//      inner=convSize/2;
	Vector<Complex> correction(inner);
	for (Int iy=-inner/2;iy<inner/2;iy++) 
	  {
	    ggridder.correctX1D(correction, iy+inner/2);
	    for (Int ix=-inner/2;ix<inner/2;ix++) 
	      screen(ix+convSize/2,iy+convSize/2)=correction(ix+inner/2);
	  }
	
	//
	// Now the w term
	//
	if(wConvSize>1) 
	  {
	    Double twoPiW=2.0*C::pi*Double(iw*iw)/uvScale(2);
	    
	    for (Int iy=-inner/2;iy<inner/2;iy++) 
	      {
		Double m=sampling(1)*Double(iy);
		Double msq=m*m;
		for (Int ix=-inner/2;ix<inner/2;ix++) 
		  {
		    Double l=sampling(0)*Double(ix);
		    Double rsq=l*l+msq;
		    if(rsq<1.0) 
		      {
			Double phase=twoPiW*(sqrt(1.0-rsq)-1.0);
			screen(ix+convSize/2,iy+convSize/2)*=Complex(cos(phase),sin(phase));
		      }
		  }
	      }
	  }
	
	//
	// Fill the complex image with the w-term...
	//
	IPosition PolnPlane(4,0,0,0,0);
	IPosition ndx(4,0,0,0,0);
	
	for(Int i=0;i<polInUse;i++)
	  {
	    PolnPlane(2)=i;
	    twoDPB.putSlice(screen, PolnPlane);
	  }
	//
	// Apply the PB...
	//
	/*
	  {
	  if (vpSJ->changed(vb,1)) {vpSJ->update(vb,1);}
	  VLAIlluminationConvFunc vlaPB;
	  
	  Vector<Int> whichStokes(2);
	  whichStokes(0)=Stokes::RR;
	  whichStokes(1)=Stokes::LL;
	  String fileName="vla1.5GHz.aperture";
	  Float overSampling=2.1; // 10% buffer - buffer needed to get re-gridding right
	  vlaPB.load(fileName,whichStokes,overSampling,True);
	  vlaPB.applyPB(twoDPB, vb);
	  }
	*/
	vlaPB.applyPB(twoDPB, vb, bandID_p);
	//
	// Now FFT and get the result back
	//
	LatticeFFT::cfft2d(twoDPB);
	//
	// Fill the convolution function planes with the result.
	//
	{
	  IPosition start(4, convSize/4, convSize/4, 0, 0),
	    //	  pbSlice(4, convSize/2-1, convSize/2-1, 2, 1);
	    pbSlice(4, convSize/2-1, convSize/2-1, polInUse, 1);
	  
	  IPosition sliceStart(4,0,0,iw,0), 
	    sliceLength(4,convSize/2-1,convSize/2-1,1,polInUse);
	  
	  convFunc(Slicer(sliceStart,sliceLength)).nonDegenerate()
	    =twoDPB.getSlice(start, pbSlice, True);
	}
      }
    
    {
      Complex cpeak = max(convFunc);
      convFunc/=cpeak;
    }
    //
    // Find the convolution function support size.  No assumption
    // about the symmetry of the conv. func. can be made (except that
    // they are same for all poln. planes).
    //
    Int lastPASlot=PAIndex;
    for(Int iw=0;iw<wConvSize;iw++)
      for(Int ipol=0;ipol<polInUse;ipol++)
	convSupport(iw,ipol,lastPASlot)=-1;
    //
    // !!!Caution: This assumes that the support size at each Poln. is
    // the same, starting from the center pixel (in pixel
    // co-ordinates).  For large pointing offsets, this might not be
    // true.
    //
    Int ConvFuncOrigin=convSize/4;  // Conv. Func. is half that size of convSize
    IPosition ndx(4,ConvFuncOrigin,0,0,0);
    for (Int iw=0;iw<wConvSize;iw++)
      {
	Bool found=False;
	ndx(2) = iw;
	Float threshold;
	
	//      Int start=ConvFuncOrigin + convSize/4;
	ndx(0)=ndx(1)=ConvFuncOrigin;
	ndx(2) = iw;
	Complex maxVal = max(convFunc);
	threshold = abs(convFunc(ndx))*1E-3;
	//
	// The functions are not azimuthally symmetric - so compute
	// the support size carefully.  Collect every PixInc pixel
	// along a quarter circle of radius R (four-fold azimuthal
	// symmetry is assumed), and check if any pixel in this list
	// is above the threshold. If TRUE, then R is the support
	// radius.  Else decrease R and check again.
	//
	Vector<Complex> vals;
	Int PixInc=1,NSteps;
	IPosition cfShape=convFunc.shape();
	Int R;
	for(R=convSize/4;R>1;R--)
	  {
	    NSteps = 90*R/PixInc; //Check every PixInc pixel along a
				  //circle of radious R
	    vals.resize(NSteps);
	    vals=0;
	    for(Int th=0;th<NSteps;th++)
	      {
		ndx(0)=(int)(ConvFuncOrigin + R*sin(2.0*M_PI*th*PixInc/R));
		ndx(1)=(int)(ConvFuncOrigin + R*cos(2.0*M_PI*th*PixInc/R));

		if ((ndx(0) < cfShape(0)) && (ndx(1) < cfShape(1)))
		    vals(th)=convFunc(ndx);
	      }
	    if (max(abs(vals)) > threshold)
	      {found=True;break;}
	  }
	//	R *=2;
	//
	// Set the support size for each W-plane and for all
	// Pol-planes.  Assuming that support size for all Pol-planes
	// is same.
	//
	if(found) 
	  {
	    Int maxR=R;//max(ndx(0),ndx(1));
	    for(Int ipol=0;ipol<polInUse;ipol++)
	      {
// 		convSupport(iw,ipol,lastPASlot)=Int(0.5+abs(Float(ConvFuncOrigin-maxR))
// 						    /Float(convSampling))+1;
		convSupport(iw,ipol,lastPASlot)=Int(R/Float(convSampling));
		//		cout << "######Check the logic of computing maxConvSupport" << endl;
		//		if (maxConvSupport == -1)
		if (lastPASlot == 0)
		  if (convSupport(iw,ipol,lastPASlot) > maxConvSupport)
		    maxConvSupport = convSupport(iw,ipol,lastPASlot);
	      }
	  }
      }
    
    //  if (lastPASlot == 0) maxConvSupport += 10;
    
    if(convSupport(0,0,lastPASlot)<1) 
      logIO() << "Convolution function is misbehaved - support seems to be zero"
	      << LogIO::EXCEPTION;
    
    logIO() << LogOrigin("PBWProjectFT", "Re-sizing the convolution functions")
	    << "Re-sizing the convolution functions"
	    << LogIO::POST;
    
    {
      Int bot=ConvFuncOrigin-convSampling*maxConvSupport,//-convSampling/2, 
      top=ConvFuncOrigin+convSampling*maxConvSupport;//+convSampling/2;
      
      Array<Complex> tmp;
      IPosition blc(4,bot,bot,0,0), trc(4,top,top,wConvSize-1,polInUse-1);
      
      tmp = convFunc(blc,trc);
      (*convFuncCache[lastPASlot]).resize(tmp.shape());
      (*convFuncCache[lastPASlot]) = tmp; 
      convFunc.reference(*convFuncCache[lastPASlot]);
    }    
    
    //
    // Normalize such that plane 0 sums to 1 (when jumping in steps of
    // convSampling).  This is really not necessary here since we do
    // the normalizing by the area more accurately in the gridder
    // (fpbwproj.f).
    //
    ndx(2)=ndx(3)=0;
    
    
    Complex pbSum=0.0;
    
    for(Int nw=0;nw<wConvSize;nw++)
      for(Int np=0;np<polInUse;np++)
	{
	  ndx(2) = nw; ndx(3)=np;
	  
	  ConvFuncOrigin = convFunc.shape()(0)/2+1;
	  Int thisConvSupport=convSampling*convSupport(nw,np,lastPASlot);
	  pbSum=0.0;
	  for(Int iy=-thisConvSupport;iy<thisConvSupport;iy+=convSampling)
	    for(Int ix=-thisConvSupport;ix<thisConvSupport;ix+=convSampling)
	      {
		ndx(0)=ix+ConvFuncOrigin;ndx(1)=iy+ConvFuncOrigin;
		pbSum += convFunc(ndx);
	      }
	  if(pbSum>0.0)  
	    {
	      //
	      // Normalize each Poln. plane by the area under its convfunc.
	      //
	      Int Nx = convFunc.shape()(0), Ny=convFunc.shape()(1);
	      
	      for (ndx(1)=0;ndx(1)<Ny;ndx(1)++) 
		for (ndx(0)=0;ndx(0)<Nx;ndx(0)++) 
		  convFunc(ndx) /= pbSum;
	    }
	  else 
	    throw(SynthesisFTMachineError("Convolution function integral is not positive"));
	  pbSum=0.0;
	  for(Int iy=-thisConvSupport;iy<thisConvSupport;iy+=convSampling)
	    for(Int ix=-thisConvSupport;ix<thisConvSupport;ix+=convSampling)
	      {
		ndx(0)=ix+ConvFuncOrigin;ndx(1)=iy+ConvFuncOrigin;
		pbSum += convFunc(ndx);
	      }
	}
    //
    // Make the average PB
    //
    makeAveragePB(vb, image, polInUse,avgPB);
  }
  //
  //------------------------------------------------------------------------------
  //
  void PBWProjectFT::initializeToVis(ImageInterface<Complex>& iimage,
				     const VisBuffer& vb)
  {
    image=&iimage;
    
    ok();
    
    init();
    makingPSF = False;
    initMaps(vb);
    
    findConvFunction(*image, vb);
    //  
    // Initialize the maps for polarization and channel. These maps
    // translate visibility indices into image indices
    //
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
    
    if(image->shape().product()>cachesize) isTiled=True;
    else                                   isTiled=False;
    //
    // If we are memory-based then read the image in and create an
    // ArrayLattice otherwise just use the PagedImage
    //
    if(isTiled)  lattice=image;
    else 
      {
	IPosition gridShape(4, nx, ny, npol, nchan);
	griddedData.resize(gridShape);
	griddedData=Complex(0.0);
	
	IPosition stride(4, 1);
	IPosition blc(4, (nx-image->shape()(0))/2,
		      (ny-image->shape()(1))/2, 0, 0);
	IPosition trc(blc+image->shape()-stride);
	
	IPosition start(4, 0);
	griddedData(blc, trc) = image->getSlice(start, image->shape());
	
	if(arrayLattice) delete arrayLattice; arrayLattice=0;
	arrayLattice = new ArrayLattice<Complex>(griddedData);
	lattice=arrayLattice;
      }
    
    AlwaysAssert(lattice, AipsError);
    
    logIO() << LogIO::DEBUGGING << "Starting FFT of image" << LogIO::POST;
    
    Vector<Float> sincConv(nx);
    Float centerX=nx/2+1;
    for (Int ix=0;ix<nx;ix++) 
      {
	Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
	//      Float x=C::pi*Float(ix-centerX)*Float(convSupport(0))/(Float(nx));
	if(ix==centerX) sincConv(ix)=1.0;
	else            sincConv(ix)=sin(x)/x;
      }
    
    Vector<Complex> correction(nx);
    //  correction=Complex(1.0, 0.0);
    //
    // Do the Grid-correction
    //
    {
      normalizeAvgPB();
      
      IPosition cursorShape(4, nx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
      
      LatticeStepper lpb(avgPB.shape(),cursorShape,axisPath);
      LatticeIterator<Float> lipb(avgPB, lpb);
      
      Vector<Complex> griddedVis;
      //
      // Grid correct in anticipation of the convolution by the
      // convFunc.  Each polarization plane is corrected by the
      // appropraite primary beam.
      //
      for(lix.reset(),lipb.reset();!lix.atEnd();lix++,lipb++) 
	{
	  Int iy=lix.position()(1);
	  gridder->correctX1D(correction,iy);
	  griddedVis = lix.rwVectorCursor();
	  
	  Vector<Float> PBCorrection(lipb.rwVectorCursor().shape());
	  PBCorrection = lipb.rwVectorCursor();
	  // Int polnPlane = lipb.position()(2);
	  
	  for(int ix=0;ix<nx;ix++) 
	    {
	      // PBCorrection(ix) = (FUNC(PBCorrection(ix)))/(sincConv(ix)*sincConv(iy));
	      
	      if (doPBCorrection)
		{
		  PBCorrection(ix) = FUNC(PBCorrection(ix))/(sincConv(ix)*sincConv(iy));
 		  if ((abs(PBCorrection(ix)*correction(ix))) >= pbLimit_p)
		    {lix.rwVectorCursor()(ix) /= (PBCorrection(ix))*correction(ix);}
 		  else
		    // 		    {lix.rwVectorCursor()(ix) /= correction(ix);}
 		    {lix.rwVectorCursor()(ix) *= (sincConv(ix)*sincConv(iy));}
		}
	      else 
		lix.rwVectorCursor()(ix) /= (correction(ix)/(sincConv(ix)*sincConv(iy)));
	    }
	}
    }
    //
    // Now do the FFT2D in place
    //
    LatticeFFT::cfft2d(*lattice);
    
    logIO() << LogIO::DEBUGGING << "Finished FFT" << LogIO::POST;
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::initializeToVis(ImageInterface<Complex>& iimage,
				     const VisBuffer& vb,
				     Array<Complex>& griddedVis,
				     Vector<Double>& uvscale)
  {
    initializeToVis(iimage, vb);
    griddedVis.assign(griddedData); //using the copy for storage
    uvscale.assign(uvScale);
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::finalizeToVis()
  {
    logIO() << "##########finalizeToVis()###########" << LogIO::DEBUGGING << LogIO::POST;
    if(isTiled) 
      {
	logIO() << LogOrigin("PBWProjectFT", "finalizeToVis")  << LogIO::NORMAL;
	
	AlwaysAssert(imageCache, AipsError);
	AlwaysAssert(image, AipsError);
	ostringstream o;
	imageCache->flush();
	imageCache->showCacheStatistics(o);
	logIO() << o.str() << LogIO::POST;
      }
    if(pointingToImage) delete pointingToImage; pointingToImage=0;
  }
  //
  //---------------------------------------------------------------
  //
  // Initialize the FFT to the Sky. Here we have to setup and
  // initialize the grid.
  //
  void PBWProjectFT::initializeToSky(ImageInterface<Complex>& iimage,
				     Matrix<Float>& weight,
				     const VisBuffer& vb)
  {
    logIO() << "#########initializeToSky()##########" << LogIO::DEBUGGING << LogIO::POST;
    
    // image always points to the image
    image=&iimage;
    
    //  if(convSize==0) 
    init();
    initMaps(vb);
    //  Bool vpSJChanged=vpSJ->changed(vb,1);
    //  if (vpSJChanged) 
    
    //  findConvFunction(*image, vb);
    
    // Initialize the maps for polarization and channel. These maps
    // translate visibility indices into image indices
    
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
    
    if(image->shape().product()>cachesize) isTiled=True;
    else                                   isTiled=False;
    
    
    sumWeight=0.0;
    weight.resize(sumWeight.shape());
    weight=0.0;
    //
    // Initialize for in memory or to disk gridding. lattice will
    // point to the appropriate Lattice, either the ArrayLattice for
    // in memory gridding or to the image for to disk gridding.
    //
    if(isTiled) 
      {
	imageCache->flush();
	image->set(Complex(0.0));
	lattice=image;
      }
    else 
      {
	IPosition gridShape(4, nx, ny, npol, nchan);
	griddedData.resize(gridShape);
	griddedData=Complex(0.0);
	if(arrayLattice) delete arrayLattice; arrayLattice=0;
	arrayLattice = new ArrayLattice<Complex>(griddedData);
	lattice=arrayLattice;
      }
    AlwaysAssert(lattice, AipsError);
    //  resetPBs = True; 
    //pbNormalized = False;
    PAIndex = -1;
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::finalizeToSky()
  {
    //
    // Now we flush the cache and report statistics For memory based,
    // we don't write anything out yet.
    //
    logIO() << "#########finalizeToSky()#########" << LogIO::DEBUGGING << LogIO::POST;
    if(isTiled) 
      {
	logIO() << LogOrigin("PBWProjectFT", "finalizeToSky")  << LogIO::NORMAL;
	
	AlwaysAssert(image, AipsError);
	AlwaysAssert(imageCache, AipsError);
	imageCache->flush();
	ostringstream o;
	imageCache->showCacheStatistics(o);
	logIO() << o.str() << LogIO::POST;
      }
    if(pointingToImage) delete pointingToImage; pointingToImage=0;
    PAIndex = -1;
    //    vpSJ->reset();
    paChangeDetector.reset();
    cfCache.finalize();
    convFuncCacheReady=True;
    /*
      {
      cout << "Writing average PB" << endl;
      normalizeAvgPB();
      ostringstream Name;
      Name << "avgPB.im";
      {
      PagedImage<Float> tmp(avgPB.shape(), avgPB.coordinates(), Name);
      LatticeExpr<Float> le(abs(avgPB));
      tmp.copyData(le);
      }
      }
    */
  }
  //
  //---------------------------------------------------------------
  //
  Array<Complex>* PBWProjectFT::getDataPointer(const IPosition& centerLoc2D,
					       Bool readonly) 
  {
    Array<Complex>* result;
    // Is tiled: get tiles and set up offsets
    centerLoc(0)=centerLoc2D(0);
    centerLoc(1)=centerLoc2D(1);
    result=&imageCache->tile(offsetLoc, centerLoc, readonly);
    gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
    return result;
  }
  
#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define gpbwproj gpbwproj_
#define dpbwproj dpbwproj_
#define dpbwgrad dpbwgrad_
#endif
  
  extern "C" { 
    void gpbwproj(Double *uvw,
		  Double *dphase,
		  const Complex *values,
		  Int *nvispol,
		  Int *nvischan,
		  Int *dopsf,
		  const Int *flag,
		  const Int *rflag,
		  const Float *weight,
		  Int *nrow,
		  Int *rownum,
		  Double *scale,
		  Double *offset,
		  Complex *grid,
		  Int *nx,
		  Int *ny,
		  Int *npol,
		  Int *nchan,
		  const Double *freq,
		  const Double *c,
		  Int *support,
		  Int *convsize,
		  Int *sampling,
		  Int *wconvsize,
		  Complex *convfunc,
		  Int *chanmap,
		  Int *polmap,
		  Int *polused,
		  Double *sumwt,
		  Int *ant1,
		  Int *ant2,
		  Int *nant,
		  Int *scanno,
		  Double *sigma,
		  Float *raoff,
		  Float *decoff,
		  Double *area,
		  Int *doGrad,
		  Int *doPointingCorrection,
		  Int *nPA,
		  Int *paIndex,
		  Int *CFMap,
		  Int *ConjCFMap,
		  Double *currentCFPA, Double *actualPA);
    void dpbwproj(Double *uvw,
		  Double *dphase,
		  Complex *values,
		  Int *nvispol,
		  Int *nvischan,
		  const Int *flag,
		  const Int *rflag,
		  Int *nrow,
		  Int *rownum,
		  Double *scale,
		  Double *offset,
		  const Complex *grid,
		  Int *nx,
		  Int *ny,
		  Int *npol,
		  Int *nchan,
		  const Double *freq,
		  const Double *c,
		  Int *support,
		  Int *convsize,
		  Int *sampling,
		  Int *wconvsize,
		  Complex *convfunc,
		  Int *chanmap,
		  Int *polmap,
		  Int *polused,
		  Int *ant1, 
		  Int *ant2, 
		  Int *nant, 
		  Int *scanno,
		  Double *sigma, 
		  Float *raoff, Float *decoff,
		  Double *area, 
		  Int *dograd,
		  Int *doPointingCorrection,
		  Int *nPA,
		  Int *paIndex,
		  Int *CFMap,
		  Int *ConjCFMap,
		  Double *currentCFPA, Double *actualPA);
    void dpbwgrad(Double *uvw,
		  Double *dphase,
		  Complex *values,
		  Int *nvispol,
		  Int *nvischan,
		  Complex *gazvalues,
		  Complex *gelvalues,
		  Int *doconj,
		  const Int *flag,
		  const Int *rflag,
		  Int *nrow,
		  Int *rownum,
		  Double *scale,
		  Double *offset,
		  const Complex *grid,
		  Int *nx,
		  Int *ny,
		  Int *npol,
		  Int *nchan,
		  const Double *freq,
		  const Double *c,
		  Int *support,
		  Int *convsize,
		  Int *sampling,
		  Int *wconvsize,
		  Complex *convfunc,
		  Int *chanmap,
		  Int *polmap,
		  Int *polused,
		  Int *ant1, 
		  Int *ant2, 
		  Int *nant, 
		  Int *scanno,
		  Double *sigma, 
		  Float *raoff, Float *decoff,
		  Double *area, 
		  Int *dograd,
		  Int *doPointingCorrection,
		  Int *nPA,
		  Int *paIndex,
		  Int *CFMap,
		  Int *ConjCFMap,
		  Double *currentCFPA, Double *actualPA);
  }
  //
  //----------------------------------------------------------------------
  //
  void PBWProjectFT::runFortranGet(Matrix<Double>& uvw,Vector<Double>& dphase,
				   Cube<Complex>& visdata,
				   IPosition& s,
				   //				Cube<Complex>& gradVisAzData,
				   //				Cube<Complex>& gradVisElData,
				   //				IPosition& gradS,
				   Int& Conj,
				   Cube<Int>& flags,Vector<Int>& rowFlags,
				   Int& rownr,Vector<Double>& actualOffset,
				   Array<Complex>* dataPtr,
				   Int& aNx, Int& aNy, Int& npol, Int& nchan,
				   VisBuffer& vb,Int& Nant_p, Int& scanNo,
				   Double& sigma,
				   Array<Float>& l_off,
				   Array<Float>& m_off,
				   Double area,
				   Int& doGrad,
				   Int paIndex)
  {
    enum whichGetStorage {RAOFF,DECOFF,UVW,DPHASE,VISDATA,GRADVISAZ,GRADVISEL,
			  FLAGS,ROWFLAGS,UVSCALE,ACTUALOFFSET,DATAPTR,VBFREQ,
			  CONVSUPPORT,CONVFUNC,CHANMAP,POLMAP,VBANT1,VBANT2,CONJCFMAP,CFMAP};
    Vector<Bool> deleteThem(21);
    
    Double *uvw_p, *dphase_p, *actualOffset_p, *vb_freq_p, *uvScale_p;
    Complex *visdata_p, *dataPtr_p, *f_convFunc_p;
    //  Complex *gradVisAzData_p, *gradVisElData_p;
    Int *flags_p, *rowFlags_p, *chanMap_p, *polMap_p, *convSupport_p, *vb_ant1_p, *vb_ant2_p,
      *ConjCFMap_p, *CFMap_p;
    Float *l_off_p, *m_off_p;
    Double actualPA;
    
    Vector<Int> ConjCFMap, CFMap;
    /*
      ConjCFMap = CFMap = polMap;
      CFMap = makeConjPolMap(vb);
    */
    Int N;
    actualPA = getPA(vb);

    N=polMap.nelements();
    CFMap = polMap; ConjCFMap = polMap;
    for(Int i=0;i<N;i++) CFMap[i] = polMap[N-i-1];
    
    ConjCFMap_p     = ConjCFMap.getStorage(deleteThem(CONJCFMAP));
    CFMap_p         = CFMap.getStorage(deleteThem(CFMAP));
    
    uvw_p           = uvw.getStorage(deleteThem(UVW));
    dphase_p        = dphase.getStorage(deleteThem(DPHASE));
    visdata_p       = visdata.getStorage(deleteThem(VISDATA));
    //  gradVisAzData_p = gradVisAzData.getStorage(deleteThem(GRADVISAZ));
    //  gradVisElData_p = gradVisElData.getStorage(deleteThem(GRADVISEL));
    flags_p         = flags.getStorage(deleteThem(FLAGS));
    rowFlags_p      = rowFlags.getStorage(deleteThem(ROWFLAGS));
    uvScale_p       = uvScale.getStorage(deleteThem(UVSCALE));
    actualOffset_p  = actualOffset.getStorage(deleteThem(ACTUALOFFSET));
    dataPtr_p       = dataPtr->getStorage(deleteThem(DATAPTR));
    vb_freq_p       = vb.frequency().getStorage(deleteThem(VBFREQ));
    convSupport_p   = convSupport.getStorage(deleteThem(CONVSUPPORT));
    f_convFunc_p      = convFunc.getStorage(deleteThem(CONVFUNC));
    chanMap_p       = chanMap.getStorage(deleteThem(CHANMAP));
    polMap_p        = polMap.getStorage(deleteThem(POLMAP));
    vb_ant1_p       = vb.antenna1().getStorage(deleteThem(VBANT1));
    vb_ant2_p       = vb.antenna2().getStorage(deleteThem(VBANT2));
    l_off_p     = l_off.getStorage(deleteThem(RAOFF));
    m_off_p    = m_off.getStorage(deleteThem(DECOFF));
    
    Int npa=convSupport.shape()(2),actualConvSize;
    Int paIndex_Fortran = paIndex;
    actualConvSize = convFunc.shape()(0);
    
    IPosition shp=convSupport.shape();
    
    dpbwproj(uvw_p,
	     dphase_p,
	     //		  vb.modelVisCube().getStorage(del),
	     visdata_p,
	     &s(0),
	     &s(1),
	     //	   gradVisAzData_p,
	     //	   gradVisElData_p,
	     //	    &gradS(0),
	     //	    &gradS(1),
	     //	   &Conj,
	     flags_p,
	     rowFlags_p,
	     &s(2),
	     &rownr,
	     uvScale_p,
	     actualOffset_p,
	     dataPtr_p,
	     &aNx,
	     &aNy,
	     &npol,
	     &nchan,
	     vb_freq_p,
	     &C::c,
	     convSupport_p,
	     &actualConvSize,
	     &convSampling,
	     &wConvSize,
	     f_convFunc_p,
	     chanMap_p,
	     polMap_p,
	     &polInUse,
	     vb_ant1_p,
	     vb_ant2_p,
	     &Nant_p,
	     &scanNo,
	     &sigma,
	     l_off_p, m_off_p,
	     &area,
	     &doGrad,
	     &doPointing,
	     &npa,
	     &paIndex_Fortran,
	     CFMap_p,
	     ConjCFMap_p,
	     &currentCFPA
	     ,&actualPA
	     );
    
    ConjCFMap.freeStorage((const Int *&)ConjCFMap_p,deleteThem(CONJCFMAP));
    CFMap.freeStorage((const Int *&)CFMap_p,deleteThem(CFMAP));
    
    l_off.freeStorage((const Float*&)l_off_p,deleteThem(RAOFF));
    m_off.freeStorage((const Float*&)m_off_p,deleteThem(DECOFF));
    uvw.freeStorage((const Double*&)uvw_p,deleteThem(UVW));
    dphase.freeStorage((const Double*&)dphase_p,deleteThem(DPHASE));
    visdata.putStorage(visdata_p,deleteThem(VISDATA));
    //  gradVisAzData.putStorage(gradVisAzData_p,deleteThem(GRADVISAZ));
    //  gradVisElData.putStorage(gradVisElData_p,deleteThem(GRADVISEL));
    flags.freeStorage((const Int*&) flags_p,deleteThem(FLAGS));
    rowFlags.freeStorage((const Int *&)rowFlags_p,deleteThem(ROWFLAGS));
    actualOffset.freeStorage((const Double*&)actualOffset_p,deleteThem(ACTUALOFFSET));
    dataPtr->freeStorage((const Complex *&)dataPtr_p,deleteThem(DATAPTR));
    uvScale.freeStorage((const Double*&) uvScale_p,deleteThem(UVSCALE));
    vb.frequency().freeStorage((const Double*&)vb_freq_p,deleteThem(VBFREQ));
    convSupport.freeStorage((const Int*&)convSupport_p,deleteThem(CONVSUPPORT));
    convFunc.freeStorage((const Complex *&)f_convFunc_p,deleteThem(CONVFUNC));
    chanMap.freeStorage((const Int*&)chanMap_p,deleteThem(CHANMAP));
    polMap.freeStorage((const Int*&) polMap_p,deleteThem(POLMAP));
    vb.antenna1().freeStorage((const Int*&) vb_ant1_p,deleteThem(VBANT1));
    vb.antenna2().freeStorage((const Int*&) vb_ant2_p,deleteThem(VBANT2));
  }
  //
  //----------------------------------------------------------------------
  //
  void PBWProjectFT::runFortranGetGrad(Matrix<Double>& uvw,Vector<Double>& dphase,
				       Cube<Complex>& visdata,
				       IPosition& s,
				       Cube<Complex>& gradVisAzData,
				       Cube<Complex>& gradVisElData,
				       //				     IPosition& gradS,
				       Int& Conj,
				       Cube<Int>& flags,Vector<Int>& rowFlags,
				       Int& rownr,Vector<Double>& actualOffset,
				       Array<Complex>* dataPtr,
				       Int& aNx, Int& aNy, Int& npol, Int& nchan,
				       VisBuffer& vb,Int& Nant_p, Int& scanNo,
				       Double& sigma,
				       Array<Float>& l_off,
				       Array<Float>& m_off,
				       Double area,
				       Int& doGrad,
				       Int paIndex)
  {
    enum whichGetStorage {RAOFF,DECOFF,UVW,DPHASE,VISDATA,GRADVISAZ,GRADVISEL,
			  FLAGS,ROWFLAGS,UVSCALE,ACTUALOFFSET,DATAPTR,VBFREQ,
			  CONVSUPPORT,CONVFUNC,CHANMAP,POLMAP,VBANT1,VBANT2,CONJCFMAP,CFMAP};
    Vector<Bool> deleteThem(21);
    
    Double *uvw_p, *dphase_p, *actualOffset_p, *vb_freq_p, *uvScale_p;
    Complex *visdata_p, *dataPtr_p, *f_convFunc_p;
    Complex *gradVisAzData_p, *gradVisElData_p;
    Int *flags_p, *rowFlags_p, *chanMap_p, *polMap_p, *convSupport_p, *vb_ant1_p, *vb_ant2_p,
      *ConjCFMap_p, *CFMap_p;
    Float *l_off_p, *m_off_p;
    Double actualPA;

    Vector<Int> ConjCFMap, CFMap;
    actualPA = getPA(vb);
    ConjCFMap = polMap;
    CFMap = makeConjPolMap(vb);
    /*
      Int N;
      N=polMap.nelements();
      CFMap = polMap; ConjCFMap = polMap;
      for(Int i=0;i<N;i++) CFMap[i] = polMap[N-i-1];
    */
    ConjCFMap_p     = ConjCFMap.getStorage(deleteThem(CONJCFMAP));
    CFMap_p         = CFMap.getStorage(deleteThem(CFMAP));
    
    uvw_p           = uvw.getStorage(deleteThem(UVW));
    dphase_p        = dphase.getStorage(deleteThem(DPHASE));
    visdata_p       = visdata.getStorage(deleteThem(VISDATA));
    gradVisAzData_p = gradVisAzData.getStorage(deleteThem(GRADVISAZ));
    gradVisElData_p = gradVisElData.getStorage(deleteThem(GRADVISEL));
    flags_p         = flags.getStorage(deleteThem(FLAGS));
    rowFlags_p      = rowFlags.getStorage(deleteThem(ROWFLAGS));
    uvScale_p       = uvScale.getStorage(deleteThem(UVSCALE));
    actualOffset_p  = actualOffset.getStorage(deleteThem(ACTUALOFFSET));
    dataPtr_p       = dataPtr->getStorage(deleteThem(DATAPTR));
    vb_freq_p       = vb.frequency().getStorage(deleteThem(VBFREQ));
    convSupport_p   = convSupport.getStorage(deleteThem(CONVSUPPORT));
    f_convFunc_p      = convFunc.getStorage(deleteThem(CONVFUNC));
    chanMap_p       = chanMap.getStorage(deleteThem(CHANMAP));
    polMap_p        = polMap.getStorage(deleteThem(POLMAP));
    vb_ant1_p       = vb.antenna1().getStorage(deleteThem(VBANT1));
    vb_ant2_p       = vb.antenna2().getStorage(deleteThem(VBANT2));
    l_off_p     = l_off.getStorage(deleteThem(RAOFF));
    m_off_p    = m_off.getStorage(deleteThem(DECOFF));
    
    Int npa=convSupport.shape()(2),actualConvSize;
    Int paIndex_Fortran = paIndex;
    actualConvSize = convFunc.shape()(0);
    
    IPosition shp=convSupport.shape();

    dpbwgrad(uvw_p,
	     dphase_p,
	     //		  vb.modelVisCube().getStorage(del),
	     visdata_p,
	     &s(0),
	     &s(1),
	     gradVisAzData_p,
	     gradVisElData_p,
	     //	    &gradS(0),
	     //	    &gradS(1),
	     &Conj,
	     flags_p,
	     rowFlags_p,
	     &s(2),
	     &rownr,
	     uvScale_p,
	     actualOffset_p,
	     dataPtr_p,
	     &aNx,
	     &aNy,
	     &npol,
	     &nchan,
	     vb_freq_p,
	     &C::c,
	     convSupport_p,
	     &actualConvSize,
	     &convSampling,
	     &wConvSize,
	     f_convFunc_p,
	     chanMap_p,
	     polMap_p,
	     &polInUse,
	     vb_ant1_p,
	     vb_ant2_p,
	     &Nant_p,
	     &scanNo,
	     &sigma,
	     l_off_p, m_off_p,
	     &area,
	     &doGrad,
	     &doPointing,
	     &npa,
	     &paIndex_Fortran,
	     CFMap_p,
	     ConjCFMap_p,
	     &currentCFPA
	     ,&actualPA
	     );

    ConjCFMap.freeStorage((const Int *&)ConjCFMap_p,deleteThem(CONJCFMAP));
    CFMap.freeStorage((const Int *&)CFMap_p,deleteThem(CFMAP));
    
    l_off.freeStorage((const Float*&)l_off_p,deleteThem(RAOFF));
    m_off.freeStorage((const Float*&)m_off_p,deleteThem(DECOFF));
    uvw.freeStorage((const Double*&)uvw_p,deleteThem(UVW));
    dphase.freeStorage((const Double*&)dphase_p,deleteThem(DPHASE));
    visdata.putStorage(visdata_p,deleteThem(VISDATA));
    gradVisAzData.putStorage(gradVisAzData_p,deleteThem(GRADVISAZ));
    gradVisElData.putStorage(gradVisElData_p,deleteThem(GRADVISEL));
    flags.freeStorage((const Int*&) flags_p,deleteThem(FLAGS));
    rowFlags.freeStorage((const Int *&)rowFlags_p,deleteThem(ROWFLAGS));
    actualOffset.freeStorage((const Double*&)actualOffset_p,deleteThem(ACTUALOFFSET));
    dataPtr->freeStorage((const Complex *&)dataPtr_p,deleteThem(DATAPTR));
    uvScale.freeStorage((const Double*&) uvScale_p,deleteThem(UVSCALE));
    vb.frequency().freeStorage((const Double*&)vb_freq_p,deleteThem(VBFREQ));
    convSupport.freeStorage((const Int*&)convSupport_p,deleteThem(CONVSUPPORT));
    convFunc.freeStorage((const Complex *&)f_convFunc_p,deleteThem(CONVFUNC));
    chanMap.freeStorage((const Int*&)chanMap_p,deleteThem(CHANMAP));
    polMap.freeStorage((const Int*&) polMap_p,deleteThem(POLMAP));
    vb.antenna1().freeStorage((const Int*&) vb_ant1_p,deleteThem(VBANT1));
    vb.antenna2().freeStorage((const Int*&) vb_ant2_p,deleteThem(VBANT2));
  }
  //
  //----------------------------------------------------------------------
  //
  void PBWProjectFT::runFortranPut(Matrix<Double>& uvw,Vector<Double>& dphase,
				   const Complex& visdata,
				   IPosition& s,
				   //				Cube<Complex>& gradVisAzData,
				   //				Cube<Complex>& gradVisElData,
				   //				IPosition& gradS,
				   Int& Conj,
				   Cube<Int>& flags,Vector<Int>& rowFlags,
				   const Matrix<Float>& weight,
				   Int& rownr,Vector<Double>& actualOffset,
				   Array<Complex>& dataPtr,
				   Int& aNx, Int& aNy, Int& npol, Int& nchan,
				   const VisBuffer& vb,Int& Nant_p, Int& scanNo,
				   Double& sigma,
				   Array<Float>& l_off,
				   Array<Float>& m_off,
				   Matrix<Double>& sumWeight,
				   Double& area,
				   Int& doGrad,
				   Int& doPSF,
				   Int paIndex)
  {
    enum whichGetStorage {RAOFF,DECOFF,UVW,DPHASE,VISDATA,GRADVISAZ,GRADVISEL,
			  FLAGS,ROWFLAGS,UVSCALE,ACTUALOFFSET,DATAPTR,VBFREQ,
			  CONVSUPPORT,CONVFUNC,CHANMAP,POLMAP,VBANT1,VBANT2,WEIGHT,
			  SUMWEIGHT,CONJCFMAP,CFMAP};
    Vector<Bool> deleteThem(23);
    
    Double *uvw_p, *dphase_p, *actualOffset_p, *vb_freq_p, *uvScale_p;
    Complex *dataPtr_p, *f_convFunc_p;
    //  Complex *gradVisAzData_p, *gradVisElData_p;
    Int *flags_p, *rowFlags_p, *chanMap_p, *polMap_p, *convSupport_p, *vb_ant1_p, *vb_ant2_p,
      *ConjCFMap_p, *CFMap_p;
    Float *l_off_p, *m_off_p;
    Float *weight_p;Double *sumwt_p;
    Double actualPA;
    const Complex *visdata_p=&visdata;
    
    Vector<Int> ConjCFMap, CFMap;
    actualPA = getPA(vb);
    ConjCFMap = polMap;
    //    cout << actualPA*57.295 - currentCFPA*57.295 << endl;
    /*
      Int N;
      N=polMap.nelements();
      for(Int i=0;i<N;i++) CFMap[i] = polMap[N-i-1];
    */
    CFMap = polMap; ConjCFMap = polMap;
    CFMap = makeConjPolMap(vb);
    
    ConjCFMap_p     = ConjCFMap.getStorage(deleteThem(CONJCFMAP));
    CFMap_p         = CFMap.getStorage(deleteThem(CFMAP));
    
    uvw_p           = uvw.getStorage(deleteThem(UVW));
    dphase_p        = dphase.getStorage(deleteThem(DPHASE));
    //  visdata_p       = visdata.getStorage(deleteThem(VISDATA));
    //  gradVisAzData_p = gradVisAzData.getStorage(deleteThem(GRADVISAZ));
    //  gradVisElData_p = gradVisElData.getStorage(deleteThem(GRADVISEL));
    flags_p         = flags.getStorage(deleteThem(FLAGS));
    rowFlags_p      = rowFlags.getStorage(deleteThem(ROWFLAGS));
    uvScale_p       = uvScale.getStorage(deleteThem(UVSCALE));
    actualOffset_p  = actualOffset.getStorage(deleteThem(ACTUALOFFSET));
    dataPtr_p       = dataPtr.getStorage(deleteThem(DATAPTR));
    vb_freq_p       = (Double *)(vb.frequency().getStorage(deleteThem(VBFREQ)));
    convSupport_p   = convSupport.getStorage(deleteThem(CONVSUPPORT));
    f_convFunc_p      = convFunc.getStorage(deleteThem(CONVFUNC));
    chanMap_p       = chanMap.getStorage(deleteThem(CHANMAP));
    polMap_p        = polMap.getStorage(deleteThem(POLMAP));
    vb_ant1_p       = (Int *)(vb.antenna1().getStorage(deleteThem(VBANT1)));
    vb_ant2_p       = (Int *)(vb.antenna2().getStorage(deleteThem(VBANT2)));
    l_off_p     = l_off.getStorage(deleteThem(RAOFF));
    m_off_p    = m_off.getStorage(deleteThem(DECOFF));
    weight_p        = (Float *)(weight.getStorage(deleteThem(WEIGHT)));
    sumwt_p         = sumWeight.getStorage(deleteThem(SUMWEIGHT));
    
    
    Int npa=convSupport.shape()(2),actualConvSize;
    Int paIndex_Fortran = paIndex; 
    actualConvSize = convFunc.shape()(0);
    
    IPosition shp=convSupport.shape();
    
    gpbwproj(uvw_p,
	     dphase_p,
	     //		  vb.modelVisCube().getStorage(del),
	     visdata_p,
	     &s(0),
	     &s(1),
	     //	   gradVisAzData_p,
	     //	   gradVisElData_p,
	     //	    &gradS(0),
	     //	    &gradS(1),
	     //	   &Conj,
	     &doPSF,
	     flags_p,
	     rowFlags_p,
	     weight_p,
	     &s(2),
	     &rownr,
	     uvScale_p,
	     actualOffset_p,
	     dataPtr_p,
	     &aNx,
	     &aNy,
	     &npol,
	     &nchan,
	     vb_freq_p,
	     &C::c,
	     convSupport_p,
	     &actualConvSize,
	     &convSampling,
	     &wConvSize,
	     f_convFunc_p,
	     chanMap_p,
	     polMap_p,
	     &polInUse,
	     sumwt_p,
	     vb_ant1_p,
	     vb_ant2_p,
	     &Nant_p,
	     &scanNo,
	     &sigma,
	     l_off_p, m_off_p,
	     &area,
	     &doGrad,
	     &doPointing,
	     &npa,
	     &paIndex_Fortran,
	     CFMap_p,
	     ConjCFMap_p,
	     &currentCFPA
	     ,&actualPA
	     );
    
    ConjCFMap.freeStorage((const Int *&)ConjCFMap_p,deleteThem(CONJCFMAP));
    CFMap.freeStorage((const Int *&)CFMap_p,deleteThem(CFMAP));
    
    l_off.freeStorage((const Float*&)l_off_p,deleteThem(RAOFF));
    m_off.freeStorage((const Float*&)m_off_p,deleteThem(DECOFF));
    uvw.freeStorage((const Double*&)uvw_p,deleteThem(UVW));
    dphase.freeStorage((const Double*&)dphase_p,deleteThem(DPHASE));
    //  visdata.putStorage(visdata_p,deleteThem(VISDATA));
    //  gradVisAzData.putStorage(gradVisAzData_p,deleteThem(GRADVISAZ));
    //  gradVisElData.putStorage(gradVisElData_p,deleteThem(GRADVISEL));
    flags.freeStorage((const Int*&) flags_p,deleteThem(FLAGS));
    rowFlags.freeStorage((const Int *&)rowFlags_p,deleteThem(ROWFLAGS));
    actualOffset.freeStorage((const Double*&)actualOffset_p,deleteThem(ACTUALOFFSET));
    dataPtr.freeStorage((const Complex *&)dataPtr_p,deleteThem(DATAPTR));
    uvScale.freeStorage((const Double*&) uvScale_p,deleteThem(UVSCALE));
    vb.frequency().freeStorage((const Double*&)vb_freq_p,deleteThem(VBFREQ));
    convSupport.freeStorage((const Int*&)convSupport_p,deleteThem(CONVSUPPORT));
    convFunc.freeStorage((const Complex *&)f_convFunc_p,deleteThem(CONVFUNC));
    chanMap.freeStorage((const Int*&)chanMap_p,deleteThem(CHANMAP));
    polMap.freeStorage((const Int*&) polMap_p,deleteThem(POLMAP));
    vb.antenna1().freeStorage((const Int*&) vb_ant1_p,deleteThem(VBANT1));
    vb.antenna2().freeStorage((const Int*&) vb_ant2_p,deleteThem(VBANT2));
    weight.freeStorage((const Float*&)weight_p,deleteThem(WEIGHT));
    sumWeight.putStorage(sumwt_p,deleteThem(SUMWEIGHT));
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::put(const VisBuffer& vb, Int row, Bool dopsf,
			 FTMachine::Type type)
  {
    // Take care of translation of Bools to Integer
    Int idopsf=0;
    makingPSF=dopsf;
    if(dopsf) idopsf=1;
    
    findConvFunction(*image, vb);
    
    const Cube<Complex> *data;
    if(type==FTMachine::MODEL)
      data=&(vb.modelVisCube());
    else if(type==FTMachine::CORRECTED)
      data=&(vb.correctedVisCube());
    else
      data=&(vb.visCube());
    
    Bool isCopy;
    const casa::Complex *datStorage=data->getStorage(isCopy);
    //  Array<Float> l_offsets,m_offsets;
    Int NAnt = 0;
    if (doPointing) NAnt = findPointingOffsets(vb,l_offsets,m_offsets,True);
    
    //
    // If row is -1 then we pass through all rows
    //
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
      } 
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
      }
    //    
    // Get the uvws in a form that Fortran can use and do that
    // necessary phase rotation. On a Pentium Pro 200 MHz when null,
    // this step takes about 50us per uvw point. This is just barely
    // noticeable for Stokes I continuum and irrelevant for other
    // cases.
    //
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //NEGATING to correct for an image inversion problem
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    dphase*=-1.0;
    
    Cube<Int> flags(vb.flagCube().shape());
    flags=0;
    flags(vb.flagCube())=True;
    
    Vector<Int> rowFlags(vb.nRow());
    rowFlags=0;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);  
    
    //Here we redo the match or use previous match
    
    //Channel matching for the actual spectral window of buffer
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }
    
    if(isTiled) 
      {// Tiled Version
	Double invLambdaC=vb.frequency()(0)/C::c;
	Vector<Double> uvLambda(2);
	Vector<Int> centerLoc2D(2);
	centerLoc2D=0;
	//
	// Loop over all rows
	//
	for (Int rownr=startRow; rownr<=endRow; rownr++) 
	  {
	    // Calculate uvw for this row at the center frequency
	    uvLambda(0)=uvw(0,rownr)*invLambdaC;
	    uvLambda(1)=uvw(1,rownr)*invLambdaC;
	    centerLoc2D=gridder->location(centerLoc2D, uvLambda);
	    //
	    // Is this point on the grid?
	    //
	    if(gridder->onGrid(centerLoc2D)) 
	      {
		// Get the tile
		Array<Complex>* dataPtr=getDataPointer(centerLoc2D, False);
		Int aNx=dataPtr->shape()(0);
		Int aNy=dataPtr->shape()(1);
		//
		// Now use FORTRAN to do the gridding. Remember to 
		// ensure that the shape and offsets of the tile are 
		// accounted for.
		//
		Vector<Double> actualOffset(3);
		for (Int i=0;i<2;i++) actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
		
		actualOffset(2)=uvOffset(2);
		IPosition s(flags.shape());
		//
		// Now pass all the information down to a FORTRAN routine to
		// do the work
		//
		Int Conj=0,doPSF;
		Int ScanNo=0,doGrad=0;
		Double area=1.0;
		
		Int tmpPAI=PAIndex+1;
		if (dopsf) doPSF=1; else doPSF=0;
		runFortranPut(uvw,dphase,*datStorage,s,Conj,flags,rowFlags,
			      vb.imagingWeight(),rownr,actualOffset,
			      *dataPtr,aNx,aNy,npol,nchan,vb,NAnt,ScanNo,sigma,
			      l_offsets,m_offsets,sumWeight,area,doGrad,doPSF,tmpPAI);
	      }
	  }
      }
    else 
      {//Non-tiled version
	IPosition s(flags.shape());
	
	Int Conj=0,doPSF=0;
	Int ScanNo=0,doGrad=0;Double area=1.0;
	
	if (dopsf) doPSF=1;
	
	Int tmpPAI=PAIndex+1;
	runFortranPut(uvw,dphase,*datStorage,s,Conj,flags,rowFlags,
		      vb.imagingWeight(),
		      row,uvOffset,
		      griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
		      l_offsets,m_offsets,sumWeight,area,doGrad,doPSF,tmpPAI);
      }
    
    data->freeStorage(datStorage, isCopy);
  }
  //
  //----------------------------------------------------------------------
  //
  void PBWProjectFT::initVisBuffer(VisBuffer& vb, Type whichVBColumn)
  {
    if (whichVBColumn      == FTMachine::MODEL)    vb.modelVisCube()=Complex(0.0,0.0);
    else if (whichVBColumn == FTMachine::OBSERVED) vb.visCube()=Complex(0.0,0.0);
  }
  //
  //----------------------------------------------------------------------
  //
  void PBWProjectFT::initVisBuffer(VisBuffer& vb, Type whichVBColumn, Int row)
  {
    if (whichVBColumn == FTMachine::MODEL)
      vb.modelVisCube().xyPlane(row)=Complex(0.0,0.0);
    else if (whichVBColumn == FTMachine::OBSERVED)
      vb.visCube().xyPlane(row)=Complex(0.0,0.0);
  }
  //
  //---------------------------------------------------------------
  //
  // Predict the coherences as well as their derivatives w.r.t. the
  // pointing offsets.
  //
  void PBWProjectFT::get(VisBuffer& vb,       
			 VisBuffer& gradVBAz,
			 VisBuffer& gradVBEl,
			 Cube<Float>& pointingOffsets,
			 Int row,  // default row=-1 
			 Type whichVBColumn, // default whichVBColumn = FTMachine::MODEL
			 Type whichGradVBColumn,// default whichGradVBColumn = FTMachine::MODEL
			 Int Conj, Int doGrad) // default Conj=0, doGrad=1
  {
    // If row is -1 then we pass through all rows
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
	initVisBuffer(vb,whichVBColumn);
	if (doGrad)
	  {
	    initVisBuffer(gradVBAz, whichGradVBColumn);
	    initVisBuffer(gradVBEl, whichGradVBColumn);
	  }
      }
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
	initVisBuffer(vb,whichVBColumn,row);
	if (doGrad)
	  {
	    initVisBuffer(gradVBAz, whichGradVBColumn,row);
	    initVisBuffer(gradVBEl, whichGradVBColumn,row);
	  }
      }
    
    findConvFunction(*image, vb);

    //    cout << "PBWP::get: Current PA = " << currentCFPA << endl;
    //  Array<Float> l_offsets,m_offsets;
    Int NAnt=0;
    if (doPointing)   NAnt = findPointingOffsets(vb,l_offsets,m_offsets,False);
    //    NAnt=pointingOffsets.shape()(2)-1;
    //    l_offsets  = pointingOffsets(IPosition(3,0,0,0),IPosition(3,0,0,NAnt));
    //    m_offsets = pointingOffsets(IPosition(3,1,0,0),IPosition(3,1,0,NAnt));
//      cout << "PBW:Pointingoffsets:: " << l_offsets << endl
//  	 << m_offsets << endl;

    //    cout << "PBW::get(): pointingOffsets.shape() = " << pointingOffsets.shape() << endl;
    // Get the uvws in a form that Fortran can use
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //NEGATING to correct for an image inversion problem
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    dphase*=-1.0;
    
    
    Cube<Int> flags(vb.flagCube().shape());
    flags=0;
    flags(vb.flagCube())=True;
    
    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);
    
    //Here we redo the match or use previous match
    //
    //Channel matching for the actual spectral window of buffer
    //
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }
    
    Vector<Int> rowFlags(vb.nRow());
    rowFlags=0;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    
    Bool del;
    IPosition s,gradS;
    Cube<Complex> visdata,gradVisAzData,gradVisElData;
    if (whichVBColumn == FTMachine::MODEL) 
      {
	s = vb.modelVisCube().shape();
	visdata.reference(vb.modelVisCube());
      }
    else if (whichVBColumn == FTMachine::OBSERVED)  
      {
	s = vb.visCube().shape();
	visdata.reference(vb.visCube());
      }
    
    if (doGrad)
      {
	if (whichGradVBColumn == FTMachine::MODEL) 
	  {
	    //	    gradS = gradVBAz.modelVisCube().shape();
	    gradVisAzData.reference(gradVBAz.modelVisCube());
	    gradVisElData.reference(gradVBEl.modelVisCube());
	  }
	else if (whichGradVBColumn == FTMachine::OBSERVED)  
	  {
	    //	    gradS = gradVBAz.visCube().shape();
	    gradVisAzData.reference(gradVBAz.visCube());
	    gradVisElData.reference(gradVBEl.visCube());
	  }
      }
    
    if(isTiled) 
      {
	Double invLambdaC=vb.frequency()(0)/C::c;
	Vector<Double> uvLambda(2);
	Vector<Int> centerLoc2D(2);
	centerLoc2D=0;
	
	// Loop over all rows
	for (Int rownr=startRow; rownr<=endRow; rownr++) 
	  {
	    
	    // Calculate uvw for this row at the center frequency
	    uvLambda(0)=uvw(0, rownr)*invLambdaC;
	    uvLambda(1)=uvw(1, rownr)*invLambdaC;
	    centerLoc2D=gridder->location(centerLoc2D, uvLambda);
	    
	    // Is this point on the grid?
	    if(gridder->onGrid(centerLoc2D)) 
	      {
		
		// Get the tile
		Array<Complex>* dataPtr=getDataPointer(centerLoc2D, True);
		gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
		Int aNx=dataPtr->shape()(0);
		Int aNy=dataPtr->shape()(1);
		
		// Now use FORTRAN to do the gridding. Remember to 
		// ensure that the shape and offsets of the tile are 
		// accounted for.
		
		Vector<Double> actualOffset(3);
		for (Int i=0;i<2;i++) 
		  actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
		
		actualOffset(2)=uvOffset(2);
		IPosition s(vb.modelVisCube().shape());
		
		Int ScanNo=0, tmpPAI;
		Double area=1.0;
		tmpPAI = PAIndex+1;
		runFortranGetGrad(uvw,dphase,visdata,s,
				  gradVisAzData,gradVisElData,
				  Conj,flags,rowFlags,rownr,
				  actualOffset,dataPtr,aNx,aNy,npol,nchan,vb,NAnt,ScanNo,sigma,
				  l_offsets,m_offsets,area,doGrad,tmpPAI);
	      }
	  }
      }
    else 
      {
	
	IPosition s(vb.modelVisCube().shape());
	Int ScanNo=0, tmpPAI;
	Double area=1.0;
	tmpPAI = PAIndex+1;
	runFortranGetGrad(uvw,dphase,visdata/*vb.modelVisCube()*/,s,
			  gradVisAzData, gradVisElData,
			  Conj,flags,rowFlags,row,
			  uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
			  l_offsets,m_offsets,area,doGrad,tmpPAI);
	/*
	runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,row,
		      uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
		      l_offsets,m_offsets,area,doGrad,tmpPAI);
	*/
      }
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::get(VisBuffer& vb, Int row)
  {
    // If row is -1 then we pass through all rows
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
	vb.modelVisCube()=Complex(0.0,0.0);
      }
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
	vb.modelVisCube().xyPlane(row)=Complex(0.0,0.0);
      }
    
    findConvFunction(*image, vb);
    
    //  Array<Float> l_offsets,m_offsets;
    Int NAnt=0;
    if (doPointing)   NAnt = findPointingOffsets(vb,l_offsets,m_offsets,True);
    
    // Get the uvws in a form that Fortran can use
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //NEGATING to correct for an image inversion problem
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    dphase*=-1.0;
    
    
    Cube<Int> flags(vb.flagCube().shape());
    flags=0;
    flags(vb.flagCube())=True;
    
    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);
    
    //Here we redo the match or use previous match
    //
    //Channel matching for the actual spectral window of buffer
    //
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }
    
    Vector<Int> rowFlags(vb.nRow());
    rowFlags=0;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    
    if(isTiled) 
      {
	
	Double invLambdaC=vb.frequency()(0)/C::c;
	Vector<Double> uvLambda(2);
	Vector<Int> centerLoc2D(2);
	centerLoc2D=0;
	
	// Loop over all rows
	for (Int rownr=startRow; rownr<=endRow; rownr++) 
	  {
	    
	    // Calculate uvw for this row at the center frequency
	    uvLambda(0)=uvw(0, rownr)*invLambdaC;
	    uvLambda(1)=uvw(1, rownr)*invLambdaC;
	    centerLoc2D=gridder->location(centerLoc2D, uvLambda);
	    
	    // Is this point on the grid?
	    if(gridder->onGrid(centerLoc2D)) 
	      {
		
		// Get the tile
		Array<Complex>* dataPtr=getDataPointer(centerLoc2D, True);
		gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
		Int aNx=dataPtr->shape()(0);
		Int aNy=dataPtr->shape()(1);
		
		// Now use FORTRAN to do the gridding. Remember to 
		// ensure that the shape and offsets of the tile are 
		// accounted for.
		
		Vector<Double> actualOffset(3);
		for (Int i=0;i<2;i++) 
		  actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
		
		actualOffset(2)=uvOffset(2);
		IPosition s(vb.modelVisCube().shape());
		
		Int Conj=0,doGrad=0,ScanNo=0;
		Double area=1.0;
		
		runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,rownr,
			      actualOffset,dataPtr,aNx,aNy,npol,nchan,vb,NAnt,ScanNo,sigma,
			      l_offsets,m_offsets,area,doGrad,PAIndex+1);
	      }
	  }
      }
    else 
      {
	
	IPosition s(vb.modelVisCube().shape());
	Int Conj=0,doGrad=0,ScanNo=0;
	Double area=1.0;
	
	runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,row,
		      uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
		      l_offsets,m_offsets,area,doGrad,PAIndex+1);
      }
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::get(VisBuffer& vb, Cube<Complex>& modelVis, 
			 Array<Complex>& griddedVis, Vector<Double>& scale,
			 Int row)
  {
    Int nX=griddedVis.shape()(0);
    Int nY=griddedVis.shape()(1);
    Vector<Double> offset(2);
    offset(0)=Double(nX)/2.0;
    offset(1)=Double(nY)/2.0;
    // If row is -1 then we pass through all rows
    Int startRow, endRow, nRow;
    if (row==-1) 
      {
	nRow=vb.nRow();
	startRow=0;
	endRow=nRow-1;
	modelVis.set(Complex(0.0,0.0));
      } 
    else 
      {
	nRow=1;
	startRow=row;
	endRow=row;
	modelVis.xyPlane(row)=Complex(0.0,0.0);
      }
    
    //  Array<Float> l_offsets,m_offsets;
    Int NAnt=0;
    
    if (doPointing) 
      //    NAnt = findPointingOffsets(vb,l_offsets, m_offsets,False);
      NAnt = findPointingOffsets(vb,l_offsets, m_offsets,True);
    
    
    //  
    // Get the uvws in a form that Fortran can use
    //
    Matrix<Double> uvw(3, vb.uvw().nelements());
    uvw=0.0;
    Vector<Double> dphase(vb.uvw().nelements());
    dphase=0.0;
    //
    //NEGATING to correct for an image inversion problem
    //
    for (Int i=startRow;i<=endRow;i++) 
      {
	for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
	uvw(2,i)=vb.uvw()(i)(2);
      }
    
    rotateUVW(uvw, dphase, vb);
    refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
    
    // This is the convention for dphase
    dphase*=-1.0;
    
    Cube<Int> flags(vb.flagCube().shape());
    flags=0;
    flags(vb.flagCube())=True;
    
    //Check if ms has changed then cache new spw and chan selection
    if(vb.newMS())
      matchAllSpwChans(vb);
    
    //Channel matching for the actual spectral window of buffer
    if(doConversion_p[vb.spectralWindow()])
      matchChannel(vb.spectralWindow(), vb);
    else
      {
	chanMap.resize();
	chanMap=multiChanMap_p[vb.spectralWindow()];
      }
    
    Vector<Int> rowFlags(vb.nRow());
    rowFlags=0;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    
    IPosition s(modelVis.shape());
    Int Conj=0,doGrad=0,ScanNo=0;
    Double area=1.0;
    
    runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,row,
		  offset,&griddedVis,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
		  l_offsets,m_offsets,area,doGrad,PAIndex+1);
  }
  //
  //---------------------------------------------------------------
  //
  // Finalize the FFT to the Sky. Here we actually do the FFT and
  // return the resulting image
  ImageInterface<Complex>& PBWProjectFT::getImage(Matrix<Float>& weights,
						  Bool normalize) 
  {
    AlwaysAssert(lattice, AipsError);
    AlwaysAssert(image, AipsError);
    
    logIO() << "#########getimage########" << LogIO::DEBUGGING << LogIO::POST;
    
    logIO() << LogOrigin("PBWProjectFT", "getImage") << LogIO::NORMAL;
    
    weights.resize(sumWeight.shape());
    
    convertArray(weights, sumWeight);
    //  
    // If the weights are all zero then we cannot normalize otherwise
    // we don't care.
    //
    if(max(weights)==0.0) 
      {
	if(normalize) logIO() << LogIO::SEVERE
			      << "No useful data in PBWProjectFT: weights all zero"
			      << LogIO::POST;
	else logIO() << LogIO::WARN << "No useful data in PBWProjectFT: weights all zero"
		     << LogIO::POST;
      }
    else
      {
	const IPosition latticeShape = lattice->shape();
	
	logIO() << LogIO::DEBUGGING
		<< "Starting FFT and scaling of image" << LogIO::POST;
	//    
	// x and y transforms (lattice has the gridded vis.  Make the
	// dirty images)
	//
	LatticeFFT::cfft2d(*lattice,False);
	
	//
	// Apply the gridding correction
	//    
	//      if (!makingPSF)
	{
	  normalizeAvgPB();
	  Int inx = lattice->shape()(0);
	  Int iny = lattice->shape()(1);
	  Vector<Complex> correction(inx);
	  //	correction=Complex(1.0, 0.0);
	  
	  Vector<Float> sincConv(nx);
	  Float centerX=nx/2+1;
	  for (Int ix=0;ix<nx;ix++) 
	    {
	      Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
	      //	    Float x=C::pi*Float(ix-centerX)*Float(convSupport(0))/Float(nx);
	      if(ix==centerX) sincConv(ix)=1.0;
	      else 	    sincConv(ix)=sin(x)/x;
	    }
	  
	  IPosition cursorShape(4, inx, 1, 1, 1);
	  IPosition axisPath(4, 0, 1, 2, 3);
	  LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
	  LatticeIterator<Complex> lix(*lattice, lsx);
	  
	  LatticeStepper lavgpb(avgPB.shape(),cursorShape,axisPath);
	  LatticeIterator<Float> liavgpb(avgPB, lavgpb);
	  
	  for(lix.reset(),liavgpb.reset();
	      !lix.atEnd();
	      lix++,liavgpb++) 
	    {
	      Int pol=lix.position()(2);
	      Int chan=lix.position()(3);
	      //	    Int polnPlane = liavgpb.position()(2);
	      
	      if(weights(pol, chan)>0.0) 
		{
		  Int iy=lix.position()(1);
		  gridder->correctX1D(correction,iy);
		  
		  // 		Vector<Float> PBCorrection(lipb.rwVectorCursor().shape()),
		  // 		  avgPB(liavgpb.rwVectorCursor().shape());
		  Vector<Float> PBCorrection(liavgpb.rwVectorCursor().shape()),
		    avgPBVec(liavgpb.rwVectorCursor().shape());
		  
		  PBCorrection = liavgpb.rwVectorCursor();
		  avgPBVec = liavgpb.rwVectorCursor();
		  
		  for(int i=0;i<PBCorrection.shape();i++)
		    {
		      PBCorrection(i)=FUNC(avgPBVec(i))*sincConv(i)*sincConv(iy);
		      if ((abs(PBCorrection(i)*correction(i))) >= pbLimit_p)
			lix.rwVectorCursor()(i) /= PBCorrection(i)*correction(i);
		      else if (!makingPSF)
			//		      lix.rwVectorCursor()(i) /= correction(i)*sincConv(i)*sincConv(iy);
			lix.rwVectorCursor()(i) /= (sincConv(i)*sincConv(iy));
		    }

		  if(normalize) 
		    {
		      Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
		      lix.rwCursor()*=rnorm;
		    }
		  else 
		    {
		      Complex rnorm(Float(inx)*Float(iny));
		      lix.rwCursor()*=rnorm;
		    }
		}
	      else 
		lix.woCursor()=0.0;
	    }
	}
	
	/*
	  {
	  // Write the residual image to the disk for debugging
	  //
	  // Check the section from the image BEFORE converting to a lattice 
	  //
	  IPosition blc(4, (nx-image->shape()(0))/2,
	  (ny-image->shape()(1))/2, 0, 0);
	  IPosition stride(4, 1);
	  IPosition trc(blc+image->shape()-stride);
	  //
	  // Do the copy
	  //
	  image->put(griddedData(blc, trc));
	  ostringstream name;
	  name << "thePolnImage.im";
	  PagedImage<Float> tmp((*image).shape(), (*image).coordinates(), name);
	  LatticeExpr<Float> le(abs((*image)));
	  tmp.copyData(le);
	  }
	*/
	if(!isTiled) 
	  {
	    //
	    // Check the section from the image BEFORE converting to a lattice 
	    //
	    IPosition blc(4, (nx-image->shape()(0))/2,
			  (ny-image->shape()(1))/2, 0, 0);
	    IPosition stride(4, 1);
	    IPosition trc(blc+image->shape()-stride);
	    //
	    // Do the copy
	    //
	    image->put(griddedData(blc, trc));
	    if(arrayLattice) delete arrayLattice; arrayLattice=0;
	    griddedData.resize(IPosition(1,0));
	  }
      }
    
    return *image;
  }
  //
  //---------------------------------------------------------------
  //
  // Get weight image
  void PBWProjectFT::getWeightImage(ImageInterface<Float>& weightImage,
				    Matrix<Float>& weights) 
  {
    
    logIO() << LogOrigin("PBWProjectFT", "getWeightImage") << LogIO::NORMAL;
    
    weights.resize(sumWeight.shape());
    convertArray(weights,sumWeight);
    
    const IPosition latticeShape = weightImage.shape();
    
    Int nx=latticeShape(0);
    Int ny=latticeShape(1);
    
    IPosition loc(2, 0);
    IPosition cursorShape(4, nx, ny, 1, 1);
    IPosition axisPath(4, 0, 1, 2, 3);
    LatticeStepper lsx(latticeShape, cursorShape, axisPath);
    LatticeIterator<Float> lix(weightImage, lsx);
    for(lix.reset();!lix.atEnd();lix++) 
      {
	Int pol=lix.position()(2);
	Int chan=lix.position()(3);
	lix.rwCursor()=weights(pol,chan);
      }
  }
  //
  //---------------------------------------------------------------
  //
  Bool PBWProjectFT::toRecord(String& error, RecordInterface& outRec, 
			      Bool withImage) 
  {
    
    // Save the current PBWProjectFT object to an output state record
    Bool retval = True;
    Double cacheVal=(Double) cachesize;
    outRec.define("cache", cacheVal);
    outRec.define("tile", tilesize);
    
    Vector<Double> phaseValue(2);
    String phaseUnit;
    phaseValue=mTangent_p.getAngle().getValue();
    phaseUnit= mTangent_p.getAngle().getUnit();
    outRec.define("phasevalue", phaseValue);
    outRec.define("phaseunit", phaseUnit);
    
    Vector<Double> dirValue(3);
    String dirUnit;
    dirValue=mLocation_p.get("m").getValue();
    dirUnit=mLocation_p.get("m").getUnit();
    outRec.define("dirvalue", dirValue);
    outRec.define("dirunit", dirUnit);
    
    outRec.define("padding", padding_p);
    outRec.define("maxdataval", maxAbsData);
    
    Vector<Int> center_loc(4), offset_loc(4);
    for (Int k=0; k<4 ; k++)
      {
	center_loc(k)=centerLoc(k);
	offset_loc(k)=offsetLoc(k);
      }
    outRec.define("centerloc", center_loc);
    outRec.define("offsetloc", offset_loc);
    outRec.define("sumofweights", sumWeight);
    if(withImage && image)
      { 
	ImageInterface<Complex>& tempimage(*image);
	Record imageContainer;
	String error;
	retval = (retval || tempimage.toRecord(error, imageContainer));
	outRec.defineRecord("image", imageContainer);
      }
    return retval;
  }
  //
  //---------------------------------------------------------------
  //
  Bool PBWProjectFT::fromRecord(String& error, const RecordInterface& inRec)
  {
    Bool retval = True;
    imageCache=0; lattice=0; arrayLattice=0;
    Double cacheVal;
    inRec.get("cache", cacheVal);
    cachesize=(Long) cacheVal;
    inRec.get("tile", tilesize);
    
    Vector<Double> phaseValue(2);
    inRec.get("phasevalue",phaseValue);
    String phaseUnit;
    inRec.get("phaseunit",phaseUnit);
    Quantity val1(phaseValue(0), phaseUnit);
    Quantity val2(phaseValue(1), phaseUnit); 
    MDirection phasecenter(val1, val2);
    
    mTangent_p=phasecenter;
    // This should be passed down too but the tangent plane is 
    // expected to be specified in all meaningful cases.
    tangentSpecified_p=True;  
    Vector<Double> dirValue(3);
    String dirUnit;
    inRec.get("dirvalue", dirValue);
    inRec.get("dirunit", dirUnit);
    MVPosition dummyMVPos(dirValue(0), dirValue(1), dirValue(2));
    MPosition mLocation(dummyMVPos, MPosition::ITRF);
    mLocation_p=mLocation;
    
    inRec.get("padding", padding_p);
    inRec.get("maxdataval", maxAbsData);
    
    Vector<Int> center_loc(4), offset_loc(4);
    inRec.get("centerloc", center_loc);
    inRec.get("offsetloc", offset_loc);
    uInt ndim4 = 4;
    centerLoc=IPosition(ndim4, center_loc(0), center_loc(1), center_loc(2), 
			center_loc(3));
    offsetLoc=IPosition(ndim4, offset_loc(0), offset_loc(1), offset_loc(2), 
			offset_loc(3));
    inRec.get("sumofweights", sumWeight);
    if(inRec.nfields() > 12 )
      {
	Record imageAsRec=inRec.asRecord("image");
	if(!image) image= new TempImage<Complex>(); 
	
	String error;
	retval = (retval || image->fromRecord(error, imageAsRec));    
	
	// Might be changing the shape of sumWeight
	init(); 
	
	if(isTiled) 
	  lattice=image;
	else 
	  {
	    //
	    // Make the grid the correct shape and turn it into an
	    // array lattice Check the section from the image BEFORE
	    // converting to a lattice
	    //
	    IPosition gridShape(4, nx, ny, npol, nchan);
	    griddedData.resize(gridShape);
	    griddedData=Complex(0.0);
	    IPosition blc(4, (nx-image->shape()(0))/2,
			  (ny-image->shape()(1))/2, 0, 0);
	    IPosition start(4, 0);
	    IPosition stride(4, 1);
	    IPosition trc(blc+image->shape()-stride);
	    griddedData(blc, trc) = image->getSlice(start, image->shape());
	    
	    if(arrayLattice) delete arrayLattice; arrayLattice=0;
	    arrayLattice = new ArrayLattice<Complex>(griddedData);
	    lattice=arrayLattice;
	  }
	
	AlwaysAssert(lattice, AipsError);
	AlwaysAssert(image, AipsError);
      };
    return retval;
  }
  //
  //---------------------------------------------------------------
  //
  void PBWProjectFT::ok() 
  {
    AlwaysAssert(image, AipsError);
  }
  //----------------------------------------------------------------------
  //
  // Make a plain straightforward honest-to-god image. This returns a
  // complex image, without conversion to Stokes. The polarization
  // representation is that required for the visibilities.
  //
  void PBWProjectFT::makeImage(FTMachine::Type type, 
			       VisSet& vs,
			       ImageInterface<Complex>& theImage,
			       Matrix<Float>& weight) 
  {
    logIO() << LogOrigin("PBWProjectFT", "makeImage") << LogIO::NORMAL;
    
    if(type==FTMachine::COVERAGE) 
      logIO() << "Type COVERAGE not defined for Fourier transforms"
	      << LogIO::EXCEPTION;
    
    
    // Initialize the gradients
    ROVisIter& vi(vs.iter());
    
    // Loop over all visibilities and pixels
    VisBuffer vb(vi);
    
    // Initialize put (i.e. transform to Sky) for this model
    vi.origin();
    
    if(vb.polFrame()==MSIter::Linear) 
      StokesImageUtil::changeCStokesRep(theImage, SkyModel::LINEAR);
    else 
      StokesImageUtil::changeCStokesRep(theImage, SkyModel::CIRCULAR);
    
    initializeToSky(theImage,weight,vb);
    
    // Loop over the visibilities, putting VisBuffers
    //    vpSJ->reset();
    //    ofstream os("time.dat");
    paChangeDetector.reset();
    Int rowsDone=0;
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
      {
	for (vi.origin(); vi.more(); vi++) 
	  {
	    //      if (vpSJ->changed(vb,1)) break;
	    
	    if (type==FTMachine::PSF) makingPSF=True;
	    findConvFunction(theImage,vb);
	    
	    switch(type) 
	      {
	      case FTMachine::RESIDUAL:
		vb.visCube()=vb.correctedVisCube();
		vb.visCube()-=vb.modelVisCube();
		put(vb, -1, False);
		break;
	      case FTMachine::MODEL:
		vb.visCube()=vb.modelVisCube();
		put(vb, -1, False);
		break;
	      case FTMachine::CORRECTED:
		vb.visCube()=vb.correctedVisCube();
		put(vb, -1, False);
		break;
	      case FTMachine::PSF:
		vb.visCube()=Complex(1.0,0.0);
		makingPSF = True;
		put(vb, -1, True);
		break;
	      case FTMachine::OBSERVED:
	      default:
		put(vb, -1, False);
		break;
	      }
// 	    rowsDone += vi.nRow();
// 	    cout << "Rows done = " << rowsDone << " out of " << vi.nRowChunk() << " "
// 	       << vb.antenna1()(0) << " " << vb.antenna2()(0) 
// 	       << " " << getCurrentTimeStamp(vb)-(Double)4.51738e+09 << endl;
	  }
      }
    finalizeToSky();
    // Normalize by dividing out weights, etc.
    getImage(weight, True);
  }
  
  void PBWProjectFT::setPAIncrement(const Quantity& paIncrement)
  {
    paChangeDetector.setTolerance(paIncrement);
  }
} //# NAMESPACE CASA - END

