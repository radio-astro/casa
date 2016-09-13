// -*- C++ -*-
//# AWConvFunc2.cc: Implementation of the AWConvFunc2 class
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
//
#include <synthesis/TransformMachines/AWConvFunc2.h>
#include <synthesis/TransformMachines/SynthesisError.h>
#include <images/Images/ImageInterface.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/BeamCalc.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/CFStore2.h>
#include <synthesis/TransformMachines/PSTerm.h>
#include <synthesis/TransformMachines/WTerm.h>
#include <synthesis/TransformMachines/ATerm.h>
#include <synthesis/TransformMachines/VLACalcIlluminationConvFunc.h>
#include <synthesis/TransformMachines/ConvolutionFunction.h>
#include <synthesis/TransformMachines/PolOuterProduct.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <lattices/LatticeMath/LatticeFFT.h>
#include <casa/Utilities/CompositeNumber.h>
#include <casa/OS/Timer.h>
#include <ostream>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace casa{
  //
  //----------------------------------------------------------------------
  //
  void AWConvFunc2::fillConvFuncBuffer(CFBuffer& cfb, CFBuffer& cfWtb,
				      const Int& nx, const Int& ny, 
				      const Vector<Double>& freqValues,
				      const Vector<Double>& wValues,
				      const Double& wScale,
				      const PolMapType& muellerElements,
				      const PolMapType& muellerElementsIndex,
				      const VisBuffer& vb, const Float& psScale,
				      PSTerm& psTerm, WTerm& wTerm, ATerm& aTerm)
  {
    // Unused variable from the dark-ages era interface that should ultimately go.
    (void)psScale;
    (void)muellerElementsIndex;

    LogIO log_l(LogOrigin("AWConvFunc2", "fillConvFuncBuffer[R&D]"));
    //    Int ttt=0;
    Complex cfNorm, cfWtNorm, cpeak, wtcpeak;
    Double vbPA = getPA(vb);
    for (uInt inu=0;inu<freqValues.nelements();inu++) // All freq. channels
      {
	//
	// Cache the A-Term Jones matrix for this frequency
	//
	Float sampling, samplingWt;
	Int xSupport, ySupport, xSupportWt, ySupportWt, MNdx=0;
	CoordinateSystem cs_l;
	Int conjFreqIndex, bandID;
	//
	// Extract the CF parameters from the CFBuffer
	//
	cfb.getParams(cs_l, sampling, xSupport, ySupport, freqValues(inu), wValues(0), MNdx);
	cfb.getParams(cs_l, sampling, xSupport, ySupport, freqValues(inu), wValues(0), MNdx);
	//
	// Compute the conjugate frequency for the current frequency.
	//
	Double conjFreq=SynthesisUtils::conjFreq(freqValues(inu),imRefFreq_p);
	conjFreq=SynthesisUtils::nearestValue(freqValues, conjFreq, conjFreqIndex);
	//
	// Modify the CS.  This converts the Stokes axis to the
	// require value (not necessary in this full-Jones version)
	// and converts from Stoke to feed-basis (always necessary).
	//
	CoordinateSystem modifiedPolCS_l=cs_l;  makeConjPolAxis(modifiedPolCS_l);
	IPosition jonesShape(4,nx,ny,1,4);

	TempImage<Complex> jones_l(jonesShape, modifiedPolCS_l);
	jones_l.set(Complex(1.0,0.0));
	aTerm.makeFullJones(jones_l, vb, True, bandID,-1);

	for (uInt imx=0;imx<muellerElements.nelements();imx++) // Loop over all MuellerElements
	  for (uInt imy=0;imy<muellerElements(imx).nelements();imy++)
	    cerr << imx << " " << imy << " " << muellerElements(imx)(imy) << endl;

	{
	  String name("fullJones.im");
	  storeImg(name, jones_l);
	}

	cerr << "Exiting AWConvFunc2.cc:169" << endl;
	exit(0);

	for (uInt imx=0;imx<muellerElements.nelements();imx++) // Loop over all MuellerElements
	  for (uInt imy=0;imy<muellerElements(imx).nelements();imy++)
	      {
		// Extract the parameters index by (MuellerElement, Freq, W)
		cfWtb.getParams(cs_l, samplingWt, xSupportWt, ySupportWt, 
				freqValues(inu), 
				//				wValues(iw), 
				wValues(0), 
				muellerElements(imx)(imy));
		cfb.getParams(cs_l, sampling, xSupport, ySupport, 
			      freqValues(inu), 
			      wValues(0), 
			      muellerElements(imx)(imy));
		IPosition pbshp(4,nx,ny,1,1);
		//
		// Cache the A-Term for this polarization and frequency
		//

		CoordinateSystem conjPolCS_l=cs_l;  makeConjPolAxis(conjPolCS_l);
		TempImage<Complex> ftATerm_l(pbshp, cs_l), ftATermSq_l(pbshp,conjPolCS_l);
		Bool doSquint=True; Complex tt;
		ftATerm_l.set(Complex(1.0,0.0));   ftATermSq_l.set(Complex(1.0,0.0));
		aTerm.applySky(ftATerm_l, vb, doSquint, 0);
		aTerm.applySky(ftATermSq_l, vb, doSquint, 0,conjFreq);
		
		Int directionIndex=cs_l.findCoordinate(Coordinate::DIRECTION);
		DirectionCoordinate dc=cs_l.directionCoordinate(directionIndex);
		Vector<Double> cellSize;
		cellSize = dc.increment();

		//
		// Now compute the PS x W-Term and apply the cached
		// A-Term to build the full CF.
		//
    		for (uInt iw=0;iw<wValues.nelements();iw++)     // All w-planes
    		  {
		    log_l << " CF("
			  << "M:"<<muellerElements(imx)(imy) 
			  << ",C:" << inu 
			  << ",W:" << iw << "): " << LogIO::POST;
    		    Array<Complex> &cfWtBuf=(*(cfWtb.getCFCellPtr(freqValues(inu), wValues(iw), 
								  muellerElements(imx)(imy))->storage_p));
		    Array<Complex> &cfBuf=(*(cfb.getCFCellPtr(freqValues(inu), wValues(iw), 
							      muellerElements(imx)(imy))->storage_p));
		    
		    cfWtBuf.resize(pbshp);
		    cfBuf.resize(pbshp);

		    const Vector<Double> sampling_l(2,sampling);
		    //		    Double wval = wValues[iw];
		    Matrix<Complex> cfBufMat(cfBuf.nonDegenerate()), 
		      cfWtBufMat(cfWtBuf.nonDegenerate());
		    //
		    // Apply the Prolate Spheroidal and W-Term kernels
		    //
		    Vector<Double> s(2); s=sampling;
		    if (psTerm.isNoOp())
		      cfBufMat = cfWtBufMat = 1.0;
		    else
		      {
			psTerm.applySky(cfBufMat, False);   // Assign (psScale set in psTerm.init()
			psTerm.applySky(cfWtBufMat, False); // Assign
		      }

		    wTerm.applySky(cfBufMat, iw, cellSize, wScale, cfBuf.shape()(0));///4);

    		    IPosition PolnPlane(4,0,0,0,0),
		      pbShape(4, cfBuf.shape()(0), cfBuf.shape()(1), 1, 1);
		    //-------------------------------------------------------------		    
		    TempImage<Complex> twoDPB_l(pbShape, cs_l);
		    TempImage<Complex> twoDPBSq_l(pbShape,cs_l);
		    //-------------------------------------------------------------		    
		    cfWtBuf *= ftATerm_l.get()*conj(ftATermSq_l.get());
		    cfBuf *= ftATerm_l.get();

		    twoDPB_l.putSlice(cfBuf, PolnPlane);
		    twoDPBSq_l.putSlice(cfWtBuf, PolnPlane);
		    Bool PBSQ = False;
		    if(PBSQ) makePBSq(twoDPBSq_l); 

    		    CoordinateSystem cs=twoDPB_l.coordinates();
    		    Int index= twoDPB_l.coordinates().findCoordinate(Coordinate::SPECTRAL);
    		    SpectralCoordinate SpCS = twoDPB_l.coordinates().spectralCoordinate(index);
		    
    		    Double cfRefFreq=SpCS.referenceValue()(0);
    		    Vector<Double> refValue; refValue.resize(1); refValue(0)=cfRefFreq;
    		    SpCS.setReferenceValue(refValue);
    		    cs.replaceCoordinate(SpCS,index);
		    //
		    // Now FT the function and copy the data from
		    // TempImages back to the CFBuffer buffers
		    //
		    //tim.mark();
    		    LatticeFFT::cfft2d(twoDPB_l);
    		    LatticeFFT::cfft2d(twoDPBSq_l);

		    IPosition shp(twoDPB_l.shape());
		    IPosition start(4, 0, 0, 0, 0), pbSlice(4, shp[0]-1, shp[1]-1,1/*polInUse*/, 1),
		      sliceLength(4,cfBuf.shape()[0]-1,cfBuf.shape()[1]-1,1,1);
		    
		    cfBuf(Slicer(start,sliceLength)).nonDegenerate()
		      =(twoDPB_l.getSlice(start, pbSlice, True));
		    
		    shp = twoDPBSq_l.shape();
		    IPosition pbSqSlice(4, shp[0]-1, shp[1]-1, 1, 1),
		      sqSliceLength(4,cfWtBuf.shape()(0)-1,cfWtBuf.shape()[1]-1,1,1);
		    
		    cfWtBuf(Slicer(start,sqSliceLength)).nonDegenerate()
		      =(twoDPBSq_l.getSlice(start, pbSqSlice, True));

		    //
		    // Resize WT-CF
		    //
		    if (iw==0) wtcpeak = max(cfWtBuf);
		    cfWtBuf /= wtcpeak;
		    resizeCF(cfWtBuf, xSupportWt, ySupportWt, samplingWt,0.0);
		    log_l << "CF WT Support: " << xSupport << " (" << xSupportWt << ") " << "pixels" <<  LogIO::POST;

		    Vector<Double> ftRef(2);
		    ftRef(0)=cfWtBuf.shape()(0)/2.0;
		    ftRef(1)=cfWtBuf.shape()(1)/2.0;
		    CoordinateSystem ftCoords=cs_l;
		    SynthesisUtils::makeFTCoordSys(cs_l, cfWtBuf.shape()(0), ftRef, ftCoords);

		    cfWtb.setParams(inu,iw,imx,imy,//muellerElements(imx)(imy),
				    ftCoords, samplingWt, xSupportWt, ySupportWt,
				    freqValues(inu), wValues(iw), muellerElements(imx)(imy));
		    cfWtb.getCFCellPtr(freqValues(inu), wValues(iw), 
				       muellerElements(imx)(imy))->pa_p=Quantity(vbPA,"rad");

		    // 
		    // Resize CF
		    //
		    cpeak = max(cfBuf);  cfBuf /= cpeak;
		    resizeCF(cfBuf, xSupport, ySupport, sampling,0.0);
		    log_l << "CF Support: " << xSupport << " (" << xSupportWt << ") " << "pixels" <<  LogIO::POST;

		    ftRef(0)=cfBuf.shape()(0)/2.0;
		    ftRef(1)=cfBuf.shape()(1)/2.0;

		    if (iw == 0)
		      {
			cfNorm=0; cfWtNorm=0;
			cfNorm = cfArea(cfBufMat, xSupport, ySupport, sampling);
			cfWtNorm = cfArea(cfWtBufMat, xSupportWt, ySupportWt, sampling);
		      }

		    cfBuf /= cfNorm;
		    cfWtBuf /= cfWtNorm;

		    ftCoords=cs_l;
		    SynthesisUtils::makeFTCoordSys(cs_l, cfBuf.shape()(0), ftRef, ftCoords);

		    cfb.setParams(inu,iw,imx,imy,//muellerElements(imx)(imy),
				  ftCoords, sampling, xSupport, ySupport,
				  freqValues(inu), wValues(iw), muellerElements(imx)(imy));
		    cfb.getCFCellPtr(freqValues(inu), wValues(iw), 
				     muellerElements(imx)(imy))->pa_p=Quantity(vbPA,"rad");

		    //
		    // Now tha the CFs have been computed, cache its
		    // paramters in CFCell for quick access in tight
		    // loops (in the re-sampler, e.g.).
		    //
		    (cfWtb.getCFCellPtr(freqValues(inu), wValues(iw), 
					muellerElements(imx)(imy)))->initCache();
		    (cfb.getCFCellPtr(freqValues(inu), wValues(iw), 
				      muellerElements(imx)(imy)))->initCache();
    		  }
	      }
    	}
  }
};
