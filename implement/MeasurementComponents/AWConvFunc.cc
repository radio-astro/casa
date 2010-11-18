// -*- C++ -*-
//# AWConvFunc.cc: Implementation of the AWConvFunc class
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
#include <synthesis/MeasurementComponents/AWConvFunc.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>
#include <synthesis/MeasurementComponents/WTerm.h>
#include <images/Images/ImageInterface.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/BeamCalc.h>
#include <synthesis/MeasurementComponents/CFStore.h>
#include <synthesis/MeasurementComponents/ATerm.h>
#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/ConvolutionFunction.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <lattices/Lattices/LatticeFFT.h>

namespace casa{

  AWConvFunc& AWConvFunc::operator=(const AWConvFunc& other)
  {
    if(this!=&other) 
      {
	ATerm_p = other.ATerm_p;
      }
    return *this;

  }
  
  void AWConvFunc::makeConvFunction(const ImageInterface<Complex>& image,
  				    const VisBuffer& vb,
  				    const Int wConvSize,
  				    const Float pa,
  				    CFStore& cfs,
  				    CFStore& cfwts)
  {
    Int convSize, convSampling, polInUse;
    Double wScale=1; Int bandID_l=-1;
    Array<Complex> convFunc_l, convWeights_l;
    
    Int nx=image.shape()(0);
    if (bandID_l == -1) bandID_l=getVisParams(vb);
    
    logIO() << LogOrigin("AWConvFunc", "makeConvFunction") 
	    << "Making a new convolution function for PA="
	    << pa*(180/C::pi) << "deg"
	    << LogIO::NORMAL 
	    << LogIO::POST;
    
    if(wConvSize>0) {
      logIO() << "Using " << wConvSize << " planes for W-projection" << LogIO::POST;
      Double maxUVW;
      maxUVW=0.25/abs(image.coordinates().increment()(0));
      logIO() << "Estimating maximum possible W = " << maxUVW
	      << " (wavelengths)" << LogIO::POST;
      
      Double invLambdaC=vb.frequency()(0)/C::c;
      Double invMinL = vb.frequency()((vb.frequency().nelements())-1)/C::c;
      logIO() << "wavelength range = " << 1.0/invLambdaC << " (m) to " 
	      << 1.0/invMinL << " (m)" << LogIO::POST;
      if (wConvSize > 1)
	{
	  wScale=Float((wConvSize-1)*(wConvSize-1))/maxUVW;
	  logIO() << "Scaling in W (at maximum W) = " << 1.0/wScale
		  << " wavelengths per pixel" << LogIO::POST;
	}
    }
    //  
    // Get the coordinate system
    //
    CoordinateSystem coords(image.coordinates());
    //
    // Set up the convolution function. 
    //
    if(wConvSize>0) 
      {
	if(wConvSize>256) 
	  {
	    convSampling=4;
	    convSize=min(nx,512);
	  }
	else 
	  {
	    convSampling=4;
	    convSize=min(nx,2048);
	  }
      }
    else 
      {
	convSampling=4;
	convSize=nx;
      }
    convSampling=ATerm_p->getOversampling();
    convSize=ATerm_p->getConvSize();
    //
    // Make a two dimensional image to calculate auto-correlation of
    // the ideal illumination pattern. We want this on a fine grid in
    // the UV plane
    //
    Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
    AlwaysAssert(directionIndex>=0, AipsError);
    DirectionCoordinate dc=coords.directionCoordinate(directionIndex);
    Vector<Double> sampling;
    sampling = dc.increment();
    
    sampling*=Double(convSampling);
    sampling*=Double(nx)/Double(convSize);

    dc.setIncrement(sampling);
    
    Vector<Double> unitVec(2);
    unitVec=convSize/2;
    dc.setReferencePixel(unitVec);
    
    // Set the reference value to that of the image
    coords.replaceCoordinate(dc, directionIndex);
    //
    // Make an image with circular polarization axis.  Return the
    // no. of vis. poln. planes that will be used in making the user
    // defined Stokes image.
    //
    Vector<Int> polMap_l, feedStokes_l;
    ATerm_p->getParams(polMap_l, feedStokes_l);
    polInUse=ATerm_p->makePBPolnCoords(vb, polMap_l, convSize, convSampling, 
				       image.coordinates(),nx,nx,
				       coords,feedStokes_l);
    //------------------------------------------------------------------
    // Make the sky Stokes PB.  This will be used in the gridding
    // correction
    //------------------------------------------------------------------
    IPosition pbShape(4, convSize, convSize, polInUse, 1);
    TempImage<Complex> twoDPB(pbShape, coords);
    IPosition pbSqShp(pbShape);
    
    unitVec=pbSqShp[0]/2;
    dc.setReferencePixel(unitVec);
    coords.replaceCoordinate(dc, directionIndex);
    
    TempImage<Complex> twoDPBSq(pbSqShp,coords);
    twoDPB.set(Complex(1.0,0.0));
    twoDPBSq.set(Complex(1.0,0.0));
    //
    // Accumulate the various terms that constitute the gridding
    // convolution function.
    //
    //------------------------------------------------------------------
    Bool writeResults;
    writeResults=False;
    
    Int inner=convSize/convSampling;
    cfs.data = new Array<Complex>(IPosition(4,convSize,convSize, wConvSize,polInUse));
    cfwts.data = new Array<Complex>(IPosition(4,convSize,convSize, wConvSize,polInUse));
    convFunc_l.reference(*cfs.data);
    convWeights_l.reference(*cfwts.data);
    convFunc_l=0;
    convWeights_l=0.0;
    cfs.resize(wConvSize);
    cfwts.resize(wConvSize);

    IPosition start(4, 0, 0, 0, 0);
    IPosition pbSlice(4, convSize, convSize, 1, 1);
    
    Matrix<Complex> screen(convSize, convSize);
    //    VLACalcIlluminationConvFunc vlaPB;
    WTerm wterm;
    //    Long cachesize=(HostInfo::memoryTotal(true)/8)*1024;
    //    vlaPB.setMaximumCacheSize(cachesize);
    
    for (Int iw=0;iw<wConvSize;iw++) 
      {
	screen = 1.0;
	wterm.apply(screen, iw, sampling, wConvSize, wScale, inner);
	//
	// Fill the complex image with the w-term...
	//
	IPosition PolnPlane(4,0,0,0,0);
	IPosition ndx(4,0,0,0,0);
	
	for(Int i=0;i<polInUse;i++)
	  {
	    PolnPlane(2)=i;
	    twoDPB.putSlice(screen, PolnPlane);
	    twoDPBSq.putSlice(screen, PolnPlane);
	  }
	//
	// Apply the PB...
	//
	Bool doSquint=True;
	ATerm_p->applySky(twoDPB, vb, doSquint);
	doSquint = False;
	//	ATerm_p->applyPBSq(twoDPBSq, vb, bandID_l, doSquint);
	ATerm_p->applySky(twoDPBSq, vb, doSquint);
	// 	twoDPB.put(abs(twoDPB.get()));
	// 	twoDPBSq.put(abs(twoDPBSq.get()));
	// {
	//   String name("twoDPB.before.im");
	//   storeImg(name,twoDPB);
	// }
	
	Complex cpeak=max(twoDPB.get());
	twoDPB.put(twoDPB.get()/cpeak);
	cpeak=max(twoDPBSq.get());
	twoDPBSq.put(twoDPBSq.get()/cpeak);
	//	twoDPBSq.set(1.0);
	// {
	//   String name("twoDPB.im");
	//   storeImg(name,twoDPB);
	// }
	
	CoordinateSystem cs=twoDPB.coordinates();
	Int index= twoDPB.coordinates().findCoordinate(Coordinate::SPECTRAL);
	SpectralCoordinate SpCS = twoDPB.coordinates().spectralCoordinate(index);
	
	Double cfRefFreq=SpCS.referenceValue()(0);
	Vector<Double> refValue; refValue.resize(1); refValue(0)=cfRefFreq;
	SpCS.setReferenceValue(refValue);
	cs.replaceCoordinate(SpCS,index);
	//
	// Now FFT and get the result back
	//
	// {
	//   String name("twoDPB.im");
	//   storeImg(name,twoDPB);
	// }
	LatticeFFT::cfft2d(twoDPB);
	LatticeFFT::cfft2d(twoDPBSq);
	// {
	//   String name("twoDPBFT.im");
	//   storeImg(name,twoDPB);
	// }
	//
	// Fill the convolution function planes with the result.
	//
	{
	  IPosition start(4, 0, 0, 0, 0),
	    pbSlice(4, twoDPB.shape()[0]-1, twoDPB.shape()[1]-1, polInUse, 1);
	  IPosition sliceStart(4,0,0,iw,0), 
	    sliceLength(4,convFunc_l.shape()[0]-1,convFunc_l.shape()[1]-1,1,polInUse);
	  
	  convFunc_l(Slicer(sliceStart,sliceLength)).nonDegenerate()
	    =(twoDPB.getSlice(start, pbSlice, True));
	  
	  IPosition shp(twoDPBSq.shape());
	  
	  IPosition sqStart(4, 0, 0, 0, 0),
	    pbSqSlice(4, shp[0]-1, shp[1]-1, polInUse, 1);
	  IPosition sqSliceStart(4,0,0,iw,0), 
	    sqSliceLength(4,shp[0]-1,shp[1]-1,1,polInUse);
	  
	  convWeights_l(Slicer(sqSliceStart,sqSliceLength)).nonDegenerate()
	    =(twoDPBSq.getSlice(sqStart, pbSqSlice, True));
	  
	}
      }
    
    {
      Complex cpeak = max(convFunc_l);
      convFunc_l/=cpeak;
      cpeak=max(convWeights_l);
      convWeights_l/=cpeak;
    }
    //
    // Find the convolution function support size.  No assumption
    // about the symmetry of the conv. func. can be made (except that
    // they are same for all poln. planes).
    //
    
    Int maxConvSupport=-1;
    Int ConvFuncOrigin=convFunc_l.shape()[0]/2;  // Conv. Func. is half that size of convSize
    IPosition ndx(4,ConvFuncOrigin,0,0,0);
    
    Int maxConvWtSupport=0, supportBuffer;
    for (Int iw=0;iw<wConvSize;iw++)
      {
	Bool found=False;
	Float threshold;
	Int R;
	ndx(2) = iw;
	
	ndx(0)=ndx(1)=ConvFuncOrigin;
	ndx(2) = iw;
	//	Complex maxVal = max(convFunc);
	threshold = abs(convFunc_l(ndx))*ATerm_p->getSupportThreshold();
	//
	// Find the support size of the conv. function in pixels
	//
	Int wtR;
	found = findSupport(convWeights_l,threshold,ConvFuncOrigin,wtR);
	found = findSupport(convFunc_l,threshold,ConvFuncOrigin,R);
	
	//	R *=2.5;
	//
	// Set the support size for each W-plane and for all
	// Pol-planes.  Assuming that support size for all Pol-planes
	// is same.
	//
	if(found) 
	  {
	    //	    Int maxR=R;//max(ndx(0),ndx(1));
	    cfs.sampling(0)=cfwts.sampling(0)=convSampling;
	    for(Int ipol=0;ipol<polInUse;ipol++)
	      {
		cfs.xSupport(iw)=cfs.ySupport(iw)=Int(R/cfs.sampling(0));
		cfs.xSupport(iw)=cfs.ySupport(iw)=Int(0.5+Float(R)/cfs.sampling(0))+1;

		cfwts.xSupport(iw)=cfwts.ySupport(iw)=Int(R*ATerm_p->getConvWeightSizeFactor()/
							  cfwts.sampling(0));
		cfwts.xSupport(iw)=cfwts.ySupport(iw)=Int(0.5+Float(R)*
							  ATerm_p->getConvWeightSizeFactor()/
							  cfwts.sampling(0))+1;

		if (cfs.maxXSupport == -1)
		  if (cfs.xSupport(iw) > maxConvSupport)
		    maxConvSupport = cfs.xSupport(iw);
		  maxConvWtSupport=cfwts.xSupport(iw);//HOW CAN THIS BE RIGHT!!!!
	      }
	  }
      }
    
    if(cfs.xSupport(0)<1) 
      logIO() << "Convolution function is misbehaved - support seems to be zero"
	      << LogIO::EXCEPTION;
    
    logIO() << LogOrigin("AWConvFunc", "makeConvFunction")
	    << "Re-sizing the convolution functions"
	    << LogIO::POST;
    
    {
      supportBuffer = ATerm_p->getOversampling();;
      Int bot=(Int)(ConvFuncOrigin-cfs.sampling[0]*maxConvSupport-supportBuffer),//-convSampling/2, 
	top=(Int)(ConvFuncOrigin+cfs.sampling[0]*maxConvSupport+supportBuffer);//+convSampling/2;
      bot = max(0,bot);
      top = min(top, convFunc_l.shape()(0)-1);
      {
	Array<Complex> tmp;
	IPosition blc(4,bot,bot,0,0), trc(4,top,top,wConvSize-1,polInUse-1);
	//
	// Cut out the conv. func., copy in a temp. array, resize the
	// CFStore.data, and copy the cutout version to CFStore.data.
	//
	tmp = convFunc_l(blc,trc);
	cfs.data->resize(tmp.shape());
	*cfs.data = tmp; 
	convFunc_l.reference(*cfs.data);
      }
      
      supportBuffer = (Int)(ATerm_p->getOversampling()*ATerm_p->getConvWeightSizeFactor());
      bot=(Int)(ConvFuncOrigin-cfwts.sampling[0]*maxConvWtSupport-supportBuffer);
      top=(Int)(ConvFuncOrigin+cfwts.sampling[0]*maxConvWtSupport+supportBuffer);
      bot=max(0,bot);
      top=min(top,convWeights_l.shape()(0)-1);
      {
      	Array<Complex> tmp;
      	IPosition blc(4,bot,bot,0,0), trc(4,top,top,wConvSize-1,polInUse-1);
      
      	tmp = convWeights_l(blc,trc);
	cfwts.data->resize(tmp.shape());
	*cfwts.data = tmp;
	convWeights_l.reference(*cfwts.data);
      }
    }    
    
    //
    // Normalize such that plane 0 sums to 1 (when jumping in steps of
    // convSampling).  This is really not necessary here since we do
    // the normalizing by the area more accurately in the gridder
    // (fpbwproj.f).
    //
    ndx(2)=ndx(3)=0;
    
    
    Complex pbSum=0.0;
    IPosition peakPix(ndx);
    
    Int Nx = convFunc_l.shape()(0), Ny=convFunc_l.shape()(1);
    
    for(Int nw=0;nw<wConvSize;nw++)
      for(Int np=0;np<polInUse;np++)
	{
	  ndx(2) = nw; ndx(3)=np;
	  {
	    //
	    // Locate the pixel with the peak value.  That's the
	    // origin in pixel co-ordinates.
	    //
	    Float peak=0;
	    peakPix = 0;
	    for(ndx(1)=0;ndx(1)<convFunc_l.shape()(1);ndx(1)++)
	      for(ndx(0)=0;ndx(0)<convFunc_l.shape()(0);ndx(0)++)
		if (abs(convFunc_l(ndx)) > peak) {peakPix = ndx;peak=abs(convFunc_l(ndx));}
	  }
	  
	  ConvFuncOrigin = peakPix(0);
	  //	  ConvFuncOrigin = convFunc.shape()(0)/2+1;
	  //	  Int thisConvSupport=convSampling*convSupport(nw,np,lastPASlot);
	  Int thisConvSupport=cfs.xSupport(nw);
	  pbSum=0.0;
	  
	  for(Int iy=-thisConvSupport;iy<thisConvSupport;iy++)
	    for(Int ix=-thisConvSupport;ix<thisConvSupport;ix++)
	      {
		ndx(0)=ix*(Int)cfs.sampling[0]+ConvFuncOrigin;
		ndx(1)=iy*(Int)cfs.sampling[0]+ConvFuncOrigin;
		pbSum += real(convFunc_l(ndx));
	      }
	  if(pbSum>0.0)  
	    {
	      //
	      // Normalize each Poln. plane by the area under its convfunc.
	      //
	      Nx = convFunc_l.shape()(0), Ny = convFunc_l.shape()(1);
	      for (ndx(1)=0;ndx(1)<Ny;ndx(1)++) 
		for (ndx(0)=0;ndx(0)<Nx;ndx(0)++) 
		  {
		    convFunc_l(ndx) /= pbSum;
		  }
	      
	      Nx = convWeights_l.shape()(0); Ny = convWeights_l.shape()(1);
	      for (ndx(1)=0;  ndx(1)<Ny;  ndx(1)++) 
		for (ndx(0)=0;  ndx(0)<Nx;  ndx(0)++) 
		  {
		    convWeights_l(ndx) /= pbSum*pbSum;
		  }
	    }
	  else 
	    throw(SynthesisFTMachineError("Convolution function integral is not positive"));
	  
	  Vector<Float> maxVal(convWeights_l.shape()(2));
	  Vector<IPosition> posMax(convWeights_l.shape()(2));
	  SynthesisUtils::findLatticeMax(convWeights_l,maxVal,posMax); 
	}
    
    Int index=coords.findCoordinate(Coordinate::SPECTRAL);
    SpectralCoordinate spCS = coords.spectralCoordinate(index);
    Vector<Double> refValue; refValue.resize(1);refValue(0)=spCS.referenceValue()(0);
    spCS.setReferenceValue(refValue);
    coords.replaceCoordinate(spCS,index);

    cfs.coordSys=coords;         cfwts.coordSys=coords; 
    cfs.pa=Quantity(pa,"rad");   cfwts.pa=Quantity(pa,"rad");

    //    ATerm_p->makeConvFunction(image,vb,wConvSize,pa,cfs,cfwts);
  }

