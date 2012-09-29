//# Feather.cc: Implementation of Feather.h
//# Copyright (C) 1997-2012
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#include <synthesis/MeasurementEquations/Feather.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <casa/OS/HostInfo.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/iostream.h>
#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <scimath/Mathematics/MathFunc.h>
#include <tables/Tables/ExprNode.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slice.h>
#include <components/ComponentModels/GaussianBeam.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageRegrid.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/Image2DConvolver.h>
#include <synthesis/TransformMachines/PBMath.h>
#include <lattices/Lattices/LatticeExpr.h> 
#include <lattices/Lattices/LatticeFFT.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <coordinates/Coordinates/ObsInfo.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>

namespace casa { //# NAMESPACE CASA - BEGIN


  void Feather::applyDishDiam(ImageInterface<Complex>& image, GaussianBeam& beam, Float effDiam, ImageInterface<Float>& fftim, Vector<Quantity>& extraconv){
    /*
    MathFunc<Float> dd(SPHEROIDAL);
    Vector<Float> valsph(31);
    for(Int k=0; k <31; ++k){
      valsph(k)=dd.value((Float)(k)/10.0);
    }
    Quantity fulrad((52.3/2.0*3.0/1.1117),"arcsec");
    Quantity lefreq(224.0/effDiam*9.0, "GHz");
    
    PBMath1DNumeric elpb(valsph, fulrad, lefreq);*/
    ////////////////////////
    /*{
      PagedImage<Float> thisScreen(image.shape(), image.coordinates(), "Before_apply.image");
      LatticeExpr<Float> le(abs(image));
      thisScreen.copyData(le);
      }*/
    //////////////////////
    
    Double freq  = worldFreq(image.coordinates(), 0);
    //cerr << "Freq " << freq << endl;
    Quantity halfpb=Quantity(1.22*C::c/freq/effDiam, "rad");
    //PBMath1DAiry elpb( Quantity(effDiam,"m"), Quantity(0.01,"m"),
    //				       Quantity(0.8564,"deg"), Quantity(1.0,"GHz"));
    GaussianBeam newBeam(halfpb, halfpb, Quantity(0.0, "deg"));
    try {
      
      GaussianBeam toBeUsed;
      //cerr << "beam " << beam.toVector() << endl;
      //cerr << "newBeam " << newBeam.toVector() << endl;
      beam.deconvolve(toBeUsed, newBeam);
      extraconv.resize(3);
      // use the Major difference
      extraconv(0) = toBeUsed.getMajor();
      extraconv(1) = toBeUsed.getMinor();
      extraconv(2) = toBeUsed.getPA();

    }
    catch (const AipsError& x) {
      throw(AipsError("Beam due to new effective diameter may be smaller than the beam of original dish image"));
    }
    //////////////////////
    //1 GHz equivalent   
    //    halfpb=Quantity(1.22*C::c/1.0e9/effDiam, "rad");
    //cerr << "halfpb " << halfpb << endl;
    //PBMath1DGauss elpb(halfpb, Quantity(0.8564,"deg"), Quantity(1.0,"GHz"), True);
    
    fftim.set(0.0);
   
    IPosition center(4, Int((fftim.shape()(0)/4)*2), 
		     Int((fftim.shape()(1)/4)*2),0,0);
    fftim.putAt(1.0, center);
    StokesImageUtil::Convolve(fftim, newBeam, False);
    StokesImageUtil::From(image, fftim);
    /*
    TempImage<Complex> elbeamo(image.shape(), image.coordinates());
    elbeamo.set(1.0);
    MDirection wcenter;  
    {
      Int directionIndex=
	image.coordinates().findCoordinate(Coordinate::DIRECTION);
      DirectionCoordinate
	directionCoord=image.coordinates().directionCoordinate(directionIndex);
      Vector<Double> pcenter(2);
      pcenter(0) = image.shape()(directionIndex)/2;
      pcenter(1) = image.shape()(directionIndex+1)/2;    
      directionCoord.toWorld( wcenter, pcenter );
    }
    elpb.applyPB(elbeamo, elbeamo, wcenter, Quantity(0.0, "deg"), 
			   BeamSquint::NONE);
    
   
    StokesImageUtil::To(fftim, elbeamo);
    */
    ///////////////////
    //StokesImageUtil::FitGaussianPSF(fftim, 
    //    			   beam);
     //cerr << "To be convolved beam 2 " << beam.toVector() << endl;
     ////////////////

    LatticeFFT::cfft2d(image);
    //image.copyData((LatticeExpr<Complex>)(elbeamo) );
    //elbeamo.copyData(image);
    // LatticeFFT::cfft2d(elbeamo, False);
    //StokesImageUtil::To(fftim, elbeamo);
    //StokesImageUtil::FitGaussianPSF(fftim, 
    //				    beam);
    /////////
    /*{
      PagedImage<Float> thisScreen(image.shape(), image.coordinates(), "After_apply.image");
      //LatticeExpr<Float> le(abs(image));
      thisScreen.copyData(fftim);
      }*/ 
    ////////
  }


