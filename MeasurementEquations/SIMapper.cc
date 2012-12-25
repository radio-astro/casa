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
  
  SIMapper::SIMapper( CountedPtr<FTMachine> ftmachine, CountedPtr<SIDeconvolver> deconvolver, CountedPtr<CoordinateSystem> imcoordsys, CountedPtr<SIMaskHandler> maskhandler, Int mapperid) 
  {
    LogIO os( LogOrigin("SIMapper","Construct a mapper",WHERE) );

    itsFTMachine = ftmachine;
    itsDeconvolver = deconvolver;
    itsCoordSys = imcoordsys;
    itsMaskHandler = maskhandler;

    itsImShape = IPosition();

    updatedmodel_p = False;
    mapperid_p = mapperid;

    allocateImageMemory();

    partitionImages();

  }
  
  SIMapper::~SIMapper() 
  {
  }
  
  // Allocate Memory and open images.
  void SIMapper::allocateImageMemory()
  {

    LogIO os( LogOrigin("SIMapper","allocateImageMemory",WHERE) );

    os << "Mapper " << mapperid_p << " : Calculate required memory, and allocate" << LogIO::POST;

    //// TODO : If only the major cycle is called (niter=0), don't allocate Image, Psf, Weight...

    itsImage=0.0;
    itsPsf=0.2;
    itsModel=0.0;
    itsWeight=1.0;

    // Read nchan from itsCoordSys --- and allocate 'images' of that shape/size.
    SpectralCoordinate scoord = itsCoordSys->spectralCoordinate();

    //itsImShape = IPosition(4,imx,imy,npol, (scoord.worldValues()).nelements()  );
    itsImShape = IPosition(4,1,1,1, (scoord.worldValues()).nelements()  );

    // Initial Peak Residuals - for single-pixel-image testing.
    itsOriginalResidual = 1.0;
    if ( mapperid_p == 0 )  itsOriginalResidual=1.0;
    if ( mapperid_p == 1 )  itsOriginalResidual=0.5;

    itsResidual=itsOriginalResidual;

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


  // TODO : Check which axes is which, and pick the appropriate shape.
  //             Use an imageAxesMap to go from imx,imy,npol,nchan to itsImShape
  Int SIMapper::getNx()
  {
    return (itsImShape.nelements()==4)? itsImShape[0] : 0 ;
  }
  Int SIMapper::getNy()
  {
    return (itsImShape.nelements()==4)? itsImShape[1] : 0 ;
  }
  Int SIMapper::getNChan()
  {
    return (itsImShape.nelements()==4)? itsImShape[3] : 0 ;
  }
  Int SIMapper::getNPol()
  {
    return (itsImShape.nelements()==4)? itsImShape[2] : 0 ;
  }




  // Run the deconvolver
  // TODO : Loop over the list of reference SubImages here.
  //  This means, for iteration control, each SubImage is treated the same as a separate Mapper.
  void SIMapper::deconvolve( SISubIterBot &loopcontrols  )
  {
    LogIO os( LogOrigin("SIMapper","deconvolve",WHERE) );

    updatedmodel_p = itsDeconvolver->deconvolve( loopcontrols, itsResidual, itsPsf, itsModel, itsMaskHandler,  mapperid_p );

  }

  // Calculate the peak residual for this mapper
  Float SIMapper::getPeakResidual()
  {
    LogIO os( LogOrigin("SIMapper","getPeakResidual",WHERE) );

    Float maxresidual = itsResidual;

    return maxresidual;
  }

  // Calculate the total model flux
  Float SIMapper::getModelFlux()
  {
    LogIO os( LogOrigin("SIMapper","getModelFlux",WHERE) );

    Float modelflux = itsModel;

    return modelflux;
  }

  // Calculate the PSF sidelobe level...
  Float SIMapper::getPSFSidelobeLevel()
  {
    LogIO os( LogOrigin("SIMapper","getPSFSidelobeLevel",WHERE) );

    /// Calculate only once, store and return for all subsequent calls.

    Float psfsidelobe = itsPsf;

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
    
    itsDeconvolver->restore( itsImage, itsBeam, itsModel, itsResidual, itsWeight );

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

    itsResidual = itsOriginalResidual - itsModel;
    
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

