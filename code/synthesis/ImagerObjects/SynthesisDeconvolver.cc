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
				       itsBeam(0.0),
				       itsDeconvolverId(0),
				       itsScales(Vector<Float>())
  {
    
  }
  
  SynthesisDeconvolver::~SynthesisDeconvolver() 
  {
        LogIO os( LogOrigin("SynthesisDeconvolver","descructor",WHERE) );
	os << LogIO::DEBUG1 << "SynthesisDeconvolver destroyed" << LogIO::POST;
  }

  void SynthesisDeconvolver::setupDeconvolution(const SynthesisParamsDeconv& decpars)
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","setupDeconvolution",WHERE) );

    itsImageName = decpars.imageName;
    itsStartingModelName = decpars.startModel;
    itsDeconvolverId = decpars.deconvolverId;
    
    os << "Set Deconvolution Options for [" << itsImageName << "] : " << decpars.algorithm ;
    if( itsStartingModelName.length() > 0 ) os << " , starting from model : " << itsStartingModelName;
    os << LogIO::POST;

    try
      {
	if(decpars.algorithm==String("hogbom"))
	  {
	    itsDeconvolver = new SDAlgorithmHogbomClean(); 
	  }
	else if(decpars.algorithm==String("mtmfs"))
	  {
	    itsDeconvolver = new SDAlgorithmMSMFS( decpars.nTaylorTerms, decpars.scales ); 
	  } 
	else if(decpars.algorithm==String("clark"))
	  {
	    itsDeconvolver = new SDAlgorithmClarkClean("clark"); 
	  } 
	else if(decpars.algorithm==String("clarkstokes"))
	  {
	    itsDeconvolver = new SDAlgorithmClarkClean("clarkstokes"); 
	  } 
	else if(decpars.algorithm==String("multiscale"))
	  {
	    itsDeconvolver = new SDAlgorithmMSClean( decpars.scales ); 
	  } 
	else if(decpars.algorithm==String("mem"))
	  {
	    itsDeconvolver = new SDAlgorithmMEM( "entropy" ); 
	  } 
	else
	  {
	    throw( AipsError("Un-known algorithm : "+decpars.algorithm) );
	  }

	// Set restoring beam options
	itsDeconvolver->setRestoringBeam( decpars.restoringbeam, decpars.usebeam );

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing a Deconvolver : "+x.getMesg()) );
      }
    
    itsAddedModel=False;
  }
  
   Record SynthesisDeconvolver::initMinorCycle( )
  { 
    LogIO os( LogOrigin("SynthesisDeconvolver","initMinorCycle",WHERE) );
    Record returnRecord;

    try {
      // Do the Gather if/when needed and check that images exist on disk. Normalize by Weights too.
      //gatherImages();

      /*
      if( itsDeconvolver->getAlgorithmName() == "msmfs" )
	{  itsImages = new SIImageStoreMultiTerm( itsImageName, itsDeconvolver->getNTaylorTerms() ); }
      else
	{  itsImages = new SIImageStore( itsImageName ); }
      */

      itsImages = makeImageStore( itsImageName );

      // If a starting model exists, this will initialize the ImageStore with it. Will do this only once.
      setStartingModel();
 
      // Normalize by the weight image.
      ///divideResidualByWeight();

      // Calculate Peak Residual and Max Psf Sidelobe, and fill into SubIterBot.
      itsLoopController.setPeakResidual( itsImages->getPeakResidual() );
      itsLoopController.setMaxPsfSidelobe( itsImages->getPSFSidelobeLevel() );
      returnRecord = itsLoopController.getCycleInitializationRecord();

      itsImages->printImageStats();

      os << LogIO::DEBUG2 << "Initialized minor cycle. Returning returnRec" << LogIO::POST;

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
	itsImages = makeImageStore( itsImageName );
      }

    itsDeconvolver->restore(itsImages);

  }

  // Restore Image.
  void SynthesisDeconvolver::pbcor()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","pbcor",WHERE) );

    if( itsImages.null() )
      {
	itsImages = makeImageStore( itsImageName );
      }

    itsDeconvolver->pbcor(itsImages);

  }



  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  CountedPtr<SIImageStore> SynthesisDeconvolver::makeImageStore( String imagename )
  {
    CountedPtr<SIImageStore> imstore;
    if( itsDeconvolver->getAlgorithmName() == "mtmfs" )
      {  imstore =  new SIImageStoreMultiTerm( imagename, itsDeconvolver->getNTaylorTerms(), True ); }
    else
      {  imstore = new SIImageStore( imagename, True ); }

    return imstore;

  }


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
  
  /*
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
*/

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

  /*
  //  Bool SynthesisDeconvolver::findMinMaxMask(const Array<Float>& lattice,
  void SynthesisDeconvolver::findMinMax(const Array<Float>& lattice,
					const Array<Float>& mask,
					Float& minVal, Float& maxVal,
					Float& minValMask, Float& maxValMask)
  {
    IPosition posmin(lattice.shape().nelements(), 0);
    IPosition posmax(lattice.shape().nelements(), 0);

    if( sum(mask) <1e-06 ) {minValMask=0.0; maxValMask=0.0;}
    else { minMaxMasked(minValMask, maxValMask, posmin, posmax, lattice,mask); }

    minMax( minVal, maxVal, posmin, posmax, lattice );
  }

  void SynthesisDeconvolver::printImageStats()
  {
    LogIO os( LogOrigin("SynthesisDeconvolver","getPeakResidual",WHERE) );
    Float minresmask, maxresmask, minres, maxres;
    findMinMax( itsImages->residual()->get(), itsImages->mask()->get(), minres, maxres, minresmask, maxresmask );

    os << "[" << itsImageName << "]:" ;
    os << " Peak residual (max,min) " ;
    if( minresmask!=0.0 || maxresmask!=0.0 )
      { os << "within mask : (" << maxresmask << "," << minresmask << ") "; }
    os << "over full image : (" << maxres << "," << minres << ")" << LogIO::POST;

  }
*/

} //# NAMESPACE CASA - END

