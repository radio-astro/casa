//# SIImageStore.cc: Implementation of Imager.h
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

#include <synthesis/MeasurementEquations/SIImageStore.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  
  SIImageStore::SIImageStore() 
  {
    itsPsf=NULL;
    itsModel=NULL;
    itsResidual=NULL;
    itsWeight=NULL;
    itsImage=NULL;

    itsImageName=String("");
    itsImageShape=IPosition();

  }

  SIImageStore::SIImageStore(String imagename) 
  {
    LogIO os( LogOrigin("SIImageStore","Open existing Images",WHERE) );

    itsImageName = imagename;

    if( doImagesExist( ) )
      {
	itsResidual = new PagedImage<Float> (itsImageName+String(".residual"));
	itsPsf = new PagedImage<Float> (itsImageName+String(".psf"));
	itsWeight = new PagedImage<Float> (itsImageName+String(".weight"));

	itsImageShape = itsResidual->shape();

      }
    else
      {
	throw( AipsError( "Images do not exist. Please create them" ) );
      }

    if( doesModelImageExist() )
      {
	itsModel = new PagedImage<Float> (itsImageName+String(".model"));
      }
    else
      {
	throw( AipsError( "Model Image does not exist. Please create it" ) );
      }
  }

  SIImageStore::SIImageStore(String imagename, 
			     CountedPtr<CoordinateSystem> imcoordsys, 
			     IPosition imshape)
  {
    LogIO os( LogOrigin("SIImageStore","Open new Images",WHERE) );

    itsImageName = imagename;
    itsImageShape = imshape;

    // Make the Images.
    //     -- Check, and create a new image only if it does not already exist on disk.
    //     -- If it exists on disk, check that it's shape and coordinates 

    if( doImagesExist( ) )
      {
	os << "Images already exist. Overwriting them";
      }

    itsResidual = new PagedImage<Float> (itsImageShape, *imcoordsys, itsImageName+String(".residual"));
    itsPsf = new PagedImage<Float> (itsImageShape, *imcoordsys, itsImageName+String(".psf"));
    itsWeight = new PagedImage<Float> (itsImageShape, *imcoordsys, itsImageName+String(".weight"));
    
    if( doesModelImageExist() )
      {
	os << "Model Image already exists. Overwriting it";
      }
    itsModel = new PagedImage<Float> (itsImageShape, *imcoordsys, itsImageName+String(".model"));

    os << LogIO::POST;

  }

  
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  SIImageStore::~SIImageStore() 
  {
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  IPosition SIImageStore::getShape()
  {
    return itsImageShape;
  }

  String SIImageStore::getName()
  {
    return itsImageName;
  }

  CountedPtr<PagedImage<Float> > SIImageStore::psf()
  {
    return itsPsf;
  }
  CountedPtr<PagedImage<Float> > SIImageStore::residual()
  {
    return itsResidual;
  }
  CountedPtr<PagedImage<Float> > SIImageStore::weight()
  {
    return itsWeight;
  }
  CountedPtr<PagedImage<Float> > SIImageStore::model()
  {
    return itsModel;
  }
  CountedPtr<PagedImage<Float> > SIImageStore::image()
  {
    return itsImage;
  }

  // TODO : Move to an image-wrapper class ? Same function exists in SynthesisDeconvolver.
  Bool SIImageStore::doImagesExist()
  {
    LogIO os( LogOrigin("SIImageStore","doImagesExist",WHERE) );
    // Check if imagename.residual, imagename.psf. imagename.weight
    // exist on disk and if they're the right shape.
    // If the shape is not right, complain here and throw an exception (or just say it will get overwritten)

    Directory impsf( itsImageName+String(".psf") );
    Directory imresidual( itsImageName+String(".residual") );
    Directory imweight( itsImageName+String(".weight") );

    return impsf.exists() & imresidual.exists() & imweight.exists();
  }

  Bool SIImageStore::doesModelImageExist()
  {
    LogIO os( LogOrigin("SIImageStore","doesModelImageExist",WHERE) );
    // Check if the model image exists.
    // If it exists, then...
    //   If the shape is not right, attempt a re-grid onto itsImageShape here.
    //   If it's a component list, but the FTM needs an image, evaluate the model image here.
    //   If none of the above here, then complain.

    Directory immodel( itsImageName+String(".model") );
    return immodel.exists();
  }


  void SIImageStore::allocateRestoredImage()
  {

    //    itsImage = new PagedImage<Float> (itsImageShape, *imcoordsys, itsImageName+String(".residual"));
  }


  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END