  void Feather::feather(const String& image, const ImageInterface<Float>& high, const ImageInterface<Float>& low0, const Float& sdScale, const String& lowPSF, const Bool useDefaultPB, const String& vpTableStr, Float effDiam, const Bool doPlot){

    LogIO os(LogOrigin("Feather", "feather()", WHERE));
   try {

   




      GaussianBeam hBeam , lBeam;
      Vector<Quantity> extraconv;
      ImageInfo highInfo=high.imageInfo();
      hBeam=highInfo.restoringBeam();
      ImageInfo lowInfo=low0.imageInfo();
      lBeam=lowInfo.restoringBeam();
      if(hBeam.isNull()) {
	os << LogIO::WARN 
	   << "High resolution image does not have any resolution information - will be unable to scale correctly.\n" 
	   << LogIO::POST;
      }
      
      PBMath * myPBp = 0;
      if((lowPSF=="") && lBeam.isNull()) {
	// create the low res's PBMath object, needed to apply PB 
	// to make high res Fourier weight image
	if (useDefaultPB) {
	  // look up the telescope in ObsInfo
	  ObsInfo oi = low0.coordinates().obsInfo();
	  String myTelescope = oi.telescope();
	  if (myTelescope == "") {
	    os << LogIO::SEVERE << "No telescope imbedded in low res image" 
	       << LogIO::POST;
	    os << LogIO::SEVERE << "Create a PB description with the vpmanager"
	       << LogIO::EXCEPTION;
	  }
	  Quantity qFreq;
	  {
	    Double freq  = worldFreq(low0.coordinates(), 0);
	    qFreq = Quantity( freq, "Hz" );
	  }
	  String band;
	  PBMath::CommonPB whichPB;
	  String pbName;
	  // get freq from coordinates
	  PBMath::whichCommonPBtoUse (myTelescope, qFreq, band, whichPB, 
				      pbName);
	  if (whichPB  == PBMath::UNKNOWN) {
	    os << LogIO::SEVERE << "Unknown telescope for PB type: " 
	       << myTelescope << LogIO::EXCEPTION;
	  }
	  myPBp = new PBMath(whichPB);
	} else {
	  // get the PB from the vpTable
	  Table vpTable( vpTableStr );
	  ROScalarColumn<TableRecord> recCol(vpTable, (String)"pbdescription");
	  myPBp = new PBMath(recCol(0));
	}
	AlwaysAssert((myPBp != 0), AipsError);
      }

      // regrid the single dish image
      TempImage<Float> low(high.shape(), high.coordinates());
      {
	IPosition axes(2,0,1);
	if(high.shape().nelements() >2){
	  Int spectralAxisIndex=high.coordinates().
	    findCoordinate(Coordinate::SPECTRAL);
	  if(spectralAxisIndex > -1){
	    axes.resize(3);
	    axes(0)=0;
	    axes(1)=1;
	    axes(2)=spectralAxisIndex+1;
	  }
	}
	ImageRegrid<Float> ir;
	ir.regrid(low, Interpolate2D::LINEAR, axes, low0);
      }
    
      // get image center direction (needed for SD PB, which is needed for
      // the high res Fourier weight image
      MDirection wcenter;  
      {
	Int directionIndex=
	  high.coordinates().findCoordinate(Coordinate::DIRECTION);
	AlwaysAssert(directionIndex>=0, AipsError);
	DirectionCoordinate
	  directionCoord=high.coordinates().directionCoordinate(directionIndex);
	Vector<Double> pcenter(2);
	pcenter(0) = high.shape()(0)/2;
	pcenter(1) = high.shape()(1)/2;    
	directionCoord.toWorld( wcenter, pcenter );
      }
      
      // make the weight image for high res Fourier plane:  1 - normalized(FT(sd_PB))
      IPosition myshap(high.shape());
      for( uInt k=2; k< myshap.nelements(); ++k){
	myshap(k)=1;
      }
      
      TempImage<Float> lowpsf0;
      TempImage<Complex> cweight(myshap, high.coordinates());
      if(lowPSF=="") {
	os << LogIO::NORMAL // Loglevel INFO
           << "Using primary beam to determine weighting.\n" << LogIO::POST;
	if(lBeam.isNull()) {
	  cweight.set(1.0);
	  if (myPBp != 0) {
	    myPBp->applyPB(cweight, cweight, wcenter, Quantity(0.0, "deg"), 
			   BeamSquint::NONE);
	  
	    lowpsf0=TempImage<Float>(cweight.shape(), cweight.coordinates());
	    
	    os << LogIO::NORMAL // Loglevel INFO
               << "Determining scaling from SD Primary Beam.\n"
	       << LogIO::POST;
	    StokesImageUtil::To(lowpsf0, cweight);
	    StokesImageUtil::FitGaussianPSF(lowpsf0, 
					    lBeam);
	  }
	  delete myPBp;
	}
	else{
	  os << LogIO::NORMAL // Loglevel INFO
             << "Determining scaling from SD restoring beam.\n"
	     << LogIO::POST;
	  lowpsf0=TempImage<Float>(cweight.shape(), cweight.coordinates());
	  lowpsf0.set(0.0);
	  IPosition center(4, Int((cweight.shape()(0)/4)*2), 
			   Int((cweight.shape()(1)/4)*2),0,0);
	  lowpsf0.putAt(1.0, center);
	  StokesImageUtil::Convolve(lowpsf0, lBeam, False);
	  StokesImageUtil::From(cweight, lowpsf0);

	}
      }
      else {
	os << LogIO::NORMAL // Loglevel INFO
           << "Using specified low resolution PSF to determine weighting.\n" 
	   << LogIO::POST;
	// regrid the single dish psf
	PagedImage<Float> lowpsfDisk(lowPSF);
	IPosition lshape(lowpsfDisk.shape());
	lshape.resize(4);
	lshape(2)=1; lshape(3)=1;
	TempImage<Float>lowpsf(lshape,lowpsfDisk.coordinates());
	IPosition blc(lowpsfDisk.shape());
	IPosition trc(lowpsfDisk.shape());
	blc(0)=0; blc(1)=0;
	trc(0)=lowpsfDisk.shape()(0)-1;
	trc(1)=lowpsfDisk.shape()(1)-1;
	for( uInt k=2; k < lowpsfDisk.shape().nelements(); ++k){
	  blc(k)=0; trc(k)=0;	  	  
	}// taking first plane
	Slicer sl(blc, trc, Slicer::endIsLast);
	lowpsf.copyData(SubImage<Float>(lowpsfDisk, sl, False));
	lowpsf0=TempImage<Float> (myshap, high.coordinates());
	{
	  ImageRegrid<Float> ir;
	  IPosition axes(2,0,1);   // if its a cube, regrid the spectral too
	  ir.regrid(lowpsf0, Interpolate2D::LINEAR, axes, lowpsf);
	}
	if(lBeam.isNull()) {
	  os << LogIO::NORMAL // Loglevel INFO
             << "Determining scaling from low resolution PSF.\n" << LogIO::POST;
	  StokesImageUtil::FitGaussianPSF(lowpsf, lBeam);
	}
	StokesImageUtil::From(cweight, lowpsf0);
      }
 
      LatticeFFT::cfft2d( cweight );
      if(effDiam >0.0){
	//cerr << "in effdiam" << effDiam << endl;
	applyDishDiam(cweight, lBeam, effDiam, lowpsf0, extraconv);
	lowpsf0.copyData((LatticeExpr<Float>)(lowpsf0/max(lowpsf0)));
	StokesImageUtil::FitGaussianPSF(lowpsf0, lBeam);
	
	Int directionIndex=
	  cweight.coordinates().findCoordinate(Coordinate::DIRECTION);
	Image2DConvolver<Float>::convolve(
		    os, low, low, VectorKernel::toKernelType("gauss"), IPosition(2, directionIndex, directionIndex+1),
		    extraconv, True, 1.0, True
		    );

      }
      LatticeExprNode node = max( cweight );
      Float fmax = abs(node.getComplex());
      cweight.copyData(  (LatticeExpr<Complex>)( 1.0f - cweight/fmax ) );
      //Plotting part
      if(doPlot){
	CoordinateSystem ftCoords(cweight.coordinates());
	Double freq=worldFreq(ftCoords, 0);
	////
	Int directionIndex=ftCoords.findCoordinate(Coordinate::DIRECTION);
	Array<Complex> tmpval;
	IPosition start=cweight.shape();
	IPosition shape=cweight.shape()/2;
	for(uInt k=0; k < shape.nelements(); ++k){
	  start[k]=0;
	  if(k != uInt(directionIndex+1))
	    shape[k]=1;
	}
	start[directionIndex+1]=cweight.shape()[directionIndex+1]/2;
	start[directionIndex]=cweight.shape()[directionIndex]/2;
	cweight.getSlice(tmpval, start, shape, True);
	Vector<Float> x=amplitude(tmpval);
	Vector<Float> xdish=(Float(1.0) - x)*Float(sdScale);
	tmpval.resize();
	shape=cweight.shape()/2;
	for(uInt k=0; k < shape.nelements(); ++k){
	  start[k]=0;
	  if(k != uInt(directionIndex))
	    shape[k]=1;
	}
	start[directionIndex+1]=cweight.shape()[directionIndex+1]/2;
	start[directionIndex]=cweight.shape()[directionIndex]/2;
	cweight.getSlice(tmpval, start, shape, True);
	Vector<Float> y=amplitude(tmpval);
	Vector<Float> ydish=(Float(1.0)-y)*Float(sdScale);
	DirectionCoordinate dc=ftCoords.directionCoordinate(directionIndex);
	Vector<Bool> axes(2); axes(0)=True;axes(1)=True;
	Vector<Int> elshape(2); 
	elshape(0)=cweight.shape()[directionIndex];
	elshape(1)=cweight.shape()[directionIndex+1];
	Coordinate* ftdc=dc.makeFourierCoordinate(axes,elshape);	
	Vector<Double> xpix(x.nelements());
	indgen(xpix);
	xpix +=Double(cweight.shape()[directionIndex])/2.0;
	Matrix<Double> pix(2, xpix.nelements());
	Matrix<Double> world(2, xpix.nelements());
	Vector<Bool> failures;
	pix.row(1)=elshape(0)/2;
	pix.row(0)=xpix;
	ftdc->toWorldMany(world, pix, failures);
	xpix=world.row(0);
	//cerr << "xpix " << xpix << endl;
	xpix=fabs(xpix)*(C::c)/freq;
	Vector<Double> ypix(y.nelements());
	indgen(ypix);
	ypix +=Double(cweight.shape()[directionIndex+1])/2.0;
	pix.resize(2, ypix.nelements());
	world.resize();
	pix.row(1)=ypix;
	pix.row(0)=elshape(1)/2;
	ftdc->toWorldMany(world, pix, failures);
	ypix=world.row(1);
	ypix=fabs(ypix)*(C::c/freq);
	PlotServerProxy *plotter = dbus::launch<PlotServerProxy>( );
	dbus::variant panel_id = plotter->panel( "Feather function slice cuts", "Distance in m", "Amp", "Feather",
					       std::vector<int>( ), "right");

	if ( panel_id.type( ) != dbus::variant::INT ) {
	  os << LogIO::SEVERE << "failed to start plotter" << LogIO::POST;
	  return ;
	}

      
	plotter->line(dbus::af(xpix),dbus::af(x),"blue","x-interferometer weight",panel_id.getInt( ));
	plotter->line(dbus::af(xpix),dbus::af(xdish),"cyan","x-singledish weight",panel_id.getInt( ));
	
	plotter->line(dbus::af(ypix),dbus::af(y),"green","y-interferometer weight",panel_id.getInt( ));
	plotter->line(dbus::af(ypix),dbus::af(ydish),"purple","y-singledish weight",panel_id.getInt( ));

      }
      //End plotting part
      // FT high res image
      TempImage<Complex> cimagehigh(high.shape(), high.coordinates() );
      StokesImageUtil::From(cimagehigh, high);
      LatticeFFT::cfft2d( cimagehigh );
      
      // FT low res image
      TempImage<Complex> cimagelow(high.shape(), high.coordinates() );
      StokesImageUtil::From(cimagelow, low);
      LatticeFFT::cfft2d( cimagelow );


      // This factor comes from the beam volumes
      if(sdScale != 1.0)
        os << LogIO::NORMAL // Loglevel INFO
           << "Multiplying single dish data by user specified factor "
           << sdScale << ".\n" << LogIO::POST;
      Float sdScaling  = sdScale;
      if (! hBeam.isNull() && ! lBeam.isNull()) {

	Float beamFactor=
	  hBeam.getArea("arcsec2")/lBeam.getArea("arcsec2");
	os << LogIO::NORMAL // Loglevel INFO
           << "Applying additional scaling for ratio of the volumes of the high to the low resolution images : "
	   <<  beamFactor << ".\n" << LogIO::POST;
	sdScaling*=beamFactor;
      }
      else {
	os << LogIO::WARN << "Insufficient information to scale correctly.\n" 
	   << LogIO::POST;
      }

      // combine high and low res, appropriately normalized, in Fourier
      // plane. The vital point to remember is that cimagelow is already
      // multiplied by 1-cweight so we only need adjust for the ratio of beam
      // volumes
      Vector<Int> extraAxes(cimagehigh.shape().nelements()-2);
      if(extraAxes.nelements() > 0){
	
	if(extraAxes.nelements() ==2){
	  Int n3=cimagehigh.shape()(2);
	  Int n4=cimagehigh.shape()(3);
	  IPosition blc(cimagehigh.shape());
	  IPosition trc(cimagehigh.shape());
	  blc(0)=0; blc(1)=0;
	  trc(0)=cimagehigh.shape()(0)-1;
	  trc(1)=cimagehigh.shape()(1)-1;
	  for (Int j=0; j < n3; ++j){
	    for (Int k=0; k < n4 ; ++k){
	      blc(2)=j; trc(2)=j;
	      blc(3)=k; trc(3)=k;
	      Slicer sl(blc, trc, Slicer::endIsLast);
	      SubImage<Complex> cimagehighSub(cimagehigh, sl, True);
	      SubImage<Complex> cimagelowSub(cimagelow, sl, True);
	      cimagehighSub.copyData(  (LatticeExpr<Complex>)((cimagehighSub * cweight + cimagelowSub * sdScaling)));
	    }
	  }
	}
      }
      else{
	cimagehigh.copyData(  
			    (LatticeExpr<Complex>)((cimagehigh * cweight 
						    + cimagelow * sdScaling)));
      }
      // FT back to image plane
      LatticeFFT::cfft2d( cimagehigh, False);
    
      // write to output image
      PagedImage<Float> featherImage(high.shape(), high.coordinates(), image );
      StokesImageUtil::To(featherImage, cimagehigh);
      ImageUtilities::copyMiscellaneous(featherImage, high);

      try{
      { // write data processing history into image logtable
	LoggerHolder imagelog (False);
	LogSink& sink = imagelog.sink();
	LogOrigin lor(String("Feather"), String("feather()"));
	LogMessage msg(lor);
	ostringstream oos;
	oos << endl << "Imager::feather() input paramaters:" << endl
	    << "Feathered image =      '" << image   << "'" << endl
	    << "High resolution image ='" << high.name() << "'" << endl
	    << "Low resolution image = '" << low0.name()  << "'" << endl
	    << "Low resolution PSF =   '" << lowPSF  << "'" << endl << endl;
	String inputs(oos);
	sink.postLocally(msg.message(inputs));
	imagelog.flush();

	LoggerHolder& log = featherImage.logger();
	log.append(imagelog);
	log.flush();
      }
      }catch(exception& x){

	os << LogIO::WARN << "Caught exception: " << x.what()
       << LogIO::POST;
	

      }
      catch(...){
	os << LogIO::SEVERE << "Unknown exception handled" << LogIO::POST;

      }
   







   }catch(exception& x){

	os << LogIO::WARN << "Caught exception: " << x.what()
	   << LogIO::POST;  
   }
    


  }

  Double Feather::worldFreq(const CoordinateSystem& cs, Int spectralpix){
    ///freq part
    Int spectralIndex=cs.findCoordinate(Coordinate::SPECTRAL);
    SpectralCoordinate
      spectralCoord=
      cs.spectralCoordinate(spectralIndex);
    Vector<String> units(1); units = "Hz";
    spectralCoord.setWorldAxisUnits(units);	
    Vector<Double> spectralWorld(1);
    Vector<Double> spectralPixel(1);
    spectralPixel(0) = spectralpix;
    spectralCoord.toWorld(spectralWorld, spectralPixel);  
    Double freq  = spectralWorld(0);
    return freq;
  }





}//# NAMESPACE CASA - END
