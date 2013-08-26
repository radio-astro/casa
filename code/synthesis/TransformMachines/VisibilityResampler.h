// -*- C++ -*-
//# VisibilityResampler.h: Definition of the VisibilityResampler class
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

#ifndef SYNTHESIS_VISIBILITYRESAMPLER_H
#define SYNTHESIS_VISIBILITYRESAMPLER_H

#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/VBStore.h>
#include <synthesis/TransformMachines/VisibilityResamplerBase.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
//#include <synthesis/MSVis/AsynchronousTools.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  class VisibilityResampler: public VisibilityResamplerBase
  {
  public: 
    VisibilityResampler(): VisibilityResamplerBase() {};
    //    VisibilityResampler(const CFStore& cfs): VisibilityResamplerBase(cfs) {};
    VisibilityResampler(const VisibilityResampler& other):VisibilityResamplerBase()
    {copy(other);}

    //    {setConvFunc(cfs);};
    virtual ~VisibilityResampler() {};

    //    VisibilityResampler& operator=(const VisibilityResampler& other);

    void copy(const VisibilityResamplerBase& other)
    {VisibilityResamplerBase::copy(other);}

    virtual VisibilityResamplerBase* clone() 
    {return new VisibilityResampler(*this);}

    virtual void getParams(Vector<Double>& uvwScale, Vector<Double>& offset, Vector<Double>& dphase)
    {uvwScale.assign(uvwScale_p); offset.assign(offset_p); dphase.assign(dphase_p);};
    virtual void setParams(const Vector<Double>& uvwScale, const Vector<Double>& offset,
			   const Vector<Double>& dphase)
    {
      // SynthesisUtils::SETVEC(uvwScale_p, uvwScale); 
      // SynthesisUtils::SETVEC(offset_p, offset);
      // SynthesisUtils::SETVEC(dphase_p, dphase);
      uvwScale_p.reference(uvwScale);
      offset_p.reference(offset);
      dphase_p.reference(dphase);
    };

    virtual void setMaps(const Vector<Int>& chanMap, const Vector<Int>& polMap)
    {
      // SynthesisUtils::SETVEC(chanMap_p,chanMap);
      // SynthesisUtils::SETVEC(polMap_p,polMap);
      chanMap_p.reference(chanMap);
      polMap_p.reference(polMap);
    }

    virtual void setFreqMaps(const Matrix<Double>& spwChanFreqs, const Matrix<Double>& spwChanConjFreqs)
    {
      spwChanFreq_p.assign(spwChanFreqs);
      spwChanConjFreq_p.assign(spwChanConjFreqs);
    }

    virtual void setConvFunc(const CFStore& cfs) 
    {
      convFuncStore_p = cfs;
    };
    virtual void setCFMaps(const Vector<Int>& cfMap, const Vector<Int>& conjCFMap) {(void)cfMap;(void)conjCFMap;};
    //
    //------------------------------------------------------------------------------
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a gridding).
    //
    // In this class, these just call the private templated version.
    // The first variant grids onto a double precision grid while the
    // second one does it on a single precision grid.
    //
    virtual void DataToGrid(Array<DComplex>& griddedData, VBStore& vbs, 
			    Matrix<Double>& sumwt, const Bool& dopsf,
			    Bool /*useConjFreqCF*/=False)
    {DataToGridImpl_p(griddedData, vbs, dopsf, sumwt);}

    virtual void DataToGrid(Array<Complex>& griddedData, VBStore& vbs, 
    			    Matrix<Double>& sumwt, const Bool& dopsf,
			    Bool /*useConjFreqCF*/=False)
    {DataToGridImpl_p(griddedData, vbs, dopsf, sumwt);}

    //
    //------------------------------------------------------------------------------
    //
    // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
    //
    virtual void GridToData(VBStore& vbs,const Array<Complex>& griddedData); 
    //    virtual void GridToData(VBStore& vbs, Array<Complex>& griddedData); 

    virtual void ComputeResiduals(VBStore& vbs);
    virtual void setMutex(async::Mutex *mu) {myMutex_p = mu;};

    // Genealogical baggage -- required for the
    // MultiThreadedVisibilityResampler -- that everyone else has to
    // carray around.
    //
    // These are no-ops for unithreaded samplers.
    //
    virtual void init(const Bool& doublePrecision) {(void)doublePrecision;};
    virtual void GatherGrids(Array<DComplex>& griddedData, Matrix<Double>& sumwt) {(void)griddedData;(void)sumwt;};
    virtual void GatherGrids(Array<Complex>& griddedData, Matrix<Double>& sumwt) {(void)griddedData;(void)sumwt;};
    virtual void initializePutBuffers(const Array<DComplex>& griddedData,
				      const Matrix<Double>& sumwt) {(void)griddedData;(void)sumwt;};
    virtual void initializePutBuffers(const Array<Complex>& griddedData,
				      const Matrix<Double>& sumwt) {(void)griddedData;(void)sumwt;};
    virtual void initializeDataBuffers(VBStore& vbs) {(void)vbs;};


    virtual void releaseBuffers() {};
    //
    //------------------------------------------------------------------------------
    //----------------------------Private parts-------------------------------------
    //------------------------------------------------------------------------------
    //
  protected:
    async::Mutex *myMutex_p;
    // Vector<Double> uvwScale_p, offset_p, dphase_p;
    // Vector<Int> chanMap_p, polMap_p;
    // CFStore convFuncStore_p;
    //    Int inc0_p, inc1_p, inc2_p, inc3_p;
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
    //
    template <class T>
    void DataToGridImpl_p(Array<T>& griddedData, VBStore& vb,  
			  const Bool& dopsf, Matrix<Double>& sumwt,Bool useConjFreqCF=False);

    // void sgrid(Vector<Double>& pos, Vector<Int>& loc, Vector<Int>& off, 
    // 	       Complex& phasor, const Int& irow, const Matrix<Double>& uvw, 
    // 	       const Double& dphase, const Double& freq, 
    // 	       const Vector<Double>& scale, const Vector<Double>& offset,
    // 	       const Vector<Float>& sampling);

    /*
    void sgrid(Int& ndim, Double* pos, Int* loc, Int* off, 
    	       Complex& phasor, const Int& irow, const Double* uvw, 
    	       const Double& dphase, const Double& freq, 
    	       const Double* scale, const Double* offset,
    	       const Float* sampling);

    inline Bool onGrid (const Int& nx, const Int& ny, 
			const Vector<Int>& __restrict__ loc, 
			const Vector<Int>& __restrict__ support) __restrict__ 
    {
      return (((loc(0)-support[0]) >= 0 ) && ((loc(0)+support[0]) < nx) &&
	      ((loc(1)-support[1]) >= 0 ) && ((loc(1)+support[1]) < ny));
    };
    inline Bool onGrid (const Int& nx, const Int& ny, 
			const Int& loc0, const Int& loc1, 
			const Int& support) __restrict__ 
    {
      return (((loc0-support) >= 0 ) && ((loc0+support) < nx) &&
	      ((loc1-support) >= 0 ) && ((loc1+support) < ny));
    };

    // Array assignment operator in CASACore requires lhs.nelements()
    // == 0 or lhs.nelements()=rhs.nelements()
    // template <class T>
    // inline void SETVEC(Vector<T>& lhs, const Vector<T>& rhs)
    // {lhs.resize(rhs.shape()); lhs = rhs;};


    //
    // Internal methods to address a 4D array.  These should ulimately
    // moved to a Array4D class in CASACore
    //

    // This is called less frequently.  Currently once per VisBuffer
    inline void cacheAxisIncrements(Int& n0, Int& n1, Int& n2, Int& n3)
    {inc0_p=1, inc1_p=inc0_p*n0, inc2_p=inc1_p*n1, inc3_p=inc2_p*n2;(void)n3;}


    // The following two methods are called in the innermost loop.
    inline Complex getFrom4DArray(const Complex* __restrict__ store,
				  const Int* __restrict__ iPos) __restrict__ 
    {return store[iPos[0] + iPos[1]*inc1_p + iPos[2]*inc2_p +iPos[3]*inc3_p];};

    template <class T>
    void addTo4DArray(T* __restrict__ store,
		      const Int* __restrict__ iPos, 
		      Complex& nvalue, Double& wt) __restrict__ 
    {store[iPos[0] + iPos[1]*inc1_p + iPos[2]*inc2_p +iPos[3]*inc3_p] += (nvalue*wt);}
    */
  };
}; //# NAMESPACE CASA - END

#endif // 
