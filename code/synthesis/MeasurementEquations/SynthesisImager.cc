//# SynthesisImager.cc: Implementation of Imager.h
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

#include<synthesis/MeasurementEquations/SIIterBot.h>
#include <synthesis/MeasurementEquations/SynthesisImager.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
//#include <casadbus/utilities/BusAccess.h>
//#include <casadbus/session/DBusSession.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisImager::SynthesisImager() : itsMappers(SIMapperCollection()), 
				       //				       itsVisSet(NULL),
				       itsCurrentFTMachine(NULL), 
				       itsCurrentDeconvolver(NULL), 
				       itsCurrentCoordSys(NULL),
				       itsCurrentMaskHandler(NULL),
				       itsSkyModel(SISkyModel()),
				       itsSkyEquation(SISkyEquation()),
                                       itsLoopController(),
                                       startmodel_p(String("")), 
				       usescratch_p(True)
  {
    
  }
  
  SynthesisImager::~SynthesisImager() 
  {
  }
  
  // Make this read in a list of MS's and selection pars....
  void  SynthesisImager::selectData(Record selpars)
  {
    LogIO os( LogOrigin("SynthesisImager","selectData",WHERE) );
    
    Vector<String> mslist,fieldlist,spwlist;
    
    try
      {
	
	if( selpars.isDefined("vis") ) { selpars.get( RecordFieldId("vis") , mslist ); }
	if( selpars.isDefined("field") ) { selpars.get( RecordFieldId("field") , fieldlist ); }
	if( selpars.isDefined("spw") ) { selpars.get( RecordFieldId("spw") , spwlist ); }
	
	
	uInt nms = mslist.nelements();
	if(fieldlist.nelements() != nms){os << LogIO::EXCEPTION << "Need " << nms << " field selection strings, one for each specified MS" << LogIO::POST; }
	if(spwlist.nelements() != nms){os << LogIO::EXCEPTION << "Need " << nms << " spw selection strings, one for each specified MS" << LogIO::POST; }
	
	for(uInt sel=0; sel<nms; sel++)
	  {
	    os << "MS : " << mslist[sel];
	    os << "   Selection : spw='" << spwlist[sel] << "'";
	    os << " field='" << fieldlist[sel] << "'" << LogIO::POST;
	  }
	
	if( selpars.isDefined("usescratch") ) { selpars.get( RecordFieldId("usescratch"), usescratch_p ); }
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading selection parameter record : "+x.getMesg()) );
      }
    
    try
      {
	
	os << "Setup vi/vb and construct SkyEquation" << LogIO::POST;
	//VisSet vset;
	itsSkyEquation.init(); // vset );
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in creating SkyEquation : "+x.getMesg()) );
      }
    
  }// end of selectData()
  
  // Construct Image Coordinates
  void  SynthesisImager::defineImage(Record impars)
  {
    LogIO os( LogOrigin("SynthesisImager","defineImage",WHERE) );
    
    os << "Define/construct Image Coordinates" << LogIO::POST;

    /* Use the image name to create a unique service name */
    try {
      std::string imagename = impars.asString( RecordFieldId("imagename"));
      itsLoopController.reset( new SIIterBot("SynthesisImage_" + imagename));
      // Read and interpret input parameters.
    } catch(AipsError &x)
      {
	throw( AipsError("Error in reading input image-parameters: "+x.getMesg()) );
      }

      


    Int nchan=1;

    try
      {
	if( impars.isDefined("nchan") ) { impars.get( RecordFieldId("nchan") , nchan ); }

	// Read and interpret input parameters.
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading input image-parameters: "+x.getMesg()) );
      }
    
    try
      {
	
	//itsCurrentCoordSys = XXXX

	itsCurrentCoordSys = new CoordinateSystem();
	SpectralCoordinate* mySpectral=0;
	
	MFrequency::Types imfreqref=MFrequency::REST;
	Vector<Double> chanFreq( nchan );
	for(Int ch=0;ch<nchan;ch++)
	  {
	    chanFreq[ch] = 1.0e+09 + (Double)ch * 1.0e+06;
	  }
	Double restFreq = 1.0e+09;
	mySpectral = new SpectralCoordinate(imfreqref, chanFreq, restFreq);

	itsCurrentCoordSys->addCoordinate(*mySpectral);
	if(mySpectral) delete mySpectral;

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing image coordinate system : "+x.getMesg()) );
      }
    
    
  }// end of defineImage
  
  void  SynthesisImager::setupImaging(Record gridpars)
  {
    LogIO os( LogOrigin("SynthesisImager","setupImaging",WHERE) );
    os << "Set Imaging Options - Construct FTMachine" << LogIO::POST;
    
    try
      {
	
	if( gridpars.isDefined("startmodel") ) { gridpars.get( RecordFieldId("startmodel"), startmodel_p ); }
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading Imaging Parameters : "+x.getMesg()) );
      }
    
    try
      {
	
	/// itsCurrentFTMachine = XXX
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing FTMachine : "+x.getMesg()) );
      }
    
  }// end of setupImaging
  
  void SynthesisImager::setupDeconvolution(Record /*decpars*/)
  {
    LogIO os( LogOrigin("SynthesisImager","setupDeconvolution",WHERE) );
    os << "Set Deconvolution Options - Construct Deconvolver" << LogIO::POST;
    
    try
      {
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading deconvolution parameters: "+x.getMesg()) );
      }
    
    try
      {
	/// itsCurrentDeconvolver = XXX
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
  
  void SynthesisImager::initMapper()
  {
    LogIO os( LogOrigin("SynthesisImager","initMapper", WHERE) );
    os << "Construct a Mapper from the current FTMachine and Deconvolver, and allocate Image Memory" << LogIO::POST;
    try
      {

	/*	
	Int nMappers = itsMappers.nelements();
	itsMappers.resize(nMappers+1, True);
	itsMappers[nMappers] = new SIMapper( itsCurrentFTMachine, itsCurrentDeconvolver, itsCurrentCoordSys , nMappers );
	*/

	itsMappers.addMapper( itsCurrentFTMachine, itsCurrentDeconvolver, itsCurrentCoordSys, itsCurrentMaskHandler );

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing Mapper : "+x.getMesg()) );
      }
    
    
  }//end of initMapper
  
  void SynthesisImager::setIterationDetails(Record iterpars)
  {
    LogIO os( LogOrigin("SynthesisImager","updateIterationDetails",WHERE) );
    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));
                 
        itsLoopController->setControlsFromRecord(iterpars);
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in updating iteration parameters : " +
                         x.getMesg()) );
      }
  }

  Record SynthesisImager::getIterationDetails()
  {
    LogIO os( LogOrigin("SynthesisImager","getIterationDetails",WHERE) );
    Record returnRecord;
    try {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

      returnRecord = itsLoopController->getDetailsRecord();
    } catch(AipsError &x) {
      throw( AipsError("Error in retrieving iteration parameters : " +
                       x.getMesg()) );
    }
    return returnRecord;
  }

  Record SynthesisImager::getIterationSummary()
  {
    LogIO os( LogOrigin("SynthesisImager","getIterationSummary",WHERE) );
    Record returnRecord;
    try {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

      returnRecord = itsLoopController->getSummaryRecord();
    } catch(AipsError &x) {
      throw( AipsError("Error in retrieving iteration parameters : " +
                       x.getMesg()) );
    }
    return returnRecord;
  }


  void SynthesisImager::setupIteration(Record iterpars)
  {
    LogIO os( LogOrigin("SynthesisImager","setupIteration",WHERE) );
    os << "Set Iteration Control Options : Construct SISkyModel" << LogIO::POST;
     try
      {
        setIterationDetails(iterpars);
        itsSkyModel.init(); // Send in iteration-control parameters here.// Or... do nothing.
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing SkyModel : "+x.getMesg()) );
      }
  } //end of setupIteration
  
  void SynthesisImager::setInteractiveMode(Bool interactiveMode) 
  {
    LogIO os( LogOrigin("SynthesisImager","setupIteration",WHERE) );
    os << "Setting intractive mode to " 
       << ((interactiveMode) ? "Active" : "Inactive") << LogIO::POST;
    try {
      if (itsLoopController.get() == NULL) 
        throw( AipsError("Iteration Control un-initialized"));
      itsLoopController->changeInteractiveMode(interactiveMode);
    } catch(AipsError &x) {
      throw( AipsError("Error Setting Interactive Mode : "+x.getMesg()) );
    }
  }
  
  void SynthesisImager::initCycles()
  {
    LogIO os( LogOrigin("SynthesisImager","initCycles", WHERE) );
    os << "Do nothing. (Construct SkyModel and SkyEquation)" << LogIO::POST;
    try
      {
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing image coordinate system : "+x.getMesg()) );
      }
  }
  
  bool SynthesisImager::cleanComplete() {
    bool returnValue;
    try {
      if (itsLoopController.get() == NULL) 
        throw( AipsError("Iteration Control un-initialized"));
      returnValue=itsLoopController->cleanComplete(itsMappers.findPeakResidual());
    } catch (AipsError &x) {
      throw( AipsError("Error checking clean complete state : "+x.getMesg()) );
    }
    return returnValue; 
  }

  /*
    Record  SynthesisImager::initLoops()
    {
    LogIO os( LogOrigin("SynthesisImager","initLoops",WHERE) );
    os << "Initialize Cleaning" << LogIO::POST;
    
    }
  */
  
  void  SynthesisImager::endLoops()//SIIterBot& loopcontrol)
  {
    LogIO os( LogOrigin("SynthesisImager","endLoops",WHERE) );
    
    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

        if(itsLoopController->getUpdatedModelFlag() ==True)
	  {
	    os << "Restore Image (and normalize to Flat Sky) " << LogIO::POST;
	    itsSkyModel.restore( itsMappers );
	  }
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing image coordinate system : "+
                         x.getMesg()) );
      }
  }// end of endLoops
  
  void SynthesisImager::runMajorCycle()
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );
    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

 	if(itsLoopController->getCompletedNiter()==0 &&
           startmodel_p.length()>1 )
 	  {
            itsLoopController->setUpdatedModelFlag(true);
 	  }
	
	if(itsLoopController->getMajorCycleCount() ==0)
	  {
	    os << "Make PSFs, weights and initial dirty/residual images. " ;
	  }
	else
	  {
	    if(! itsLoopController->getUpdatedModelFlag())
	      {
		os << "No new model. No need to update residuals in a major cycle." << LogIO::POST;
		return;
	      }
	    os << "Update residual image in major cycle " << 
              String::toString(itsLoopController->getMajorCycleCount()) << ". ";
	  }
	
	if(itsLoopController->cleanComplete(itsMappers.findPeakResidual()) &&
           itsLoopController->getUpdatedModelFlag())
	  {
	    if(usescratch_p==True)
	      {
		os << "Save image model to MS in MODEL_DATA column on disk" 
                   << LogIO::POST;
	      }
	    else
	      {
		os << "Save image model to MS as a Record for on-the-fly prediction" << LogIO::POST;
	      }
	  }
	else
	  {
	    os << LogIO::POST;
	  }
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in setting up Major Cycle : "+x.getMesg()) );
      }
    
    
    try
      {    
	// se.runMajorCycle(xxxxx . modeltoms = usescratch)
	itsSkyEquation.runMajorCycle( itsMappers );
	itsLoopController->incrementMajorCycleCount();
	itsLoopController->addSummaryMajor();

	/* The first time, when PSFs are made, all mappers need to compute 
           PSF parameters ( peak sidelobe level, etc ) 0 and store it inside 
           the mappers. */

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in running Major Cycle : "+x.getMesg()) );
      }    
  }// end of runMajorCycle
  
  void  SynthesisImager::runMinorCycle() //SIIterBot& loopcontrols)
  {
    LogIO os( LogOrigin("SynthesisImager","runMinorCycle",WHERE) );

    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

        itsSkyModel.runMinorCycle( itsMappers , *itsLoopController );
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
      }
  }// end of runMinorCycle
  
} //# NAMESPACE CASA - END

