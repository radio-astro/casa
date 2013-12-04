//# GridBoth.h: Definition for GridBoth
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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

#ifndef SYNTHESIS_GRIDBOTH_H
#define SYNTHESIS_GRIDBOTH_H

#include <synthesis/TransformMachines/GridFT.h>
#include <synthesis/MeasurementComponents/SDGrid.h>

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
// FTMachine is a Machine for Fourier Transforms. GridBoth does
// Single Dish gridding in a similar way
// </etymology>
//
// <synopsis> 
// The <linkto class=SkyEquation>SkyEquation</linkto> needs to be able
// to perform Fourier transforms on visibility data and to grid
// single dish data.
// GridBoth allows efficient Single Dish processing using a 
// <linkto class=VisBuffer>VisBuffer</linkto> which encapsulates
// a chunk of visibility (typically all baselines for one time)
// together with all the information needed for processing
// (e.g. direction coordinates).
//
// Gridding and degridding in GridBoth are performed using a
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

template<class T> class TempImage;

class GridBoth : public FTMachine {
public:

  // Constructor: cachesize is the size of the cache in words
  // (e.g. a few million is a good number), tilesize is the
  // size of the tile used in gridding (cannot be less than
  // 12, 16 works in most cases), and convType is the type of
  // gridding used (SF is prolate spheriodal wavefunction,
  // and BOX is plain box-car summation). mLocation is
  // the position to be used in some phase rotations. If
  // mTangent is specified then the uvw rotation is done for
  // that location iso the image center.
  // <group>
  GridBoth(SkyJones& sj, Long cachesize, Int tilesize,
	   String sdConvType="BOX",
	   String synConvType="SF",
	   Float padding=1.0,
	   Float sdScale=1.0,
	   Float sdWeight=1.0);
  GridBoth(SkyJones& sj, Long cachesize, Int tilesize,
	   MPosition mLocation,
	   String sdConvType="BOX",
	   String synConvType="SF",
	   Float padding=1.0,
	   Float sdScale=1.0,
	   Float sdWeight=1.0);
  GridBoth(SkyJones& sj, Long cachesize, Int tilesize,
	   MPosition mLocation, MDirection mTangent,
	   String sdConvType="BOX",
	   String synConvType="SF",
	   Float padding=1.0,
	   Float sdScale=1.0,
	   Float sdWeight=1.0);
  // </group>

  // Copy constructor
  GridBoth(const GridBoth &other);

  // Assignment operator
  GridBoth &operator=(const GridBoth &other);

  ~GridBoth();

  // Construct from a Record containing the GridFT state
  GridBoth(const RecordInterface& stateRec);

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
    {throw(AipsError("GridBoth::normalizeImage() called"));}

  // Get the final weights image
  void getWeightImage(ImageInterface<Float>&, Matrix<Float>&);

  // Save and restore the GridFT to and from a record
  virtual Bool toRecord(String& error, RecordInterface& outRec, 
			Bool withImage=False, const String diskimage="");
  virtual Bool fromRecord(String& error, const RecordInterface& inRec);

  // Has this operator changed since the last application?
  virtual Bool changed(const VisBuffer& vb);
  virtual void setMiscInfo(const Int qualifier){(void)qualifier;};
  virtual void ComputeResiduals(VisBuffer&/*vb*/, Bool /*useCorrected*/) {};
  virtual String name() const { return "GridBoth";};

private:

  FTMachine* synMachine_p; // Synthesis machine
  FTMachine* sdMachine_p;  // Single Dish machine
  FTMachine* lastMachine_p; // Last Machine used

  // Images for Synthesis and SD 
  TempImage<Complex>* sdImage_p;
  TempImage<Complex>* synImage_p;

  Float sdScale_p, sdWeight_p;

  void ok();

  void init();

};

} //# NAMESPACE CASA - END

#endif
