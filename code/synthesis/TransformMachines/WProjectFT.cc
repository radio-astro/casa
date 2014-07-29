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
#include <synthesis/TransformMachines/WProjectFT.h>
#include <synthesis/TransformMachines/WPConvFunc.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
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
#ifdef HAS_OMP
#include <omp.h>
#endif
namespace casa { //# NAMESPACE CASA - BEGIN

WProjectFT::WProjectFT( Int nWPlanes, Long icachesize, Int itilesize, 
			Bool usezero, Bool useDoublePrec)
  : FTMachine(), padding_p(1.0), nWPlanes_p(nWPlanes),
    imageCache(0), cachesize(icachesize), tilesize(itilesize),
    gridder(0), isTiled(False), 
    maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)), usezero_p(usezero), 
    machineName_p("WProjectFT")
{
  convSize=0;
  tangentSpecified_p=False;
  lastIndex_p=0;
  useDoubleGrid_p=useDoublePrec;
  wpConvFunc_p=new WPConvFunc();

}
WProjectFT::WProjectFT(Int nWPlanes, 
		       MPosition mLocation, 
		       Long icachesize, Int itilesize, 
		       Bool usezero, Float padding, Bool useDoublePrec)
  : FTMachine(), padding_p(padding), nWPlanes_p(nWPlanes),
    imageCache(0), cachesize(icachesize), tilesize(itilesize),
    gridder(0), isTiled(False),  
    maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
    usezero_p(usezero),  
    machineName_p("WProjectFT")
{
  convSize=0;
  savedWScale_p=0.0;
  tangentSpecified_p=False;
  mLocation_p=mLocation;
  lastIndex_p=0;
  wpConvFunc_p=new WPConvFunc();
  useDoubleGrid_p=useDoublePrec;
}
WProjectFT::WProjectFT(
		       Int nWPlanes, MDirection mTangent, 
		       MPosition mLocation, 
		       Long icachesize, Int itilesize, 
		       Bool usezero, Float padding, Bool useDoublePrec)
  : FTMachine(), padding_p(padding), nWPlanes_p(nWPlanes),
    imageCache(0), cachesize(icachesize), tilesize(itilesize),
    gridder(0), isTiled(False),  
    maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
    usezero_p(usezero), 
    machineName_p("WProjectFT")
{
  convSize=0;
  savedWScale_p=0.0;
  mTangent_p=mTangent;
  tangentSpecified_p=True;
  mLocation_p=mLocation;
  lastIndex_p=0;
  wpConvFunc_p=new WPConvFunc();
  useDoubleGrid_p=useDoublePrec;
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
    //Do the base parameters
    FTMachine::operator=(other);
    
    
    padding_p=other.padding_p;
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
    usezero_p=other.usezero_p;
    machineName_p=other.machineName_p;
    wpConvFunc_p=other.wpConvFunc_p;
  };
  return *this;
};

//----------------------------------------------------------------------
  WProjectFT::WProjectFT(const WProjectFT& other) :FTMachine(), machineName_p("WProjectFT")
{
  operator=(other);
}

FTMachine* WProjectFT::cloneFTM(){
  return new WProjectFT(*this);
}

//----------------------------------------------------------------------
void WProjectFT::init() {
  /*  if((padding_p*padding_p*image->shape().product())>cachesize) {
    isTiled=True;
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
  }
  else {*/
    // We are padding.
    isTiled=False;
    CompositeNumber cn(uInt(image->shape()(0)*2));    
    nx    = cn.nextLargerEven(Int(padding_p*Float(image->shape()(0))-0.5));
    ny    = cn.nextLargerEven(Int(padding_p*Float(image->shape()(1))-0.5));   
    npol  = image->shape()(2);
    nchan = image->shape()(3);
    //}
  
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
  toVis_p=True;
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


  isTiled=False;
  // If we are memory-based then read the image in and create an
  // ArrayLattice otherwise just use the PagedImage
  /*if(isTiled) {
    lattice=CountedPtr<Lattice<Complex> > (image, False);
  }
  else {
   }
  */
  //AlwaysAssert(lattice, AipsError);
  
  prepGridForDegrid();
}

