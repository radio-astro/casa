// -*- C++ -*-
//# rGridFT.cc: Implementation of rGridFT class
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

#include <synthesis/MSVis/VisibilityIterator.h>
#include <casa/Quanta/UnitMap.h>
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
#include <synthesis/MeasurementComponents/rGridFT.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/MeasurementComponents/VisibilityResampler.h>
#include <synthesis/MeasurementComponents/MultiThreadedVisResampler.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/MeasurementComponents/VBStore.h>
#include <scimath/Mathematics/RigidVector.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisSet.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixIter.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <measures/Measures/UVWMachine.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <casa/Utilities/CompositeNumber.h>
#include <casa/OS/Timer.h>
#include <casa/sstream.h>
#define DORES True
namespace casa { //# NAMESPACE CASA - BEGIN

  rGridFT::rGridFT(Long icachesize, Int itilesize, 
		   CountedPtr<VisibilityResamplerBase>& visResampler,
		   String iconvType, Float padding,
		 Bool usezero, Bool useDoublePrec)
: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize), tilesize(itilesize),
  gridder(0), isTiled(False), convType(iconvType),
  maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
  usezero_p(usezero), noPadding_p(False), usePut2_p(False), 
  machineName_p("rGridFT"), visResampler_p(visResampler)//visResampler_p(useDoublePrec)

{
  logIO() << LogOrigin("rGridFT", "rGridFT")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  useDoubleGrid_p=useDoublePrec;  
  canComputeResiduals_p=DORES;
}

  rGridFT::rGridFT(Long icachesize, Int itilesize, 
		   CountedPtr<VisibilityResamplerBase>& visResampler,String iconvType,
		   MPosition mLocation, Float padding, Bool usezero, 
		   Bool useDoublePrec)
: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
  tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType), maxAbsData(0.0), centerLoc(IPosition(4,0)),
  offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False), 
  usePut2_p(False), machineName_p("rGridFT"), visResampler_p(visResampler)
{
  logIO() << LogOrigin("rGridFT", "rGridFT")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  mLocation_p=mLocation;
  tangentSpecified_p=False;
  useDoubleGrid_p=useDoublePrec;
  canComputeResiduals_p=DORES;
}

rGridFT::rGridFT(Long icachesize, Int itilesize, 
		 CountedPtr<VisibilityResamplerBase>& visResampler, 
		 String iconvType,
		 MDirection mTangent, Float padding, Bool usezero, Bool useDoublePrec)
: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
  tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType), maxAbsData(0.0), centerLoc(IPosition(4,0)),
  offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False), 
  usePut2_p(False), machineName_p("rGridFT"), visResampler_p(visResampler)
{
  logIO() << LogOrigin("rGridFT", "rGridFT")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  mTangent_p=mTangent;
  tangentSpecified_p=True;
  useDoubleGrid_p=useDoublePrec;
  canComputeResiduals_p=DORES;
}

rGridFT::rGridFT(Long icachesize, Int itilesize, 
		 CountedPtr<VisibilityResamplerBase>& visResampler, 
		 String iconvType, MPosition mLocation, MDirection mTangent, Float padding,
		 Bool usezero, Bool useDoublePrec)
: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
  tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType), maxAbsData(0.0), centerLoc(IPosition(4,0)),
  offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False), 
  usePut2_p(False),machineName_p("rGridFT"), visResampler_p(visResampler)
{
  logIO() << LogOrigin("rGridFT", "rGridFT")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  mLocation_p=mLocation;
  mTangent_p=mTangent;
  tangentSpecified_p=True;
  useDoubleGrid_p=useDoublePrec;
  canComputeResiduals_p=DORES;
}

rGridFT::rGridFT(const RecordInterface& stateRec)
  : FTMachine()
{
  // Construct from the input state record
  logIO() << LogOrigin("rGridFT", "rGridFT(RecordInterface)")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  String error;
  if (!fromRecord(error, stateRec)) 
    throw (AipsError("Failed to create gridder: " + error));
  visResampler_p->init(useDoubleGrid_p);
  canComputeResiduals_p=DORES;
}

