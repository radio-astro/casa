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
    VisibilityResampler() {};
    virtual ~VisibilityResampler() {};
    
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
    //
    // In this class, these just call the private templated version.
    //
    virtual void DataToGrid(VBStore& vb, Array<DComplex>& griddedData,  const Bool& dopsf,
			    Matrix<Double>& sumwt,const CFStore& cfs, const Vector<Double>& dphase,  
			    const Vector<Int>& chanMap, const Vector<Int>& polMap,
			    const Vector<Double>& scale, const Vector<Double>& offset)
    {DataToGridImpl(vb, griddedData, dopsf, sumwt, cfs, dphase, chanMap, polMap, scale, offset);}

    virtual void DataToGrid(VBStore& vb, Array<Complex>& griddedData,  const Bool& dopsf,
		    Matrix<Double>& sumwt,const CFStore& cfs, const Vector<Double>& dphase,  
		    const Vector<Int>& chanMap, const Vector<Int>& polMap,
		    const Vector<Double>& scale, const Vector<Double>& offset)
    {DataToGridImpl(vb, griddedData, dopsf, sumwt, cfs, dphase, chanMap, polMap, scale, offset);}

    //
    // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. gridding)
    //
    virtual void GridToData(const Array<Complex>& griddedData, VBStore& vbs, 
		    const CFStore& cfs, const Vector<Double>& dphase,
		    const Vector<Int>& chanMap, const Vector<Int>& polMap,
		    const Vector<Double>& scale, const Vector<Double>& offset);

  private:
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
    //
    template <class T>
    void DataToGridImpl(VBStore& vb, Array<T>& griddedData,  const Bool& dopsf,
		    Matrix<Double>& sumwt,const CFStore& cfs, const Vector<Double>& dphase,  
		    const Vector<Int>& chanMap, const Vector<Int>& polMap,
		    const Vector<Double>& scale, const Vector<Double>& offset);

    void sgrid(Vector<Double>& pos, Vector<Int>& loc, Vector<Int>& off, Complex& phasor,
	       const Int& irow,
	       const Matrix<Double>& uvw, const Vector<Double>& dphase, 
	       const Double& freq, const Vector<Double>& scale, const Vector<Double>& offset,
	       const Vector<Float>& sampling);

    inline Bool onGrid (const Int& nx, const Int& ny, const Vector<Int>& loc, 
			const Vector<Int>& support)
    {
      return (((loc(0)-support[0]) >= 0 ) && ((loc(0)+support[0]) < nx) &&
	      ((loc(1)-support[1]) >= 0 ) && ((loc(1)+support[1]) < ny));
    };
  };
}; //# NAMESPACE CASA - END

#endif // 
