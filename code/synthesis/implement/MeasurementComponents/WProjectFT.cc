//# WProjectFT.cc: Implementation of WProjectFT class
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
#include <casa/Utilities/CountedPtr.h>
#include <measures/Measures/Stokes.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>
#include <synthesis/MeasurementComponents/WProjectFT.h>
#include <synthesis/MeasurementComponents/WPConvFunc.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/SimOrdMap.h>
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
#include <casa/iostream.h>
#include <casa/iomanip.h>
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

namespace casa { //# NAMESPACE CASA - BEGIN

WProjectFT::WProjectFT( Int nWPlanes, Long icachesize, Int itilesize, 
		   Bool usezero)
  : FTMachine(), padding_p(1.0), nWPlanes_p(nWPlanes),
    imageCache(0), cachesize(icachesize), tilesize(itilesize),
    gridder(0), isTiled(False), 
    maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
    pointingToImage(0), usezero_p(usezero),machineName_p("WProjectFT")
{
  convSize=0;
  tangentSpecified_p=False;
  lastIndex_p=0;

  wpConvFunc_p=new WPConvFunc();

}
WProjectFT::WProjectFT(Int nWPlanes, 
		       MPosition mLocation, 
		       Long icachesize, Int itilesize, 
		       Bool usezero, Float padding)
  : FTMachine(), padding_p(padding), nWPlanes_p(nWPlanes),
    imageCache(0), cachesize(icachesize), tilesize(itilesize),
    gridder(0), isTiled(False),  
    maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
    pointingToImage(0), usezero_p(usezero), machineName_p("WProjectFT")
{
  convSize=0;
  savedWScale_p=0.0;
  tangentSpecified_p=False;
  mLocation_p=mLocation;
  lastIndex_p=0;
  wpConvFunc_p=new WPConvFunc();
  
}
WProjectFT::WProjectFT(
		       Int nWPlanes, MDirection mTangent, 
		       MPosition mLocation, 
		       Long icachesize, Int itilesize, 
		       Bool usezero, Float padding)
  : FTMachine(), padding_p(padding), nWPlanes_p(nWPlanes),
    imageCache(0), cachesize(icachesize), tilesize(itilesize),
    gridder(0), isTiled(False),  
    maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
    pointingToImage(0), usezero_p(usezero),machineName_p("WProjectFT")
{
  convSize=0;
  savedWScale_p=0.0;
  mTangent_p=mTangent;
  tangentSpecified_p=True;
  mLocation_p=mLocation;
  lastIndex_p=0;
  wpConvFunc_p=new WPConvFunc();
}

WProjectFT::WProjectFT(const RecordInterface& stateRec)
  : FTMachine(),machineName_p("WProjectFT")
{
  // Construct from the input state record
  String error;
  if (!fromRecord(error, stateRec)) {
    throw (AipsError("Failed to create WProjectFT: " + error));
  };
}
//---------------------------------------------------------------------- 
WProjectFT& WProjectFT::operator=(const WProjectFT& other)
{
  if(this!=&other) {
    nAntenna_p=other.nAntenna_p;
    mLocation_p=other.mLocation_p;
    distance_p=other.distance_p;
    lastFieldId_p=other.lastFieldId_p;
    lastMSId_p=other.lastMSId_p;
    nx=other.nx;
    ny=other.ny;
    npol=other.npol;
    nchan=other.nchan;
    nvischan=other.nvischan;
    nvispol=other.nvispol;
    chanMap.resize();
    chanMap=other.chanMap;
    polMap.resize();
    polMap=other.polMap;
    doUVWRotation_p=other.doUVWRotation_p;
    freqFrameValid_p=other.freqFrameValid_p;
    selectedSpw_p.resize();
    selectedSpw_p=other.selectedSpw_p;
    multiChanMap_p=other.multiChanMap_p;
    padding_p=other.padding_p;
    nVisChan_p.resize();
    nVisChan_p=other.nVisChan_p;
    spectralCoord_p=other.spectralCoord_p;
    doConversion_p.resize();
    doConversion_p=other.doConversion_p;
    nWPlanes_p=other.nWPlanes_p;
    imageCache=other.imageCache;
    cachesize=other.cachesize;
    tilesize=other.tilesize;
    if(other.gridder==0)
      gridder=0;
    else{
      uvScale.resize();
      uvOffset.resize();
      uvScale=other.uvScale;
      uvOffset=other.uvOffset;
      gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						     uvScale, uvOffset,
						     "SF");
    }

    isTiled=other.isTiled;
    //lattice=other.lattice;
    //arrayLattice=other.arrayLattice;
    lattice=0;
    arrayLattice=0;

    maxAbsData=other.maxAbsData;
    centerLoc=other.centerLoc;
    offsetLoc=other.offsetLoc;
    pointingToImage=other.pointingToImage;
    usezero_p=other.usezero_p;
    machineName_p=other.machineName_p;
    wpConvFunc_p=other.wpConvFunc_p;
  };
  return *this;
};