void WProjectFT::prepGridForDegrid(){
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


void WProjectFT::finalizeToVis()
{
  griddedData.resize();
  if(isTiled) {
    
    logIO() << LogOrigin("WProjectFT", "finalizeToVis")  << LogIO::NORMAL;
    
    AlwaysAssert(imageCache, AipsError);
    AlwaysAssert(image, AipsError);
    ostringstream o;
    imageCache->flush();
    imageCache->showCacheStatistics(o);
    logIO() << o.str() << LogIO::POST;
  }
}


// Initialize the FFT to the Sky. Here we have to setup and initialize the
// grid. 
void WProjectFT::initializeToSky(ImageInterface<Complex>& iimage,
			       Matrix<Float>& weight,
			       const VisBuffer& vb)
{
  // image always points to the image
  image=&iimage;
  toVis_p=False;
  
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
    if(useDoubleGrid_p){
      griddedData.resize();
      griddedData2.resize(gridShape);
      griddedData2=DComplex(0.0);
    }
    //if(arrayLattice) delete arrayLattice; arrayLattice=0;
   
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
#define gwgrid gwgrid_
#define gwproj gwproj_
#define dwproj dwproj_
#define sectgwgridd sectgwgridd_
#define sectgwgrids sectgwgrids_
#define sectdwgrid sectdwgrid_
#define locuvw locuvw_
#endif

extern "C" { 
  void locuvw(const Double*, const Double*, const Double*, const Int*, const Double*, const Double*, const Int*, 
	      Int*, Int*, Complex*, const Int*, const Int*, const Double*);
  //Double precision gridding
  void gwgrid(const Double*,
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
	      Int*,
	      const Complex*,
	      Int*,
	      Int*,
	      Double*);

  void sectgwgridd(const Double*,
		   const Complex*,
	      const Int*,
	      const Int*,
	      const Int*,
	      const Int*,
	      const Int*,
	      const Float*,
	      const Int*,
	      DComplex*,
	      const Int*,
	      const Int*,
	      const Int *,
	      const Int *,
		   //support
	      const Int*,
	      const Int*,
	      const Int*,
	      const Int*,
	      const Complex*,
	      const Int*,
	      const Int*,
		   Double*,
		   //x0
		   const Int*,
		   const Int*,
		   const Int*, 
		   const Int*, 
		   const Int*, 
		   const Int*,
		   const Int*,
		   const Int*,
		   const Complex*
		   );

  //Single precision gridding
    void sectgwgrids(const Double*,
		   const Complex*,
	      const Int*,
	      const Int*,
	      const Int*,
	      const Int*,
	      const Int*,
	      const Float*,
	      const Int*,
	      Complex*,
	      const Int*,
	      const Int*,
	      const Int *,
	      const Int *,
		   //support
	      const Int*,
	      const Int*,
	      const Int*,
	      const Int*,
	      const Complex*,
	      const Int*,
	      const Int*,
		   Double*,
		   //x0
		   const Int*,
		   const Int*,
		   const Int*, 
		   const Int*, 
		   const Int*, 
		   const Int*,
		   const Int*,
		   const Int*,
		   const Complex*
		   );


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

  void sectdwgrid(const Double*,
		  Complex*,
		  const Int*,
		  const Int*,
		  const Int*,
		  const Int*,
		  const Int*,
		  const Complex*,
		  const Int*,
		  const Int*,
		  const Int *,
		  const Int *,
		  //support
		  const Int*,
		  const Int*,
		  const Int*,
		  const Int*,
		  const Complex*,
		  const Int*,
		  const Int*,
		  //rbeg, rend, loc, off, phasor
		  const Int*,
		  const Int*,
		  const Int*,
		  const Int*,
		  const Complex*);
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
		     FTMachine::Type type)
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


  const Matrix<Float> *imagingweight;
  imagingweight=&(vb.imagingWeight());

  if(dopsf) type=FTMachine::PSF;

  Cube<Complex> data;
  //Fortran gridder need the flag as ints 
  Cube<Int> flags;
  Matrix<Float> elWeight;
  interpolateFrequencyTogrid(vb, *imagingweight,data, flags, elWeight, type);
  
  
  Bool iswgtCopy;
  const Float *wgtStorage;
  wgtStorage=elWeight.getStorage(iswgtCopy);


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
  
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);


  
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
  
  
  Bool del;
  //    IPosition s(flags.shape());
  Vector<Int> s(flags.shape().nelements());
  convertArray(s, flags.shape().asVector());
  Int nvp=s[0];
  Int nvc=s[1];
  Int nvisrow=s[2];

  Int csamp=convSampling;
  
  Bool uvwcopy; 
  const Double *uvwstor=uvw.getStorage(uvwcopy);
  Bool gridcopy;
  Bool convcopy;
  const Complex *convstor=convFunc.getStorage(convcopy);
  Cube<Int> loc(3, nvc, nRow);
  Cube<Int> off(3, nvc, nRow);
  Matrix<Complex> phasor(nvc, nRow);
  Bool delphase;
  Complex * phasorstor=phasor.getStorage(delphase);
  const Double * visfreqstor=interpVisFreq_p.getStorage(del);
  const Double * scalestor=uvScale.getStorage(del);
  const Double * offsetstor=uvOffset.getStorage(del);
  Int * locstor=loc.getStorage(del);
  Int * offstor=off.getStorage(del);
  const Double *dpstor=dphase.getStorage(del);
  Double cinv=Double(1.0)/C::c;
  Int irow;
  Int dow=1;
  Int nth=1;
#ifdef HAS_OMP
  if(numthreads_p >0){
    nth=min(numthreads_p, omp_get_max_threads());
  }
  else{   
    nth= omp_get_max_threads();
  }
  nth=min(4,nth);
#endif

#pragma omp parallel default(none) private(irow) firstprivate(visfreqstor, nvc, scalestor, offsetstor, csamp, phasorstor, uvwstor, locstor, offstor, dpstor, cinv, dow) shared(startRow, endRow) num_threads(nth)  
{
#pragma omp for
  for (irow=startRow; irow<=endRow;irow++){
    //locateuvw(uvwstor,dpstor, visfreqstor, nvc, scalestor, offsetstor, csamp, 
    //	      locstor, 
    //	      offstor, phasorstor, irow, True);
    locuvw(uvwstor, dpstor, visfreqstor, &nvc, scalestor, offsetstor, &csamp, locstor, offstor, phasorstor, &irow, &dow, &cinv);
  }  

 }//end pragma parallel

  Int x0, y0, nxsub, nysub, ixsub, iysub, icounter, ix, iy;
  ixsub=1;
  iysub=1;
  if (nth >3){
    ixsub=2;
    iysub=2; 
  }
  else if(nth >1){
     ixsub=2;
     iysub=1; 
  }
  x0=1;
  y0=1;
  nxsub=nx;
  nysub=ny;
  Int rbeg=startRow+1;
  Int rend=endRow+1;
  Block<Matrix<Double> > sumwgt(ixsub*iysub);
  for (icounter=0; icounter < ixsub*iysub; ++icounter){
    sumwgt[icounter].resize(sumWeight.shape());
    sumwgt[icounter].set(0.0);
  }
  const Int* pmapstor=polMap.getStorage(del);
  const Int *cmapstor=chanMap.getStorage(del);
  Int nc=nchan;
  Int np=npol;
  Int nxp=nx;
  Int nyp=ny;
  Int csize=convSize;
   Int wcsize=wConvSize;
  const Int * flagstor=flags.getStorage(del);
  const Int * rowflagstor=rowFlags.getStorage(del);
  const Int * suppstor=convSupport.getStorage(del);

  if(!useDoubleGrid_p){
    Complex *gridstor=griddedData.getStorage(gridcopy);
#pragma omp parallel default(none) private(icounter,ix,iy,x0,y0,nxsub,nysub, del) firstprivate(idopsf, uvwstor, datStorage, wgtStorage, flagstor, rowflagstor, convstor, pmapstor, cmapstor, gridstor, suppstor, nxp, nyp, np, nc,ixsub, iysub, rend, rbeg, csamp, csize, wcsize, nvp, nvc, nvisrow, phasorstor, locstor, offstor) shared(sumwgt) num_threads(ixsub*iysub)
    {
#pragma omp for      
    for(icounter=0; icounter < ixsub*iysub; ++icounter){
      ix= (icounter+1)-((icounter)/ixsub)*ixsub;
      iy=(icounter)/ixsub+1;
      y0=(nyp/iysub)*(iy-1)+1;
      nysub=nyp/iysub;
      if( iy == iysub) {
	nysub=nyp-(nyp/iysub)*(iy-1);
      }
      x0=(nxp/ixsub)*(ix-1)+1;
      nxsub=nxp/ixsub;
      if( ix == ixsub){
	nxsub=nxp-(nxp/ixsub)*(ix-1);
      } 

      sectgwgrids(uvwstor,
	   datStorage,
	   &nvp,
	   &nvc,
	   &idopsf,
	   flagstor,
	   rowflagstor,
	   wgtStorage,
	   &nvisrow,
	   gridstor,
	   &nxp,
	   &nyp,
	   &np,
	   &nc,
	   suppstor,
	   &csize,
	   &csamp,
	   &wcsize,
	   convstor,
	   cmapstor,
	   pmapstor,
		  (sumwgt[icounter]).getStorage(del), 
		  &x0, &y0, &nxsub, &nysub, &rbeg, &rend, locstor, offstor,
		 phasorstor);
    }
    }//end pragma parallel



    for (icounter=0; icounter < ixsub*iysub; ++icounter){
      sumWeight=sumWeight+sumwgt[icounter];
    }    
    griddedData.putStorage(gridstor, gridcopy);
    
  }
  else{
    DComplex *gridstor=griddedData2.getStorage(gridcopy);
#pragma omp parallel default(none) private(icounter,ix,iy,x0,y0,nxsub,nysub, del) firstprivate(idopsf, uvwstor, datStorage, wgtStorage, flagstor, rowflagstor, convstor, pmapstor, cmapstor, gridstor, suppstor, nxp, nyp, np, nc,ixsub, iysub, rend, rbeg, csamp, csize, wcsize, nvp, nvc, nvisrow, phasorstor, locstor, offstor) shared(sumwgt) num_threads(ixsub*iysub)
    {
#pragma omp for      
    for(icounter=0; icounter < ixsub*iysub; ++icounter){
      ix= (icounter+1)-((icounter)/ixsub)*ixsub;
      iy=(icounter)/ixsub+1;
      y0=(nyp/iysub)*(iy-1)+1;
      nysub=nyp/iysub;
      if( iy == iysub) {
	nysub=nyp-(nyp/iysub)*(iy-1);
      }
      x0=(nxp/ixsub)*(ix-1)+1;
      nxsub=nxp/ixsub;
      if( ix == ixsub){
	nxsub=nxp-(nxp/ixsub)*(ix-1);
      } 

      sectgwgridd(uvwstor,
	   datStorage,
	   &nvp,
	   &nvc,
	   &idopsf,
	   flagstor,
	   rowflagstor,
	   wgtStorage,
	   &nvisrow,
	   gridstor,
	   &nxp,
	   &nyp,
	   &np,
	   &nc,
	   suppstor,
	   &csize,
	   &csamp,
	   &wcsize,
	   convstor,
	   cmapstor,
	   pmapstor,
		  (sumwgt[icounter]).getStorage(del), 
		  &x0, &y0, &nxsub, &nysub, &rbeg, &rend, locstor, offstor,
		 phasorstor);
    }
    }//end pragma parallel



    for (icounter=0; icounter < ixsub*iysub; ++icounter){
      sumWeight=sumWeight+sumwgt[icounter];
    }
    griddedData2.putStorage(gridstor, gridcopy);
  }
  uvw.freeStorage(uvwstor, uvwcopy);
  convFunc.freeStorage(convstor, convcopy);
  
  if(!dopsf)
    data.freeStorage(datStorage, isCopy);
  elWeight.freeStorage(wgtStorage,iswgtCopy);
}

