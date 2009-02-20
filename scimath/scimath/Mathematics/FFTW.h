//# Copyright (C) 1993,1994,1995,1997,1999,2000,2001
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

//# $Id: FFTPack.h 20299 2008-04-03 05:56:44Z gervandiepen $

#ifndef SCIMATH_FFTW_H
#define SCIMATH_FFTW_H

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Arrays/Matrix.h>

#include <casa/aips.h>
#include <fftw3.h>

namespace casa {

// <summary>C++ interface to the FFTW library</summary>
// <reviewed reviewer="NONE" date="" tests="" demos="">
// </reviewed>
// <synopsis>
//    This is a wrapper of FFTW
// </synopsis>

class FFTW
{
public:
  FFTW() ;
  
  ~FFTW() ;

  // polymorphic interface to fftw[f]_plan...
  void plan_r2c(const IPosition &size, Float *in, Complex *out) ;
  void plan_r2c(const IPosition &size, Double *in, DComplex *out) ;
  void plan_c2r(const IPosition &size, Complex *in, Float *out) ;
  void plan_c2r(const IPosition &size, DComplex *in, Double *out) ;
  void plan_c2c_forward(const IPosition &size, DComplex *in) ;
  void plan_c2c_forward(const IPosition &size, Complex *in) ;
  void plan_c2c_backward(const IPosition &size, DComplex *in) ;
  void plan_c2c_backward(const IPosition &size, Complex *in) ;
  
  // polymorphic interface to fftw[f]_execute...
  void r2c(const IPosition &size, Float *in, Complex *out) ;
  void r2c(const IPosition &size, Double *in, DComplex *out) ;
  void c2r(const IPosition &size, Complex *in, Float *out);
  void c2r(const IPosition &size, DComplex *in, Double *out);
  void c2c(const IPosition &size, Complex *in, Bool forward);
  void c2c(const IPosition &size, DComplex *in, Bool forward);

 private:
    fftwf_plan itsPlanR2Cf;
    fftw_plan itsPlanR2C;

    fftwf_plan itsPlanC2Rf;
    fftw_plan itsPlanC2R;

    fftwf_plan itsPlanC2CFf;   // forward
    fftw_plan itsPlanC2CF;

    fftwf_plan itsPlanC2CBf;   // backward
    fftw_plan itsPlanC2CB;

    unsigned flags;
};    
    
} //# NAMESPACE CASA - END

#endif
