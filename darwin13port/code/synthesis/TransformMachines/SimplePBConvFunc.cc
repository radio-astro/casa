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
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/OS/Timer.h>
#include <casa/Utilities/Assert.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVAngle.h>
#include <measures/Measures/MeasTable.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <ms/MeasurementSets/MSColumns.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>

#include <scimath/Mathematics/ConvolveGridder.h>

#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisibilityIterator.h>

#include <synthesis/TransformMachines/SimplePBConvFunc.h>
#include <synthesis/TransformMachines/SkyJones.h>

#include <casa/Utilities/CompositeNumber.h>
#include <math.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimplePBConvFunc::SimplePBConvFunc(): nchan_p(-1),
        npol_p(-1), pointToPix_p(), directionIndex_p(-1), thePix_p(0),
        filledFluxScale_p(False),doneMainConv_p(0),
                                      
	calcFluxScale_p(True), convFunctionMap_p(-1), actualConvIndex_p(-1), convSize_p(0), convSupport_p(0), pointingPix_p()  {
    //

    pbClass_p=PBMathInterface::COMMONPB;
}

  SimplePBConvFunc::SimplePBConvFunc(const PBMathInterface::PBClass typeToUse): 
    nchan_p(-1),npol_p(-1),pointToPix_p(),
    directionIndex_p(-1), thePix_p(0), filledFluxScale_p(False),doneMainConv_p(0), 
     calcFluxScale_p(True), convFunctionMap_p(-1), actualConvIndex_p(-1), convSize_p(0), convSupport_p(0), pointingPix_p() {
    //
    pbClass_p=typeToUse;

  }
  SimplePBConvFunc::SimplePBConvFunc(const RecordInterface& rec, const Bool calcfluxneeded)
  : nchan_p(-1),npol_p(-1),pointToPix_p(), directionIndex_p(-1), thePix_p(0), filledFluxScale_p(False),
    doneMainConv_p(0), 
    calcFluxScale_p(calcfluxneeded), convFunctionMap_p(-1), actualConvIndex_p(-1), convSize_p(0), convSupport_p(0), pointingPix_p()
  {
    String err;
    fromRecord(err, rec, calcfluxneeded);
  }
  SimplePBConvFunc::~SimplePBConvFunc(){
    //

  }

  void SimplePBConvFunc::storeImageParams(const ImageInterface<Complex>& iimage,
					  const VisBuffer& vb){
    //image signature changed...rather simplistic for now
    if((iimage.shape().product() != nx_p*ny_p*nchan_p*npol_p) || nchan_p < 1){
      csys_p=iimage.coordinates();
      Int coordIndex=csys_p.findCoordinate(Coordinate::DIRECTION);
      AlwaysAssert(coordIndex>=0, AipsError);
      directionIndex_p=coordIndex;
      dc_p=csys_p.directionCoordinate(directionIndex_p);
      ObsInfo imInfo=csys_p.obsInfo();
      String tel= imInfo.telescope();
      MPosition pos;
      if (vb.msColumns().observation().nrow() > 0) {
	tel = vb.msColumns().observation().telescopeName()(vb.msColumns().observationId()(0));
      }
      if (tel.length() == 0 || 
	  !MeasTable::Observatory(pos,tel)) {
	// unknown observatory, use first antenna
	pos=vb.msColumns().antenna().positionMeas()(0);
      }
      //cout << "TELESCOPE " << tel << endl;
      //Store this to build epochs via the time access of visbuffer later
      timeMType_p=MEpoch::castType(vb.msColumns().timeMeas()(0).getRef().getType());
      timeUnit_p=Unit(vb.msColumns().timeMeas().measDesc().getUnits()(0).getName());
      // timeUnit_p=Unit("s");
      //cout << "UNIT " << timeUnit_p.getValue() << " name " << timeUnit_p.getName()  << endl;
      pointFrame_p=MeasFrame(imInfo.obsDate(), pos);
      MDirection::Ref elRef(dc_p.directionType(), pointFrame_p);
      //For now we set the conversion from this direction 
      pointToPix_p=MDirection::Convert( MDirection(), elRef);
      nx_p=iimage.shape()(coordIndex);
      ny_p=iimage.shape()(coordIndex+1);
      pointingPix_p.resize(nx_p, ny_p);
      pointingPix_p.set(False);
      coordIndex=csys_p.findCoordinate(Coordinate::SPECTRAL);
      Int pixAxis=csys_p.pixelAxes(coordIndex)[0];
      nchan_p=iimage.shape()(pixAxis);
      coordIndex=csys_p.findCoordinate(Coordinate::STOKES);
      pixAxis=csys_p.pixelAxes(coordIndex)[0];
      npol_p=iimage.shape()(pixAxis);
      if(calcFluxScale_p){
    	  if(fluxScale_p.shape().nelements()==0){
    		  fluxScale_p=TempImage<Float>(IPosition(4,nx_p,ny_p,npol_p,nchan_p), csys_p);
    		  fluxScale_p.set(0.0);
    	  }
    	  filledFluxScale_p=False;
      }
      
    }

  }

  void SimplePBConvFunc::toPix(const VisBuffer& vb){
    thePix_p.resize(2);

    if(dc_p.directionType() !=  MDirection::castType(vb.direction1()(0).getRef().getType())){
      //pointToPix_p.setModel(theDir);
      
      MEpoch timenow(Quantity(vb.time()(0), timeUnit_p), timeMType_p);
      //cerr << "Ref " << vb.direction1()(0).getRefString() <<  " ep " << timenow.getRefString() << " time " << MVTime(timenow.getValue().getTime()).string(MVTime::YMD) << endl; 
      pointFrame_p.resetEpoch(timenow);
      ///////////////////////////
      //MDirection some=pointToPix_p(vb.direction1()(0));
      //MVAngle mvRa=some.getAngle().getValue()(0);
      //MVAngle mvDec=some.getAngle().getValue()(1);
      
      //cout  << mvRa(0.0).string(MVAngle::TIME,8) << "   ";
      // cout << mvDec.string(MVAngle::DIG2,8) << "   ";
      //cout << MDirection::showType(some.getRefPtr()->getType()) << endl;

      //////////////////////////
      //pointToPix holds pointFrame_p by reference...
      //thus good to go for conversion
      direction1_p=pointToPix_p(vb.direction1()(0));
      direction2_p=pointToPix_p(vb.direction2()(0));
      dc_p.toPixel(thePix_p, direction1_p);

    }
    else{
      direction1_p=vb.direction1()(0);
      direction2_p=vb.direction2()(0);
      dc_p.toPixel(thePix_p, vb.direction1()(0));
    }
  }

  void SimplePBConvFunc::setWeightImage(CountedPtr<TempImage<Float> >& wgtimage){
    convWeightImage_p=wgtimage;
    calcFluxScale_p=True;

  }
 
  void SimplePBConvFunc::reset(){
    doneMainConv_p.resize();
    convFunctions_p.resize(0, True);
    convWeights_p.resize(0, True);
    convSizes_p.resize(0, True);
    convSupportBlock_p.resize(0, True);
    convFunctionMap_p.clear();
  }



  Int SimplePBConvFunc::convIndex(const VisBuffer& vb){
	  String elkey=String::toString(vb.msId())+String("_")+String::toString(vb.spectralWindow());
	  if(vbConvIndex_p.count(elkey) > 0){
		  return vbConvIndex_p[elkey];
	  }
	  Int val=vbConvIndex_p.size();
	  vbConvIndex_p[elkey]=val;
	  return val;
  }
