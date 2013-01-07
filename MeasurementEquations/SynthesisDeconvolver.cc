//# SynthesisDeconvolver.cc: Implementation of Imager.h
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

#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>

#include<synthesis/MeasurementEquations/SIIterBot.h>
#include <synthesis/MeasurementEquations/SynthesisDeconvolver.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisDeconvolver::SynthesisDeconvolver() : 
				       itsDeconvolver(NULL), 
				       itsMaskHandler(NULL),
                                       itsImageShape(IPosition()),
				       itsCoordSys(NULL),
                                       itsImageName(""),
                                       itsPartImageNames(Vector<String>(0)),
				       itsModel(NULL),
				       itsImage(NULL),itsPsf(NULL),itsResidual(NULL),itsWeight(NULL),
				       itsPartImages(),itsPartPsfs(),itsPartResiduals(),itsPartWeights(),
				       itsBeam(0.0)
  {
    
  }
  
  SynthesisDeconvolver::~SynthesisDeconvolver() 
  {
  }
  
  
  void SynthesisDeconvolver::setupDeconvolution(Record decpars)
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","setupDeconvolution",WHERE) );
    os << "Set Deconvolution Options - Construct Deconvolver" << LogIO::POST;
    
    try
      {

      if( decpars.isDefined("imagename") )  // A single string
	{ itsImageName = decpars.asString( RecordFieldId("imagename")); }
      else
	{throw( AipsError("imagename not specified")); }

      if( decpars.isDefined("partimagenames") )  // A vector of strings
	{ decpars.get( RecordFieldId("partimagenames") , itsPartImageNames ); }
      else
	{ itsPartImageNames.resize(0); }


      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading deconvolution parameters: "+x.getMesg()) );
      }
    
    try
      {
	itsDeconvolver = new SDAlgorithmBase();
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing a Deconvolver : "+x.getMesg()) );
      }

    try
      {
	/// itsCurrentMaskHandler = XXX ( check the mask input for accepted formats )
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing a MaskHandler : "+x.getMesg()) );
      }
    
    
  }//end of setupDeconvolution
  
  
  Record SynthesisDeconvolver::executeMinorCycle(Record& subIterBotRecord)
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","executeMinorCycle",WHERE) );
    Record returnRecord;

    try {

      // Do the Gather if/when needed and check that images exist on disk.
      if( !checkImagesOnDisk() ) 
	{
	  throw( AipsError("Cannot validate images on disk, before starting") );
	}

      SISubIterBot loopController(subIterBotRecord);
      deconvolve( loopController );
      returnRecord = loopController.serialize();

    } catch(AipsError &x) {
      throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
    }
    return returnRecord;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  Bool SynthesisDeconvolver::checkImagesOnDisk() 
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","checkImagesOnDisk",WHERE) );

    // Check if full images exist
    Bool foundFullImage = doImagesExist( itsImageName );

    // Check if part images exist
    Bool foundPartImages = itsPartImageNames.nelements()>0 ? True : False ;
    for ( uInt part=0; part < itsPartImageNames.nelements() ; part++ )
      {
	foundPartImages &= doImagesExist( itsPartImageNames[part] );
      }

    if( foundPartImages == True ) // Partial Images exist. Check that 'full' exists, and do the gather. 
      {
	if ( foundFullImage == True ) // Full image exists. Just check that shapes match with parts.
	  {
	    //// itsImageShape = xxx
	    //// itsCoordSys = new .... xxx
	  }
	else // Full image does not exist. Need to make it, using the shape and coords of part[0]
	  {

	    /*
	    itsResidual = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".residual"));
	    itsPsf = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".psf"));
	    itsWeight = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".weight"));
	    */
	    foundFullImage = True;
	  }

	// By now, all partial images and the full images exist on disk, and have the same shape.
	gatherImages();

      }
    else // No partial images supplied. Operating only with full images.
      {
	if ( foundFullImage == True ) // Just open them and check that shapes match each other.
	  {
	    itsResidual = new PagedImage<Float> ( itsImageName+String(".residual") );
	    itsPsf = new PagedImage<Float> ( itsImageName+String(".psf") );
	    itsWeight = new PagedImage<Float> ( itsImageName+String(".weight") );
	    /// If weight image is not given, do not make a PagedImage. Leave it 'null'.
	    
	    itsImageShape = itsResidual->shape(); // Check with shapes of psf and weight.
	  }
	else // No full image on disk either. Error.
	  {
	    return False;
	  }
      }

    return foundFullImage;

  }

  // TODO : Move to an image-wrapper class ? Same function exists in SIMapper
  Bool SynthesisDeconvolver::doImagesExist( String /*imageName*/ )
  {
    // Check if imagename.residual, imagename.psf. imagename.weight
    // exist on disk and if they're the right shape.
    return False;
  }


  /// Make a list of Slices, to send sequentially to the deconvolver.
  /// Loop over this list of reference subimages in the 'deconvolve' call.
  /// This will support...
  ///    - channel cube clean
  ///    - stokes cube clean
  ///    - partitioned-image clean (facets ?)
  ///    - 3D deconvolver
  Vector<Slicer> SynthesisDeconvolver::partitionImages()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","partitionImages",WHERE) );

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
    Vector<Slicer> decSlices( nSubImages );

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

    return decSlices;

  }// end of partitionImages


  void SynthesisDeconvolver::gatherImages()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver", "gatherImages",WHERE) );

    //AlwaysAssert( itsResidual , AipsError ) ; /// Check that all image pointers are valid here.

    os << "Gather residual, psf, weight images : " << itsPartImageNames << " onto :" << itsImageName << LogIO::POST;

  }

  void SynthesisDeconvolver::divideResidualImageByWeight()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver", "divFullImageByWeight",WHERE) );

    // If Weight image is not specified, treat it as though it were filled with ones. 
    // ( i.e.  itsWeight = NULL )

    os << "Dividing the residual image " << itsImageName+String(".residual") << " by the weightimage " << itsImageName+String(".weight") << LogIO::POST;
  }

  // #############################################
  // #############################################
  // #############################################
  // #############################################


  // Run the deconvolver for each Slice.
  // This means, for iteration control, each SubImage is treated the same as a separate Mapper.
  void SynthesisDeconvolver::deconvolve( SISubIterBot &loopcontrols  )
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","deconvolve",WHERE) );

    Bool isModelUpdated=False;

    divideResidualImageByWeight();

    // Make a list of Slicers
    Vector<Slicer> decSlices = partitionImages();

    for( uInt subim=0; subim<decSlices.nelements(); subim++)
      {

	//cout << "Mapper : " << itsMapperId << "  Deconvolving : " << decSlices[subim] << endl;
	SubImage<Float> subResidual( *itsResidual, decSlices[subim], True );
	SubImage<Float> subPsf( *itsPsf, decSlices[subim], True );
	SubImage<Float> subModel( *itsModel, decSlices[subim], True );
	//// MASK too....  SubImage subMask( *itsResidual, decSlices[subim], True );

	isModelUpdated |= itsDeconvolver->deconvolve( loopcontrols, subResidual, subPsf, subModel, itsMaskHandler, subim );
        loopcontrols.resetCycleIter();

      }
    loopcontrols.setUpdatedModelFlag( isModelUpdated );
  }

  // Calculate the peak residual for this mapper
  Float SynthesisDeconvolver::getPeakResidual()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","getPeakResidual",WHERE) );

    Float maxresidual = max( itsResidual->get() );

    return maxresidual;
  }

  // Calculate the total model flux
  Float SynthesisDeconvolver::getModelFlux()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","getModelFlux",WHERE) );

    Float modelflux = sum( itsModel->get() );

    return modelflux;
  }

  // Calculate the PSF sidelobe level...
  Float SynthesisDeconvolver::getPSFSidelobeLevel()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","getPSFSidelobeLevel",WHERE) );

    /// Calculate only once, store and return for all subsequent calls.

    Float psfsidelobe = max( itsPsf->get() );

    return psfsidelobe;
  }

  // Restore Image.
  void SynthesisDeconvolver::restore()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","restoreImage",WHERE) );
    
    itsImage = new PagedImage<Float> (itsImageShape, *itsCoordSys, itsImageName+String(".image"));
    itsDeconvolver->restore( *itsImage, itsBeam, *itsModel, *itsResidual, *itsWeight );

  }


  // This is for interactive-clean.
  void SynthesisDeconvolver::getCopyOfResidualAndMask( TempImage<Float> &/*residual*/,
                                           TempImage<Float> &/*mask*/ )
  {
    // Actually all I think we need here are filenames JSK 12/12/12
    // resize/shape and copy the residual image and mask image to these in/out variables.
    // Allocate Memory here.
  }
  void SynthesisDeconvolver::setMask( TempImage<Float> &/*mask*/ )
  {
    // Here we will just pass in the new names
    // Copy the input mask to the local main image mask
  }




} //# NAMESPACE CASA - END

