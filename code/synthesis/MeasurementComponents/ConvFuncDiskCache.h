// -*- C++ -*-
//# ConvFuncDiskCache.cc: Definition of the ConvFuncDiskCache class
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
#ifndef SYNTHESIS_CONVFUNCDISKCACHE_H
#define SYNTHESIS_CONVFUNCDISKCACHE_H

#include <casa/Arrays/Matrix.h>
#include <msvis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <synthesis/TransformMachines/VPSkyJones.h>
// Apparently not required here? (gmoellen 06Nov20)
//#include <synthesis/MeasurementComponents/EPTimeVarVisJones.h>
#include <synthesis/TransformMachines/Utils.h>
namespace casa { //# NAMESPACE CASA - BEGIN
  // <summary> 
  //
  // An object to manage the caches of pre-computed convolution
  // functions on the disk and in the memory.
  //
  // </summary>
  
  // <use visibility=export>
  
  // <reviewed reviewer="" date="" tests="" demos="">
  
  // <prerequisite>
  //  No pre-requisites.  
  // </prerequisite>
  //
  // <etymology> 
  //
  // ConvFuncDiskCache is an object, to write convolution
  // functions from the memory cache to the disk cache, and
  // search/load the disk cache for convolution functions for a give
  // Parallactic Angle.  
  //
  //</etymology>
  //
  // <synopsis> 
  //
  // FTMachines uses pre-computed convolution functions for gridding
  // (FTMachine::put()) and prediction (FTMachine::get()).  For
  // <linkto class=PBWProjectFT>PBWProjectFT</linkto>, the convolution
  // function computation is expensive.  Once computed, it is
  // efficient to cache these functions as a function of Parallactic
  // Angle and the value of the w-term (if significant). 
  //
  // ConvFuncDiskCache class provides interface to the disk cache and
  // is used in <linkto class=PBWProjectFT>PBWProjectFT</linkto> to
  // search and load convolution functions from the disk.  If a new
  // convolution function is computed in <linkto
  // class=PBWProjectFT>PBWProjectFT</linkto>, the disk cache is
  // updated using the services of this class as well.
  //
  // </synopsis> 
  //
  // <example>
  // </example>
  //
  // <motivation>
  //
  // Factor out the code for managing convolution function caches
  // (memory and disk caches) from the FTMachine code.  This is a
  // specialized service, and has nothing to do with the details of
  // gridding and prediction of visibilities (which is the function of
  // FTMachines).
  //
  // </motivation>
  //
  // <todo asof="2005/07/21">
  //
  // <ul> Generalize to handle convolution functions for inhomogeneous
  // arrays and multi-feed antennas.
  //
  // </todo>

  class ConvFuncDiskCache
  {
  public:
    ConvFuncDiskCache():paList(),XSup(),YSup(), cfPrefix("CF"), aux("aux.dat") {};
    ConvFuncDiskCache& operator=(const ConvFuncDiskCache& other);
    ~ConvFuncDiskCache() {};
    void setCacheDir(const char *dir) {Dir = dir;}
    void initCache();
    void cacheConvFunction(Int which, Float pa, Array<Complex>& cf, CoordinateSystem& coords,
			   CoordinateSystem& ftcoords, Int& convSize, Cube<Int>& convSupport, 
			   Float convSampling, String nameQualifier="",Bool savePA=True);
    void cacheWeightsFunction(Int which, Float pa, Array<Complex>& cfWt, CoordinateSystem& coords,
			      Int& convSize, Cube<Int>& convSupport, Float convSampling);
    Bool searchConvFunction(const VisBuffer& vb, VPSkyJones& vpSJ, Int& which, Float &pa);
    Bool searchConvFunction(const VisBuffer& vb, ParAngleChangeDetector& vpSJ, 
			    Int& which, Float &pa);
    Bool loadConvFunction(Int where, Int Nx, PtrBlock < Array<Complex> *> & convFuncCache,
			  Cube<Int> &convSupport, Vector<Float>& convSampling,
			  Double& cfRefFreq,CoordinateSystem& coordys, String prefix="/CF");
    void finalize();
    void finalize(ImageInterface<Float>& avgPB);
    void loadAvgPB(ImageInterface<Float>& avgPB);
  private:
    Vector<Float> paList, Sampling;
    Cube<Int> XSup, YSup;
    String Dir, cfPrefix, aux;
  };
}

#endif
