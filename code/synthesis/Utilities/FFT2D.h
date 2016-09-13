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
   ///This is a not a full generic   fft class...use FFTServer or LatticeFFT for that
   //This is optimized with minimal memcopies for 2D FFTs 
   //Assumes 2D x, y array to be even numbers (e.g (100, 200)...will not work for (101, 200))
 public:
   FFT2D(Bool useFFTW=True);
   ~FFT2D();
   FFT2D& operator=(const FFT2D& other);
   //out has to be a pointer to an array [(x/2+1), y] shape 
   void r2cFFT(Complex*& out, Float*& in, Long x, Long y);
   ///Real to complex FFT2D ..each plane of out will contain FFT in X/2+1, Y...
   //the remainder of of the X part is untouched of out lattice is larger than 
   //X/2+1 on the x-axis   
   void r2cFFT(Lattice<Complex>& out, Lattice<Float>& in);
   ///In place 2D FFT; out has to be of shape [x,y] (origin is at the x/2,y/2)
   void c2cFFT(Complex*& out, Long x, Long y, Bool toFreq=True);
   void c2cFFT(DComplex*& out, Long x, Long y, Bool toFreq=True);
   //This will return the 2D FFT of each x-y planes back into the lattice.
   void c2cFFT(Lattice<Complex>& inout, Bool toFreq=True);
   void c2cFFT(Lattice<DComplex>& inout, Bool toFreq=True);
   //The toFreq=False in FFTShift does the normalization of 1/N_sample expected of ifft
   void fftShift(Complex*& scr,  Long x, Long y, Bool toFreq=False);
   void fftShift(DComplex*& scr,  Long x, Long y, Bool toFreq=False);
   void fftShift(Float*& scr,  Long x, Long y);
   void doFFT(Complex*& out, Long x, Long y, Bool toFreq);
   void doFFT(DComplex*& out, Long x, Long y, Bool toFreq);
   void doFFT(Complex*& out, Float *& in, Long x, Long y);
 private:
   //FFTW stuff
   fftwf_plan planC2C_p;
   fftwf_plan planR2C_p;
   fftw_plan planC2CD_p;
   Bool useFFTW_p;
   //FFTPack stuff
   std::vector<Float> wsave_p;
   Int lsav_p;
   //FFTW fft1_p;
 };
};// end of namespace casa
#endif
