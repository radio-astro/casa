//# SimplePBConvFunc.cc: implementation of SimplePBConvFunc
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
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
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

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>


#include <synthesis/MeasurementComponents/SimplePBConvFunc.h>
#include <synthesis/MeasurementComponents/SkyJones.h>

namespace casa { //# NAMESPACE CASA - BEGIN

   SimplePBConvFunc::SimplePBConvFunc(): PixelatedConvFunc<Complex>(),
					convFunctionMap_p(-1), 
					actualConvIndex_p(-1), convSize_p(0), convSupport_p(0) {
    //

     pbClass_p=PBMathInterface::COMMONPB;
  }

  SimplePBConvFunc::SimplePBConvFunc(const PBMathInterface::PBClass typeToUse): PixelatedConvFunc<Complex>(),
					convFunctionMap_p(-1), 
					actualConvIndex_p(-1), convSize_p(0), convSupport_p(0) {
    //
    pbClass_p=typeToUse;

  }

  SimplePBConvFunc::~SimplePBConvFunc(){
    //

  }

  void SimplePBConvFunc::findConvFunction(const ImageInterface<Complex>& iimage, 
					  const VisBuffer& vb,
					  const Int& convSampling,
					  Cube<Complex>& convFunc, 
					  Cube<Complex>& weightConvFunc, 
					  Vector<Int>& convsize,
					  Vector<Int>& convSupport,
					  Vector<Int>& convFuncMap
					  ){

    //Only one plane in this version
    convFuncMap.resize();
    convFuncMap=Vector<Int>(vb.nRow(),0);
    //break reference
    convFunc.resize();
    weightConvFunc.resize();
    if(checkPBOfField(vb)){
      convFunc.reference(*(convFunctions_p[actualConvIndex_p]));
      weightConvFunc.reference(*(convWeights_p[actualConvIndex_p]));
      convsize=Vector<Int>(1,convSize_p);
      convSupport=Vector<Int>(1,convSupport_p);
      return;
    }
    LogIO os;
    os << LogOrigin("SimplePBConv", "findConvFunction")  << LogIO::NORMAL;
  
    
  // Get the coordinate system
    CoordinateSystem coords(iimage.coordinates());
    
    
    
    // Make a two dimensional image to calculate the
    // primary beam. We want this on a fine grid in the
    // UV plane 
    Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
    AlwaysAssert(directionIndex>=0, AipsError);

    // Set up the convolution function.
    Int nx=iimage.shape()(directionIndex);
    Int ny=iimage.shape()(directionIndex+1);
    convSize_p=max(nx,ny)*convSampling;

    DirectionCoordinate dc=coords.directionCoordinate(directionIndex);
    Vector<Double> sampling;
    sampling = dc.increment();
    sampling*=Double(convSampling);
    sampling(0)*=Double(nx)/Double(convSize_p);
    sampling(1)*=Double(ny)/Double(convSize_p);
    dc.setIncrement(sampling);
    
    Vector<Double> unitVec(2);
    unitVec=convSize_p/2;
    dc.setReferencePixel(unitVec);
    
    // Set the reference value to that of the pointing position of the
    // current buffer since that's what will be applied
    //####will need to use this when using a much smaller convsize than image.
    //  getXYPos(vb, 0);
    // dc.setReferenceValue(worldPosMeas.getAngle().getValue());
    
    coords.replaceCoordinate(dc, directionIndex);
    //  coords.list(logIO(), MDoppler::RADIO, IPosition(), IPosition());
    
    IPosition pbShape(4, convSize_p, convSize_p, 1, 1);
    TempImage<Complex> twoDPB(pbShape, coords);
    
    
    
    convFunc_p.resize(convSize_p, convSize_p);
    convFunc_p=0.0;
    
    IPosition start(4, 0, 0, 0, 0);
    IPosition pbSlice(4, convSize_p, convSize_p, 1, 1);
    
    // Accumulate terms 
    Matrix<Complex> screen(convSize_p, convSize_p);
    screen=1.0;
    // Either the SkyJones
    twoDPB.putSlice(screen, start);
    sj_p->apply(twoDPB, twoDPB, vb, 0); 
    
    //*****Test
    TempImage<Complex> twoDPB2(pbShape, coords);
    {
      TempImage<Float> screen2(pbShape, coords);
      Matrix<Float> screenoo(convSize_p, convSize_p);
      screenoo.set(1.0);
      screen2.putSlice(screenoo,start);
      sj_p->applySquare(screen2, screen2, vb, 0);
      LatticeExpr<Complex> le(screen2);
      twoDPB2.copyData(le);
    }
    //****************
    
    //addBeamCoverage(twoDPB);
    
    
    // Now FFT and get the result back
    LatticeFFT::cfft2d(twoDPB);
    LatticeFFT::cfft2d(twoDPB2);
    
    // Write out FT of screen as an image
    if(0) {
      CoordinateSystem ftCoords(coords);
      directionIndex=ftCoords.findCoordinate(Coordinate::DIRECTION);
      AlwaysAssert(directionIndex>=0, AipsError);
      dc=coords.directionCoordinate(directionIndex);
      Vector<Bool> axes(2); axes(0)=True;axes(1)=True;
      Vector<Int> shape(2); shape(0)=convSize_p;shape(1)=convSize_p;
      Coordinate* ftdc=dc.makeFourierCoordinate(axes,shape);
      ftCoords.replaceCoordinate(*ftdc, directionIndex);
      delete ftdc; ftdc=0;
      ostringstream os1;
      os1 << "FTScreen_" << vb.fieldId() ;
      PagedImage<Float> thisScreen(pbShape, ftCoords, String(os1));
      LatticeExpr<Float> le(abs(twoDPB));
      thisScreen.copyData(le);
    }
    
    convFunc_p=twoDPB.get(True);
    //convFunc/=max(abs(convFunc));
    Float maxAbsConvFunc=max(amplitude(convFunc_p));
    
    Float minAbsConvFunc=min(amplitude(convFunc_p));
    convSupport_p=-1;
    Bool found=False;
    Bool found2=True;
    Int trial2=0;
    Int trial=0;
    for (trial=convSize_p/2-2;trial>0;trial--) {
      //Searching down a diagonal
      if(abs(convFunc_p(convSize_p/2-trial,convSize_p/2-trial)) >  (1.0e-2*maxAbsConvFunc)) {
	found=True;
	trial=Int(sqrt(2.0*Float(trial*trial)));
	break;
      }
    }
    /*
    for (trial2=convSize_p/2-2;trial2>0;trial2--) {
      //Searching from centre moving away
      if(abs(convFunc_p(trial2,trial2)) <  (5.0e-2*maxAbsConvFunc)) {
	found2=True;
	break;
      }
    }
    
    if(found2 && (trial2 < trial)){
      trial=trial2;
      found=True;
    }
    */
    if(!found){
      if((maxAbsConvFunc-minAbsConvFunc) > (1.0e-2*maxAbsConvFunc)) 
	found=True;
      // if it drops by more than 2 magnitudes per pixel
      trial=3;
    }
    /*
    if(trial < 3) 
      trial=3;
    */
    if(found) {
      convSupport_p=Int(0.5+Float(trial)/Float(convSampling))+1;
    }
    else {
      os << "Convolution function is misbehaved - support seems to be zero\n"
	 << "Reasons can be: \nThe image definition not covering one or more of the pointings selected \n"
         << "Or no unflagged data in a given pointing"
	 << LogIO::EXCEPTION;
    }
    
    // Normalize such that plane 0 sums to 1 (when jumping in
    // steps of convSampling)
    
    Double pbSum=0.0;
    for (Int iy=-convSupport_p;iy<=convSupport_p;iy++) {
      for (Int ix=-convSupport_p;ix<=convSupport_p;ix++) {
	Complex val=convFunc_p(ix*convSampling+convSize_p/2,
			       iy*convSampling+convSize_p/2);
	
	pbSum+=sqrt(real(val)*real(val)+ imag(val)*imag(val));
      }
    }
    
    if(pbSum>0.0) {
      convFunc_p*=Complex(1.0/pbSum,0.0);
    }
    else {
      os << "Convolution function integral is not positive"
	      << LogIO::EXCEPTION;
    }
    
    //##########################################
    os << "Convolution support = " << convSupport_p
	    << " pixels in Fourier plane"
	    << LogIO::POST;
    
    convSupportBlock_p.resize(actualConvIndex_p+1);
    convSizes_p.resize(actualConvIndex_p+1);
    //Only one beam for now...but later this should be able to
    // take all the beams for the different antennas.
    convSupportBlock_p[actualConvIndex_p]=new Vector<Int>(1);
    convSizes_p[actualConvIndex_p]=new Vector<Int> (1);
    (*(convSupportBlock_p[actualConvIndex_p]))[0]=convSupport_p;
    convFunctions_p.resize(actualConvIndex_p+1);
    convWeights_p.resize(actualConvIndex_p+1);
    convFunctions_p[actualConvIndex_p]= new Cube<Complex>();
    convWeights_p[actualConvIndex_p]= new Cube<Complex>();
    Int newConvSize=2*(convSupport_p+2)*convSampling;
    //NEED to chop this right ...and in the centre
    if(newConvSize < convSize_p){
      IPosition blc(2, (convSize_p/2)-(newConvSize/2),
		    (convSize_p/2)-(newConvSize/2));
      IPosition trc(2, (convSize_p/2)+(newConvSize/2-1),
		    (convSize_p/2)+(newConvSize/2-1));
      convFunctions_p[actualConvIndex_p]->resize(newConvSize, newConvSize, 1);
      convFunctions_p[actualConvIndex_p]->xyPlane(0)=convFunc_p(blc,trc);
      convSize_p=newConvSize;
      convWeights_p[actualConvIndex_p]->resize(newConvSize, newConvSize, 1);
      convWeights_p[actualConvIndex_p]->xyPlane(0)=twoDPB2.get(True)(blc,trc)*Complex(1.0/pbSum,0.0);
      
      convFunc_p.resize();//break any reference
      weightConvFunc_p.resize();
      convFunc_p.reference(convFunctions_p[actualConvIndex_p]->xyPlane(0));
      weightConvFunc_p.reference(convWeights_p[actualConvIndex_p]->xyPlane(0));
      (*convSizes_p[actualConvIndex_p])[0]=convSize_p;
    }
    else{
      convFunctions_p[actualConvIndex_p]->resize(convSize_p, convSize_p,1);
      convFunctions_p[actualConvIndex_p]->xyPlane(0)=convFunc_p;
    }
   

    convFunc.reference(*(convFunctions_p[actualConvIndex_p]));
    weightConvFunc.reference(*(convWeights_p[actualConvIndex_p]));
    convsize=Vector<Int>(1,convSize_p);
    convSupport=Vector<Int>(1,convSupport_p);
    
    
  }


