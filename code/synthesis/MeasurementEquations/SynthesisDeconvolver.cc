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

#include <synthesis/MeasurementEquations/SynthesisDeconvolver.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisDeconvolver::SynthesisDeconvolver() : 
				       itsDeconvolver(NULL), 
				       itsMaskHandler(NULL),
				       itsImages(CountedPtr<SIImageStore>()),
				       itsPartImages(Vector<CountedPtr<SIImageStore> >()),
                                       itsImageName(""),
                                       itsPartImageNames(Vector<String>(0)),
				       itsDeconvolverId(0),
				       itsBeam(0.0)
  {
    
  }
  
  SynthesisDeconvolver::~SynthesisDeconvolver() 
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","descructor",WHERE) );
    os << "SynthesisDeconvolver destroyed" << LogIO::POST;
  }
  
  
  void SynthesisDeconvolver::setupDeconvolution(Record decpars)
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","setupDeconvolution",WHERE) );
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

      if( decpars.isDefined("id") )
	{ decpars.get( RecordFieldId("id") , itsDeconvolverId ); }

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading deconvolution parameters: "+x.getMesg()) );
      }
    
    os << "Set Deconvolution Options for image [" << itsDeconvolverId << "] :" << itsImageName ;
    if( itsPartImageNames.nelements()>0 ) os << " constructed from : " << itsPartImageNames;
    os << LogIO::POST;

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
  

  Record SynthesisDeconvolver::initMinorCycle( )
  { 
    LogIO os( LogOrigin("SynthesisDeconvolver","initMinorCycle",WHERE) );
    Record returnRecord;

    try {
      // Do the Gather if/when needed and check that images exist on disk.
      setupImagesOnDisk();
      // By now, all 'full' images have been gathered, but not normalized. Even in the non-parallel case.
      // Normalize the residual image. i.e. Calculate the principal solution  
      divideResidualByWeight();

      // Calculate Peak Residual and Max Psf Sidelobe, and fill into SubIterBot.
      //SISubIterBot loopController(subIterBotRecord);
      loopController.setPeakResidual( getPeakResidual() );
      loopController.setMaxPsfSidelobe( getPSFSidelobeLevel() );
      returnRecord = loopController.getCycleInitializationRecord();
    } catch(AipsError &x) {
      throw( AipsError("Error initializing the Minor Cycle : "+x.getMesg()) );
    }
    
    return returnRecord;
  }
  
  
  Record SynthesisDeconvolver::executeMinorCycle(Record& minorCycleControlRec)
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","executeMinorCycle",WHERE) );
    Record returnRecord;

    try {
      loopController.setCycleControls(minorCycleControlRec);
      itsDeconvolver->deconvolve( loopController, itsImages, itsDeconvolverId );
      returnRecord = loopController.getCycleExecutionRecord();

      // Set the model image in itsImages. If it's the same image name, this is a no-op internally.
      divideModelByWeight(); // This is currently a no-op
      scatterModel(); // This is a no-op for the single-node case.

    } catch(AipsError &x) {
      throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
    }
    return returnRecord;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  void SynthesisDeconvolver::setupImagesOnDisk() 
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","setupImagesOnDisk",WHERE) );

    // Check if full images exist, and open them if possible.
    Bool foundFullImage=False;
    try
      {
	itsImages = new SIImageStore( itsImageName );
	foundFullImage = True;
      }
    catch(AipsError &x)
      {
	//throw( AipsError("Error in constructing a Deconvolver : "+x.getMesg()) );
	foundFullImage = False;
      }


    // Check if part images exist
    Bool foundPartImages = itsPartImageNames.nelements()>0 ? True : False ;
    itsPartImages.resize( itsPartImageNames.nelements() );

    for ( uInt part=0; part < itsPartImageNames.nelements() ; part++ )
      {
	try
	  {
	    itsPartImages[part] = new SIImageStore( itsPartImageNames[part] );
	    foundPartImages |= True;
	  }
	catch(AipsError &x)
	  {
	    //throw( AipsError("Error in constructing a Deconvolver : "+x.getMesg()) );
	    foundPartImages = False;
	  }
      }
    if( foundPartImages == False) 
      { 
	itsPartImages.resize(0); 
      }
    else // Check that all have the same shape.
      {
	AlwaysAssert( itsPartImages.nelements() > 0 , AipsError );
	IPosition tempshape = itsPartImages[0]->getShape();
	for( uInt part=1; part<itsPartImages.nelements(); part++ )
	  {
	    if( tempshape != itsPartImages[part]->getShape() )
	      {
		throw( AipsError("Shapes of partial images to be combined, do not match") );
	      }
	  }
      }



    // Make sure all images exist and are consistent with each other. At the end, itsImages should be valid
    if( foundPartImages == True ) // Partial Images exist. Check that 'full' exists, and do the gather. 
      {
	if ( foundFullImage == True ) // Full image exists. Just check that shapes match with parts.
	  {
	    os << "Partial and Full images exist. Checking if part images have the same shape as the full image : ";
	    IPosition fullshape = itsImages->getShape();
	    
	    for ( uInt part=0; part < itsPartImages.nelements() ; part++ )
	      {
		IPosition partshape = itsPartImages[part]->getShape();
		if( partshape != fullshape )
		  {
		    os << "NO" << LogIO::POST;
		    throw( AipsError("Shapes of the partial and full images on disk do not match. Cannot gather") );
		  }
	      }
	    os << "Yes" << LogIO::POST;

	  }
	else // Full image does not exist. Need to make it, using the shape and coords of part[0]
	  {
	    os << "Only partial images exist. Need to make full images" << LogIO::POST;

	    AlwaysAssert( itsPartImages.nelements() > 0, AipsError );
	    PagedImage<Float> temppart( itsPartImageNames[0]+".residual" );
	    IPosition tempshape = temppart.shape();
	    CoordinateSystem tempcsys = temppart.coordinates();

	    itsImages = new SIImageStore( itsImageName, tempcsys, tempshape );
	    foundFullImage = True;
	  }

	// By now, all partial images and the full images exist on disk, and have the same shape.
	gatherImages();

      }
    else // No partial images supplied. Operating only with full images.
      {
	if ( foundFullImage == True ) 
	  {
	    os << "Full images exist : " << itsImageName << LogIO::POST;
	  }
	else // No full image on disk either. Error.
	  {
	    throw( AipsError("No images named " + itsImageName + " found on disk. No partial images found either.") );
	  }
      }
    

  }// end of setupImagesOnDisk



  void SynthesisDeconvolver::gatherImages()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver", "gatherImages",WHERE) );
    os << "Gather residual, psf, weight images : " << itsPartImageNames << " onto :" << itsImageName << LogIO::POST;

    AlwaysAssert( itsPartImages.nelements()>0 , AipsError );

    // Add intelligence to modify all only the first time, but later, only residual;
    itsImages->resetImages( /*psf*/True, /*residual*/True, /*weight*/True ); 

    for( uInt part=0;part<itsPartImages.nelements();part++)
      {
	itsImages->addImages( itsPartImages[part], /*psf*/True, /*residual*/True, /*weight*/True );
      }

  }// end of gatherImages

  void SynthesisDeconvolver::scatterModel()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver", "scatterModel",WHERE) );

    if( itsPartImages.nelements() > 0 )
      {
	os << "Send the model from : " << itsImageName << " to all nodes :" << itsPartImageNames << LogIO::POST;
	
	for( uInt part=0;part<itsPartImages.nelements();part++)
	  {
	    itsPartImages[part]->setModelImage( itsImages->getName() );
	  }
      }
  }// end of gatherImages


  void SynthesisDeconvolver::divideResidualByWeight()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver", "divideResidualByWeight",WHERE) );

    // If Weight image is not specified, treat it as though it were filled with ones. 
    // ( i.e.  itsWeight = NULL )

    itsImages->divideResidualByWeight( 0.1 );

  }

  void SynthesisDeconvolver::divideModelByWeight()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver", "divideModelByWeight",WHERE) );

    // If Weight image is not specified, treat it as though it were filled with ones. 
    // ( i.e.  itsWeight = NULL )

    /// This is a no-op here.... Need a way to activate this only for A-Projection.
    ///    itsImages->divideModelByWeight( 0.1 );

  }




  // #############################################
  // #############################################
  // #############################################
  // #############################################

  // Set a starting model.
  // NOTE : This is to be called only when the deconvolver is being used stand-alone.
  //  When used with SynthesisImager, the starting model needs to go only into SI, not SD.
  void SynthesisDeconvolver::setStartingModel(Record modpars)
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","setStartingModel",WHERE) );
    String modelname("");
    try{
      
      if( modpars.isDefined("modelname") )  // A single string
	{ modelname = modpars.asString( RecordFieldId("modelname")); }
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading starting model: "+x.getMesg()) );
      }
    
    try
      {
	
	if( modelname.length()>0 && !itsImages.null() )
	  {
	    os << "Setting " << modelname << " as starting model for deconvolution " << LogIO::POST;
	    itsImages->setModelImage( modelname );
	  }
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in setting  starting model for deconvolution: "+x.getMesg()) );
      }

  }
  
  
  // Calculate the peak residual for this mapper
  Float SynthesisDeconvolver::getPeakResidual()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","getPeakResidual",WHERE) );

    Float maxresidual = max( itsImages->residual()->get() );

    return maxresidual;
  }

  // Calculate the total model flux
  Float SynthesisDeconvolver::getModelFlux()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","getModelFlux",WHERE) );

    Float modelflux = sum( itsImages->model()->get() );

    return modelflux;
  }

  // Calculate the PSF sidelobe level...
  Float SynthesisDeconvolver::getPSFSidelobeLevel()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","getPSFSidelobeLevel",WHERE) );

    /// Calculate only once, store and return for all subsequent calls.

    Float psfsidelobe = max( itsImages->psf()->get() );

    return psfsidelobe;
  }

  // Restore Image.
  void SynthesisDeconvolver::restore()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","restoreImage",WHERE) );

    itsDeconvolver->restore(itsImages);

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

