// -*- C++ -*-
//# CFBuffer.h: Definition of the CFBuffer class
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
#ifndef SYNTHESIS_CFBUFFER_H
#define SYNTHESIS_CFBUFFER_H
#include <synthesis/TransformMachines/SynthesisError.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <synthesis/TransformMachines/CFDefs.h>
#include <synthesis/TransformMachines/CFCell.h>
#include <synthesis/TransformMachines/Utils.h>
#include <images/Images/ImageInterface.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Utilities/Sort.h>
#include <casa/Logging/LogOrigin.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogIO.h>

//
// <summary> defines interface for the storage for convolution functions </summary>

// <prerequisite>
// </prerequisite>
//
// <etymology> 
//
//  CFBuffer is basic in-memory storage for convolution functions
//  as a function of polarization and frequency at a particular value
//  of Parallactic Angle.
//
//</etymology>
//
// <synopsis> 
//
// The CFBuffer class encapsulates the storage and associated
// auxillary information required for the convolution functions.  The
// <linto class=CFStore>CFStore</linkto> class is a collection of
// CFBuffer objects.  A collection of CFStore objects is held and
// managed by the <linto class=FTMachine>FTMachine</linkto> and the
// appropriate one, depending on the Time/PA value, polarization and
// frequency of the data in the <linkto
// class=VisBuffer>VisBuffer</linkto>, is supplied to the <linkto
// class=VisibilityResampler>VisibilityResampler</linkto> object of
// re-sampling the data onto a grid (or vice versa).
//
// This buffer holds the convolution functions for the required range
// of frequencies and polarizations at a particular Parallactic Angle
// (or time), for one W-value and for a single baseline (antenna
// pair).
//
// Conceptually, this object holds the convolution functions
// parameterized by the properties of the electromagnetic radiation
// (polarization state, frequency and the w-value which is the Fresnel
// term and implicitly a function of the frequency).  The <linkto
// class=CFStore>CFStore</linkto> object holds a list of this object
// index by the telescope related parameters (antenna1,
// antenna2, Parallactic Angle or Time, etc.).
//
//</synopsis>
//
// <example>
// </example>
//
// <motivation>
//
// To factor out the details of effecient in-memory storage of the
// convolution functions into a separate class.  This class can then
// be optmized by specializations for various types of convolution
// functions and imaging algorithms without the need to change the
// imaging framework.
//
// </motivation>
//


using namespace casa::CFDefs;
using namespace std;
namespace casa { //# NAMESPACE CASA - BEGIN
  //  template <class T>
  typedef Complex TT;

  struct CFBStruct {
    CFCStruct *CFBStorage;
    int shape[3];
    Double *freqValues, *wValues, *pointingOffset;
    Double fIncr, wIncr;
    Int **muellerElementsIndex, **muellerElements, 
      **conjMuellerElementsIndex, **conjMuellerElements,
      **conjFreqNdxMap, **freqNdxMap;
    Int nMueller;
    

    CFCStruct* getCFB(int i, int j, int k)
    { return &(CFBStorage[i + (shape[1]-1)*j + (shape[1]-1)*(shape[2]-1)*k]);}
    //    { return &(CFBStorage[i + (shape[1]-1)*j + (shape[2]-1)*k]);}
  } ;

  class CFBuffer
  {
  public:
    //
    //========================= Administrative Parts ==========================
    //------------------------------------------------------------------
    //
    CFBuffer(): wValues_p(), maxXSupport_p(-1), maxYSupport_p(-1), pointingOffset_p(), cfHitsStats(),
		freqNdxMapsReady_p(False), freqNdxMap_p(), conjFreqNdxMap_p()
    {};
    
    CFBuffer(Int maxXSup, Int maxYSup):
      wValues_p(), maxXSupport_p(maxXSup), maxYSupport_p(maxYSup), pointingOffset_p(), cfHitsStats(),
      freqNdxMapsReady_p(False), freqNdxMap_p(), conjFreqNdxMap_p()
    {
      // storage_p.resize(1,1,1); 
      // storage_p(0,0,0) = new Array<TT>(dataPtr);
      // coordSys_p.resize(1,1,1); 
      // coordSys_p(0,0,0) = cs;
    };
    
