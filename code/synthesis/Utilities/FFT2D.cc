//# FFT2D.cc: implementation of FFT2D
//# Copyright (C) 2016
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU  General Public License as published by
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
//DEDICATED TO HONGLIN YE 

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/OS/HostInfo.h>
#include <synthesis/Utilities/FFT2D.h>
#include <lattices/Lattices/Lattice.h>
#ifdef _OPENMP
#include <omp.h>
#endif
namespace casa { //# NAMESPACE CASA - BEGIN

  FFT2D::FFT2D(Bool useFFTW): useFFTW_p(useFFTW), wsave_p(0), lsav_p(0){
    if(useFFTW_p){
      Int numThreads=HostInfo::numCPUs(True);
#ifdef _OPENMP
      numThreads=omp_get_max_threads();
#endif      
      fftwf_init_threads();
      fftwf_plan_with_nthreads(numThreads);
      ///For double precision
      fftw_init_threads();
      fftw_plan_with_nthreads(numThreads);
    }
   
  }
  FFT2D::~FFT2D(){
    if(useFFTW_p)
       fftw_cleanup_threads();

  }

  FFT2D& FFT2D::operator=(const FFT2D& other){
    if(this != &other){
      planC2C_p=other.planC2C_p;
      planR2C_p=other.planR2C_p;
      useFFTW_p=other.useFFTW_p;
      wsave_p.resize(other.wsave_p.size());
      wsave_p=other.wsave_p;
      lsav_p=other.lsav_p;

    }
    return *this;
  }

