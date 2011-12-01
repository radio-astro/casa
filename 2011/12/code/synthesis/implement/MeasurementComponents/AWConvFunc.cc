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
#include <casa/Utilities/CompositeNumber.h>
#include <casa/ostream.h>
namespace casa{

  AWConvFunc& AWConvFunc::operator=(const AWConvFunc& other)
  {
    if(this!=&other) 
      {
	ATerm_p = other.ATerm_p;
      }
    return *this;

  }

  void AWConvFunc::makePBSq(ImageInterface<Complex>& PB)
  {
    Int sizeX=PB.shape()(0), sizeY=PB.shape()(1);
    IPosition cursorShape(4, sizeX, sizeY, 1, 1), axisPath(4,0,1,2,3);
    Array<Complex> buf; PB.get(buf,False);
    ArrayLattice<Complex> lat(buf, True);
    LatticeStepper latStepper(lat.shape(), cursorShape,axisPath);
    LatticeIterator<Complex> latIter(lat, latStepper);
    
    IPosition start0(4,0,0,0,0), start1(4,0,0,1,0), length(4,sizeX, sizeY,1,1);
    Slicer slicePol0(start0, length), slicePol1(start1, length);
    Array<Complex> pol0, pol1,tmp;

    lat.getSlice(pol0, slicePol0);
    lat.getSlice(pol1, slicePol1);
    tmp = pol0;
    pol0 = pol0*conj(pol1);
    pol1 = tmp*conj(pol1);
  }
  
  void AWConvFunc::makeConvFunction(const ImageInterface<Complex>& image,
  				    const VisBuffer& vb,
  				    const Int wConvSize,
  				    const Float pa,
  				    CFStore& cfs,
  				    CFStore& cfwts)
  {
    LogIO log_l(LogOrigin("AWConvFunc", "makeConvFunction"));
    Int convSize, convSampling, polInUse;
    Double wScale=1; Int bandID_l=-1;
    Array<Complex> convFunc_l, convWeights_l;
    Double cfRefFreq;
    
    Int nx=image.shape()(0);
    if (bandID_l == -1) bandID_l=getVisParams(vb,image.coordinates());
    
    log_l << "Making a new convolution function for PA="
	  << pa*(180/C::pi) << "deg"
	  << LogIO::NORMAL << LogIO::POST;
    
    if(wConvSize>0) {
      log_l << "Using " << wConvSize << " planes for W-projection" << LogIO::POST;
      Double maxUVW;
      maxUVW=0.25/abs(image.coordinates().increment()(0));
      log_l << "Estimating maximum possible W = " << maxUVW
	    << " (wavelengths)" << LogIO::POST;
      
      Double invLambdaC=vb.frequency()(0)/C::c;
      Double invMinL = vb.frequency()((vb.frequency().nelements())-1)/C::c;
      log_l << "wavelength range = " << 1.0/invLambdaC << " (m) to " 
	    << 1.0/invMinL << " (m)" << LogIO::POST;
      if (wConvSize > 1)
	{
	  wScale=Float((wConvSize-1)*(wConvSize-1))/maxUVW;
	  log_l << "Scaling in W (at maximum W) = " << 1.0/wScale
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
    polInUse=ATerm_p->makePBPolnCoords(vb, convSize, convSampling, 
				       image.coordinates(),nx,nx,
				       coords);//,feedStokes_l);
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
    WTerm wterm;
    
    for (Int iw=0;iw<wConvSize;iw++) 
      {
	//
	// Fill the complex image with the FT(W-Kernel) for the iw
	// th. w-plane....
	//

	//
	//	wterm.applySky(screen, iw, sampling, wConvSize, wScale, inner);
	wterm.applySky(screen, iw, sampling,  wScale, convSize);
	IPosition PolnPlane(4,0,0,0,0);
	IPosition ndx(4,0,0,0,0);
	//
	// Copy the FT(W-Kernel) to all polarization planes of the image.	
	//
	for(Int i=0;i<polInUse;i++)
	  {
	    PolnPlane(2)=i;
	    twoDPB.putSlice(screen, PolnPlane);
	    twoDPBSq.putSlice(screen, PolnPlane);
	  }
	//
	// Multiply the image planes with appropriate PB functions.  
	// This results in FT(W-Kernel) x FT(A-Kernel).
	//
	Bool doSquint=True;
	ATerm_p->applySky(twoDPB, vb, doSquint, 0);
	ATerm_p->applySky(twoDPBSq, vb, doSquint, 0);
	makePBSq(twoDPBSq);
	// {
	//   ostringstream name;
	//   name << "twoDPBSq.before." << iw << ".im";
	//   storeImg(name,twoDPBSq);
	// }
	
	Complex cpeak=max(twoDPB.get());
	twoDPB.put(twoDPB.get()/cpeak);
	cpeak=max(twoDPBSq.get());
	twoDPBSq.put(twoDPBSq.get()/cpeak);
	
	CoordinateSystem cs=twoDPB.coordinates();
	Int index= twoDPB.coordinates().findCoordinate(Coordinate::SPECTRAL);
	SpectralCoordinate SpCS = twoDPB.coordinates().spectralCoordinate(index);
	
	cfRefFreq=SpCS.referenceValue()(0);
	Vector<Double> refValue; refValue.resize(1); refValue(0)=cfRefFreq;
	SpCS.setReferenceValue(refValue);
	cs.replaceCoordinate(SpCS,index);
	//
	// Now FFT and get the result.  This is FTInverse [
	// FT(W-Kernel) x FT(A-Kernel) ] which is equal to convoultion
	// of the A-Kernel with the W-Kernel.
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
	// Fill the convolution function planes with the result.  This
	// fills the convFunc_l buffer, one w-plane in a loop.
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
	Float threshold, wtThreshold;
	Int R;
	ndx(2) = iw;
	
	ndx(0)=ndx(1)=ConvFuncOrigin;
	ndx(2) = iw;
	threshold   = abs(convFunc_l(ndx))*ATerm_p->getSupportThreshold();
	wtThreshold = abs(convWeights_l(ndx))*ATerm_p->getSupportThreshold();
	//
	// Find the support size of the conv. function in pixels
	//
	Int wtR;
	found = findSupport(convWeights_l,wtThreshold,ConvFuncOrigin,wtR);
	found = findSupport(convFunc_l,threshold,ConvFuncOrigin,R);
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

		cfwts.xSupport(iw)=cfwts.ySupport(iw)=Int(wtR*ATerm_p->getConvWeightSizeFactor()/
							  cfwts.sampling(0));
		cfwts.xSupport(iw)=cfwts.ySupport(iw)=Int(0.5+Float(wtR)*
							  ATerm_p->getConvWeightSizeFactor()/
							  cfwts.sampling(0))+1;

		if (cfs.maxXSupport == -1)
		  if (cfs.xSupport(iw) > maxConvSupport)
		    maxConvSupport = cfs.xSupport(iw);
		if (cfwts.maxXSupport == -1)
		  if (cfwts.xSupport(iw) > maxConvWtSupport)
		    maxConvWtSupport = cfwts.xSupport(iw);
	      }
	  }
      }
    
    if(cfs.xSupport(0)<1) 
      log_l << "Convolution function is misbehaved - support seems to be zero"
	    << LogIO::EXCEPTION;
    
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
	// CompositeNumber compNum;
	// cerr << "Nearest larger composite number = " << tmp.shape()[0] 
	//      << " "  << compNum.nextLarger(tmp.shape()[0]) << endl;
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
    Vector<Double> refValue; refValue.resize(1);refValue(0)=cfRefFreq;
    spCS.setReferenceValue(refValue);
    coords.replaceCoordinate(spCS,index);

    cfs.coordSys=coords;         cfwts.coordSys=coords; 
    cfs.pa=Quantity(pa,"rad");   cfwts.pa=Quantity(pa,"rad");

    //    ATerm_p->makeConvFunction(image,vb,wConvSize,pa,cfs,cfwts);
  }