//---------------------------------------------------------------------- 
rGridFT& rGridFT::operator=(const rGridFT& other)
{
  if(this!=&other) {
    //Do the base parameters
    FTMachine::operator=(other);
    
    //private params
    imageCache=other.imageCache;
    cachesize=other.cachesize;
    tilesize=other.tilesize;
    convType=other.convType;
    uvScale.resize();
    uvOffset.resize();
    uvScale=other.uvScale;
    uvOffset=other.uvOffset;
    if(other.gridder==0)
      gridder=0;
    else{  
      gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						     uvScale, uvOffset,
						     convType);
    }
    isTiled=other.isTiled;
    //lattice=other.lattice;
    lattice=0;
    tilesize=other.tilesize;
    arrayLattice=0;
    maxAbsData=other.maxAbsData;
    centerLoc=other.centerLoc;
    offsetLoc=other.offsetLoc;
    padding_p=other.padding_p;
    usezero_p=other.usezero_p;
    noPadding_p=other.noPadding_p;
    visResampler_p = other.visResampler_p;//Copy the pointer
    // Since visResampler_p is a CountedPtr, we need to clone it for
    // new rGridFT( *ft) in CubeSkyEquation to work properly.
    //visResampler_p=other.visResampler_p->clone();
    *visResampler_p = *other.visResampler_p; // Call the appropriate operator=()
  };
  return *this;
};

//----------------------------------------------------------------------
  rGridFT::rGridFT(const rGridFT& other) : FTMachine(), machineName_p("rGridFT")
  {
    //  visResampler_p.init(useDoubleGrid_p);
    operator=(other);
  }

//----------------------------------------------------------------------
//  CountedPtr<rGridFT> rGridFT::clone()
  rGridFT* rGridFT::clone()
  {
    rGridFT* newftm = new rGridFT(*this);
     CountedPtr<VisibilityResamplerBase> newvisresampler = newftm->visResampler_p->clone();
    newftm->visResampler_p = newvisresampler;
    return newftm;
  }

