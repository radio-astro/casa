// -*- C++ -*-
//# AWProjectFT.cc: Implementation of AWProjectFT class
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
#include <ms/MeasurementSets/MSRange.h>
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>
#include <synthesis/MeasurementComponents/AWProjectFT.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageRegrid.h>
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

#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/IlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/ExpCache.h>
#include <synthesis/MeasurementComponents/CExp.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>
#include <scimath/Mathematics/MathFunc.h>

#include <casa/System/ProgressMeter.h>

#define CONVSIZE (1024*2)
#define CONVWTSIZEFACTOR 1.0
#define OVERSAMPLING 20
#define THRESHOLD 1E-4
#define USETABLES 0           // If equal to 1, use tabulated exp() and
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
#define nwcppeij nwcppeij_
#endif
    //
    //---------------------------------------------------------------
    //
    IlluminationConvFunc awEij;
    void awcppeij(Double *griduvw, Double *area,
		 Double *raoff1, Double *decoff1,
		 Double *raoff2, Double *decoff2, 
		 Int *doGrad,
		 Complex *weight,
		 Complex *dweight1,
		 Complex *dweight2,
		 Double *currentCFPA)
    {
      Complex w,d1,d2;
      awEij.getValue(griduvw, raoff1, raoff2, decoff1, decoff2,
		    area,doGrad,w,d1,d2,*currentCFPA);
      *weight   = w;
      *dweight1 = d1;
      *dweight2 = d2;
    }
  }
  //
  //---------------------------------------------------------------
  //
#define FUNC(a)  (((a)))
  AWProjectFT::AWProjectFT(Int nWPlanes, Long icachesize, 
			   String& cfCacheDirName,
			   Bool applyPointingOffset,
			   Bool doPBCorr,
			   Int itilesize, 
			   Float pbLimit,
			   Bool usezero)
    : FTMachine(), padding_p(1.0), nWPlanes_p(nWPlanes),
      imageCache(0), cachesize(icachesize), tilesize(itilesize),
      gridder(0), isTiled(False), arrayLattice(0), lattice(0), 
      maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
      mspc(0), msac(0), pointingToImage(0), usezero_p(usezero),
      doPBCorrection(doPBCorr),
      Second("s"),Radian("rad"),Day("d"), noOfPASteps(0),
      pbNormalized(False),resetPBs(True), rotateAperture_p(True),
      evlacf_p(), cfs_p(), cfwts_p(), cfCache(), paChangeDetector(), cfStokes(),Area(), 
      avgPBSaved(False),avgPBReady(False)
  {
    epJ=NULL;
    convSize=0;
    tangentSpecified_p=False;
    lastIndex_p=0;
    paChangeDetector.reset();
    pbLimit_p=pbLimit;
    //
    // Get various parameters from the visibilities.  
    //
    bandID_p=-1;
    if (applyPointingOffset) doPointing=1; else doPointing=0;

    convFuncCacheReady=False;
    maxConvSupport=-1;  
    //
    // Set up the Conv. Func. disk cache manager object.
    //
    cfCache.setCacheDir(cfCacheDirName.data());
    cfCache.initCache();
    convSampling=OVERSAMPLING;
    convSize=CONVSIZE;
    Long hostRAM = (HostInfo::memoryTotal(true)*1024); // In bytes
    hostRAM = hostRAM/(sizeof(Float)*2); // In complex pixels
    if (cachesize > hostRAM) cachesize=hostRAM;
    sigma=1.0;
  }
  //
  //---------------------------------------------------------------
  //
  AWProjectFT::AWProjectFT(const RecordInterface& stateRec)
    : FTMachine(),Second("s"),Radian("rad"),Day("d")
  {
    //
    // Construct from the input state record
    //
    String error;
    
    if (!fromRecord(stateRec)) {
      throw (AipsError("Failed to create AWProjectFT: " ));
    };
    bandID_p = -1;
    maxConvSupport=-1;
    convSampling=OVERSAMPLING;
    convSize=CONVSIZE;
  }
  //
  //----------------------------------------------------------------------
  //
  AWProjectFT::AWProjectFT(const AWProjectFT& other):FTMachine()
  {
    operator=(other);
  }
  //
  //---------------------------------------------------------------
  //
  AWProjectFT& AWProjectFT::operator=(const AWProjectFT& other)
  {
    if(this!=&other) 
      {
	//Do the base parameters
	FTMachine::operator=(other);

	
	padding_p=other.padding_p;
	
	nWPlanes_p=other.nWPlanes_p;
	imageCache=other.imageCache;
	cachesize=other.cachesize;
	tilesize=other.tilesize;
	cfRefFreq_p = other.cfRefFreq_p;
	if(other.gridder==0) gridder=0;
	else
	  {
	    uvScale.resize();
	    uvOffset.resize();
	    uvScale=other.uvScale;
	    uvOffset=other.uvOffset;
	    gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
							   uvScale, uvOffset,
							   "SF");
	  }

	isTiled=other.isTiled;
	lattice=0;
	arrayLattice=0;

	maxAbsData=other.maxAbsData;
	centerLoc=other.centerLoc;
	offsetLoc=other.offsetLoc;
	pointingToImage=other.pointingToImage;
	usezero_p=other.usezero_p;

	
	padding_p=other.padding_p;
	nWPlanes_p=other.nWPlanes_p;
	imageCache=other.imageCache;
	cachesize=other.cachesize;
	tilesize=other.tilesize;
	isTiled=other.isTiled;
	maxAbsData=other.maxAbsData;
	centerLoc=other.centerLoc;
	offsetLoc=other.offsetLoc;
	mspc=other.mspc;
	msac=other.msac;
	pointingToImage=other.pointingToImage;
	usezero_p=other.usezero_p;
	doPBCorrection = other.doPBCorrection;
	maxConvSupport= other.maxConvSupport;

	epJ=other.epJ;
	convSize=other.convSize;
	lastIndex_p=other.lastIndex_p;
	paChangeDetector=other.paChangeDetector;
	pbLimit_p=other.pbLimit_p;
	//
	// Get various parameters from the visibilities.  
	//
	bandID_p = other.bandID_p;
	doPointing=other.doPointing;

	convFuncCacheReady=other.convFuncCacheReady;
	maxConvSupport=other.maxConvSupport;
	//
	// Set up the Conv. Func. disk cache manager object.
	//
	cfCache=other.cfCache;
	convSampling=other.convSampling;
	convSize=other.convSize;
	cachesize=other.cachesize;
    
	resetPBs=other.resetPBs;
	pbNormalized=other.pbNormalized;
	currentCFPA=other.currentCFPA;
	lastPAUsedForWtImg = other.lastPAUsedForWtImg;
	cfStokes=other.cfStokes;
	Area=other.Area;
	avgPB = other.avgPB;
	avgPBReady = other.avgPBReady;
      };
    return *this;
  };
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::init() 
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
	if (sizeof(long) < 4)  // 32-bit machine
	  {
	    Int tmpCacheVal=static_cast<Int>(cachesize);
	    imageCache=new LatticeCache <Complex> (*image, tmpCacheVal, tileShape, 
						   tileOverlapVec,
						   (tileOverlap>0.0));
	  }
	else  // 64-bit machine
	  {
	    Long tmpCacheVal=cachesize;
	    imageCache=new LatticeCache <Complex> (*image, tmpCacheVal, tileShape, 
						   tileOverlapVec,
						   (tileOverlap>0.0));
	  }
      }
    
