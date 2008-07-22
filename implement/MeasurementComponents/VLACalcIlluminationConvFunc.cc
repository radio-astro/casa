// -*- C++ -*-
//# VLAIlluminationConvFunc.cc: Implementation for VLAIlluminationConvFunc
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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

#define USETABLES 1
#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <images/Images/ImageRegrid.h>
#include <images/Images/PagedImage.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/File.h>
#include <fstream>
#include <casa/sstream.h>
namespace casa{

  //
  //------------------------------------------------------------------------
  //
  VLACalcIlluminationConvFunc::VLACalcIlluminationConvFunc():IlluminationConvFunc()
  {
    ap.oversamp = 3;
    ap.x0=-13.0; ap.y0=-13.0;
    ap.dx=0.5; ap.dy=0.5;

    ap.nx=ap.ny=104;
    ap.pa=lastPA=18000000;
    ap.freq=1.365;
    ap.freq=1.4;
    ap.band = BeamCalc_VLA_L;
    //    ap.band = bandID;
    IPosition shape(4,ap.nx,ap.ny,4,1);
    ap.aperture = new TempImage<Complex>();
    if (maximumCacheSize() > 0) ap.aperture->setMaximumCacheSize(maximumCacheSize());
    ap.aperture->resize(shape);
   }


  CoordinateSystem VLACalcIlluminationConvFunc::makeUVCoords(CoordinateSystem& imageCoordSys,
							 IPosition& shape)
  {
    CoordinateSystem FTCoords = imageCoordSys;

    Int dirIndex=FTCoords.findCoordinate(Coordinate::DIRECTION);
    DirectionCoordinate dc=imageCoordSys.directionCoordinate(dirIndex);
    Vector<Bool> axes(2); axes=True;
    Vector<Int> dirShape(2); dirShape(0)=shape(0);dirShape(1)=shape(1);
    Coordinate* FTdc=dc.makeFourierCoordinate(axes,dirShape);
    //    cout << dc.increment() << " " << dirShape << " " << FTdc->increment() << endl;

    FTCoords.replaceCoordinate(*FTdc,dirIndex);
    delete FTdc;

    return FTCoords;
  }

  void VLACalcIlluminationConvFunc::applyPB(ImageInterface<Float>& pbImage,
					    const VisBuffer& vb, const Vector<Float>& paList, Int bandID)
  {
    CoordinateSystem skyCS(pbImage.coordinates());
    IPosition skyShape(pbImage.shape());
    TempImage<Complex> uvGrid;
    if (maximumCacheSize() > 0) uvGrid.setMaximumCacheSize(maximumCacheSize());
    regridAperture(skyCS, skyShape, uvGrid, vb, paList, False, bandID);
    //    cout << "Regrid Aperture done" <<endl;
    fillPB(*(ap.aperture),pbImage);
    //    cout << "FillPB done" << endl;
//     String name("calcI.im");
//     storeImg(name,pbImage);
//     exit(0);
  }

  void VLACalcIlluminationConvFunc::applyPB(ImageInterface<Float>& pbImage,
					    const VisBuffer& vb, Int bandID)
  {
    CoordinateSystem skyCS(pbImage.coordinates());
    IPosition skyShape(pbImage.shape());
    //    TempImage<Complex> uvGrid(skyShape, skyCS);
    TempImage<Complex> uvGrid;
    if (maximumCacheSize() > 0) uvGrid.setMaximumCacheSize(maximumCacheSize());
    regridAperture(skyCS, skyShape, uvGrid, vb,False, bandID);
    fillPB(*(ap.aperture),pbImage);
//     String name("calcI.im");
//     storeImg(name,pbImage);
//     exit(0);
  }

  void VLACalcIlluminationConvFunc::applyPB(ImageInterface<Complex>& pbImage, 
					    const VisBuffer& vb, Int bandID)
  {
    CoordinateSystem skyCS(pbImage.coordinates());
    IPosition skyShape(pbImage.shape());
    //    TempImage<Complex> uvGrid(skyShape, skyCS);
    TempImage<Complex> uvGrid;
    if (maximumCacheSize() > 0) uvGrid.setMaximumCacheSize(maximumCacheSize());
    regridAperture(skyCS, skyShape, uvGrid, vb, True, bandID);
    fillPB(*(ap.aperture),pbImage);
//       String name("calcAF.im");
//       storeImg(name,pbImage);
//       exit(0);
  }

