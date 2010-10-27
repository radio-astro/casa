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
#ifndef SYNTHESIS_CFCACHE_H
#define SYNTHESIS_CFCACHE_H

#include <casa/Arrays/Matrix.h>
#include <msvis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <synthesis/MeasurementComponents/VPSkyJones.h>
#include <synthesis/MeasurementComponents/CFStore.h>
#include <synthesis/MeasurementComponents/CFDefs.h>
#include <synthesis/MeasurementComponents/Utils.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  using namespace CFDefs;
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
  // CFDiskCache is an object, to write convolution
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
  // CFDiskCache class provides interface to the disk cache and
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

  class CFDiskCache
  {
  public:
    typedef Array<Complex> CFType ;
    typedef PtrBlock< CFType *> CFCacheType;
    typedef PtrBlock< CFStore *> CFStoreCacheType;
    CFDiskCache():
      logIO_p(), memCache_p(), XSup(), YSup(), paList(), key2IndexMap(),
      cfPrefix("CF"), aux("aux.dat") 
    {};
    CFDiskCache& operator=(const CFDiskCache& other);
    ~CFDiskCache();
    void setCacheDir(const char *dir) {Dir = dir;}
    void initCache();
    void setPAChangeDetector(const ParAngleChangeDetector& paCD) {paCD_p=paCD;};
    void cacheConvFunction(Int which, Float pa, Array<Complex>& cf, CoordinateSystem& coords,
			   CoordinateSystem& ftcoords, Int& convSize, Cube<Int>& convSupport, 
			   Float convSampling, String nameQualifier="",Bool savePA=True);
    void cacheWeightsFunction(Int which, Float pa, Array<Complex>& cfWt, CoordinateSystem& coords,
			      Int& convSize, Cube<Int>& convSupport, Float convSampling);
    Bool searchConvFunction(const VisBuffer& vb, VPSkyJones& vpSJ, Int& which, Float &pa);
    Bool searchConvFunction(const VisBuffer& vb, const ParAngleChangeDetector& vpSJ, 
			    Int& which, Float &pa);
    Bool loadConvFunction(Int where, Int Nx, CFStoreCacheType & convFuncCache,
			  CFStore& cfs,
			  // Cube<Int> &convSupport, Vector<Float>& convSampling,
			  // Double& cfRefFreq,CoordinateSystem& coordys, 
			  String prefix="/CF");
      
    Int locateConvFunction(const Int Nw, const VisBuffer& vb,
			   const ParAngleChangeDetector& paCD,
			   CFStore& cfs
			   // CFType& convFunc,
			   // Cube<Int> &convSupport,
			   // Vector<Float>& convSampling,
			   // Double& cfRefFreq, 
			   // CoordinateSystem& coordSys
			   );
    Int locateConvFunction(const Int Nw, 
			   const VisBuffer& vb,
			   CFStore& cfs
			   // CFType& convFunc,
			   // Cube<Int> &convSupport,
			   // Vector<Float>& convSampling,
			   // Double& cfRefFreq, 
			   // CoordinateSystem& coordSys
			   )
    {return locateConvFunction(Nw, vb, paCD_p, cfs// convFunc, convSupport, convSampling,
			       // cfRefFreq, coordSys
			       );}
    void finalize();
    void finalize(ImageInterface<Float>& avgPB);
    void loadAvgPB(ImageInterface<Float>& avgPB);
    enum CACHETYPE {DISKCACHE=1, MEMCACHE};
  protected:
    LogIO logIO_p;
    LogIO& logIO() {return logIO_p;};

  private:
    CFStoreCacheType memCache_p;
    Matrix<Int> XSup, YSup;
    Vector<Float> paList, Sampling;
    Matrix<Float> key2IndexMap; // Nx2 [PAVal, Freq]
    String Dir, cfPrefix, aux;
    ParAngleChangeDetector paCD_p;
    void ftCoordSys(const CoordinateSystem& coords,
		    const Int& convSize,
		    const Vector<Double>& ftRef,
		    CoordinateSystem& ftCoords);

  };
}

#endif