    ~CFBuffer() 
    {
      LogIO log_l(LogOrigin("CFBuffer","~CFBuffer[R&D]"));
      log_l << "CF Hits stats gathered: " << cfHitsStats << endl;
    };
    
    CountedPtr<CFBuffer> clone();
    void allocCells(const Cube<CountedPtr<CFCell> >& cells);
    void setParams(const CFBuffer& other);
    //
    //============================= Functional Parts ============================
    //------------------------------------------------------------------
    //
    //    CFBuffer& operator=(const CFBuffer& other);
    //
    // Get the single convolution function as an Array<T> for the
    // supplied value of the frequency and the muellerElement.
    // Mueller element is essentially the polarization product, but
    // can be any of the of 16 elements of the outer product.
    //
    //-------------------------------------------------------------------------
    //
    inline Int nChan() {return nChan_p;}
    inline Int nW() {return nW_p;}
    inline Int nMuellerElements() {return nPol_p;}
    inline IPosition shape() {IPosition shp(3,nChan_p, nW_p, nPol_p); return shp;}
    
    inline Vector<Double> getFreqList() {return freqValues_p;};
    inline Vector<Double> getWList() {return wValues_p;};
    
    CFCell& getCFCell(const Double& freqVal, const Double& wValue, 
		      const Int & muellerElement); 
    // muellerElement: (i,j) of the Mueller Matrix
    CountedPtr<CFCell>& getCFCellPtr(const Double& freqVal, const Double& wValue, 
				     const Int & muellerElement); 
    CFCell& operator()(const Int& i, const Int& j, const Int& k) {return *cfCells_p(i,j,k);}
    CFCell& getCFCell(const Int& i, const Int& j, const Int& k);

    CountedPtr<CFCell >& getCFCellPtr(const Int& i, const Int& j, const Int& k);
    
    //=========================================================================
    Array<TT>& getCF(const Double& freqVal, const Double& wValue, 
		     const Int & muellerElement)
    {return *(getCFCell(freqVal, wValue, muellerElement).storage_p);}
    // muellerElement: (i,j) of the Mueller Matrix
    
    CountedPtr<Array<TT> >& getCFPtr(const Double& freqVal, const Double& wValue, 
				     const Int & muellerElement) 
    {return getCFCellPtr(freqVal, wValue, muellerElement)->storage_p;}
    
    Array<TT>& getCF(const Int& i, const Int& j, const Int& k)
    {return *(getCFCell(i,j,k).storage_p);}
    
    CountedPtr<Array<TT> >& getCFPtr(const Int& i, const Int& j, const Int& k)
    {return getCFCellPtr(i,j,k)->storage_p;}
    
    
    //
    // Get the parameters of a the CFs indexed by values.  The version
    // which returns also the Coordinate System associated with the
    // CFs are slow (CoordinateSystem::operator=() is surprisingly
    // expensive!).  So do not use this in tight loops.  If it is
    // required, use the version without the co-ordinate system below.
    //
    void getParams(CoordinateSystem& cs, Float& sampling, 
		   Int& xSupport, Int& ySupport, 
		   const Double& freqVal, const Double& wValue, 
		   const Int& muellerElement);
    //-------------------------------------------------------------------------
    // Get CF by directly indexing in the list of CFs (data vector)
    inline void getParams(CoordinateSystem& cs, Float& sampling, 
			  Int& xSupport, Int& ySupport, 
			  const Int& i, const Int& j, const Int& k)
    {
      cs = cfCells_p(i,j,k)->coordSys_p;
      sampling = cfCells_p(i,j,k)->sampling_p;
      xSupport = cfCells_p(i,j,k)->xSupport_p;
      ySupport = cfCells_p(i,j,k)->ySupport_p;
    }
    void getParams(Double& freqVal, Float& sampling, 
		   Int& xSupport, Int& ySupport, 
		   const Int& iFreq, const Int& iW, const Int& iPol)
    {
      sampling = cfCells_p(iFreq,iW,iPol)->sampling_p;
      xSupport = cfCells_p(iFreq,iW,iPol)->xSupport_p;
      ySupport = cfCells_p(iFreq,iW,iPol)->ySupport_p;
      freqVal = freqValues_p(iFreq);
    }
    