void SimplePBConvFunc::findConvFunction(const ImageInterface<Complex>& iimage, 
					const VisBuffer& vb,
					const Int& convSampling,
					const Vector<Double>& visFreq, 
					  Array<Complex>& convFunc, 
					  Array<Complex>& weightConvFunc, 
					  Vector<Int>& convsize,
					  Vector<Int>& convSupport,
					  Vector<Int>& convFuncPolMap,
					  Vector<Int>& convFuncChanMap,
					  Vector<Int>& convFuncRowMap
					  ){



    storeImageParams(iimage, vb);
    convFuncChanMap.resize(vb.nChannel());
    Vector<Double> beamFreqs;
    findUsefulChannels(convFuncChanMap, beamFreqs, vb, visFreq);
    //cerr << "CHANMAP " << convFuncChanMap << endl;
    Int nBeamChans=beamFreqs.nelements();
    //indgen(convFuncChanMap);
    convFuncPolMap.resize(vb.nCorr());
    convFuncPolMap.set(0);
    //Only one plane in this version
    convFuncRowMap.resize();
    convFuncRowMap=Vector<Int>(vb.nRow(),0);
    //break reference
    convFunc.resize();
    weightConvFunc.resize();
    LogIO os;
    os << LogOrigin("SimplePBConv", "findConvFunction")  << LogIO::NORMAL;
  
    
    // Get the coordinate system
    CoordinateSystem coords(iimage.coordinates());
    
    
    actualConvIndex_p=convIndex(vb);
    //cerr << "In findConv " << actualConvIndex_p << endl;
    // Make a two dimensional image to calculate the
    // primary beam. We want this on a fine grid in the
    // UV plane 
    Int directionIndex=directionIndex_p;
    AlwaysAssert(directionIndex>=0, AipsError);

    // Set up the convolution function.
    Int nx=nx_p;
    Int ny=ny_p;
    //    convSize_p=max(nx,ny)*convSampling;
    //cerr << "size " << nx << "  " << ny << endl;
    //3 times the support size
    if(doneMainConv_p.shape()[0] < (actualConvIndex_p+1)){
      // cerr << "resizing DONEMAIN " <<   doneMainConv_p.shape()[0] << endl;
    	doneMainConv_p.resize(actualConvIndex_p+1, True);
    	doneMainConv_p[actualConvIndex_p]=False;
    }

    if(!(doneMainConv_p[actualConvIndex_p])){

      //convSize_p=4*(sj_p->support(vb, coords));
      convSize_p=Int(max(nx_p, ny_p)*2.0)/2*convSampling;
      // Make this a nice composite number, to speed up FFTs
      CompositeNumber cn(uInt(convSize_p*2.0));    
      convSize_p  = cn.nextLargerEven(Int(convSize_p));
      //      cerr << "convSize : " << convSize_p << endl;

    }
    
   
    toPix(vb);
    //Timer tim;
    //tim.mark();
    addPBToFlux(vb);
    //tim.show("After addPBToFlux");
    DirectionCoordinate dc=dc_p;

    //where in the image in pixels is this pointing
    Vector<Double> pixFieldDir(2);
    pixFieldDir=thePix_p;

    //cerr << "pix of pointing " << pixFieldDir << endl;
    MDirection fieldDir=direction1_p;
    //shift from center
    pixFieldDir(0) = pixFieldDir(0) - Double(nx / 2);
    pixFieldDir(1) = pixFieldDir(1) - Double(ny / 2);

    //phase gradient per pixel to apply
    pixFieldDir(0) = -pixFieldDir(0)*2.0*C::pi/Double(nx)/Double(convSampling);
    pixFieldDir(1) = -pixFieldDir(1)*2.0*C::pi/Double(ny)/Double(convSampling);

    //cerr << "DonemainConv " << doneMainConv_p[actualConvIndex_p] << endl;
    if(!doneMainConv_p[actualConvIndex_p]){
      Vector<Double> sampling;
      sampling = dc.increment();
      sampling*=Double(convSampling);
      sampling(0)*=Double(nx)/Double(convSize_p);
      sampling(1)*=Double(ny)/Double(convSize_p);
      dc.setIncrement(sampling);
      
      
      Vector<Double> unitVec(2);
      unitVec=convSize_p/2;
      dc.setReferencePixel(unitVec);
      
      
      //make sure we are using the same units
      fieldDir.set(dc.worldAxisUnits()(0));
      
      dc.setReferenceValue(fieldDir.getAngle().getValue());
      
      coords.replaceCoordinate(dc, directionIndex);
      Int spind=coords.findCoordinate(Coordinate::SPECTRAL);
      SpectralCoordinate spCoord=coords.spectralCoordinate(spind);
      spCoord.setReferencePixel(Vector<Double>(1,0.0));
      spCoord.setReferenceValue(Vector<Double>(1, beamFreqs(0)));
      if(beamFreqs.nelements() >1)
	spCoord.setIncrement(Vector<Double>(1, beamFreqs(1)-beamFreqs(0)));
      coords.replaceCoordinate(spCoord, spind);
      //cerr << "BEAM freqs " << beamFreqs << endl;

      //  coords.list(logIO(), MDoppler::RADIO, IPosition(), IPosition());
      
      IPosition pbShape(4, convSize_p, convSize_p, 1, nBeamChans);
      Int memtobeused=0;
      Long memtot=HostInfo::memoryFree();
      //check for 32 bit OS and limit it to 2Gbyte
      if( sizeof(void*) == 4){
    	  if(memtot > 2000000)
    		  memtot=2000000;
      }
      if(memtot <= 2000000)
    	  memtobeused=0;
      //cerr << "mem to be used " << memtobeused << endl;
      //tim.mark();
      IPosition start(4, 0, 0, 0, 0);
      IPosition pbSlice(4, convSize_p, convSize_p, 1, 1);
      //cerr << "pbshape " << pbShape << endl;
      TempImage<Complex> twoDPB(TiledShape(pbShape), coords, memtobeused);

      //tim.show("after making one image");
      convFunc_p.resize(convSize_p, convSize_p);
      convFunc_p=0.0;
      
      

      // Accumulate terms 
      //Matrix<Complex> screen(convSize_p, convSize_p);
      //screen=1.0;
      // Either the SkyJones
      //tim.mark();
      //twoDPB.set(Complex(1.0,0.0));
      IPosition blcin(4, 0, 0, 0, 0);
      IPosition trcin(4, convSize_p-1, convSize_p-1, 0, 0);
      for (Int k=0; k < nBeamChans; ++k){
	blcin[3]=k;
	trcin[3]=k;
	Slicer slin(blcin, trcin, Slicer::endIsLast);
	SubImage<Complex> subim(twoDPB, slin, True);
	subim.set(Complex(1.0,0.0));
      //twoDPB.putSlice(screen, start);
	sj_p->apply(subim, subim, vb, 0); 
      }
      //tim.show("after an apply" );
      //*****Test
      TempImage<Complex> twoDPB2(TiledShape(pbShape), coords, memtobeused);
      //Old way
      
      {
    	  TempImage<Float> screen2(TiledShape(pbShape), coords, memtobeused);
    	  //	Matrix<Float> screenoo(convSize_p, convSize_p);
    	  //screenoo.set(1.0);
    	  //screen2.putSlice(screenoo,start);
    	  //screen2.set(1.0);
	  for (Int k=0; k < nBeamChans; ++k){
	    blcin[3]=k;
	    trcin[3]=k;
	    Slicer slin(blcin, trcin, Slicer::endIsLast);
	    SubImage<Float> subim(screen2, slin, True);
	    subim.set(1.0);
	    //twoDPB.putSlice(screen, start);
	    sj_p->applySquare(subim, subim, vb, 0); 
	  }
    	  //sj_p->applySquare(screen2, screen2, vb, 0);
    	  LatticeExpr<Complex> le(screen2);
    	  twoDPB2.copyData(le);
      }
      //tim.show("After applys ");
      
      
 
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
	os1 << "Screen_" << vb.fieldId() ;
	PagedImage<Float> thisScreen(pbShape, ftCoords, String(os1));
	LatticeExpr<Float> le(abs(twoDPB));
		thisScreen.copyData(le);
      }
      
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
    
      convFunc_p=twoDPB.getSlice(IPosition(4,0,0,0,0), IPosition(4, convSize_p, convSize_p, 1, 1), True);
      
      //convFunc/=max(abs(convFunc));
      Float maxAbsConvFunc=max(amplitude(convFunc_p));
      
      Float minAbsConvFunc=min(amplitude(convFunc_p));
      convSupport_p=-1;
      Bool found=False;
      //Bool found2=True;
      //Int trial2=0;
      Int trial=0;
      for (trial=convSize_p/2-2;trial>0;trial--) {
	//Searching down a diagonal
	if(abs(convFunc_p(convSize_p/2-trial,convSize_p/2-trial)) >  (1.0e-2*maxAbsConvFunc)) {
	  found=True;
	  trial=Int(sqrt(2.0*Float(trial*trial)));
	  break;
	}
      }
      if(!found){
	if((maxAbsConvFunc-minAbsConvFunc) > (1.0e-2*maxAbsConvFunc)) 
	  found=True;
	// if it drops by more than 2 magnitudes per pixel
	trial=( convSize_p > (10*convSampling)) ? 5*convSampling : (convSize_p/2 - 4*convSampling);
      }

      if(trial < 5*convSampling) 
	trial=( convSize_p > (10*convSampling)) ? 5*convSampling : (convSize_p/2 - 4*convSampling);
      
      if(found) {
	convSupport_p=Int(0.5+Float(trial)/Float(convSampling))+1;
      }
      else {
	os << "Convolution function is misbehaved - support seems to be zero\n"
	   << "Reasons can be: \n(1)The image definition not covering one or more of the pointings selected"
           << "(2) No unflagged data in a given pointing\n"
	   << "(3) The entries in the POINTING subtable do not match the field being imaged."
	   << "Please check, and try again with an empty POINTING subtable.)\n"
	   << LogIO::EXCEPTION;
      }

      // Normalize such that plane 0 sums to 1 (when jumping in
      // steps of convSampling)
      
      Double pbSum=0.0;
      
      

      for (Int iy=-convSupport_p;iy<=convSupport_p;iy++) {
	for (Int ix=-convSupport_p;ix<=convSupport_p;ix++) {
	  Complex val=convFunc_p(ix*convSampling+convSize_p/2,
				 iy*convSampling+convSize_p/2);
	  pbSum+=real(val);
	  //pbSum+=sqrt(real(val)*real(val)+ imag(val)*imag(val));
	}
      }
      
      //pbSum=sum(amplitude(convFunc_p))/Double(convSampling)/Double(convSampling);

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
      convFunctions_p[actualConvIndex_p]= new Array<Complex>();
      convWeights_p[actualConvIndex_p]= new Array<Complex>();
      Int newConvSize=2*(convSupport_p+2)*convSampling;
      //NEED to chop this right ...and in the centre
      if(newConvSize >=convSize_p)
    	  newConvSize=convSize_p;

      IPosition blc(4, (convSize_p/2)-(newConvSize/2),
    		  (convSize_p/2)-(newConvSize/2), 0, 0);
      IPosition trc(4, (convSize_p/2)+(newConvSize/2-1),
		      (convSize_p/2)+(newConvSize/2-1), 0, nBeamChans-1);
      convFunctions_p[actualConvIndex_p]->resize(IPosition(5, newConvSize, newConvSize, 1, nBeamChans,1));
      //cerr << "convFunc shape " << (convFunctions_p[actualConvIndex_p])->shape() << 
      //"  " << " twoDPB shape " <<twoDPB.get(False)(blc,trc).shape() << endl;
      convFunctions_p[actualConvIndex_p]->copyMatchingPart(twoDPB.get(False)(blc,trc)*Complex(1.0/pbSum,0.0));
      convSize_p=newConvSize;
      convWeights_p[actualConvIndex_p]->resize(IPosition(5, newConvSize, newConvSize, 1, nBeamChans,1));
      convWeights_p[actualConvIndex_p]->copyMatchingPart(twoDPB2.get(False)(blc,trc)*Complex(1.0/pbSum,0.0));
	
      convFunc_p.resize();//break any reference
      (*convSizes_p[actualConvIndex_p])[0]=convSize_p;
      doneMainConv_p[actualConvIndex_p]=True;
      
    }

    //Apply the shift phase gradient
    convFunc.resize();
    weightConvFunc.resize();
    convFunc.assign(*(convFunctions_p[actualConvIndex_p]));
    weightConvFunc.assign(*(convWeights_p[actualConvIndex_p]));
    Bool copyconv, copywgt;
    Complex *cv=convFunc.getStorage(copyconv);
    Complex *wcv=weightConvFunc.getStorage(copywgt);
    //cerr << "Field " << vb.fieldId() << " spw " << vb.spectralWindow() << " phase grad: " << pixFieldDir << endl;
   
    for (Int nc=0; nc < nBeamChans; ++nc){ 
    	Int planeoffset=nc*convSize_p*convSize_p;
    	for (Int iy=0;iy<convSize_p;iy++) {
    		Double cy, sy;
		Int offset;
	       
    		SINCOS(Double(iy-convSize_p/2)*pixFieldDir(1), sy, cy);
    		Complex phy(cy,sy) ;
    		offset = iy*convSize_p+planeoffset;
    		for (Int ix=0;ix<convSize_p;ix++) {
    			Double cx, sx;
    			SINCOS(Double(ix-convSize_p/2)*pixFieldDir(0), sx, cx);
    			Complex phx(cx,sx) ;
			cv[ix+offset]= cv[ix+offset]*phx*phy;
			wcv[ix+offset]= wcv[ix+offset]*phx*phy;

    		}
    	}
    }
    convFunc.putStorage(cv, copyconv);
    weightConvFunc.putStorage(wcv, copywgt);
    convsize.resize();
    convsize=*(convSizes_p[actualConvIndex_p]);
    convSupport.resize();
    convSupport=(*(convSupportBlock_p[actualConvIndex_p]));
    
    
  }

  void SimplePBConvFunc::setSkyJones(SkyJones* sj){
    sj_p=sj;
  }

  void SimplePBConvFunc::findUsefulChannels(Vector<Int>& chanMap, Vector<Double>& chanFreqs,  const VisBuffer& vb, const Vector<Double>& freq){
    chanMap.resize(freq.nelements());
    Vector<Double> localfreq=vb.frequency();
    Double minfreq=min(freq);
    
    Double origwidth=freq.nelements()==1 ? 1e12 : (max(freq)-min(freq))/(freq.nelements()-1);
    ///Fractional bandwidth which will trigger mutiple PB in one spw
    Double tol=(max(freq))*0.5/100;
    
    Int nchan=Int(lround((max(freq)-min(freq))/tol));
   
    //cerr  << "TOLERA " << tol << " nchan " << nchan << " vb.nchan " << vb.nChannel() << endl;
    //Number of beams that matters are the ones in the data
    if(nchan > vb.nChannel())
      nchan=vb.nChannel();

    if(tol < origwidth) tol=origwidth;
    chanFreqs.resize();
    if(nchan >= (Int)(freq.nelements()-1)) { indgen(chanMap); chanFreqs=freq; return;}
    if((nchan==0) || (freq.nelements()==1)) { chanFreqs=Vector<Double>(1, freq[0]);chanMap.set(0); return;}

    //readjust the tolerance...
    tol=(max(freq)-min(freq)+origwidth)/Double(nchan);
    chanFreqs.resize(nchan);
    for (Int k=0; k < nchan; ++k)
      chanFreqs[k]=minfreq-origwidth+tol/2.0+tol*Double(k);
    Int activechan=0;
    chanMap.set(-1);
    for (uInt k=0; k < chanMap.nelements(); ++k){
     
      while((activechan< nchan) && Float(fabs(freq[k]-chanFreqs[activechan])) > Float(tol/2.0)){
	//		cerr << "k " << k << " atcivechan " << activechan << " comparison " 
	//     << freq[k] << "    " << chanFreqs[activechan]  << endl;	
	++activechan;
      }
      if(activechan != nchan)
	chanMap[k]=activechan;
      //////////////////
      if(chanMap[k] < 0)
	cerr << "freq diffs " << freq[k]-chanFreqs << "  TOL " << tol/2.0 << endl;

      ///////////////////////////
      activechan=0;
    }

    return;
  }


  Bool SimplePBConvFunc::checkPBOfField(const VisBuffer& vb){
    //Int fieldid=vb.fieldId();
    String msid=vb.msName(True);
    /*
     if(convFunctionMap_p.ndefined() > 0){
      if (((fluxScale_p.shape()[3] != nchan_p) || (fluxScale_p.shape()[2] != npol_p)) && calcFluxScale_p){
	convFunctionMap_p.clear();
      }
    }
    // if you rename the ms might be a problem
    String mapid=msid+String("_")+String::toString(fieldid);
    if(convFunctionMap_p.ndefined() == 0){
      convFunctionMap_p.define(mapid, 0);    
      actualConvIndex_p=0;
      if(calcFluxScale_p){
	// 0ne channel only is needed to keep track of pb coverage
	if(fluxScale_p.shape().nelements()==0){
	  fluxScale_p=TempImage<Float>(IPosition(4,nx_p,ny_p,npol_p,1), csys_p);
	  fluxScale_p.set(0.0);
	}
	filledFluxScale_p=False;
      }
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
      //convFunc_p.reference(convFunctions_p[actualConvIndex_p]->xyPlane(0));
      //weightConvFunc_p.reference(convWeights_p[actualConvIndex_p]->xyPlane(0));
      //Again this for one time of antenna only later should be fixed for all 
      // antennas independently
      convSupport_p=(*convSupportBlock_p[actualConvIndex_p])[0];
      convSize_p=(*convSizes_p[actualConvIndex_p])[0];

  }
*/
 
 return True;



  }

  ImageInterface<Float>&  SimplePBConvFunc::getFluxScaleImage(){

    if(!calcFluxScale_p)
      throw(AipsError("Programmer error: Cannot get flux scale"));
    if(!filledFluxScale_p){
      IPosition blc=fluxScale_p.shape();
      IPosition trc=fluxScale_p.shape();
      blc(0)=0; blc(1)=0; trc(0)=nx_p-1; trc(1)=ny_p-1;
      
      for (Int j=0; j < fluxScale_p.shape()(2); ++j){
	for (Int k=0; k < fluxScale_p.shape()(3) ; ++k){
	  
	  blc(2)=j; trc(2)=j;
	  blc(3)=k; trc(3)=k;
	  Slicer sl(blc, trc, Slicer::endIsLast);
	  SubImage<Float> fscalesub(fluxScale_p, sl, True);
	  Float planeMax;
	  LatticeExprNode LEN = max( fscalesub );
	  planeMax =  LEN.getFloat();
	  if(planeMax !=0){
	    fscalesub.copyData( (LatticeExpr<Float>) (fscalesub/planeMax));
	    
	  }
	}
      }
      /*
      if(0) {
	ostringstream os2;
	os2 << "ALL_" << "BEAMS" ;
	PagedImage<Float> thisScreen2(fluxScale_p.shape(), fluxScale_p.coordinates(), String(os2));
	thisScreen2.copyData(fluxScale_p);
      }
      */

      filledFluxScale_p=True;
    }
      

    return fluxScale_p;
    
  }


  Bool SimplePBConvFunc::toRecord(RecordInterface& rec){
    Int numConv=convFunctions_p.nelements();
    // not saving the protected variables as they are generated by
    // the first  call to storeImageParams 
    try{
      rec.define("name", "SimplePBConvFunc");
      rec.define("numconv", numConv);
      //cerr << "num of conv " << numConv << "  " << convFunctionMap_p.ndefined() << "  " <<convFunctions_p.nelements() << endl;
      std::map<String, Int>::iterator it=vbConvIndex_p.begin();
      for (Int k=0; k < numConv; ++k){
	rec.define("convfunctions"+String::toString(k), *(convFunctions_p[k]));
	rec.define("convweights"+String::toString(k), *(convWeights_p[k]));
	rec.define("convsizes"+String::toString(k), *(convSizes_p[k]));
	rec.define("convsupportblock"+String::toString(k), *(convSupportBlock_p[k]));
	//cerr << "k " << k << " key " << convFunctionMap_p.getKey(k) << " val " << convFunctionMap_p.getVal(k) << endl;
	rec.define(String("key")+String::toString(k), it->first);
	rec.define(String("val")+String::toString(k), it->second);
	it++;
      }
      rec.define("pbclass", Int(pbClass_p));
      rec.define("actualconvindex",  actualConvIndex_p);
      rec.define("donemainconv", doneMainConv_p);
      //The following is not needed ..can be regenerated
      //rec.define("pointingpix", pointingPix_p);
    }
    catch(AipsError x) {
      return False;
    }
    return True;
  }

  Bool SimplePBConvFunc::fromRecord(String& err, const RecordInterface& rec, Bool calcFluxneeded){
     Int numConv=0;
     //make sure storeImageParams is triggered
     nchan_p=0;
     
     try{
       if(!rec.isDefined("name") || rec.asString("name") != "SimplePBConvFunc"){
	 throw(AipsError("Wrong record to recover HetArray from"));
	}
       rec.get("numconv", numConv);
       convFunctions_p.resize(numConv, True, False);
       convSupportBlock_p.resize(numConv, True, False);
       convWeights_p.resize(numConv, True, False);
       convSizes_p.resize(numConv, True, False);
       convFunctionMap_p=SimpleOrderedMap<String, Int>(-1);
       vbConvIndex_p.erase(vbConvIndex_p.begin(), vbConvIndex_p.end());
       for (Int k=0; k < numConv; ++k){
	 convFunctions_p[k]=new Array<Complex>();
	 convWeights_p[k]=new Array<Complex>();
	 convSizes_p[k]=new Vector<Int>();
	 convSupportBlock_p[k]=new Vector<Int>();
	 rec.get("convfunctions"+String::toString(k), *(convFunctions_p[k]));
	 rec.get("convsupportblock"+String::toString(k), *(convSupportBlock_p[k]));
	 rec.get("convweights"+String::toString(k), *(convWeights_p[k]));
	 rec.get("convsizes"+String::toString(k), *(convSizes_p[k]));
	 String key;
	 Int val;
	 rec.get(String("key")+String::toString(k), key);
	 rec.get(String("val")+String::toString(k), val);
	 vbConvIndex_p[key]=val;
	 //convFunctionMap_p.define(key,val);
       }
       pbClass_p=static_cast<PBMathInterface::PBClass>(rec.asInt("pbclass"));
       rec.get("actualconvindex",  actualConvIndex_p);
       pointingPix_p.resize();
       //rec.get("pointingpix", pointingPix_p);
       calcFluxScale_p=calcFluxneeded;

     }
     catch(AipsError x) {
       err=x.getMesg();
       return False;
     }
     return True;
     
  }
  void SimplePBConvFunc::addPBToFlux(const VisBuffer& vb){
    if(calcFluxScale_p){
      Vector<Int> pixdepoint(2, -100000);
      convertArray(pixdepoint, thePix_p);
      if((pixdepoint(0) >=0) && (pixdepoint(0) < pointingPix_p.shape()[0]) && (pixdepoint(1) >=0) && (pixdepoint(1) < pointingPix_p.shape()[1])  && !pointingPix_p(pixdepoint(0), pixdepoint(1))){
	 TempImage<Float> thispb(fluxScale_p.shape(), fluxScale_p.coordinates());
	 thispb.set(1.0);
	 sj_p->applySquare(thispb, thispb, vb, 0);
	 LatticeExpr<Float> le(fluxScale_p+thispb);
	 fluxScale_p.copyData(le);
	 pointingPix_p(pixdepoint(0), pixdepoint(1))=True;
	 //LatticeExprNode LEN = max(fluxScale_p);
	 //Float maxsca=LEN.getFloat();
	 //Tempporary fix when cubesky is chunking...do not add on 
	 //already defined position
	 //if(maxsca > 1.98){
	 //  cerr << "avoiding subtract " << endl;
	//fluxScale_p.copyData(LatticeExpr<Float>(fluxScale_p-thispb));

	 //}      
      /*
      if(0) {
	ostringstream os1;
	os1 << "SINGLE_" << vb.fieldId() ;
	PagedImage<Float> thisScreen(fluxScale_p.shape(), fluxScale_p.coordinates(), String(os1));
	thisScreen.copyData(thispb);
	ostringstream os2;
	os2 << "ALL_" << vb.fieldId() ;
	PagedImage<Float> thisScreen2(fluxScale_p.shape(), fluxScale_p.coordinates(), String(os2));
	thisScreen2.copyData(fluxScale_p);
      }
      */
       }
    }

  }

  void SimplePBConvFunc::sliceFluxScale(Int npol) {
     IPosition fshp=fluxScale_p.shape();
     if (fshp(2)>npol){
       npol_p=npol;
       // use first npol planes...
       IPosition blc(4,0,0,0,0);
       IPosition trc(4,fluxScale_p.shape()(0)-1, fluxScale_p.shape()(1)-1,npol-1,fluxScale_p.shape()(3)-1);
       Slicer sl=Slicer(blc, trc, Slicer::endIsLast);
       //writeable if possible
       SubImage<Float> fluxScaleSub = SubImage<Float> (fluxScale_p, sl, True);
       fluxScale_p = TempImage<Float>(fluxScaleSub.shape(),fluxScaleSub.coordinates());
       LatticeExpr<Float> le(fluxScaleSub);
       fluxScale_p.copyData(le);
     }
  }

} //# NAMESPACE CASA - END