  void FFT2D::r2cFFT(Lattice<Complex>& out, Lattice<Float>& in){
    
    IPosition shp=in.shape();
    if(shp.nelements() <2)
      throw(AipsError("Lattice has to be 2 dimensional to use FFT2D"));
    Long x= in.shape()(0);
    Long y=in.shape()(1);
    if(out.shape()(0) < (x/2+1))
      throw(AipsError("out shape has to be x/2+1 in size  for real to complex FFT2D"));
    for(uInt k=1; k < shp.nelements(); ++k){
      if(shp(k) != out.shape()(k))
	throw(AipsError("shapes of out lattice does not match in lattice for FFT2D")); 
    }
    Long numplanes=shp.product()/x/y;
    IPosition blc(shp.nelements(), 0);
    IPosition shape=in.shape();
   
    for (uInt ax=2; ax < shp.nelements(); ++ax)
      shape(ax)=1;
    IPosition outshape=shape;
    outshape(0)=x/2+1;
 
    Array<Complex> arr;
    Array<Float> arrf;
    Bool isRef;
    Bool del;
    Bool delf;
    Complex *scr;
    Float *scrf;
    
    
    
    for (Long n=0; n< numplanes; ++n){
      isRef=out.getSlice(arr, blc, outshape); 
      scr=arr.getStorage(del);
      ///Use this method rather than arrf=in.getSlice(blc,shape) 
      ///as this may be a reference ..the other is a copy always...
      /// can gain 0.8s or so for a 10000x10000 array circa 2016
      in.getSlice( arrf, blc, shape);
      scrf=arrf.getStorage(delf);
      r2cFFT(scr, scrf, x, y);      
      arr.putStorage(scr, del);
      arrf.putStorage(scrf, delf);
      
      if(!isRef){
	out.putSlice(arr, blc);
	
      }
      //Now calculate next plane
       
      Bool addNextAx=True;
      for (uInt ax=2; ax < shp.nelements(); ++ax){
	if(addNextAx){
	  blc(ax) +=1;
	  addNextAx=False;
	}
	if(blc(ax)> shp(ax)-1){
	  blc(ax)=0;
	  addNextAx=True;
	}
       
      }
      
    }
  }
  void  FFT2D::r2cFFT(Complex*& out,  Float*& in, Long x, Long y){
    if(x%2 != 0 || y%2 != 0)
      throw(AipsError("Programmer error: FFT2D does not deal with odd numbers on x-y plane"));
    fftShift(in, x, y);
    doFFT(out, in, x, y);
    //fft1_p.plan_r2c(IPosition(2,x,y), in, out);
    //fft1_p.r2c(IPosition(2,x,y), in, out);
    //flipArray out is of shape x/2+1, y
    Complex* scr=out;
    Matrix<Complex> tmpo(x/2+1, y/2);
    Bool gool;
    Complex* tmpptr=tmpo.getStorage(gool);
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
    for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < (x/2+1); ++ii){
	tmpptr[jj*(x/2+1)+ii]=scr[jj*(x/2+1)+ii];
	scr[jj*(x/2+1)+ii]=scr[(y/2)*(x/2+1)+jj*(x/2+1)+ii];
      }
    }
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
	for (Long jj=0; jj< y/2; ++jj){
	  for(Long ii=0; ii < x/2; ++ii){
	    scr[(y/2)*(x/2+1)+jj*(x/2+1)+ii]=tmpptr[jj*(x/2+1)+ii];
	  }
	} 

  }
  void FFT2D::c2cFFT(Lattice<Complex>& inout, Bool toFreq){
    IPosition shp=inout.shape();
    if(shp.nelements() <2)
      throw(AipsError("Lattice has to be 2 dimensional to use FFT2D"));
    Long x= inout.shape()(0);
    Long y=inout.shape()(1);
    Long numplanes=inout.shape().product()/x/y;
    IPosition blc(inout.shape().nelements(), 0);
    IPosition shape=inout.shape();
    for (uInt ax=2; ax < shp.nelements(); ++ax)
      shape(ax)=1;
    Array<Complex> arr;
    Bool isRef;
    Bool del;
    Complex *scr;

    for (Long n=0; n< numplanes; ++n){
      isRef=inout.getSlice(arr, blc, shape); 
      scr=arr.getStorage(del);
      c2cFFT(scr, x, y, toFreq);
      arr.putStorage(scr, del);
      if(!isRef)
	inout.putSlice(arr, blc);
      //Now calculate next plane 
      Bool addNextAx=True;
      for (uInt ax=2; ax < shp.nelements(); ++ax){
	if(addNextAx){
	  blc(ax) +=1;
	  addNextAx=False;
	}
	if(blc(ax)> shp(ax)-1){
	  blc(ax)=0;
	  addNextAx=True;
	}
       
      }
    }
  }

  void FFT2D::c2cFFT(Lattice<DComplex>& inout, Bool toFreq){
    IPosition shp=inout.shape();
    if(shp.nelements() <2)
      throw(AipsError("Lattice has to be 2 dimensional to use FFT2D"));
    Long x= inout.shape()(0);
    Long y=inout.shape()(1);
    Long numplanes=inout.shape().product()/x/y;
    IPosition blc(inout.shape().nelements(), 0);
    IPosition shape=inout.shape();
    for (uInt ax=2; ax < shp.nelements(); ++ax)
      shape(ax)=1;
    Array<DComplex> arr;
    Bool isRef;
    Bool del;
    DComplex *scr;

    for (Long n=0; n< numplanes; ++n){
      isRef=inout.getSlice(arr, blc, shape); 
      scr=arr.getStorage(del);
      c2cFFT(scr, x, y, toFreq);
      arr.putStorage(scr, del);
      if(!isRef)
	inout.putSlice(arr, blc);
      //Now calculate next plane 
      Bool addNextAx=True;
      for (uInt ax=2; ax < shp.nelements(); ++ax){
	if(addNextAx){
	  blc(ax) +=1;
	  addNextAx=False;
	}
	if(blc(ax)> shp(ax)-1){
	  blc(ax)=0;
	  addNextAx=True;
	}
       
      }
    }
  }

  void FFT2D::c2cFFT(Complex*& out, Long x, Long y, Bool toFreq){
    if(x%2 != 0 || y%2 !=0)
      throw(AipsError("Programmer error: FFT2D does not deal with odd numbers on x-y plane"));
    fftShift(out, x, y, True);
    doFFT(out, x, y, toFreq);
    /*Int dim[2]={Int(x), Int(y)};
    if(toFreq){
      
      planC2C_p=fftwf_plan_dft(2, dim,  reinterpret_cast<fftwf_complex *>(out),  reinterpret_cast<fftwf_complex *>(out), FFTW_FORWARD, FFTW_ESTIMATE);
      
      //fft1_p.plan_c2c_forward(IPosition(2, x, y),  out);
    }
    else{
       planC2C_p=fftwf_plan_dft(2, dim,  reinterpret_cast<fftwf_complex *>(out),  reinterpret_cast<fftwf_complex *>(out), FFTW_BACKWARD, FFTW_ESTIMATE);
      //  fft1_p.plan_c2c_backward(IPosition(2, x, y),  out);
    }
    fftwf_execute(planC2C_p);
    */
    fftShift(out, x, y, toFreq);

  }
 void FFT2D::c2cFFT(DComplex*& out, Long x, Long y, Bool toFreq){
    if(x%2 != 0 || y%2 !=0)
      throw(AipsError("Programmer error: FFT2D does not deal with odd numbers on x-y plane"));
    fftShift(out, x, y, True);
    doFFT(out, x, y, toFreq); 
    fftShift(out, x, y, toFreq);

  }
  void FFT2D::doFFT(DComplex*& out, Long x, Long y, Bool toFreq){
    if(useFFTW_p){
      //Will need to seperate the plan from the execute if we want to run this in multiple threads
      Int dim[2]={Int(x), Int(y)};
      if(toFreq){
	
	planC2CD_p=fftw_plan_dft(2, dim,  reinterpret_cast<fftw_complex *>(out),  reinterpret_cast<fftw_complex *>(out), FFTW_FORWARD, FFTW_ESTIMATE);
      
	//fft1_p.plan_c2c_forward(IPosition(2, x, y),  out);
      }
      else{
	planC2CD_p=fftw_plan_dft(2, dim,  reinterpret_cast<fftw_complex *>(out),  reinterpret_cast<fftw_complex *>(out), FFTW_BACKWARD, FFTW_ESTIMATE);
	//  fft1_p.plan_c2c_backward(IPosition(2, x, y),  out);
      }
      fftw_execute(planC2CD_p);
      
    }
    else{
      throw(AipsError("Double precision FFT with FFTPack is not implemented"));
    }
  }
   void FFT2D::doFFT(Complex*& out, Long x, Long y, Bool toFreq){
    if(useFFTW_p){
      //Will need to seperate the plan from the execute if we want to run this in multiple threads
      Int dim[2]={Int(x), Int(y)};
      if(toFreq){
	
	planC2C_p=fftwf_plan_dft(2, dim,  reinterpret_cast<fftwf_complex *>(out),  reinterpret_cast<fftwf_complex *>(out), FFTW_FORWARD, FFTW_ESTIMATE);
      
	//fft1_p.plan_c2c_forward(IPosition(2, x, y),  out);
      }
      else{
	planC2C_p=fftwf_plan_dft(2, dim,  reinterpret_cast<fftwf_complex *>(out),  reinterpret_cast<fftwf_complex *>(out), FFTW_BACKWARD, FFTW_ESTIMATE);
	//  fft1_p.plan_c2c_backward(IPosition(2, x, y),  out);
      }
      fftwf_execute(planC2C_p);
      
    }
    else{
      Int ier;
      Int x1=Int(x);
      Int y1=Int(y);
      if(wsave_p.size()==0){
	wsave_p.resize(2*x1*y1+15);
	lsav_p=2*x1*y1+15;
	Float *wsaveptr=wsave_p.data();
	FFTPack::cfft2i(x1, y1, wsaveptr, lsav_p, ier);
      }
      std::vector<Float> work(2*x1*y1);
      Int lenwrk=2*x1*y1;
      Float* workptr=work.data();
      Float* wsaveptr=wsave_p.data();
      if(toFreq)
	FFTPack::cfft2f(x1, y1, x1, out, wsaveptr, lsav_p, workptr, lenwrk, ier);
      else
	FFTPack::cfft2b(x1, y1, x1, out, wsaveptr, lsav_p, workptr, lenwrk, ier);
    }
  }
  void FFT2D::doFFT(Complex*& out, Float*& in, Long x, Long y){
    if(useFFTW_p){
      Int dim[2]={Int(x), Int(y)};
	
      planR2C_p=fftwf_plan_dft_r2c(2, dim,  in, reinterpret_cast<fftwf_complex *>(out), FFTW_ESTIMATE);
      
      //fft1_p.plan_c2c_forward(IPosition(2, x, y),  out);
     
      fftwf_execute(planR2C_p);
      
    }
    else{
      /*
      Int ier;
      Int x1=Int(x);
      Int y1=Int(y);
      if(wsave_p.size()==0){
	wsave_p.resize(2*x1*y1+15);
	lsav_p=2*x1*y1+15;
	Float *wsaveptr=wsave_p.data();
	FFTPack::cfft2i(x1, y1, wsaveptr, lsav_p, ier);
      }
      std::vector<Float> work(2*x1*y1);
      Int lenwrk=2*x1*y1;
      Float* workptr=work.data();
      Float* wsaveptr=wsave_p.data();
      if(toFreq)
	FFTPack::cfft2f(x1, y1, x1, out, wsaveptr, lsav_p, workptr, lenwrk, ier);
      else
	FFTPack::cfft2b(x1, y1, x1, out, wsaveptr, lsav_p, workptr, lenwrk, ier);
      */
      throw(AipsError("Not implemented FFTPack r2c yet"));
    }
  }

  void FFT2D::fftShift(Complex*& s,  Long x, Long y, Bool toFreq){
    ////Lets try our own flip
    
    Bool gool;
    Complex* scr=s;
    {
      Matrix<Complex> tmpo(x/2, y/2);
      Complex* tmpptr=tmpo.getStorage(gool);
      ////TEST
	  //omp_set_num_threads(1);
	  /////
	  /*
	    #pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
	    for (Long jj=0; jj< y/2; ++jj){
	    for(Long ii=0; ii < x/2; ++ii){
	    tmpptr[jj*x/2+ii]=scr[(y/2-jj-1)*x+(x/2-ii-1)];
	    scr[(y/2)*x+(jj*x+x/2)+ii]=scr[jj*x+ii];
	    }
	    }
	  */
      Float divid=1.0f;
      if(!toFreq)
	divid=1.0f/(Float(x)*Float(y));
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr, divid)
      for (Long jj=0; jj< y/2; ++jj){
	for(Long ii=0; ii < x/2; ++ii){
	  tmpptr[jj*x/2+ii]=scr[(y/2)*x+(jj*x+x/2)+ii]*divid;
	  scr[(y/2)*x+(jj*x+x/2)+ii]=scr[jj*x+ii]*divid;
	}
      }
#pragma omp parallel for default(none) firstprivate(x,y, tmpptr, scr)
	  for (Long jj=0; jj< y/2; ++jj){
	    for(Long ii=0; ii < x/2; ++ii){
	      scr[jj*x+ii]=tmpptr[jj*x/2+ii];
	    }
	  }
#pragma omp parallel for default(none) firstprivate(x,y, tmpptr, scr, divid)
	  for (Long jj=0; jj< y/2; ++jj){
	    for(Long ii=0; ii < x/2; ++ii){
	      tmpptr[jj*x/2+ii]=scr[(jj*x+x/2)+ii]*divid;
	      scr[(jj*x+x/2)+ii]=scr[(y/2)*x+jj*x+ii]*divid;
	    }
	  }
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
	  for (Long jj=0; jj< y/2; ++jj){
	    for(Long ii=0; ii < x/2; ++ii){
	      scr[(y/2)*x+jj*x+ii]=tmpptr[jj*x/2+ii];
	    }
	  }
	  tmpo.putStorage(tmpptr, gool);
    }
    
    ////
    
    //if(rot)
    /*{
      
      Matrix<Complex> tmpo(x, y/2);
      Complex* tmpptr=tmpo.getStorage(gool);
      for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < x; ++ii){
      tmpptr[jj*x+ii]=scr[(y-jj-1)*x+(x-ii-1)];
      scr[(y-jj-1)*x+(x-ii-1)]=scr[jj*x+ii];
      }
      }
      for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < x; ++ii){
      scr[jj*x+ii]= tmpptr[jj*x+ii];
      }
      }
      }*/
    
    
    

  }