void WProjectFT::get(VisBuffer& vb, Int row)
{
  

  findConvFunction(*image, vb);
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
  
  Int nvp=data.shape()(0);
  Int nvc=data.shape()(1);
  Int nvisrow=data.shape()(2);
  Int nc=nchan;
  Int np=npol;
  Int nxp=nx;
  Int nyp=ny;
  Cube<Int> loc(3, nvc, nvisrow);
  Cube<Int> off(3, nvc, nRow);
  Int csamp=convSampling;
  Int csize=convSize;
  Int wcsize=wConvSize;
  Matrix<Complex> phasor(nvc, nRow);
  Bool delphase;
  Bool del;
  Complex * phasorstor=phasor.getStorage(delphase);
  const Double * visfreqstor=interpVisFreq_p.getStorage(del);
  const Double * scalestor=uvScale.getStorage(del);
  const Double * offsetstor=uvOffset.getStorage(del);
  Int * locstor=loc.getStorage(del);
  Int * offstor=off.getStorage(del);
  const Int * flagstor=flags.getStorage(del);
  const Int * rowflagstor=rowFlags.getStorage(del);
  const Double *dpstor=dphase.getStorage(del);
  Bool uvwcopy; 
  const Double *uvwstor=uvw.getStorage(uvwcopy);
  Bool gridcopy;
  const Complex *gridstor=griddedData.getStorage(gridcopy);
  Bool convcopy;
  const Complex *convstor=convFunc.getStorage(convcopy);
  const Int* pmapstor=polMap.getStorage(del);
  const Int *cmapstor=chanMap.getStorage(del);
  const Int * suppstor=convSupport.getStorage(del);
  Int irow;
  Int nth=1;
#ifdef HAS_OMP
  if(numthreads_p >0){
    nth=min(numthreads_p, omp_get_max_threads());
  }
  else{   
    nth= omp_get_max_threads();
  }
  nth=min(4,nth);
#endif
  Int dow=1;
  Double cinv=Double(1.0)/C::c;

#pragma omp parallel default(none) private(irow) firstprivate(visfreqstor, nvc, scalestor, offsetstor, csamp, phasorstor, uvwstor, locstor, offstor, dpstor, dow, cinv) shared(startRow, endRow) num_threads(nth) 
  {
#pragma omp for
    for (irow=startRow; irow<=endRow; ++irow){
      /*locateuvw(uvwstor,dpstor, visfreqstor, nvc, scalestor, offsetstor, csamp, 
		locstor, 
		offstor, phasorstor, irow, True);*/
      locuvw(uvwstor, dpstor, visfreqstor, &nvc, scalestor, offsetstor, &csamp, locstor, offstor, phasorstor, &irow, &dow, &cinv);
  }  

  }//end pragma parallel
  Int rbeg=startRow+1;
  Int rend=endRow+1;
  Int npart=1;
  if (nth >3){
    npart=4;
  }
  else if(nth >1){
    npart=2; 
  }
  Int ix=0;
  #pragma omp parallel default(none) private(ix, rbeg, rend) firstprivate(uvwstor, datStorage, flagstor, rowflagstor, convstor, pmapstor, cmapstor, gridstor, nxp, nyp, np, nc, suppstor, csamp, csize, wcsize, nvp, nvc, nvisrow, phasorstor, locstor, offstor) shared(npart) num_threads(npart)
  {
    #pragma omp for 
    for (ix=0; ix< npart; ++ix){
      rbeg=ix*(nvisrow/npart)+1;
      rend=(ix != (npart-1)) ? (rbeg+(nvisrow/npart)-1) : (rbeg+(nvisrow/npart)+nvisrow%npart-1) ;
      // cerr << "rbeg " << rbeg << " rend " << rend << " nRow " << nvisrow << endl;
      sectdwgrid(uvwstor,
		 datStorage,
		 &nvp,
		 &nvc,
		 flagstor,
		 rowflagstor,
		 &nvisrow,
		 gridstor,
		 &nxp,
		 &nyp,
		 &np,
		 &nc,
		 suppstor,
		 &csize,
		 &csamp,
		 &wcsize,
		 convstor,
		 cmapstor,
		 pmapstor,
		 &rbeg, &rend, locstor, offstor, phasorstor);
    }

  }//end pragma parallel
  data.putStorage(datStorage, isCopy);
  uvw.freeStorage(uvwstor, uvwcopy);
  griddedData.freeStorage(gridstor, gridcopy);
  convFunc.freeStorage(convstor, convcopy);
  

  interpolateFrequencyFromgrid(vb, data, FTMachine::MODEL);
}



// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& WProjectFT::getImage(Matrix<Float>& weights,
					      Bool normalize) 
{
  //AlwaysAssert(lattice, AipsError);
  AlwaysAssert(image, AipsError);
  
  if((griddedData.nelements() ==0) && (griddedData2.nelements()==0)){
     throw(AipsError("Programmer error ...request for image without right order of calls"));
  }
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
    logIO() << LogIO::DEBUGGING
	    << "Starting FFT and scaling of image" << LogIO::POST;
    
    if(useDoubleGrid_p){
      ArrayLattice<DComplex> darrayLattice(griddedData2);
      LatticeFFT::cfft2d(darrayLattice,False);
      griddedData.resize(griddedData2.shape());
      convertArray(griddedData, griddedData2);
      griddedData2.resize();
      arrayLattice = new ArrayLattice<Complex>(griddedData);
      lattice=arrayLattice;
    }else{
      arrayLattice = new ArrayLattice<Complex>(griddedData);
      lattice=arrayLattice;
      LatticeFFT::cfft2d(*lattice,False);

    }


    const IPosition latticeShape = lattice->shape();
    
    
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
	if(weights(pol, chan)!=0.0) {
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
  griddedData.resize();
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

Bool WProjectFT::toRecord(String& error,
			  RecordInterface& outRec, Bool withImage, const String diskimage)
{  

  /*

  CountedPtr<WPConvFunc> wpConvFunc_p;
  */

  // Save the current WProjectFT object to an output state record
  Bool retval = True;
  //save the base class variables
  //this is a memory hog and slow on saving and recovering...better to recompute convfunctions
  /* Record wpconvrec;
  if(wpConvFunc_p->toRecord(wpconvrec))
    outRec.defineRecord("wpconvfunc", wpconvrec);
  */
  if(!FTMachine::toRecord(error, outRec, withImage, diskimage))
    return False;

  outRec.define("uvscale", uvScale);
  outRec.define("uvoffset", uvOffset);
  outRec.define("istiled", isTiled);
  outRec.define("cachesize", Int64(cachesize));
  outRec.define("tilesize", tilesize);
  outRec.define("maxabsdata", maxAbsData);
  Vector<Int> center_loc(4), offset_loc(4);
  for (Int k=0; k<4 ; k++){
    center_loc(k)=centerLoc(k);
    offset_loc(k)=offsetLoc(k);
  }
  outRec.define("centerloc", center_loc);
  outRec.define("offsetloc", offset_loc);
  outRec.define("padding", padding_p);
  outRec.define("nwplanes", nWPlanes_p);
  outRec.define("savedwscale", savedWScale_p);
  outRec.define("usezero", usezero_p);
  outRec.define("convfunc", convFunc);
  outRec.define("convsampling", convSampling);
  outRec.define("convsize", convSize);
  outRec.define("convsupport", convSupport);
  outRec.define("convsizes", convSizes_p);
  outRec.define("wconvsize", wConvSize);
  outRec.define("lastindex", lastIndex_p);



  return retval;
}

Bool WProjectFT::fromRecord(String& error,
			    const RecordInterface& inRec)
{
  if(!FTMachine::fromRecord(error, inRec))
    return False;
  machineName_p="WProjectFT";
  Bool retval = True;
  imageCache=0; lattice=0; arrayLattice=0;
  inRec.get("uvscale", uvScale);
  inRec.get("uvoffset", uvOffset);
  inRec.get("istiled", isTiled);
  cachesize=inRec.asInt64("cachesize");
  inRec.get("tilesize", tilesize);
  inRec.get("maxabsdata", maxAbsData);
  Vector<Int> center_loc(4), offset_loc(4);
  inRec.get("centerloc", center_loc);
  inRec.get("offsetloc", offset_loc);
  uInt ndim4 = 4;
  centerLoc=IPosition(ndim4, center_loc(0), center_loc(1), center_loc(2), 
		      center_loc(3));
  offsetLoc=IPosition(ndim4, offset_loc(0), offset_loc(1), offset_loc(2), 
		      offset_loc(3));
  if(inRec.isDefined("wpconvfunc")){
    wpConvFunc_p=new WPConvFunc(inRec.asRecord("wpconvfunc"));
  }
  else{
    wpConvFunc_p=new WPConvFunc();
  }
  inRec.get("padding", padding_p);
  inRec.get("nwplanes", nWPlanes_p);
  inRec.get("savedwscale", savedWScale_p);
  inRec.get("usezero", usezero_p);
  inRec.get("convfunc", convFunc);
  inRec.get("convsampling", convSampling);
  inRec.get("convsize", convSize);
  inRec.get("convsupport", convSupport);
  inRec.get("convsizes", convSizes_p);
  inRec.get("wconvsize", wConvSize);
  inRec.get("lastindex", lastIndex_p);
  gridder=0;
    ///setup some of the parameters
  init();
     

  
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
    StokesImageUtil::changeCStokesRep(theImage, StokesImageUtil::LINEAR);
  }
  else {
    StokesImageUtil::changeCStokesRep(theImage, StokesImageUtil::CIRCULAR);
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


String WProjectFT::name() const {

  return machineName_p;

}



} //# NAMESPACE CASA - END

