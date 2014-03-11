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

#include <synthesis/ImagerObjects/SynthesisDeconvolver.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisDeconvolver::SynthesisDeconvolver() : 
				       itsDeconvolver(NULL), 
				       itsMaskHandler(NULL),
				       itsImages(CountedPtr<SIImageStore>()),
				       //				       itsPartImages(Vector<CountedPtr<SIImageStore> >()),
                                       itsImageName(""),
				       //                                       itsPartImageNames(Vector<String>(0)),
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

    String algorithm("test");

    uInt nTaylorTerms=1; // Try to remove....
    try
      {

      if( decpars.isDefined("imagename") )  // A single string
	{ itsImageName = decpars.asString( RecordFieldId("imagename")); }
      else
	{throw( AipsError("imagename not specified")); }

      /*
      if( decpars.isDefined("partimagenames") )  // A vector of strings
	{ decpars.get( RecordFieldId("partimagenames") , itsPartImageNames ); }
      else
	{ itsPartImageNames.resize(0); }
      */

      if( decpars.isDefined("id") )
	{ decpars.get( RecordFieldId("id") , itsDeconvolverId ); }

      if( decpars.isDefined("algo") )
	{ decpars.get( RecordFieldId("algo") , algorithm ); algorithm.downcase(); }

      
      if( decpars.isDefined("startmodel") )  // A single string
	{ itsStartingModelName = decpars.asString( RecordFieldId("startmodel")); }

      if( decpars.isDefined("ntaylorterms") )
	{ decpars.get( RecordFieldId("ntaylorterms") , nTaylorTerms ); }


      // Scale sizes...


      }
    catch(AipsError &x)
      {
	os << "Error in reading parameters " << LogIO::POST;
	throw( AipsError("Error in reading deconvolution parameters: "+x.getMesg()) );
      }
    
    os << "Set Deconvolution Options for image [" << itsDeconvolverId << "] :" << itsImageName ;
    if( itsStartingModelName.length() > 0 ) os << " , starting from model : " << itsStartingModelName;
    //    if( itsPartImageNames.nelements()>0 ) os << " constructed from : " << itsPartImageNames;
    os << LogIO::POST;

    try
      {
	/*
	if(algorithm==String("test")) 
	  {
	    itsDeconvolver = new SDAlgorithmTest(); 
	  }
	
	  else */ if(algorithm==String("hogbom"))
	  {
	    itsDeconvolver = new SDAlgorithmHogbomClean(); 
	  }
	  else if(algorithm==String("msmfs"))
	  {
	    itsDeconvolver = new SDAlgorithmMSMFS( nTaylorTerms ); 
	    } 
	else
	  {
	    throw( AipsError("Un-known algorithm : "+algorithm) );
	  }
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

    // Set flag to add model image
    itsAddedModel=False;
      
  }//end of setupDeconvolution
  

  Record SynthesisDeconvolver::initMinorCycle( )
  { 
    LogIO os( LogOrigin("SynthesisDeconvolver","initMinorCycle",WHERE) );
    Record returnRecord;

    try {
      // Do the Gather if/when needed and check that images exist on disk. Normalize by Weights too.
      //gatherImages();

      if( itsDeconvolver->getAlgorithmName() == "msmfs" )
	{  itsImages = new SIImageStoreMultiTerm( itsImageName, itsDeconvolver->getNTaylorTerms() ); }
      else
	{  itsImages = new SIImageStore( itsImageName ); }

      // If a starting model exists, this will initialize the ImageStore with it. Will do this only once.
      setStartingModel();
 
      // Normalize by the weight image.
      ///divideResidualByWeight();

      // Calculate Peak Residual and Max Psf Sidelobe, and fill into SubIterBot.
      //SISubIterBot itsLoopController(subIterBotRecord);
      itsLoopController.setPeakResidual( getPeakResidual() );
      itsLoopController.setMaxPsfSidelobe( getPSFSidelobeLevel() );
      returnRecord = itsLoopController.getCycleInitializationRecord();

      os << "Initialized minor cycle. Returning returnRec" << LogIO::POST;

    } catch(AipsError &x) {
      throw( AipsError("Error initializing the Minor Cycle for "  + itsImageName + " : "+x.getMesg()) );
    }
    
    return returnRecord;
  }
  
  
  Record SynthesisDeconvolver::executeMinorCycle(Record& minorCycleControlRec)
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","executeMinorCycle",WHERE) );
    Record returnRecord;

    try {
      itsLoopController.setCycleControls(minorCycleControlRec);
      //   maskHandler.makeAutoMask( itsImages );
      itsDeconvolver->deconvolve( itsLoopController, itsImages, itsDeconvolverId );
      returnRecord = itsLoopController.getCycleExecutionRecord();

      //scatterModel(); // This is a no-op for the single-node case.

      itsImages->releaseLocks();

    } catch(AipsError &x) {
      throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
    }
    return returnRecord;
  }

  // Restore Image.
  void SynthesisDeconvolver::restore()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","restoreImage",WHERE) );

    if( itsImages.null() )
      {
	itsImages = new SIImageStore( itsImageName );
      }

    itsDeconvolver->restore(itsImages);

  }



  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



  // #############################################
  // #############################################
  // #############################################
  // #############################################

  // Set a starting model.
  void SynthesisDeconvolver::setStartingModel()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","setStartingModel",WHERE) );

    if(itsAddedModel==True) {return;}
    
    try
      {
	
	if( itsStartingModelName.length()>0 && !itsImages.null() )
	  {
	    os << "Setting " << itsStartingModelName << " as starting model for deconvolution " << LogIO::POST;
	    itsImages->setModelImage( itsStartingModelName );
	  }

	itsAddedModel=True;
	
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

    Float psfsidelobe = fabs(min( itsImages->psf()->get() ));

    if(psfsidelobe == 1.0)
      {
	//os << LogIO::WARN << "For testing only. Set psf sidelobe level to 0.01" << LogIO::POST;
	psfsidelobe = 0.01;
      }

    return psfsidelobe;
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

