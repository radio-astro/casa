//# Mosaicft.cc: Implementation of MosaicFTNew class
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

#include <synthesis/TransformMachines/MosaicFTNew.h>


#include <msvis/MSVis/VisibilityIterator.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/UnitVal.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/UVWMachine.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>
#include <synthesis/TransformMachines/SimplePBConvFunc.h>
#include <synthesis/TransformMachines/HetArrayConvFunc.h>
#include <synthesis/TransformMachines/PBMath.h>
#include <synthesis/TransformMachines/VPSkyJones.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
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
#include <lattices/LRegions/LCBox.h>
#include <lattices/LEL/LatticeExpr.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/LatticeMath/LatticeFFT.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <casa/Utilities/CompositeNumber.h>
#include <casa/OS/Timer.h>
#include <casa/OS/HostInfo.h>
#include <casa/sstream.h>
#ifdef _OPENMP
#include <omp.h>
#endif
namespace casa { //# NAMESPACE CASA - BEGIN

  FTMachine* MosaicFTNew::cloneFTM(){
    return new MosaicFTNew(*this);
  }

// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& MosaicFTNew::getImage(Matrix<Float>& weights,
					    Bool normalize) 
{
  //AlwaysAssert(lattice, AipsError);
  AlwaysAssert(image, AipsError);

   if((griddedData.nelements() ==0) && (griddedData2.nelements()==0)){
    throw(AipsError("Programmer error ...request for image without right order of calls"));
  }

  logIO() << LogOrigin("MosaicFTNew", "getImage") << LogIO::NORMAL;
  
  weights.resize(sumWeight.shape());
  
  convertArray(weights, sumWeight);
  
  // If the weights are all zero then we cannot normalize
  // otherwise we don't care.
  if(max(weights)==0.0) {
    if(normalize) {
      logIO() << LogIO::SEVERE << "No useful data in MosaicFTNew: weights all zero"
	      << LogIO::POST;
    }
    else {
      logIO() << LogIO::WARN << "No useful data in MosaicFTNew: weights all zero"
	      << LogIO::POST;
    }
  }
  else {
    
    //const IPosition latticeShape = lattice->shape();
    
    logIO() << LogIO::DEBUGGING
	    << "Starting FFT and scaling of image" << LogIO::POST;
    if(useDoubleGrid_p){
      ArrayLattice<DComplex> darrayLattice(griddedData2);
      LatticeFFT::cfft2d(darrayLattice,False);
      griddedData.resize(griddedData2.shape());
      convertArray(griddedData, griddedData2);
      
      //Don't need the double-prec grid anymore...
      griddedData2.resize();
      arrayLattice = new ArrayLattice<Complex>(griddedData);
      lattice=arrayLattice;

    }
    else{
      arrayLattice = new ArrayLattice<Complex>(griddedData);
      lattice=arrayLattice;
      LatticeFFT::cfft2d(*lattice,False);
    }
    
    {
      Int inx = lattice->shape()(0);
      Int iny = lattice->shape()(1);
      Vector<Complex> correction(inx);
      correction=Complex(1.0, 0.0);

      IPosition cursorShape(4, inx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
      for(lix.reset();!lix.atEnd();lix++) 
	{
	Int pol=lix.position()(2);
	Int chan=lix.position()(3);
	if(normalize) 
	  {
	    if(weights(pol, chan)!=0.0) 
	      {
		Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
		lix.rwCursor()*=rnorm;
	      }
	    else {
	      lix.woCursor()=0.0;
	    }
	}	  else {
	  Complex rnorm(Float(inx)*Float(iny));
	  lix.rwCursor()*=rnorm;
	}
      }
    }

    //if(!isTiled) 
    {
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
  griddedData.resize();
  image->clearCache();
  return *image;
}

// Get weight image
void MosaicFTNew::getWeightImage(ImageInterface<Float>& weightImage,
			      Matrix<Float>& weights) 
{
  
  logIO() << LogOrigin("MosaicFTNew", "getWeightImage") << LogIO::NORMAL;
  
  weights.resize(sumWeight.shape());
  convertArray(weights,sumWeight);

  Float inx = skyCoverage_p->shape()(0);
  Float iny = skyCoverage_p->shape()(1);

  weightImage.copyData( (LatticeExpr<Float>) ( (*skyCoverage_p)*inx*iny ) );
 

}

} //# NAMESPACE CASA - END


