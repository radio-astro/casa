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

    updatedmodel_p = False;
    mapperid_p = mapperid;

    // Temp
    tmpPos_p = IPosition(4,0,0,0,0);

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

    os << "Mapper " << mapperid_p << " : Calculate required memory, and allocate" << LogIO::POST;

    // Make the Images.
    itsResidual = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".residual"));
    itsPsf = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".psf"));
    itsModel = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".model"));
    itsWeight = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".weight"));

    Array<Float> pixels( itsImageShape );
    pixels = 0.0;
    itsResidual->put(pixels); 
    itsPsf->put(pixels);
    itsPsf->putAt(0.2, tmpPos_p);
    itsModel->put(pixels);
    pixels = 1.0;
    itsWeight->put(pixels);

    // Initial Peak Residuals - for single-pixel-image testing.
    itsOriginalResidual = 1.0;
    if ( mapperid_p == 0 )  itsOriginalResidual=1.0;
    if ( mapperid_p == 1 )  itsOriginalResidual=0.5;

    itsResidual->putAt(itsOriginalResidual, tmpPos_p);

    cout << "Starting residual : " << endl;
    cout << itsResidual->getAt(tmpPos_p) << endl;

    /// If there is a starting model, set updatedmodel_p = True !!

  }

  // TODO For the current deconvolver, decide how many sliced deconvolution calls to make.
  void SIMapper::partitionImages()
  {
    /// Call it separately per channel or stokes
    //itsDeconvolver->makeSubImageList();

    /// Make a list of reference SubImages here, with the correct shapes.
    /// Loop over this list of reference subimages in the 'deconvolve' call.
    /// This will support...
    ///    - channel cube clean
    ///    - stokes cube clean
    ///    - partitioned-image clean (facets ?)
    ///    - 3D deconvolver

  }


  // Run the deconvolver
  // TODO : Loop over the list of reference SubImages here.
  //  This means, for iteration control, each SubImage is treated the same as a separate Mapper.
  void SIMapper::deconvolve( SISubIterBot &loopcontrols  )
  {
    LogIO os( LogOrigin("SIMapper","deconvolve",WHERE) );

    updatedmodel_p = itsDeconvolver->deconvolve( loopcontrols, *itsResidual, *itsPsf, *itsModel, itsMaskHandler,  mapperid_p );

  }

  // Calculate the peak residual for this mapper
  Float SIMapper::getPeakResidual()
  {
    LogIO os( LogOrigin("SIMapper","getPeakResidual",WHERE) );

    Float maxresidual = itsResidual->getAt(tmpPos_p);

    return maxresidual;
  }

  // Calculate the total model flux
  Float SIMapper::getModelFlux()
  {
    LogIO os( LogOrigin("SIMapper","getModelFlux",WHERE) );

    Float modelflux = itsModel->getAt(tmpPos_p);

    return modelflux;
  }

  // Calculate the PSF sidelobe level...
  Float SIMapper::getPSFSidelobeLevel()
  {
    LogIO os( LogOrigin("SIMapper","getPSFSidelobeLevel",WHERE) );

    /// Calculate only once, store and return for all subsequent calls.

    Float psfsidelobe = itsPsf->getAt( tmpPos_p );

    return psfsidelobe;
  }

  // Check if the model has been updated or not
  Bool SIMapper::isModelUpdated()
  {
    LogIO os( LogOrigin("SIMapper","isModelUpdated",WHERE) );

    return updatedmodel_p;
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

    //itsResidual = itsOriginalResidual - itsModel;

    itsResidual->putAt( itsOriginalResidual - itsModel->getAt(tmpPos_p)  , tmpPos_p );

  }

  void SIMapper::initializeDegrid()
  {
    LogIO os( LogOrigin("SIMapper", "initializeDegrid",WHERE) );

    if ( updatedmodel_p == False ) 
      {
        os << "Mapper " << mapperid_p << " : No new model to predict visibilities from" << LogIO::POST;
      }
    else
      {
        os << "Mapper " << mapperid_p << " : Degridding current model" << LogIO::POST;
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