  void VLACalcIlluminationConvFunc::regridAperture(CoordinateSystem& skyCS,
						   IPosition& skyShape,
						   TempImage<Complex>& uvGrid,
						   const VisBuffer& vb,
						   Bool doSquint, Int bandID)
  {
    CoordinateSystem skyCoords(skyCS);

    Int index;
    Double timeValue = getCurrentTimeStamp(vb);
    Float pa;
    if (bandID != -1) ap.band = (BeamCalcBandCode)bandID;
    AlwaysAssert(ap.band>=-1, AipsError);
    {
      Vector<Float> antPA = vb.feed_pa(timeValue);
      //      pa = vb.feed_pa(timeValue)(0);
      pa = sum(antPA)/(antPA.nelements()-1);
    }

    Float Freq;
    Vector<Double> chanFreq = vb.frequency();

    if (lastPA != pa)
      {
	//	cout << "Making PB for PA = " << pa*57.2956 << endl;
	lastPA = pa;
	
	const ROMSSpWindowColumns& spwCol = vb.msColumns().spectralWindow();
	ROArrayColumn<Double> chanfreq = spwCol.chanFreq();
	ROScalarColumn<Double> reffreq = spwCol.refFrequency();
	//    cout << "MS spectralWindow column = " << chanFreq.nrow() << " " << chanFreq.getColumn() << endl;
	//     cout << "MS spectralWindow column: Chan Freq = " << chanFreq 
	// 	 << " Ref. Freq. = " << reffreq.getColumn()
	// 	 << " NRows = " << spwCol.nrow() << endl;
	
	Freq = sum(chanFreq)/chanFreq.nelements();
	ap.freq = Freq/1E9;
	
	//    IPosition imsize(ap.aperture->shape());
	IPosition imsize(skyShape);
	CoordinateSystem uvCoords = makeUVCoords(skyCoords,imsize);
	
	index = uvCoords.findCoordinate(Coordinate::LINEAR);
	LinearCoordinate lc=uvCoords.linearCoordinate(index);
	Vector<Double> incr = lc.increment();
	Double Lambda = C::c/Freq;
	
	ap.nx = skyShape(0); ap.ny = skyShape(1);
	IPosition apertureShape(ap.aperture->shape());
	apertureShape(0) = ap.nx;  apertureShape(1) = ap.ny;
	ap.aperture->resize(apertureShape);
	ap.dx = abs(incr(0)*Lambda); ap.dy = abs(incr(1)*Lambda);
	//	cout << ap.dx << " " << incr(0) << endl;
	//	ap.x0 = -(25.0/(2*ap.dx)+1)*ap.dx; ap.y0 = -(25.0/(2*ap.dy)+1)*ap.dy;
	// Following 3 lines go with the ANT tag in BeamCalc.cc
	//	Double antRadius=BeamCalcGeometryDefaults[ap.band].Rant;
	//	ap.x0 = -(antRadius/(ap.dx)+1)*ap.dx; 
	//	ap.y0 = -(antRadius/(ap.dy)+1)*ap.dy;
	// Following 2 lines go with the PIX tag in BeamCalc.cc
 	ap.x0 = -(ap.nx/2)*ap.dx; 
 	ap.y0 = -(ap.ny/2)*ap.dy;

	ap.pa=pa;
	//	cout << "x0, dx, nx = " << ap.x0 << " " << ap.dx << " " << ap.nx << endl;
	//	cout << "Calculating Aperture [PA=" << ap.pa*180/M_PI << "]...";
	ap.aperture->set(0.0);
	calculateAperture(&ap);
	//	cout << "Calculate Aperture done" << endl;
	
	//  Make the aperture function = (1,0) - for testing
	
// 	cout << "Making (1,0) aperture" << endl;
// 	IPosition ttndx(4,0,0,0,0);
// 	for(ttndx(3)=0;ttndx(3)<apertureShape(3);ttndx(3)++)
// 	  for(ttndx(2)=0;ttndx(2)<apertureShape(2);ttndx(2)++)
// 	    for(ttndx(1)=0;ttndx(1)<apertureShape(1);ttndx(1)++)
// 	      for(ttndx(0)=0;ttndx(0)<apertureShape(0);ttndx(0)++)
// 		{
// 		  Complex val;
// 		  val = ap.aperture->getAt(ttndx);
// 		  if (abs(val) != 0.0) val = Complex(1,0);
// 		  ap.aperture->putAt(val,ttndx);
// 		}
	
	//
	// Set the phase of the aperture function to zero if doSquint==F
	// Poln. axis indices
        // 0: RR, 1:RL, 2:LR, 3:LL
	// This is electic field. 0=> Poln R, 
	//                        1=> Leakage of R->L
	//                        2=> Leakage of L->R
	//                        3=> Poln L
	//
	// The squint is removed in the following code using
	// honest-to-god pixel indexing. If this is not the most
	// efficient method of doing this in AIPS++ (i.e. instead use
	// slices etc.), then this cost will show up in making the
	// average PB.  Since this goes over each pixel of a full
	// stokes (poln. really) complex image, look here (also) for
	// optimization (if required).
	//
	if (!doSquint)
	  {
	    IPosition PolnRIndex(4,0,0,0,0), PolnLIndex(4,0,0,3,0);
	    IPosition tndx(4,0,0,0,0);
	    for(tndx(3)=0;tndx(3)<apertureShape(3);tndx(3)++)   // The freq. axis
	      for(tndx(2)=0;tndx(2)<apertureShape(2);tndx(2)++) // The Poln. axis
		for(tndx(1)=0;tndx(1)<apertureShape(1);tndx(1)++)   // The spatial
		  for(tndx(0)=0;tndx(0)<apertureShape(0);tndx(0)++) // axis.
		    {
		      PolnRIndex(0)=PolnLIndex(0)=tndx(0);
		      PolnRIndex(1)=PolnLIndex(1)=tndx(1);
		      Complex val, Rval, Lval;
		      Float phase;
		      val = ap.aperture->getAt(tndx);
		      Rval = ap.aperture->getAt(PolnRIndex);
		      Lval = ap.aperture->getAt(PolnLIndex);
		      phase = arg(Rval); Rval=Complex(cos(phase),sin(phase));
		      phase = arg(Lval); Lval=Complex(cos(phase),sin(phase));
		      
		      if      (tndx(2)==0) ap.aperture->putAt(val*conj(Rval),tndx);
		      else if (tndx(2)==1) ap.aperture->putAt(val*conj(Lval),tndx);
		      else if (tndx(2)==2) ap.aperture->putAt(val*conj(Rval),tndx);
		      else if (tndx(2)==3) ap.aperture->putAt(val*conj(Lval),tndx);
		      //	ap.aperture->putAt(Complex(val.real(),0.0),tndx);
		    }
	  }
	//    CoordinateSystem uvCoords;

	//     Matrix<Double> xform(2,2);                   
	//     xform = 0.0; xform.diagonal() = 1.0;     
	//     DirectionCoordinate dirCoord(MDirection::J2000,
	// 				 Projection(Projection::SIN),  
	// 				 135*C::pi/180.0, 60*C::pi/180.0,
	// 				 -1*C::pi/180.0, 1*C::pi/180,    
	// 				 xform,52, 52); 
	Vector<Int> poln(4);
	poln(0) = Stokes::RR;
	poln(1) = Stokes::RL;
	poln(2) = Stokes::LR;
	poln(3) = Stokes::LL;
	StokesCoordinate polnCoord(poln);
	SpectralCoordinate spectralCoord(MFrequency::TOPO,Freq,1.0,0.0);
	//    uvCoords.addCoordinate(dirCoord);
	index = uvCoords.findCoordinate(Coordinate::STOKES);
	uvCoords.replaceCoordinate(polnCoord,index);
	index = uvCoords.findCoordinate(Coordinate::SPECTRAL);
	uvCoords.replaceCoordinate(spectralCoord,index);
	
	//    uvCoords.addCoordinate(polnCoord);
	//    uvCoords.addCoordinate(spectralCoord);
	
	ap.aperture->setCoordinateInfo(uvCoords);

//     	String fn="aperture.im";
//     	storeImg(fn,*(ap.aperture));
//    	exit(0);
	//
	// Now FT the re-gridded Fourier plane to get the primary beam.
	//
	//    ftAperture(uvGrid);
	ftAperture(*(ap.aperture));
      }
//     else
//       cout << "Using pre-computed aperture for PA="<<ap.pa*180/M_PI <<endl;
  }
  void VLACalcIlluminationConvFunc::regridAperture(CoordinateSystem& skyCS,
						   IPosition& skyShape,
						   TempImage<Complex>& uvGrid,
						   const VisBuffer &vb,
						   const Vector<Float>& paList,
						   Bool doSquint, Int bandID)
  {
    //*****
      //    doSquint=True;
    CoordinateSystem skyCoords(skyCS);

    Float pa, Freq;
    if (bandID != -1) ap.band = (BeamCalcBandCode)bandID;
    AlwaysAssert(ap.band>=-1, AipsError);
    Vector<Double> chanFreq = vb.frequency();

    const ROMSSpWindowColumns& spwCol = vb.msColumns().spectralWindow();
    ROArrayColumn<Double> chanfreq = spwCol.chanFreq();
    ROScalarColumn<Double> reffreq = spwCol.refFrequency();
    Freq = sum(chanFreq)/chanFreq.nelements();
	
    IPosition imsize(skyShape);
    CoordinateSystem uvCoords = makeUVCoords(skyCoords,imsize);
	
    Int index = uvCoords.findCoordinate(Coordinate::LINEAR);
    LinearCoordinate lc=uvCoords.linearCoordinate(index);
    Vector<Double> incr = lc.increment();
    Double Lambda = C::c/Freq;
    ap.freq = Freq/1E9;
	
    ap.nx = skyShape(0); ap.ny = skyShape(1);
    IPosition apertureShape(ap.aperture->shape());
    apertureShape(0) = ap.nx;  apertureShape(1) = ap.ny;
    ap.aperture->resize(apertureShape);

    TempImage<Complex> tmpAperture;tmpAperture.resize(apertureShape);
    if (maximumCacheSize() > 0) tmpAperture.setMaximumCacheSize(maximumCacheSize());

    ap.dx = abs(incr(0)*Lambda); ap.dy = abs(incr(1)*Lambda);
    //	cout << ap.dx << " " << incr(0) << endl;
    //	ap.x0 = -(25.0/(2*ap.dx)+1)*ap.dx; ap.y0 = -(25.0/(2*ap.dy)+1)*ap.dy;
    // Following 3 lines go with the ANT tag in BeamCalc.cc
    //	Double antRadius=BeamCalcGeometryDefaults[ap.band].Rant;
    //	ap.x0 = -(antRadius/(ap.dx)+1)*ap.dx; 
    //	ap.y0 = -(antRadius/(ap.dy)+1)*ap.dy;
    // Following 2 lines go with the PIX tag in BeamCalc.cc
    ap.x0 = -(ap.nx/2)*ap.dx; 
    ap.y0 = -(ap.ny/2)*ap.dy;

    //
    // Accumulate apertures for a list of PA
    //
    for(uInt ipa=0;ipa<paList.nelements();ipa++)
      {
	pa = paList[ipa];
	//*****
	    //	pa=0;
	//	cerr << pa*57.2956 << "...";
	  

	ap.pa=pa;
	ap.aperture->set(0.0);
	calculateAperture(&ap);
	//	cout << "Calculate Aperture done" << endl;
	//
	// Set the phase of the aperture function to zero if doSquint==F
	// Poln. axis indices
        // 0: RR, 1:RL, 2:LR, 3:LL
	// This is electic field. 0=> Poln R, 
	//                        1=> Leakage of R->L
	//                        2=> Leakage of L->R
	//                        3=> Poln L
	//
	// The squint is removed in the following code using
	// honest-to-god pixel indexing. If this is not the most
	// efficient method of doing this in AIPS++ (i.e. instead use
	// slices etc.), then this cost will show up in making the
	// average PB.  Since this goes over each pixel of a full
	// stokes (poln. really) complex image, look here (also) for
	// optimization (if required).
	//
	//	cout << "Starting squint removal" << endl;
	if (!doSquint)
	  {
	    IPosition PolnRIndex(4,0,0,0,0), PolnLIndex(4,0,0,3,0);
	    IPosition tndx(4,0,0,0,0);
	    for(tndx(3)=0;tndx(3)<apertureShape(3);tndx(3)++)   // The freq. axis
	      for(tndx(2)=0;tndx(2)<apertureShape(2);tndx(2)++) // The Poln. axis
		for(tndx(1)=0;tndx(1)<apertureShape(1);tndx(1)++)   // The spatial
		  for(tndx(0)=0;tndx(0)<apertureShape(0);tndx(0)++) // axis.
		    {
		      PolnRIndex(0)=PolnLIndex(0)=tndx(0);
		      PolnRIndex(1)=PolnLIndex(1)=tndx(1);
		      Complex val, Rval, Lval;
		      Float phase;
		      val = ap.aperture->getAt(tndx);
		      Rval = ap.aperture->getAt(PolnRIndex);
		      Lval = ap.aperture->getAt(PolnLIndex);
		      phase = arg(Rval); Rval=Complex(cos(phase),sin(phase));
		      phase = arg(Lval); Lval=Complex(cos(phase),sin(phase));
		      
		      if      (tndx(2)==0) ap.aperture->putAt(val*conj(Rval),tndx);
		      else if (tndx(2)==1) ap.aperture->putAt(val*conj(Lval),tndx);
		      else if (tndx(2)==2) ap.aperture->putAt(val*conj(Rval),tndx);
		      else if (tndx(2)==3) ap.aperture->putAt(val*conj(Lval),tndx);
		      //	ap.aperture->putAt(Complex(val.real(),0.0),tndx);
		    }
	  }
	//	cout << "done" << endl;
	tmpAperture += *(ap.aperture);
	//*****
	    //	break;
      }
    *(ap.aperture) = tmpAperture;
    tmpAperture.resize(IPosition(1,1));//Release temp. store.
    Vector<Int> poln(4);
    poln(0) = Stokes::RR;
    poln(1) = Stokes::RL;
    poln(2) = Stokes::LR;
    poln(3) = Stokes::LL;
    StokesCoordinate polnCoord(poln);
    SpectralCoordinate spectralCoord(MFrequency::TOPO,Freq,1.0,0.0);
    //    uvCoords.addCoordinate(dirCoord);
    index = uvCoords.findCoordinate(Coordinate::STOKES);
    uvCoords.replaceCoordinate(polnCoord,index);
    index = uvCoords.findCoordinate(Coordinate::SPECTRAL);
    uvCoords.replaceCoordinate(spectralCoord,index);
	
    ap.aperture->setCoordinateInfo(uvCoords);
    
    //*****
	//    index=skyCoords.findCoordinate(Coordinate::STOKES);
	//    skyCoords.replaceCoordinate(polnCoord,index);
	//    ap.aperture->setCoordinateInfo(skyCoords);

//     String fn="aperture.im";
//     storeImg(fn,*(ap.aperture));
//     exit(0);
	//
	// Now FT the re-gridded Fourier plane to get the primary beam.
	//
	//    ftAperture(uvGrid);
    //    cout << "FFTing...." << endl;
    ftAperture(*(ap.aperture));
    //    cout << "done" << endl;
  }

  
  