    inline void getCoordList(Vector<Double>& freqValues, Vector<Double>& wValues,
			     PolMapType& muellerElementsIndex, PolMapType& muellerElements, 
			     PolMapType& conjMuellerElementsIndex, PolMapType& conjMuellerElements, 
			     Double& fIncr, Double& wIncr)
    {
      freqValues.assign(freqValues_p);wValues.assign(wValues_p);
      muellerElements.assign(muellerElements_p);         muellerElementsIndex.assign(muellerElementsIndex_p);
      conjMuellerElements.assign(conjMuellerElements_p); conjMuellerElementsIndex.assign(conjMuellerElementsIndex_p);
      fIncr = freqValIncr_p; wIncr = wValIncr_p;
    }
    
    Int nearestNdx(const Double& val, const Vector<Double>& valList, const Double& incr);
    
    Int nearestFreqNdx(const Double& freqVal) ;
    
    inline Int nearestWNdx(const Double& wVal) 
    {
      //      return SynthesisUtils::nint(sqrt(wValIncr_p*abs(wVal)));
      return (int)(sqrt(wValIncr_p*abs(wVal)));
    }
    
    Double nearest(Bool& found, const Double& val, const Vector<Double>& valList, const Double& incr);
    
    inline Double nearestFreq(Bool& found, const Double& freqVal)
    {return nearest(found, freqVal, freqValues_p, freqValIncr_p);}
    
    inline Double nearestWVal(Bool& found, const Double& wVal)
    {return nearest(found, wVal, wValues_p, wValIncr_p);}
    
    //-------------------------------------------------------------------------
    //
    // Generate a map for the given frequency and Mueller element list
    // to the index in the internal list of CFs.  This can be used in
    // tight loops to get get direct access to the required CF.
    //
    void makeCFBufferMap(const Vector<Double>& freqVals, 
			 const Vector<Double>& wValues,
			 const MuellerMatrixType& muellerElements);
    //-------------------------------------------------------------------------
    //
    // Add a Convolution Function with associated parameters.
    //
    void addCF(Array<TT>*, //dataPtr, 
	       CoordinateSystem&,// cs, 
	       Float& ,//sampling, 
	       Int& ,//xSupport, 
	       Int& ,//ySupport,
	       Double& ,//freqValue, 
	       Double& ,//wValue, 
	       Int& //muellerElement
	       )
    {throw(AipsError("CFBuffer::addCF called"));}
    //-------------------------------------------------------------------------
    //
    void resize(const IPosition& size) {cfCells_p.resize(size);};
    void resize(const Double& wIncr, const Double& freqIncr,
		const Vector<Double>& wValues, 
		const Vector<Double>& freqValues,
		const PolMapType& muellerElements,
		const PolMapType& muellerElementsIndex,
		const PolMapType& conjMuellerElements,
		const PolMapType& conjMuellerElementsIndex);
    Int noOfMuellerElements(const PolMapType& muellerElements);
    //-------------------------------------------------------------------------
    // Set only the CF parameters.  Return to index of the CF that was set.
    //
    RigidVector<Int, 3> setParams(const Int& i, const Int& j, const Int& ipx, const Int& ipy,
				  CoordinateSystem& cs, Float& sampling,
				  Int& xSupport, Int& ySupport,
				  const Double& freqValue, const Double& wValue, 
				  const Int& muellerElement);
    void setPointingOffset(const Vector<Double>& offset) 
    {pointingOffset_p.assign(offset);};
    Vector<Double> getPointingOffset() {return pointingOffset_p;};
    //
    // Also set the size of the CF in x and y.
    //
    void setParams(Int& nx, Int& ny, CoordinateSystem& cs, Float& sampling, 
		   Int& xSupport, Int& ySupport, 
		   const Double& freqVal, const Double& wValue, 
		   const Int& muellerElement);
    RigidVector<Int,3> getIndex(const Double& freqVal, const Double& wValue, 
				const Int& muellerElement);
    //-------------------------------------------------------------------------
    //
    // Copy just the parameters from other to this.
    //
    void copyParams(const CFBuffer& other)
    {
      cfCells_p = other.cfCells_p;
      // coordSys_p = other.coordSys_p; sampling_p.assign(other.sampling_p); 
      // xSupport_p.assign(other.xSupport_p); ySupport_p.assign(other.ySupport_p);
      maxXSupport_p=other.maxXSupport_p;  maxYSupport_p=other.maxYSupport_p; 
    }
    //-------------------------------------------------------------------------
    //
    // Write the description of the storage on the supplied ostream.
    // Used mostly for debugging, but might be useful for user
    // feedback/logging.
    //
    void show(const char *Mesg=NULL,ostream &os=cerr);
    //
    // Returns True if the internal storage is not yet initialized.
    //
    Bool null() {return (cfCells_p.nelements() == 0);};
    
