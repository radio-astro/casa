// -*- C++ -*-
//# AWVisResampler.h: Definition of the AWVisResampler class
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

#ifndef SYNTHESIS_AWVISRESAMPLER_H
#define SYNTHESIS_AWVISRESAMPLER_H

#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/VBStore.h>
#include <synthesis/TransformMachines/VisibilityResampler.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  class AWVisResampler: public VisibilityResampler
  {
  public: 
    AWVisResampler(): VisibilityResampler(),
		      cached_phaseGrad_p(),
                      cached_PointingOffset_p()
    {cached_PointingOffset_p.resize(2);cached_PointingOffset_p=-1000.0;runTimeG_p=runTimeDG_p=0.0;};
    //    AWVisResampler(const CFStore& cfs): VisibilityResampler(cfs)      {}
    virtual ~AWVisResampler()                                         {};

    virtual VisibilityResamplerBase* clone()
    {return new AWVisResampler(*this);}
    
    // AWVisResampler(const AWVisResampler& other): VisibilityResampler(other),cfMap_p(), conjCFMap_p()
    // {copy(other);}

    virtual void copyMaps(const AWVisResampler& other)
    {setCFMaps(other.cfMap_p, other.conjCFMap_p);}
    virtual void copy(const VisibilityResamplerBase& other) 
    {
      VisibilityResampler::copy(other);
      // const Vector<Int> cfmap=other.getCFMap();
      // const Vector<Int> conjcfmap = other.getConjCFMap();

      // setCFMaps(cfmap,conjcfmap);
    }

    virtual void copy(const AWVisResampler& other) 
    {
      VisibilityResampler::copy(other);
      SynthesisUtils::SETVEC(cached_phaseGrad_p, other.cached_phaseGrad_p);
      SynthesisUtils::SETVEC(cached_PointingOffset_p, other.cached_PointingOffset_p);
    }

    AWVisResampler& operator=(const AWVisResampler& other) 
    {
      copy(other);      
      SynthesisUtils::SETVEC(cached_phaseGrad_p, other.cached_phaseGrad_p);
      SynthesisUtils::SETVEC(cached_PointingOffset_p, other.cached_PointingOffset_p);
      return *this;
    }

    virtual void setCFMaps(const Vector<Int>& cfMap, const Vector<Int>& conjCFMap)
    {SETVEC(cfMap_p,cfMap);SETVEC(conjCFMap_p,conjCFMap);}

    // virtual void setConvFunc(const CFStore& cfs) {convFuncStore_p = cfs;};
    //
    //------------------------------------------------------------------------------
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a gridding).
    //
    // In this class, these just call the private templated version.
    // The first variant grids onto a double precision grid while the
    // second one does it on a single precision grid.
    //
    // Note that the following calls allow using any CFStore object
    // for gridding while de-gridding uses the internal
    // convFuncStore_p object.
    // virtual void DataToGrid(Array<DComplex>& griddedData, VBStore& vbs, Matrix<Double>& sumwt,
    // 			    const Bool& dopsf, CFStore& cfs)
    // {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf,cfs);}

    // virtual void DataToGrid(Array<Complex>& griddedData, VBStore& vbs, Matrix<Double>& sumwt,
    // 			    const Bool& dopsf, CFStore& cfs)
    // {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf,cfs);}
    //
    // Simulating defaulting CFStore arguemnt in the above calls to convFuncStore_p
    //

    //***TEMP REMOVAL OF DComplex gridder*****

    virtual void DataToGrid(Array<DComplex>& griddedData, VBStore& vbs, Matrix<Double>& sumwt,
    			    const Bool& dopsf,Bool useConjFreqCF=False)
    {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf,useConjFreqCF);}

    virtual void DataToGrid(Array<Complex>& griddedData, VBStore& vbs, Matrix<Double>& sumwt,
			    const Bool& dopsf,Bool useConjFreqCF=False)
    {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf,useConjFreqCF);}

    //
    //------------------------------------------------------------------------------
    //
    // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
    //
    virtual void GridToData(VBStore& vbs,const Array<Complex>& griddedData); 
    //    virtual void GridToData(VBStore& vbs, Array<Complex>& griddedData); 
  protected:
    virtual Complex getConvFuncVal(const Cube<Double>& convFunc, const Matrix<Double>& uvw, 
				   const Int& irow, const Vector<Int>& pixel)
    {
      (void)uvw; (void)irow;return convFunc(pixel[0],pixel[1],pixel[2]);
    }
    Complex getCFArea(Complex* __restrict__& convFuncV, Double& wVal,
		      Vector<Int>& scaledSupport, Vector<Float>& scaledSampling,
		      Vector<Double>& off,
		      Vector<Int>& convOrigin, Vector<Int>& cfShape,
		      Double& sinDPA, Double& cosDPA);

  template <class T>
  Complex accumulateOnGrid(Array<T>& grid, Complex* __restrict__& convFuncV, 
			   Complex& nvalue,
			   Double& wVal, Vector<Int>& scaledSupport, 
			   Vector<Float>& scaledSampling, Vector<Double>& off,
			   Vector<Int>& convOrigin, Vector<Int>& /*cfShape*/,
			   Vector<Int>& loc, Vector<Int>& igrdpos, 
			   Double& /*sinDPA*/, Double& /*cosDPA*/,
			   Bool& finitePointingOffset, Bool dopsf);
  template <class T>
  void XInnerLoop(const Int *scaleSupport, const Float* scaledSampling,
		  const Double* off,
		  const Int* loc, Complex& cfArea,  
		  const Int * __restrict__ iGrdPosPtr,
		  Complex *__restrict__& convFuncV,
		  const Int* convOrigin,
		  Complex& nvalue,
		  Double& wVal,
		  Bool& /*finitePointingOffset*/,
		  Bool& /*doPSFOnly*/,
		  T* __restrict__ gridStore,
		  Int* iloc,
		  Complex& norm,
		  Int* igrdpos);

  template <class T>
  void accumulateFromGrid(T& nvalue, const T* __restrict__& grid, 
			  Vector<Int>& iGrdPos,
			  Complex* __restrict__& convFuncV, 
			  Double& wVal, Vector<Int>& scaledSupport, 
			  Vector<Float>& scaledSampling, Vector<Double>& off,
			  Vector<Int>& convOrigin, Vector<Int>& cfShape,
			  Vector<Int>& loc, 
			  Complex& phasor, 
			  Double& sinDPA, Double& cosDPA,
			  Bool& finitePointingOffset, 
			  Matrix<Complex>& cached_phaseGrad_p);

    //
    //------------------------------------------------------------------------------
    //----------------------------Private parts-------------------------------------
    //------------------------------------------------------------------------------
    //
  private:
    // Vector<Double> uvwScale_p, offset_p, dphase_p;
    // Vector<Int> chanMap_p, polMap_p;
    // CFStore convFuncStore_p;
    // //    Int inc0_p, inc1_p, inc2_p, inc3_p;
    // Vector<Int> inc_p;
    //    Vector<Int> cfMap_p, conjCFMap_p;
    Vector<Int> gridInc_p, cfInc_p;
    Matrix<Complex> cached_phaseGrad_p;
    Vector<Double> cached_PointingOffset_p;
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
    //
    template <class T>
    void DataToGridImpl_p(Array<T>& griddedData, VBStore& vb,  
			  Matrix<Double>& sumwt,const Bool& dopsf,
			  Bool /*useConjFreqCF*/);

    void sgrid(Vector<Double>& pos, Vector<Int>& loc, Vector<Double>& off, 
    	       Complex& phasor, const Int& irow, const Matrix<Double>& uvw, 
    	       const Double& dphase, const Double& freq, 
    	       const Vector<Double>& scale, const Vector<Double>& offset,
    	       const Vector<Float>& sampling);

    inline Bool onGrid (const Int& nx, const Int& ny, const Int& nw, 
    			const Vector<Int>& loc, 
    			const Vector<Int>& support)
    {
      return (((loc(0)-support[0]) >= 0 ) && ((loc(0)+support[0]) < nx) &&
    	      ((loc(1)-support[1]) >= 0 ) && ((loc(1)+support[1]) < ny) &&
    	      (loc(2) >= 0) && (loc(2) <= nw));
    };

    // Array assignment operator in CASACore requires lhs.nelements()
    // == 0 or lhs.nelements()=rhs.nelements()
    template <class T>
    inline void SETVEC(Vector<T>& lhs, const Vector<T>& rhs)
    {lhs.resize(rhs.shape()); lhs = rhs;};


    //
    // Internal methods to address a 4D array.  These should ulimately
    // moved to a Array4D class in CASACore
    //

    // This is called less frequently.  Currently once per VisBuffer
    // inline void cacheAxisIncrements(const Vector<Int>& n, Vector<Int>& inc)
    // {inc.resize(4);inc[0]=1, inc[1]=inc[0]*n[0], inc[2]=inc[1]*n[1], inc[3]=inc[2]*n[2];(void)n[3];}


    // The following method is also called from the inner loop, but
    // does not use CASA Vector (which are not thread safe, I (SB) am
    // told).
    inline Complex getFrom4DArray(const Complex *__restrict__& store,
    				  const Int* iPos, const Int* inc)
    {
      return *(store+(iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]));
      //      return store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]];
    };

    // The following two methods are called in the innermost loop.
    inline Complex getFrom4DArray(const Complex *__restrict__& store,
    				  const Vector<Int>& iPos, const Vector<Int>& inc)
    {
      return *(store+(iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]));
      //      return store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]];
    };
    inline DComplex getFrom4DArray(const DComplex *__restrict__& store,
    				  const Vector<Int>& iPos, const Vector<Int>& inc)
    {
      return *(store+(iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]));
      //      return store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]];
    };

    template <class T>
    void addTo4DArray(T *__restrict__& store,
    		      const Int *__restrict__& iPos, const Vector<Int>& inc, 
		      Complex& nvalue, Complex& wt) __restrict__
    {
      // T *tmp=store+(iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]);
      // *tmp += nvalue*wt;
      store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]] += (nvalue*wt);
    }

    //
    // This rotates the convolution function by rotating the
    // co-ordinate system.  For the accuracies already required for
    // EVLA and ALMA, this is not useful.  Leaving it hear for now....
    //
    Bool reindex(const Vector<Int>& in, Vector<Int>& out,
		 const Double& sinDPA, const Double& cosDPA,
		 const Vector<Int>& Origin, const Vector<Int>& size);

    Complex* getConvFunc_p(Vector<Int>& cfShape,
			   CFBuffer& cfb,
			   Double& wVal, Int& fndx, 
			   Int& wndx,
			   PolMapType& mNdx, PolMapType& conjMNdx,
			   Int& ipol, uInt& mRow);
    void cachePhaseGrad_p(const Vector<Double>& pointingOffset,
			  const Vector<Int>&cfShape,
			  const Vector<Int>& convOrigin,
			  const Double& cfRefFreq,
			  const Double& imRefFreq,
			  const Int& spwID=0, const Int& fieldId=0);
  };
}; //# NAMESPACE CASA - END

#endif // 