  void VLACalcIlluminationConvFunc::fillPB(ImageInterface<Complex>& inImg,
					   ImageInterface<Complex>& outImg)
  {
    IPosition imsize(outImg.shape());
    IPosition ndx(outImg.shape());
    IPosition inShape(inImg.shape()),inNdx;
    
    cerr << inShape << " " << ndx << endl;
    
    Vector<Int> inStokes,outStokes;
    Int index,s;
    index = inImg.coordinates().findCoordinate(Coordinate::STOKES);
    inStokes = inImg.coordinates().stokesCoordinate(index).stokes();
    index = outImg.coordinates().findCoordinate(Coordinate::STOKES);
    outStokes = outImg.coordinates().stokesCoordinate(index).stokes();
    ndx(3)=0;
    for(ndx(2)=0;ndx(2)<imsize(2);ndx(2)++) // The poln axes
      {
	for(s=0;s<inShape(2);s++) if (inStokes(s) == outStokes(ndx(2))) break;
	
	for(ndx(0)=0;ndx(0)<imsize(0);ndx(0)++)
	  for(ndx(1)=0;ndx(1)<imsize(1);ndx(1)++)
	    {
	      Complex cval;
	      inNdx = ndx; inNdx(2)=s;
	      cval = inImg.getAt(inNdx);
	      outImg.putAt(cval*outImg.getAt(ndx),ndx);
	    }
      }
  }
  