//----------------------------------------------------------------------
WProjectFT::WProjectFT(const WProjectFT& other) :machineName_p("WProjectFT")
{
  operator=(other);
}

//----------------------------------------------------------------------
void WProjectFT::init() {
  if((padding_p*padding_p*image->shape().product())>cachesize) {
    isTiled=True;
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
  }
  else {
    // We are padding.
    isTiled=False;
    CompositeNumber cn(uInt(image->shape()(0)*2));    
    nx    = cn.nextLargerEven(Int(padding_p*Float(image->shape()(0))-0.5));
    ny    = cn.nextLargerEven(Int(padding_p*Float(image->shape()(1))-0.5));   
    npol  = image->shape()(2);
    nchan = image->shape()(3);
  }
  
  //  if(image->shape().product()>cachesize) {
  //   isTiled=True;
  // }
  // else {
  // isTiled=False;
  // }
  //The Tiled version need some fixing: sof or now
  isTiled=False;

 
  sumWeight.resize(npol, nchan);
  
  wConvSize=max(1, nWPlanes_p);
  convSupport.resize(wConvSize);
  convSupport=0;

  uvScale.resize(3);
  uvScale=0.0;
  uvScale(0)=Float(nx)*image->coordinates().increment()(0); 
  uvScale(1)=Float(ny)*image->coordinates().increment()(1); 
  if(savedWScale_p==0.0){
    uvScale(2)=Float(wConvSize)*abs(image->coordinates().increment()(0));
  }
  else{
    uvScale(2)=savedWScale_p;
  }
  uvOffset.resize(3);
  uvOffset(0)=nx/2;
  uvOffset(1)=ny/2;
  uvOffset(2)=0;
  
  

  if(gridder) delete gridder; gridder=0;
  gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						 uvScale, uvOffset,
						 "SF");

  // Set up image cache needed for gridding. 
  if(imageCache) delete imageCache; imageCache=0;
  
  // The tile size should be large enough that the
  // extended convolution function can fit easily
  if(isTiled) {
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
}

// This is nasty, we should use CountedPointers here.
WProjectFT::~WProjectFT() {
  if(imageCache) delete imageCache; imageCache=0;
  if(gridder) delete gridder; gridder=0;
  /*
  if(arrayLattice) delete arrayLattice; arrayLattice=0;
  
  Int numofmodels=convFunctions_p.nelements();
  for (Int k=0; k< numofmodels; ++k){
    delete convFunctions_p[k];
    delete convSupportBlock_p[k];

  }
  */
  // convFuctions_p.resize();
  //  convSupportBlock_p.resize();

}


void WProjectFT::setConvFunc(CountedPtr<WPConvFunc>& pbconvFunc){

  wpConvFunc_p=pbconvFunc;
}
CountedPtr<WPConvFunc>& WProjectFT::getConvFunc(){

  return wpConvFunc_p;
}

void WProjectFT::findConvFunction(const ImageInterface<Complex>& image,
				const VisBuffer& vb) {
  




  wpConvFunc_p->findConvFunction(image, vb, wConvSize, uvScale, uvOffset,
				 padding_p,
				 convSampling, 
				 convFunc, convSize, convSupport, 
				 savedWScale_p); 

  uvScale(2)=savedWScale_p;

  
  
}

