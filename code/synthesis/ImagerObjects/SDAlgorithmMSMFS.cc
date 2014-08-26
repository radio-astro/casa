//# SDAlgorithmMSMFS.cc: Implementation of SDAlgorithmMSMFS classes
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/HostInfo.h>
#include <synthesis/ImagerObjects/SDAlgorithmMSMFS.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentList.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <casa/OS/File.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Directory.h>
#include <tables/Tables/TableLock.h>

#include<synthesis/ImagerObjects/SIMinorCycleController.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>

#include <casa/System/Choice.h>
#include <msvis/MSVis/StokesVector.h>


namespace casa { //# NAMESPACE CASA - BEGIN




  SDAlgorithmMSMFS::SDAlgorithmMSMFS( uInt nTaylorTerms, Vector<Float> scalesizes ):
    SDAlgorithmBase(),
    //    itsImages(),
    itsMatPsfs(), itsMatResiduals(), itsMatModels(),
    itsNTerms(nTaylorTerms),
    itsScaleSizes(scalesizes),
    itsMTCleaner(),
    itsMTCsetup(False)
 {
   itsAlgorithmName=String("mtmfs");
   if( itsScaleSizes.nelements()==0 ){ itsScaleSizes.resize(1); itsScaleSizes[0]=0.0; }
 }

  SDAlgorithmMSMFS::~SDAlgorithmMSMFS()
 {
   
 }

 
  void SDAlgorithmMSMFS::initializeDeconvolver( Float &peakresidual, Float &modelflux )
  {
    LogIO os( LogOrigin("SDAlgorithmMSMFS","initializeDeconvolver",WHERE) );

    AlwaysAssert( !itsImages.null(), AipsError );
    AlwaysAssert( itsNTerms == itsImages->getNTaylorTerms() , AipsError );

    itsMatPsfs.resize( 2*itsNTerms-1 );
    itsMatResiduals.resize( itsNTerms );
    itsMatModels.resize( itsNTerms );

    ////  Why is this needed ?  I hope this is by reference.
    for(uInt tix=0; tix<2*itsNTerms-1; tix++)
      {
	if(tix<itsNTerms)
	  {
	    (itsImages->residual(tix))->get( itsMatResiduals[tix], True );
	    (itsImages->model(tix))->get( itsMatModels[tix], True );
	  }
	(itsImages->psf(tix))->get( itsMatPsfs[tix], True );
      }


    //// Initialize the MultiTermMatrixCleaner.

    ///  ----------- do once ----------
    if( itsMTCsetup == False)
      {
	//cout << "Setting up the MT Cleaner once" << endl;
	//Vector<Float> scalesizes(1); scalesizes[0]=0.0;
	itsMTCleaner.setscales( itsScaleSizes );
	itsMTCleaner.setntaylorterms( itsNTerms );
	itsMTCleaner.initialise( itsImages->getShape()[0], itsImages->getShape()[1] );
	
	for(uInt tix=0; tix<2*itsNTerms-1; tix++)
	  {
	    Matrix<Float> tempMat;
	    tempMat.reference( itsMatPsfs[tix] );
	    itsMTCleaner.setpsf( tix, tempMat );
	    ///	itsMTCleaner.setpsf( tix, itsMatPsfs[tix] );
	  }
	itsMTCsetup=True;
      }
    /// -----------------------------------------

    itsImages->mask()->get( itsMatMask, True );
    //cout << "Mask in SDAlgoMSMFS : " << sum( itsMatMask ) << endl;

    /*
    if( sum( itsMatMask )==0 ) 
      {
	os << LogIO::WARN << "ZERO MASK. Forcing all pixels to 1.0" << LogIO::POST; 
	itsMatMask = 1.0; 
      }
    */

    /// Find initial max vals..
    findMaxAbsMask( itsMatResiduals[0], itsMatMask, itsPeakResidual, itsMaxPos );
    itsModelFlux = sum( itsMatModels[0] );

    peakresidual = itsPeakResidual;
    modelflux = itsModelFlux;


    // Parts to be repeated at each minor cycle start....

    Matrix<Float> tempmask(itsMatMask);
    itsMTCleaner.setmask( tempmask );

    for(uInt tix=0; tix<itsNTerms; tix++)
      {
	Matrix<Float> tempMat;
	tempMat.reference( itsMatResiduals[tix] );
	itsMTCleaner.setresidual( tix, tempMat );
	//	itsMTCleaner.setresidual( tix, itsMatResiduals[tix] );

	Matrix<Float> tempMat2;
	tempMat2.reference( itsMatModels[tix] );
	itsMTCleaner.setmodel( tix, tempMat2 );
	//	itsMTCleaner.setmodel( tix, itsMatModels[tix] );
      }


  }

  void SDAlgorithmMSMFS::takeOneStep( Float loopgain, Int cycleNiter, Float cycleThreshold, Float &peakresidual, Float &modelflux, Int &iterdone)
  {

    iterdone = itsMTCleaner.mtclean( cycleNiter, 0.0, loopgain, cycleThreshold );

    if( iterdone==-2 ) throw(AipsError("MT-Cleaner error : Non-invertible Hessian "));

    for(uInt tix=0; tix<itsNTerms; tix++)
      {
	Matrix<Float> tempMat;
	tempMat.reference( itsMatModels[tix] );

	itsMTCleaner.getmodel( tix, tempMat ); //itsMatModels[tix] );
      }
 
    /////////////////
    //    findMaxAbs( itsMatResiduals[0], itsPeakResidual, itsMaxPos );
    //    peakresidual = itsPeakResidual;

    peakresidual = itsMTCleaner.getpeakresidual();

    modelflux = sum( itsMatModels[0] ); // Performance hog ?
  }	    