  void VLACalcIlluminationConvFunc::fillPB(ImageInterface<Complex>& inImg,
					   ImageInterface<Float>& outImg)
  {
    IPosition imsize(outImg.shape());
    IPosition ndx(outImg.shape());
    IPosition inShape(inImg.shape()),inNdx;
    
    Vector<Int> inStokes,outStokes;
    Int index,s;
    index = inImg.coordinates().findCoordinate(Coordinate::STOKES);
    inStokes = inImg.coordinates().stokesCoordinate(index).stokes();
    index = outImg.coordinates().findCoordinate(Coordinate::STOKES);
    outStokes = outImg.coordinates().stokesCoordinate(index).stokes();
    ndx(3)=0;

    for(ndx(2)=0;ndx(2)<imsize(2);ndx(2)++) // The poln axes
      {
	if (outStokes(ndx(2)) == Stokes::I)
	  {
	    //
	    // Fill the outImg wiht (inImg(RR)+inImage(LL))/2 
	    //
	    for(ndx(0)=0;ndx(0)<imsize(0);ndx(0)++)
	      for(ndx(1)=0;ndx(1)<imsize(1);ndx(1)++)
		{
		  Complex cval;
		  inNdx = ndx; 
		  inNdx(2)=0; cval = inImg.getAt(inNdx);
		  inNdx(2)=3; cval += inImg.getAt(inNdx);
		  cval/2;
		  
		  outImg.putAt(abs(cval*outImg.getAt(ndx)),ndx);
		}
	  }
	else if ((outStokes(ndx(2)) == Stokes::RR) ||
		 (outStokes(ndx(2)) == Stokes::RL) ||
		 (outStokes(ndx(2)) == Stokes::LR) ||
		 (outStokes(ndx(2)) == Stokes::LL))
	  {
	    for(s=0;s<inShape(2);s++) if (inStokes(s) == outStokes(ndx(2))) break;
	
	    for(ndx(0)=0;ndx(0)<imsize(0);ndx(0)++)
	      for(ndx(1)=0;ndx(1)<imsize(1);ndx(1)++)
		{
		  Complex cval;
		  inNdx = ndx; inNdx(2)=s;
		  cval = inImg.getAt(inNdx);
		  outImg.putAt(abs(cval*outImg.getAt(ndx)),ndx);
		}
	  }
	else throw(AipsError("Unsupported Stokes found in VLACalcIlluminationConvFunc::fillPB."));
      }
  }
  
