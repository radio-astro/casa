//# MosaicFT.cc: Implementation of MosaicFT class
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
#include <synthesis/MeasurementComponents/MosaicFT.h>
#include <synthesis/MeasurementComponents/SimplePBConvFunc.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/WCBox.h>
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

namespace casa { //# NAMESPACE CASA - BEGIN

  MosaicFT::MosaicFT(SkyJones* sj, MPosition mloc, String stokes,
		   Long icachesize, Int itilesize, 
		     Bool usezero, Bool useDoublePrec)
  : FTMachine(), sj_p(sj),
    imageCache(0),  cachesize(icachesize), tilesize(itilesize), gridder(0),
    isTiled(False),
    maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
    mspc(0), msac(0), pointingToImage(0), usezero_p(usezero), convSampling(1),
    skyCoverage_p(0), machineName_p("MosaicFT"), stokes_p(stokes)
{
  convSize=0;
  tangentSpecified_p=False;
  lastIndex_p=0;
  doneWeightImage_p=False;
  convWeightImage_p=0;
  pbConvFunc_p=new SimplePBConvFunc();
    
  mLocation_p=mloc;
  useDoubleGrid_p=useDoublePrec;  
  // We should get rid of the ms dependence in the constructor
  // not used
}

MosaicFT::MosaicFT(const RecordInterface& stateRec)
  : FTMachine()
{
  // Construct from the input state record
  String error;
  if (!fromRecord(error, stateRec)) {
    throw (AipsError("Failed to create MosaicFT: " + error));
  };
}

//---------------------------------------------------------------------- 
MosaicFT& MosaicFT::operator=(const MosaicFT& other)
{
  if(this!=&other) {

    //Do the base parameters
    FTMachine::operator=(other);
     
    convSampling=other.convSampling;
    sj_p=other.sj_p;
    imageCache=other.imageCache;
    cachesize=other.cachesize;
    tilesize=other.tilesize;
    isTiled=other.isTiled;
    //lattice=other.lattice;
    lattice=0;
    // arrayLattice=other.arrayLattice;
    // weightLattice=other.weightLattice;
    //if(arrayLattice) delete arrayLattice;
    arrayLattice=0;
    //if(weightLattice) delete weightLattice;
    weightLattice=0;
    maxAbsData=other.maxAbsData;
    centerLoc=other.centerLoc;
    offsetLoc=other.offsetLoc;
    pointingToImage=other.pointingToImage;
    usezero_p=other.usezero_p;
    doneWeightImage_p=other.doneWeightImage_p;
    pbConvFunc_p=other.pbConvFunc_p;
    stokes_p=other.stokes_p;
    if(!other.skyCoverage_p.null())
      skyCoverage_p=other.skyCoverage_p;
    else
      skyCoverage_p=0;
    if(other.convWeightImage_p !=0)
      convWeightImage_p=(TempImage<Complex> *)other.convWeightImage_p->cloneII();
    else
      convWeightImage_p=0;
    if(other.gridder==0)
      gridder=0;
    else{
      uvScale=other.uvScale;
      uvOffset=other.uvOffset;
      gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						     uvScale, uvOffset,
						     "SF");
    }
    
  };
  return *this;
};

//----------------------------------------------------------------------
MosaicFT::MosaicFT(const MosaicFT& other): machineName_p("MosaicFT")
{
  operator=(other);
}

//----------------------------------------------------------------------
//void MosaicFT::setSharingFT(MosaicFT& otherFT){
//  otherFT_p=&otherFT;
//}
void MosaicFT::init() {
  
  if((image->shape().product())>cachesize) {
    isTiled=True;
  }
  else {
    isTiled=False;
  }
  //For now only isTiled False works.
  isTiled=False;
  nx    = image->shape()(0);
  ny    = image->shape()(1);
  npol  = image->shape()(2);
  nchan = image->shape()(3);


  //  if(skyCoverage_p==0){
  //    Double memoryMB=HostInfo::memoryTotal()/1024.0/(20.0);
  //    skyCoverage_p=new TempImage<Float> (IPosition(4,nx,ny,1,1),
  //					image->coordinates(), memoryMB);
    //Setting it to zero
//   (*skyCoverage_p)-=(*skyCoverage_p);
//  }
  sumWeight.resize(npol, nchan);
  
  convSupport=0;

  uvScale.resize(2);
  uvScale=0.0;
  uvScale(0)=Float(nx)*image->coordinates().increment()(0); 
  uvScale(1)=Float(ny)*image->coordinates().increment()(1); 
    
  uvOffset.resize(2);
  uvOffset(0)=nx/2;
  uvOffset(1)=ny/2;
  
  if(gridder) delete gridder; gridder=0;
  gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						 uvScale, uvOffset,
						 "SF");

  // Set up image cache needed for gridding. 
  if(imageCache) delete imageCache; imageCache=0;
  
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
MosaicFT::~MosaicFT() {
  if(imageCache) delete imageCache; imageCache=0;
  //  if(arrayLattice) delete arrayLattice; arrayLattice=0;
}


