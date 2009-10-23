//# HetArrayConvFunc.cc: Implementation for HetArrayConvFunc
//# Copyright (C) 2008
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
#include <casa/Containers/SimOrdMap.h>
#include <scimath/Mathematics/MathFunc.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/CompositeNumber.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
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

#include <ms/MeasurementSets/MSColumns.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>

#include <synthesis/MeasurementComponents/PBMath1DAiry.h>
#include <synthesis/MeasurementComponents/PBMath1DNumeric.h>
#include <synthesis/MeasurementComponents/HetArrayConvFunc.h>
namespace casa { //# NAMESPACE CASA - BEGIN

  HetArrayConvFunc::HetArrayConvFunc() : convFunctionMap_p(-1), antDiam2IndexMap_p(-1),msId_p(-1), actualConvIndex_p(-1)
  {
    
    init(PBMathInterface::AIRY);
  }

  HetArrayConvFunc::HetArrayConvFunc(const PBMathInterface::PBClass typeToUse):
    convFunctionMap_p(-1), antDiam2IndexMap_p(-1),msId_p(-1), actualConvIndex_p(-1) 
  {
    
    init(typeToUse);

  }

  HetArrayConvFunc::~HetArrayConvFunc(){
    //
  }

  void HetArrayConvFunc::init(const PBMathInterface::PBClass typeTouse){
    doneMainConv_p=False;
    filledFluxScale_p=False;
    pbClass_p=typeTouse;
  }

  

  void HetArrayConvFunc::findAntennaSizes(const VisBuffer& vb){

    if(msId_p != vb.msId()){
      msId_p=vb.msId();
      const ROMSAntennaColumns& ac=vb.msColumns().antenna();
      antIndexToDiamIndex_p.resize(ac.nrow());
      antIndexToDiamIndex_p.set(-1);
      Int diamIndex=antDiam2IndexMap_p.ndefined();
      Vector<Double> dishDiam=ac.dishDiameter().getColumn();
      for (uInt k=0; k < dishDiam.nelements(); ++k){
	if((diamIndex !=0) && antDiam2IndexMap_p.isDefined(dishDiam(k))){
	  antIndexToDiamIndex_p(k)=antDiam2IndexMap_p(dishDiam(k));
	}
	else{
	  if(dishDiam[k] > 0.0){ //there may be stations with no dish on
	    antDiam2IndexMap_p.define(dishDiam(k), diamIndex);
	    antIndexToDiamIndex_p(k)=diamIndex;
	    antMath_p.resize(diamIndex+1);
	    if(pbClass_p== PBMathInterface::AIRY){
	      Quantity qdiam(dishDiam(k),"m");
	      
	      //VLA ratio of blockage to dish
	      Quantity blockDiam(dishDiam(k)/25.0*2.0, "m");	      
	      antMath_p[diamIndex]=new PBMath1DAiry(qdiam, blockDiam,  
						    Quantity(150,"arcsec"), 
						    Quantity(100.0,"GHz"));
	      
	      
	    }
	    else{

	      cout<< "Don't deal with non airy dishes yet " << endl;
	    }
	    ++diamIndex;
	  } 
	}

      }


    }
    
    
    


  }

