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
#include <synthesis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/TempImage.h>
#include <images/Images/PagedImage.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <synthesis/TransformMachines/VPSkyJones.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/CFDefs.h>
#include <synthesis/TransformMachines/Utils.h>
// #include <casa/Tables/Table.h>

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
  //  CFStore class
  // </prerequisite>
  //
  // <etymology> 
  //
  // CFCache is an object, to write convolution
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
  // CFCache class provides interface to the disk cache and
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
  //
  //----------------------------------------------------------------------
  class CFCacheTable
  {
  public:
    CFCacheTable(): freqList(), wList(), muellerList(), cfNameList() {};
    ~CFCacheTable() {};
    
    CFCacheTable& operator=(const CFCacheTable& other)
    {
      //      if (other != *this)
	{
	  freqList = other.freqList;
	  wList = other.wList;
	  muellerList = other.muellerList;
	  cfNameList = other.cfNameList;
	}
      return *this;
    }

    vector<Double> freqList, wList;
    vector<Int> muellerList;
    vector<String> cfNameList;
  };
  //
  //----------------------------------------------------------------------
  //
  class CFCache
  {
  public:
    typedef Vector< CFStore > CFStoreCacheType;
    typedef vector<CFCacheTable> CFCacheTableType;
    CFCache(const char *cfDir="CF"):
      memCache_p(), memCacheWt_p(), cfCacheTable_p(), XSup(), YSup(), paList(), 
      paList_p(), key2IndexMap(),
      Dir(""), cfPrefix(cfDir), aux("aux.dat"), paCD_p(), avgPBReady_p(False),
      avgPBReadyQualifier_p("")
    {};
    CFCache& operator=(const CFCache& other);
    ~CFCache();
    //
    // Method to set the disk cache directory name
    //
    void setCacheDir(const char *dir) {Dir = dir;}
    String getCacheDir() {return Dir;};
    //
    // Method to initialize the internal memory cache.
    //
    void initCache();
    void initCache2();
    //
    // Compute the size of the memory cache in bytes
    //
    Long size();
    //
    // Method to set the class to caluclate the differential
    // Parallactic Angle.  The ParAngleChangeDetector also holds the
    // delta PA value (user defined).
    //
    void setPAChangeDetector(const ParAngleChangeDetector& paCD) {paCD_p = paCD;};
    //
    // Methods to cache the convolution function.
    //
    // Top level method interfacing with the CFStore object
    //-------------------------------------------------------------------
    void cacheConvFunction(CFStore& cfs, 
			   String nameQualifier="",Bool savePA=True)
    {cacheConvFunction(cfs.pa, cfs,nameQualifier,savePA);}
    //-------------------------------------------------------------------
    // One level lower - the Parallactic angle can be separately
    // provided.
    void cacheConvFunction(const Quantity pa, CFStore& cfs,
			   String nameQualifier="",Bool savePA=True)
    {cacheConvFunction(pa.getValue("rad"), cfs, nameQualifier,savePA);}
    //-------------------------------------------------------------------
    // The Parallactic angle as a floating point number in radians.
    void cacheConvFunction(const Float pa, CFStore& cfs, 
			   String nameQualifier="",Bool savePA=True);
    //-------------------------------------------------------------------
    // Lowest level - all information about CFStore is explicitly
    // provided as basic types
    Int cacheConvFunction(Int which, const Float& pa, CFType& cf, 
			  CoordinateSystem& coords, CoordinateSystem& ftcoords, 
			  Int& convSize, 
			  Vector<Int>& xConvSupport, Vector<Int>& yConvSupport, 
			  Float convSampling, String nameQualifier="",Bool savePA=True);
    //-------------------------------------------------------------------
    // Methods to sarch for a convolution function in the caches (disk
    // or memory) for the give Parallactic Angle value.
    //
    Bool searchConvFunction(Int& which, const Quantity pa, const Quantity dPA )
    {return searchConvFunction(which, pa.getValue("rad"), dPA.getValue("rad"));};

    Bool searchConvFunction(Int& which, const Float pa, const Float dPA );
    //
    // Lower level method to load a convolution function from the disk.
    //
    Int loadFromDisk(Int where, Float pa, Float dPA,
		     Int Nx, CFStoreCacheType & convFuncCache,
		     CFStore& cfs, String nameQualifier="");
    //
    // Method to locate a convolution function for the given w-term
    // index and PA value.  This is the top level function that must
    // be used by the clients.  This uses searchConvFunction() and
    // loadFromDisk() methods and the private methods to return a
    // convolution function.
    //
    // Returns CFDefs::NOTCACHED if the convolution function was not
    // found in the cache, CFDefs::MEMCACHE or CFDefs::DISKCACHE if
    // the function was found in memory or disk cache respectively.
    //
    Int locateConvFunction(CFStore& cfs, CFStore& cftws, const Int Nw, 
			   const Quantity pa, const Quantity dPA, 
			   const Int mosXPos=0, const Int mosYPos=0)
    {return locateConvFunction(cfs, cftws, Nw,pa.getValue("rad"), dPA.getValue("rad"),mosXPos,mosYPos);};

    Int locateConvFunction(CFStore& cfs, const Int Nw, 
			   const Quantity pa, const Quantity dPA, 
			   const String& nameQualifier="",
			   const Int mosXPos=0, const Int mosYPos=0)
    {return locateConvFunction(cfs, Nw,pa.getValue("rad"), dPA.getValue("rad"),nameQualifier, mosXPos,mosYPos);};

    Int locateConvFunction(CFStore& cfs, CFStore& cfwts,
			   const Int Nw, const Float pa, const Float dPA,
			   const Int mosXPos=0, const Int mosYPos=0);

    Int locateConvFunction(CFStore& cfs, const Int Nw, const Float pa, const Float dPA, 
			   const String& nameQualifier="",
			   const Int mosXPos=0, const Int mosYPos=0);
    //
    // Methods to write the auxillary information from the memory
    // cache to the disk cache.  Without this call, the disk cache
    // might not be complete.  It is safe to call this method at
    // anytime during the life of this object.
    //
    void flush();
    void flush(ImageInterface<Float>& avgPB, String qualifier=String(""));
    Int loadAvgPB(ImageInterface<Float>& avgPB, String qualifier=String(""));
    Int loadAvgPB(CountedPtr<ImageInterface<Float> > & avgPB, String qualifier=String(""))
    {if (avgPB.null()) avgPB = new TempImage<Float>(); return loadAvgPB(*avgPB,qualifier);};

    Bool avgPBReady(const String& qualifier=String("")) 
    {return (avgPBReady_p && (avgPBReadyQualifier_p == qualifier));};

  private:
    CFStoreCacheType memCache_p, memCacheWt_p;
    CFCacheTableType cfCacheTable_p;

    Matrix<Int> XSup, YSup;
    Vector<Float> paList, Sampling;
    vector<Float> paList_p;
    Matrix<Float> key2IndexMap; // Nx2 [PAVal, Freq]
    String Dir, cfPrefix, aux;
    ParAngleChangeDetector paCD_p;
    //
    // Internal method to convert the direction co-ordinates of the
    // given CoordinateSystem to its Fourier conjuguate co-ordinates.
    //
    void makeFTCoordSys(const CoordinateSystem& coords,
			const Int& convSize,
			const Vector<Double>& ftRef,
			CoordinateSystem& ftCoords);
    //
    // Internal method to add the given convolution function to the
    // memory cache.
    //
    Int addToMemCache(CFStoreCacheType& cfCache, 
		      Float pa, CFType* cf, CoordinateSystem& coords,
		      Vector<Int>& xConvSupport,
		      Vector<Int>& yConvSupport,
		      Float convSampling);
    CFStoreCacheType& getMEMCacheObj(const String& nameQualifier);

    Bool avgPBReady_p;
    String avgPBReadyQualifier_p;
  };
}

#endif