void FFT2D::fftShift(DComplex*& s,  Long x, Long y, Bool toFreq){
    ////Lets try our own flip
    
    Bool gool;
    DComplex* scr=s;
    {
      Matrix<DComplex> tmpo(x/2, y/2);
      DComplex* tmpptr=tmpo.getStorage(gool);
      ////TEST
	  //omp_set_num_threads(1);
	  /////
	  /*
	    #pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
	    for (Long jj=0; jj< y/2; ++jj){
	    for(Long ii=0; ii < x/2; ++ii){
	    tmpptr[jj*x/2+ii]=scr[(y/2-jj-1)*x+(x/2-ii-1)];
	    scr[(y/2)*x+(jj*x+x/2)+ii]=scr[jj*x+ii];
	    }
	    }
	  */
      Double divid=1.0f;
      if(!toFreq)
	divid=1.0f/(Double(x)*Double(y));
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr, divid)
      for (Long jj=0; jj< y/2; ++jj){
	for(Long ii=0; ii < x/2; ++ii){
	  tmpptr[jj*x/2+ii]=scr[(y/2)*x+(jj*x+x/2)+ii]*divid;
	  scr[(y/2)*x+(jj*x+x/2)+ii]=scr[jj*x+ii]*divid;
	}
      }
#pragma omp parallel for default(none) firstprivate(x,y, tmpptr, scr)
	  for (Long jj=0; jj< y/2; ++jj){
	    for(Long ii=0; ii < x/2; ++ii){
	      scr[jj*x+ii]=tmpptr[jj*x/2+ii];
	    }
	  }
#pragma omp parallel for default(none) firstprivate(x,y, tmpptr, scr, divid)
	  for (Long jj=0; jj< y/2; ++jj){
	    for(Long ii=0; ii < x/2; ++ii){
	      tmpptr[jj*x/2+ii]=scr[(jj*x+x/2)+ii]*divid;
	      scr[(jj*x+x/2)+ii]=scr[(y/2)*x+jj*x+ii]*divid;
	    }
	  }
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
	  for (Long jj=0; jj< y/2; ++jj){
	    for(Long ii=0; ii < x/2; ++ii){
	      scr[(y/2)*x+jj*x+ii]=tmpptr[jj*x/2+ii];
	    }
	  }
	  tmpo.putStorage(tmpptr, gool);
    }
    
    ////
    
    //if(rot)
    /*{
      
      Matrix<Complex> tmpo(x, y/2);
      Complex* tmpptr=tmpo.getStorage(gool);
      for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < x; ++ii){
      tmpptr[jj*x+ii]=scr[(y-jj-1)*x+(x-ii-1)];
      scr[(y-jj-1)*x+(x-ii-1)]=scr[jj*x+ii];
      }
      }
      for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < x; ++ii){
      scr[jj*x+ii]= tmpptr[jj*x+ii];
      }
      }
      }*/
    
    
    

  }
  void FFT2D::fftShift(Float*& s,  Long x, Long y){
    ////Lets try our own flip
      
    Bool gool;
    Float* scr=s;
    Matrix<Float> tmpo(x/2, y/2);
    Float* tmpptr=tmpo.getStorage(gool);
    ////TEST
    //omp_set_num_threads(1);
    /////
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
    for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < x/2; ++ii){
	tmpptr[jj*x/2+ii]=scr[(y/2)*x+(jj*x+x/2)+ii];
	scr[(y/2)*x+(jj*x+x/2)+ii]=scr[jj*x+ii];
      }
    }
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
    for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < x/2; ++ii){
	scr[jj*x+ii]=tmpptr[jj*x/2+ii];
      }
    }
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
    for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < x/2; ++ii){
	tmpptr[jj*x/2+ii]=scr[(jj*x+x/2)+ii];
	scr[(jj*x+x/2)+ii]=scr[(y/2)*x+jj*x+ii];
      }
    }
#pragma omp parallel for default(none) firstprivate(x, y, tmpptr, scr)
    for (Long jj=0; jj< y/2; ++jj){
      for(Long ii=0; ii < x/2; ++ii){
	scr[(y/2)*x+jj*x+ii]=tmpptr[jj*x/2+ii];
      }
    }
    

    
  }

} //# NAMESPACE CASA - END
