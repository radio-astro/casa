//# HetArrayConvFunc.cc: Implementation for HetArrayConvFunc
//# Copyright (C) 2008-2013
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
#include <scimath/Mathematics/FFTServer.h>
#include <measures/Measures/MeasTable.h>
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
#include <lattices/Lattices/LatticeConcat.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>


#include <ms/MeasurementSets/MSColumns.h>

#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisibilityIterator.h>

#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/PBMath1DAiry.h>
#include <synthesis/TransformMachines/PBMath1DNumeric.h>
#include <synthesis/TransformMachines/PBMath2DImage.h>
#include <synthesis/TransformMachines/HetArrayConvFunc.h>
#include <synthesis/MeasurementEquations/VPManager.h>

#include <casa/OS/Timer.h>



namespace casa { //# NAMESPACE CASA - BEGIN

  HetArrayConvFunc::HetArrayConvFunc() : convFunctionMap_p(0), nDefined_p(0), antDiam2IndexMap_p(-1),msId_p(-1), actualConvIndex_p(-1)
  {
    calcFluxScale_p=True;
    init(PBMathInterface::AIRY);
  }

  HetArrayConvFunc::HetArrayConvFunc(const PBMathInterface::PBClass typeToUse, const String vpTable):
    convFunctionMap_p(0), nDefined_p(0), antDiam2IndexMap_p(-1),msId_p(-1), actualConvIndex_p(-1), vpTable_p(vpTable)
  {
    calcFluxScale_p=True;
    init(typeToUse);

  }