void MosaicFT::setConvFunc(CountedPtr<SimplePBConvFunc>& pbconvFunc){


  pbConvFunc_p=pbconvFunc;
  

}

CountedPtr<SimplePBConvFunc>& MosaicFT::getConvFunc(){
  return pbConvFunc_p;
}
//copy the innards to save on conv function calculation and storage
/*
void MosaicFT::copySharedInfo(){
  convFunctions_p.resize(otherFT_p->convFunctions_p.size());
  for (uInt k=0; k< otherFT_p->convFunctions_p.size(); ++k){
    convFunctions_p[k]=otherFT_p->convFunctions_p[k];
  }
  convWeights_p.resize(otherFT_p->convWeights_p.size());
  for (uInt k=0; k< otherFT_p->convWeights_p.size(); ++k)
    convWeights_p[k]=otherFT_p->convWeights_p[k];

  skyCoverage_p=otherFT_p->skyCoverage_p;

  convSupportBlock_p.resize(otherFT_p->convSupportBlock_p.size());
  for (uInt k=0; k< otherFT_p->convSupportBlock_p.size(); ++k)
    convSupportBlock_p[k]=otherFT_p->convSupportBlock_p[k];

  convFunctionMap_p=otherFT_p->convFunctionMap_p;

  convSizes_p.resize(otherFT_p->convSizes_p.size());
  for (uInt k=0; k< otherFT_p->convSizes_p.size(); ++k)
    convSizes_p[k]=otherFT_p->convSizes_p[k];

  convSampling=otherFT_p->convSampling;

}
*/

void MosaicFT::findConvFunction(const ImageInterface<Complex>& iimage,
				const VisBuffer& vb) {
  
  
  convSampling=1;
  if(pbConvFunc_p.null())
    pbConvFunc_p=new SimplePBConvFunc();
  pbConvFunc_p->setSkyJones(sj_p);
  pbConvFunc_p->findConvFunction(iimage, vb, convSampling, convFunc, weightConvFunc_p, convSizePlanes_p, convSupportPlanes_p, convRowMap_p);
  //For now only use one size and support
  convSize=max(convSizePlanes_p);
  convSupport=max(convSupportPlanes_p);
 				 
}

void MosaicFT::initializeToVis(ImageInterface<Complex>& iimage,
			       const VisBuffer& vb)
{
  image=&iimage;
  
  ok();
  
  //  if(convSize==0) {
    init();
    findConvFunction(*image, vb);
    //  }
  
  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);
  
  if((image->shape().product())>cachesize) {
    isTiled=True;
  }
  else {
    isTiled=False;
  }
  //For now isTiled=False
  isTiled=False;
  nx    = image->shape()(0);
  ny    = image->shape()(1);
  npol  = image->shape()(2);
  nchan = image->shape()(3);

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
  /*
  if(!sj_p) {

    Vector<Complex> correction(nx);
    correction=Complex(1.0, 0.0);
    // Do the Grid-correction
    IPosition cursorShape(4, nx, 1, 1, 1);
    IPosition axisPath(4, 0, 1, 2, 3);
    LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
    LatticeIterator<Complex> lix(*lattice, lsx);
    for(lix.reset();!lix.atEnd();lix++) {
      gridder->correctX1D(correction, lix.position()(1));
      lix.rwVectorCursor()/=correction;
    }
  
  }
  */
  // Now do the FFT2D in place
  LatticeFFT::cfft2d(*lattice);
  
  logIO() << LogIO::DEBUGGING << "Finished FFT" << LogIO::POST;
  
}


