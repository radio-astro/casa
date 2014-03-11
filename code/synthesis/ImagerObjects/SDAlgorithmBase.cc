//# SDAlgorithmBase.cc: Implementation of SDAlgorithmBase classes
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
#include <synthesis/ImagerObjects/SDAlgorithmBase.h>
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

namespace casa { //# NAMESPACE CASA - BEGIN


  SDAlgorithmBase::SDAlgorithmBase():
    itsAlgorithmName("Test")
    //    itsDecSlices (),
    //    itsResidual(), itsPsf(), itsModel()
 {
 }

  SDAlgorithmBase::~SDAlgorithmBase()
 {
   
 }


  void SDAlgorithmBase::deconvolve( SIMinorCycleController &loopcontrols, 
				    CountedPtr<SIImageStore> &imagestore,
				    Int deconvolverid)
  {
    LogIO os( LogOrigin("SDAlgorithmBase","deconvolve",WHERE) );

    // Make a list of Slicers.
    //partitionImages( imagestore );

    Bool onechan=False, onepol=False;
    queryDesiredShape(onechan, onepol);
    uInt nSubChans, nSubPols;
    imagestore->getNSubImageStores(onechan,onepol,nSubChans,nSubPols);

    Bool usemask=True; // This should be a parameter...

    // Init the model image here.
    if( ! imagestore->checkValidity(True/*psf*/, True/*res*/,usemask/*wgt*/,True/*model*/,
				    False/*image*/, usemask/*mask*/ ) ) 
      { throw(AipsError("Internal Error : Invalid ImageStore for " + imagestore->getName())); }

    os << "-------------------------------------------------------------------------------------------------------------" << LogIO::POST;


    os << "Run " << itsAlgorithmName << " minor-cycle on " << nSubChans*nSubPols 
       << " slices of [" << deconvolverid << "]:" << imagestore->getName()
       << " [ CycleThreshold=" << loopcontrols.getCycleThreshold()
       << ", CycleNiter=" << loopcontrols.getCycleNiter() 
       << ", Gain=" << loopcontrols.getLoopGain()
       << " ]" << LogIO::POST;

    for( uInt chanid=0; chanid<nSubChans;chanid++)
      {
	for( uInt polid=0; polid<nSubPols; polid++)
	  {
	    itsImages = imagestore->getSubImageStore( chanid, onechan, polid, onepol );
	    
	    // Assign current subimages.
	    //	initializeSubImages( imagestore, subimageid );
	    
	    Int startiteration = loopcontrols.getIterDone();
	    Float peakresidual=0.0;
	    Float modelflux=0.0;
	    Int iterdone=0;
	    //	Bool converged=False;

	    // Or, call this from outside... in SynthesisImager.....
	    itsMaskHandler.makeAutoMask( itsImages ); //, (loopcontrols.getCycleThreshold()/peakresidual) );
	    
	    initializeDeconvolver( peakresidual, modelflux );
	    
	    Float startpeakresidual = peakresidual;
	    Float startmodelflux = modelflux;
	    
	    while ( ! checkStop( loopcontrols,  peakresidual ) )
	      {
		
		takeOneStep( loopcontrols.getLoopGain(), 
			     loopcontrols.getCycleNiter(), 
			     loopcontrols.getCycleThreshold(),
			     peakresidual, 
			     modelflux,
			     iterdone);
		
		//cout << "SDAlgoBase: After one step, dec : " << deconvolverid << "  plane : " << subimageid << "    residual=" << peakresidual << " model=" << modelflux << " iters=" << iterdone << endl; 
		
		loopcontrols.incrementMinorCycleCount( iterdone );
		loopcontrols.setPeakResidual( peakresidual );
		loopcontrols.addSummaryMinor( deconvolverid, chanid+polid*nSubChans, 
					      modelflux, peakresidual );
	      }// end of minor cycle iterations for this subimage.
	    
	    finalizeDeconvolver();
	    
	    // same as checking on itscycleniter.....
	    loopcontrols.setUpdatedModelFlag( loopcontrols.getIterDone()-startiteration );
	    
	    os << "[D" << deconvolverid << ":C" << chanid << ":P" << polid << "]"
	       <<" iters=" << startiteration+1 << "-" << loopcontrols.getIterDone()
	       << ", peakres=" << startpeakresidual << "-" << peakresidual 
	       << ", model=" << startmodelflux << "-" << modelflux
	       << LogIO::POST;
	    
	    loopcontrols.resetCycleIter(); 
	    
	  }// end of SubImage Loop
      }
  }// end of deconvolve
  