  void SimplePBConvFunc::setSkyJones(SkyJones* sj){
    sj_p=sj;
  }

  Bool SimplePBConvFunc::checkPBOfField(const VisBuffer& vb){
    Int fieldid=vb.fieldId();
    Int msid=vb.msId();
    String mapid=String::toString(msid)+String("_")+String::toString(fieldid);
    if(convFunctionMap_p.ndefined() == 0){
      convFunctionMap_p.define(mapid, 0);    
      actualConvIndex_p=0;
      return False;
    }
    
    if(!convFunctionMap_p.isDefined(mapid)){
      actualConvIndex_p=convFunctionMap_p.ndefined();
      convFunctionMap_p.define(mapid, actualConvIndex_p);
      return False;
    }
    else{
      actualConvIndex_p=convFunctionMap_p(mapid);
      convFunc_p.resize(); // break any reference
      weightConvFunc_p.resize(); 
      //Here we will need to use the right xyPlane for different PA range.
      convFunc_p.reference(convFunctions_p[actualConvIndex_p]->xyPlane(0));
      weightConvFunc_p.reference(convWeights_p[actualConvIndex_p]->xyPlane(0));
      //Again this for one time of antenna only later should be fixed for all 
      // antennas independently
      convSupport_p=(*convSupportBlock_p[actualConvIndex_p])[0];
      convSize_p=(*convSizes_p[actualConvIndex_p])[0];

  }
 
 return True;



  }



} //# NAMESPACE CASA - END
