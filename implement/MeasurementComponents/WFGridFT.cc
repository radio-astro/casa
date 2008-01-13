//# WFGridFT.cc: Implementation of WFGridFT class
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
#include <msvis/MSVis/VisSet.h>
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
#include <synthesis/MeasurementComponents/WFGridFT.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <casa/Containers/Block.h>
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
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <casa/OS/Timer.h>
#include <casa/sstream.h>
#include <measures/Measures/UVWMachine.h>
#include <casa/System/ProgressMeter.h>


namespace casa { //# NAMESPACE CASA - BEGIN

WFGridFT::WFGridFT(Long icachesize, Int itilesize, String iconvType,
	       MPosition mLocation, MDirection mTangent, Float padding)
  : GridFT(icachesize, itilesize, iconvType, mLocation, mTangent, padding)
{

}


// Memory back to the world

WFGridFT::~WFGridFT(){
  if( nmaps_p >0){
    for (Int k=0; k< nmaps_p; k++){
      //      delete sumWeightPtr[k];
      //      delete imagePtr[k];
      delete gridderNumber[k];
      delete arrayLatticeNumber[k];
      delete griddedDataNumber[k];
    }
   arrayLattice=0;
   gridder=0; 
  }
}


#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define ggridft ggridft_
#define dgridft dgridft_
#endif

extern "C" { 
   void ggridft(Double*,
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
		Double*,
		Int*,
		Int*,
		Double*);
   void dgridft(Double*,
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
		Double*,
		Int*,
		Int*);
}


//----------------------------------------------------------------------
void WFGridFT::initMapsMulti(const VisBuffer& vb, Int nmaps, Int nth) {

  logIO() << LogOrigin("FTMachine", "initMapsMulti") << LogIO::NORMAL;

  AlwaysAssert(image, AipsError);

  nmaps_p = nmaps;

  nvischan  = vb.frequency().nelements();

  // Set the frame for the UVWMachine
  mFrame_p=MeasFrame(MEpoch(Quantity(vb.time()(0), "s")), mLocation_p);

  // First get the CoordinateSystem for the image and then find
  // the DirectionCoordinate
  CoordinateSystem coords=image->coordinates();
  Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
  AlwaysAssert(directionIndex>=0, AipsError);
  DirectionCoordinate
    directionCoord=coords.directionCoordinate(directionIndex);

  // Now we need MDirection of the image phase center. This is
  // what we define it to be. So we define it to be the
  // center pixel. So we have to do the conversion here.
  // This is independent of padding since we just want to know 
  // what the world coordinates are for the phase center
  // pixel
  {
    Vector<Double> pixelPhaseCenter(2);
    pixelPhaseCenter(0)=Double(image->shape()(0))/2.0;
    pixelPhaseCenter(1)=Double(image->shape()(1))/2.0;
    directionCoord.toWorld(mImage_p, pixelPhaseCenter);
  
    //   mImageHolder.resize(nmaps);
   mImageHolder[nth]=mImage_p;

    //    directionCoord.toWorld(mImageHolder[nth], pixelPhaseCenter);
  }


  //need to set the pol and channel stuff only once...the first call here
 if(nth==0){
   initMaps(vb);
 }

 //We want to explicitly reset the uvwmachine in put and get here.

 if(uvwMachine_p) delete uvwMachine_p; uvwMachine_p=0;
 
}


void WFGridFT::makeMultiImage(FTMachine::Type type, 
			  VisSet& vs,
			  PtrBlock< TempImage<Complex> * >& imagepointer,
			  PtrBlock< Matrix<Float> * >& weightpointer, Int nmodels) {




  // Initialize the gradients
  ROVisIter& vi(vs.iter());
  //  Int numberOfRows= vs.numberCoh();
  //  vi.setRowBlocking(numberOfRows);
  // Loop over all visibilities and pixels
  VisBuffer vb(vi);
  
  // Initialize put (i.e. transform to Sky) for this model
  vi.origin();

  //  if(vb.polFrame()==MSIter::Linear) {
  //    StokesImageUtil::changeCStokesRep(image, SkyModel::LINEAR);
  //  }
  //  else {
  //    StokesImageUtil::changeCStokesRep(image, SkyModel::CIRCULAR);
  //  }
  
  initializeToMultiSky(imagepointer,weightpointer,vb, nmodels);

  Int cohDone=0;
  
  ostringstream modelName;modelName<<"FTMachine : transforming";
  ProgressMeter pm(1.0, Double(vs.numberCoh()+1),
		   modelName, "", "", "", True);
      vi.originChunks();
      vi.origin();

      


  // Loop over the visibilities, putting VisBuffers
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin(); vi.more(); vi++) {
      
      switch(type) {
      case FTMachine::RESIDUAL:
	vb.visCube()=vb.correctedVisCube();
	vb.visCube()-=vb.modelVisCube();
        multiPut(vb, -1, False, nmodels);
        break;
      case FTMachine::MODEL:
	vb.visCube()=vb.modelVisCube();
        multiPut(vb, -1, False, nmodels);
        break;
      case FTMachine::CORRECTED:
	vb.visCube()=vb.correctedVisCube();
        multiPut(vb, -1, False, nmodels);
        break;
      case FTMachine::PSF:
	vb.visCube()=Complex(1.0,0.0);
        multiPut(vb, -1, True, nmodels);
        break;
      case FTMachine::OBSERVED:
      default:
        multiPut(vb, -1, False,nmodels);
        break;
      }
      cohDone+=vb.nRow();
      pm.update(Double(cohDone+1));
    }
  }
  //  finalizeToSky();

  for(Int k=0; k <nmodels; k++){
    mapNumber_p=k;
    changeMapNumber(k);
    (imagepointer[k])->copyData(getImage(*weightpointer[k], True));

  }
}