void WProjectFT::initializeToVis(ImageInterface<Complex>& iimage,
			       const VisBuffer& vb)
{
  image=&iimage;
  
  ok();
  
  //   if(convSize==0) {
  init();
  // }
  findConvFunction(*image, vb);

  
  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);
  
  //  nx    = image->shape()(0);
  //  ny    = image->shape()(1);
  //  npol  = image->shape()(2);
  //  nchan = image->shape()(3);

  if(image->shape().product()>cachesize) {
    isTiled=True;
  }
  else {
    isTiled=False;
  }

  isTiled=False;
  // If we are memory-based then read the image in and create an
  // ArrayLattice otherwise just use the PagedImage
  if(isTiled) {
    lattice=CountedPtr<Lattice<Complex> > (image, False);
  }
  else {
    IPosition gridShape(4, nx, ny, npol, nchan);
    griddedData.resize(gridShape);
    griddedData=Complex(0.0);
    
    IPosition stride(4, 1);
    IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
		  (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
    IPosition trc(blc+image->shape()-stride);
    
    IPosition start(4, 0);
    griddedData(blc, trc) = image->getSlice(start, image->shape());
    
    //if(arrayLattice) delete arrayLattice; arrayLattice=0;
    arrayLattice = new ArrayLattice<Complex>(griddedData);
    lattice=arrayLattice;
  }
  
  //AlwaysAssert(lattice, AipsError);
  
  logIO() << LogIO::DEBUGGING << "Starting FFT of image" << LogIO::POST;
  
  Int npixCorr=max(nx,ny);
  Vector<Float> sincConv(npixCorr);
  for (Int ix=0;ix<npixCorr;ix++) {
    Float x=C::pi*Float(ix-npixCorr/2)/(Float(npixCorr)*Float(convSampling));
    if(ix==npixCorr/2) {
      sincConv(ix)=1.0;
    }
    else {
      sincConv(ix)=sin(x)/x;
    }
  }


  Vector<Complex> correction(nx);
  correction=Complex(1.0, 0.0);
  // Do the Grid-correction
  IPosition cursorShape(4, nx, 1, 1, 1);
  IPosition axisPath(4, 0, 1, 2, 3);
  LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
  LatticeIterator<Complex> lix(*lattice, lsx);
  for(lix.reset();!lix.atEnd();lix++) {
    Int iy=lix.position()(1);
    gridder->correctX1D(correction,iy);
    for (Int ix=0;ix<nx;ix++) {
      correction(ix)/=(sincConv(ix)*sincConv(iy));
    }
    lix.rwVectorCursor()/=correction;
  }

  // Now do the FFT2D in place
  LatticeFFT::cfft2d(*lattice);
  
  logIO() << LogIO::DEBUGGING << "Finished FFT" << LogIO::POST;
  
}


void WProjectFT::initializeToVis(ImageInterface<Complex>& iimage,
			       const VisBuffer& vb,
			       Array<Complex>& griddedVis,
			       Vector<Double>& uvscale){
  
  initializeToVis(iimage, vb);
  griddedVis.assign(griddedData); //using the copy for storage
  uvscale.assign(uvScale);
  
}

void WProjectFT::finalizeToVis()
{
  if(isTiled) {
    
    logIO() << LogOrigin("WProjectFT", "finalizeToVis")  << LogIO::NORMAL;
    
    AlwaysAssert(imageCache, AipsError);
    AlwaysAssert(image, AipsError);
    ostringstream o;
    imageCache->flush();
    imageCache->showCacheStatistics(o);
    logIO() << o.str() << LogIO::POST;
  }
  if(pointingToImage) delete pointingToImage; pointingToImage=0;
}


// Initialize the FFT to the Sky. Here we have to setup and initialize the
// grid. 
void WProjectFT::initializeToSky(ImageInterface<Complex>& iimage,
			       Matrix<Float>& weight,
			       const VisBuffer& vb)
{
  // image always points to the image
  image=&iimage;
  
  //  if(convSize==0) {
  init();
  //  }
  findConvFunction(*image, vb);
  
  
  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);
  
  //  nx    = image->shape()(0);
  //  ny    = image->shape()(1);
  //  npol  = image->shape()(2);
  //  nchan = image->shape()(3);

  //  if(image->shape().product()>cachesize) {
  //  isTiled=True;
  // }
  // else {
  //  isTiled=False;
  // }
  isTiled=False;
  sumWeight=0.0;
  weight.resize(sumWeight.shape());
  weight=0.0;
  
  // Initialize for in memory or to disk gridding. lattice will
  // point to the appropriate Lattice, either the ArrayLattice for
  // in memory gridding or to the image for to disk gridding.
  if(isTiled) {
    imageCache->flush();
    image->set(Complex(0.0));
    lattice=CountedPtr<Lattice<Complex> > (image, False);
  }
  else {
    IPosition gridShape(4, nx, ny, npol, nchan);
    griddedData.resize(gridShape);
    griddedData=Complex(0.0);
    //if(arrayLattice) delete arrayLattice; arrayLattice=0;
    arrayLattice = new ArrayLattice<Complex>(griddedData);
    lattice=arrayLattice;
  }
  //AlwaysAssert(lattice, AipsError);
  
}

