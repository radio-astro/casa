//# FFT2D.h 
//# Copyright (C) 2016
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  General Public
//# License for more details.
//#
//# You should have received a copy of the GNU  General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $kgolap$
#ifndef SYNTHESIS_FFT2D_H
#define SYNTHESIS_FFT2D_H
#include <scimath/Mathematics/FFTW.h>
#include <scimath/Mathematics/FFTPack.h>
#include <fftw3.h>
namespace casa{
 class FFT2D
 {
 public:
   FFT2D(Bool useFFTW=True);
   ~FFT2D();
   //out has to be a pointer to an array [(x/2+1), y] shape 
   void r2cFFT(Complex*& out, Float*& in, Long x, Long y);
   ///In place 2D FFT; out has to be of shape [x,y]
   void c2cFFT(Complex*& out, Long x, Long y, Bool toFreq=True);
   //The toFreq=False in FFTShift does the normalization of 1/N_sample expected of ifft
   void fftShift(Complex*& scr,  Long x, Long y, Bool toFreq=False);
   void fftShift(Float*& scr,  Long x, Long y);
   void doFFT(Complex*& out, Long x, Long y, Bool toFreq);
 private:
   //FFTW stuff
   fftwf_plan planC2C_p;
   fftwf_plan planC2R_p;
   Bool useFFTW_p;
   //FFTPack stuff
   std::vector<Float> wsave_p;
   Int lsav_p;
   FFTW fft1_p;
 };
};// end of namespace casa
#endif
