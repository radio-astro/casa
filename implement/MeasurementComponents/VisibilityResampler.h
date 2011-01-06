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

#include <synthesis/MeasurementComponents/CFStore.h>
#include <synthesis/MeasurementComponents/VBStore.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  class VisibilityResampler
  {
  public: 
    VisibilityResampler(): 
      uvwScale_p(), offset_p(), chanMap_p(), polMap_p(), convFuncStore_p()
    {};
    VisibilityResampler(const CFStore& cfs): 
      uvwScale_p(), offset_p(), chanMap_p(), polMap_p(), convFuncStore_p()
    {setConvFunc(cfs);};

    virtual ~VisibilityResampler() {};
    

    VisibilityResampler& operator=(const VisibilityResampler& other);


    virtual void setParams(const Vector<Double>& uvwScale, const Vector<Double>& offset,
			   const Vector<Double>& dphase)
    {SETVEC(uvwScale_p, uvwScale); SETVEC(offset_p, offset);SETVEC(dphase_p, dphase);};

    virtual void setMaps(const Vector<Int>& chanMap, const Vector<Int>& polMap)
    {SETVEC(chanMap_p,chanMap);SETVEC(polMap_p,polMap);}

    virtual void setConvFunc(const CFStore& cfs) {convFuncStore_p = cfs;};
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
    virtual void DataToGrid(Array<DComplex>& griddedData,  
			    VBStore& vbs, Matrix<Double>& sumwt,
			    const Bool& dopsf, CFStore& cfs)
    {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf,cfs);}

    virtual void DataToGrid(Array<Complex>& griddedData, 
			    VBStore& vbs, Matrix<Double>& sumwt,
			    const Bool& dopsf, CFStore& cfs)
    {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf,cfs);}
    //
    // Simulating defaulting CFStore arguemnt in the above calls to convFuncStore_p
    //
    virtual void DataToGrid(Array<DComplex>& griddedData,  
			    VBStore& vbs, Matrix<Double>& sumwt,
			    const Bool& dopsf)
    {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf,convFuncStore_p);}

    virtual void DataToGrid(Array<Complex>& griddedData, 
			    VBStore& vbs, Matrix<Double>& sumwt,
			    const Bool& dopsf)
    {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf,convFuncStore_p);}

    //
    //------------------------------------------------------------------------------
    //
    // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
    //
    virtual void GridToData(VBStore& vbs,const Array<Complex>& griddedData); 
    //    virtual void GridToData(VBStore& vbs, Array<Complex>& griddedData); 
  protected:
    virtual Double getConvFuncVal(const Vector<Double>& convFunc,
				  const Matrix<Double>& uvw, 
				  const Int& irow,
				  const Vector<Int>& pixel)
    {
      return convFunc[pixel[0]]*convFunc[pixel[1]] ;
    }
    //
    //------------------------------------------------------------------------------
    //----------------------------Private parts-------------------------------------
    //------------------------------------------------------------------------------
    //
  private:
    Vector<Double> uvwScale_p, offset_p, dphase_p;
    Vector<Int> chanMap_p, polMap_p;
    CFStore convFuncStore_p;
    Int inc0_p, inc1_p, inc2_p, inc3_p;
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
    //
    template <class T>
    void DataToGridImpl_p(Array<T>& griddedData, VBStore& vb,  
			  Matrix<Double>& sumwt,const Bool& dopsf,CFStore& cfs);

    void sgrid(Vector<Double>& pos, Vector<Int>& loc, Vector<Int>& off, 
	       Complex& phasor, const Int& irow, const Matrix<Double>& uvw, 
	       const Double& dphase, const Double& freq, 
	       const Vector<Double>& scale, const Vector<Double>& offset,
	       const Vector<Float>& sampling);

    inline Bool onGrid (const Int& nx, const Int& ny, const Vector<Int>& loc, 
			const Vector<Int>& support)
    {
      return (((loc(0)-support[0]) >= 0 ) && ((loc(0)+support[0]) < nx) &&
	      ((loc(1)-support[1]) >= 0 ) && ((loc(1)+support[1]) < ny));
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
    inline void cacheAxisIncrements(Int& n0, Int& n1, Int& n2, Int& n3)
    {inc0_p=1, inc1_p=inc0_p*n0, inc2_p=inc1_p*n1, inc3_p=inc2_p*n2;(void)n3;}


    // The following two methods are called in the innermost loop.
    inline Complex getFrom4DArray(const Complex* store,const Int* iPos)
    {return store[iPos[0] + iPos[1]*inc1_p + iPos[2]*inc2_p +iPos[3]*inc3_p];};

    template <class T>
    void addTo4DArray(T* store,const Int* iPos, Complex& nvalue, Double& wt)
    {store[iPos[0] + iPos[1]*inc1_p + iPos[2]*inc2_p +iPos[3]*inc3_p] += (nvalue*wt);}
  };
}; //# NAMESPACE CASA - END

#endif // 