  /*
  void VLACalcIlluminationConvFunc::fillPB(ImageInterface<Complex>& inImg,
					   ImageInterface<Float>& outImg)
  {
    IPosition imsize(outImg.shape());
    IPosition ndx(outImg.shape());
    IPosition inShape(inImg.shape()), inNdx;
    Vector<Int> inStokes,outStokes;
    Int index;
    index = inImg.coordinates().findCoordinate(Coordinate::STOKES);
    inStokes = inImg.coordinates().stokesCoordinate(index).stokes();
    index = outImg.coordinates().findCoordinate(Coordinate::STOKES);
    outStokes = outImg.coordinates().stokesCoordinate(index).stokes();
    
    ndx(3)=0;
    for(ndx(0)=0;ndx(0)<imsize(0);ndx(0)++)
      {
	
	for(ndx(1)=0;ndx(1)<imsize(1);ndx(1)++)
	  {
	    //
	    // Average along the polarization axes and fillin the
	    // amp. of the average in the output image.
	    // 
	    Complex cval=0.0;
	    
 	    ndx(2)=0;cval = inImg.getAt(ndx);
 	    for(ndx(2)=0;ndx(2)<imsize(2);ndx(2)++) // The poln axes
 	      outImg.putAt(abs(cval*outImg.getAt(ndx)),ndx);
	  }
      }
  }
  
  */
  