ImageInterface<Complex>& WFGridFT::getImage(Matrix<Float>& weights, 
					    Bool normalize) 
{
  Matrix<Float> myweight;

  if(nmaps_p > 1){
    if(arrayLatticeNumber[mapNumber_p]) delete arrayLatticeNumber[mapNumber_p]; 
    arrayLattice = new ArrayLattice<Complex>(*(griddedDataNumber[mapNumber_p]));
    //Just trying to get this to compile now....need to get rid of this class
    arrayLatticeNumber[mapNumber_p]=&(*arrayLattice);
    //    arrayLattice=arrayLatticeNumber[mapNumber_p];
    lattice=arrayLattice;
    image= imagePtr[mapNumber_p];
    gridder=gridderNumber[mapNumber_p];
    convertArray(sumWeight,*(sumWeightPtr[mapNumber_p])); 
    myweight.resize((sumWeightPtr[mapNumber_p])->shape());
    myweight = (*(sumWeightPtr[mapNumber_p]));
  }
  AlwaysAssert(gridder, AipsError);
  AlwaysAssert(image, AipsError);

  logIO() << LogOrigin("GridFT", "getImage") << LogIO::NORMAL;

 
  // If the weights are all zero then we cannot normalize
  // otherwise we don't care.
  if(normalize&&max(myweight)==0.0) {
    logIO() << LogIO::SEVERE << "No useful data: weights all zero"
	    << LogIO::POST;
  }
  else {

    const IPosition latticeShape = lattice->shape();
    
    logIO() << LogIO::DEBUGGING
	    << "Starting FFT and scaling of image" << LogIO::POST;
    
    // x and y transforms
    LatticeFFT::cfft2d(*lattice,False);
    

    {
      // Do the Grid-correction
      IPosition loc(2, 0);
      IPosition cursorShape(4, nx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(latticeShape, cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
      //===
      Float pixels=Float(nx)*Float(ny);
      //===
      for(lix.reset();!lix.atEnd();lix++) {
	Int pol=lix.position()(2);
	Int chan=lix.position()(3);
	if(myweight(pol, chan)>0.0) {
     	  //Complex rnorm(Float(nx)*Float(ny));
          //====
	  Float rnorm=pixels; 
          //==    
	  if(normalize) rnorm/=myweight(pol,chan);
	  //
	  //if(max(abs(lix.cursor()))>0.0f) {
            loc(1)=lix.position()(1);
	    for(Int ix=0;ix<nx;ix++) {
	      loc(0)=ix;
	      lix.rwVectorCursor()(ix)/=gridder->correct(loc);
	    }
	  //} 
	  //
	  lix.rwCursor()*=Complex(rnorm);
	}
	else {
	  lix.woCursor()=0.0;
	}
      }
    }
    
    if(!isTiled) {
      // Now find the SubLattice corresponding to the image
      IPosition gridShape(4, nx, ny, npol, nchan);
      IPosition blc(4, (nx-image->shape()(0))/2, (ny-image->shape()(1))/2, 0, 0);
      IPosition stride(4, 1);
      IPosition trc(blc+image->shape()-stride);
      LCBox gridBox(blc, trc, gridShape);
      SubLattice<Complex> gridSub(*arrayLattice, gridBox); 
      
      // Do the copy
      image->copyData(gridSub);
    }
    


    if(normalize) {
      logIO() << LogIO::DEBUGGING
	      << "Finished FFT, grid correction and normalization of image"
	      << LogIO::POST;
    }
    else {
      logIO() << LogIO::DEBUGGING
	      << "Finished FFT and grid correction of image" << LogIO::POST;
    }
  }
 
  return *image ;
  
}

void WFGridFT::initializeToMultiVis(PtrBlock< TempImage<Complex> * >& iimage,
			           const VisBuffer& vb, Int nmaps)
{
  // iimage is the ptrblock of image from sm usually

    uInt nptrstuff=arrayLatticeNumber.nelements();
  //On linux it crashes if delete all.. so the last arrayLattice/gridder
  // is getting leaked.
  for (Int k=0 ; k < (Int(nptrstuff)-1) ; ++k){
    if(arrayLatticeNumber[k])
      delete arrayLatticeNumber[k];
    arrayLatticeNumber[k]=0;
  }
  nptrstuff=griddedDataNumber.nelements();
  for (uInt k=0 ; k < nptrstuff ; ++k){
    if(griddedDataNumber[k])
      delete griddedDataNumber[k];
    griddedDataNumber[k]=0;
  }
  nptrstuff=gridderNumber.nelements();
  for (Int k=0 ; k < (Int(nptrstuff)-1) ; ++k){
    if(gridderNumber[k])
      delete gridderNumber[k];
    gridderNumber[k]=0;
  }


  assignMapNumber(nmaps);
  arrayLatticeNumber.resize(nmaps);   arrayLatticeNumber=0;
  griddedDataNumber.resize(nmaps); griddedDataNumber=0;  
  mImageHolder.resize(nmaps);
  maxAbsDataNumber.resize(nmaps); 
  gridderNumber.resize(nmaps); gridderNumber=0;
  nxPixels.resize(nmaps);
  nyPixels.resize(nmaps);
  sumWeightPtr.resize(nmaps); sumWeightPtr=0;

  imagePtr=iimage;


  for (Int i=0; i < nmaps; i++){

    image=imagePtr[i];

  

    initMulti(nmaps, i);

    // Initialize the maps for polarization and channel. These maps
    // translate visibility indices into image indices
    initMapsMulti(vb, nmaps, i);
    
    // Need to reset nx, ny for padding
    // Padding is possible only for non-tiled processing
    isTiled = False; 

    nxPixels[i]    = Int(padding_p*Float(image->shape()(0)));
    nyPixels[i]    = Int(padding_p*Float(image->shape()(1)));

    if(i==0){
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
	nx    = Int(padding_p*Float(image->shape()(0)));
	ny    = Int(padding_p*Float(image->shape()(1)));
	npol  = image->shape()(2);
	nchan = image->shape()(3);
      }
    }
    // If we are memory-based then read the image in and create an
    // ArrayLattice otherwise just use the PagedImage
    if(isTiled) {
      lattice=image;
    }
    else {
      // Make the grid the correct shape and turn it into an array lattice
      nx=nxPixels[i];
      ny=nyPixels[i];
      IPosition gridShape(4, nx, ny, npol, nchan);
      if (griddedDataNumber[i]) delete griddedDataNumber[i];
      Array<Complex> * tempgrid = new Array<Complex> (gridShape, Complex(0.0));
      griddedDataNumber[i]=tempgrid;
      arrayLattice=arrayLatticeNumber[i];
      //if(arrayLattice) delete arrayLattice; arrayLattice=0;
      arrayLatticeNumber[i] = new ArrayLattice<Complex>(*(griddedDataNumber[i]));


    // Now find the SubLattice corresponding to the image
      IPosition blc(4, (nx-image->shape()(0))/2, (ny-image->shape()(1))/2, 0, 0);
      IPosition stride(4, 1);
      IPosition trc(blc+image->shape()-stride);
      LCBox gridBox(blc, trc, gridShape);
      SubLattice<Complex> gridSub(*(arrayLatticeNumber[i]), gridBox, True);
      gridSub.copyData(*(imagePtr[i]));
      lattice=arrayLatticeNumber[i];
    }

    //AlwaysAssert(lattice, AipsError);
    
    logIO() << LogIO::DEBUGGING
	    << "Starting grid correction and FFT of image" << LogIO::POST;
    
    // Do the Grid-correction. Here we check for an empty image
    // to ensure that we don't waste time FFTing it.
    {
      LatticeStepper ls(lattice->shape(), IPosition(4, nx, 1, 1, 1),
			IPosition(4, 0, 1, 2, 3));
      LatticeIterator<Complex> li(*(arrayLatticeNumber[i]), ls);
      
      maxAbsDataNumber[i]=0.0;
      for(li.reset();!li.atEnd();li++) {
	Int iy=li.position()(1);
	IPosition loc(2, 0, iy);
	for(Int ix=0;ix<nx;ix++) {
	  loc(0)=ix;
	  Float absThis=abs(li.rwVectorCursor()(ix));
	  if(absThis>0.0) {
	    li.rwVectorCursor()(ix)/=gridderNumber[i]->correct(loc);
	    if(absThis>maxAbsDataNumber[i]) maxAbsDataNumber[i]=absThis;
	  }
	}
      }
    }
    

  
  // Now do the FFT2D in place
    if (maxAbsDataNumber[i] > 0.0) {
      LatticeFFT::cfft2d(*(arrayLatticeNumber[i]));
      arrayLattice=arrayLatticeNumber[i];
    }
  }
  
  logIO() << LogIO::DEBUGGING
	  << "Finished grid correction and FFT of image" << LogIO::POST;
      
}

void WFGridFT::initializeToMultiSky( PtrBlock< TempImage<Complex> * >& iimage,
			      PtrBlock<Matrix<Float> * >& weightPtr, const VisBuffer& vb, Int nmaps)
{
  // image always points to the image

  nxPixels.resize(nmaps);
  nyPixels.resize(nmaps);
  uInt nptrstuff=arrayLatticeNumber.nelements();
  //On linux it crashes if delete all.. so the last arrayLattice/gridder
  // is getting leaked.
  for (Int k=0 ; k < (Int(nptrstuff)-1) ; ++k){
    if(arrayLatticeNumber[k])
      delete arrayLatticeNumber[k];
    arrayLatticeNumber[k]=0;
  }
  nptrstuff=griddedDataNumber.nelements();
  for (uInt k=0 ; k < nptrstuff ; ++k){
    if(griddedDataNumber[k])
      delete griddedDataNumber[k];
    griddedDataNumber[k]=0;
  }
  nptrstuff=gridderNumber.nelements();
  for (Int k=0 ; k < (Int(nptrstuff)-1) ; ++k){
    if(gridderNumber[k])
      delete gridderNumber[k];
    gridderNumber[k]=0;
  }
  arrayLatticeNumber.resize(nmaps); arrayLatticeNumber=0; 
  griddedDataNumber.resize(nmaps);   griddedDataNumber=0;
  mImageHolder.resize(nmaps); 
  gridderNumber.resize(nmaps);  gridderNumber=0;
 
  imagePtr=iimage;
  sumWeightPtr=weightPtr;

 for (Int i = 0; i< nmaps; i++){
   image=iimage[i];
   initMulti(nmaps, i);
   // Initialize the maps for polarization and channel. These maps
   // translate visibility indices into image indices
   initMapsMulti(vb, nmaps, i);
   // Need to reset nx, ny for padding
   // Padding is possible only for non-tiled processing
   if((padding_p*padding_p*image->shape().product())>cachesize) {
     isTiled=True;
     
    logIO() << LogIO::WARN
	    << "The amount of memory is too low for WFGridFT" << LogIO::POST;
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
   }
   else {
     // We are padding.
     isTiled=False;
     nx    = Int(padding_p*Float(image->shape()(0)));
     ny    = Int(padding_p*Float(image->shape()(1)));
     nxPixels(i)=nx;
     nyPixels(i)=ny;
     npol  = image->shape()(2);
     nchan = image->shape()(3);
   }
   
  sumWeight=0.0;
  (sumWeightPtr[i])->set(0.0); 
  
  // Initialize for in memory or to disk gridding. lattice will
  // point to the appropriate Lattice, either the ArrayLattice for
  // in memory gridding or to the image for to disk gridding.
  if(isTiled) {
    imageCache->flush();
    image->set(Complex(0.0));
    lattice=image;
  }
  else {
    nx=nxPixels[i];
    ny=nyPixels[i];
    IPosition gridShape(4, nx, ny, npol, nchan);
    if(griddedDataNumber[i]) delete griddedDataNumber[i];
    Array<Complex> * tempgrid = new Array<Complex> (gridShape, Complex(0.0));
    griddedDataNumber[i]=tempgrid;
    arrayLattice=arrayLatticeNumber[i];
    //    if(arrayLattice) delete arrayLattice; arrayLattice=0;
    arrayLattice = new ArrayLattice<Complex>(*(griddedDataNumber[i]));
    //Just making sure it compiles...this class has to go
    arrayLatticeNumber[i]=&(*arrayLattice);
    lattice=arrayLattice;
  }
  //AlwaysAssert(lattice, AipsError);
 }
}


void WFGridFT::multiPut(const VisBuffer& vb, Int row, Bool dopsf, Int nmaps)
{
  // If row is -1 then we pass through all rows
  Matrix<Float> newSumWeight(npol, nchan);
  sumWeight.resize(newSumWeight.shape());

  static Int passOne=0;
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
  // Take care of translation of Bools to Integer
  Int idopsf=0;
  if(dopsf) idopsf=1;

  Cube<Int> flags(vb.flagCube().shape());
  flags=0;
  flags(vb.flagCube())=True;
  Vector<Int> rowFlags(vb.nRow());
  rowFlags=0;
  rowFlags(vb.flagRow())=1; 
  if(!usezero_p) {
    for (Int rownr=startRow; rownr<=endRow; rownr++) {
      if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) rowFlags(rownr)=1;
    }
  }
  
 Int rowsave=row;
 for (Int nth=0; nth < nmaps; nth++){
   row=rowsave;
  // Get the uvws in a form that Fortran can use and do that
  // necessary phase rotation. On a Pentium Pro 200 MHz
  // when null, this step takes about 50us per uvw point. This
  // is just barely noticeable for Stokes I continuum and
  // irrelevant for other cases.
  // if (passOne == 0){
  passOne = 1;
  Matrix<Double> uvw(3, vb.uvw().nelements());
  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());
  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
  }
   if(uvwMachine_p) delete uvwMachine_p; uvwMachine_p=0;
   mImage_p=mImageHolder[nth];


   uvwMachine_p=new UVWMachine(mImage_p, vb.phaseCenter(), mFrame_p, False, True); 
  rotateUVW(uvw, dphase, vb);
  nx=nxPixels[nth];
  ny=nyPixels[nth];
  isTiled=False;
  if(isTiled) {
    Double invLambdaC=vb.frequency()(nvischan/2)/C::c;
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
       Matrix<Float> counter(npol, nchan);
        counter=0.0;
        Vector<Double> actualOffset(2);
        for (Int i=0;i<2;i++) {
          actualOffset(i)=uvOffset(i)-Double(offsetLoc(i));
	}
	IPosition s(vb.visCube().shape());
        // Now pass all the information down to a 
	// FORTRAN routine to do the work
	ggridft(uvw.getStorage(del),
		dphase.getStorage(del),
		vb.visCube().getStorage(del),
                &s(0),
		&s(1),
		&idopsf,
		flags.getStorage(del),
		rowFlags.getStorage(del),
		vb.imagingWeight().getStorage(del),
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
		&(gridder->cSupport()(0)),
		&(gridder->cSampling()),
		gridder->cFunction().getStorage(del),
		chanMap.getStorage(del),
		polMap.getStorage(del),
		sumWeight.getStorage(del));
      }
    }
  }
  else {
    Bool del;
    //   Matrix<Float> counter(npol, nchan);
   newSumWeight=*(sumWeightPtr[nth]);
   convertArray(sumWeight,newSumWeight);
    IPosition s(vb.visCube().shape());
    ggridft(uvw.getStorage(del),
	    dphase.getStorage(del),
	    vb.visCube().getStorage(del),
	    &s(0),
	    &s(1),
	    &idopsf,
	    flags.getStorage(del),
	    rowFlags.getStorage(del),
	    vb.imagingWeight().getStorage(del),
	    &s(2),
	    &row,
	    uvScale.getStorage(del),
	    uvOffset.getStorage(del),
	    (*griddedDataNumber[nth]).getStorage(del),
	    &nx,
	    &ny,
	    &npol,
	    &nchan,
	    vb.frequency().getStorage(del),
	    &C::c,
	    &((gridderNumber[nth])->cSupport()(0)),
      	    &((gridderNumber[nth])->cSampling()),
	    (gridderNumber[nth])->cFunction().getStorage(del),
	    chanMap.getStorage(del),
	    polMap.getStorage(del),
	    sumWeight.getStorage(del));
    convertArray(newSumWeight, sumWeight);
    (*(sumWeightPtr[nth]))=newSumWeight;
    

  }
 }





}