void MosaicFT::finalizeToVis()
{
  if(isTiled) {
    
    logIO() << LogOrigin("MosaicFT", "finalizeToVis")  << LogIO::NORMAL;
    
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
void MosaicFT::initializeToSky(ImageInterface<Complex>& iimage,
			       Matrix<Float>& weight,
			       const VisBuffer& vb)
{
  // image always points to the image
  image=&iimage;
  
  //  if(convSize==0) {
    init();
    findConvFunction(*image, vb);
    //  }
  
  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);
  
  if((image->shape().product())>cachesize) {
    isTiled=True;
  }
  else {
    isTiled=False;
  }
  //For now isTiled has to be false
  isTiled=False;
  nx    = image->shape()(0);
  ny    = image->shape()(1);
  npol  = image->shape()(2);
  nchan = image->shape()(3);

  sumWeight=0.0;
  weight.resize(sumWeight.shape());
  weight=0.0;
  
  // Initialize for in memory or to disk gridding. lattice will
  // point to the appropriate Lattice, either the ArrayLattice for
  // in memory gridding or to the image for to disk gridding.
  if(isTiled) {
    imageCache->flush();
    image->set(Complex(0.0));
    lattice=CountedPtr<Lattice<Complex> >(image, False);
    if( !doneWeightImage_p && (convWeightImage_p==0)){
      
      convWeightImage_p=new  TempImage<Complex> (iimage.shape(), 
						 iimage.coordinates());




      convWeightImage_p->set(Complex(0.0));
      weightLattice=convWeightImage_p;

    }
  }
  else {
    IPosition gridShape(4, nx, ny, npol, nchan);
    griddedData.resize(gridShape);
    if(useDoubleGrid_p){
      griddedData2.resize(gridShape);
      griddedData2=DComplex(0.0);
    }
    else{
      griddedData=Complex(0.0);
    }
    //if(arrayLattice) delete arrayLattice; arrayLattice=0;
    arrayLattice = new ArrayLattice<Complex>(griddedData);
    lattice=arrayLattice;
      
    if( !doneWeightImage_p && (convWeightImage_p==0)){
     
      
 
      convWeightImage_p=new  TempImage<Complex> (iimage.shape(), 
						 iimage.coordinates());
      griddedWeight.resize(gridShape);
      /*IPosition stride(4, 1);
      IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
		    (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
      IPosition trc(blc+image->shape()-stride);
      
      griddedWeight(blc, trc).set(Complex(0.0));
      */
      if(useDoubleGrid_p){
	griddedWeight2.resize(gridShape);
	griddedData2=DComplex(0.0);
      }
      else{
	griddedWeight=Complex(0.0);
      }
      //if(weightLattice) delete weightLattice; weightLattice=0;
      weightLattice = new ArrayLattice<Complex>(griddedWeight);

    }

  }
  // AlwaysAssert(lattice, AipsError);
  
}

void MosaicFT::reset(){

  doneWeightImage_p=False;

}

void MosaicFT::finalizeToSky()
{
  
  // Now we flush the cache and report statistics
  // For memory based, we don't write anything out yet.
  if(isTiled) {
    logIO() << LogOrigin("MosaicFT", "finalizeToSky")  << LogIO::NORMAL;
    
    AlwaysAssert(image, AipsError);
    AlwaysAssert(imageCache, AipsError);
    imageCache->flush();
    ostringstream o;
    imageCache->showCacheStatistics(o);
    logIO() << o.str() << LogIO::POST;
  }

  

  if(!doneWeightImage_p){
     if(useDoubleGrid_p){
      convertArray(griddedWeight, griddedWeight2);
      //Don't need the double-prec grid anymore...
      griddedWeight2.resize();
    }

    LatticeFFT::cfft2d(*weightLattice, False);
    //Get the stokes right
    CoordinateSystem coords=convWeightImage_p->coordinates();
    Int stokesIndex=coords.findCoordinate(Coordinate::STOKES);
    Int npol=1;
    Vector<Int> whichStokes(npol);
    if(stokes_p=="I" || stokes_p=="RR" || stokes_p=="LL" ||stokes_p=="XX" 
       || stokes_p=="YY"){
      npol=1;
      whichStokes(0)=Stokes::type(stokes_p);
    }
    else if(stokes_p=="IV"){
      npol=2;
      whichStokes.resize(2);
      whichStokes(0)=Stokes::I;
      whichStokes(1)=Stokes::V;
    }
    else if(stokes_p=="QU"){
      npol=2;
      whichStokes.resize(2);
      whichStokes(0)=Stokes::Q;
      whichStokes(1)=Stokes::U;
    }
    else if(stokes_p=="RRLL"){
      npol=2;
      whichStokes.resize(2);
      whichStokes(0)=Stokes::RR;
      whichStokes(1)=Stokes::LL;
    }   
    else if(stokes_p=="XXYY"){
      npol=2;
      whichStokes.resize(2);
      whichStokes(0)=Stokes::XX;
      whichStokes(1)=Stokes::YY;
    }  
    else if(stokes_p=="IQU"){
      npol=3;
      whichStokes.resize(3);
      whichStokes(0)=Stokes::I;
      whichStokes(1)=Stokes::Q;
      whichStokes(2)=Stokes::U;
    }
    else if(stokes_p=="IQUV"){
      npol=4;
      whichStokes.resize(4);
      whichStokes(0)=Stokes::I;
      whichStokes(1)=Stokes::Q;
      whichStokes(2)=Stokes::U;
      whichStokes(3)=Stokes::V;
    } 
    
    StokesCoordinate newStokesCoord(whichStokes);
    coords.replaceCoordinate(newStokesCoord, stokesIndex);
    IPosition imshp=convWeightImage_p->shape();
    imshp(2)=npol;


    skyCoverage_p=new TempImage<Float> (imshp, coords);
    IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2,
		    (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
    IPosition stride(4, 1);
    IPosition trc(blc+image->shape()-stride);
    
    // Do the copy
    IPosition start(4, 0);
    convWeightImage_p->put(griddedWeight(blc, trc));
    StokesImageUtil::To(*skyCoverage_p, *convWeightImage_p);
    if(npol>1){
      // only the I get it right Q and U or V may end up with zero depending 
      // if RR or XX
      blc(0)=0; blc(1)=0; blc(3)=0;blc(2)=0;
      trc=skyCoverage_p->shape()-stride;
      trc(2)=0;
      SubImage<Float> isubim(*skyCoverage_p, Slicer(blc, trc, Slicer::endIsLast));
      for (Int k=1; k < npol; ++k){
	blc(2)=k; trc(2)=k;
	SubImage<Float> quvsubim(*skyCoverage_p, Slicer(blc, trc, Slicer::endIsLast), True);
	quvsubim.copyData(isubim);
      }

    }
    //Store this image in the pbconvfunc object as
    //it can be used for rescaling or shared by other ftmachines that use
    //this pbconvfunc
    pbConvFunc_p->setWeightImage(skyCoverage_p);
    delete convWeightImage_p;
    convWeightImage_p=0;
    doneWeightImage_p=True;

    if(1){
      PagedImage<Float> thisScreen(skyCoverage_p->shape(), 
				   skyCoverage_p->coordinates(), "Screen");
      thisScreen.copyData(*skyCoverage_p);
    }
    
  }

  if(pointingToImage) delete pointingToImage; pointingToImage=0;
}

Array<Complex>* MosaicFT::getDataPointer(const IPosition& centerLoc2D,
					 Bool readonly) {
  Array<Complex>* result;
  // Is tiled: get tiles and set up offsets
  centerLoc(0)=centerLoc2D(0);
  centerLoc(1)=centerLoc2D(1);
  result=&imageCache->tile(offsetLoc,centerLoc, readonly);
  gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
  return result;
}

#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define gmoss gmoss_
#define gmosd gmosd_
#define dmos dmos_
#endif

extern "C" { 
  void gmosd(const Double*,
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
	      DComplex*,
	      Int*,
	      Int*,
	      Int *,
	      Int *,
	      const Double*,
	      const Double*,
	      Int*,
	      Int*,
	      Int*,
	      const Complex*,
	      Int*,
	      Int*,
	      Double*,
	      DComplex*,
	      Complex*,
	      Int*,
              Int*,
              Int*);
  void gmoss(const Double*,
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
	      const Complex*,
	      Int*,
	      Int*,
	      Double*,
	      Complex*,
	      Complex*,
	      Int*,
              Int*,
              Int*);
  void dmos(const Double*,
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
	      const Complex*,
	      Int*,
	      Int*,
              Int*,
              Int*);
}
void MosaicFT::put(const VisBuffer& vb, Int row, Bool dopsf,
		   FTMachine::Type type,
		   const Matrix<Float>& imwght)
{


  
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

  //No point in reading data if its not matching in frequency
  if(max(chanMap)==-1)
    return;


  findConvFunction(*image, vb);

  const Matrix<Float> *imagingweight;
  if(imwght.nelements()>0){
    imagingweight=&imwght;
  }
  else{
    imagingweight=&(vb.imagingWeight());
  }
  


  if(dopsf) type=FTMachine::PSF;

  Cube<Complex> data;
  //Fortran gridder need the flag as ints 
  Cube<Int> flags;
  Matrix<Float> elWeight;
  interpolateFrequencyTogrid(vb, *imagingweight,data, flags, elWeight, type);


  Bool iswgtCopy;
  const Float *wgtStorage;
  wgtStorage=elWeight.getStorage(iswgtCopy);


  Int nConvFunc=convFunc.shape()(2);

  Bool isCopy;
  const Complex *datStorage=0;

  if(!dopsf)
    datStorage=data.getStorage(isCopy);
    
  
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
  
  doUVWRotation_p=True;
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  // Get the pointing positions. This can easily consume a lot 
  // of time thus we are for now assuming a field per 
  // vb chunk...need to change that accordingly if we start using
  // multiple pointings per vb.
  //Warning 

  // Take care of translation of Bools to Integer
  Int idopsf=0;
  if(dopsf) idopsf=1;
  
  
  Vector<Int> rowFlags(vb.nRow());
  rowFlags=0;
  rowFlags(vb.flagRow())=True;
  if(!usezero_p) {
    for (Int rownr=startRow; rownr<=endRow; rownr++) {
      if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    }
  }
  
  


  //Tell the gridder to grid the weights too ...need to do that once only
  Int doWeightGridding=1;
  if(doneWeightImage_p)
    doWeightGridding=-1;
  Bool del;
  //    IPosition s(flags.shape());
  const IPosition& fs=flags.shape();
  std::vector<Int>s(fs.begin(), fs.end());
  Bool uvwcopy; 
  const Double *uvwstor=uvw.getStorage(uvwcopy);
  Bool gridcopy;
  Bool convcopy;
  const Complex *convstor=convFunc.getStorage(convcopy);
  Bool weightcopy;
  if(useDoubleGrid_p) {
    DComplex *gridstor=griddedData2.getStorage(gridcopy);
    DComplex *gridwgtstor=griddedWeight2.getStorage(weightcopy);
    gmosd(uvwstor,
	  dphase.getStorage(del),
	   datStorage,
	   &s[0],
	   &s[1],
	   &idopsf,
	   flags.getStorage(del),
	   rowFlags.getStorage(del),
	   wgtStorage,
	   &s[2],
	   &row,
	   uvScale.getStorage(del),
	   uvOffset.getStorage(del),
	   gridstor,
	   &nx,
	   &ny,
	   &npol,
	   &nchan,
	   interpVisFreq_p.getStorage(del),
	   &C::c,
	   &convSupport,
	   &convSize,
	   &convSampling,
	   convstor,
	   chanMap.getStorage(del),
	   polMap.getStorage(del),
	   sumWeight.getStorage(del),
	   gridwgtstor,
	   weightConvFunc_p.getStorage(del),
	   &doWeightGridding,
	   convRowMap_p.getStorage(del),
	   &nConvFunc
	   );
    
    griddedData2.putStorage(gridstor, gridcopy);
    griddedWeight2.putStorage(gridwgtstor, weightcopy);
    
  }
  else {
    Complex *gridstor=griddedData.getStorage(gridcopy);
    Complex *gridwgtstor=griddedWeight.getStorage(weightcopy);
    gmoss(uvwstor,
	   dphase.getStorage(del),
	   datStorage,
	   &s[0],
	   &s[1],
	   &idopsf,
	   flags.getStorage(del),
	   rowFlags.getStorage(del),
	   wgtStorage,
	   &s[2],
	   &row,
	   uvScale.getStorage(del),
	   uvOffset.getStorage(del),
	   gridstor,
	   &nx,
	   &ny,
	   &npol,
	   &nchan,
	   interpVisFreq_p.getStorage(del),
	   &C::c,
	   &convSupport,
	   &convSize,
	   &convSampling,
	   convstor,
	   chanMap.getStorage(del),
	   polMap.getStorage(del),
	   sumWeight.getStorage(del),
	   gridwgtstor,
	   weightConvFunc_p.getStorage(del),
	   &doWeightGridding,
	   convRowMap_p.getStorage(del),
	   &nConvFunc
	   );
   
    griddedData.putStorage(gridstor, gridcopy);
    griddedWeight.putStorage(gridwgtstor, weightcopy);
  }
  convFunc.freeStorage(convstor, convcopy);
  uvw.freeStorage(uvwstor, uvwcopy);
  if(!dopsf)
    data.freeStorage(datStorage, isCopy);
  elWeight.freeStorage(wgtStorage,iswgtCopy);



}

void MosaicFT::get(VisBuffer& vb, Int row)
{
  

  findConvFunction(*image, vb);
  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row==-1) {
    nRow=vb.nRow();
    startRow=0;
    endRow=nRow-1;
    //  vb.modelVisCube()=Complex(0.0,0.0);
  } else {
    nRow=1;
    startRow=row;
    endRow=row;
    //  vb.modelVisCube().xyPlane(row)=Complex(0.0,0.0);
  }
  

  Int nConvFunc=convFunc.shape()(2);


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
  
  doUVWRotation_p=True;
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);
  
  
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
  //No point in reading data if its not matching in frequency
  if(max(chanMap)==-1)
    return;

  Cube<Complex> data;
  Cube<Int> flags;
  getInterpolateArrays(vb, data, flags);

  Complex *datStorage;
  Bool isCopy;
  datStorage=data.getStorage(isCopy);


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
      Int aNx=dataPtr->shape()(0);
      Int aNy=dataPtr->shape()(1);
      
      // Now use FORTRAN to do the gridding. Remember to 
      // ensure that the shape and offsets of the tile are 
      // accounted for.
      Bool del;
      Vector<Double> actualOffset(2);
      for (Int i=0;i<2;i++) {
	actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
      }
      //      IPosition s(data.shape());
      const IPosition& fs=data.shape();
      std::vector<Int> s(fs.begin(), fs.end());

      dmos(uvw.getStorage(del),
	     dphase.getStorage(del),
	     datStorage,
	     &s[0],
	     &s[1],
	     flags.getStorage(del),
	     rowFlags.getStorage(del),
	     &s[2],
	     &rownr,
	     uvScale.getStorage(del),
	     actualOffset.getStorage(del),
	     dataPtr->getStorage(del),
	     &aNx,
	     &aNy,
	     &npol,
	     &nchan,
	     interpVisFreq_p.getStorage(del),
	     &C::c,
	     &convSupport,
	     &convSize,
	     &convSampling,
	     convFunc.getStorage(del),
	     chanMap.getStorage(del),
	     polMap.getStorage(del),
	     convRowMap_p.getStorage(del),
	     &nConvFunc);
      }
    }
  }
  else {
    Bool del;
     Bool uvwcopy; 
     const Double *uvwstor=uvw.getStorage(uvwcopy);
     Bool gridcopy;
     const Complex *gridstor=griddedData.getStorage(gridcopy);
     Bool convcopy;
     const Complex *convstor=convFunc.getStorage(convcopy);
     //     IPosition s(data.shape());
     const IPosition& fs=data.shape();
     std::vector<Int> s(fs.begin(), fs.end());
     dmos(uvwstor,
	    dphase.getStorage(del),
	    datStorage,
	    &s[0],
	    &s[1],
	    flags.getStorage(del),
	    rowFlags.getStorage(del),
	    &s[2],
	    &row,
	    uvScale.getStorage(del),
	    uvOffset.getStorage(del),
	    gridstor,
	    &nx,
	    &ny,
	    &npol,
	    &nchan,
	    interpVisFreq_p.getStorage(del),
	    &C::c,
	    &convSupport,
	    &convSize,
	    &convSampling,
	    convstor,
	    chanMap.getStorage(del),
	    polMap.getStorage(del),
	    convRowMap_p.getStorage(del),
	    &nConvFunc);
     data.putStorage(datStorage, isCopy);
     uvw.freeStorage(uvwstor, uvwcopy);
     griddedData.freeStorage(gridstor, gridcopy);
     convFunc.freeStorage(convstor, convcopy);
  }
  interpolateFrequencyFromgrid(vb, data, FTMachine::MODEL);
}



// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& MosaicFT::getImage(Matrix<Float>& weights,
					    Bool normalize) 
{
  //AlwaysAssert(lattice, AipsError);
  AlwaysAssert(image, AipsError);
  
  logIO() << LogOrigin("MosaicFT", "getImage") << LogIO::NORMAL;
  
  weights.resize(sumWeight.shape());
  
  convertArray(weights, sumWeight);
  
  // If the weights are all zero then we cannot normalize
  // otherwise we don't care.
  if(max(weights)==0.0) {
    if(normalize) {
      logIO() << LogIO::SEVERE << "No useful data in MosaicFT: weights all zero"
	      << LogIO::POST;
    }
    else {
      logIO() << LogIO::WARN << "No useful data in MosaicFT: weights all zero"
	      << LogIO::POST;
    }
  }
  else {
    
    const IPosition latticeShape = lattice->shape();
    
    logIO() << LogIO::DEBUGGING
	    << "Starting FFT and scaling of image" << LogIO::POST;
    if(useDoubleGrid_p){
      convertArray(griddedData, griddedData2);
      //Don't need the double-prec grid anymore...
      griddedData2.resize();
    }
    // x and y transforms
    LatticeFFT::cfft2d(*lattice,False);
    
    {
      Int inx = lattice->shape()(0);
      Int iny = lattice->shape()(1);
      Vector<Complex> correction(inx);
      correction=Complex(1.0, 0.0);

      IPosition cursorShape(4, inx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
      for(lix.reset();!lix.atEnd();lix++) {
	Int pol=lix.position()(2);
	Int chan=lix.position()(3);
	if(weights(pol, chan)!=0.0) {
	  /*
	  if(!sj_p) {
	    gridder->correctX1D(correction, lix.position()(1));
	    lix.rwVectorCursor()/=correction;
	  }
	  */
	  if(normalize) {
	    Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
	    lix.rwCursor()*=rnorm;
	  }
	  //	  else {
	  //	    Complex rnorm(Float(inx)*Float(iny));
	  //	    lix.rwCursor()*=rnorm;
	  //	  }
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
      IPosition start(4, 0);
      image->put(griddedData(blc, trc));
    }
  }
  
  return *image;
}

// Get weight image
void MosaicFT::getWeightImage(ImageInterface<Float>& weightImage,
			      Matrix<Float>& weights) 
{
  
  logIO() << LogOrigin("MosaicFT", "getWeightImage") << LogIO::NORMAL;
  
  weights.resize(sumWeight.shape());
  convertArray(weights,sumWeight);
  /*
  weightImage.copyData((LatticeExpr<Float>) 
		       (iif((pbConvFunc_p->getFluxScaleImage()) > (0.0), 
			    (*skyCoverage_p),0.0)));
  */
  weightImage.copyData(*skyCoverage_p);

 

}

void MosaicFT::getFluxImage(ImageInterface<Float>& fluxImage) {

  IPosition inShape=(pbConvFunc_p->getFluxScaleImage()).shape();
  IPosition outShape=fluxImage.shape();
  if(outShape==inShape){
    fluxImage.copyData(pbConvFunc_p->getFluxScaleImage());
  }
  else if((outShape(0)==inShape(0)) && (outShape(1)==inShape(1)) 
	  && (outShape(2)==inShape(2))){
    //case where CubeSkyEquation is chunking...copy the first pol-cube
    IPosition cursorShape(4, inShape(0), inShape(1), inShape(2), 1);
    IPosition axisPath(4, 0, 1, 2, 3);
    LatticeStepper lsout(outShape, cursorShape, axisPath);
    LatticeStepper lsin(inShape, cursorShape, axisPath);
    LatticeIterator<Float> liout(fluxImage, lsout);
    RO_LatticeIterator<Float> liin(pbConvFunc_p->getFluxScaleImage(), lsin);
    liin.reset();
    for(liout.reset();!liout.atEnd();liout++) {
      if(inShape(2)==1)
	liout.woMatrixCursor()=liin.matrixCursor();
      else
	liout.woCubeCursor()=liin.cubeCursor();
    }


  }
  else{
    //Should not reach here but the we're getting old
    cout << "Bad case of shape mismatch in flux image shape" << endl;
  }

}

CountedPtr<TempImage<Float> >& MosaicFT::getConvWeightImage(){
  if(!doneWeightImage_p)
    finalizeToSky();
  return skyCoverage_p;
}
Bool MosaicFT::toRecord(String& error, RecordInterface& outRec, 
			Bool withImage) {
  
  // Save the current MosaicFT object to an output state record
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

Bool MosaicFT::fromRecord(String& error, const RecordInterface& inRec)
{
  Bool retval = True;
  imageCache=0; lattice=0; arrayLattice=0;
  Double cacheVal;
  inRec.get("cache", cacheVal);
  cachesize=(Long)cacheVal;
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

void MosaicFT::ok() {
  AlwaysAssert(image, AipsError);
}

// Make a plain straightforward honest-to-God image. This returns
// a complex image, without conversion to Stokes. The representation
// is that required for the visibilities.
//----------------------------------------------------------------------
void MosaicFT::makeImage(FTMachine::Type type, 
			 VisSet& vs,
			 ImageInterface<Complex>& theImage,
			 Matrix<Float>& weight) {
  
  
  logIO() << LogOrigin("MosaicFT", "makeImage") << LogIO::NORMAL;
  
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

Bool MosaicFT::getXYPos(const VisBuffer& vb, Int row) {
  
  
  const ROMSPointingColumns& act_mspc=vb.msColumns().pointing();
  uInt pointIndex=getIndex(act_mspc, vb.time()(row), vb.timeInterval()(row));
  if((pointIndex<0)||(pointIndex>=act_mspc.time().nrow())) {
    //    ostringstream o;
    //    o << "Failed to find pointing information for time " <<
    //      MVTime(vb.time()(row)/86400.0);
    //    logIO_p << LogIO::DEBUGGING << String(o) << LogIO::POST;
    //    logIO_p << String(o) << LogIO::POST;
    
    worldPosMeas = vb.msColumns().field().phaseDirMeas(vb.fieldId());
  }
  else {
   
      worldPosMeas=act_mspc.directionMeas(pointIndex);
      // Make a machine to convert from the worldPosMeas to the output
      // Direction Measure type for the relevant frame
 

 
  }

  if(!pointingToImage) {
    // Set the frame - choose the first antenna. For e.g. VLBI, we
    // will need to reset the frame per antenna
    FTMachine::mLocation_p=vb.msColumns().antenna().positionMeas()(0);
    mFrame_p=MeasFrame(MEpoch(Quantity(vb.time()(row), "s")),
		       FTMachine::mLocation_p);
    MDirection::Ref outRef(directionCoord.directionType(), mFrame_p);
    pointingToImage = new MDirection::Convert(worldPosMeas, outRef);
  
    if(!pointingToImage) {
      logIO_p << "Cannot make direction conversion machine" << LogIO::EXCEPTION;
    }
  }
  else {
    mFrame_p.resetEpoch(MEpoch(Quantity(vb.time()(row), "s")));
  }
  
  worldPosMeas=(*pointingToImage)(worldPosMeas);
 
  Bool result=directionCoord.toPixel(xyPos, worldPosMeas);
  if(!result) {
    logIO_p << "Failed to find pixel location for " 
	    << worldPosMeas.getAngle().getValue() << LogIO::EXCEPTION;
    return False;
  }
  return result;
  
}
// Get the index into the pointing table for this time. Note that the 
// in the pointing table, TIME specifies the beginning of the spanned
// time range, whereas for the main table, TIME is the centroid.
// Note that the behavior for multiple matches is not specified! i.e.
// if there are multiple matches, the index returned depends on the
// history of previous matches. It is deterministic but not obvious.
// One could cure this by searching but it would be considerably
// costlier.
Int MosaicFT::getIndex(const ROMSPointingColumns& mspc, const Double& time,
		       const Double& interval) {
  Int start=lastIndex_p;
  // Search forwards
  Int nrows=mspc.time().nrow();
  if(nrows<1) {
    //    logIO_p << "No rows in POINTING table - cannot proceed" << LogIO::EXCEPTION;
    return -1;
  }
  for (Int i=start;i<nrows;i++) {
    Double midpoint = mspc.time()(i); // time in POINTING table is midpoint
    // If the interval in the pointing table is negative, use the last
    // entry. Note that this may be invalid (-1) but in that case 
    // the calling routine will generate an error
    if(mspc.interval()(i)<0.0) {
      return lastIndex_p;
    }
    // Pointing table interval is specified so we have to do a match
    else {
      // Is the midpoint of this pointing table entry within the specified
      // tolerance of the main table entry?
      if(abs(midpoint-time) < (mspc.interval()(i)/2.0)) {
	lastIndex_p=i;
	return i;
      }
    }
  }
  // Search backwards
  for (Int i=start;i>=0;i--) {
    Double midpoint = mspc.time()(i); // time in POINTING table is midpoint
    if(mspc.interval()(i)<0.0) {
      return lastIndex_p;
    }
    // Pointing table interval is specified so we have to do a match
    else {
      // Is the midpoint of this pointing table entry within the specified
      // tolerance of the main table entry?
      if(abs(midpoint-time) < (mspc.interval()(i)/2.0)) {
	lastIndex_p=i;
	return i;
      }
    }
  }
  // No match!
  return -1;
}




void MosaicFT::addBeamCoverage(ImageInterface<Complex>& pbImage){

  CoordinateSystem cs(pbImage.coordinates());
  //  IPosition blc(4,0,0,0,0);
  //  IPosition trc(pbImage.shape());
  //  trc(0)=trc(0)-1;
  //  trc(1)=trc(1)-1;
  // trc(2)=0;
  //  trc(3)=0;
  WCBox *wbox= new WCBox(LCBox(pbImage.shape()), cs);
  SubImage<Float> toAddTo(*skyCoverage_p, ImageRegion(wbox), True);
  TempImage<Float> beamStokes(pbImage.shape(), cs);
  StokesImageUtil::To(beamStokes, pbImage);
  //  toAddTo.copyData((LatticeExpr<Float>)(toAddTo + beamStokes ));
  skyCoverage_p->copyData((LatticeExpr<Float>)(*skyCoverage_p + beamStokes ));


}

String MosaicFT::name(){
  return machineName_p;
}

} //# NAMESPACE CASA - END