  void VLACalcIlluminationConvFunc::ftAperture(TempImage<Complex>& uvgrid)
  {
    //
    // Make SkyJones
    //
    LatticeFFT::cfft2d(uvgrid);
//     TempImage<Complex> tmp(uvgrid);
//     IPosition shape(uvgrid.shape());
//     IPosition ndx(shape),ndx1,ndx2,ndx3;
//     ndx1 = ndx2 = ndx3 = ndx;
//     ndx(3)=ndx1(3)=ndx2(3) = ndx3(3) = 0;
//     ndx(2)=0;ndx1(2)=0; ndx2(2) = 1; ndx3(2)=1;
//     for(ndx(0)=ndx1(0)=0;ndx(0)<shape(0);ndx(0)++,ndx1(0)++)
//       for(ndx(1)=ndx1(1)=0;ndx(1)<shape(1);ndx(1)++,ndx1(1)++)
// 	tmp(ndx) = uvgrid(ndx)*conjg(uvgrid(ndx1);
//     for(ndx2(0)=ndx3(0)=0;ndx2(0)<shape(0);ndx2(0)++,ndx3(0)++)
//       for(ndx2(1)=ndx3(1)=0;ndx2(1)<shape(1);ndx2(1)++,ndx3(1)++)
// 	tmp(ndx2) = tmp(ndx2) uvgrid(ndx)*conjg(uvgrid(ndx1);

//      String fn("pJones.im");
//      storeImg(fn,uvgrid);
//      exit(0);
    //
    // Now make SkyMuller
    //
    skyMuller(uvgrid);
//      String fn("Muller.im");
//      storeImg(fn,uvgrid);
//      exit(0);
//     Array<Complex> buf=uvgrid.get();
//     buf *= (buf);
//     uvgrid.put(buf);
//    exit(0);    
  }
  