//----------------------------------------------------------------------
void rGridFT::init() {

  logIO() << LogOrigin("rGridFT", "init")  << LogIO::NORMAL;
  canComputeResiduals_p = DORES;
  ok();

  /* hardwiring isTiled is False
  // Padding is possible only for non-tiled processing
  if((padding_p*padding_p*image->shape().product())>cachesize) {
    isTiled=True;
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
  }
  else {
  */
    // We are padding.
    isTiled=False;
    if(!noPadding_p){
      CompositeNumber cn(uInt(image->shape()(0)*2));    
      nx    = cn.nextLargerEven(Int(padding_p*Float(image->shape()(0))-0.5));
      ny    = cn.nextLargerEven(Int(padding_p*Float(image->shape()(1))-0.5));
    }
    else{
      nx    = image->shape()(0);
      ny    = image->shape()(1);
    }
    npol  = image->shape()(2);
    nchan = image->shape()(3);
    // }

  sumWeight.resize(npol, nchan);

  uvScale.resize(2);
  uvScale(0)=(Float(nx)*image->coordinates().increment()(0)); 
  uvScale(1)=(Float(ny)*image->coordinates().increment()(1)); 
  uvOffset.resize(2);
  uvOffset(0)=nx/2;
  uvOffset(1)=ny/2;

  // Now set up the gridder. The possibilities are BOX and SF
  if(gridder) delete gridder; gridder=0;
  gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						 uvScale, uvOffset,
						 convType);

  // Setup the CFStore object to carry relavent info. of the Conv. Func.
  cfs_p.xSupport = gridder->cSupport();
  cfs_p.ySupport = gridder->cSupport();
  cfs_p.sampling.resize(2);
  cfs_p.sampling = gridder->cSampling();
  if (cfs_p.rdata.null())
      cfs_p.rdata = new Array<Double>(gridder->cFunction());
  // else
  //   (*cfs_p.rdata) = gridder->cFunction();
    


  visResampler_p->setConvFunc(cfs_p);


  // Set up image cache needed for gridding. For BOX-car convolution
  // we can use non-overlapped tiles. Otherwise we need to use
  // overlapped tiles and additive gridding so that only increments
  // to a tile are written.
  if(imageCache) delete imageCache; imageCache=0;

  if(isTiled) {
    Float tileOverlap=0.5;
    if(convType=="box") {
      tileOverlap=0.0;
    }
    else {
      tileOverlap=0.5;
      tilesize=max(12,tilesize);
    }
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
rGridFT::~rGridFT() {
  if(imageCache) delete imageCache; imageCache=0;
  //if(arrayLattice) delete arrayLattice; arrayLattice=0;
  if(gridder) delete gridder; gridder=0;
}

// Initialize for a transform from the Sky domain. This means that
// we grid-correct, and FFT the image

void rGridFT::initializeToVis(ImageInterface<Complex>& iimage,
			     const VisBuffer& vb)
{
  image=&iimage;

  ok();

  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);
  visResampler_p->setMaps(chanMap, polMap);

  // Need to reset nx, ny for padding
  // Padding is possible only for non-tiled processing
  

  // If we are memory-based then read the image in and create an
  // ArrayLattice otherwise just use the PagedImage
  if(isTiled) {
    lattice=CountedPtr<Lattice<Complex> >(image, False);
  }
  else {
     IPosition gridShape(4, nx, ny, npol, nchan);
     griddedData.resize(gridShape);
     //griddedData can be a reference of image data...if not using model col
     //hence using an undocumented feature of resize that if 
     //the size is the same as old data it is not changed.
     //if(!usePut2_p) griddedData.set(0);
     griddedData.set(Complex(0.0));

     IPosition stride(4, 1);
     IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
     IPosition trc(blc+image->shape()-stride);

     IPosition start(4, 0);
     griddedData(blc, trc) = image->getSlice(start, image->shape());

     //if(arrayLattice) delete arrayLattice; arrayLattice=0;
     arrayLattice = new ArrayLattice<Complex>(griddedData);
     lattice=arrayLattice;
  }

  //AlwaysAssert(lattice, AipsError);

  logIO() << LogIO::DEBUGGING
	  << "Starting grid correction and FFT of image" << LogIO::POST;

  // Do the Grid-correction. 
    {
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
  
    // Now do the FFT2D in place
    LatticeFFT::cfft2d(*lattice);
    
    logIO() << LogIO::DEBUGGING
	    << "Finished grid correction and FFT of image" << LogIO::POST;
    
}




void rGridFT::finalizeToVis()
{
  if(isTiled) {

    logIO() << LogOrigin("rGridFT", "finalizeToVis")  << LogIO::NORMAL;

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
void rGridFT::initializeToSky(ImageInterface<Complex>& iimage,
			     Matrix<Float>& weight, const VisBuffer& vb)
{
  // image always points to the image
  image=&iimage;

  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);
  visResampler_p->setMaps(chanMap, polMap);


  
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
  }
  else {
    IPosition gridShape(4, nx, ny, npol, nchan);
    griddedData.resize(gridShape);
    griddedData=Complex(0.0);
    if(useDoubleGrid_p){
      griddedData2.resize(gridShape);
      griddedData2=DComplex(0.0);
    }
    //iimage.get(griddedData, False);
    //if(arrayLattice) delete arrayLattice; arrayLattice=0;
    arrayLattice = new ArrayLattice<Complex>(griddedData);
    lattice=arrayLattice;
  }
  // if(useDoubleGrid_p) visResampler_p->initializePutBuffers(griddedData2, sumWeight);
  // else                visResampler_p->initializePutBuffers(griddedData, sumWeight);
  if(useDoubleGrid_p) visResampler_p->initializeToSky(griddedData2, sumWeight);
  else                visResampler_p->initializeToSky(griddedData, sumWeight);
  //AlwaysAssert(lattice, AipsError);
}



void rGridFT::finalizeToSky()
{  
  //AlwaysAssert(lattice, AipsError);
  // Now we flush the cache and report statistics
  // For memory based, we don't write anything out yet.
  if(isTiled) {
    logIO() << LogOrigin("rGridFT", "finalizeToSky")  << LogIO::NORMAL;

    AlwaysAssert(image, AipsError);
    AlwaysAssert(imageCache, AipsError);
    imageCache->flush();
    ostringstream o;
    imageCache->showCacheStatistics(o);
    logIO() << o.str() << LogIO::POST;
  }
  // if(useDoubleGrid_p) visResampler_p->GatherGrids(griddedData2, sumWeight);
  // else                visResampler_p->GatherGrids(griddedData, sumWeight);
  if(useDoubleGrid_p) visResampler_p->finalizeToSky(griddedData2, sumWeight);
  else                visResampler_p->finalizeToSky(griddedData, sumWeight);
}



Array<Complex>* rGridFT::getDataPointer(const IPosition& centerLoc2D,
				       Bool readonly) {
  Array<Complex>* result;
  // Is tiled: get tiles and set up offsets
  centerLoc(0)=centerLoc2D(0);
  centerLoc(1)=centerLoc2D(1);
  result=&imageCache->tile(offsetLoc,centerLoc, readonly);
  gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
  return result;
}

void rGridFT::put(const VisBuffer& vb, Int row, Bool dopsf, 
		 FTMachine::Type type)
{


  gridOk(gridder->cSupport()(0));

  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())   matchAllSpwChans(vb);
  
  //Here we redo the match or use previous match
  
  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()])
    matchChannel(vb.spectralWindow(), vb);
  else
    {
      chanMap.resize();
      chanMap=multiChanMap_p[vb.spectralWindow()];
    }

  //No point in reading data if its not matching in frequency
  if(max(chanMap)==-1) return;

  const Matrix<Float> *imagingweight;
  imagingweight=&(vb.imagingWeight());
  
  if(dopsf) {type=FTMachine::PSF;}

  Cube<Complex> data;
  //Fortran gridder need the flag as ints 
  Cube<Int> flags;
  Matrix<Float> elWeight;
  interpolateFrequencyTogrid(vb, *imagingweight,data, flags, elWeight, type);


  Int startRow, endRow, nRow;
  if (row==-1) { nRow=vb.nRow(); startRow=0; endRow=nRow-1; } 
  else         { nRow=1; startRow=row; endRow=row; }

  // Get the uvws in a form that Fortran can use and do that
  // necessary phase rotation. On a Pentium Pro 200 MHz
  // when null, this step takes about 50us per uvw point. This
  // is just barely noticeable for Stokes I continuum and
  // irrelevant for other cases.
  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
  }
   
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  // Set up VBStore object to point to the relavent info. of the VB.
  VBStore vbs;

  vbs.nRow_p = vb.nRow();
  vbs.beginRow_p = 0;
  vbs.endRow_p = vbs.nRow_p;

  vbs.uvw_p.reference(uvw);
  //  vbs.uvw_p.reference(vb.uvwMat());
  vbs.imagingWeight_p.reference(elWeight);
  vbs.visCube_p.reference(data);
  // vbs.modelCube_p.reference(vb.modelVisCube());
  // if (useCorrected) 
  //   vbs.correctedCube_p.reference(vb.correctedVisCube());
  // else 
  //   vbs.visCube_p.reference(vb.visCube());
  //   //  vbs.useCorrected_p = useCorrected;

  vbs.freq_p.reference(interpVisFreq_p);
  vbs.rowFlag_p.resize(0); vbs.rowFlag_p = vb.flagRow();  
  //  vbs.rowFlag_p.reference(vb.flagRow());  
  if(!usezero_p) 
    for (Int rownr=startRow; rownr<=endRow; rownr++) 
      if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) vbs.rowFlag_p(rownr)=True;

  // Really nice way of converting a Cube<Int> to Cube<Bool>.
  // However the VBS objects should ultimately be references
  // directly to bool cubes.
  //  vbs.rowFlag.resize(rowFlags.shape());  vbs.rowFlag  = False; vbs.rowFlag(rowFlags) = True;

  //**************
  vbs.flagCube_p.resize(flags.shape());    vbs.flagCube_p = False; vbs.flagCube_p(flags!=0) = True;
  //  vbs.flagCube_p.reference(vb.flagCube());
  //**************

  visResampler_p->setParams(uvScale,uvOffset,dphase);
  visResampler_p->setMaps(chanMap, polMap);
    
  //Double or single precision gridding.
  if(useDoubleGrid_p) visResampler_p->DataToGrid(griddedData2, vbs, sumWeight, dopsf);
  else                visResampler_p->DataToGrid(griddedData, vbs, sumWeight, dopsf); 
}