  void HetArrayConvFunc::findConvFunction(const ImageInterface<Complex>& iimage, 
					  const VisBuffer& vb,
					  const Int& convSampling,
					  Cube<Complex>& convFunc, 
					  Cube<Complex>& weightConvFunc, 
					  Vector<Int>& convsize,
					  Vector<Int>& convSupport,
					  Vector<Int>& rowMap)
  {

    storeImageParams(iimage);
    findAntennaSizes(vb);
    uInt ndish=antMath_p.nelements();
    if(ndish==0)
      throw(AipsError("Don't have dishsize"));
    Int ndishpair;
    if(ndish==1)
      ndishpair=1;
    else
      ndishpair=factorial(ndish)/factorial(ndish-2)/2 + ndish;
    
    convFunc.resize();
    weightConvFunc.resize();
    rowMap.resize();
    convsize.resize();
    convSupport.resize();

    if(checkPBOfField(vb, rowMap) && (rowMap.shape()[0]==vb.nRow())){
      convFunc.reference(convFunc_p);
      weightConvFunc.reference(weightConvFunc_p);
      convsize=*convSizes_p[actualConvIndex_p];
      convSupport=convSupport_p;
      return;
    }
    // Get the coordinate system
    CoordinateSystem coords(iimage.coordinates());
    Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
    AlwaysAssert(directionIndex>=0, AipsError);
    // Set up the convolution function.
    Int nx=nx_p;
    Int ny=ny_p;
    Int support=0;
    if(!doneMainConv_p){
      for (uInt ii=0; ii < ndish; ++ii){
	support=max((antMath_p[ii])->support(coords), support);
      }
      convSize_p=support*convSampling;
      CompositeNumber cn(convSize_p);
      convSize_p=cn.nearestEven(convSize_p);
    }
    
    MDirection fieldDir=vb.direction1()(0);
    
    DirectionCoordinate dc=coords.directionCoordinate(directionIndex);
    //where in the image in pixels is this pointing
    Vector<Double> pixFieldDir(2);
    dc.toPixel(pixFieldDir, fieldDir);
    //shift from center
    pixFieldDir(0)=pixFieldDir(0)- Double(nx / 2);
    pixFieldDir(1)=pixFieldDir(1)- Double(ny / 2);
    //phase gradient per pixel to apply
    pixFieldDir(0)=-pixFieldDir(0)*2.0*C::pi/Double(nx)/Double(convSampling);
    pixFieldDir(1)=-pixFieldDir(1)*2.0*C::pi/Double(ny)/Double(convSampling);


    if(!doneMainConv_p){
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
    

      IPosition pbShape(4, convSize_p, convSize_p, 1, 1);
      TempImage<Complex> twoDPB(pbShape, coords);    
    
      convFunc_p.resize(convSize_p, convSize_p, ndishpair);
      convFunc_p=0.0;
      weightConvFunc_p.resize(convSize_p, convSize_p, ndishpair);
      weightConvFunc_p=0.0;
    
  
      TempImage<Complex> pBScreen(pbShape, coords);
      TempImage<Complex> pB2Screen(pbShape, coords);
      IPosition start(4, 0, 0, 0, 0);
      convSupport_p.resize(ndishpair);
      for (uInt k=0; k < ndish; ++k){
      
	for (uInt j =k ; j < ndish; ++j){
	
	  Matrix<Complex> screen(convSize_p, convSize_p);
	  screen=1.0;
	  pBScreen.putSlice(screen, start);
	  //one antenna 
	  (antMath_p[k])->applyVP(pBScreen, pBScreen, vb.direction1()(0));
	  //Then the other
	  (antMath_p[j])->applyVP(pBScreen, pBScreen, vb.direction2()(0));
	  //*****************
	  //if(0){
	  //  ostringstream os1;
	  //  os1 << "PB_field_" << vb.fieldId() << "_antpair_" << k <<"_"<<j ;
	  //  PagedImage<Float> thisScreen(pbShape, coords, String(os1));
	  //  LatticeExpr<Float> le(abs(pBScreen));
	  //  thisScreen.copyData(le);
	  //
	  //	}
	  //*****************
	  Matrix<Complex> screenoo(convSize_p, convSize_p);
	  screenoo.set(1.0);
	  pB2Screen.putSlice(screenoo, start);
	//one antenna 
	  (antMath_p[k])->applyPB(pB2Screen, pB2Screen, vb.direction1()(0));
	  //Then the other
	  (antMath_p[j])->applyPB(pB2Screen, pB2Screen, vb.direction2()(0));
	  

	  pBScreen.copyData((LatticeExpr<Complex>) (iif(abs(pBScreen)> 5e-2, pBScreen, 0)));
	  pB2Screen.copyData((LatticeExpr<Complex>) (iif(abs(pB2Screen)> 25e-4, pB2Screen, 0)));

	  LatticeFFT::cfft2d(pBScreen);
	  LatticeFFT::cfft2d(pB2Screen);
	

	  Int plane=0;
	  for (uInt jj=0; jj < k; ++jj)
	    plane=plane+ndish-jj-1;
	  plane=plane+j;

	  convFunc_p.xyPlane(plane)=pBScreen.get(True);
	  weightConvFunc_p.xyPlane(plane)=pB2Screen.get(True);
	  supportAndNormalize(plane, convSampling);
	  
	}
	
      }


      doneMainConv_p=True;
      convSave_p.resize();
      weightSave_p.resize();
      convSave_p=convFunc_p;
      weightSave_p=weightConvFunc_p;
      Int newConvSize=2*(max(convSupport_p)+2)*convSampling;
      if(newConvSize < convSize_p){
	IPosition blc(3, (convSize_p/2)-(newConvSize/2),
		      (convSize_p/2)-(newConvSize/2),0);
	IPosition trc(3, (convSize_p/2)+(newConvSize/2-1),
		      (convSize_p/2)+(newConvSize/2-1), ndishpair-1);
	convSave_p.resize(newConvSize, newConvSize, ndishpair);
	convSave_p=convFunc_p(blc,trc);
	convSize_p=newConvSize;
	weightSave_p.resize(newConvSize, newConvSize, ndishpair);
	weightSave_p=weightConvFunc_p(blc,trc);
      }

    }
    /*
    rowMap.resize(vb.nRow());
    for (Int k=0; k < vb.nRow(); ++k){
      //plane of convfunc that match this pair of antennas
      rowMap(k)=antIndexToDiamIndex_p(vb.antenna1()(k))*ndish+
	antIndexToDiamIndex_p(vb.antenna2()(k));

    }
    */
    makerowmap(vb, rowMap);

    convFunctions_p.resize(actualConvIndex_p+1);
    convWeights_p.resize(actualConvIndex_p+1);
    convFunctions_p[actualConvIndex_p]= new Cube<Complex>();
    convWeights_p[actualConvIndex_p]= new Cube<Complex>();
    convSupportBlock_p.resize(actualConvIndex_p+1);
    convSizes_p.resize(actualConvIndex_p+1);
    convSupportBlock_p[actualConvIndex_p]=new Vector<Int>(ndishpair);
    (*convSupportBlock_p[actualConvIndex_p])=convSupport_p;
    convSizes_p[actualConvIndex_p]=new Vector<Int> (ndishpair);
    
    convFunctions_p[actualConvIndex_p]->resize(convSize_p, convSize_p, ndishpair);
    *(convFunctions_p[actualConvIndex_p])=convSave_p; 
    convWeights_p[actualConvIndex_p]->resize(convSize_p, convSize_p, ndishpair);
    *(convWeights_p[actualConvIndex_p])=weightSave_p;


    //Apply the shift phase gradient

    for (Int iy=0;iy<convSize_p;iy++) { 
      Complex phy(cos(Double(iy-convSize_p/2)*pixFieldDir(1)),sin(Double(iy-convSize_p/2)*pixFieldDir(1))) ;
      for (Int ix=0;ix<convSize_p;ix++) {
	Complex phx(cos(Double(ix-convSize_p/2)*pixFieldDir(0)),sin(Double(ix-convSize_p/2)*pixFieldDir(0))) ;
	for(Int iz=0; iz <ndishpair; ++iz){
	  (*(convFunctions_p[actualConvIndex_p]))(ix,iy,iz)= (*(convFunctions_p[actualConvIndex_p]))(ix,iy,iz)*phx*phy;
	  (*(convWeights_p[actualConvIndex_p]))(ix,iy,iz)= (*(convWeights_p[actualConvIndex_p]))(ix,iy,iz)*phx*phy;
	}
      }
    }
    
    //For now all have the same size convsize;
    convSizes_p[actualConvIndex_p]->set(convSize_p);
    
    //We have to get the references right now
    convFunc_p.resize();
    convFunc_p.reference(*convFunctions_p[actualConvIndex_p]);
    weightConvFunc_p.resize();
    weightConvFunc_p.reference(*convWeights_p[actualConvIndex_p]);

    convFunc.reference(convFunc_p);
    weightConvFunc.reference(weightConvFunc_p);
    convsize=*convSizes_p[actualConvIndex_p];
    convSupport=convSupport_p;


  }