  void VLACalcIlluminationConvFunc::store(String& fileName){storeImg(fileName,convFunc_p);}
  
  void VLACalcIlluminationConvFunc::storeImg(String& fileName,ImageInterface<Complex>& theImg)
  {
    ostringstream reName,imName;
    reName << "re" << fileName;
    imName << "im" << fileName;
    PagedImage<Complex> ctmp(theImg.shape(), theImg.coordinates(), fileName);
    LatticeExpr<Complex> le(theImg);
    ctmp.copyData(le);
    {
      PagedImage<Float> tmp(theImg.shape(), theImg.coordinates(), reName);
      LatticeExpr<Float> le(abs(theImg));
      tmp.copyData(le);
    }
    {
      PagedImage<Float> tmp(theImg.shape(), theImg.coordinates(), imName);
      LatticeExpr<Float> le(arg(theImg));
      tmp.copyData(le);
    }
  }
  
  void VLACalcIlluminationConvFunc::storeImg(String& fileName,ImageInterface<Float>& theImg)
  {
    PagedImage<Float> tmp(theImg.shape(), theImg.coordinates(), fileName);
    LatticeExpr<Float> le(theImg);
    tmp.copyData(le);
  }
  
  void VLACalcIlluminationConvFunc::storePB(String& fileName)
  {
    {
      ostringstream Name;
      Name << "re" << fileName;
      IPosition newShape(convFunc_p.shape());
      newShape(0)=newShape(1)=200;
      PagedImage<Float> tmp(newShape, convFunc_p.coordinates(), Name);
      //    PagedImage<Float> tmp(convFunc_p.shape(), FTCoords, Name);
      LatticeExpr<Float> le(real(convFunc_p));
      tmp.copyData(le);
    }
    {
      ostringstream Name;
      Name << "im" << fileName;
      
      IPosition newShape(convFunc_p.shape());
      newShape(0)=newShape(1)=200;
      PagedImage<Float> tmp(newShape, convFunc_p.coordinates(), Name);
      //    PagedImage<Float> tmp(convFunc_p.shape(), FTCoords, Name);
      LatticeExpr<Float> le(imag(convFunc_p));
      tmp.copyData(le);
    }
  }
  void VLACalcIlluminationConvFunc::loadFromImage(String& fileName)
  {
  };