void WProjectFT::finalizeToSky()
{
  
  // Now we flush the cache and report statistics
  // For memory based, we don't write anything out yet.
  if(isTiled) {
    logIO() << LogOrigin("WProjectFT", "finalizeToSky")  << LogIO::NORMAL;
    
    AlwaysAssert(image, AipsError);
    AlwaysAssert(imageCache, AipsError);
    imageCache->flush();
    ostringstream o;
    imageCache->showCacheStatistics(o);
    logIO() << o.str() << LogIO::POST;
  }
  if(pointingToImage) delete pointingToImage; pointingToImage=0;
}

Array<Complex>* WProjectFT::getDataPointer(const IPosition& centerLoc2D,
					 Bool readonly) {
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
#define gwproj gwproj_
#define dwproj dwproj_
#endif

extern "C" { 
  void gwproj(const Double*,
	      Double*,
	      const Complex*,
	      Int*,
	      Int*,
	      Int*,
	      const Int*,
	      const Int*,
	      const Float*,
	      Int*,
	      Int*,
	      Double*,
	      Double*,
	      Complex*,
	      Int*,
	      Int*,
	      Int *,
	      Int *,
	      const Double*,
	      const Double*,
	      Int*,
	      Int*,
	      Int*,
	      Int*,
	      const Complex*,
	      Int*,
	      Int*,
	      Double*);
  void dwproj(const Double*,
	      Double*,
	      Complex*,
	      Int*,
	      Int*,
	      const Int*,
	      const Int*,
	      Int*,
	      Int*,
	      Double*,
	      Double*,
	      const Complex*,
	      Int*,
	      Int*,
	      Int *,
	      Int *,
	      const Double*,
	      const Double*,
	      Int*,
	      Int*,
	      Int*,
	      Int*,
	      const Complex*,
	      Int*,
	      Int*);
}
void WProjectFT::put(const VisBuffer& vb, Int row, Bool dopsf,
		     FTMachine::Type type, const Matrix<Float>& imwght )
{
  
  const Matrix<Float> *imagingweight;
  if(imwght.nelements()>0)
    imagingweight=&imwght;
  else
    imagingweight=&(vb.imagingWeight());
  Bool iswgtCopy;
  const Float *wgtStorage;
  wgtStorage=imagingweight->getStorage(iswgtCopy);

  const Cube<Complex> *data;
  if(type==FTMachine::MODEL){
    data=&(vb.modelVisCube());
  }
  else if(type==FTMachine::CORRECTED){
    data=&(vb.correctedVisCube());
  }
  else{
    data=&(vb.visCube());
  }

  Bool isCopy;
  const Complex *datStorage=data->getStorage(isCopy);


  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row==-1) {
    nRow=vb.nRow();
    startRow=0;
    endRow=nRow-1;
  } else {
    nRow=1;
    startRow=row;
    endRow=row;
  }
  
  // Get the uvws in a form that Fortran can use and do that
  // necessary phase rotation. On a Pentium Pro 200 MHz
  // when null, this step takes about 50us per uvw point. This
  // is just barely noticeable for Stokes I continuum and
  // irrelevant for other cases.
  Matrix<Double> uvw(3, vb.uvw().nelements());
  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());
  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
  }
  
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  // This is the convention for dphase ....hmmm why ?
  // dphase*=-1.0;

  
  // Take care of translation of Bools to Integer
  Int idopsf=0;
  if(dopsf) idopsf=1;
  
  Cube<Int> flags(vb.flagCube().shape());
  flags=0;
  flags(vb.flagCube())=True;
  
  Vector<Int> rowFlags(vb.nRow());
  rowFlags=0;
  rowFlags(vb.flagRow())=True;
  if(!usezero_p) {
    for (Int rownr=startRow; rownr<=endRow; rownr++) {
      if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    }
  }
  
  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())
    matchAllSpwChans(vb);
  
  //Here we redo the match or use previous match
  
  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()]){
    matchChannel(vb.spectralWindow(), vb);
  }
  else{
    chanMap.resize();
    chanMap=multiChanMap_p[vb.spectralWindow()];
  }
  
  if(isTiled) {
    
    Double invLambdaC=vb.frequency()(0)/C::c;
    Vector<Double> uvLambda(2);
    Vector<Int> centerLoc2D(2);
    centerLoc2D=0;
    
    // Loop over all rows
    for (Int rownr=startRow; rownr<=endRow; rownr++) {
      
      // Calculate uvw for this row at the center frequency
      uvLambda(0)=uvw(0,rownr)*invLambdaC;
      uvLambda(1)=uvw(1,rownr)*invLambdaC;
      centerLoc2D=gridder->location(centerLoc2D, uvLambda);
      
      // Is this point on the grid?
      if(gridder->onGrid(centerLoc2D)) {
	
	// Get the tile
	Array<Complex>* dataPtr=getDataPointer(centerLoc2D, False);
	Int aNx=dataPtr->shape()(0);
	Int aNy=dataPtr->shape()(1);
	
	// Now use FORTRAN to do the gridding. Remember to 
	// ensure that the shape and offsets of the tile are 
	// accounted for.
	Bool del;
	Vector<Double> actualOffset(3);
	for (Int i=0;i<2;i++) {
	  actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
	}
	actualOffset(2)=uvOffset(2);
	IPosition s(flags.shape());
	// Now pass all the information down to a 
	// FORTRAN routine to do the work
	gwproj(uvw.getStorage(del),
	       dphase.getStorage(del),
	       datStorage,
	       &s(0),
	       &s(1),
	       &idopsf,
	       flags.getStorage(del),
	       rowFlags.getStorage(del),
	       wgtStorage,
	       &s(2),
	       &rownr,
	       uvScale.getStorage(del),
	       actualOffset.getStorage(del),
	       dataPtr->getStorage(del),
	       &aNx,
	       &aNy,
	       &npol,
	       &nchan,
	       vb.frequency().getStorage(del),
	       &C::c,
	       convSupport.getStorage(del),
	       &convSize,
	       &convSampling,
	       &wConvSize,
	       convFunc.getStorage(del),
	       chanMap.getStorage(del),
	       polMap.getStorage(del),
	       sumWeight.getStorage(del));
      }
    }
  }
  else {
    Bool del;
    Bool uvwcopy; 
    const Double *uvwstor=uvw.getStorage(uvwcopy);
    Bool gridcopy;
    Complex *gridstor=griddedData.getStorage(gridcopy);
    Bool convcopy;
    const Complex *convstor=convFunc.getStorage(convcopy);
    IPosition s(flags.shape());
    gwproj(uvwstor,
	   dphase.getStorage(del),
	   datStorage,
	   &s(0),
	   &s(1),
	   &idopsf,
	   flags.getStorage(del),
	   rowFlags.getStorage(del),
	   wgtStorage,
	   &s(2),
	   &row,
	   uvScale.getStorage(del),
	   uvOffset.getStorage(del),
	   gridstor,
	   &nx,
	   &ny,
	   &npol,
	   &nchan,
	   vb.frequency().getStorage(del),
	   &C::c,
	   convSupport.getStorage(del),
	   &convSize,
	   &convSampling,
	   &wConvSize,
	   convstor,
	   chanMap.getStorage(del),
	   polMap.getStorage(del),
	   sumWeight.getStorage(del));
    uvw.freeStorage(uvwstor, uvwcopy);
    griddedData.putStorage(gridstor, gridcopy);
    convFunc.freeStorage(convstor, convcopy);
  }
  

  data->freeStorage(datStorage, isCopy);
  imagingweight->freeStorage(wgtStorage,iswgtCopy);
}

