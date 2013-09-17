//# SDGrid.h: Definition for SDGrid
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_SDGRID_H
#define SYNTHESIS_SDGRID_H

#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/TransformMachines/SkyJones.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/FFTServer.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/Measure.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> An FTMachine for Gridding Single Dish data
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=FTMachine>FTMachine</linkto> module
//   <li> <linkto class=SkyEquation>SkyEquation</linkto> module
//   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
// </prerequisite>
//
// <etymology>
// FTMachine is a Machine for Fourier Transforms. SDGrid does
// Single Dish gridding in a similar way
// </etymology>
//
// <synopsis> 
// The <linkto class=SkyEquation>SkyEquation</linkto> needs to be able
// to perform Fourier transforms on visibility data and to grid
// single dish data.
// SDGrid allows efficient Single Dish processing using a 
// <linkto class=VisBuffer>VisBuffer</linkto> which encapsulates
// a chunk of visibility (typically all baselines for one time)
// together with all the information needed for processing
// (e.g. direction coordinates).
//
// Gridding and degridding in SDGrid are performed using a
// novel sort-less algorithm. In this approach, the gridded plane is
// divided into small patches, a cache of which is maintained in memory
// using a general-purpose <linkto class=LatticeCache>LatticeCache</linkto> class. As the (time-sorted)
// visibility data move around slowly in the image plane, patches are
// swapped in and out as necessary. Thus, optimally, one would keep at
// least one patch per scan line of data.
//
// A grid cache is defined on construction. If the gridded image plane is smaller
// than this, it is kept entirely in memory and all gridding and
// degridding is done entirely in memory. Otherwise a cache of tiles is
// kept an paged in and out as necessary. Optimally the cache should be
// big enough to hold all polarizations and frequencies for one
// complete scan line.
// The paging rate will then be small. As the cache size is
// reduced below this critical value, paging increases. The algorithm will
// work for only one patch but it will be very slow!
//
// The gridding and degridding steps are implemented in Fortran
// for speed. In gridding, the visibilities are added onto the
// grid points in the neighborhood using a weighting function.
// In degridding, the value is derived by a weight summ of the
// same points, using the same weighting function.
// </synopsis> 
//
// <example>
// See the example for <linkto class=SkyModel>SkyModel</linkto>.
// </example>
//
// <motivation>
// Define an interface to allow efficient processing of chunks of 
// visibility data
// </motivation>
//
// <todo asof="97/10/01">
// <ul> Deal with large VLA spectral line case 
// </todo>

class SDGrid : public FTMachine {
public:

  // Constructor: cachesize is the size of the cache in words
  // (e.g. a few million is a good number), tilesize is the
  // size of the tile used in gridding (cannot be less than
  // 12, 16 works in most cases), and convType is the type of
  // gridding used (SF is prolate spheriodal wavefunction,
  // and BOX is plain box-car summation). mLocation is
  // the position to be used in some phase rotations. If
  // mTangent is specified then the uvw rotation is done for
  // that location iso the image center. userSupport is to allow 
  // larger support for the convolution if the user wants it ..-1 will 
  // use the default  i.e 1 for BOX and 3 for others
  // <group>
  SDGrid(SkyJones& sj, Int cachesize, Int tilesize,
	 String convType="BOX", Int userSupport=-1);
  SDGrid(MPosition& ml, SkyJones& sj, Int cachesize,
	 Int tilesize, String convType="BOX", Int userSupport=-1,
	 Float minweight=0.);
  SDGrid(Int cachesize, Int tilesize,
	 String convType="BOX", Int userSupport=-1);
  SDGrid(MPosition& ml, Int cachesize, Int tilesize,
	 String convType="BOX", Int userSupport=-1, Float minweight=0.);
  SDGrid(MPosition& ml, Int cachesize, Int tilesize,
	 String convType="TGAUSS", Float truncate=-1.0, 
         Float gwidth=0.0, Float jwidth=0.0, Float minweight=0.);
  // </group>

  // Copy constructor
  SDGrid(const SDGrid &other);

  // Assignment operator
  SDGrid &operator=(const SDGrid &other);

  ~SDGrid();