  void VLACalcIlluminationConvFunc::skyMuller(ImageInterface<Complex>& skyJones)
  {
    Array<Complex> buf=skyJones.get(),tmp;

    IPosition shape(buf.shape());
    IPosition sliceStart0(4,0,0,0,0),sliceStart1(4,0,0,0,0),
      sliceLength(4,shape(0),shape(1),1,1);
    //
    // Giving up on fancy slicing of arrays etc. (the commented code
    // below).  Just do pixel-by-pixel multiplications for of the
    // Jones planes. For now, computing only the diagonal of the
    // SkyMuller.
    //
    IPosition t(4,0,0,0,0),n0(4,0,0,0,0),n1(4,0,0,0,0);

    Float peak;
    peak=0;
    for(t(2)=0;t(2)<shape(2);t(2)++)
      for(t(1)=0;t(1)<shape(1);t(1)++)
	for(t(0)=0;t(0)<shape(0);t(0)++)
	  if (abs(buf(t)) > peak) peak = abs(buf(t));
    if (peak > 1E-8)
      for(t(3)=0;t(3)<shape(3);t(3)++) // Freq axis
	for(t(2)=0;t(2)<shape(2);t(2)++) // Poln axis
	  for(t(1)=0;t(1)<shape(1);t(1)++) // y axis
	    for(t(0)=0;t(0)<shape(0);t(0)++) // X axis
	      buf(t) = buf(t)/peak;

    tmp = buf;

    t(0)=t(1)=t(2)=t(3)=0;
    t(2)=0;n0(2)=0;n1(2)=0; //RR
    for(  n0(0)=n1(0)=t(0)=0;n0(0)<shape(0);n0(0)++,n1(0)++,t(0)++)
      for(n0(1)=n1(1)=t(1)=0;n0(1)<shape(1);n0(1)++,n1(1)++,t(1)++)
	buf(t) = (tmp(n0)*(tmp(n1)));

    t(2)=1;n0(2)=3;n1(2)=0; //LR
    for(  n0(0)=n1(0)=t(0)=0;n0(0)<shape(0);n0(0)++,n1(0)++,t(0)++)
      for(n0(1)=n1(1)=t(1)=0;n0(1)<shape(1);n0(1)++,n1(1)++,t(1)++)
	buf(t) = (tmp(n0)*conj(tmp(n1)));

    t(2)=2;n0(2)=0;n1(2)=3; //RL
    for(  n0(0)=n1(0)=t(0)=0;n0(0)<shape(0);n0(0)++,n1(0)++,t(0)++)
      for(n0(1)=n1(1)=t(1)=0;n0(1)<shape(1);n0(1)++,n1(1)++,t(1)++)
	buf(t) = (tmp(n0)*conj(tmp(n1)));

    t(2)=3;n0(2)=3;n1(2)=3; //LL
    for(  n0(0)=n1(0)=t(0)=0;n0(0)<shape(0);n0(0)++,n1(0)++,t(0)++)
      for(n0(1)=n1(1)=t(1)=0;n0(1)<shape(1);n0(1)++,n1(1)++,t(1)++)
	buf(t) = (tmp(n0)*(tmp(n1)));
    /*
    sliceStart0(3)=0; sliceStart1(3)=0;
    Slicer s0(sliceStart0,sliceLength),s1(sliceStart1,sliceLength);

    buf(s0) = tmp(s1);
    buf(s0) *= tmp(s1);
    //
    // Muller[1,1]
    //
    sliceStart0(3)=0; sliceStart1(3)=1;
    buf(Slicer(sliceStart0,sliceLength)) = tmp(Slicer(sliceStart0,sliceLength))
	*tmp(Slicer(sliceStart1,sliceLength));
    //
    // Muller[2,2]
    //
    sliceStart0(3)=1; sliceStart1(3)=0;
    buf(Slicer(sliceStart0,sliceLength)) = tmp(Slicer(sliceStart0,sliceLength))
	*tmp(Slicer(sliceStart1,sliceLength));
    //
    // Muller[3,3]
    //
    sliceStart0(3)=2; sliceStart1(3)=0;
    buf(Slicer(sliceStart0,sliceLength)) = tmp(Slicer(sliceStart0,sliceLength))
	*tmp(Slicer(sliceStart1,sliceLength));
	*/
    skyJones.put(buf);
  }

  Int getVLABandID(Double& freq,String&telescopeName)
  {
    if (telescopeName=="VLA")
      {
	if ((freq >=1.34E9) && (freq <=1.73E9))
	  return BeamCalc_VLA_L;
	else if ((freq >=4.5E9) && (freq <=5.0E9))
	  return BeamCalc_VLA_C;
	else if ((freq >=8.0E9) && (freq <=8.8E9))
	  return BeamCalc_VLA_X;
	else if ((freq >=14.4E9) && (freq <=15.4E9))
	  return BeamCalc_VLA_U;
	else if ((freq >=22.0E9) && (freq <=24.0E9))
	  return BeamCalc_VLA_K;
	else if ((freq >=40.0E9) && (freq <=50.0E9))
	  return BeamCalc_VLA_Q;
	else if ((freq >=100E6) && (freq <=300E6))
	  return BeamCalc_VLA_4;
      }
    else if (telescopeName=="EVLA")
      {
	if ((freq >=1.0E9) && (freq <=2.0E9))
	  return BeamCalc_EVLA_L;
	else if ((freq >=2.0E9) && (freq <=4.0E9))
	  return BeamCalc_EVLA_S;
	else if ((freq >=4.0E9) && (freq <=8.0E9))
	  return BeamCalc_EVLA_C;
	else if ((freq >=8.0E9) && (freq <=12.0E9))
	  return BeamCalc_EVLA_X;
	else if ((freq >=12.0E9) && (freq <=18.0E9))
	  return BeamCalc_EVLA_U;
	else if ((freq >=18.0E9) && (freq <=26.5E9))
	  return BeamCalc_EVLA_K;
	else if ((freq >=26.5E9) && (freq <=40.8E9))
	  return BeamCalc_EVLA_K;
	else if ((freq >=4.0E9) && (freq <=50.0E9))
	  return BeamCalc_EVLA_Q;
      }
    ostringstream mesg;
    mesg << telescopeName << "/" << freq << "(Hz) combination not recognized.";
    throw(SynthesisError(mesg.str()));
  }

};