void rGridFT::get(VisBuffer& vb, Int row)
{

  gridOk(gridder->cSupport()(0));
  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row < 0) { nRow=vb.nRow(); startRow=0; endRow=nRow-1;} 
  else         { nRow=1; startRow=row; endRow=row; }

  // Get the uvws in a form that Fortran can use
  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
  }
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())  matchAllSpwChans(vb);


  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()])
    matchChannel(vb.spectralWindow(), vb);
  else
    {
      chanMap.resize();
      chanMap=multiChanMap_p[vb.spectralWindow()];
    }

  //No point in reading data if its not matching in frequency
  if(max(chanMap)==-1)    return;

  Cube<Complex> data;
  Cube<Int> flags;
  getInterpolateArrays(vb, data, flags);

  // Apparently we don't support "tiled gridding" any more (good! :)).
  if(isTiled) 
    throw(SynthesisFTMachineError("rGridFT::get(): Internal error.  isTiled is True. "));
  else 
    {
      VBStore vbs;
      vbs.nRow_p = vb.nRow();
      vbs.beginRow_p = 0;
      vbs.endRow_p = vbs.nRow_p;

      vbs.uvw_p.reference(uvw);
      //    vbs.imagingWeight.reference(elWeight);
      vbs.visCube_p.reference(data);
      vbs.freq_p.reference(interpVisFreq_p);
      vbs.rowFlag_p.resize(0); vbs.rowFlag_p = vb.flagRow();  
      if(!usezero_p) 
	for (Int rownr=startRow; rownr<=endRow; rownr++) 
	  if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) vbs.rowFlag_p(rownr)=True;

      // Really nice way of converting a Cube<Int> to Cube<Bool>.
      // However these should ultimately be references directly to bool
      // cubes.
      vbs.flagCube_p.resize(flags.shape());    vbs.flagCube_p = False; vbs.flagCube_p(flags!=0) = True;
      //    vbs.rowFlag.resize(rowFlags.shape());  vbs.rowFlag  = False; vbs.rowFlag(rowFlags) = True;
      
      visResampler_p->setParams(uvScale,uvOffset,dphase);
      visResampler_p->setMaps(chanMap, polMap);

      // De-gridding
      visResampler_p->GridToData(vbs, griddedData);
    }
  interpolateFrequencyFromgrid(vb, data, FTMachine::MODEL);
}



// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& rGridFT::getImage(Matrix<Float>& weights, Bool normalize) 
{
  //AlwaysAssert(lattice, AipsError);
  AlwaysAssert(gridder, AipsError);
  AlwaysAssert(image, AipsError);
  logIO() << LogOrigin("rGridFT", "getImage") << LogIO::NORMAL;

  weights.resize(sumWeight.shape());

  convertArray(weights, sumWeight);
  // If the weights are all zero then we cannot normalize
  // otherwise we don't care.
  if(normalize&&max(weights)==0.0) {
    logIO() << LogIO::SEVERE << "No useful data in rGridFT: weights all zero"
	    << LogIO::POST;
  }
  else {

    const IPosition latticeShape = lattice->shape();
    
    logIO() << LogIO::DEBUGGING
	    << "Starting FFT and scaling of image" << LogIO::POST;
    

  
    // if(useDoubleGrid_p){
    //   convertArray(griddedData, griddedData2);
    //   //Don't need the double-prec grid anymore...
    //   griddedData2.resize();
    // }

    // x and y transforms
    //    LatticeFFT::cfft2d(*lattice,False);
    //
    // Retain the double precision grid for FFT as well.  Convert it
    // to single precision just after (since images are still single
    // precision).
    //
    if(useDoubleGrid_p)
      {
	ArrayLattice<DComplex> darrayLattice(griddedData2);
	LatticeFFT::cfft2d(darrayLattice,False);
	convertArray(griddedData, griddedData2);
	//Don't need the double-prec grid anymore...
	griddedData2.resize();
      }
    else
      LatticeFFT::cfft2d(*lattice,False);

    {
      Int inx = lattice->shape()(0);
      Int iny = lattice->shape()(1);
      Vector<Complex> correction(inx);
      correction=Complex(1.0, 0.0);
      // Do the Grid-correction
      IPosition cursorShape(4, inx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
      for(lix.reset();!lix.atEnd();lix++) {
	Int pol=lix.position()(2);
	Int chan=lix.position()(3);
	if(weights(pol, chan)!=0.0) {
	  gridder->correctX1D(correction, lix.position()(1));
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
      IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
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
void rGridFT::getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights) 
{

  logIO() << LogOrigin("rGridFT", "getWeightImage") << LogIO::NORMAL;

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
  for(lix.reset();!lix.atEnd();lix++) {
    Int pol=lix.position()(2);
    Int chan=lix.position()(3);
    lix.rwCursor()=weights(pol,chan);
  }
}

Bool rGridFT::toRecord(String&,// error,
		       RecordInterface& outRec, Bool withImage)
{
  // Save the current rGridFT object to an output state record
  Bool retval = True;

  Double cacheVal=(Double)cachesize;
  outRec.define("cache", cacheVal);
  outRec.define("tile", tilesize);
  outRec.define("gridfunction", convType);

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

Bool rGridFT::fromRecord(String&,// error,
			 const RecordInterface& inRec)
{
  Bool retval = True;
  gridder=0; imageCache=0; lattice=0; arrayLattice=0;
  Double cacheVal;
  inRec.get("cache", cacheVal);
  cachesize=(Long)cacheVal;
  inRec.get("tile", tilesize);
  inRec.get("gridfunction", convType);

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
      lattice=CountedPtr<Lattice<Complex> >(image, False);
    }
    else {
      // Make the grid the correct shape and turn it into an array lattice
      // Check the section from the image BEFORE converting to a lattice 
      IPosition gridShape(4, nx, ny, npol, nchan);
      griddedData.resize(gridShape);
      griddedData=Complex(0.0);
      IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
      IPosition start(4, 0);
      IPosition stride(4, 1);
      IPosition trc(blc+image->shape()-stride);
      griddedData(blc, trc)=image->getSlice(start, image->shape());
      
      //if(arrayLattice) delete arrayLattice; arrayLattice=0;
      arrayLattice = new ArrayLattice<Complex>(griddedData);
      lattice=arrayLattice;
    }

    //AlwaysAssert(lattice, AipsError);
    AlwaysAssert(gridder, AipsError);
    AlwaysAssert(image, AipsError);
  };
  return retval;
}

void rGridFT::ok() {
  AlwaysAssert(image, AipsError);
}

// Make a plain straightforward honest-to-God image. This returns
// a complex image, without conversion to Stokes. The representation
// is that required for the visibilities.
//----------------------------------------------------------------------
void rGridFT::makeImage(FTMachine::Type type, 
		       VisSet& vs,
		       ImageInterface<Complex>& theImage,
		       Matrix<Float>& weight) {


  logIO() << LogOrigin("rGridFT", "makeImage") << LogIO::NORMAL;

  if(type==FTMachine::COVERAGE) {
    logIO() << "Type COVERAGE not defined for Fourier transforms" << LogIO::EXCEPTION;
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

String rGridFT::name(){

  return machineName_p;


}

void rGridFT::ComputeResiduals(VisBuffer&vb, Bool useCorrected)
{
  VBStore vbs;
  vbs.nRow_p = vb.nRow();
  vbs.modelCube_p.reference(vb.modelVisCube());
  if (useCorrected) vbs.correctedCube_p.reference(vb.correctedVisCube());
  else vbs.visCube_p.reference(vb.visCube());
  vbs.useCorrected_p = useCorrected;
  visResampler_p->ComputeResiduals(vbs);
}

} //# NAMESPACE CASA - END