  // Initialize transform to Visibility plane using the image
  // as a template. The image is loaded and Fourier transformed.
  void initializeToVis(ImageInterface<Complex>& image,
		       const VisBuffer& vb);

  // Finalize transform to Visibility plane: flushes the image
  // cache and shows statistics if it is being used.
  void finalizeToVis();

  // Initialize transform to Sky plane: initializes the image
  void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
		       const VisBuffer& vb);

  // Finalize transform to Sky plane: flushes the image
  // cache and shows statistics if it is being used. DOES NOT
  // DO THE FINAL TRANSFORM!
  void finalizeToSky();

  // Get actual coherence from grid by degridding
  void get(VisBuffer& vb, Int row=-1);

  // Put coherence to grid by gridding.
  void put(const VisBuffer& vb, Int row=-1, Bool dopsf=False,
	   FTMachine::Type type=FTMachine::OBSERVED);

  // Get the final image: do the Fourier transform and
  // grid-correct, then optionally normalize by the summed weights
  ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);
  virtual void normalizeImage(Lattice<Complex>& /*skyImage*/,
			      const Matrix<Double>& /*sumOfWts*/,
			      Lattice<Float>& /*sensitivityImage*/,
			      Bool /*fftNorm*/)
    {throw(AipsError("SDGrid::normalizeImage() called"));}

  // Get the final weights image
  void getWeightImage(ImageInterface<Float>&, Matrix<Float>&);

  // Has this operator changed since the last application?
  virtual Bool changed(const VisBuffer& vb);
  virtual void setMiscInfo(const Int qualifier){(void)qualifier;};
  virtual void ComputeResiduals(VisBuffer& /*vb*/, Bool /*useCorrected*/) {};

  virtual String name() const;

private:

  // Find the Primary beam and convert it into a convolution buffer
  void findPBAsConvFunction(const ImageInterface<Complex>& image,
			    const VisBuffer& vb);

  SkyJones* sj_p;

  // Get the appropriate data pointer
  Array<Complex>* getDataPointer(const IPosition&, Bool);
  Array<Float>* getWDataPointer(const IPosition&, Bool);

  void ok();

  void init();

  // Image cache
  LatticeCache<Complex> * imageCache;
  LatticeCache<Float> * wImageCache;

  // Sizes
  Int cachesize, tilesize;

  // Is this tiled?
  Bool isTiled;

  // Storage for weights
  ImageInterface<Float>* wImage;

  // Array lattice
  Lattice<Complex> * arrayLattice;
  Lattice<Float> * wArrayLattice;

  // Lattice. For non-tiled gridding, this will point to arrayLattice,
  //  whereas for tiled gridding, this points to the image
  Lattice<Complex>* lattice;
  Lattice<Float>* wLattice;

  String convType;

  // Useful IPositions
  IPosition centerLoc, offsetLoc;

  // Array for non-tiled gridding
  Array<Complex> griddedData;
  Array<Float> wGriddedData;


  DirectionCoordinate directionCoord;

  MDirection::Convert* pointingToImage;

  Vector<Double> xyPos;
  //Original xypos of moving source
  Vector<Double> xyPosMovingOrig_p;

  MDirection worldPosMeas;

  Cube<Int> flags;

  Vector<Float> convFunc;
  Int convSampling;
  Int convSize;
  Int convSupport;
  Int userSetSupport_p;
  
  Float truncate_p;
  Float gwidth_p;
  Float jwidth_p;

  Float minWeight_p;

  Int lastIndex_p;

  Int getIndex(const ROMSPointingColumns& mspc, const Double& time,
	       const Double& interval);

  Bool getXYPos(const VisBuffer& vb, Int row);

  //get the MDirection from a chosen column of pointing table
  MDirection directionMeas(const ROMSPointingColumns& mspc, const Int& index);
  MDirection directionMeas(const ROMSPointingColumns& mspc, const Int& index, const Double& time);
  MDirection interpolateDirectionMeas(const ROMSPointingColumns& mspc, const Double& time,
                                  const Int& index, const Int& index1, const Int& index2);

  //for debugging
  //FILE *pfile;
};

} //# NAMESPACE CASA - END

#endif
