// -*- C++ -*-
//# VisibilityResamplerBase.h: Definition of the VisibilityResamplerBase class
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

#ifndef SYNTHESIS_VISIBILITYRESAMPLERBASE_H
#define SYNTHESIS_VISIBILITYRESAMPLERBASE_H

#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/CFStore2.h>
#include <synthesis/TransformMachines/ConvolutionFunction.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/VBStore.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <synthesis/MSVis/AsynchronousTools.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>
#include <casa/OS/Timer.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  class VisibilityResamplerBase
  {
  public: 
    VisibilityResamplerBase(): 
      uvwScale_p(), offset_p(), chanMap_p(), polMap_p(), spwChanFreq_p(), spwChanConjFreq_p (), convFuncStore_p(), inc_p(),
      cfMap_p(), conjCFMap_p(), runTimeG_p(0.0), runTimeDG_p(0.0),runTimeG1_p(0.0), runTimeG2_p(0.0), runTimeG3_p(0.0), runTimeG4_p(0.0), runTimeG5_p(0.0), runTimeG6_p(0.0), runTimeG7_p(0.0),
      timer_p()
    {};
    // VisibilityResamplerBase(const CFStore& cfs): 
    //   uvwScale_p(), offset_p(), chanMap_p(), polMap_p(), convFuncStore_p(), inc_p(),
    //   cfMap_p(), conjCFMap_p()
    // {setConvFunc(cfs);};

    VisibilityResamplerBase(const VisibilityResamplerBase& other):
      uvwScale_p(), offset_p(), chanMap_p(), polMap_p(), spwChanFreq_p(), spwChanConjFreq_p (), convFuncStore_p(), inc_p(),
      cfMap_p(), conjCFMap_p()
    {copy(other);}

    virtual ~VisibilityResamplerBase() {};

    VisibilityResamplerBase& operator=(const VisibilityResamplerBase& other);

    virtual VisibilityResamplerBase* clone() = 0;

    virtual void copy(const VisibilityResamplerBase& other);
    virtual void setParams(const Vector<Double>& uvwScale, 
			   const Vector<Double>& offset,
			   const Vector<Double>& dphase) = 0;

    virtual void setMaps(const Vector<Int>& chanMap, const Vector<Int>& polMap) = 0;
    virtual void setCFMaps(const Vector<Int>& cfMap, const Vector<Int>& conjCFMap)=0;
    virtual void setFreqMaps(const Matrix<Double>& spwChanFreqs, const Matrix<Double>& spwnChanConjFreqs) = 0;

    virtual void setConvFunc(const CFStore& cfs) = 0;
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
			    Bool useConjFreqCF=False) = 0;

    virtual void DataToGrid(Array<Complex>& griddedData, VBStore& vbs, 
    			    Matrix<Double>& sumwt, const Bool& dopsf,
			    Bool useConjFreqCF=False) = 0;
    //
    //------------------------------------------------------------------------------
    //
    // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
    //
    virtual void GridToData(VBStore& vbs,const Array<Complex>& griddedData) = 0; 
    //    virtual void GridToData(VBStore& vbs, Array<Complex>& griddedData); 

    virtual void ComputeResiduals(VBStore& vbs) = 0;
    
    // Forward looking genealogical baggage -- required for the
    // MultiThreadedVisibilityResampler
    virtual void init(const Bool& doublePrecision) = 0;
    virtual void GatherGrids(Array<DComplex>& griddedData, Matrix<Double>& sumwt) = 0;
    virtual void GatherGrids(Array<Complex>& griddedData, Matrix<Double>& sumwt) = 0;
    virtual void initializePutBuffers(const Array<DComplex>& griddedData,
				      const Matrix<Double>& sumwt) = 0;
    virtual void initializePutBuffers(const Array<Complex>& griddedData,
				      const Matrix<Double>& sumwt) = 0;
    virtual void initializeDataBuffers(VBStore& vbs)=0;
    //
    // Aliases for more readable code at the FTMachine layer.
    //
    inline void finalizeToSky(Array<DComplex>& griddedData, Matrix<Double>& sumwt) 
    {GatherGrids(griddedData, sumwt);};
    inline void finalizeToSky(Array<Complex>& griddedData, Matrix<Double>& sumwt) 
    {GatherGrids(griddedData, sumwt);};
    inline void initializeToSky(const Array<DComplex>& griddedData,const Matrix<Double>& sumwt) 
    {initializePutBuffers(griddedData, sumwt);};
    inline void initializeToSky(const Array<Complex>& griddedData,const Matrix<Double>& sumwt)
    {initializePutBuffers(griddedData, sumwt);};
    const Vector<Int> getCFMap() {return cfMap_p;};
    const Vector<Int> getConjCFMap() {return conjCFMap_p;};

    
    virtual void releaseBuffers() = 0;
    VBRow2CFMapType& getVBRow2CFMap() {return vbRow2CFMap_p;};
    VBRow2CFBMapType& getVBRow2CFBMap() {return vbRow2CFBMap_p;};
    virtual Int makeVBRow2CFMap(CFStore2& cfs,
				ConvolutionFunction& cf,
				const VisBuffer& vb, const Quantity& dPA,
				const Vector<Int>& dataChan2ImChanMap,
				const Vector<Int>& dataPol2ImPolMap,
				const Vector<Double>& pointingOffset);

    Double runTimeG_p, runTimeDG_p, runTimeG1_p, runTimeG2_p, runTimeG3_p, runTimeG4_p, runTimeG5_p, runTimeG6_p, runTimeG7_p;
    Timer timer_p;
    //
    //------------------------------------------------------------------------------
    //----------------------------Private parts-------------------------------------
    //------------------------------------------------------------------------------
    //
    //  private:
  protected:
    Vector<Double> uvwScale_p, offset_p, dphase_p;
    Vector<Int> chanMap_p, polMap_p;
    Matrix<Double> spwChanFreq_p, spwChanConjFreq_p;
    CFStore convFuncStore_p;
    //    Int inc0_p, inc1_p, inc2_p, inc3_p;
    Vector<Int> inc_p;
    Int* __restrict__ incPtr_p;
    Vector<Int> cfMap_p, conjCFMap_p;
    VBRow2CFMapType vbRow2CFMap_p;
    VBRow2CFBMapType vbRow2CFBMap_p;
    

    void sgrid(Int& ndim, 
	       Double* __restrict__  pos, 
	       Int* __restrict__  loc, 
	       Int* __restrict__  off, 
    	       Complex& phasor, const Int& irow, 
	       const Double* __restrict__  uvw, 
    	       const Double& dphase, const Double& freq, 
    	       const Double* __restrict__  scale, 
	       const Double* __restrict__  offset,
    	       const Float* __restrict__  sampling);

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


    //===============================================================================
    // CASACORE-LEVEL MATERIAL
    //===============================================================================
    // Internal methods to address a 4D array.  These should ulimately
    // moved to a Array4D class in CASACore
    //

    // This is called less frequently.  Currently once per VisBuffer
    inline void cacheAxisIncrements(const Int& n0, const Int& n1, const Int& n2, const Int& n3)
    {
      //      inc0_p=1, inc1_p=inc0_p*n0, inc2_p=inc1_p*n1, inc3_p=inc2_p*n2;(void)n3;
      inc_p.resize(4);
      inc_p[0]=1; inc_p[1]=inc_p[0]*n0; inc_p[2]=inc_p[1]*n1; inc_p[3]=inc_p[2]*n2;(void)n3;
      Bool D;
      incPtr_p = inc_p.getStorage(D);
    }
    inline void cacheAxisIncrements(const Vector<Int>& n)
    {cacheAxisIncrements(n[0],n[1],n[2],n[3]);}

    inline void cacheAxisIncrements(const Vector<Int>& n, Vector<Int>& inc)
    {inc.resize(4);inc[0]=1; inc[1]=inc[0]*n[0]; inc[2]=inc[1]*n[1]; inc[3]=inc[2]*n[2];(void)n[3];}

    inline void cacheAxisIncrements(const Int n[4], Int inc[4])
    {inc[0]=1; inc[1]=inc[0]*n[0]; inc[2]=inc[1]*n[1]; inc[3]=inc[2]*n[2];(void)n[3];}

    // Version that use internally cached inc_p
    //    template <class T>
    inline void addTo4DArray(DComplex* __restrict__& store, Int* __restrict__& iPos, 
			     Complex& nvalue, Double& wt) __restrict__ 
    {addTo4DArray(store, iPos, incPtr_p, nvalue, wt);}

    inline void addTo4DArray(Complex* __restrict__& store, Int* __restrict__& iPos, 
			     Complex& nvalue, Double& wt) __restrict__ 
    {addTo4DArray(store, iPos, incPtr_p, nvalue, wt);}

    
    // Version where inc_p is supplied from outside
    inline void addTo4DArray(DComplex* __restrict__& store, Int* __restrict__& iPos, 
			     Int* __restrict__ inc, Complex& nvalue, Double& wt) __restrict__ 
    {store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]] += (nvalue*Complex(wt));}

    inline void addTo4DArray(Complex* __restrict__& store, Int* __restrict__& iPos, 
			     Int* __restrict__ inc, Complex& nvalue, Double& wt) __restrict__ 
    {store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]] += (nvalue*Complex(wt));}


    inline Complex getFrom4DArray(const Complex* __restrict__& store, 
				  const Int* __restrict__& iPos, 
				  const Vector<Int>& inc) 
    //  __restrict__ 
    {return store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]];};

    inline Complex getFrom4DArray(const Complex* __restrict__& store, 
				  const Vector<Int> iPos, const Vector<Int>& inc) 
    //  __restrict__ 
    {return store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]];};

    inline DComplex getFrom4DArray(const DComplex* __restrict__& store, 
		                   const Int* __restrict__& iPos, 
				   const Vector<Int>& inc) 
    //  __restrict__ 
    {return store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]];};

    inline DComplex getFrom4DArray(const DComplex* __restrict__& store, 
				  const Vector<Int> iPos, const Vector<Int>& inc) 
    //  __restrict__ 
    {return store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]];};


    // The following two methods are called in the innermost loop.
    inline Complex getFrom4DArray(const Complex* __restrict__& store, const Int* __restrict__& iPos) 
    //  __restrict__ 
    {return getFrom4DArray(store, iPos, inc_p);}

    inline DComplex getFrom4DArray(const DComplex* __restrict__& store, const Int* __restrict__& iPos) 
    //  __restrict__ 
    {return getFrom4DArray(store, iPos, inc_p);}

  };
}; //# NAMESPACE CASA - END

#endif // 
