//# WPConvFunc.cc: implementation of WPConvFunc
//# Copyright (C) 2007
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$
#include <casa/sstream.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/OS/HostInfo.h>
#include <casa/Utilities/Assert.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>


#include <synthesis/MeasurementComponents/WPConvFunc.h>


namespace casa { //# NAMESPACE CASA - BEGIN


 WPConvFunc::WPConvFunc(): PixelatedConvFunc<Complex>(),
				 convFunctionMap_p(-1), 
				 actualConvIndex_p(-1), convSize_p(0), convSupport_p(0) {
   //
  }

  WPConvFunc::~WPConvFunc(){
    //usage of CountedPtr keeps this simple

  }

  void WPConvFunc::findConvFunction(const ImageInterface<Complex>& image, 
				    const VisBuffer& vb,
				    const Int& wConvSize,
				    const Vector<Double>& uvScale,
				    const Vector<Double>& uvOffset, 
				    Int& convSampling,
				    Cube<Complex>& convFunc, 
				    Int& convSize,
				    Vector<Int>& convSupport, Double& wScale){




  if(checkCenterPix(image)){ 
    convFunc.resize();
    convFunc.reference(convFunc_p);
    convSize=convSize_p;
    convSampling=convSampling_p;
    convSupport=convSupport_p;
    wScale=wScale_p;
    return;
  }
  LogIO os;
  os << LogOrigin("WPConvFunc", "findConvFunction")  << LogIO::NORMAL;
  
  
  if(wConvSize>1) {
    os << "W projection using " << wConvSize << " planes" << LogIO::POST;
    Double maxUVW;
    maxUVW=0.25/abs(image.coordinates().increment()(0));
    os << "Estimating maximum possible W = " << maxUVW
	    << " (wavelengths)" << LogIO::POST;
    
    Double invLambdaC=vb.frequency()(0)/C::c;
    os << "Typical wavelength = " << 1.0/invLambdaC
	    << " (m)" << LogIO::POST;
    
    //    uvScale(2)=sqrt(Float(wConvSize-1))/maxUVW;
    //    uvScale(2)=(Float(wConvSize-1))/maxUVW;
    wScale=Float((wConvSize-1)*(wConvSize-1))/maxUVW;
    wScale_p=wScale;
    os << "Scaling in W (at maximum W) = " << 1.0/wScale_p
	    << " wavelengths per pixel" << LogIO::POST;
  }
  
  // Get the coordinate system
  CoordinateSystem coords(image.coordinates());
  Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
  nx_p=image.shape()(directionIndex); 
  ny_p=image.shape()(directionIndex+1);

  // Set up the convolution function. 
  if(wConvSize>1) {
    /* if(wConvSize>256) {
      convSampling=4;
      convSize=min(nx,ny); 
      Int maxMemoryMB=HostInfo::memoryTotal()/1024; 
      if(maxMemoryMB > 4000){
	convSize=min(convSize,1024);
      }
      else{
	convSize=min(convSize,512);
      }

    }
    else {
      convSampling=4;
      convSize=min(nx,ny);
      convSize=min(convSize,1024);
    }
    */

    Int maxMemoryMB=HostInfo::memoryTotal()/1024;
    //nominal  512 wprojplanes above that you may (or not) go swapping
    Double maxConvSizeConsidered=sqrt(Double(maxMemoryMB)/8.0*1024.0*1024.0/512.0);
    convSampling_p=4;
    convSize=max(nx_p,ny_p);
    convSize=min(convSize,Int(maxConvSizeConsidered/2.0)*2);
    
  }
  else {
    convSampling_p=1;
    convSize=max(nx_p,ny_p);
  }
  convSampling=convSampling_p;
  Int maxConvSize=convSize;
  
  // Make a two dimensional image to calculate the
  // primary beam. We want this on a fine grid in the
  // UV plane 
  AlwaysAssert(directionIndex>=0, AipsError);
  DirectionCoordinate dc=coords.directionCoordinate(directionIndex);
  Vector<Double> sampling;
  sampling = dc.increment();
  sampling*=Double(convSampling_p);
  //sampling*=Double(max(nx,ny))/Double(convSize);
  sampling[0]*=Double(nx_p)/Double(convSize);
  sampling[1]*=Double(ny_p)/Double(convSize);
  dc.setIncrement(sampling);
  
  Vector<Double> unitVec(2);
  unitVec=convSize/2;
  dc.setReferencePixel(unitVec);
  
  // Set the reference value to that of the image center for sure.
  {
    // dc.setReferenceValue(mTangent_p.getAngle().getValue());
    MDirection wcenter;  
    Vector<Double> pcenter(2);
    pcenter(0) = nx_p/2;
    pcenter(1) = ny_p/2;    
    coords.directionCoordinate(directionIndex).toWorld( wcenter, pcenter );
    dc.setReferenceValue(wcenter.getAngle().getValue());
  }
  coords.replaceCoordinate(dc, directionIndex);
  //  coords.list(os, MDoppler::RADIO, IPosition(), IPosition());
  
  IPosition pbShape(4, convSize, convSize, 1, 1);
  TempImage<Complex> twoDPB(pbShape, coords);

  Int inner=convSize/convSampling_p;
  ConvolveGridder<Double, Complex>
    ggridder(IPosition(2, inner, inner), uvScale, uvOffset, "SF");

  convFunc.resize(); // break any reference 
  convFunc.resize(convSize/2-1, convSize/2-1, wConvSize);
  convFunc.set(0.0);

  IPosition start(4, 0, 0, 0, 0);
  IPosition pbSlice(4, convSize, convSize, 1, 1);
  
  Bool writeResults=False;
  Int warner=0;

  // Accumulate terms 
  Matrix<Complex> screen(convSize, convSize);
  for (Int iw=0;iw<wConvSize;iw++) {
    // First the w term
    screen=0.0;
    if(wConvSize>1) {
      //      Double twoPiW=2.0*C::pi*sqrt(Double(iw))/uvScale(2);
      //      Double twoPiW=2.0*C::pi*Double(iw)/uvScale(2);
      Double twoPiW=2.0*C::pi*Double(iw*iw)/wScale_p;
      for (Int iy=-inner/2;iy<inner/2;iy++) {
	Double m=sampling(1)*Double(iy);
	Double msq=m*m;
	for (Int ix=-inner/2;ix<inner/2;ix++) {
	  Double l=sampling(0)*Double(ix);
	  Double rsq=l*l+msq;
	  if(rsq<1.0) {
	    Double phase=twoPiW*(sqrt(1.0-rsq)-1.0);
	    screen(ix+convSize/2,iy+convSize/2)=Complex(cos(phase),sin(phase));
	  }
	}
      }
    }
    else {
      screen=1.0;
    }
    // spheroidal function
    Vector<Complex> correction(inner);
    for (Int iy=-inner/2;iy<inner/2;iy++) {
      ggridder.correctX1D(correction, iy+inner/2);
      for (Int ix=-inner/2;ix<inner/2;ix++) {
	screen(ix+convSize/2,iy+convSize/2)*=correction(ix+inner/2);
      }
    }
    twoDPB.putSlice(screen, IPosition(4, 0));
    // Write out screen as an image
    if(writeResults) {
      ostringstream name;
      name << "Screen" << iw+1;
      if(Table::canDeleteTable(name)) Table::deleteTable(name);
      PagedImage<Float> thisScreen(pbShape, coords, name);
      LatticeExpr<Float> le(real(twoDPB));
      thisScreen.copyData(le);
    }

    // Now FFT and get the result back
    LatticeFFT::cfft2d(twoDPB);

    // Write out FT of screen as an image
    if(writeResults) {
      CoordinateSystem ftCoords(coords);
      directionIndex=ftCoords.findCoordinate(Coordinate::DIRECTION);
      AlwaysAssert(directionIndex>=0, AipsError);
      dc=coords.directionCoordinate(directionIndex);
      Vector<Bool> axes(2); axes(0)=True;axes(1)=True;
      Vector<Int> shape(2); shape(0)=convSize;shape(1)=convSize;
      Coordinate* ftdc=dc.makeFourierCoordinate(axes,shape);
      ftCoords.replaceCoordinate(*ftdc, directionIndex);
      delete ftdc; ftdc=0;
      ostringstream name;
      name << "FTScreen" << iw+1;
      if(Table::canDeleteTable(name)) Table::deleteTable(name);
      PagedImage<Float> thisScreen(pbShape, ftCoords, name);
      LatticeExpr<Float> le(real(twoDPB));
      thisScreen.copyData(le);
    }
    IPosition start(4, convSize/2, convSize/2, 0, 0);
    IPosition pbSlice(4, convSize/2-1, convSize/2-1, 1, 1);
    convFunc.xyPlane(iw)=twoDPB.getSlice(start, pbSlice, True);
  }
  convFunc/=max(abs(convFunc));

  // Find the edge of the function by stepping in from the
  // uv plane edge. We do this for each plane to save time on the
  // gridding (about a factor of two)
  convSupport=-1;
  for (Int iw=0;iw<wConvSize;iw++) {
    Bool found=False;
    Int trial=0;
    for (trial=convSize/2-2;trial>0;trial--) {
      if((abs(convFunc(trial,0,iw))>1e-3)||(abs(convFunc(0,trial,iw))>1e-3) ) {
	found=True;
	break;
      }
    }
    if(found) {
      convSupport(iw)=Int(0.5+Float(trial)/Float(convSampling_p))+1;
      if(convSupport(iw)*convSampling_p*2 >= maxConvSize){
	convSupport(iw)=convSize/2/convSampling_p-1;
	++warner;
      }
    }
  }
  
  if(convSupport(0)<1) {
    os << "Convolution function is misbehaved - support seems to be zero"
	    << LogIO::EXCEPTION;
  }

  if(warner > 5) {
    os << LogIO::WARN 
	    <<"Many of the Convolution functions go beyond " << maxConvSize 
	    <<" pixels allocated" << LogIO::POST;
    os << LogIO::WARN
	    << "You may consider reducing the size of your image or use facets"
	    << LogIO::POST;
  }
  // Normalize such that plane 0 sums to 1 (when jumping in
  // steps of convSampling)
  Double pbSum=0.0;
  for (Int iy=-convSupport(0);iy<=convSupport(0);iy++) {
    for (Int ix=-convSupport(0);ix<=convSupport(0);ix++) {
      pbSum+=real(convFunc(abs(ix)*convSampling_p,abs(iy)*convSampling_p,0));
    }
  }
  if(pbSum>0.0) {
    convFunc*=Complex(1.0/pbSum,0.0);
  }
  else {
    os << "Convolution function integral is not positive"
	    << LogIO::EXCEPTION;
  }
  os << "Convolution support = " << convSupport*convSampling_p
	  << " pixels in Fourier plane"
	  << LogIO::POST;


  convSupportBlock_p.resize(actualConvIndex_p+1);
  convSupportBlock_p[actualConvIndex_p]= new Vector<Int>();
  convSupportBlock_p[actualConvIndex_p]->assign(convSupport);
  convFunctions_p.resize(actualConvIndex_p+1);
  convFunctions_p[actualConvIndex_p]= new Cube<Complex>();
  Int newConvSize=2*(max(convSupport)+2)*convSampling;
  
  if(newConvSize < convSize){
    IPosition blc(3, 0,0,0);
    IPosition trc(3, (newConvSize/2-2),
		  (newConvSize/2-2),
		  convSupport.shape()(0)-1);
    *(convFunctions_p[actualConvIndex_p])=convFunc(blc,trc);
    convSize=newConvSize;
  }
  else{
    (convFunctions_p[actualConvIndex_p])->assign(convFunc);
  }
  Int maxMemoryMB=HostInfo::memoryTotal()/1024;
  Int memoryMB;
  memoryMB = Int(Double(convSize/2-1)*Double(convSize/2-1)*
		 Double(wConvSize)*8.0/1024.0/1024.0);
  os << "Memory used in gridding function = "
	  << memoryMB << " MB from maximum "
	  << maxMemoryMB << " MB" << LogIO::POST;
  convFunc.resize();
  convFunc.reference(*convFunctions_p[actualConvIndex_p]);
  convSizes_p.resize(actualConvIndex_p+1, True);
  convSizes_p(actualConvIndex_p)=convSize;

  }

Bool WPConvFunc::checkCenterPix(const ImageInterface<Complex>& image){

  CoordinateSystem imageCoord=image.coordinates();
  MDirection wcenter;  
  Int directionIndex=imageCoord.findCoordinate(Coordinate::DIRECTION);
  DirectionCoordinate
    directionCoord=imageCoord.directionCoordinate(directionIndex);
  Vector<Double> incr=directionCoord.increment();
  nx_p=image.shape()(directionIndex);
  ny_p=image.shape()(directionIndex+1);


  //Images with same number of pixels and increments can have the same conv functions
  ostringstream oos;
  oos << setprecision(6);

  oos << nx_p << "_"<< fabs(incr(0)) << "_";
  oos << ny_p << "_"<< fabs(incr(1));
  String imageKey(oos);

  if(convFunctionMap_p.ndefined() == 0){
    convFunctionMap_p.define(imageKey, 0);    
    actualConvIndex_p=0;
    return False;
  }
   
  if(!convFunctionMap_p.isDefined(imageKey)){
    actualConvIndex_p=convFunctionMap_p.ndefined();
    convFunctionMap_p.define(imageKey,actualConvIndex_p);
    return False;
  }
  else{
    actualConvIndex_p=convFunctionMap_p(imageKey);
    convFunc_p.resize(); // break any reference
    convFunc_p.reference(*convFunctions_p[actualConvIndex_p]);
    convSupport_p.resize();
    convSupport_p.reference(*convSupportBlock_p[actualConvIndex_p]);
    convSize_p=convSizes_p[actualConvIndex_p];

  }

  return True;
}




} //# NAMESPACE CASA - END