void WFGridFT::changeMapNumber(Int& i){

mapNumber_p=i;
image=imagePtr[i];
//maxAbsData=maxAbsDataNumber[i];
arrayLattice=arrayLatticeNumber[i];
gridder=gridderNumber[i];
griddedData.resize();
griddedData = (*(griddedDataNumber[i]));

}
void WFGridFT::initMulti(Int nmaps, Int nth ) {

  logIO() << LogOrigin("GridFT", "init")  << LogIO::NORMAL;

  


  ok();

  // Padding is possible only for non-tiled processing
  //if((padding_p*padding_p*image->shape().product())>cachesize) {
    //isTiled=True;
    //nx    = image->shape()(0);
    //ny    = image->shape()(1);
    //npol  = image->shape()(2);
    //nchan = image->shape()(3);
    //}
    //else {
    // We are padding.
  isTiled=False;
  nx    = Int(padding_p*Float(image->shape()(0)));
  ny    = Int(padding_p*Float(image->shape()(1)));
  npol  = image->shape()(2);
  nchan = image->shape()(3);
    //  }
    
  sumWeight.resize(npol, nchan);
  Matrix<Float> * tempsum;
  tempsum=sumWeightPtr[nth];
 
  //  if(tempsum) delete tempsum;
  if(!tempsum){
    tempsum= new Matrix<Float> (npol, nchan);
    sumWeightPtr[nth]=tempsum;
  }
  else{
    tempsum->resize(npol, nchan);
  }

  uvScale.resize(2);
  uvScale(0)=(Float(nx)*image->coordinates().increment()(0)); 
  uvScale(1)=(Float(ny)*image->coordinates().increment()(1)); 
  uvOffset.resize(2);
  uvOffset(0)=nx/2;
  uvOffset(1)=ny/2;

  // Now set up the gridder. The possibilities are BOX and SF
  //  gridderNumber.resize(nmaps);
  //  imagePtr.resize(nmaps);

  gridder= (gridderNumber[nth]);
  if(gridder) delete gridder; 
    //     gridder=0;
  gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						 uvScale, uvOffset,
						 convType); 
  gridderNumber[nth]=gridder;
  // Set up image cache needed for gridding. For BOX-car convolution
  // we can use non-overlapped tiles. Otherwise we need to use
  // overlapped tiles and additive gridding so that only increments
  // to a tile are written.
  if(imageCache) delete imageCache; imageCache=0;

  if(isTiled) {
    Float tileOverlap=0.5;
    if(convType=="BOX") {
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
    Int tmpCacheVal=(Int) cachesize;
    imageCache=new LatticeCache <Complex> (*image, tmpCacheVal, tileShape, 
					   tileOverlapVec,
					   (tileOverlap>0.0));

  }



}