    Cube<CountedPtr<CFCell> >& getStorage() {return cfCells_p;};
    void makePersistent(const char *dir);
    
    void primeTheCache();
    void initMaps(const VisBuffer& vb,const Matrix<Double>& freqSelection,const Double& imRefFreq);
    void initPolMaps(PolMapType& polMap, PolMapType& conjPolMap);
    //
    // For CUDA kernel
    //
    void getFreqNdxMaps(Vector<Vector<Int> >& freqNdx, Vector<Vector<Int> >& conjFreqNdx);
    inline Int nearestFreqNdx(const Int& spw, const Int& chan, const Bool conj=False)
    {
      if (conj) return conjFreqNdxMap_p[spw][chan];
      else  return freqNdxMap_p[spw][chan];
    }
    
    void getAsStruct(CFBStruct& st);
    
    static void initCFBStruct(CFBStruct& cfbSt) 
    {
      cfbSt.CFBStorage=NULL;
      cfbSt.freqValues=NULL;
      cfbSt.wValues=NULL;
      cfbSt.muellerElementsIndex=NULL;
      cfbSt.muellerElements=NULL;
      cfbSt.conjMuellerElementsIndex=NULL;
      cfbSt.conjMuellerElements=NULL;
      cfbSt.shape[0]=cfbSt.shape[1]=cfbSt.shape[2]=0;
      cfbSt.fIncr=cfbSt.wIncr=0.0;
    }
    void fill(const Int& nx, const Int& ny, 
	      const Vector<Double>& freqValues,
	      const Vector<Double>& wValues,
	      const PolMapType& muellerElements);
    
    IPosition getShape() {return cfCells_p.shape();}
    //
    //============================= Protected Parts ============================
    //------------------------------------------------------------------
    //
  protected:
    //
    // The storage buffer for the pixel values in CFCell is Array<T>
    // rather than Matrix<T> to accomodate rotationally symmetric CFs
    // (like the Prolate Spheroidal) which can be held as a Vector of
    // values.
    //
    Cube<CountedPtr<CFCell> > cfCells_p;// freqValues x wValues x muellerElements
    Vector<Double> wValues_p, freqValues_p;
    PolMapType muellerElements_p, muellerElementsIndex_p,conjMuellerElements_p,conjMuellerElementsIndex_p; 
    Double wValIncr_p, freqValIncr_p;
    MuellerMatrixType muellerMask_p;
    
    Int nPol_p, nChan_p, nW_p, maxXSupport_p, maxYSupport_p;
    Vector<Double> pointingOffset_p;
    Cube<Int> cfHitsStats;
    Bool freqNdxMapsReady_p;
    Vector<Vector<Int> > freqNdxMap_p, conjFreqNdxMap_p;
    void ASSIGNVVofI(Int** &target,Vector<Vector<Int> >& source, Bool& doAlloc);
  };
  
} //# NAMESPACE CASA - END
#endif
