//# Copyright (C) 1994,1995,1996,1997,1998,1999,2003
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
//# $Id: FFTServer.tcc 20253 2008-02-23 15:15:00Z gervandiepen $


#include <scimath/Mathematics/FFTW.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/Assert.h>


#include <casa/aips.h>
#include <casa/OS/HostInfo.h>
#include <fftw3.h>

#include <iostream>

namespace casa {

  FFTW::FFTW()
  {
    itsPlanR2Cf = NULL;
    itsPlanR2C = NULL;
    itsPlanC2Rf = NULL;
    itsPlanC2R = NULL;
    itsPlanC2CFf = NULL;
    itsPlanC2CF = NULL;
    itsPlanC2CBf = NULL;
    itsPlanC2CB = NULL;
    
    int numCPUs = HostInfo::numCPUs();
    int nthreads;
    if (numCPUs <= 1) {
      nthreads = 1;
    }
    else {
      nthreads = numCPUs;
    }
    
    //std::cout << "init threads " << fftw_init_threads() << std::endl;
    fftwf_plan_with_nthreads(nthreads);
    fftw_plan_with_nthreads(nthreads);
    
    //std::cerr << "will use " << nthreads << " threads " << std::endl;
    
    flags = FFTW_ESTIMATE;  
    
    //flags = FFTW_MEASURE;   std::cerr << "Will FFTW_MEASURE..." << std::endl;
    //flags = FFTW_PATIENT;
    
    // TODO: use wisdom
  }

  FFTW::~FFTW() {
    
    fftwf_destroy_plan(itsPlanR2Cf);
    fftw_destroy_plan(itsPlanR2C);
    
    fftwf_destroy_plan(itsPlanC2Rf);
    fftw_destroy_plan(itsPlanC2R);
    
    fftwf_destroy_plan(itsPlanC2CFf); 
    fftw_destroy_plan(itsPlanC2CF);
    
    fftwf_destroy_plan(itsPlanC2CBf); 
    fftw_destroy_plan(itsPlanC2CB);
    
    // We cannot deinitialize FFTW as in the following because
    // there may be other instances of this class around
#if 0
    fftw_cleanup();
    fftwf_cleanup();
    fftw_cleanup_threads();
    fftwf_cleanup_threads();
#endif
  }
  
  
  void FFTW::plan_r2c(const IPosition &size, Float *in, Complex *out) 
  {
    itsPlanR2Cf = fftwf_plan_dft_r2c(size.nelements(),
				     size.storage(),
				     in,
				     reinterpret_cast<fftwf_complex *>(out), 
				     flags);
  }

  void FFTW::plan_r2c(const IPosition &size, Double *in, DComplex *out) 
  {
    itsPlanR2C = fftw_plan_dft_r2c(size.nelements(),
				   size.storage(),
				   in,
				   reinterpret_cast<fftw_complex *>(out), 
				   flags);
  }

  void FFTW::plan_c2r(const IPosition &size, Complex *in, Float *out) {
    itsPlanC2Rf = fftwf_plan_dft_c2r(size.nelements(),
				     size.storage(),
				     reinterpret_cast<fftwf_complex *>(in),
				     out, 
				     flags);

  }
  void FFTW::plan_c2r(const IPosition &size, DComplex *in, Double *out) {
    itsPlanC2R = fftw_plan_dft_c2r(size.nelements(),
				   size.storage(),
				   reinterpret_cast<fftw_complex *>(in), 
				   out,
				   flags);

  }

  void FFTW::plan_c2c_forward(const IPosition &size, DComplex *in) {
    itsPlanC2CF = fftw_plan_dft(size.nelements(),
				size.storage(),
				reinterpret_cast<fftw_complex *>(in), 
				reinterpret_cast<fftw_complex *>(in), 
				FFTW_FORWARD, flags);

  }
    
  void FFTW::plan_c2c_forward(const IPosition &size, Complex *in) {
    itsPlanC2CFf = fftwf_plan_dft(size.nelements(),
				  size.storage(),
				  reinterpret_cast<fftwf_complex *>(in), 
				  reinterpret_cast<fftwf_complex *>(in), 
				  FFTW_FORWARD, flags);
  }

  void FFTW::plan_c2c_backward(const IPosition &size, DComplex *in) {
    itsPlanC2CB = fftw_plan_dft(size.nelements(),
				size.storage(),
				reinterpret_cast<fftw_complex *>(in), 
				reinterpret_cast<fftw_complex *>(in), 
				FFTW_BACKWARD, flags);
      
  }
    
  void FFTW::plan_c2c_backward(const IPosition &size, Complex *in) {
    itsPlanC2CBf = fftwf_plan_dft(size.nelements(),
				  size.storage(),
				  reinterpret_cast<fftwf_complex *>(in), 
				  reinterpret_cast<fftwf_complex *>(in), 
				  FFTW_BACKWARD, flags);
  }
    
  void FFTW::r2c(const IPosition &size, Float *in, Complex *out) 
  {
    // the parameters are used only in order to overload this function
    // suppress warnings about unused parameters
    if (size(0)) ; if (in) ; if (out) ;
    fftwf_execute(itsPlanR2Cf);
  }
    
  void FFTW::r2c(const IPosition &size, Double *in, DComplex *out) 
  {
    if (size(0)) ; if (in) ; if (out) ;
    fftw_execute(itsPlanR2C);
  }

  void FFTW::c2r(const IPosition &size, Complex *in, Float *out)
  {
    if (size(0)) ; if (in) ; if (out) ;
    fftwf_execute(itsPlanC2Rf);
  }
    
  void FFTW::c2r(const IPosition &size, DComplex *in, Double *out)
  {
    if (size(0)) ; if (in) ; if (out) ;
    fftw_execute(itsPlanC2R);
  }
    
  void FFTW::c2c(const IPosition &size, Complex *in, Bool forward)
  {
    if (size(0)) ; if (in) ;
    if (forward) {
      fftwf_execute(itsPlanC2CFf);
    }
    else {
      fftwf_execute(itsPlanC2CBf);
    }
  }
    
  void FFTW::c2c(const IPosition &size, DComplex *in, Bool forward)
  {
    if (size(0)) ; if (in) ;
    if (forward) {
      fftw_execute(itsPlanC2CF);
    }
    else {
      fftw_execute(itsPlanC2CB);
    }
  }

} //# NAMESPACE CASA - END