void WProjectFT::get(VisBuffer& vb, Int row)
{
  

  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row==-1) {
    nRow=vb.nRow();
    startRow=0;
    endRow=nRow-1;
    //vb.modelVisCube()=Complex(0.0,0.0);
  } else {
    nRow=1;
    startRow=row;
    endRow=row;
    //vb.modelVisCube().xyPlane(row)=Complex(0.0,0.0);
  }
  
  // Get the uvws in a form that Fortran can use
  Matrix<Double> uvw(3, vb.uvw().nelements());
  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());
  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
  }
  
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  // This is the convention for dphase
  // dphase*=-1.0;

  
  Cube<Int> flags(vb.flagCube().shape());
  flags=0;
  flags(vb.flagCube())=True;
  
  
  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())
    matchAllSpwChans(vb);
  //Here we redo the match or use previous match
  
  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()]){
    matchChannel(vb.spectralWindow(), vb);
  }
  else{
    chanMap.resize();
    chanMap=multiChanMap_p[vb.spectralWindow()];
  }
  
  Vector<Int> rowFlags(vb.nRow());
  rowFlags=0;
  rowFlags(vb.flagRow())=True;
  if(!usezero_p) {
    for (Int rownr=startRow; rownr<=endRow; rownr++) {
      if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    }
  }
  

  if(isTiled) {
    
    Double invLambdaC=vb.frequency()(0)/C::c;
    Vector<Double> uvLambda(2);
    Vector<Int> centerLoc2D(2);
    centerLoc2D=0;
    
    // Loop over all rows
    for (Int rownr=startRow; rownr<=endRow; rownr++) {
      
      // Calculate uvw for this row at the center frequency
      uvLambda(0)=uvw(0, rownr)*invLambdaC;
      uvLambda(1)=uvw(1, rownr)*invLambdaC;
      centerLoc2D=gridder->location(centerLoc2D, uvLambda);

      // Is this point on the grid?
      if(gridder->onGrid(centerLoc2D)) {
	
	// Get the tile
	Array<Complex>* dataPtr=getDataPointer(centerLoc2D, True);
	gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
	Int aNx=dataPtr->shape()(0);
	Int aNy=dataPtr->shape()(1);
	
	// Now use FORTRAN to do the gridding. Remember to 
	// ensure that the shape and offsets of the tile are 
	// accounted for.
	Bool del;
	Vector<Double> actualOffset(3);
	for (Int i=0;i<2;i++) {
	  actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
	}
	actualOffset(2)=uvOffset(2);
	IPosition s(vb.modelVisCube().shape());
	dwproj(uvw.getStorage(del),
	       dphase.getStorage(del),
	       vb.modelVisCube().getStorage(del),
	       &s(0),
	       &s(1),
	       flags.getStorage(del),
	       rowFlags.getStorage(del),
	       &s(2),
	       &rownr,
	       uvScale.getStorage(del),
	       actualOffset.getStorage(del),
	       dataPtr->getStorage(del),
	       &aNx,
	       &aNy,

	       &npol,
	       &nchan,
	       vb.frequency().getStorage(del),
	       &C::c,
	       convSupport.getStorage(del),
	       &convSize,
	       &convSampling,
	       &wConvSize,
	       convFunc.getStorage(del),
	       chanMap.getStorage(del),
	       polMap.getStorage(del));
      }
    }
  }
  else {
    Bool del;
    Bool isCopy;
    Complex *datStorage=vb.modelVisCube().getStorage(isCopy);
    Bool uvwcopy; 
    const Double *uvwstor=uvw.getStorage(uvwcopy);
    Bool gridcopy;
    const Complex *gridstor=griddedData.getStorage(gridcopy);
    Bool convcopy;
    const Complex *convstor=convFunc.getStorage(convcopy);
    IPosition s(vb.modelVisCube().shape());
    dwproj(uvwstor,
	   dphase.getStorage(del),
	   datStorage,
	   &s(0),
	   &s(1),
	   flags.getStorage(del),
	   rowFlags.getStorage(del),
	   &s(2),
	   &row,
	   uvScale.getStorage(del),
	   uvOffset.getStorage(del),
	   gridstor,
	   &nx,
	   &ny,
	   &npol,
	   &nchan,
	   vb.frequency().getStorage(del),
	   &C::c,
	   convSupport.getStorage(del),
	   &convSize,
	   &convSampling,
	   &wConvSize,
	   convstor,
	   chanMap.getStorage(del),
	   polMap.getStorage(del));
    vb.modelVisCube().putStorage(datStorage, isCopy);
    uvw.freeStorage(uvwstor, uvwcopy);
    griddedData.freeStorage(gridstor, gridcopy);
    convFunc.freeStorage(convstor, convcopy);
  }

 
}