#if(USETABLES)
    Double StepSize;
    Int N=500000;
    StepSize = abs((((2*nx)/uvScale(0))/(sigma) + 
		    MAXPOINTINGERROR*1.745329E-02*(sigma)/3600.0))/N;
    if (!awEij.isReady())
      {
	logIO() << LogOrigin("AWProjectFT","init")
		<< "Making lookup table for exp function with a resolution of " 
		<< StepSize << " radians.  "
		<< "Memory used: " << sizeof(Float)*N/(1024.0*1024.0)<< " MB." 
		<< LogIO::NORMAL 
		<<LogIO::POST;
	
	awEij.setSigma(sigma);
	awEij.initExpTable(N,StepSize);
	//    ExpTab.build(N,StepSize);
	
	logIO() << LogOrigin("AWProjectFT","init")
		<< "Making lookup table for complex exp function with a resolution of " 
		<< 2*M_PI/N << " radians.  "
		<< "Memory used: " << 2*sizeof(Float)*N/(1024.0*1024.0) << " MB." 
		<< LogIO::NORMAL
		<< LogIO::POST;
	awEij.initCExpTable(N);
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
  AWProjectFT::~AWProjectFT() 
  {
      if(imageCache) delete imageCache; imageCache=0;
      if(gridder) delete gridder; gridder=0;
  }
  //
  //---------------------------------------------------------------
  //
  MDirection::Convert AWProjectFT::makeCoordinateMachine(const VisBuffer& vb,
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
    String ObsName=vb.msColumns().observation().telescopeName()(vb.arrayId());
    
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
  int AWProjectFT::findPointingOffsets(const VisBuffer& vb, 
					Array<Float> &l_off,
					Array<Float> &m_off,
					Bool Evaluate)
  {

    //    throw(AipsError("PBWProject::findPointingOffsets temporarily disabled. (gmoellen 06Nov10)"));

    Int NAnt = 0;
    MEpoch LAST;
    Double thisTime = getCurrentTimeStamp(vb);
    //    Array<Float> pointingOffsets = epJ->nearest(thisTime);
    if (epJ==NULL) return 0;
    Array<Float> pointingOffsets; epJ->nearest(thisTime,pointingOffsets);
    NAnt=pointingOffsets.shape()(2);
    l_off.resize(IPosition(3,1,1,NAnt)); // Poln x NChan x NAnt 
    m_off.resize(IPosition(3,1,1,NAnt)); // Poln x NChan x NAnt 
    // Can't figure out how to do the damn slicing of [Pol,NChan,NAnt,1] array
    // into [Pol,NChan,NAnt] array
    //
    //    l_off = pointingOffsets(Slicer(IPosition(4,0,0,0,0),IPosition(4,1,1,NAnt+1,0)));
    //    m_off = pointingOffsets(Slicer(IPosition(4,1,0,0,0),IPosition(4,1,1,NAnt+1,0)));
    IPosition tndx(3,0,0,0), sndx(4,0,0,0,0);
    for(tndx(2)=0;tndx(2)<NAnt; tndx(2)++,sndx(2)++)
      //    for(Int j=0;j<NAnt;j++)
      {
	// l_off(IPosition(3,0,0,j)) = pointingOffsets(IPosition(4,0,0,j,0));
	// m_off(IPosition(3,0,0,j)) = pointingOffsets(IPosition(4,1,0,j,0));

	sndx(0)=0; l_off(tndx) = pointingOffsets(sndx);
	sndx(0)=2; m_off(tndx) = pointingOffsets(sndx);
      }
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
    //
    // An array of shape [2,1,1]!
    //
    Array<Double> phaseDir = vb.msColumns().field().phaseDir().getColumn();
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
  int AWProjectFT::findPointingOffsets(const VisBuffer& vb, 
					 Cube<Float>& pointingOffsets,
					Array<Float> &l_off,
					Array<Float> &m_off,
					Bool Evaluate)
  {
    Int NAnt = 0;
    Float tmp;
    // TBD: adapt the following to VisCal mechanism:
    MEpoch LAST;
    
    NAnt=pointingOffsets.shape()(2);
    l_off.resize(IPosition(3,2,1,NAnt));
    m_off.resize(IPosition(3,2,1,NAnt));
    IPosition ndx(3,0,0,0),ndx1(3,0,0,0);
    for(ndx(2)=0;ndx(2)<NAnt;ndx(2)++)
      {
	ndx1=ndx;
	ndx(0)=0;ndx1(0)=0;	tmp=l_off(ndx)  = pointingOffsets(ndx1);//Axis_0,Pol_0,Ant_i
	ndx(0)=1;ndx1(0)=1;	tmp=l_off(ndx)  = pointingOffsets(ndx1);//Axis_0,Pol_1,Ant_i
	ndx(0)=0;ndx1(0)=2;	tmp=m_off(ndx)  = pointingOffsets(ndx1);//Axis_1,Pol_0,Ant_i
	ndx(0)=1;ndx1(0)=3;	tmp=m_off(ndx)  = pointingOffsets(ndx1);//Axis_1,Pol_1,Ant_i
      }

//     l_off  = pointingOffsets(IPosition(3,0,0,0),IPosition(3,0,0,NAnt));
//     m_off = pointingOffsets(IPosition(3,1,0,0),IPosition(3,1,0,NAnt));
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
    //
    // An array of shape [2,1,1]!
    //
    Array<Double> phaseDir = vb.msColumns().field().phaseDir().getColumn();
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
  void AWProjectFT::makeSensitivityImage(Lattice<Complex>& wtImage,
					   ImageInterface<Float>& sensitivityImage,
					   const Matrix<Float>& sumWt,
					   const Bool& doFFTNorm)
  {
    Bool doSumWtNorm=True;
    if (sumWt.shape().nelements()==0) doSumWtNorm=False;

    if ((sumWt.shape().nelements() < 2) || 
	(sumWt.shape()(0) != wtImage.shape()(2)) || 
	(sumWt.shape()(1) != wtImage.shape()(3)))
      throw(AipsError("makeSensitivityImage(): "
		      "Sum of weights per poln and chan required"));
    Float sumWtVal=1.0;

    LatticeFFT::cfft2d(wtImage,False);
    Int sizeX=wtImage.shape()(0), sizeY=wtImage.shape()(1);
    sensitivityImage.resize(wtImage.shape());
    Array<Float> senBuf;
    sensitivityImage.get(senBuf,False);
    ArrayLattice<Float> senLat(senBuf, True);

    //
    // Copy one 2D plane at a time, normalizing by the sum of weights
    // and possibly 2D FFT.
    //
    // Set up Lattice iteratos on wtImage and sensitivityImage
    //
    IPosition axisPath(4, 0, 1, 2, 3);
    IPosition cursorShape(4, sizeX, sizeY, 1, 1);
    LatticeStepper wtImStepper(wtImage.shape(), cursorShape, axisPath);
    LatticeIterator<Complex> wtImIter(wtImage, wtImStepper);
    LatticeStepper senImStepper(senLat.shape(), cursorShape, axisPath);
    LatticeIterator<Float> senImIter(senLat, senImStepper);
    //
    // Iterate over channel and polarization axis
    //
    if (!doFFTNorm) sizeX=sizeY=1;
    for(wtImIter.reset(),senImIter.reset();  !wtImIter.atEnd(); wtImIter++,senImIter++) 
      {
	Int pol=wtImIter.position()(2), chan=wtImIter.position()(3);
	if (doSumWtNorm) sumWtVal=sumWt(pol,chan);
	senImIter.rwCursor() = (real(wtImIter.rwCursor())
				*Float(sizeX)*Float(sizeY)/sumWtVal);
      }
    //
    // The following code is averaging RR and LL planes and writing
    // the result to back to both planes.  This needs to be
    // generalized for full-pol case.
    //
    IPosition start0(4,0,0,0,0), start1(4,0,0,1,0), length(4,sizeX,sizeY,1,1);
    Slicer slicePol0(start0,length), slicePol1(start1,length);
    Array<Float> polPlane0, polPlane1;
    senLat.getSlice(polPlane0,slicePol0);
    senLat.getSlice(polPlane1,slicePol1);
    polPlane0=(polPlane0+polPlane1)/2.0;
    polPlane1=polPlane0;
    // senLat.putSlice(polPlane0,IPosition(4,0,0,0,0));
    // senLat.putSlice(polPlane1,IPosition(4,0,0,1,0));
    // cerr << "Pol0: " << polPlane0.shape() << " " << max(polPlane0) << endl;
    // cerr << "Pol1: " << polPlane1.shape() << " " << max(polPlane1) << endl;
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::normalizeAvgPB()
  {
    if (!pbNormalized)
      {
	pbPeaks.resize(avgPB.shape()(2),True);
	if (makingPSF) pbPeaks = 1.0;
	else pbPeaks /= (Float)noOfPASteps;
	pbPeaks = 1.0;
	logIO() << LogOrigin("AWProjectFT", "normalizeAvgPB")  
		<< "Normalizing the average PBs to " << 1.0
		<< LogIO::NORMAL
		<< LogIO::POST;
	
	IPosition avgPBShape(avgPB.shape()),ndx(4,0,0,0,0);
	Vector<Float> peak(avgPBShape(2));
	
	Bool isRefF;
	Array<Float> avgPBBuf;
	isRefF=avgPB.get(avgPBBuf);
	
	Float pbMax = max(avgPBBuf);

	ndx=0;
	for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
	  for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
	    {
	      IPosition plane1(ndx);
	      plane1=ndx;
	      plane1(2)=1; // The other poln. plane
	      avgPBBuf(ndx) = (avgPBBuf(ndx) + avgPBBuf(plane1))/2.0;
	    }
	for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
	  for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
	    {
	      IPosition plane1(ndx);
	      plane1=ndx;
	      plane1(2)=1; // The other poln. plane
	      avgPBBuf(plane1) = avgPBBuf(ndx);
	    }
	if (fabs(pbMax-1.0) > 1E-3)
	  {
	    //	    avgPBBuf = avgPBBuf/noOfPASteps;
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
      }
    pbNormalized = True;
  }
  //
  //---------------------------------------------------------------
  //
  Bool AWProjectFT::makeAveragePB0(const VisBuffer& vb, 
				     const ImageInterface<Complex>& image,
				     //TempImage<Float>& thesquintPB,
				     TempImage<Float>& theavgPB)
  {
    TempImage<Float> localPB;
    
    logIO() << LogOrigin("AWProjecFT","makeAveragePB")
	    << LogIO::NORMAL;
    
    localPB.resize(image.shape()); localPB.setCoordinateInfo(image.coordinates());
    localPB.setMaximumCacheSize(cachesize);
    // cerr << "Max. cache size = " << localPB.maximumCacheSize() << " " << cachesize << endl;
    //
    // If this is the first time, resize the average PB
    //
    if (resetPBs)
      {
	logIO() << "Initializing the average PBs"
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

    {
      VLACalcIlluminationConvFunc vlaPB;
      Nant_p     = vb.msColumns().antenna().nrow();
      if (bandID_p == -1) bandID_p=evlacf_p.getVisParams(vb);
      vlaPB.applyPB(localPB, vb, bandID_p);
    }
    
    IPosition twoDPBShape(localPB.shape());
    TempImage<Complex> localTwoDPB(twoDPBShape,localPB.coordinates());
    localTwoDPB.setMaximumCacheSize(cachesize);
    Float peak=0;
    Int NAnt;
    noOfPASteps++;
    NAnt=1;
   
//     logIO() << " Shape of localPB Cube : " << twoDPBShape << LogIO::POST;
//     logIO() << " Shape of avgPB Cube : " << theavgPB.shape() << LogIO::POST;

    for(Int ant=0;ant<NAnt;ant++)
      { //Ant loop
	{
	  IPosition ndx(4,0,0,0,0);
	  for(ndx(0)=0; ndx(0)<twoDPBShape(0); ndx(0)++)
	    for(ndx(1)=0; ndx(1)<twoDPBShape(1); ndx(1)++)
	      for(ndx(2)=0; ndx(2)<twoDPBShape(2); ndx(2)++)
	       for(ndx(3)=0; ndx(3)<twoDPBShape(3); ndx(3)++)
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
	  
	  IPosition fs(fbuf.shape());
	  {
	    IPosition ndx(4,0,0,0,0),avgNDX(4,0,0,0,0);
	    for(ndx(3)=0,avgNDX(3)=0;ndx(3)<fs(3);ndx(3)++,avgNDX(3)++)
	    {
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
	  }
	  if (!isRefF) theavgPB.put(fbuf);
	  pbPeaks += peak;
	}
      }
    theavgPB.setCoordinateInfo(localPB.coordinates());
    return True; // i.e., an average PB was made and is in the mem. cache
  }
  //
  //---------------------------------------------------------------
  //
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
  Int AWProjectFT::locateConvFunction(const Int Nw, const Float pa)
  {
    Int i;
    Quantity dPA = paChangeDetector.getParAngleTolerance(),
      PA=Quantity(pa,"rad");
    if ((i=cfCache.locateConvFunction(Nw, PA, dPA, cfs_p))!=NOTCACHED)
      convFunc.reference(*cfs_p.data);
    return i;
  }
  void AWProjectFT::makeCFPolMap(const VisBuffer& vb, const Vector<Int>& locCfStokes,
				 Vector<Int>& polM)
  {
    Vector<Int> msStokes = vb.corrType();
    Int nPol = msStokes.nelements();
    polM.resize(polMap.shape());
    polM = -1;

    for(Int i=0;i<nPol;i++)
      for(uInt j=0;j<locCfStokes.nelements();j++)
	if (locCfStokes(j) == msStokes(i))
	    {polM(i) = j;break;}
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
  void AWProjectFT::makeConjPolMap(const VisBuffer& vb, 
				     const Vector<Int> cfPolMap, 
				     Vector<Int>& conjPolMap)
  {
    //
    // All the Natak (Drama) below with slicers etc. is to extract the
    // Poln. info. for the first IF only (not much "information
    // hiding" for the code to slice arrays in a general fashion).
    //
    // Extract the shape of the array to be sliced.
    //
    Array<Int> stokesForAllIFs = vb.msColumns().polarization().corrType().getColumn();
    IPosition stokesShape(stokesForAllIFs.shape());
    IPosition firstIFStart(stokesShape),firstIFLength(stokesShape);
    //
    // Set up the start and length IPositions to extract only the
    // first column of the array.  The following is required since the
    // array could have only one column as well.
    //
    firstIFStart(0)=0;firstIFLength(0)=stokesShape(0);
    for(uInt i=1;i<stokesShape.nelements();i++) {firstIFStart(i)=0;firstIFLength(i)=1;}
    //
    // Construct the slicer and produce the slice.  .nonDegenerate
    // required to ensure the result of slice is a pure vector.
    //
    Vector<Int> visStokes = stokesForAllIFs(Slicer(firstIFStart,firstIFLength)).nonDegenerate();

    conjPolMap = cfPolMap;
    
    Int i,j,N = cfPolMap.nelements();
    for(i=0;i<N;i++)
      if (cfPolMap[i] > -1)
	if      (visStokes[i] == Stokes::RR) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::LL) break; 
	    conjPolMap[i]=cfPolMap[j];
	  }
	else if (visStokes[i] == Stokes::LL) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::RR) break; 
	    conjPolMap[i]=cfPolMap[j];
	  }
	else if (visStokes[i] == Stokes::LR) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::RL) break; 
	    conjPolMap[i]=cfPolMap[j];
	  }
	else if (visStokes[i] == Stokes::RL) 
	  {
	    conjPolMap[i]=-1;
	    for(j=0;j<N;j++) if (visStokes[j] == Stokes::LR) break; 
	    conjPolMap[i]=cfPolMap[j];
	  }
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::findConvFunction(const ImageInterface<Complex>& image,
				       const VisBuffer& vb)
  {
    Int PAIndex_l=0;
    if (!paChangeDetector.changed(vb,0)) return;

    logIO() << LogOrigin("AWProjectFT", "findConvFunction")  << LogIO::NORMAL;
    
    ok();
    
    
    CoordinateSystem coords(image.coordinates());
    {
      Int spIndex=image.coordinates().findCoordinate(Coordinate::SPECTRAL); ;
      cfRefFreq_p = image.coordinates().spectralCoordinate(spIndex).referenceValue()(0);
    }
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
   polInUse = evlacf_p.makePBPolnCoords(coords,cfStokes,vb,polMap);
   //    polInUse=evlacf_p.makePBPolnCoords(coords,vb);
    
   Float pa=getVBPA(vb);
   Int cfSource=locateConvFunction(wConvSize, pa);
   lastPAUsedForWtImg = currentCFPA = pa;
    
   Bool pbMade=False;
   if (cfSource==DISKCACHE) // CF found and loaded from the disk cache
     {
	//	cout << "### New CFPA = " << currentCFPA << endl;
	polInUse  = cfs_p.data->shape()(3);
	wConvSize = cfs_p.data->shape()(2);
	try
	  {
	    cfCache.loadAvgPB(avgPB);
	    avgPBReady=True;
	  }
	catch (AipsError& err)
	  {
	    logIO() << "Average PB does not exist in the cache.  A fresh one will be made."
		    << LogIO::NORMAL << LogIO::POST;
	    pbMade=makeAveragePB0(vb, image, avgPB);
	    pbNormalized=False; normalizeAvgPB(); pbNormalized=True;
	  }
      }
    else if (cfSource==MEMCACHE)  // CF found in the mem. cache
      {
      }
    else                     // CF not found in either cache
      {
	//
	// Make the CF, update the average PB and update the CF and
	// the avgPB disk cache
	//
	PAIndex_l = abs(cfSource);
        //
        // Load the average PB from the disk since it's going to be
        // updated in memory and on the disk.  Without loading it from
        // the disk (from a potentially more complete existing cache),
        // the average PB can get inconsistant with the rest of the
        // cache.
        //
// 	logIO() << LogOrigin("AWProjectFT::findConvFunction()","") 
// 		<< "Making the convolution function for PA=" << pa << "deg."
// 		<< LogIO::NORMAL 
// 		<< LogIO::POST;
	evlacf_p.makeConvFunction(image,wConvSize,vb, pa, polMap,cfStokes,
				  cfs_p, cfwts_p);
	convFunc.reference(*cfs_p.data);
	//	makeConvFunction(image,vb,pa);
	try
	  {
	    cfCache.loadAvgPB(avgPB);
	    resetPBs = False;
	    avgPBReady=True;
	  }
	catch(SynthesisFTMachineError &err)
	  {
	    logIO() << LogOrigin("AWProjectFT::findConvFunction()","") 
		    << "Average PB does not exist in the cache.  A fresh one will be made." 
		    << LogIO::NORMAL 
		    << LogIO::POST;
	    pbMade=makeAveragePB0(vb, image, avgPB);
	  }

	//	makeAveragePB(vb, image, polInUse,avgPB);
	pbNormalized=False; 
	normalizeAvgPB();
	pbNormalized=True;
	Int index=coords.findCoordinate(Coordinate::SPECTRAL);
	SpectralCoordinate spCS = coords.spectralCoordinate(index);
	Vector<Double> refValue; refValue.resize(1);refValue(0)=cfRefFreq_p;
	spCS.setReferenceValue(refValue);
	coords.replaceCoordinate(spCS,index);

	cfs_p.coordSys=coords; cfs_p.pa=Quantity(pa,"rad");
	cfwts_p.coordSys=coords; cfwts_p.pa=Quantity(pa,"rad");

	cfCache.cacheConvFunction(cfs_p);
	cfCache.cacheConvFunction(cfwts_p,"WT",False);

	cfCache.flush(); // Write the aux info file

	if (pbMade) cfCache.flush(avgPB); // Save the AVG PB and write the aux info.
      }

    verifyShapes(avgPB.shape(), image.shape());

    Int lastPASlot = PAIndex_l;

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
	Int maxMemoryMB=HostInfo::memoryTotal(true)/1024;
	String unit(" KB");
	Float memoryKB=0;
	memoryKB=(Float)cfCache.size();
	
	memoryKB = Int(memoryKB/1024.0+0.5);
	if (memoryKB > 1024) {memoryKB /=1024; unit=" MB";}
	
	logIO() << "Memory used in gridding functions = "
		<< (Int)(memoryKB+0.5) << unit << " out of a maximum of "
		<< maxMemoryMB << " MB" << LogIO::POST;
	
	//
	// Show the list of support sizes along the w-axis for the current PA.
	//
	//	logIO() << "Convolution support [CF#= " << lastPASlot 
	logIO() << "Convolution support = " << cfs_p.xSupport 
		<< " pixels in Fourier plane" << LogIO::POST;
      }

    IPosition shp(convFunc.shape());
    IPosition ndx(shp);
    ndx =0;
    Area.resize(Area.nelements()+1,True);
    Area(lastPASlot)=0;
    Complex a=0;
    for(ndx(0)=0;ndx(0)<shp(0);ndx(0)++)
      for(ndx(1)=0;ndx(1)<shp(1);ndx(1)++)
	Area(lastPASlot)+=convFunc(ndx);
  }
  //
  //------------------------------------------------------------------------------
  //
  void AWProjectFT::initializeToVis(ImageInterface<Complex>& iimage,
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
    else isTiled=False;
    //
    // If we are memory-based then read the image in and create an
    // ArrayLattice otherwise just use the PagedImage
    //

    isTiled=False;

    if(isTiled){
    	lattice=CountedPtr<Lattice<Complex> > (image, False);
    }
    else 
      {
	IPosition gridShape(4, nx, ny, npol, nchan);
	griddedData.resize(gridShape);
	griddedData=Complex(0.0);
	
	IPosition stride(4, 1);
	IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
		      (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
	IPosition trc(blc+image->shape()-stride);
	
	IPosition start(4, 0);
	griddedData(blc, trc) = image->getSlice(start, image->shape());
	
	arrayLattice = new ArrayLattice<Complex>(griddedData);
	lattice=arrayLattice;
      }

    //AlwaysAssert(lattice, AipsError);
    
    logIO() << LogIO::DEBUGGING << "Starting FFT of image" << LogIO::POST;
    
    Vector<Float> sincConv(nx);
    Float centerX=nx/2;
    for (Int ix=0;ix<nx;ix++) 
      {
	Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
	if(ix==centerX) sincConv(ix)=1.0;
	else            sincConv(ix)=sin(x)/x;
      }
    
    Vector<Complex> correction(nx);
    //
    // Do the Grid-correction
    //
    {
      normalizeAvgPB();
      
      IPosition cursorShape(4, nx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
	  
      verifyShapes(avgPB.shape(), image->shape());
      Array<Float> avgBuf; avgPB.get(avgBuf);
      if (max(avgBuf) < 1e-04)
	throw(AipsError("Normalization by PB requested but either PB not found in the cache "
			"or is ill-formed."));

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
	  for(int ix=0;ix<nx;ix++) 
	    {
	      // PBCorrection(ix) = (FUNC(PBCorrection(ix)))/(sincConv(ix)*sincConv(iy));
	      
	      //
	      // This is with PS functions included
	      //
	      // if (doPBCorrection)
	      // 	{
	      // 	  PBCorrection(ix) = FUNC(PBCorrection(ix))/(sincConv(ix)*sincConv(iy));
	      // 	  //PBCorrection(ix) = FUNC(PBCorrection(ix))*(sincConv(ix)*sincConv(iy));
 	      // 	  if ((abs(PBCorrection(ix)*correction(ix))) >= pbLimit_p)
	      // 	    {lix.rwVectorCursor()(ix) /= (PBCorrection(ix))*correction(ix);}
 	      // 	  else
	      // 	    {lix.rwVectorCursor()(ix) *= (sincConv(ix)*sincConv(iy));}
	      // 	}
	      // else 
	      // 	lix.rwVectorCursor()(ix) /= (correction(ix)/(sincConv(ix)*sincConv(iy)));
	      //
	      // This without the PS functions
	      //
	      if (doPBCorrection)
		{
		  // PBCorrection(ix) = FUNC(PBCorrection(ix))/(sincConv(ix)*sincConv(iy));
		  PBCorrection(ix) = FUNC(PBCorrection(ix))*(sincConv(ix)*sincConv(iy));
//		  PBCorrection(ix) = (PBCorrection(ix))*(sincConv(ix)*sincConv(iy));
 		  if ((abs(PBCorrection(ix))) >= pbLimit_p)
		    {lix.rwVectorCursor()(ix) /= (PBCorrection(ix));}
 		  else
		    {lix.rwVectorCursor()(ix) *= (sincConv(ix)*sincConv(iy));}
		}
	      else 
		lix.rwVectorCursor()(ix) /= (1.0/(sincConv(ix)*sincConv(iy)));
	    }
	}
    }
    // {
    //   ostringstream name;
    //   cout << image->shape() << endl;
    //   name << "theModel.im";
    //   PagedImage<Float> tmp(image->shape(), image->coordinates(), name);
    //   Array<Complex> buf;
    //   Bool isRef = lattice->get(buf);
    //   cout << "The model max. = " << max(buf) << endl;
    //   LatticeExpr<Float> le(abs((*lattice)));
    //   tmp.copyData(le);
    // }
    //
    // Now do the FFT2D in place
    //
//     {
//       Array<Complex> buf;
//       Bool isRef = lattice->get(buf);
//     }
    LatticeFFT::cfft2d(*lattice);

    logIO() << LogIO::DEBUGGING << "Finished FFT" << LogIO::POST;
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::initializeToVis(ImageInterface<Complex>& iimage,
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
  void AWProjectFT::finalizeToVis()
  {
    logIO() << "##########finalizeToVis()###########" << LogIO::DEBUGGING << LogIO::POST;
    if(isTiled) 
      {
	logIO() << LogOrigin("AWProjectFT", "finalizeToVis")  << LogIO::NORMAL;
	
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
  void AWProjectFT::initializeToSky(ImageInterface<Complex>& iimage,
				     Matrix<Float>& weight,
				     const VisBuffer& vb)
  {
    logIO() << "#########initializeToSky()##########" << LogIO::DEBUGGING << LogIO::POST;
    
    // image always points to the image
    image=&iimage;
    
    init();
    initMaps(vb);
    
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
	lattice=CountedPtr<Lattice<Complex> > (image, False);
      }
    else 
      {
	IPosition gridShape(4, nx, ny, npol, nchan);
	griddedData.resize(gridShape);
	griddedData=Complex(0.0);
	arrayLattice = new ArrayLattice<Complex>(griddedData);
	lattice=arrayLattice;
      }
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::finalizeToSky()
  {
    //
    // Now we flush the cache and report statistics For memory based,
    // we don't write anything out yet.
    //
    logIO() << "#########finalizeToSky()#########" << LogIO::DEBUGGING << LogIO::POST;
    if(isTiled) 
      {
	logIO() << LogOrigin("AWProjectFT", "finalizeToSky")  << LogIO::NORMAL;
	
	AlwaysAssert(image, AipsError);
	AlwaysAssert(imageCache, AipsError);
	imageCache->flush();
	ostringstream o;
	imageCache->showCacheStatistics(o);
	logIO() << o.str() << LogIO::POST;
      }
    if(pointingToImage) delete pointingToImage; pointingToImage=0;

    paChangeDetector.reset();
    cfCache.flush();
    convFuncCacheReady=True;
  }
  //
  //---------------------------------------------------------------
  //
  Array<Complex>* AWProjectFT::getDataPointer(const IPosition& centerLoc2D,
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
		  Double *currentCFPA, Double *actualPA,Double *cfRefFreq_p);
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
		  Double *currentCFPA, Double *actualPA, Double *cfRefFreq_p);
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
		  Double *currentCFPA, Double *actualPA, Double *cfRefFreq_p);
  }
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::runFortranGet(Matrix<Double>& uvw,Vector<Double>& dphase,
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
    actualPA = getVBPA(vb);

    N=polMap.nelements();
    CFMap = polMap; ConjCFMap = polMap;
    for(Int i=0;i<N;i++) CFMap[i] = polMap[N-i-1];
    
    Array<Complex> rotatedConvFunc;
//     SynthesisUtils::rotateComplexArray(logIO(), convFunc, convFuncCS_p, 
// 				       rotatedConvFunc,(currentCFPA-actualPA),"CUBIC");
    SynthesisUtils::rotateComplexArray(logIO(), convFunc, convFuncCS_p, 
    				       rotatedConvFunc,0.0,"LINEAR");
    // SynthesisUtils::rotateComplexArray(logIO(), convFunc, convFuncCS_p, 
    // 				       rotatedConvFunc,(currentCFPA-actualPA),"LINEAR");

    ConjCFMap = polMap;
    makeCFPolMap(vb,cfStokes,CFMap);
    makeConjPolMap(vb,CFMap,ConjCFMap);

    
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
    convSupport_p   = cfs_p.xSupport.getStorage(deleteThem(CONVSUPPORT));
    //    f_convFunc_p      = convFunc.getStorage(deleteThem(CONVFUNC));
    f_convFunc_p      = rotatedConvFunc.getStorage(deleteThem(CONVFUNC));
    chanMap_p       = chanMap.getStorage(deleteThem(CHANMAP));
    polMap_p        = polMap.getStorage(deleteThem(POLMAP));
    vb_ant1_p       = vb.antenna1().getStorage(deleteThem(VBANT1));
    vb_ant2_p       = vb.antenna2().getStorage(deleteThem(VBANT2));
    l_off_p     = l_off.getStorage(deleteThem(RAOFF));
    m_off_p    = m_off.getStorage(deleteThem(DECOFF));
    
    //    Int npa=convSupport.shape()(2),actualConvSize;
    Int npa=1,actualConvSize;
    Int paIndex_Fortran = paIndex;
    actualConvSize = cfs_p.data->shape()(0);
    
    //    IPosition shp=convSupport.shape();
    
    dpbwproj(uvw_p,
	     dphase_p,
	     //		  vb.modelVisCube().getStorage(del),
	     visdata_p,
	     &s.asVector()(0),
	     &s.asVector()(1),
	     //	   gradVisAzData_p,
	     //	   gradVisElData_p,
	     //	    &gradS(0),
	     //	    &gradS(1),
	     //	   &Conj,
	     flags_p,
	     rowFlags_p,
	     &s.asVector()(2),
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
	     ,&actualPA,&cfRefFreq_p
	     );
    
    ConjCFMap.freeStorage((const Int *&)ConjCFMap_p,deleteThem(CONJCFMAP));
    CFMap.freeStorage((const Int *&)CFMap_p,deleteThem(CFMAP));
    
    l_off.freeStorage((const Float*&)l_off_p,deleteThem(RAOFF));
    m_off.freeStorage((const Float*&)m_off_p,deleteThem(DECOFF));
    uvw.freeStorage((const Double*&)uvw_p,deleteThem(UVW));
    dphase.freeStorage((const Double*&)dphase_p,deleteThem(DPHASE));
    visdata.putStorage(visdata_p,deleteThem(VISDATA));
    flags.freeStorage((const Int*&) flags_p,deleteThem(FLAGS));
    rowFlags.freeStorage((const Int *&)rowFlags_p,deleteThem(ROWFLAGS));
    actualOffset.freeStorage((const Double*&)actualOffset_p,deleteThem(ACTUALOFFSET));
    dataPtr->freeStorage((const Complex *&)dataPtr_p,deleteThem(DATAPTR));
    uvScale.freeStorage((const Double*&) uvScale_p,deleteThem(UVSCALE));
    vb.frequency().freeStorage((const Double*&)vb_freq_p,deleteThem(VBFREQ));
    cfs_p.xSupport.freeStorage((const Int*&)convSupport_p,deleteThem(CONVSUPPORT));
    convFunc.freeStorage((const Complex *&)f_convFunc_p,deleteThem(CONVFUNC));
    chanMap.freeStorage((const Int*&)chanMap_p,deleteThem(CHANMAP));
    polMap.freeStorage((const Int*&) polMap_p,deleteThem(POLMAP));
    vb.antenna1().freeStorage((const Int*&) vb_ant1_p,deleteThem(VBANT1));
    vb.antenna2().freeStorage((const Int*&) vb_ant2_p,deleteThem(VBANT2));
  }
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::runFortranGetGrad(Matrix<Double>& uvw,Vector<Double>& dphase,
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
    actualPA = getVBPA(vb);
    ConjCFMap = polMap;
    makeCFPolMap(vb,cfStokes,CFMap);
    makeConjPolMap(vb,CFMap,ConjCFMap);

    Array<Complex> rotatedConvFunc;
//     SynthesisUtils::rotateComplexArray(logIO(), convFunc, convFuncCS_p, 
//  				       rotatedConvFunc,(currentCFPA-actualPA),"LINEAR");
    SynthesisUtils::rotateComplexArray(logIO(), convFunc, convFuncCS_p, 
    				       rotatedConvFunc,0.0);
    // SynthesisUtils::rotateComplexArray(logIO(), convFunc, convFuncCS_p, 
    // 				       rotatedConvFunc,(currentCFPA-actualPA),"LINEAR");

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
    convSupport_p   = cfs_p.xSupport.getStorage(deleteThem(CONVSUPPORT));
    //    f_convFunc_p      = convFunc.getStorage(deleteThem(CONVFUNC));
    f_convFunc_p      = rotatedConvFunc.getStorage(deleteThem(CONVFUNC));
    chanMap_p       = chanMap.getStorage(deleteThem(CHANMAP));
    polMap_p        = polMap.getStorage(deleteThem(POLMAP));
    vb_ant1_p       = vb.antenna1().getStorage(deleteThem(VBANT1));
    vb_ant2_p       = vb.antenna2().getStorage(deleteThem(VBANT2));
    l_off_p     = l_off.getStorage(deleteThem(RAOFF));
    m_off_p    = m_off.getStorage(deleteThem(DECOFF));
    
    //    Int npa=convSupport.shape()(2),actualConvSize;
    Int npa=1,actualConvSize;
    Int paIndex_Fortran = paIndex;
    actualConvSize = cfs_p.data->shape()(0);
    
    //    IPosition shp=convSupport.shape();

    dpbwgrad(uvw_p,
	     dphase_p,
	     //		  vb.modelVisCube().getStorage(del),
	     visdata_p,
	     &s.asVector()(0),
	     &s.asVector()(1),
	     gradVisAzData_p,
	     gradVisElData_p,
	     //	    &gradS(0),
	     //	    &gradS(1),
	     &Conj,
	     flags_p,
	     rowFlags_p,
	     &s.asVector()(2),
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
	     ,&actualPA,&cfRefFreq_p
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
    cfs_p.xSupport.freeStorage((const Int*&)convSupport_p,deleteThem(CONVSUPPORT));
    convFunc.freeStorage((const Complex *&)f_convFunc_p,deleteThem(CONVFUNC));
    chanMap.freeStorage((const Int*&)chanMap_p,deleteThem(CHANMAP));
    polMap.freeStorage((const Int*&) polMap_p,deleteThem(POLMAP));
    vb.antenna1().freeStorage((const Int*&) vb_ant1_p,deleteThem(VBANT1));
    vb.antenna2().freeStorage((const Int*&) vb_ant2_p,deleteThem(VBANT2));
  }
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::runFortranPut(Matrix<Double>& uvw,Vector<Double>& dphase,
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
    actualPA = getVBPA(vb);
    ConjCFMap = polMap;

    Array<Complex> rotatedConvFunc;
//    SynthesisUtils::rotateComplexArray(logIO(), convFunc, convFuncCS_p, 
//				       rotatedConvFunc,(currentCFPA-actualPA),"LINEAR");
     SynthesisUtils::rotateComplexArray(logIO(), convFunc, convFuncCS_p, 
 				       rotatedConvFunc,0.0,"LINEAR");

    /*
    CFMap = polMap; ConjCFMap = polMap;
    CFMap = makeConjPolMap(vb);
    */
     makeCFPolMap(vb,cfStokes,CFMap);
    makeConjPolMap(vb,CFMap,ConjCFMap);

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
    convSupport_p   = cfs_p.xSupport.getStorage(deleteThem(CONVSUPPORT));
    //    f_convFunc_p      = convFunc.getStorage(deleteThem(CONVFUNC));
    f_convFunc_p      = rotatedConvFunc.getStorage(deleteThem(CONVFUNC));
    chanMap_p       = chanMap.getStorage(deleteThem(CHANMAP));
    polMap_p        = polMap.getStorage(deleteThem(POLMAP));
    vb_ant1_p       = (Int *)(vb.antenna1().getStorage(deleteThem(VBANT1)));
    vb_ant2_p       = (Int *)(vb.antenna2().getStorage(deleteThem(VBANT2)));
    l_off_p     = l_off.getStorage(deleteThem(RAOFF));
    m_off_p    = m_off.getStorage(deleteThem(DECOFF));
    weight_p        = (Float *)(weight.getStorage(deleteThem(WEIGHT)));
    sumwt_p         = sumWeight.getStorage(deleteThem(SUMWEIGHT));
    
    
    //    Int npa=convSupport.shape()(2),actualConvSize;
    Int npa=1,actualConvSize;
    Int paIndex_Fortran = paIndex; 
    actualConvSize = cfs_p.data->shape()(0);
    
    //    IPosition shp=convSupport.shape();
    
    gpbwproj(uvw_p,
	     dphase_p,
	     //		  vb.modelVisCube().getStorage(del),
	     visdata_p,
	     &s.asVector()(0),
	     &s.asVector()(1),
	     //	   gradVisAzData_p,
	     //	   gradVisElData_p,
	     //	    &gradS(0),
	     //	    &gradS(1),
	     //	   &Conj,
	     &doPSF,
	     flags_p,
	     rowFlags_p,
	     weight_p,
	     &s.asVector()(2),
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
	     ,&actualPA,&cfRefFreq_p
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
    cfs_p.xSupport.freeStorage((const Int*&)convSupport_p,deleteThem(CONVSUPPORT));
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
  void AWProjectFT::put(const VisBuffer& vb, Int row, Bool dopsf,
			  FTMachine::Type type,
			  const Matrix<Float>& imwght)
  {
    // Take care of translation of Bools to Integer
    Int idopsf=0;
    makingPSF=dopsf;
    if(dopsf) idopsf=1;
    
    findConvFunction(*image, vb);
    

    const Matrix<Float> *imagingweight;
    if(imwght.nelements()>0)
      imagingweight=&imwght;
    else
      imagingweight=&(vb.imagingWeight());

    const Cube<Complex> *data;
    if(type==FTMachine::MODEL)
      data=&(vb.modelVisCube());
    else if(type==FTMachine::CORRECTED)
      data=&(vb.correctedVisCube());
    else
      data=&(vb.visCube());
    
    Bool isCopy;
    const casa::Complex *datStorage=data->getStorage(isCopy);
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
    rowFlags(vb.flagRow())=True;
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
		
		Int tmpPAI=0;
		if (dopsf) doPSF=1; else doPSF=0;
		runFortranPut(uvw,dphase,*datStorage,s,Conj,flags,rowFlags,
			      *imagingweight,rownr,actualOffset,
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
	
	Int tmpPAI=0;
	runFortranPut(uvw,dphase,*datStorage,s,Conj,flags,rowFlags,
		      *imagingweight,
		      row,uvOffset,
		      griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
		      l_offsets,m_offsets,sumWeight,area,doGrad,doPSF,tmpPAI);
      }
    
    data->freeStorage(datStorage, isCopy);
  }
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::initVisBuffer(VisBuffer& vb, Type whichVBColumn)
  {
    if (whichVBColumn      == FTMachine::MODEL)    vb.modelVisCube()=Complex(0.0,0.0);
    else if (whichVBColumn == FTMachine::OBSERVED) vb.visCube()=Complex(0.0,0.0);
  }
  //
  //----------------------------------------------------------------------
  //
  void AWProjectFT::initVisBuffer(VisBuffer& vb, Type whichVBColumn, Int row)
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
  void AWProjectFT::nget(VisBuffer& vb,
			  // These offsets should be appropriate for the VB
			  Array<Float>& l_off, Array<Float>& m_off,
			  Cube<Complex>& Mout,
			  Cube<Complex>& dMout1,
			  Cube<Complex>& dMout2,
			  Int Conj, Int doGrad)
  {
    Int startRow, endRow, nRow;
    nRow=vb.nRow();
    startRow=0;
    endRow=nRow-1;

    Mout = dMout1 = dMout2 = Complex(0,0);

    findConvFunction(*image, vb);
    Int NAnt=0;
    Nant_p     = vb.msColumns().antenna().nrow();
    if (bandID_p == -1) bandID_p=evlacf_p.getVisParams(vb);
    if (doPointing)   
      NAnt = findPointingOffsets(vb,l_offsets,m_offsets,False);

    l_offsets=l_off;
    m_offsets=m_off;
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
    rowFlags(vb.flagRow())=True;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    
    IPosition s,gradS;
    Cube<Complex> visdata,gradVisAzData,gradVisElData;
    //
    // visdata now references the Mout data structure rather than to the internal VB storeage.
    //
    visdata.reference(Mout);

    if (doGrad)
      {
	// The following should reference some slice of dMout?
	gradVisAzData.reference(dMout1);
	gradVisElData.reference(dMout2);
      }
    //
    // Begin the actual de-gridding.
    //
    if(isTiled) 
      {
	logIO() << "AWProjectFT::nget(): The sky model is tiled" << LogIO::NORMAL << LogIO::POST;
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
		tmpPAI = 1;
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
	Int ScanNo=0, tmpPAI, trow=-1;
	Double area=1.0;
	tmpPAI = 1;
	runFortranGetGrad(uvw,dphase,visdata/*vb.modelVisCube()*/,s,
			  gradVisAzData, gradVisElData,
			  Conj,flags,rowFlags,trow,
			  uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
			  l_offsets,m_offsets,area,doGrad,tmpPAI);
      }
    
  }
  void AWProjectFT::get(VisBuffer& vb,       
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

    Nant_p     = vb.msColumns().antenna().nrow();
    if (bandID_p == -1) bandID_p=evlacf_p.getVisParams(vb);
    Int NAnt=0;
    if (doPointing)   
      NAnt = findPointingOffsets(vb,pointingOffsets,l_offsets,m_offsets,False);

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
    //    
    //Check if ms has changed then cache new spw and chan selection
    //
    if(vb.newMS()) matchAllSpwChans(vb);
    
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
    rowFlags(vb.flagRow())=True;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
	
    for (Int rownr=startRow; rownr<=endRow; rownr++) 
      if (vb.antenna1()(rownr) != vb.antenna2()(rownr)) 
	rowFlags(rownr) = (vb.flagRow()(rownr)==True);
    
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
		tmpPAI = 1;
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

	tmpPAI = 1;

	runFortranGetGrad(uvw,dphase,visdata/*vb.modelVisCube()*/,s,
			  gradVisAzData, gradVisElData,
			  Conj,flags,rowFlags,row,
			  uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
			  l_offsets,m_offsets,area,doGrad,tmpPAI);
// 	runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,row,
// 		      uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
// 		      l_offsets,m_offsets,area,doGrad,tmpPAI);
      }
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::get(VisBuffer& vb, Int row)
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
    
    Nant_p     = vb.msColumns().antenna().nrow();
    if (bandID_p == -1) bandID_p=evlacf_p.getVisParams(vb);
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
    rowFlags(vb.flagRow())=True;
    
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
		Int tmpPAI=1;
		runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,rownr,
			      actualOffset,dataPtr,aNx,aNy,npol,nchan,vb,NAnt,ScanNo,sigma,
			      l_offsets,m_offsets,area,doGrad,tmpPAI);
	      }
	  }
      }
    else 
      {
	
	IPosition s(vb.modelVisCube().shape());
	Int Conj=0,doGrad=0,ScanNo=0;
	Double area=1.0;
	Int tmpPAI=1;
	runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,row,
		      uvOffset,&griddedData,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
		      l_offsets,m_offsets,area,doGrad,tmpPAI);
	/*
	static int junk=0;
	if (junk==4)
	  {
	    cout << "Time = " << vb.time()/1e9 << endl;
	  for(Int i=0;i<vb.modelVisCube().shape()(2);i++)
	    cout << "PBWP: Residual: " << i 
		 << " " << vb.modelVisCube()(0,0,i) 
		 << " " << vb.modelVisCube()(3,0,i)
		 << " " << vb.visCube()(0,0,i) 
		 << " " << vb.visCube()(3,0,i)
		 << " " << vb.flag()(0,i) 
		 << " " << vb.antenna1()(i)<< "-" << vb.antenna2()(i) 
		 << " " << vb.flagRow()(i) 
		 << " " << vb.flagCube()(0,0,i) 
		 << " " << vb.flagCube()(3,0,i) 
		 << endl;
	  }
	junk++;
	*/
      }
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::get(VisBuffer& vb, Cube<Complex>& modelVis, 
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
    
    Int NAnt=0;
    
    if (doPointing) 
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
    rowFlags(vb.flagRow())=True;
    if(!usezero_p) 
      for (Int rownr=startRow; rownr<=endRow; rownr++) 
	if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    
    IPosition s(modelVis.shape());
    Int Conj=0,doGrad=0,ScanNo=0;
    Double area=1.0;
    Int tmpPAI=1;
    runFortranGet(uvw,dphase,vb.modelVisCube(),s,Conj,flags,rowFlags,row,
		  offset,&griddedVis,nx,ny,npol,nchan,vb,NAnt,ScanNo,sigma,
		  l_offsets,m_offsets,area,doGrad,tmpPAI);
  }
  //
  //---------------------------------------------------------------
  //
  // Finalize the FFT to the Sky. Here we actually do the FFT and
  // return the resulting image
  ImageInterface<Complex>& AWProjectFT::getImage(Matrix<Float>& weights,
						  Bool normalize) 
  {
    //AlwaysAssert(lattice, AipsError);
    AlwaysAssert(image, AipsError);
    
    logIO() << "#########getimage########" << LogIO::DEBUGGING << LogIO::POST;
    
    logIO() << LogOrigin("AWProjectFT", "getImage") << LogIO::NORMAL;
    
    weights.resize(sumWeight.shape());
    
    convertArray(weights, sumWeight);
    //  
    // If the weights are all zero then we cannot normalize otherwise
    // we don't care.
    //
    if(max(weights)==0.0) 
      {
	if(normalize) logIO() << LogIO::SEVERE
			      << "No useful data in AWProjectFT: weights all zero"
			      << LogIO::POST;
	else logIO() << LogIO::WARN << "No useful data in AWProjectFT: weights all zero"
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
	{
	  normalizeAvgPB();
	  Int inx = lattice->shape()(0);
	  Int iny = lattice->shape()(1);
	  Vector<Complex> correction(inx);
	  
	  Vector<Float> sincConv(nx);
	  Float centerX=nx/2;
	  for (Int ix=0;ix<nx;ix++) 
	    {
	      Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
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
	      
	      if(weights(pol, chan)>0.0) 
		{
		  Int iy=lix.position()(1);
		  gridder->correctX1D(correction,iy);
		  
		  Vector<Float> PBCorrection(liavgpb.rwVectorCursor().shape()),
		    avgPBVec(liavgpb.rwVectorCursor().shape());
		  
		  PBCorrection = liavgpb.rwVectorCursor();
		  avgPBVec = liavgpb.rwVectorCursor();

		  for(int i=0;i<PBCorrection.shape();i++)
		    {
		      //
		      // This with the PS functions
		      //
		      // PBCorrection(i)=FUNC(avgPBVec(i))*sincConv(i)*sincConv(iy);
		      // if ((abs(PBCorrection(i)*correction(i))) >= pbLimit_p)
		      // 	lix.rwVectorCursor()(i) /= PBCorrection(i)*correction(i);
 		      // else if (!makingPSF)
 		      // 	lix.rwVectorCursor()(i) /= correction(i)*sincConv(i)*sincConv(iy);
		      //
		      // This without the PS functions
		      //
 		      PBCorrection(i)=FUNC(avgPBVec(i))*sincConv(i)*sincConv(iy);
 		      if ((abs(PBCorrection(i))) >= pbLimit_p)
		      	lix.rwVectorCursor()(i) /= PBCorrection(i);
	 	      else if (!makingPSF)
 		      	lix.rwVectorCursor()(i) /= sincConv(i)*sincConv(iy);
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


	if(!isTiled) 
	  {
	    //
	    // Check the section from the image BEFORE converting to a lattice 
	    //
	    IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
			  (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
	    IPosition stride(4, 1);
	    IPosition trc(blc+image->shape()-stride);
	    //
	    // Do the copy
	    //
	    image->put(griddedData(blc, trc));
	    griddedData.resize(IPosition(1,0));
	  }
      }
    
    return *image;
  }
  //
  //---------------------------------------------------------------
  //
  // Get weight image
  void AWProjectFT::getWeightImage(ImageInterface<Float>& weightImage,
				    Matrix<Float>& weights) 
  {
    
    logIO() << LogOrigin("AWProjectFT", "getWeightImage") << LogIO::NORMAL;
    
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
  Bool AWProjectFT::toRecord(RecordInterface& outRec, Bool withImage) 
  {
    
    // Save the current AWProjectFT object to an output state record
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
  Bool AWProjectFT::fromRecord(const RecordInterface& inRec)
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
    	  lattice=CountedPtr<Lattice<Complex> > (image, False);
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
	    IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
			  (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
	    IPosition start(4, 0);
	    IPosition stride(4, 1);
	    IPosition trc(blc+image->shape()-stride);
	    griddedData(blc, trc) = image->getSlice(start, image->shape());
	    
	    arrayLattice = new ArrayLattice<Complex>(griddedData);
	    lattice=arrayLattice;
	  }
	
	AlwaysAssert(image, AipsError);
      };
    return retval;
  }
  //
  //---------------------------------------------------------------
  //
  void AWProjectFT::ok() 
  {
    AlwaysAssert(image, AipsError);
  }
  //----------------------------------------------------------------------
  //
  // Make a plain straightforward honest-to-god image. This returns a
  // complex image, without conversion to Stokes. The polarization
  // representation is that required for the visibilities.
  //
  void AWProjectFT::makeImage(FTMachine::Type type, 
			       VisSet& vs,
			       ImageInterface<Complex>& theImage,
			       Matrix<Float>& weight) 
  {
    logIO() << LogOrigin("AWProjectFT", "makeImage") << LogIO::NORMAL;
    
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

    //    
    // Loop over the visibilities, putting VisBuffers
    //
    paChangeDetector.reset();

    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
      {
	for (vi.origin(); vi.more(); vi++) 
	  {
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
	  }
      }
    finalizeToSky();
    // Normalize by dividing out weights, etc.
    getImage(weight, True);
  }
  
  void AWProjectFT::setPAIncrement(const Quantity& paIncrement)
  {
    paChangeDetector.setTolerance(paIncrement);
    rotateAperture_p = True;
    if (paIncrement.getValue("rad") < 0)
      rotateAperture_p = False;
    logIO() << LogIO::NORMAL <<"Setting PA increment to " << paIncrement.getValue("deg") << " deg" << endl;
    cfCache.setPAChangeDetector(paChangeDetector);
  }

  Bool AWProjectFT::verifyShapes(IPosition pbShape, IPosition skyShape)
  {
    if ((pbShape(0) != skyShape(0)) && // X-axis
	(pbShape(1) != skyShape(1)) && // Y-axis
	(pbShape(2) != skyShape(2)))   // Poln-axis
      {
	throw(AipsError("Sky and/or polarization shape of the avgPB "
			"and the sky model do not match."));
	return False;
      }
    return True;
    
  }

  void AWProjectFT::makeAntiAliasingOp(Vector<Complex>& op, const Int nx_l, const Double HPBW)
  {
    MathFunc<Float> sf(SPHEROIDAL);
    if (op.nelements() != (uInt)nx_l)
      {
	op.resize(nx_l);
	Int inner=nx_l/2, center=nx_l/2;
	//
	// The "complicated" equation below is worthy of a comment (as
	// notes are called in program text).
        //
	// uvScale/nx == Size of a pixel size in the image in radians.
	// Lets call it dx.  HPBW is the HPBW for the antenna at the
	// centre freq. in use.  HPBW/dx == Pixel where the PB will be
	// ~0.5x its peak value.  ((2*N*HPBW)/dx) == the pixel where
	// the N th. PB sidelobe will be (rougly speaking).  When this
	// value is equal to 3.0, the Spheroidal implemtation goes to
	// zero!
	//
	Float dx=uvScale(0)*convSampling/nx;
	Float MaxSideLobeNum = 3.0;
	Float S=1.0*dx/(MaxSideLobeNum*2*HPBW),cfScale;
	
	cout << "UVSCALE = " << uvScale(0) << " " << convSampling << endl;
	cout << "HPBW = " << HPBW 
	     << " " << uvScale(0)/nx 
	     << " " << S 
	     << " " << dx 
	     << endl;
	

	cfScale=S=6.0/inner;
	for(Int ix=-inner;ix<inner;ix++)	    op(ix+center)=sf.value(abs((ix)*cfScale));
	// for(Int ix=-inner;ix<inner;ix++)
	//   if (abs(op(ix+center)) > 1e-8) 
	// 	    cout << "SF: " << ix 
	// 		 << " " << (ix)*cfScale 
	// 		 << " " << real(op(ix+center)) 
	// 		 << " " << imag(op(ix+center))
	// 		 << endl;
      }
  }

  void AWProjectFT::makeAntiAliasingCorrection(Vector<Complex>& correction, 
						 const Vector<Complex>& op,
						 const Int nx_l)
  {
    if (correction.nelements() != (uInt)nx_l)
      {
	correction.resize(nx_l);
	correction=0.0;
	Int opLen=op.nelements(), orig=nx_l/2;
	for(Int i=-opLen;i<opLen;i++)
	  {
	    correction(i+orig) += op(abs(i));
	  }
	ArrayLattice<Complex> tmp(correction);
	LatticeFFT::cfft(tmp,False);
	correction=tmp.get();
      }
//     for(uInt i=0;i<correction.nelements();i++)
//       cout << "FTSF: " << real(correction(i)) << " " << imag(correction(i)) << endl;
  }

  void AWProjectFT::correctAntiAliasing(Lattice<Complex>& image)
  {
    //  applyAntiAliasingOp(cf,2);
    IPosition shape(image.shape());
    IPosition ndx(shape);
    ndx=0;
    makeAntiAliasingCorrection(antiAliasingCorrection, 
			       antiAliasingOp,shape(0));

    Complex tmp,val;
    for(Int i=0;i<polInUse;i++)
      {
	ndx(2)=i;
	for (Int iy=0;iy<shape(1);iy++) 
	  {
	    for (Int ix=0;ix<shape(0);ix++) 
	      {
		ndx(0)=ix;
		ndx(1)=iy;
		tmp = image.getAt(ndx);
		val=(antiAliasingCorrection(ix)*antiAliasingCorrection(iy));
		if (abs(val) > 1e-5) tmp = tmp/val; else tmp=0.0;
		image.putAt(tmp,ndx);
	      }
	  }
      }
  }

  void AWProjectFT::applyAntiAliasingOp(ImageInterface<Complex>& cf, 
					Vector<IPosition>& maxPos,
					Double HPBW, Int op, Bool Square)
  {
    //
    // First the spheroidal function
    //
    IPosition shape(cf.shape());
    IPosition ndx(shape);
    Vector<Double> refPixel=cf.coordinates().referencePixel();
    ndx=0;
    Int nx_l=shape(0),posX,posY;
    makeAntiAliasingOp(antiAliasingOp, nx_l, HPBW);

    Complex tmp,gain;

    for(Int i=0;i<polInUse;i++)
      {
	ndx(2)=i;
	for (Int iy=-nx_l/2;iy<nx_l/2;iy++) 
	  {
	    for (Int ix=-nx_l/2;ix<nx_l/2;ix++) 
	      {
		ndx(0)=ix+nx_l/2;
		ndx(1)=iy+nx_l/2;
		tmp = cf.getAt(ndx);
		posX=ndx(0)+(Int)(refPixel(0)-maxPos(i)(0))+1;
		posY=ndx(1)+(Int)(refPixel(1)-maxPos(i)(1))+1;
		if ((posX > 0) && (posX < nx_l) &&
		    (posY > 0) && (posY < nx_l))
		  gain = antiAliasingOp(posX)*antiAliasingOp(posY);
		else
		  if (op==2) gain = 1.0; else gain=0.0;
		if (Square) gain *= gain;
		switch (op)
		  {
		  case 0: tmp = tmp+gain;break;
		  case 1: 
		    {
		      tmp = tmp*gain;
		      break;
		    }
		  case 2: tmp = tmp/gain;break;
		  }
		cf.putAt(tmp,ndx);
	      }
	  }
      }
  };
  void AWProjectFT::applyAntiAliasingOp(ImageInterface<Float>& cf, 
					Vector<IPosition>& maxPos,
					Double HPBW, Int op, Bool Square)

  {
    //
    // First the spheroidal function
    //
    IPosition shape(cf.shape());
    IPosition ndx(shape);
    Vector<Double> refPixel=cf.coordinates().referencePixel();
    ndx=0;
    Int nx_l=shape(0),posX,posY;
    makeAntiAliasingOp(antiAliasingOp, nx_l, HPBW);

    Float tmp,gain;

    for(Int i=0;i<polInUse;i++)
      {
	ndx(2)=i;
	for (Int iy=-nx_l/2;iy<nx_l/2;iy++) 
	  {
	    for (Int ix=-nx_l/2;ix<nx_l/2;ix++) 
	      {
		ndx(0)=ix+nx_l/2;
		ndx(1)=iy+nx_l/2;
		tmp = cf.getAt(ndx);
		posX=ndx(0)+(Int)(refPixel(0)-maxPos(i)(0))+1;
		posY=ndx(1)+(Int)(refPixel(1)-maxPos(i)(1))+1;
		if ((posX > 0) && (posX < nx_l) &&
		    (posY > 0) && (posY < nx_l))
		  gain = real(antiAliasingOp(posX)*antiAliasingOp(posY));
		else
		  if (op==2) gain = 1.0; else gain=0.0;
		if (Square) gain *= gain;
		switch (op)
		  {
		  case 0: tmp = tmp+gain;break;
		  case 1: 
		    {
		      tmp = tmp*gain;
		      break;
		    }
		  case 2: tmp = tmp/gain;break;
		  }
		cf.putAt(tmp,ndx);
	      }
	  }
      }
  };
} //# NAMESPACE CASA - END

