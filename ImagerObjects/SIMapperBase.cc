//# SIMapperBase.cc: Implementation of Imager.h
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

#include <synthesis/ImagerObjects/SIMapperBase.h>
#include <synthesis/ImagerObjects/SIIterBot.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapperBase::SIMapperBase( CountedPtr<SIImageStore> imagestore,
		      CountedPtr<FTMachine> ftmachine, 
		      Int mapperid)
  {
    LogIO os( LogOrigin("SIMapperBase","Construct a mapper",WHERE) );

    ft_p = ftmachine;
    ift_p = ftmachine; // This should be a clone.
    cft_p=NULL;
    itsImages = imagestore;
    itsImageShape = itsImages->getShape();

    itsIsModelUpdated = False;
    itsMapperId = mapperid;

    //    initImages();

  }
  
  SIMapperBase::~SIMapperBase() 
  {
    LogIO os( LogOrigin("SIMapperBase","destructor",WHERE) );
  }
  
  Bool SIMapperBase::releaseImageLocks() 
  {
    LogIO os( LogOrigin("SIMapperBase","releaseImageLocks",WHERE) );
    return itsImages->releaseLocks();
  }

  // Allocate Memory and open images.
  //// TODO : If only the major cycle is called (niter=0), don't allocate Image, Psf, Weight...
  void SIMapperBase::initImages()
  {
    LogIO os( LogOrigin("SIMapperBase","initImages",WHERE) );

    /*

    // Initialize all these images.
    //////////////////////////////////// ONLY FOR TESTING //////////////////////////////////
    // Initial Peak Residuals - for single-pixel-image testing.
    // In the real world, this is the gridded/imaged data.
    itsOriginalResidual.resize( itsImageShape );
    //itsOriginalResidual = 0.0;

    //    if( itsImageShape[0]==3 && itsImageShape[1]==3 )
    //  {
	itsOriginalResidual = itsImages->psf()->get();
	//  }

    // Different values for different mappers
    for (uInt ch=0; ch < itsImageShape[3]; ch++)
      {
	if ( itsMapperId == 0 )  itsOriginalResidual = itsOriginalResidual * 1.0;
	if ( itsMapperId == 1 )  itsOriginalResidual = itsOriginalResidual * 0.5;
	if ( itsMapperId == 2 )  itsOriginalResidual = itsOriginalResidual * 0.7;
      }
    // Give the first mapper a spectral line, if nchan>2
    if ( itsMapperId == 0 && itsImageShape[3] > 2 ) itsOriginalResidual = itsImages->psf()->get()*2.0;

    //////////////////////////////////// ONLY FOR TESTING /////////////////////////////////

    /// If there is a starting model, set itsIsModelUpdated = True !!

    */

  }


  // #############################################
  // #############################################
  // #######  Gridding / De-gridding functions ###########
  // #############################################
  // #############################################

  /// All these take in vb's, and just pass them on.

  void SIMapperBase::initializeGrid(/* vb */)
  {
    LogIO os( LogOrigin("SIMapperBase","initializeGrid",WHERE) );
    // itsFTM->initializeToSky( itsImages->residual(), vb )
  }

  void SIMapperBase::grid(/* vb */)
  {
    LogIO os( LogOrigin("SIMapperBase","grid",WHERE) );
  }

  //// The function that makes the PSF should check its validity, and fit the beam,
  void SIMapperBase::finalizeGrid()
  {
    LogIO os( LogOrigin("SIMapperBase","finalizeGrid",WHERE) );

    // TODO : Fill in itsImages->residual(), itsImages->psf(), itsImages->weight().
    // Do not normalize the residual by the weight. 
    //   -- Normalization happens later, via 'divideResidualImageByWeight' called from SI.divideImageByWeight()
    //   -- This will ensure that normalizations are identical for the single-node and parallel major cycles. 

    // For TESTING
    //itsImages->residual()->put( itsOriginalResidual - itsImages->model()->get() );

    //    itsImages->residual()->put( itsOriginalResidual - max(itsImages->model()->get())*itsImages->psf()->get() );

  }

  void SIMapperBase::initializeDegrid()
  {
    LogIO os( LogOrigin("SIMapperBase", "initializeDegrid",WHERE) );

  }

  void SIMapperBase::degrid()
  {
    LogIO os( LogOrigin("SIMapperBase","degrid",WHERE) );
  }

  void SIMapperBase::finalizeDegrid()
  {
    LogIO os( LogOrigin("SIMapperBase","finalizeDegrid",WHERE) );
  }

  Bool SIMapperBase::getFTMRecord(Record& /*rec*/)
  {
    LogIO os( LogOrigin("SIMapperBase","getFTMRecord",WHERE) );
    // rec = itsFTM->toRecord();
    return True;
  }
  Bool SIMapperBase::getCLRecord(Record& /*rec*/)
  {
	  LogIO os( LogOrigin("SIMapperBase","getFTMRecord",WHERE) );
      // rec = itsFTM->toRecord();
      return True;
  }
  String SIMapperBase::getImageName()
  {
    return itsImages->getName();
  }

} //# NAMESPACE CASA - END