void WProjectFT::get(VisBuffer& vb, Cube<Complex>& modelVis, 
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
  if (row==-1) {
    nRow=vb.nRow();
    startRow=0;
    endRow=nRow-1;
    //   modelVis.set(Complex(0.0,0.0));
  } else {
    nRow=1;
    startRow=row;
    endRow=row;
    //  modelVis.xyPlane(row)=Complex(0.0,0.0);
  }
  
  // Get the uvws in a form that Fortran can use
  Matrix<Double> uvw(3, vb.uvw().nelements());
  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());
  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
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
  if(doConversion_p[vb.spectralWindow()]){
    matchChannel(vb.spectralWindow(), vb);
  }
  else{
    chanMap.resize();
    chanMap=multiChanMap_p[vb.spectralWindow()];
  }
  
  Vector<Int> rowFlags(vb.nRow());
  rowFlags=0;
  rowFlags(vb.flagRow())=True;
  if(!usezero_p) {
    for (Int rownr=startRow; rownr<=endRow; rownr++) {
      if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    }
  }
  
  Bool del;
  IPosition s(modelVis.shape());
  dwproj(uvw.getStorage(del),
	 dphase.getStorage(del),
	 modelVis.getStorage(del),
	 &s(0),
	 &s(1),
	 flags.getStorage(del),
	 rowFlags.getStorage(del),
	 &s(2),
	 &row,
	 scale.getStorage(del),
	 offset.getStorage(del),
	 griddedVis.getStorage(del),
	 &nX,
	 &nY,
	 &npol,
	 &nchan,
	 vb.frequency().getStorage(del),
	 &C::c,
	 convSupport.getStorage(del),
	 &convSize,
	 &convSampling,
	 &wConvSize,
	 convFunc.getStorage(del),
	 chanMap.getStorage(del),
	 polMap.getStorage(del));
  
}


// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& WProjectFT::getImage(Matrix<Float>& weights,
					      Bool normalize) 
{
  //AlwaysAssert(lattice, AipsError);
  AlwaysAssert(image, AipsError);
  
  logIO() << LogOrigin("WProjectFT", "getImage") << LogIO::NORMAL;
  
  weights.resize(sumWeight.shape());
  
  convertArray(weights, sumWeight);
  
  // If the weights are all zero then we cannot normalize
  // otherwise we don't care.
  if(max(weights)==0.0) {
    if(normalize) {
      logIO() << LogIO::SEVERE
	      << "No useful data in WProjectFT: weights all zero"
	      << LogIO::POST;
    }
    else {
      logIO() << LogIO::WARN
	      << "No useful data in WProjectFT: weights all zero"
	      << LogIO::POST;
    }
  }
  else {
    
    const IPosition latticeShape = lattice->shape();
    
    logIO() << LogIO::DEBUGGING
	    << "Starting FFT and scaling of image" << LogIO::POST;
    
    // x and y transforms
    LatticeFFT::cfft2d(*lattice,False);
    
    {
      Int inx = lattice->shape()(0);
      Int iny = lattice->shape()(1);
      Vector<Complex> correction(inx);
      correction=Complex(1.0, 0.0);

      Int npixCorr= max(nx,ny);
      Vector<Float> sincConv(npixCorr);
      for (Int ix=0;ix<npixCorr;ix++) {
	Float x=C::pi*Float(ix-npixCorr/2)/(Float(npixCorr)*Float(convSampling));
	if(ix==npixCorr/2) {
	  sincConv(ix)=1.0;
	}
	else {
	  sincConv(ix)=sin(x)/x;
	}
      }

      IPosition cursorShape(4, inx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
      for(lix.reset();!lix.atEnd();lix++) {
	Int pol=lix.position()(2);
	Int chan=lix.position()(3);
	if(weights(pol, chan)>0.0) {
	  Int iy=lix.position()(1);
	  gridder->correctX1D(correction,iy);
	  for (Int ix=0;ix<nx;ix++) {
	    correction(ix)*=sincConv(ix)*sincConv(iy);
	  }
	  lix.rwVectorCursor()/=correction;
	  if(normalize) {
	    Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
	    lix.rwCursor()*=rnorm;
	  }
	  else {
	    Complex rnorm(Float(inx)*Float(iny));
	    lix.rwCursor()*=rnorm;
	  }
	}
	else {
	  lix.woCursor()=0.0;
	}
      }
    }

    if(!isTiled) {
      // Check the section from the image BEFORE converting to a lattice 
      IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
		    (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
      IPosition stride(4, 1);
      IPosition trc(blc+image->shape()-stride);
      
      // Do the copy
      image->put(griddedData(blc, trc));
      //if(arrayLattice) delete arrayLattice; arrayLattice=0;
      griddedData.resize(IPosition(1,0));
    }
  }
  
  return *image;
}

// Get weight image
void WProjectFT::getWeightImage(ImageInterface<Float>& weightImage,
			      Matrix<Float>& weights) 
{
  
  logIO() << LogOrigin("WProjectFT", "getWeightImage") << LogIO::NORMAL;
  
  weights.resize(sumWeight.shape());
  convertArray(weights,sumWeight);
  
  const IPosition latticeShape = weightImage.shape();
  
  Int inx=latticeShape(0);
  Int iny=latticeShape(1);
  
  IPosition loc(2, 0);
  IPosition cursorShape(4, inx, iny, 1, 1);
  IPosition axisPath(4, 0, 1, 2, 3);
  LatticeStepper lsx(latticeShape, cursorShape, axisPath);
  LatticeIterator<Float> lix(weightImage, lsx);
  for(lix.reset();!lix.atEnd();lix++) {
    Int pol=lix.position()(2);
    Int chan=lix.position()(3);
    lix.rwCursor()=weights(pol,chan);
  }
}

Bool WProjectFT::toRecord(String& error, RecordInterface& outRec, 
			Bool withImage) {
  
  // Save the current WProjectFT object to an output state record
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
  for (Int k=0; k<4 ; k++){
    center_loc(k)=centerLoc(k);
    offset_loc(k)=offsetLoc(k);
  }
  outRec.define("centerloc", center_loc);
  outRec.define("offsetloc", offset_loc);
  outRec.define("sumofweights", sumWeight);
  if(withImage && image){ 
    ImageInterface<Complex>& tempimage(*image);
    Record imageContainer;
    String error;
    retval = (retval || tempimage.toRecord(error, imageContainer));
    outRec.defineRecord("image", imageContainer);
  }
  return retval;
}

Bool WProjectFT::fromRecord(String& error, const RecordInterface& inRec)
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
  if(inRec.nfields() > 12 ){
    Record imageAsRec=inRec.asRecord("image");
    if(!image) { 
      image= new TempImage<Complex>(); 
    };
    String error;
    retval = (retval || image->fromRecord(error, imageAsRec));    
    
    // Might be changing the shape of sumWeight
    init(); 
    
    if(isTiled) {
      lattice=CountedPtr<Lattice<Complex> > (image, False);
    }
    else {
      // Make the grid the correct shape and turn it into an array lattice
      // Check the section from the image BEFORE converting to a lattice 
      IPosition gridShape(4, nx, ny, npol, nchan);
      griddedData.resize(gridShape);
      griddedData=Complex(0.0);
      IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
		    (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
      IPosition start(4, 0);
      IPosition stride(4, 1);
      IPosition trc(blc+image->shape()-stride);
      griddedData(blc, trc) = image->getSlice(start, image->shape());
      
      //if(arrayLattice) delete arrayLattice; arrayLattice=0;
      arrayLattice = new ArrayLattice<Complex>(griddedData);
      lattice=arrayLattice;
    }
    
    //AlwaysAssert(lattice, AipsError);
    AlwaysAssert(image, AipsError);
  };
  return retval;
}

void WProjectFT::ok() {
  AlwaysAssert(image, AipsError);
}

// Make a plain straightforward honest-to-God image. This returns
// a complex image, without conversion to Stokes. The representation
// is that required for the visibilities.
//----------------------------------------------------------------------
void WProjectFT::makeImage(FTMachine::Type type, 
			 VisSet& vs,
			 ImageInterface<Complex>& theImage,
			 Matrix<Float>& weight) {
  
  
  logIO() << LogOrigin("WProjectFT", "makeImage") << LogIO::NORMAL;
  
  if(type==FTMachine::COVERAGE) {
    logIO() << "Type COVERAGE not defined for Fourier transforms"
	    << LogIO::EXCEPTION;
  }
  
  
  // Initialize the gradients
  ROVisIter& vi(vs.iter());
  
  // Loop over all visibilities and pixels
  VisBuffer vb(vi);
  
  // Initialize put (i.e. transform to Sky) for this model
  vi.origin();
  
  if(vb.polFrame()==MSIter::Linear) {
    StokesImageUtil::changeCStokesRep(theImage, SkyModel::LINEAR);
  }
  else {
    StokesImageUtil::changeCStokesRep(theImage, SkyModel::CIRCULAR);
  }
  
  initializeToSky(theImage,weight,vb);
  
  // Loop over the visibilities, putting VisBuffers
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin(); vi.more(); vi++) {
      
      switch(type) {
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


String WProjectFT::name(){

  return machineName_p;

}



} //# NAMESPACE CASA - END