  Bool SDAlgorithmBase::checkStop( SIMinorCycleController &loopcontrols, 
				   Float currentresidual )
  {
    return loopcontrols.majorCycleRequired(currentresidual);
  }

  
   void SDAlgorithmBase::restore(CountedPtr<SIImageStore> imagestore )
  {

    LogIO os( LogOrigin("SDAlgorithmBase","restore",WHERE) );


    Bool onechan=False, onepol=False;
    queryDesiredShape(onechan, onepol);
    uInt nSubChans, nSubPols;
    imagestore->getNSubImageStores(onechan,onepol,nSubChans,nSubPols);

    if( ! imagestore->checkValidity(True/*psf*/, True/*res*/,False/*wgt*/,True/*model*/,True/*image*/,
				    True/*alpha*/, True/*beta*/) ) // alpha,beta will be ignored for single-term.
      { throw(AipsError("Internal Error : Invalid ImageStore for " + imagestore->getName())); }

    // Init the restored image here.
    //    imagestore->image();
    //    imagestore->model();

    for( uInt chanid=0; chanid<nSubChans;chanid++)
      {
	for( uInt polid=0; polid<nSubPols; polid++)
	  {
	    itsImages = imagestore->getSubImageStore( chanid, onechan, polid, onepol );
	    itsImages->restorePlane();
	  }
      }

    /*
    // Make a list of Slicers if it doesn't already exist. This is to allow standalone restoration
    if(itsDecSlices.nelements()==0)
      {
	partitionImages( imagestore );
      }
    
    os << "Restore all planes for " << imagestore->getName() << LogIO::POST;

    for( uInt subimageid=0; subimageid<itsDecSlices.nelements(); subimageid++)
      {
	// Assign current subimages.
	initializeSubImages( imagestore, subimageid );
	itsImage = SubImage<Float>( *(imagestore->image()), itsDecSlices[subimageid], True );

	restorePlane();

      }
    */

  }
  
  /*  
  void SDAlgorithmBase::restorePlane()
  {

    LogIO os( LogOrigin("SDAlgorithmBase","restorePlane",WHERE) );
    //     << ". Optionally, PB-correct too." << LogIO::POST;

    try
      {
	// Fit a Gaussian to the PSF.
	GaussianBeam beam = itsImages->getPSFGaussian();

	os << "Restore with beam : " << beam.getMajor(Unit("arcmin")) << " arcmin, " << beam.getMinor(Unit("arcmin"))<< " arcmin, " << beam.getPA(Unit("deg")) << " deg)" << LogIO::POST; 
	
	// Initialize restored image
	itsImage.set(0.0);
	// Copy model into it
	itsImage.copyData( LatticeExpr<Float>(itsModel )  );
	// Smooth model by beam
	StokesImageUtil::Convolve( itsImage, beam);
	// Add residual image
	itsImage.copyData( LatticeExpr<Float>( itsImage + itsResidual ) );
      }
    catch(AipsError &x)
      {
	throw( AipsError("Restoration Error " + x.getMesg() ) );
      }

  }
  */

  // Use this decide how to partition
  // the image for separate calls to 'deconvolve'.
  // Give codes to signal one or more of the following.
    ///    - channel planes separate
    ///    - stokes planes separate
    ///    - partitioned-image clean (facets ?)
  // Later, can add more complex partitioning schemes.... 
  // but there will be one place to do it, per deconvolver.
  void SDAlgorithmBase::queryDesiredShape(Bool &onechan, Bool &onepol) // , nImageFacets.
  {  
    onechan = True;
    onepol = True;
  }