  void SDAlgorithmMSMFS::finalizeDeconvolver()
  {
    //    itsResidual.put( itsMatResidual );
    //    itsModel.put( itsMatModel );

    // Why is this needed ?  If the matrices are by reference, then why do we need this ? 
    for(uInt tix=0; tix<itsNTerms; tix++)
      {
	(itsImages->residual(tix))->put( itsMatResiduals[tix] );
	(itsImages->model(tix))->put( itsMatModels[tix] );
      }

  }


  
   void SDAlgorithmMSMFS::restore(CountedPtr<SIImageStore> imagestore )
  {

    LogIO os( LogOrigin("SDAlgorithmMSMFS","restore",WHERE) );

    // Compute principal solution ( if it hasn't already been done to this ImageStore......  )
    //////  Put some image misc info in here, to say if it has been done or not. 

    for(uInt tix=0; tix<itsNTerms; tix++)
      {
	Array<Float> tempArr;
	(imagestore->residual(tix))->get( tempArr, True );	
	Matrix<Float> tempMat;
	tempMat.reference( tempArr );
	itsMTCleaner.setresidual( tix, tempMat );
      }

    itsMTCleaner.computeprincipalsolution();

    for(uInt tix=0; tix<itsNTerms; tix++)
      {
	Matrix<Float> tempRes;
	itsMTCleaner.getresidual(tix,tempRes);
	(itsImages->residual(tix))->put( tempRes );
      }
    

    SDAlgorithmBase::restore( imagestore );

  }


  /*
  void SDAlgorithmMSMFS::restorePlane()
  {

    LogIO os( LogOrigin("SDAlgorithmMSMFS","restorePlane",WHERE) );

    try
      {
	// Fit a Gaussian to the PSF.
	GaussianBeam beam = itsImages->getPSFGaussian();

	os << "Restore with beam : " 
	   << beam.getMajor(Unit("arcmin")) << " arcmin, " 
	   << beam.getMinor(Unit("arcmin"))<< " arcmin, " 
	   << beam.getPA(Unit("deg")) << " deg" << LogIO::POST; 
	
	// Compute principal solution ( if it hasn't already been done to this ImageStore......  )
	itsMTCleaner.computeprincipalsolution();
	for(uInt tix=0; tix<itsNTerms; tix++)
	  {
	    Matrix<Float> tempRes;
	    itsMTCleaner.getresidual(tix,tempRes);
	    (itsImages->residual(tix))->put( tempRes );
	  }

	// Restore all terms
	ImageInfo ii = itsImages->image(0)->imageInfo();
	ii.setRestoringBeam( beam );

	for(uInt tix=0; tix<itsNTerms; tix++)
	  {
	    (itsImages->image(tix))->set(0.0);
	    (itsImages->image(tix))->copyData( LatticeExpr<Float>(*(itsImages->model(tix))) );
	    StokesImageUtil::Convolve( *(itsImages->image(tix)) , beam);
	    (itsImages->image(tix))->copyData( LatticeExpr<Float>
					       ( *(itsImages->model(tix)) + *(itsImages->residual(tix)) )   );
	    itsImages->image()->setImageInfo(ii);
	  }	
	
	// Calculate alpha and beta
	LatticeExprNode leMaxRes = max( *( itsImages->residual(0) ) );
	Float maxres = leMaxRes.getFloat();
	Float specthreshold = maxres/5.0;  //////////// do something better here..... 

      os << "Calculating spectral parameters for  Intensity > peakresidual/5 = " << specthreshold << " Jy/beam" << LogIO::POST;
      LatticeExpr<Float> mask1(iif(((*(itsImages->image(0))))>(specthreshold),1.0,0.0));
      LatticeExpr<Float> mask0(iif(((*(itsImages->image(0))))>(specthreshold),0.0,1.0));

      /////// Calculate alpha
      LatticeExpr<Float> alphacalc( (((*(itsImages->image(1))))*mask1)/(((*(itsImages->image(0))))+(mask0)) );
      itsImages->alpha()->copyData(alphacalc);

      // Set the restoring beam for alpha
      itsImages->alpha()->setImageInfo(ii);
      //      itsImages->alpha()->table().unmarkForDelete();

      // Make a mask for the alpha image
      LatticeExpr<Bool> lemask(iif(((*(itsImages->image(0))) > specthreshold) , True, False));

      createMask(lemask, *(itsImages->alpha()));

      }
    catch(AipsError &x)
      {
	throw( AipsError("Restoration Error : " + x.getMesg() ) );
      }

  }


Bool SDAlgorithmMSMFS::createMask(LatticeExpr<Bool> &lemask, ImageInterface<Float> &outimage)
{
      ImageRegion outreg = outimage.makeMask("mask0",False,True);
      LCRegion& outmask=outreg.asMask();
      outmask.copyData(lemask);
      outimage.defineRegion("mask0",outreg, RegionHandler::Masks, True);
      outimage.setDefaultMask("mask0");
      return True;
}

*/

  // Use this decide how to partition
  // the image for separate calls to 'deconvolve'.
  void SDAlgorithmMSMFS::queryDesiredShape(Bool &onechan, Bool &onepol) // , nImageFacets.
  {  
    onechan = True;
    onepol = True;
  }


} //# NAMESPACE CASA - END