  void HetArrayConvFunc::supportAndNormalize(Int plane, Int convSampling){

    LogIO os;
    os << LogOrigin("HetArrConvFunc", "suppAndNorm")  << LogIO::NORMAL;
    // Locate support
	Int convSupport=-1;
	Float maxAbsConvFunc=max(amplitude(convFunc_p.xyPlane(plane)));
    
	Float minAbsConvFunc=min(amplitude(convFunc_p.xyPlane(plane)));
	Bool found=False;
	Int trial=0;
	for (trial=convSize_p/2-2;trial>0;trial--) {
	  //Searching down a diagonal
	  if(abs(convFunc_p.xyPlane(plane)(convSize_p/2-trial,convSize_p/2-trial)) >  (1.0e-2*maxAbsConvFunc)) {
	    found=True;
	    trial=Int(sqrt(2.0*Float(trial*trial)));
	    break;
	  }
	}
	if(!found){
	  if((maxAbsConvFunc-minAbsConvFunc) > (1.0e-2*maxAbsConvFunc)) 
	  found=True;
	  // if it drops by more than 2 magnitudes per pixel
	  trial=3;
	}
				 
	if(found) {
	  convSupport=Int(0.5+Float(trial)/Float(convSampling))+1;
	  //support is really over the edge
	  if( (convSupport*convSampling) >= convSize_p/2){
	    convSupport=convSize_p/2/convSampling-1;
	  }
	}
	else {
	  os << "Convolution function is misbehaved - support seems to be zero\n"
	     << "Reasons can be: \nThe image definition not covering one or more of the pointings selected \n"
         << "Or no unflagged data in a given pointing"
	     
	     << LogIO::EXCEPTION;
	}
	convSupport_p(plane)=convSupport;
	Double pbSum=0.0;
	/*
	Double pbSum1=0.0;
	
	for (Int iy=-convSupport;iy<=convSupport;iy++) {
	  for (Int ix=-convSupport;ix<=convSupport;ix++) {
	    Complex val=convFunc_p.xyPlane(plane)(ix*convSampling+convSize_p/2,
						  iy*convSampling+convSize_p/2);
	
	    pbSum1+=sqrt(real(val)*real(val)+ imag(val)*imag(val));
	  }
	}
    
	*/
	IPosition blc(2, -convSupport*convSampling+convSize_p/2, -convSupport*convSampling+convSize_p/2);
	IPosition trc(2, convSupport*convSampling+convSize_p/2, convSupport*convSampling+convSize_p/2);
	
	pbSum=real(sum(convFunc_p.xyPlane(plane)(blc,trc)));
	if(pbSum>0.0) {
	  (convFunc_p.xyPlane(plane))=convFunc_p.xyPlane(plane)*Complex(1.0/pbSum,0.0);
	  (weightConvFunc_p.xyPlane(plane)) =(weightConvFunc_p.xyPlane(plane))*Complex(1.0/pbSum,0.0);
	}
	else {
	  os << "Convolution function integral is not positive"
	     << LogIO::EXCEPTION;
	}



  }