  /*
  /// Make a list of Slices, to send sequentially to the deconvolver.
  /// Loop over this list of reference subimages in the 'deconvolve' call.
  /// This will support...
  ///    - channel cube clean
  ///    - stokes cube clean
  ///    - partitioned-image clean (facets ?)
  ///    - 3D deconvolver
  void SDAlgorithmBase::partitionImages( CountedPtr<SIImageStore> &imagestore )
  {
    LogIO os( LogOrigin("SDAlgorithmBase","partitionImages",WHERE) );

    IPosition imshape = imagestore->getShape();


    // TODO : Check which axes is which first !!!
    ///// chanAxis_p=CoordinateUtil::findSpectralAxis(dirty_p->coordinates());
    //// Vector<Stokes::StokesTypes> whichPols;
    //// polAxis_p=CoordinateUtil::findStokesAxis(whichPols, dirty_p->coordinates());
    uInt nx = imshape[0];
    uInt ny = imshape[1];
    uInt npol = imshape[2];
    uInt nchan = imshape[3];

    /// (1) /// Set up the Deconvolver Slicers.

    // Ask the deconvolver what shape it wants.
    Bool onechan=False, onepol=False;
    queryDesiredShape(onechan, onepol);

    uInt nSubImages = ( (onechan)?imshape[3]:1 ) * ( (onepol)?imshape[2]:1 ) ;
    uInt polstep = (onepol)?1:npol;
    uInt chanstep = (onechan)?1:nchan;

    itsDecSlices.resize( nSubImages );

    uInt subindex=0;
    for(uInt pol=0; pol<npol; pol+=polstep)
      {
	for(uInt chan=0; chan<nchan; chan+=chanstep)
	  {
	    AlwaysAssert( subindex < nSubImages , AipsError );
	    IPosition substart(4,0,0,pol,chan);
	    IPosition substop(4,nx-1,ny-1, pol+polstep-1, chan+chanstep-1);
	    itsDecSlices[subindex] = Slicer(substart, substop, Slicer::endIsLast);
	    subindex++;
	  }
      }

   }// end of partitionImages
  */

  /*
  void SDAlgorithmBase::initializeSubImages( CountedPtr<SIImageStore> &imagestore, uInt subim)
  {
    itsResidual = SubImage<Float>( *(imagestore->residual()), itsDecSlices[subim], True );
    itsPsf = SubImage<Float>( *(imagestore->psf()), itsDecSlices[subim], True );
    itsModel = SubImage<Float>( *(imagestore->model()), itsDecSlices[subim], True );

    itsImages = imagestore;

  }
  */

  /////////// Helper Functions for all deconvolvers to use if they need it.

  Bool SDAlgorithmBase::findMaxAbs(const Matrix<Float>& lattice,
					  Float& maxAbs,
					  IPosition& posMaxAbs)
  {
    posMaxAbs = IPosition(lattice.shape().nelements(), 0);
    maxAbs=0.0;
    Float minVal;
    IPosition posmin(lattice.shape().nelements(), 0);
    minMax(minVal, maxAbs, posmin, posMaxAbs, lattice);
    //cout << "min " << minVal << "  " << maxAbs << "   " << max(lattice) << endl;
    if(abs(minVal) > abs(maxAbs)){
      maxAbs=abs(minVal);
      posMaxAbs=posmin;
    }
    return True;
  }

  Bool SDAlgorithmBase::findMaxAbsMask(const Matrix<Float>& lattice,
				       const Matrix<Float>& mask,
					  Float& maxAbs,
					  IPosition& posMaxAbs)
  {

    //cout << "maxabsmask shapes : " << lattice.shape() << " " << mask.shape() << endl;

    posMaxAbs = IPosition(lattice.shape().nelements(), 0);
    maxAbs=0.0;
    Float minVal;
    IPosition posmin(lattice.shape().nelements(), 0);
    minMaxMasked(minVal, maxAbs, posmin, posMaxAbs, lattice,mask);
    //cout << "min " << minVal << "  " << maxAbs << "   " << max(lattice) << endl;
    if(abs(minVal) > abs(maxAbs)){
      maxAbs=abs(minVal);
      posMaxAbs=posmin;
    }
    return True;
  }
  
  /*

  GaussianBeam SDAlgorithmBase::getPSFGaussian()
  {

    GaussianBeam beam;
    try
      {
	if( itsPsf.ndim() > 0 )
	  {
	    StokesImageUtil::FitGaussianPSF( itsPsf, beam );
	  }
      }
    catch(AipsError &x)
      {
	LogIO os( LogOrigin("SDAlgorithmBase","getPSFGaussian",WHERE) );
	os << "Error in fitting a Gaussian to the PSF : " << x.getMesg() << LogIO::POST;
	throw( AipsError("Error in fitting a Gaussian to the PSF" + x.getMesg()) );
      }

    return beam;
  }

*/  
} //# NAMESPACE CASA - END