  Bool AWConvFunc::findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R)
  {
    LogIO log_l(LogOrigin("AWConvFunc", "findSupport"));
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
				       ImageInterface<Float>& theavgPB,
				       Bool reset)
  {
    TempImage<Complex> complexPB;
    Bool pbMade;
    pbMade = makeAverageResponse(vb, image, complexPB,reset);
    normalizeAvgPB(complexPB, theavgPB);	
    return pbMade;
  }
  
  Bool AWConvFunc::makeAverageResponse(const VisBuffer& vb, 
				       const ImageInterface<Complex>& image,
				       ImageInterface<Complex>& theavgPB,
				       Bool reset)
  {
    LogIO log_l(LogOrigin("AWConvFunc","makeAverageResponse(Complex)"));

    log_l << "Making the average response for " << ATerm_p->name() 
	  << LogIO::NORMAL  << LogIO::POST;
    
    if (reset)
      {
	log_l << "Initializing the average PBs"
	      << LogIO::NORMAL << LogIO::POST;
	theavgPB.resize(image.shape()); 
	theavgPB.setCoordinateInfo(image.coordinates());
	theavgPB.set(1.0);
      }

    ATerm_p->applySky(theavgPB, vb, True, 0);
    
    return True; // i.e., an average PB was made 
  }
  //
  //---------------------------------------------------------------
  //
  void AWConvFunc::normalizeAvgPB(ImageInterface<Complex>& inImage,
				  ImageInterface<Float>& outImage)
  {
    LogIO log_l(LogOrigin("AWConvFunc", "normalizeAvgPB"));

    IPosition inShape(inImage.shape()),ndx(4,0,0,0,0);
    Vector<Complex> peak(inShape(2));
    
    outImage.resize(inShape);
    outImage.setCoordinateInfo(inImage.coordinates());

    Bool isRefIn, isRefOut;
    Array<Complex> inBuf;
    Array<Float> outBuf;

    isRefIn  = inImage.get(inBuf);
    isRefOut = outImage.get(outBuf);
    log_l << "Normalizing the average PBs to unity"
	  << LogIO::NORMAL << LogIO::POST;
    //
    // Normalize each plane of the inImage separately to unity.
    //
    Complex inMax = max(inBuf);
    if (abs(inMax)-1.0 > 1E-3)
      {
	for(ndx(3)=0;ndx(3)<inShape(3);ndx(3)++)
	  for(ndx(2)=0;ndx(2)<inShape(2);ndx(2)++)
	    {
	      peak(ndx(2)) = 0;
	      for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
		for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
		  if (abs(inBuf(ndx)) > peak(ndx(2)))
		    peak(ndx(2)) = inBuf(ndx);
	      
	      for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
		for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
		  //		      avgPBBuf(ndx) *= (pbPeaks(ndx(2))/peak(ndx(2)));
		  inBuf(ndx) /= peak(ndx(2));
	    }
	if (isRefIn) inImage.put(inBuf);
      }

    ndx=0;
    for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
      for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
	{
	  IPosition plane1(ndx);
	  plane1=ndx;
	  plane1(2)=1; // The other poln. plane
	  //	  avgPBBuf(ndx) = (avgPBBuf(ndx) + avgPBBuf(plane1))/2.0;
	  outBuf(ndx) = sqrt(real(inBuf(ndx) * inBuf(plane1)));
	}
    //
    // Rather convoluted way of copying Pol. plane-0 to Pol. plane-1!!!
    //
    for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
      for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
	{
	  IPosition plane1(ndx);
	  plane1=ndx;
	  plane1(2)=1; // The other poln. plane
	  outBuf(plane1) = real(outBuf(ndx));
	}
  }
  //
  //-------------------------------------------------------------------------
  // Legacy code.  Should ultimately be deleteted after re-facatoring
  // is finished.
  //
  Bool AWConvFunc::makeAverageResponse_org(const VisBuffer& vb, 
					   const ImageInterface<Complex>& image,
					   ImageInterface<Float>& theavgPB,
					   Bool reset)
  {
    LogIO log_l(LogOrigin("AWConvFunc", "makeAverageResponse_org"));
    TempImage<Float> localPB;
    
    log_l << "Making the average response for " << ATerm_p->name() << LogIO::NORMAL << LogIO::POST;
    
    localPB.resize(image.shape()); localPB.setCoordinateInfo(image.coordinates());
    if (reset)
      {
	log_l << "Initializing the average PBs" << LogIO::NORMAL << LogIO::POST;
	theavgPB.resize(localPB.shape()); 
	theavgPB.setCoordinateInfo(localPB.coordinates());
	theavgPB.set(0.0);
      }
    //
    // Make the Stokes PB
    //
    localPB.set(1.0);

    // Block<CountedPtr<ImageInterface<Float > > > tmpBlock(1);
    // tmpBlock[0]=CountedPtr<ImageInterface<Float> >(&localPB, False);
    // ATerm_p->applySky(tmpBlock, vb, 0, False);
    ATerm_p->applySky(localPB, vb, False, 0);

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
	// Accumulate the shifted PBs
	//
	{
	  Bool isRefF,isRefC;
	  Array<Float> fbuf;
	  Array<Complex> cbuf;
	  isRefF=theavgPB.get(fbuf);
	  isRefC=localTwoDPB.get(cbuf);
	  
	  IPosition fs(fbuf.shape());
	  IPosition ndx(4,0,0,0,0),avgNDX(4,0,0,0,0);
	  for(ndx(3)=0,avgNDX(3)=0;ndx(3)<fs(3);ndx(3)++,avgNDX(3)++)
	    for(ndx(2)=0,avgNDX(2)=0;ndx(2)<twoDPBShape(2);ndx(2)++,avgNDX(2)++)
	      for(ndx(0)=0,avgNDX(0)=0;ndx(0)<fs(0);ndx(0)++,avgNDX(0)++)
		for(ndx(1)=0,avgNDX(1)=0;ndx(1)<fs(1);ndx(1)++,avgNDX(1)++)
		  {
		    Float val;
		    val = real(cbuf(ndx));
		    fbuf(avgNDX) += val;
		  }
	  if (!isRefF) theavgPB.put(fbuf);
	}
      }
    theavgPB.setCoordinateInfo(localPB.coordinates());
    return True; // i.e., an average PB was made
  }
  void AWConvFunc::prepareConvFunction(const VisBuffer& vb, CFStore& cfs)
  {
    ATerm_p->rotate(vb,cfs);
  };

};