  Int HetArrayConvFunc::factorial(Int n){
    Int fact=1;
    for (Int k=1; k<=n; ++k)
      fact *=k;
    return fact;
  }


  Bool HetArrayConvFunc::checkPBOfField(const VisBuffer& vb, 
					Vector<Int>& rowMap){
    
    Int fieldid=vb.fieldId();
    Int msid=vb.msId();
    String mapid=String::toString(msid)+String("_")+String::toString(fieldid);
    if(convFunctionMap_p.ndefined() == 0){
      convFunctionMap_p.define(mapid, 0);    
      actualConvIndex_p=0;
      fluxScale_p=TempImage<Float>(IPosition(4,nx_p,ny_p,npol_p,nchan_p), csys_p);
      filledFluxScale_p=False;
      fluxScale_p.set(0.0);
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
      convSupport_p.resize();
      //Here we will need to use the right xyPlane for different PA range 
      //and frequency may be 
      convFunc_p.reference(*convFunctions_p[actualConvIndex_p]);
      weightConvFunc_p.reference(*convWeights_p[actualConvIndex_p]);
      //Again this for one time of antenna only later should be fixed for all 
      // antennas independently
      //these are not really needed right now
      convSupport_p=(*convSupportBlock_p[actualConvIndex_p]);
      convSize_p=(*convSizes_p[actualConvIndex_p])[0];
      makerowmap(vb, rowMap);
    }

    return True;
  

  }

  void HetArrayConvFunc::makerowmap(const VisBuffer& vb, 
				    Vector<Int>& rowMap){

    uInt ndish=antMath_p.nelements();
    rowMap.resize(vb.nRow());
    for (Int k=0; k < vb.nRow(); ++k){
      Int index1=antIndexToDiamIndex_p(vb.antenna1()(k));
      Int index2=antIndexToDiamIndex_p(vb.antenna2()(k));
      if(index2 < index1){
	index1=index2;
	index2=antIndexToDiamIndex_p(vb.antenna1()(k));
      }
      Int plane=0;
      for (Int jj=0; jj < index1; ++jj)
	plane=plane+ndish-jj-1;
      plane=plane+index2;
      //plane of convfunc that match this pair of antennas
      rowMap(k)=plane;

    }

  }

  ImageInterface<Float>&  HetArrayConvFunc::getFluxScaleImage(){
    if(!filledFluxScale_p){ 
      //The best flux image for a heterogenous array is the weighted coverage
      fluxScale_p.copyData(*(convWeightImage_p));
      IPosition blc(4,nx_p, ny_p, npol_p, nchan_p);
      IPosition trc(4, ny_p, ny_p, npol_p, nchan_p);
      blc(0)=0; blc(1)=0; trc(0)=nx_p-1; trc(1)=ny_p-1;
      
      for (Int j=0; j < npol_p; ++j){
	for (Int k=0; k < nchan_p ; ++k){
	  
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
      filledFluxScale_p=True;  
    }
    

    return fluxScale_p;

  }


} //# NAMESPACE CASA - END