  HetArrayConvFunc::HetArrayConvFunc(const RecordInterface& rec, Bool calcFluxneeded):convFunctionMap_p(0), nDefined_p(0), antDiam2IndexMap_p(-1),msId_p(-1), actualConvIndex_p(-1) {
    String err;
    fromRecord(err, rec, calcFluxneeded);
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
      Vector<String>dishName=ac.name().getColumn();
      if(vpTable_p == ""){
      ////////We'll be using dish diameter as key
      for (uInt k=0; k < dishDiam.nelements(); ++k){
	if((diamIndex !=0) && antDiam2IndexMap_p.isDefined(String::toString(dishDiam(k)))){
	    antIndexToDiamIndex_p(k)=antDiam2IndexMap_p(String::toString(dishDiam(k)));
	}
	else{
	  if(dishDiam[k] > 0.0){ //there may be stations with no dish on
	    antDiam2IndexMap_p.define(String::toString(dishDiam(k)), diamIndex);
	    antIndexToDiamIndex_p(k)=diamIndex;
	    antMath_p.resize(diamIndex+1);
	    if(pbClass_p== PBMathInterface::AIRY){
	      Quantity qdiam(10.7, "m");
	      Quantity blockDiam(0.75, "m");
	      ///For ALMA 12m dish it is effectively 10.7 m according to Todd Hunter
	      ///@ 2011-12-06
	      if(!((vb.msColumns().observation().telescopeName()(0) =="ALMA") 
		   && (abs(dishDiam[k] - 12.0) < 0.5))){
		qdiam= Quantity (dishDiam(k),"m");	
		//VLA ratio of blockage to dish
		blockDiam= Quantity(dishDiam(k)/25.0*2.0, "m");
	      }	      
	      antMath_p[diamIndex]=new PBMath1DAiry(qdiam, blockDiam,  
						    Quantity(150,"arcsec"), 
						    Quantity(100.0,"GHz"));
	      
	      
	    }
	    else if(pbClass_p== PBMathInterface::IMAGE){
	      //Get the image name by calling code for the antenna name and array name
	      //For now hard wired to ALMA as this part of the code will not be accessed for non-ALMA 
	      //see Imager::setMosaicFTMachine 
	      // When ready to generalize then code that calls with telescope name, antenna name 
	      //(via vb.msColumns) and/or diameter and frequency via vb.frequency (indexing will need to 
	      //be upgraded to account for frequency too) should be done to return the
	      //right voltage pattern image. 
	      String vpImageName="";
	      if (abs(dishDiam[k]-7.0) < 1.0) 
		Aipsrc::find(vpImageName, "alma.vp.7m", "");
	      else
		Aipsrc::find(vpImageName, "alma.vp.12m", "") ;
	      //cerr << "first vpImagename " << vpImageName  << endl;
	      if(vpImageName==""){
		String beamPath; 
		if(!MeasTable::AntennaResponsesPath(beamPath, "ALMA")){
		  throw(AipsError("Alma beam images requested cannot be found ")); 		  
		}
		else{
		  beamPath=beamPath.before(String("AntennaResponses"));	
		  vpImageName= (abs(dishDiam[k]-7.0) < 1.0) ? beamPath
		    +String("/ALMA_AIRY_7M.VP") : 
		    beamPath+String("/ALMA_AIRY_12M.VP");
		}
		

	      }
	      //cerr << "Using the image VPs " << vpImageName << endl; 
	      if(Table::isReadable(vpImageName))
		antMath_p[diamIndex]=new PBMath2DImage(PagedImage<Complex>(vpImageName)); 
	      else
		throw(AipsError(String("Cannot find voltage pattern image ") + vpImageName));
	    }
	    else{

	      throw(AipsError("Do not  deal with non airy dishes or images of VP yet "));
	    }
	    ++diamIndex;
	  } 
	}

      }


      }
      else{// vptable specified 

	VPManager *vpman=VPManager::Instance();
	vpman->loadfromtable(vpTable_p);
	Vector<Record> recs;
	Vector<Vector<String> > antnames;

	if(vpman->imagepbinfo(antnames, recs)){
	  Vector<Bool> dishDefined(dishName.nelements(), False);
	  Int nbeams=antnames.nelements();
	  ///will be keying on file image file name here 
	  for (uInt k=0; k < dishDiam.nelements(); ++k){
	    String key;
	    Bool beamDone=False;
	    Int recordToUse=0;
	    for (Int j =0; j < nbeams; ++j){
	      key=recs[j].isDefined("realimage") ? recs[j].asString("realimage") : recs[j].asString("compleximage");
	      if(antnames[j][0]=="*" || anyEQ(dishName[k], antnames[j])){
		  dishDefined[k]=True;
		  recordToUse=j;
	      
		  if((diamIndex !=0) && antDiam2IndexMap_p.isDefined(key)){
		    antIndexToDiamIndex_p(k)=antDiam2IndexMap_p(key);
		    beamDone=True;
		  }
	      }
	    }
	    if(!beamDone && dishDefined[k]){
	      key=recs[recordToUse].isDefined("realimage") ? recs[recordToUse].asString("realimage") : recs[recordToUse].asString("compleximage");
	      antDiam2IndexMap_p.define(key, diamIndex);
	      antIndexToDiamIndex_p(k)=diamIndex;
	      antMath_p.resize(diamIndex+1);
	      if(recs[recordToUse].isDefined("realimage") && recs[recordToUse].isDefined("imagimage")){
		PagedImage<Float> realim(recs[recordToUse].asString("realimage"));
		PagedImage<Float> imagim(recs[recordToUse].asString("imagim"));
		antMath_p[diamIndex]=new PBMath2DImage(realim, imagim);
	      }
	      else {
		 antMath_p[diamIndex]=new PBMath2DImage(PagedImage<Complex>(recs[recordToUse].asString("compleximage")));
	      }
	      ++diamIndex;
	    }
	  }
	  if(!allTrue(dishDefined)){
	    //cerr << "dishDefined " << dishDefined << endl;
	    throw(AipsError("Some Antennas in the MS did not have a VP defined")); 
	  }
	}
	else{
	  throw(AipsError("Mosaic does not support non-image voltage patterns yet"));
	}
	
	//Get rid of the static class
	vpman->reset();
      }

      //cerr << "antIndexTodiamIndex " << antIndexToDiamIndex_p << endl;
    }
    
    
    


  }

  void  HetArrayConvFunc::reset(){
    doneMainConv_p=False;
    convFunctions_p.resize(0, True);
    convWeights_p.resize(0, True);
    convSizes_p.resize(0, True);
    convSupportBlock_p.resize(0, True);
    convFunctionMap_p.resize(0);

  }

 

  void HetArrayConvFunc::findConvFunction(const ImageInterface<Complex>& iimage, 
					  const VisBuffer& vb,
					  const Int& convSampling, const Vector<Double>& visFreq,
					  Array<Complex>& convFunc, 
					  Array<Complex>& weightConvFunc, 
					  Vector<Int>& convsize,
					  Vector<Int>& convSupport,
					  Vector<Int>& convFuncPolMap,
					  Vector<Int>& convFuncChanMap,
					  Vector<Int>& convFuncRowMap)
  {

    storeImageParams(iimage,vb);
    convFuncChanMap.resize(vb.nChannel());
    Vector<Double> beamFreqs;
    findUsefulChannels(convFuncChanMap, beamFreqs, vb, visFreq);
    //cerr << "SPW " << vb.spectralWindow() << "   beamFreqs "<< beamFreqs <<  " chamMap " << convFuncChanMap << endl;
    Int nBeamChans=beamFreqs.nelements();
    /////For now not doing beam rotation or squints but to be enabled easily 
    convFuncPolMap.resize(vb.nCorr());
    Int nBeamPols=1;
    convFuncPolMap.set(0);
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
    convFuncRowMap.resize();
    convsize.resize();
    convSupport.resize();

    Int isCached=checkPBOfField(vb, convFuncRowMap);
    //cout << "isCached " << isCached <<  endl;
    if(isCached==1 && (convFuncRowMap.shape()[0]==vb.nRow())){
      /*convFunc.reference(convFunc_p);
      weightConvFunc.reference(weightConvFunc_p);
      convsize=*convSizes_p[actualConvIndex_p];
      convSupport=convSupport_p;
       return;
      */
    }
    else if(isCached ==2){
      convFunc.resize();
      weightConvFunc.resize();
      convsize.resize();
      convSupport.resize();
      convFuncRowMap.resize();
      return;
      
    }
    actualConvIndex_p=convIndex(vb);
    //cerr << "actual conv index " << actualConvIndex_p << " doneMainconv " << doneMainConv_p << endl;
    if(doneMainConv_p.shape()[0] < (actualConvIndex_p+1)){
      //cerr << "resizing DONEMAIN " <<   doneMainConv_p.shape()[0] << endl;
    	doneMainConv_p.resize(actualConvIndex_p+1, True);
    	doneMainConv_p[actualConvIndex_p]=False;
    }
    ///// In multi ms mode ndishpair may change when meeting a new ms
    //// redo the calculation then
    if(doneMainConv_p[actualConvIndex_p] && ((convSupport_p.nelements() != uInt(ndishpair)) || convFunctions_p[actualConvIndex_p]->shape()[3] != nBeamChans)){
      doneMainConv_p[actualConvIndex_p]=False;
      //cerr << "invalidating doneMainConv " <<  convFunctions_p[actualConvIndex_p]->shape()[3] << " =? " << nBeamChans << " convsupp " << convSupport_p.nelements() << endl;
    }

    // Get the coordinate system
    CoordinateSystem coords(iimage.coordinates());
    Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
    AlwaysAssert(directionIndex>=0, AipsError);
    // Set up the convolution function.
    Int nx=nx_p;
    Int ny=ny_p;
    Int support=0;
    if(!doneMainConv_p[actualConvIndex_p]){
      for (uInt ii=0; ii < ndish; ++ii){
	support=max((antMath_p[ii])->support(coords), support);
      }
      support=Int(max(nx_p, ny_p)*2.0)/2;
      convSize_p=Int(max(nx_p, ny_p)*2.0)/2*convSampling;
      // Make this a nice composite number, to speed up FFTs
      CompositeNumber cn(uInt(convSize_p*2.0));    
      convSize_p  = cn.nextLargerEven(Int(convSize_p));
   
    }
    
    
    DirectionCoordinate dc=dc_p;
    //where in the image in pixels is this pointing
    Vector<Double> pixFieldDir(2);
    if(doneMainConv_p.shape()[0] < (actualConvIndex_p+1)){
      //cerr << "resizing DONEMAIN " <<   doneMainConv_p.shape()[0] << endl;
    	doneMainConv_p.resize(actualConvIndex_p+1, True);
    	doneMainConv_p[actualConvIndex_p]=False;
    }
    //no need to call toPix here as its been done already above in checkOFPB
    //thus the values are still current.
    pixFieldDir=thePix_p;
    //toPix(pixFieldDir, vb);
    MDirection fieldDir=direction1_p;
    //cout << "PixelFieldDir " << pixFieldDir << endl;
    //cout << "vb.nrow() "<< vb.nRow() << endl;
    //shift from center
    pixFieldDir(0)=pixFieldDir(0)- Double(nx / 2);
    pixFieldDir(1)=pixFieldDir(1)- Double(ny / 2);
    //phase gradient per pixel to apply
    pixFieldDir(0)=-pixFieldDir(0)*2.0*C::pi/Double(nx)/Double(convSampling);
    pixFieldDir(1)=-pixFieldDir(1)*2.0*C::pi/Double(ny)/Double(convSampling);

    
   
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

      IPosition pbShape(4, convSize_p, convSize_p, 1, nBeamChans);
      //TempImage<Complex> twoDPB(pbShape, coords);    
    
      
      TempLattice<Complex> convFuncTemp(TiledShape(IPosition(5, convSize_p/4, convSize_p/4, nBeamPols, nBeamChans, ndishpair), IPosition(5, convSize_p/4, convSize_p/4, 1, 1, 1)), 0);
      TempLattice<Complex> weightConvFuncTemp(TiledShape(IPosition(5, convSize_p/4, convSize_p/4, nBeamPols, nBeamChans, ndishpair), IPosition(5, convSize_p/4, convSize_p/4, 1, 1, 1)), 0);
	//convFunc_p.resize(IPosition(5, convSize_p, convSize_p, nBeamPols, nBeamChans, ndishpair));
       
      // convFunc_p=0.0;
      //weightConvFunc_p.resize(IPosition(5, convSize_p, convSize_p, nBeamPols, nBeamChans, ndishpair));
      //weightConvFunc_p=0.0;
      IPosition begin(5, 0, 0, 0, 0, 0);
      IPosition end(5, convFuncTemp.shape()[0]-1,  convFuncTemp.shape()[1]-1, nBeamPols-1, nBeamChans-1, 0);
      FFTServer<Float, Complex> fft(IPosition(2, convSize_p, convSize_p));
      //TempImage<Complex> pBScreen(TiledShape(pbShape, IPosition(4, convSize_p, convSize_p, 1, 1)), coords, 0);
      //TempImage<Complex> pB2Screen(TiledShape(pbShape, IPosition(4, convSize_p, convSize_p, 1, 1)), coords, 0);
      TempImage<Complex> pBScreen(TiledShape(pbShape), coords, 0);
      TempImage<Complex> pB2Screen(TiledShape(pbShape), coords, 0);
      IPosition start(4, 0, 0, 0, 0);
      convSupport_p.resize(ndishpair);
      for (uInt k=0; k < ndish; ++k){
      
	for (uInt j =k ; j < ndish; ++j){
	  //Timer tim;
	  //Matrix<Complex> screen(convSize_p, convSize_p);
	  //screen=1.0;
	  //pBScreen.putSlice(screen, start);
	  //cerr << "k " << k << " shape " << pBScreen.shape() <<  " direction1 " << direction1_p << " direction2 " << direction2_p << endl;
	  //tim.mark(); 
	  //pBScreen.set(Complex(1.0, 0.0));
	  //one antenna 
	 IPosition blcin(4, 0, 0, 0, 0);
	 IPosition trcin(4, convSize_p-1, convSize_p-1, 0, 0);
	 for (Int kk=0; kk < nBeamChans; ++kk){
	   blcin[3]=kk;
	   trcin[3]=kk;
	   //cerr << "Doing channel " << kk << endl;
	   Slicer slin(blcin, trcin, Slicer::endIsLast);
	   SubImage<Complex> subim(pBScreen, slin, True);
	   subim.set(Complex(1.0, 0.0));
	   (antMath_p[k])->applyVP(subim, subim, direction1_p);

	  //Then the other
	   (antMath_p[j])->applyVP(subim, subim, direction2_p);
	  //tim.show("After Apply ");
	  //tim.mark();
	   //pB2Screen.set(Complex(1.0,0.0));
	   SubImage<Complex> subim2(pB2Screen, slin, True);
	   subim2.set(Complex(1.0,0.0));
	  //one antenna 
	   (antMath_p[k])->applyPB(subim2, subim2, direction1_p);
	  //Then the other
	   (antMath_p[j])->applyPB(subim2, subim2, direction2_p);
	  
	   //tim.show("After Apply2 ");
	   //tim.mark();
	   subim.copyData((LatticeExpr<Complex>) (iif(abs(subim)> 5e-2, subim, 0)));
	   subim2.copyData((LatticeExpr<Complex>) (iif(abs(subim2)> 25e-4, subim2, 0)));
	    LatticeFFT::cfft2d(subim);
	    LatticeFFT::cfft2d(subim2);

       

	 }
	 /*
	 if(nBeamChans >1){
	   Slicer slin(blcin, trcin, Slicer::endIsLast);
	   SubImage<Complex> origPB(pBScreen, slin, False);
	   IPosition elshape= origPB.shape();
	   Matrix<Complex> i1=origPB.get(True);
	   SubImage<Complex> origPB2(pB2Screen, slin, False);
	   Matrix<Complex> i2=origPB2.get(True);
	   Int cenX=i1.shape()(0)/2;
	   Int cenY=i1.shape()(1)/2;
	   
	   for (Int kk=0; kk < (nBeamChans-1); ++kk){
	     Double fratio=beamFreqs(kk)/beamFreqs(nBeamChans-1);
	     cerr << "fratio " << fratio << endl;
	     blcin[3]=kk;
	     trcin[3]=kk;
	     //Slicer slout(blcin, trcin, Slicer::endIsLast);
	     Matrix<Complex> o1(i1.shape(), Complex(0.0));
	     Matrix<Complex> o2(i2.shape(), Complex(0.0));
	     for (Int yy=0;  yy < i1.shape()(1); ++yy){
	       //Int nyy= (Double(yy-cenY)*fratio) + cenY; 
	       Double nyy= (Double(yy-cenY)/fratio) + cenY;
	       Double cyy=ceil(nyy);
	       Double fyy= floor(nyy);
	       Int iy=nyy > fyy+0.5 ? cyy : fyy; 
	       if(cyy <2*cenY && fyy >=0.0)
		 for(Int xx=0; xx < i1.shape()(0); ++ xx){
		   //Int nxx= Int(Double(xx-cenX)*fratio) + cenX; 
		   Double nxx= Int(Double(xx-cenX)/fratio) + cenX;
		    Double cxx=ceil(nxx);
		    Double fxx= floor(nxx);
		    Int ix=nxx > fxx+0.5 ? cxx : fxx ;
		   if(cxx < 2*cenX && fxx >=0.0 ){
		     //Double dist=sqrt((nxx-cxx)*(nxx-cxx)+(nyy-cyy)*(nyy-cyy))/sqrt(2.0);
		     //o1(xx, yy)=float(1-dist)*i1(fxx, fyy)+ dist*i1(cxx,cyy);
		     o1(xx, yy)=i1( ix, iy);
		     //o2(xx, yy)=i2(nxx, nyy);
		     //o2(xx, yy)=float(1-dist)*i2(fxx, fyy)+ dist*i2(cxx,cyy);
		     o2(xx, yy)=i2(ix, iy);
		   }
		 }
	     }
	     pBScreen.putSlice(o1.reform(elshape), blcin);
	     pB2Screen.putSlice(o2.reform(elshape), blcin);
	   }

	 }
	 */
	   
	     //tim.show("after apply+apply2+masking+fft ");
	 //tim.mark();
	 //LatticeFFT::cfft2d(pBScreen);
	 //LatticeFFT::cfft2d(pB2Screen);
	  
	  //Matrix<Complex> lala=pBScreen.get(True);
	  //fft.fft0(lala, True);
	  //fft.flip(lala, False, False);
	  // pBScreen.put(lala.reform(IPosition(4, convSize_p, convSize_p, 1, 1)));
	  //lala=pB2Screen.get(True);
	  //fft.fft0(lala, True);
	  //fft.flip(lala, False, False);
	  //pB2Screen.put(lala.reform(IPosition(4, convSize_p, convSize_p, 1, 1)));
	  
	  //////////*****************
	  /*if(0){
	    ostringstream os1;
	    os1 << "PB_field_" << Int(thePix_p[0]) << "_" << Int(thePix_p[1]) << "_antpair_" << k <<"_"<<j ;
	    PagedImage<Float> thisScreen(pbShape, coords, String(os1));
	    LatticeExpr<Float> le(abs(pBScreen));
	    thisScreen.copyData(le);
	  
	    }*/
	  ////////*****************/
	 
	  //tim.show("after FFT ");
	  //tim.mark();
	  Int plane=0;
	  for (uInt jj=0; jj < k; ++jj)
	    plane=plane+ndish-jj-1;
	  plane=plane+j;
	  begin[4]=plane;
	  end[4]=plane;
	  Slicer slplane(begin, end, Slicer::endIsLast);
	  //cerr <<  "SHAPES " << convFunc_p(begin, end).shape() << "  " << pBScreen.get(False).shape() << " begin and end " << begin << "    " << end << endl;
	  //convFunc_p(begin, end).copyMatchingPart(pBScreen.get(False));
	  //weightConvFunc_p(begin, end).copyMatchingPart(pB2Screen.get(False));
	  IPosition blcQ(4, pbShape(0)/8*3, pbShape(1)/8*3, 0, 0);
	  IPosition trcQ(4,  pbShape(0)/8*5-1, pbShape(1)/8*5-1, nBeamPols-1, nBeamChans-1);
	  Slicer slQ(blcQ, trcQ, Slicer::endIsLast);
	  {
	    SubImage<Complex>  pBSSub(pBScreen, slQ, False);
	    SubLattice<Complex> cFTempSub(convFuncTemp,  slplane, True);
	    LatticeConcat<Complex> lc(4);
	    lc.setLattice(pBSSub);
	    //cerr << "SHAPES " << cFTempSub.shape() << "   " <<  lc.shape() << endl;
	    cFTempSub.copyData(lc);
	    //cFTempSub.copyData(pBScreen);
	  }
	  {
	    SubImage<Complex>  pB2SSub(pB2Screen, slQ, False);
	    SubLattice<Complex> cFTempSub2(weightConvFuncTemp,  slplane, True);
	    LatticeConcat<Complex> lc(4);
	    lc.setLattice(pB2SSub);
	    cFTempSub2.copyData(lc);
	    // cFTempSub2.copyData(pB2Screen);
	    //weightConvFuncTemp.putSlice(pB2Screen.get(False), begin);

	  }
	  //	  supportAndNormalize(plane, convSampling);
	  supportAndNormalizeLatt( plane, convSampling, convFuncTemp,  weightConvFuncTemp);



	  // tim.show("After search of support ");
	}
	
      }


      doneMainConv_p[actualConvIndex_p]=True;
      convFunctions_p.resize(actualConvIndex_p+1);
      convWeights_p.resize(actualConvIndex_p+1);
      convFunctions_p[actualConvIndex_p]= new Array<Complex>();
      convWeights_p[actualConvIndex_p]= new Array<Complex>();
      Int newConvSize=2*(max(convSupport_p)+2)*convSampling;
      //cerr << "new ConvSize " << newConvSize << endl;
      Int lattSize=convFuncTemp.shape()(0);
      if(newConvSize < lattSize){
	IPosition blc(5, (lattSize/2)-(newConvSize/2),
		      (lattSize/2)-(newConvSize/2),0,0,0);
	IPosition trc(5, (lattSize/2)+(newConvSize/2-1),
		      (lattSize/2)+(newConvSize/2-1), nBeamPols-1, nBeamChans-1,ndishpair-1);
	IPosition shp(5, newConvSize, newConvSize, nBeamPols, nBeamChans, ndishpair);
	
	convFunctions_p[actualConvIndex_p]= new Array<Complex>(IPosition(5, newConvSize, newConvSize, nBeamPols, nBeamChans, ndishpair ));
	convWeights_p[actualConvIndex_p]= new Array<Complex>(IPosition(5, newConvSize, newConvSize, nBeamPols, nBeamChans, ndishpair ));
	(*convFunctions_p[actualConvIndex_p])=convFuncTemp.getSlice(blc,shp);
	convSize_p=newConvSize;
	(*convWeights_p[actualConvIndex_p])=weightConvFuncTemp.getSlice(blc, shp);
	convFunc_p.resize();
	weightConvFunc_p.resize();
      }
      else{
	(*convFunctions_p[actualConvIndex_p])=convFuncTemp.get();
	(*convWeights_p[actualConvIndex_p])=weightConvFuncTemp.get();
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
    makerowmap(vb, convFuncRowMap);

    
    convSupportBlock_p.resize(actualConvIndex_p+1);
    convSizes_p.resize(actualConvIndex_p+1);
    convSupportBlock_p[actualConvIndex_p]=new Vector<Int>(ndishpair);
    (*convSupportBlock_p[actualConvIndex_p])=convSupport_p;
    convSizes_p[actualConvIndex_p]=new Vector<Int> (ndishpair);
    
    /*    convFunctions_p[actualConvIndex_p]->resize(convSize_p, convSize_p, ndishpair);
    *(convFunctions_p[actualConvIndex_p])=convSave_p; 
    convWeights_p[actualConvIndex_p]->resize(convSize_p, convSize_p, ndishpair);
    *(convWeights_p[actualConvIndex_p])=weightSave_p;
    */

    convFunc_p.resize();
    convFunc_p=(*convFunctions_p[actualConvIndex_p]);
    weightConvFunc_p.resize();
    weightConvFunc_p=(*convWeights_p[actualConvIndex_p]);
    Bool delc; Bool delw;
    Double dirX=pixFieldDir(0);
    Double dirY=pixFieldDir(1);
    Complex *convstor=convFunc_p.getStorage(delc);
    Complex *weightstor=weightConvFunc_p.getStorage(delw);
    Int elconvsize=convSize_p;
    //cerr << "SHAPES " << convFunc_p.shape() << "     " << weightConvFunc_p.shape() << endl;

#pragma omp parallel default(none) firstprivate(convstor, weightstor, dirX, dirY, elconvsize, ndishpair, nBeamChans, nBeamPols)
    {
#pragma omp for
      for(Int iy=0; iy<elconvsize; ++iy){
	applyGradientToYLine(iy,  convstor, weightstor, dirX, dirY, elconvsize, ndishpair, nBeamChans, nBeamPols);
	
      }
    }///End of pragma

    convFunc_p.putStorage(convstor, delc);
    weightConvFunc_p.putStorage(weightstor, delw);



    //For now all have the same size convsize;
    convSizes_p[actualConvIndex_p]->set(convSize_p);
    
    //We have to get the references right now
    //    convFunc_p.resize();
    //convFunc_p.reference(*convFunctions_p[actualConvIndex_p]);
    //weightConvFunc_p.resize();
    //weightConvFunc_p.reference(*convWeights_p[actualConvIndex_p]);

    convFunc.reference(convFunc_p);
    weightConvFunc.reference(weightConvFunc_p);
    convsize=*convSizes_p[actualConvIndex_p];
    convSupport=convSupport_p;


  }


  void HetArrayConvFunc::applyGradientToYLine(const Int iy, Complex*& convFunctions, Complex*& convWeights, const Double pixXdir, const Double pixYdir, Int convSize, const Int ndishpair, const Int nChan, const Int nPol){
    Double cy, sy;

    SINCOS(Double(iy-convSize/2)*pixYdir, sy, cy);
    Complex phy(cy,sy) ;
    for (Int ix=0;ix<convSize;ix++) {
      Double cx, sx;
      SINCOS(Double(ix-convSize/2)*pixXdir, sx, cx);
      Complex phx(cx,sx) ;
      for (Int ipol=0; ipol< nPol; ++ipol){
	//Int poloffset=ipol*nChan*ndishpair*convSize*convSize;
	for (Int ichan=0; ichan < nChan; ++ichan){
	  //Int chanoffset=ichan*ndishpair*convSize*convSize;
	  for(Int iz=0; iz <ndishpair; ++iz){
	    Int index=(((ipol*nChan+ichan)*ndishpair+iz)*convSize+iy)*convSize+ix;
	    convFunctions[index]= convFunctions[index]*phx*phy;
	    convWeights[index]= convWeights[index]*phx*phy;
	  }
	}
      }
    
  }
  }
   Bool HetArrayConvFunc::toRecord(RecordInterface& rec){
     
     try{
       rec.define("name", "HetArrayConvFunc");
       Int numConv=convFunctions_p.nelements();
       rec.define("ndefined", numConv);
       //rec.define("convfunctionmap", convFunctionMap_p);
       std::map<String, Int>::iterator it=vbConvIndex_p.begin();
       for (Int64 k=0; k < numConv; ++k){
	 rec.define("convfunctions"+String::toString(k), *(convFunctions_p[k]));
	 rec.define("convweights"+String::toString(k), *(convWeights_p[k]));
	 rec.define("convsizes"+String::toString(k), *(convSizes_p[k]));
	 rec.define("convsupportblock"+String::toString(k), *(convSupportBlock_p[k]));
	 rec.define(String("key")+String::toString(k), it->first);
	rec.define(String("val")+String::toString(k), it->second);
	it++;
       }
       rec.define("actualconvindex",  actualConvIndex_p);
       rec.define("donemainconv", doneMainConv_p);
       rec.define("vptable", vpTable_p);
       rec.define("pbclass", Int(pbClass_p));
       
    }
    catch(AipsError x) {
      return False;
    }
    return True;   

   }

  Bool HetArrayConvFunc::fromRecord(String& err, const RecordInterface& rec, Bool calcfluxscale){
    //Force pbmath stuff and saved image stuff
    nchan_p=0;
    msId_p=-1;
    try{
      if(!rec.isDefined("name") || rec.asString("name") != "HetArrayConvFunc"){
	throw(AipsError("Wrong record to recover HetArray from"));
      }
      nDefined_p=rec.asInt("ndefined");
      //rec.get("convfunctionmap", convFunctionMap_p);
      convFunctions_p.resize(nDefined_p, True, False);
      convSupportBlock_p.resize(nDefined_p, True, False);
      convWeights_p.resize(nDefined_p, True, False);
      convSizes_p.resize(nDefined_p, True, False);
      vbConvIndex_p.erase(vbConvIndex_p.begin(), vbConvIndex_p.end());
      for (Int64 k=0; k < nDefined_p; ++k){
	convFunctions_p[k]=new Array<Complex>();
	convWeights_p[k]=new Array<Complex>();
	convSizes_p[k]=new Vector<Int>();
	convSupportBlock_p[k]=new Vector<Int>();
	rec.get("convfunctions"+String::toString(k), *(convFunctions_p[k]));
	rec.get("convweights"+String::toString(k), *(convWeights_p[k]));
	rec.get("convsizes"+String::toString(k), *(convSizes_p[k]));
	rec.get("convsupportblock"+String::toString(k), *(convSupportBlock_p[k]));
	String key;
	Int val;
	rec.get(String("key")+String::toString(k), key);
	rec.get(String("val")+String::toString(k), val);
	vbConvIndex_p[key]=val;
      }
      //Now that we are calculating all phase gradients on the fly ..
      ///we should clean up some and get rid of the cached variables
      convSize_p=(*(convSizes_p[0]))[0];
      //convSave_p.resize();
      //rec.get("convsave", convSave_p);
      //weightSave_p.resize();
      //rec.get("weightsave", weightSave_p);
      rec.get("vptable", vpTable_p);
      rec.get("donemainconv", doneMainConv_p);
      //convSupport_p.resize();
      //rec.get("convsupport", convSupport_p);
      pbClass_p=static_cast<PBMathInterface::PBClass>(rec.asInt("pbclass"));
      calcFluxScale_p=calcfluxscale;
    }
    catch(AipsError x) {
      err=x.getMesg();
      return False;
    } 
      
    return True;
  }


  void HetArrayConvFunc::supportAndNormalize(Int plane, Int convSampling){

    LogIO os;
    os << LogOrigin("HetArrConvFunc", "suppAndNorm")  << LogIO::NORMAL;
    // Locate support
	Int convSupport=-1;
	IPosition begin(5, 0, 0, 0, 0, plane);
	IPosition end(5, convFunc_p.shape()[0]-1,  convFunc_p.shape()[1]-1, 0, 0, plane);
	Matrix<Complex> convPlane(convFunc_p(begin, end).reform(IPosition(2,convFunc_p.shape()[0], convFunc_p.shape()[1]))) ;
	Float maxAbsConvFunc=max(amplitude(convPlane));
	Float minAbsConvFunc=min(amplitude(convPlane));
	Bool found=False;
	Int trial=0;
	for (trial=convSize_p/2-2;trial>0;trial--) {
	  //Searching down a diagonal
	  if(abs(convPlane(convSize_p/2-trial,convSize_p/2-trial)) >  (1.0e-2*maxAbsConvFunc)) {
	    found=True;
	    trial=Int(sqrt(2.0*Float(trial*trial)));
	    break;
	  }
	}
	if(!found){
	  if((maxAbsConvFunc-minAbsConvFunc) > (1.0e-2*maxAbsConvFunc)) 
	  found=True;
	  // if it drops by more than 2 magnitudes per pixel
	  trial=( (10*convSampling) < convSize_p) ? 5*convSampling : (convSize_p/2 - 4*convSampling);
	}
       
				 
	if(found) {
	  if(trial < 5*convSampling) 
	    trial= ( (10*convSampling) < convSize_p) ? 5*convSampling : (convSize_p/2 - 4*convSampling);
	  convSupport=Int(0.5+Float(trial)/Float(convSampling))+1;
	  //support is really over the edge
	  if( (convSupport*convSampling) >= convSize_p/2){
	    convSupport=convSize_p/2/convSampling-1;
	  }
	}
	else {
	  /*
	  os << "Convolution function is misbehaved - support seems to be zero\n"
	     << "Reasons can be: \nThe image definition not covering one or more of the pointings selected \n"
         << "Or no unflagged data in a given pointing"
	     
	     << LogIO::EXCEPTION;
	  */
	  //OTF may have flagged stuff ...
	  convSupport=0;
	}
	//cerr << "trial " << trial << " convSupport " << convSupport << " convSize " << convSize_p << endl;
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
	if(convSupport >0){
	  IPosition blc(2, -convSupport*convSampling+convSize_p/2, -convSupport*convSampling+convSize_p/2);
	  IPosition trc(2, convSupport*convSampling+convSize_p/2, convSupport*convSampling+convSize_p/2);
	  for (Int chan=0; chan < convFunc_p.shape()[3]; ++chan){
	    begin[3]=chan;
	    end[3]=chan;
	    convPlane.resize();
	    convPlane.reference(convFunc_p(begin, end).reform(IPosition(2,convFunc_p.shape()[0], convFunc_p.shape()[1])));
	    pbSum=real(sum(convPlane(blc,trc)))/Double(convSampling)/Double(convSampling);
	    if(pbSum>0.0) {
	      (convPlane)=convPlane*Complex(1.0/pbSum,0.0);
	      convPlane.resize();
	      convPlane.reference(weightConvFunc_p(begin, end).reform(IPosition(2,convFunc_p.shape()[0], convFunc_p.shape()[1])));
	      (convPlane) =(convPlane)*Complex(1.0/pbSum,0.0);
	    }
	    else {
	      os << "Convolution function integral is not positive"
		 << LogIO::EXCEPTION;
	    }
	  }
	}
	else{
	  //no valid convolution for this pointing
	  for (Int chan=0; chan < convFunc_p.shape()[3]; ++chan){
	    begin[3]=chan;
	    end[3]=chan;
	    convFunc_p(begin, end).set(0.0);
	    weightConvFunc_p(begin, end).set(0.0);
	  //convFunc_p.xyPlane(plane).set(0.0);
	  //weightConvFunc_p.xyPlane(plane).set(0.0);
	  }
	}

  }

  void HetArrayConvFunc::supportAndNormalizeLatt(Int plane, Int convSampling, TempLattice<Complex>& convFuncLat,
						 TempLattice<Complex>& weightConvFuncLat){

    LogIO os;
    os << LogOrigin("HetArrConvFunc", "suppAndNorm")  << LogIO::NORMAL;
    // Locate support
	Int convSupport=-1;
	IPosition begin(5, 0, 0, 0, 0, plane);
	IPosition shape(5, convFuncLat.shape()[0],  convFuncLat.shape()[1], 1, 1, 1);
	//Int convSize=convSize_p;
	Int convSize=shape(0);
	Matrix<Complex> convPlane=convFuncLat.getSlice(begin, shape, True);
	Float maxAbsConvFunc=max(amplitude(convPlane));
	Float minAbsConvFunc=min(amplitude(convPlane));
	Bool found=False;
	Int trial=0;
	for (trial=convSize/2-2;trial>0;trial--) {
	  //Searching down a diagonal
	  if(abs(convPlane(convSize/2-trial,convSize/2-trial)) >  (1.0e-2*maxAbsConvFunc)) {
	    found=True;
	    trial=Int(sqrt(2.0*Float(trial*trial)));
	    break;
	  }
	}
	if(!found){
	  if((maxAbsConvFunc-minAbsConvFunc) > (1.0e-2*maxAbsConvFunc)) 
	  found=True;
	  // if it drops by more than 2 magnitudes per pixel
	  trial=( (10*convSampling) < convSize) ? 5*convSampling : (convSize/2 - 4*convSampling);
	}
       
				 
	if(found) {
	  if(trial < 5*convSampling) 
	    trial= ( (10*convSampling) < convSize) ? 5*convSampling : (convSize/2 - 4*convSampling);
	  convSupport=Int(0.5+Float(trial)/Float(convSampling))+1;
	  //support is really over the edge
	  if( (convSupport*convSampling) >= convSize/2){
	    convSupport=convSize/2/convSampling-1;
	  }
	}
	else {
	  /*
	  os << "Convolution function is misbehaved - support seems to be zero\n"
	     << "Reasons can be: \nThe image definition not covering one or more of the pointings selected \n"
         << "Or no unflagged data in a given pointing"
	     
	     << LogIO::EXCEPTION;
	  */
	  //OTF may have flagged stuff ...
	  convSupport=0;
	}
	//cerr << "trial " << trial << " convSupport " << convSupport << " convSize " << convSize_p << endl;
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
	//cerr << "convSize_p " << convSize_p <<  " convSize " << convSize << endl; 
	if(convSupport >0){
	  IPosition blc(2, -convSupport*convSampling+convSize/2, -convSupport*convSampling+convSize/2);
	  IPosition trc(2, convSupport*convSampling+convSize/2, convSupport*convSampling+convSize/2);
	  for (Int chan=0; chan < convFuncLat.shape()[3]; ++chan){
	    begin[3]=chan;
	    //end[3]=chan;
	    convPlane.resize();
	    convPlane=convFuncLat.getSlice(begin, shape, True);
	    pbSum=real(sum(convPlane(blc,trc)))/Double(convSampling)/Double(convSampling);
	    if(pbSum>0.0) {
	      (convPlane)=convPlane*Complex(1.0/pbSum,0.0);
	      convFuncLat.putSlice(convPlane, begin);
	      convPlane.resize();
	      convPlane=weightConvFuncLat.getSlice(begin, shape, True);
	      (convPlane) =(convPlane)*Complex(1.0/pbSum,0.0);
	      weightConvFuncLat.putSlice(convPlane, begin);
	    }
	    else {
	      os << "Convolution function integral is not positive"
		 << LogIO::EXCEPTION;
	    }
	  }
	}
	else{
	  //no valid convolution for this pointing
	  for (Int chan=0; chan < convFuncLat.shape()[3]; ++chan){
	    begin[3]=chan;
	    //end[3]=chan;
	    convPlane.resize(shape[0], shape[1]);
	    convPlane.set(0.0);
	    convFuncLat.putSlice(convPlane, begin);
	    weightConvFuncLat.putSlice(convPlane, begin);
	  //convFunc_p.xyPlane(plane).set(0.0);
	  //weightConvFunc_p.xyPlane(plane).set(0.0);
	  }
	}

  }

  Int HetArrayConvFunc::factorial(Int n){
    Int fact=1;
    for (Int k=1; k<=n; ++k)
      fact *=k;
    return fact;
  }


  Int HetArrayConvFunc::checkPBOfField(const VisBuffer& vb, 
				       Vector<Int>& /*rowMap*/){
    
    toPix(vb);
    Vector<Int> pixdepoint(2);
    convertArray(pixdepoint, thePix_p);
    if((pixdepoint(0) < 0) ||  pixdepoint(0) >= nx_p || pixdepoint(1) < 0 || 
       pixdepoint(1) >=ny_p){
      //cout << "in pix de point off " << pixdepoint << endl;
      return 2;
    }
    String pointingid=String::toString(pixdepoint(0))+"_"+String::toString(pixdepoint(1));
    //Int fieldid=vb.fieldId();
    String msid=vb.msName(True);
    //If channel or pol length has changed underneath...then its time to 
    //restart the map
    /*
    if(convFunctionMap_p.ndefined() > 0){
      if ((fluxScale_p.shape()[3] != nchan_p) || (fluxScale_p.shape()[2] != npol_p)){
	convFunctionMap_p.clear();
      }
    }

    */
    if(convFunctionMap_p.nelements() > 0){
      if (calcFluxScale_p && ((fluxScale_p.shape()[3] != nchan_p) || (fluxScale_p.shape()[2] != npol_p))){
	convFunctionMap_p.resize();
	nDefined_p=0;
      }
    }
    //String mapid=msid+String("_")+pointingid;
    /*
    if(convFunctionMap_p.ndefined() == 0){
      convFunctionMap_p.define(mapid, 0);    
      actualConvIndex_p=0;
      fluxScale_p=TempImage<Float>(IPosition(4,nx_p,ny_p,npol_p,nchan_p), csys_p);
      filledFluxScale_p=False;
      fluxScale_p.set(0.0);
      return -1;
    }
    */
    if(convFunctionMap_p.nelements() == 0){
      convFunctionMap_p.resize(nx_p*ny_p);
      convFunctionMap_p.set(-1);
      convFunctionMap_p[pixdepoint[1]*nx_p+pixdepoint[0]]=0;
      nDefined_p=1;
      actualConvIndex_p=0;
      if(calcFluxScale_p){
	fluxScale_p=TempImage<Float>(IPosition(4,nx_p,ny_p,npol_p,nchan_p), csys_p);
	filledFluxScale_p=False;
	fluxScale_p.set(0.0);
      }
      return -1;
    }
    
    // if(!convFunctionMap_p.isDefined(mapid)){
    //  actualConvIndex_p=convFunctionMap_p.ndefined();
    //  convFunctionMap_p.define(mapid, actualConvIndex_p);
    if(convFunctionMap_p[pixdepoint[1]*nx_p+pixdepoint[0]] <0){
      actualConvIndex_p=nDefined_p;
      convFunctionMap_p[pixdepoint[1]*nx_p+pixdepoint[0]]=nDefined_p;
      // ++nDefined_p;
      nDefined_p=1;
      return -1;
    }
    else{
      /*
      actualConvIndex_p=convFunctionMap_p[pixdepoint[1]*nx_p+pixdepoint[0]]; 
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
      */
      actualConvIndex_p=0;
      return -1;
    }

    return 1;
  

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
    if(!calcFluxScale_p)
      throw(AipsError("Programmer Error: flux image cannot be retrieved"));
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

void HetArrayConvFunc::sliceFluxScale(Int npol) {
     IPosition fshp=fluxScale_p.shape();
     if (fshp(2)>npol){
       npol_p=npol;
       // use first npol planes...
       IPosition blc(4,0,0,0,0);
       IPosition trc(4,fluxScale_p.shape()(0)-1, fluxScale_p.shape()(1)-1,npol-1,fluxScale_p.shape()(3)-1);
       Slicer sl=Slicer(blc, trc, Slicer::endIsLast);
       //writeable if possible
       SubImage<Float> fluxScaleSub = SubImage<Float> (fluxScale_p, sl, True);
       SubImage<Float> convWeightImageSub = SubImage<Float> (*convWeightImage_p, sl, True);
       fluxScale_p = TempImage<Float>(fluxScaleSub.shape(),fluxScaleSub.coordinates());
       TempImage<Float>* convWeightImage_p = new TempImage<Float> (convWeightImageSub.shape(),convWeightImageSub.coordinates());
       LatticeExpr<Float> le(fluxScaleSub);
       fluxScale_p.copyData(le);
       LatticeExpr<Float> le2(convWeightImageSub);
       convWeightImage_p->copyData(le2);
     }
  }

} //# NAMESPACE CASA - END