void WFGridFT::multiGet(VisBuffer& vb, Int row, Int nmaps)
{
  // If row is -1 then we pass through all rows
  
  Int startRow, endRow, nRow;
  if (row==-1) {
    nRow=vb.nRow();
    startRow=0;
    endRow=nRow-1;
    //    vb.modelVisCube()=Complex(0.0,0.0);
  } else {
    nRow=1;
    startRow=row;
    endRow=row;
    //   vb.modelVisCube().xyPlane(row)=Complex(0.0,0.0);
  }
 Int rowsave=row;

 // Cube<Complex> data(vb.modelVisCube());
 Cube<Complex> adder(vb.modelVisCube().shape(), Complex(0.0, 0.0));
 // data=Complex(0.0, 0.0);


 Vector<Int> rowFlags(vb.nRow());

 rowFlags.set(0);
  if(!usezero_p) {
    for (Int rownr=startRow; rownr<=endRow; rownr++) {
      if((vb.antenna1()(rownr)==vb.antenna2()(rownr)) || vb.flagRow()[rownr]) rowFlags(rownr)=1;
    }
  }

 for (Int nth=0; nth < nmaps ; nth=nth+1){


  adder.set(Complex(0.0, 0.0));
  row=rowsave;
  if(maxAbsDataNumber[nth]==0.0) continue;


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


   if(uvwMachine_p) delete uvwMachine_p; uvwMachine_p=0;
   mImage_p=mImageHolder[nth];


   uvwMachine_p=new UVWMachine(mImage_p, vb.phaseCenter(), mFrame_p, False, True); 

   rotateUVW(uvw, dphase, vb);

   nx=nxPixels[nth];
   ny=nyPixels[nth];
   
   Cube<Int> flags(vb.flagCube().shape());
   flags=0;
  flags(vb.flagCube())=True;
  isTiled=False;

  if(isTiled) {
    Double invLambdaC=vb.frequency()(nvischan/2)/C::c;
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
	IPosition s(vb.modelVisCube().shape());
	dgridft(uvw.getStorage(del),
		dphase.getStorage(del),
		adder.getStorage(del),
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
		&(gridder->cSupport()(0)),
		&(gridder->cSampling()),
		gridder->cFunction().getStorage(del),
		chanMap.getStorage(del),
		polMap.getStorage(del));
      }
    }
  }
  else {
    Bool del;
    IPosition s(vb.modelVisCube().shape());
    dgridft(uvw.getStorage(del),
	    dphase.getStorage(del),
	    adder.getStorage(del),
	    &s(0),
	    &s(1),
	    flags.getStorage(del),
	    rowFlags.getStorage(del),
	    &s(2),
	    &row,
	    uvScale.getStorage(del),
	    uvOffset.getStorage(del),
	    (*(griddedDataNumber[nth])).getStorage(del),
	    &nx,
	    &ny,
	    &npol,
	    &nchan,
	    vb.frequency().getStorage(del),
	    &C::c,
	    &((gridderNumber[nth])->cSupport()(0)),
	    &((gridderNumber[nth])->cSampling()),
	    (gridderNumber[nth])->cFunction().getStorage(del),
	    chanMap.getStorage(del),
	    polMap.getStorage(del));
  }
  
   vb.modelVisCube() += adder;
  
 }

}


} //# NAMESPACE CASA - END

