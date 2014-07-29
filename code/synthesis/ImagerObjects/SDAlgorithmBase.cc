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
    itsAlgorithmName("Test"),
    tmpPos_p( IPosition() ),
    itsImages( NULL ),
    itsDecSlices (),
    itsResidual(), itsPsf(), itsModel(),
    itsComp(0.0)
 {

   // TESTING place-holder for the position of the clean component.
   tmpPos_p = IPosition(4,0,0,0,0);

 }

  SDAlgorithmBase::~SDAlgorithmBase()
 {
   
 }


  void SDAlgorithmBase::deconvolve( SIMinorCycleController &loopcontrols, 
				    CountedPtr<SIImageStore> &imagestore,
				    Int deconvolverid)
  {
    LogIO os( LogOrigin("SDAlgorithmBase","deconvolve",WHERE) );

    itsImages = imagestore;  // TODOO : This is a private store of all full images that all functions here can use.

    // Make a list of Slicers.
    partitionImages();

    os << "-------------------------------------------------------------------------------------------------------------" << LogIO::POST;


    os << "Run " << itsAlgorithmName << " minor-cycle on " << itsDecSlices.nelements() 
       << " slices of [" << deconvolverid << "]:" << itsImages->getName()
       << " [ CycleThreshold=" << loopcontrols.getCycleThreshold()
       << ", CycleNiter=" << loopcontrols.getCycleNiter() 
       << ", Gain=" << loopcontrols.getLoopGain()
       << " ]" << LogIO::POST;


    for( uInt subimageid=0; subimageid<itsDecSlices.nelements(); subimageid++)
      {
	// Assign current subimages.
	initializeSubImages( subimageid );

	Int startiteration = loopcontrols.getIterDone();
	Float peakresidual=0.0;
	Float modelflux=0.0;
	Int iterdone=0;

	initializeDeconvolver( peakresidual, modelflux );

	Float startpeakresidual = peakresidual;
	Float startmodelflux = modelflux;

	while ( ! checkStop( loopcontrols,  peakresidual ) )
	  {
	    // Optionally, fiddle with maskhandler for autoboxing.... 
	    // mask = maskhandler->makeAutoBox();
	    
	    takeOneStep( loopcontrols.getLoopGain(), 
			 loopcontrols.getCycleNiter(), 
			 loopcontrols.getCycleThreshold(),
			 peakresidual, 
			 modelflux,
			 iterdone);

	    //	    cout << "SDAlgoBase: After one step, residual=" << peakresidual << " model=" << modelflux << " iters=" << iterdone << endl;

	    loopcontrols.incrementMinorCycleCount( iterdone );
	    loopcontrols.setPeakResidual( peakresidual );
	    loopcontrols.addSummaryMinor( deconvolverid, subimageid, modelflux, peakresidual );
	  }// end of minor cycle iterations for this subimage.

	finalizeDeconvolver();
	
	// same as checking on itscycleniter.....
	loopcontrols.setUpdatedModelFlag( loopcontrols.getIterDone()-startiteration );
	
	os << "[D" << deconvolverid << ":S" << subimageid << "]"
	   <<" iters=" << startiteration << "-" << loopcontrols.getIterDone()
	   << ", peakres=" << startpeakresidual << "-" << peakresidual 
	   << ", model=" << startmodelflux << "-" << modelflux
	   << LogIO::POST;
	
	loopcontrols.resetCycleIter(); 
	
      }// end of SubImage Loop
  }// end of deconvolve
  
  Bool SDAlgorithmBase::checkStop( SIMinorCycleController &loopcontrols, 
				   Float currentresidual )
  {
    return loopcontrols.majorCycleRequired(currentresidual);
  }

   void SDAlgorithmBase::restore(CountedPtr<SIImageStore> imagestore )
  {

    LogIO os( LogOrigin("SDAlgorithmBase","restore",WHERE) );
    
    os << "Smooth model and add residuals for " << imagestore->getName() 
       << ". Optionally, PB-correct too." << LogIO::POST;

  }


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

  /// Make a list of Slices, to send sequentially to the deconvolver.
  /// Loop over this list of reference subimages in the 'deconvolve' call.
  /// This will support...
  ///    - channel cube clean
  ///    - stokes cube clean
  ///    - partitioned-image clean (facets ?)
  ///    - 3D deconvolver
  void SDAlgorithmBase::partitionImages()
  {
    LogIO os( LogOrigin("SDAlgorithmBase","partitionImages",WHERE) );

    IPosition imshape = itsImages->getShape();


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


  void SDAlgorithmBase::initializeSubImages(uInt subim)
  {
    itsResidual = SubImage<Float>( *(itsImages->residual()), itsDecSlices[subim], True );
    itsPsf = SubImage<Float>( *(itsImages->psf()), itsDecSlices[subim], True );
    itsModel = SubImage<Float>( *(itsImages->model()), itsDecSlices[subim], True );
  }


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
      maxAbs=minVal;
      posMaxAbs=posmin;
    }
    return True;
  }
  
  
  
} //# NAMESPACE CASA - END

