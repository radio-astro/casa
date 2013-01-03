//# SIMapper.cc: Implementation of Imager.h
//# Copyright (C) 1997-2008
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

#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <casa/OS/DirectoryIterator.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>

#include <casa/OS/HostInfo.h>

#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <synthesis/MeasurementEquations/SIMapper.h>
#include <synthesis/MeasurementEquations/SIIterBot.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapper::SIMapper( String imagename, CountedPtr<FTMachine> ftmachine, CountedPtr<SIDeconvolver> deconvolver, CountedPtr<CoordinateSystem> imcoordsys, IPosition imshape, CountedPtr<SIMaskHandler> maskhandler, Int mapperid) 
  {
    LogIO os( LogOrigin("SIMapper","Construct a mapper",WHERE) );

    itsImageName = imagename;

    itsFTMachine = ftmachine;
    itsDeconvolver = deconvolver;
    itsCoordSys = imcoordsys;
    itsImageShape = imshape;
    itsMaskHandler = maskhandler;

    itsImage=NULL;
    itsPsf=NULL;
    itsModel=NULL;
    itsResidual=NULL;
    itsWeight=NULL;

    decSlices.resize();
    ftmSlices.resize();

    itsIsModelUpdated = False;
    itsMapperId = mapperid;

    allocateImageMemory();

    partitionImages();

  }
  
  SIMapper::~SIMapper() 
  {
  }
  
  // Allocate Memory and open images.
  //// TODO : If only the major cycle is called (niter=0), don't allocate Image, Psf, Weight...
  void SIMapper::allocateImageMemory()
  {

    LogIO os( LogOrigin("SIMapper","allocateImageMemory",WHERE) );

    os << "Mapper " << itsMapperId << " : Calculate required memory, and allocate" << LogIO::POST;

    // Make the Images.
    itsResidual = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".residual"));
    itsPsf = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".psf"));
    itsModel = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".model"));
    itsWeight = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".weight"));

    // Initialize all these images.
    Array<Float> pixels( itsImageShape );
    pixels = 0.0;
    itsResidual->set(0.0); 
    itsPsf->set(0.0);
    itsPsf->set(0.2);
    itsModel->set(0.0);
    itsWeight->set(1.0);

    //////////////////////////////////// ONLY FOR TESTING //////////////////////////////////
    // Initial Peak Residuals - for single-pixel-image testing.
    // In the real world, this is the gridded/imaged data.
    itsOriginalResidual.resize( itsImageShape );
    itsOriginalResidual = 0.0;

    // Different values for different mappers
    for (uInt ch=0; ch < itsImageShape[3]; ch++)
      {
	if ( itsMapperId == 0 )  itsOriginalResidual( IPosition(4,0,0,0,ch) ) = 1.0;
	if ( itsMapperId == 1 )  itsOriginalResidual( IPosition(4,0,0,0,ch) ) = 0.5;
	if ( itsMapperId == 2 )  itsOriginalResidual( IPosition(4,0,0,0,ch) ) = 0.7;
      }

    // Give the first mapper a spectral line, if nchan>2
    if ( itsMapperId == 0 && itsImageShape[3] > 2 ) itsOriginalResidual( IPosition(4,0,0,0,1) ) = 2.0;

    //////////////////////////////////// ONLY FOR TESTING /////////////////////////////////

    /// If there is a starting model, set itsIsModelUpdated = True !!

  }

  /// Make a list of Slices, to send sequentially to the deconvolver.
  /// Loop over this list of reference subimages in the 'deconvolve' call.
  /// This will support...
  ///    - channel cube clean
  ///    - stokes cube clean
  ///    - partitioned-image clean (facets ?)
  ///    - 3D deconvolver
  void SIMapper::partitionImages()
  {
    LogIO os( LogOrigin("SIMapper","partitionImages",WHERE) );

    uInt nx = itsImageShape[0];
    uInt ny = itsImageShape[1];
    uInt npol = itsImageShape[2];
    uInt nchan = itsImageShape[3];

    /// (1) /// Set up the Deconvolver Slicers.

    // Ask the deconvolver what shape it wants.
    Bool onechan=False, onepol=False;
    itsDeconvolver->queryDesiredShape(onechan, onepol);

    uInt nSubImages = ( (onechan)?itsImageShape[3]:1 ) * ( (onepol)?itsImageShape[2]:1 ) ;
    uInt polstep = (onepol)?1:npol;
    uInt chanstep = (onechan)?1:nchan;

    os << "Number of sub-deconvolvers : " << nSubImages << LogIO::POST;
    decSlices.resize(nSubImages);

    uInt subindex=0;
    for(uInt pol=0; pol<npol; pol+=polstep)
      {
	for(uInt chan=0; chan<nchan; chan+=chanstep)
	  {
	    AlwaysAssert( subindex < nSubImages , AipsError );
	    IPosition substart(4,0,0,pol,chan);
	    IPosition substop(4,nx-1,ny-1, pol+polstep-1, chan+chanstep-1);
	    decSlices[subindex] = Slicer(substart, substop, Slicer::endIsLast);
	    subindex++;
	  }
      }

    /// (2) /// Set up the FTM Slicers

    //Ask the FTMs what chunk sizes they want, and make a similar list of Slices.
    // ftmSlices.resize(nftmchunks);

  }// end of partitionImages


  // Run the deconvolver for each Slice.
  // This means, for iteration control, each SubImage is treated the same as a separate Mapper.
  void SIMapper::deconvolve( SISubIterBot &loopcontrols  )
  {
    LogIO os( LogOrigin("SIMapper","deconvolve",WHERE) );

    for( uInt subim=0; subim<decSlices.nelements(); subim++)
      {

	//cout << "Mapper : " << itsMapperId << "  Deconvolving : " << decSlices[subim] << endl;
	SubImage<Float> subResidual( *itsResidual, decSlices[subim], True );
	SubImage<Float> subPsf( *itsPsf, decSlices[subim], True );
	SubImage<Float> subModel( *itsModel, decSlices[subim], True );
	//// MASK too....  SubImage subMask( *itsResidual, decSlices[subim], True );

	itsIsModelUpdated |= itsDeconvolver->deconvolve( loopcontrols, subResidual, subPsf, subModel, itsMaskHandler,  itsMapperId , subim );
      }
  }

  // Calculate the peak residual for this mapper
  Float SIMapper::getPeakResidual()
  {
    LogIO os( LogOrigin("SIMapper","getPeakResidual",WHERE) );

    Float maxresidual = max( itsResidual->get() );

    return maxresidual;
  }

  // Calculate the total model flux
  Float SIMapper::getModelFlux()
  {
    LogIO os( LogOrigin("SIMapper","getModelFlux",WHERE) );

    Float modelflux = sum( itsModel->get() );

    return modelflux;
  }

  // Calculate the PSF sidelobe level...
  Float SIMapper::getPSFSidelobeLevel()
  {
    LogIO os( LogOrigin("SIMapper","getPSFSidelobeLevel",WHERE) );

    /// Calculate only once, store and return for all subsequent calls.

    Float psfsidelobe = max( itsPsf->get() );

    return psfsidelobe;
  }

  // Check if the model has been updated or not
  Bool SIMapper::isModelUpdated()
  {
    LogIO os( LogOrigin("SIMapper","isModelUpdated",WHERE) );

    return itsIsModelUpdated;
  }


  // Restore Image.
  void SIMapper::restore()
  {
    LogIO os( LogOrigin("SIMapper","restoreImage",WHERE) );
    
    itsImage = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".image"));
    itsDeconvolver->restore( *itsImage, itsBeam, *itsModel, *itsResidual, *itsWeight );

  }


  // This is for interactive-clean.
  void SIMapper::getCopyOfResidualAndMask( TempImage<Float> &/*residual*/,
                                           TempImage<Float> &/*mask*/ )
  {
    // Actually all I think we need here are filenames JSK 12/12/12
    // resize/shape and copy the residual image and mask image to these in/out variables.
    // Allocate Memory here.
  }
  void SIMapper::setMask( TempImage<Float> &/*mask*/ )
  {
    // Here we will just pass in the new names
    // Copy the input mask to the local main image mask
  }



  // #############################################
  // #############################################
  // #######  Functions to be called from SISkyEquation ###########
  // #############################################
  // #  Inside the Mapper, the 'vb' is not to be used. It's only to pass to FTM.
  // #  All vi,vb operations should stay in SISkyEquation
  // #############################################

  /// All these take in vb's, and just pass them on.

  void SIMapper::initializeGrid()
  {
    // itsFTM->initializeToSky( itsResidual, vb )
  }

  void SIMapper::grid()
  {
  }

  //// The function that makes the PSF should check its validity, and fit the beam,
  void SIMapper::finalizeGrid()
  {

    itsResidual->put( itsOriginalResidual - itsModel->get() );

  }

  void SIMapper::initializeDegrid()
  {
    LogIO os( LogOrigin("SIMapper", "initializeDegrid",WHERE) );

    if ( itsIsModelUpdated == False ) 
      {
        os << "Mapper " << itsMapperId << " : No new model to predict visibilities from" << LogIO::POST;
      }
    else
      {
        os << "Mapper " << itsMapperId << " : Degridding current model" << LogIO::POST;
      }
  }

  void SIMapper::degrid()
  {
  }

  void SIMapper::finalizeDegrid()
  {
  }

  Record SIMapper::getFTMRecord()
  {
    Record rec;
    // rec = itsFTM->toRecord();
    return rec;
  }
   
} //# NAMESPACE CASA - END