  Bool AWConvFunc::findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R)
  {
    Double NSteps;
    Int PixInc=1;
    Vector<Complex> vals;
    IPosition ndx(4,origin,0,0,0);
    Bool found=False;
    IPosition cfShape=func.shape();
    Int convSize = cfShape(0);
    for(R=convSize/4;R>1;R--)
      {
	NSteps = 90*R/PixInc; //Check every PixInc pixel along a
	//circle of radious R
	vals.resize((Int)(NSteps+0.5));
	vals=0;
	for(Int th=0;th<NSteps;th++)
	  {
	    ndx(0)=(int)(origin + R*sin(2.0*M_PI*th*PixInc/R));
	    ndx(1)=(int)(origin + R*cos(2.0*M_PI*th*PixInc/R));
	    
	    if ((ndx(0) < cfShape(0)) && (ndx(1) < cfShape(1)))
	      vals(th)=func(ndx);
	  }
	if (max(abs(vals)) > threshold)
	  {found=True;break;}
      }
    return found;
  }

  Bool AWConvFunc::makeAverageResponse(const VisBuffer& vb, 
				       const ImageInterface<Complex>& image,
				       TempImage<Float>& theavgPB,
				       Bool reset)
  {
    TempImage<Float> localPB;
    
    logIO() << LogOrigin("AWConvFunc","makeAverageResponse")
	    << LogIO::NORMAL;
    
    localPB.resize(image.shape()); localPB.setCoordinateInfo(image.coordinates());
    if (reset)
      {
	logIO() << "Initializing the average PBs"
		<< LogIO::NORMAL
		<< LogIO::POST;
	theavgPB.resize(localPB.shape()); 
	theavgPB.setCoordinateInfo(localPB.coordinates());
	theavgPB.set(0.0);
      }
    //
    // Make the Stokes PB
    //
    localPB.set(1.0);

    ATerm_p->applySky(localPB, vb, False);

    IPosition twoDPBShape(localPB.shape());
    TempImage<Complex> localTwoDPB(twoDPBShape,localPB.coordinates());
    //    localTwoDPB.setMaximumCacheSize(cachesize);
    Int NAnt;
    NAnt=1;

    for(Int ant=0;ant<NAnt;ant++)
      { //Ant loop
	{
	  IPosition ndx(4,0,0,0,0);
	  for(ndx(0)=0; ndx(0)<twoDPBShape(0); ndx(0)++)
	    for(ndx(1)=0; ndx(1)<twoDPBShape(1); ndx(1)++)
	      for(ndx(2)=0; ndx(2)<twoDPBShape(2); ndx(2)++)
	       for(ndx(3)=0; ndx(3)<twoDPBShape(3); ndx(3)++)
		  localTwoDPB.putAt(Complex((localPB(ndx)),0.0),ndx);
	}
	//
	// If antenna pointing errors are not applied, no shifting
	// (which can be expensive) is required.
	//
	//
	// Accumulate the shifted PBs
	//
	{
	  Bool isRefF,isRefC;
	  Array<Float> fbuf;
	  Array<Complex> cbuf;
	  isRefF=theavgPB.get(fbuf);
	  isRefC=localTwoDPB.get(cbuf);
	  
	  IPosition fs(fbuf.shape());
	  {
	    IPosition ndx(4,0,0,0,0),avgNDX(4,0,0,0,0);
	    for(ndx(3)=0,avgNDX(3)=0;ndx(3)<fs(3);ndx(3)++,avgNDX(3)++)
	    {
	      for(ndx(2)=0,avgNDX(2)=0;ndx(2)<twoDPBShape(2);ndx(2)++,avgNDX(2)++)
		{
		  for(ndx(0)=0,avgNDX(0)=0;ndx(0)<fs(0);ndx(0)++,avgNDX(0)++)
		    for(ndx(1)=0,avgNDX(1)=0;ndx(1)<fs(1);ndx(1)++,avgNDX(1)++)
		      {
			Float val;
			val = real(cbuf(ndx));
			fbuf(avgNDX) += val;
		      }
		}
	    }
	  }
	  if (!isRefF) theavgPB.put(fbuf);
	}
      }
    theavgPB.setCoordinateInfo(localPB.coordinates());
    return True; // i.e., an average PB was made and is in the mem. cache
  }
};
