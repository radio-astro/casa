//# FringeVisJones.cc: Implementation of Fringe fitting
//# Copyright (C) 1996-2003,2004
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

#include <synthesis/MeasurementComponents/FringeVisJones.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <msvis/MSVis/VisBuffer.h>

#include <coordinates/Coordinates/LinearCoordinate.h>

#include <casa/sstream.h>

#include <casa/Arrays.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/LogiArray.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Exceptions/Error.h>
#include <scimath/Fitting/LSQFit.h>
#include <scimath/Functionals/ScalarSampledFunctional.h>
#include <scimath/Functionals/Interpolate1D.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/BasicSL/Constants.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/Stokes.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/Aipsrc.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>

#include <casa/iostream.h>

namespace casa {
// ------------------------------------------------------------------
// Start of methods for GJonesDelayRateSB

#undef FRING
#ifdef FRING

GJonesDelayRateSB::GJonesDelayRateSB(VisSet& vs, Double interval, 
				     Double preavg, Double deltat, 
				     Int refant) : 
  GJones(vs, interval, preavg, deltat, refant) { init(); }

GJonesDelayRateSB::~GJonesDelayRateSB() {}

// Initial single-band fringe-fitter
Bool GJonesDelayRateSB::solve (VisEquation& me)
{
  LogMessage message(LogOrigin("GJonesDelayRateSB","solve"));
  
  // For the moment, several FF-specific options are hard-wired here:
  Bool doFFT=True;
  Bool doAlefPorcas=True;
  Double FFTthresh=5.0;
  
  if (doAlefPorcas) doFFT=True;
  
  Bool doStack=False;
  Bool doLS=False;
  
  Int pad(4);
  Vector<Double> fraterange(2);
  fraterange=0.0;
  Vector<Double> delayrange(2);
  delayrange=0.0;
  
  // Make a local copy of the Measurement Equation so we can change
  // some of the entries
  VisEquation lme(me);
  
  // Set the Jones matrix in the local copy of the Measurement Equation 
  lme.setVisJones(*this);
  
  // Tell the VisEquation to use the internal (chunked) VisSet
  AlwaysAssert(vs_, AipsError);
  lme.setVisSet(*vs_);
  
  // Count number of failed solutions
  Int failed=0;
  
  VisIter& vi(vs_->iter());
  VisBuffer vb(vi);
  
  // Vector of reference times for chunks:
  Vector<Double> reftime;
  
  // Iterate chunks
  Int slot, chunk;
  for(chunk=0, vi.originChunks(); vi.moreChunks(); vi.nextChunk(),chunk++) {
    
    // Map the data chunk to a unique time slot number
    //    Double midTime=(MJDStart_(chunk)+MJDStop_(chunk)) / 2.0;
    // Use midpoint of data:
    Double midTime=( min(vb.time()) + max(vb.time())  ) / 2.0;
    slot=findSlot(midTime);
    
    // Which spectral window is this?
    Int spw=vi.spectralWindow();
    
    {
      ostringstream o; o<<typeName()<<" Slot="<<chunk+1<<", " 
			<< fieldName_(chunk)<<", spw="<<spw+1<<": "
			<< MVTime(MJDStart_(slot)/86400.0)<<" to "
			<< MVTime(MJDStop_(slot)/86400.0);
      message.message(o);
      logSink().post(message);
      
    }
    {
      ostringstream o; o << "==========================================================";
      message.message(o);
      logSink().post(message);
      
    }
    
    // initialize ChiSquare calculation
    lme.initChiSquare(*this);
    
    // Invalidate the antenna gain cache
    invalidateCache();
    
    // Construct an FFT server
    FFTServer<Float,Complex> ffts;
    
    // The ME at GJonesDelayRateSB:
    VisBuffer correctedvb = lme.corrected();
    VisBuffer corruptedvb = lme.corrupted();
    
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // STEP 1:  Bookkeeping for this chunk
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // Get time, indexing columns into vectors:
    Vector<Double> vbtimes = correctedvb.time();
    Vector<Int> vbant1 = correctedvb.antenna1();
    Vector<Int> vbant2 = correctedvb.antenna2();
    
    // min/max times:
    Double tmin(min(vbtimes(vbtimes>0.0).getCompressedArray()));
    Double tmax(max(vbtimes));
    
    
    // Number of channels
    Int nChan = correctedvb.nChannel();
    
    // Number/names/indices of parallel-hand pols:
    Int nPol = 2;
    if (correctedvb.corrType().nelements() == 1) nPol = 1;
    Vector<String> polname(nPol);
    Vector<Int> polpos(nPol);
    polname(0)=Stokes::name(Stokes::type(correctedvb.corrType()(0)));
    polpos(0)=0;
    if (nPol==2) {
      if (polznSwitch(correctedvb)) {
	polname(1)=Stokes::name(Stokes::type(correctedvb.corrType()(0)+1));
	polpos(1)=1;
      } else {
	polname(1)=Stokes::name(Stokes::type(correctedvb.corrType()(0)+3));
	polpos(1)=3;
      }
    }
    
    // Total number of possible ifrs:
    Int nifr=numberAnt_*(numberAnt_-1)/2;  
    
    // Vector of antennas in unique ifrs:
    Vector<Int> ifr1(nifr,0);
    Vector<Int> ifr2(nifr,0);
    
    // Vector to hold sampling interval (sec) for each ifr
    Vector<Double> ifrsamp(nifr,0.0);
    
    // Vector to hold time population of this ifr:
    Vector<Int> ifrpop(nifr,0);
    
    // Will count ifrs deliberately in case some are missing:
    nifr=0;
    
    // Naively loop over all antenna combinations to
    // find the complete set of baselines in this chunk,
    // and the population and sampling interval of each:
    
    // only iant2 > iant1
    for (Int iant1=0; iant1<(numberAnt_-1); iant1++) {
      for (Int iant2=iant1+1; iant2<numberAnt_; iant2++) {  
	
        // If this ifr is necessary/desired:
        //  (if !doAlefPorcas nor !doStack, then only reference ifrs pass)
        Bool dothisifr=(doAlefPorcas || 
			doStack || 
			(iant1==refant() || iant2==refant()));
	if ( dothisifr ) {
	  
          // An array selecting unflagged rows containing this ifr
          Vector<Bool> ifrmask(vbtimes.nelements());
	  ifrmask=((!correctedvb.flagRow()) && 
		   anyEQ(True,!correctedvb.flag()) &&
		   (vbant1==iant1) && (vbant2==iant2) );
	  
          // The masked list of times for this ifr:
	  MaskedArray<Double> mvbtimes(vbtimes,ifrmask);
	  
          // If there are any good rows for this ifr,
          //  record this ifr in ifr lists, and get 
          //  its sampling interval:
          //       (may wish to raise minimum number?)
	  
          //  Will need to be careful here when multi-Spw-capable.
          //   e.g., in case different Spws have different sampling.
	  
	  ifrpop(nifr)=mvbtimes.nelementsValid();
          if (ifrpop(nifr) > 0) {
	    ifr1(nifr)=iant1;
	    ifr2(nifr)=iant2;
	    
	    // Now, only the times for this ifr:
	    Vector<Double> ifrtimes(mvbtimes.getCompressedArray());
	    
	    
            // Find sampling interval for this ifr:
            //  (the average of intervals < 10% greater than the minimum)
	    Vector<Double> dt(ifrpop(nifr)-1);
	    dt=ifrtimes(Slice(1,ifrpop(nifr)-1,1)) -
	      ifrtimes(Slice(0,ifrpop(nifr)-1,1)); 
            ifrsamp(nifr)=mean( dt(dt>0.0 && 1.1*min(dt)>dt).getCompressedArray() );
	    
            // Increment ifr counter:
	    nifr++;
	  } // if (ifrpop...)
	} // if (dothisifr...)
      } // for (iant2...)
    } // for (iant1...)
    
    
    // The actual ifr lists are only those where we
    //  required *and* found data:
    ifr1.resize(nifr,True);
    ifr2.resize(nifr,True);
    ifrsamp.resize(nifr,True);
    ifrpop.resize(nifr,True);
    
    // Vectors to store baseline solutions:
    Vector<Double> ifrphase(nifr);
    Vector<Double> ifrdelay(nifr);
    Vector<Double> ifrfrate(nifr);
    Vector<Double> ifrdrate(nifr);
    Vector<Double> ifrsnr(nifr);
    
    // The global sampling interval is minimum of ifrsamp:
    Double samp=min(ifrsamp(ifrsamp>0.0).getCompressedArray());
    
    // The number of samples is the total time-range / samp + 1:
    Int nSamp=1 + ifloor(0.5+(tmax - tmin)/samp);
    
    // The reference time for this chunk is:
    reftime.resize(chunk+1,True);
    reftime(chunk)=tmin + samp*static_cast<Double>(nSamp/2);
    
    // The SpW center frequency and increment:
    //   (careful here when multi-SpW capable!!)
    Vector<Double> freqs = correctedvb.frequency();
    Double chanInc = freqs(1) - freqs(0);
    Double centerFreq = mean(freqs);
    
    
    
    // End of STEP 1.
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    
    // For each polarization:
    for (Int ipol=0; ipol<nPol; ipol++) {
      
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // STEP 2:  Gridding and, as necessary, sync interpolation/shift:
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      // We will stage data into a (nSamp,nChan,nifr) grid.
      //   This will (eventually) make stacking easier and more efficient.
      // If this array turns out to be too large in memory, may have
      //   to consider doing much of this step on-the-fly at the beginning 
      //   of STEP 3. 
      // Using nSamp as first index since this makes access for
      //   interpolation quicker.
      IPosition gridsize(3,nSamp,nChan,nifr);
      Cube<Complex> gdata(gridsize,0.0);
      
      
      // Keep track of valid cells for each ifr:
      Vector<Int> nvalid(nifr,0);
      
      // An vector of row numbers:
      Vector<Int> rows(vbtimes.nelements());
      indgen(rows);
      
      // For each ifr:
      for (Int ifr=0; ifr<nifr; ifr++) {
	// An array selecting unflagged rows containing this ifr
	Vector<Bool> ifrmask(vbtimes.nelements());
	ifrmask=((!correctedvb.flagRow()) && 
		 anyEQ(True,!correctedvb.flag()) &&
		 (vbant1==ifr1(ifr)) && (vbant2==ifr2(ifr)) );
	
	// A masked vector of row numbers:
	MaskedArray<Int> mifrrows(rows,ifrmask);      
	// The list of row numbers for this ifr:
	Vector<Int> ifrrows=mifrrows.getCompressedArray();
	
	// A vector to hold offset for each timestamp (time units):
	Vector<Double> toffset(ifrrows.nelements());
	
	for (uInt irow=0; irow<ifrrows.nelements() ; irow++) {
	  // The time index for this row:
	  Int sampidx=ifloor(0.25+(vbtimes(ifrrows(irow))-tmin)/samp);
	  
	  // The time offset for this sample:
	  toffset(irow)=vbtimes(ifrrows(irow))-(tmin+samp*static_cast<Double>(sampidx));
	  
	  // Copy each unflagged channel to staging grid:
	  for (Int ichan=0; ichan<nChan ; ichan++) {
	    if ( !correctedvb.flag()(ichan,ifrrows(irow)) ) {
       	      nvalid(ifr)++;
	      gdata(sampidx,ichan,ifr) = 
		correctedvb.visibility()(ichan,ifrrows(irow))(polpos(ipol))/
		corruptedvb.visibility()(ichan,ifrrows(irow))(polpos(ipol));
	    } 	    
	  } // for (ichan...)
	} // for (irow...)
	
        // If necessary, apply interpolation and/or shift to each channel:
	// Undersampling factor is rounded ifrsamp/samp:
	Double usampfact=floor(0.5 + ifrsamp(ifr)/samp);
	// Shift (in samples) is mean(time offsets)/samp:
	Double tshift=mean(toffset)/samp;
	
	
	// If convolution/shift required:
	if (usampfact > 1.01 || tshift > 0.01) {
	  
	  // A vector to contain the 1D transformed convolving function:
	  Vector<Complex> tconv(nSamp);
	  tconv=0.0;
	  Int istart(0), istop(nSamp-1);
	  
	  // Adjust istart/istop for box function of appropriate width:
	  if (usampfact > 1.01) {
	    istart=(nSamp/2)-(nSamp/static_cast<Int>(usampfact)/2);
	    istop= (nSamp/2)+(nSamp/static_cast<Int>(usampfact)/2);
	    // Edges of box at 0.5:
	    tconv(istart)=0.5;
	    tconv(istop)=0.5;
	    // Top of box=1.0
	    tconv(Slice(istart+1,istop-istart-1,1))=1.0;
	  } else {
	    // No undersampling:
	    tconv=1.0;
	  }
	  
	  // Do shift?
	  if (tshift > 0.01) {
	    for (Int icnv=istart; icnv<=istop; icnv++) {
	      // sign is opposite that of forward transform for negative shift in time
	      Float arg=-2.0*C::pi*static_cast<Float>(icnv-nSamp/2)*tshift/static_cast<Float>(nSamp);
	      tconv(icnv)=tconv(icnv)*Complex(cos(arg),sin(arg));
	    }
	  }
	  
	  // this ifr
	  Matrix<Complex> thisifr(gdata.xyPlane(ifr));
	  
	  // Loop over each channel and apply tconv:
	  for (Int ichan=0; ichan<nChan; ichan++) {
	    // this chan:
	    Vector<Complex> thischan(thisifr.column(ichan));
	    
	    // transform forward:
	    ffts.fft(thischan,True);
	    
	    // apply tconv:
	    for (Int isamp=0; isamp<nSamp; isamp++) {
	      thischan(isamp)=thischan(isamp)*tconv(isamp);
	    }
	    
	    // transform back:
	    ffts.fft(thischan,False);
	    
	  } // for (ichan...)
	  
	} // if undersamp'd or shift required
	
      } // for (ifr...)
      
      
      // End of STEP 2.
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // STEP 3:  Now do (stacking), padding, transform, and search
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      // Make a coordsys to describe the fringe-rate/delay grid:
      Vector<Double> dfrrefpix(2); 
      dfrrefpix(0)=pad*nSamp/2;
      dfrrefpix(1)=pad*nChan/2;
      Vector<Double> dfrrefval(2); 
      dfrrefval(0)=0.0;
      dfrrefval(1)=0.0;
      Vector<Double> dfrinc(2); 
      dfrinc(0)=1.0/(pad*nSamp*samp);
      dfrinc(1)=1.0/(pad*nChan*chanInc);   // *fringe* rate
      Matrix<Double> pc(2,2); 
      pc= 0; pc.diagonal() = 1.0;
      Vector<String> dfrname(2);  
      dfrname(0) = "Fringe-rate";
      dfrname(1) = "Delay";
      Vector<String> dfrunits(2); 
      dfrunits(0) = "Hz";
      dfrunits(1) = "sec";
      LinearCoordinate dfrcoord(dfrname,dfrunits,dfrrefval,dfrinc,pc,dfrrefpix);
      // Set search ranges to whole if not already specified
      if (fraterange(0)==fraterange(1)) {
	fraterange(0)=-0.5/samp;
	fraterange(1)=abs(fraterange(0));
      }
      if (delayrange(0)==delayrange(1)) {
	delayrange(0)=-0.5/chanInc;
	delayrange(1)=abs(delayrange(0));
      }
      
      // Find pixel coords of search range:
      Vector<Double> pixvect(2), wldvect(2);
      //  blc:
      wldvect(0)=fraterange(0);
      wldvect(1)=delayrange(0);
      dfrcoord.toPixel(pixvect,wldvect);
      pixvect(0)=max(0.0,pixvect(0));
      pixvect(1)=max(0.0,pixvect(1));
      
      IPosition blc(2, ifloor(pixvect(0)) , ifloor(pixvect(1)) );
      //  trc:
      wldvect(0)=fraterange(1);
      wldvect(1)=delayrange(1);
      dfrcoord.toPixel(pixvect,wldvect);
      pixvect(0)=min((Float)(pad*nSamp-1),pixvect(0));
      pixvect(1)=min((Float)(pad*nChan-1),pixvect(1));
      
      IPosition trc(2, ifloor(pixvect(0)) , ifloor(pixvect(1)) );
      
      // Form the (large) matrix in which the padded FFT occurs 
      IPosition fftsize(2,pad*nSamp,pad*nChan);
      Matrix<Complex> fftgrid(fftsize,0.0);
      
      // Form the reference to central region of fftgrid, into which
      //  the actual unpadded gdata is copied
      //  (note integer arithmetic in samp0 and chan0)
      Int samp0 = pad*nSamp/2 - nSamp/2;
      Int chan0 = pad*nChan/2 - nChan/2;
      Matrix<Complex> fftcenter(fftgrid(Slice(samp0,nSamp,1),Slice(chan0,nChan,1)));
      
      // Form array that references only the search region of fftgrid
      Matrix<Complex> fftsearch(fftgrid(blc,trc));
      
      // Announce baselines solutions:
      if (doAlefPorcas) {
	{
	  ostringstream o; o<<"Baseline solutions for " << polname(ipol) << ":";
	  message.message(o);
	  logSink().post(message);
	}
	{
	  ostringstream o; o << "----------------------------------------------------------";
	  message.message(o);
	  logSink().post(message);
	}
      }
      
      for (Int ifr=0; ifr<nifr; ifr++) {
	
        // Only do reference baselines unless doAlefPorcas:
	Bool dothisifr;
	dothisifr = doAlefPorcas || ifr1(ifr)==refant() || ifr2(ifr)==refant();
	
	if ( dothisifr ) {
	  
	  // Ensure fftgrid starts off full of zero:
	  fftgrid=0.0;
	  
	  // Copy this baseline into proper portion of fftspace
	  fftcenter=gdata.xyPlane(ifr);
	  
          // Add 2BL stacking components (TBD)
	  
          // Add 3BL stacking components (TBD)
	  
	  Vector<Float> tr(6);
	  tr(0)=-1.0f;
	  tr(1)=1.0f;
	  tr(2)=0.0f;
	  tr(3)=-1.0f;
	  tr(4)=0.0f;
	  tr(5)=1.0f;
	  Matrix<Float> mph(fftsize);
	  phase(mph,fftgrid);
	  mph=mph*180.0/C::pi;
	  
          // Do transform:
	  ffts.fft(fftgrid,True);
	  
	  Matrix<Float> mamp(fftsize);
	  amplitude(mamp,fftgrid);
	  
	  // Find amplitude peak (smax @ pixmax) only over specified range:
	  Float smin,smax;
	  IPosition pixmin(2),pixmax(2);
	  minMax(smin,smax,pixmin,pixmax,amplitude(fftsearch));
	  
	  Vector<Float> x(1),y(1);
	  
	  x(0)=static_cast<Float>(pixmax(0));
	  y(0)=static_cast<Float>(pixmax(1));
	  
	  // put pixmax in overall pix coords:
	  pixmax=pixmax+blc;
	  
	  // Translate pixel coords to world coords:
	  Vector<Double> dfrpixpeak(2), dfrpeak(2);
	  dfrpixpeak(0)=(Double) pixmax(0);
	  dfrpixpeak(1)=(Double) pixmax(1);
	  dfrcoord.toWorld(dfrpeak, dfrpixpeak);
	  
	  // Mask pixels containing peak and its sidelobes
	  IPosition fftsize(2,pad*nSamp,pad*nChan);
	  Matrix<Bool> rmsgrid(fftsize,T);
	  rmsgrid=False;
	  Int minmask, width;
	  // Avoid unsampled part of transform:
	  Int usampfact=ifloor(0.5 + ifrsamp(ifr)/samp);
	  Int istart=(pad*nSamp/2)-(pad*nSamp/usampfact/2);
	  Int istop= (pad*nSamp/2)+(pad*nSamp/usampfact/2);
	  
	  rmsgrid(Slice(istart,istop-istart,1), Slice(0,pad*nChan,1))=True;
	  minmask=pixmax(0)-pad;
	  minmask=max(0,minmask);
	  width=min(2*pad+1, pad*nSamp-minmask);
	  rmsgrid(Slice(minmask,width,1),Slice(0,pad*nChan,1))=False;
	  minmask=pixmax(1)-pad;
	  minmask=max(0,minmask);
	  width=min(2*pad+1, pad*nChan-minmask);
	  rmsgrid(Slice(0,pad*nSamp,1),Slice(minmask,width,1))=False;
	  
	  Array<Float> rtrans,itrans;
	  rtrans=real(fftgrid(rmsgrid).getCompressedArray());
	  itrans=imag(fftgrid(rmsgrid).getCompressedArray());
	  
	  IPosition oned(1,rtrans.nelements());
	  
	  Vector<Float> vrtrans(rtrans.reform(oned)),vitrans(itrans.reform(oned));
	  
	  
	  // Find rms in real & imag, snr:
	  Float frms=( stddev(real(fftgrid(rmsgrid).getCompressedArray()))+
                       stddev(imag(fftgrid(rmsgrid).getCompressedArray())) )/2.0;
	  
          // Store this baseline's solution:
	  ifrphase(ifr)=arg(fftgrid(pixmax));
	  ifrfrate(ifr)=dfrpeak(0);
	  ifrdrate(ifr)=ifrfrate(ifr)/centerFreq;
	  ifrdelay(ifr)=dfrpeak(1);
	  ifrsnr(ifr)=smax/frms;	    
	  
	  // Report this baseline's solution:
	  if (doAlefPorcas) {
	    ostringstream o; o << "BL=" << ifr1(ifr)+1 << "-" << ifr2(ifr)+1 
			       << ": Ph= " << ifrphase(ifr)*180.0/C::pi << "deg" 
			       << ", D= " << ifrdelay(ifr)/1.0e-9 << "ns" 
			       << ", FR= " << ifrfrate(ifr) << "Hz"
			       << ", SNR= " << ifrsnr(ifr);
	    message.message(o);
	    logSink().post(message);
	  }
	  
	} // if (dothisifr)
      } // for (ifr...)
      
      
      // End of STEP 3.
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // STEP 4:  If desired, do Alef-Porcas fit, else adopt antenna
      //           solutions from reference baseline solutions:
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      // Space to store the antenna-based solutions:
      Matrix<Double> antsol(3,numberAnt_);
      
      // Reference into antsol for p, d, fr:
      Vector<Double> antPhase(antsol.row(0));
      Vector<Double> antDelay(antsol.row(1));
      Vector<Double> antFRate(antsol.row(2));
      // Space for the Delay Rates, filled in later
      Vector<Double> antDRate(numberAnt_);
      
      // Solution OK flag:
      Vector<Bool> antsolok(numberAnt_,False);
      
      if (doAlefPorcas) {
	
	// The fitter tool; simultaneous fit for ph, d, fr, for each ant:
	LSQFit apfitphase(numberAnt_);
	LSQFit apfitdelay(numberAnt_);
	LSQFit apfitfrate(numberAnt_);
	
	// Space for the +/-1,0 coeffs:
	Vector<Double> coeffs(numberAnt_);
	
	// Build LLS problem:
	for (Int iIfr=0; iIfr<nifr ; iIfr++) {
	  
	  // If ifr snr is ok, include 
	  if (ifrsnr(iIfr) > FFTthresh) {
	    // zero the coeffs:
	    coeffs=0.0;
	    
	    // Set solution OK:
	    antsolok(ifr1(iIfr))=True;
	    antsolok(ifr2(iIfr))=True;
	    
	    // Set coeffs for this ifr, and antsolok=True:
	    if (ifr1(iIfr)!=refant()) coeffs(ifr1(iIfr))=1.0;
	    if (ifr2(iIfr)!=refant()) coeffs(ifr2(iIfr))=-1.0;
	    
	    // Stack together obs'd values for simultaneous soln:
	    Vector<Double> obs(3);
	    obs(0)=ifrphase(iIfr);
	    obs(1)=ifrdelay(iIfr);
	    obs(2)=ifrfrate(iIfr);
	    
	    // Make the normal equation for this ifr:
	    apfitphase.makeNorm(coeffs.data, ifrsnr(iIfr)*ifrsnr(iIfr),
				ifrphase(iIfr));
	    apfitdelay.makeNorm(coeffs.data, ifrsnr(iIfr)*ifrsnr(iIfr),
				ifrdelay(iIfr));
	    apfitfrate.makeNorm(coeffs.data, ifrsnr(iIfr)*ifrsnr(iIfr),
				ifrfrate(iIfr));
	  }
	}
	
	// Are we fully determined?
	uInt rank(0);
	Bool ok=apfitphase.invert(rank);
	ok=apfitdelay.invert(rank);
	ok=apfitfrate.invert(rank);
	
	// Return solution in apsol:
	//  (solve wants an unsized array in which to deposit solutions)
	Matrix<Double> apsol(3,numberAnt_);
	
	// If so, then do fit:
	if (ok) {
	  apfitphase.solve(&apsol(0,0));
	  apfitdelay.solve(&apsol(1,0));
	  apfitfrate.solve(&apsol(2,0));
	} else {
	  cout << "Underdetermined!";
	}
	
	// Copy solutions to antsol:
	if (antsol.conform(apsol)) {
	  antsol=apsol;
	} else {
	  cout << "Solve array does not conform to solution array!" << endl;
	}
	
      } else {
	// For each ref ifr, copy to antenna solutions:
	for (Int iIfr=0; iIfr<nifr ; iIfr++) {
	  // only ref ifrs with enough SNR:
	  if ( (ifr1(iIfr)==refant() || ifr2(iIfr)==refant()) && ifrsnr(iIfr)>FFTthresh ) {
	    Int iant;
	    Double antsgn;
	    // Which ant is ref; set sign:
	    if (ifr1(iIfr)==refant()) {
	      iant=ifr2(iIfr);
	      antsgn=-1.0;
	    } else {
	      iant=ifr1(iIfr);
	      antsgn=1.0;
	    }
	    antsolok(iant) = True;
	    antPhase(iant) = antsgn*ifrphase(iIfr);
	    antDelay(iant) = antsgn*ifrdelay(iIfr);
	    antFRate(iant) = antsgn*ifrfrate(iIfr);
	  } // if (ref ifr and snr)
	} // for (iIfr...)
      } // !doAlefPorcas
      
      
      {
	ostringstream o; o<<"Antenna solutions @ " << MVTime(reftime(chunk)/86400.0)
			  << " for " << polname(ipol) << ":";
	message.message(o);
	logSink().post(message);
      }
      {
	ostringstream o; o << "----------------------------------------------------------";
	message.message(o);
	logSink().post(message);
      }
      
      for (Int iAnt=0; iAnt<numberAnt_ ; iAnt++ ) {
        // Calculate Delay rate:
        antDRate(iAnt) = antFRate(iAnt)/centerFreq;
	
	// Report good antenna-based solutions:
	if (antsolok(iAnt) && iAnt!=refant()) {
	  ostringstream o; o << "Ant=" << iAnt+1 
			     << ": Ph= " << antPhase(iAnt)*180.0/C::pi << "deg" 
			     << ", D= " << antDelay(iAnt)/1.0e-9 << "ns" 
			     << ", FR= " << antFRate(iAnt) << "Hz";
	  message.message(o);
	  logSink().post(message);
	}
      }
      
      
      // AK: At this point, we have antenna-based solutions appropriate
      //  for storage in a Cal Table for the current polarization (ipol).
      //  The reference time/freq for these solutions is the center (N/2 
      //  in both dimensions) of the unpadded FFT grid.  The reference 
      //  time is stored in reftime(chunk) (seconds since MJD=0), and the
      //  reference frequency is that of the (N/2)th channel and is stored 
      //  in centerFreq (Hz).
      //  The solutions (indexed by antenna) are in antPhase (rad), 
      //  antDelay (s), antFRate (Hz), and antDRate (s/s).  The 
      //  delay rate is calculated at centerFreq (NOT the REF_FREQ) of
      //  the SpW.  (There is potential for confusion here since the
      //  same freq must be used in applying the delay rate.)
      
      //  Only those antennas with baseline solution(s) with SNR>=5
      //   are passed to the Alef-Porcas solution.  There is not yet
      //   a measure of antenna-based solutin "SNR", but the solution
      //   should be good if the baseline solution(s) were ok.
      
      //  To apply these solutions to baseline a-b:
      
      //    corrected=raw / exp[ i(antPhase(a)-antPhase(b)) +
      //          i*2*pi*(freq-centerFreq)*(antDelay(a)-antDelay(b)) +
      //          i*2*pi*centerFreq*(time-reftime)*(antDRate(a)-antDRate(b))]
      
      //  (i.e., these solutions are opposite in sign to that
      //   produced by FRING; this should be verified by actually
      //   applying a cal table and looking at the data)
      
      
      
      // End of STEP 4.
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      
      
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // STEP 5:  If desired, do LLS fit:
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      
      //   (TBD)
      
      // End of STEP 5.
      //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    } // for (ipol...)
  } // for (chunk...)
  
  return(True);
}

#endif

} //# End namespace casa
