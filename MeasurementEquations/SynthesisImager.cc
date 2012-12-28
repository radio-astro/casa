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
                                       itsCurrentImageShape(IPosition()),
                                       itsCurrentImageName(""),
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
    
    Vector<String> mslist(0),fieldlist(0),spwlist(0);
   
    try
      {
	// TODO : If critical params are unspecified, throw exceptions.
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
    Int nchan=1,npol=1,imx=1,imy=1;
    String phasecenter =  "19:59:28.500 +40.44.01.50";
    Double cellx=10.0,celly=10.0;

    std::string imagename;
    try {

      // TODO : If critical params are unspecified, throw exceptions.
      if( impars.isDefined("imagename") ) 
	{ imagename = impars.asString( RecordFieldId("imagename")); }
      else
	{throw( AipsError("imagename not specified")); }

      if( impars.isDefined("nchan") ) 
	{ impars.get( RecordFieldId("nchan") , nchan ); }
      else
	{throw( AipsError("nchan not specified")); }

      // Read and interpret input parameters.
    } catch(AipsError &x)
      {
	throw( AipsError("Error in reading input image-parameters: "+x.getMesg()) );
      }

    try
      {
	
	itsCurrentCoordSys = buildImageCoordinateSystem(phasecenter, 
                                                        cellx, celly, imx, imy, npol, nchan );
        itsCurrentImageShape = IPosition(4,imx,imy,npol,nchan);

        itsCurrentImageName = imagename;

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
      // TODO : If critical params are unspecified, throw exceptions.
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
	itsCurrentDeconvolver = new SIDeconvolver();
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

	itsMappers.addMapper( itsCurrentImageName,  itsCurrentFTMachine, itsCurrentDeconvolver, itsCurrentCoordSys, itsCurrentImageShape, itsCurrentMaskHandler );

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
        itsLoopController.reset( new SIIterBot("SynthesisImage_"));
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
  
  Record SynthesisImager::getMajorCycleControls()
  {
    /* This method makes all decisions about how the major cycle should 
       execute.  The output of this is passed to the runMajorCycle method
    */
    LogIO os( LogOrigin("SynthesisImager","getMajorCycleControls",WHERE) );
    Record returnRecord;

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
		return returnRecord;//With appropriate flag
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
    return returnRecord;
  }

  void SynthesisImager::endMajorCycle()
  {
    LogIO os( LogOrigin("SynthesisImager","endMajorCycle",WHERE) );

    try{
      if (itsLoopController.get() == NULL) 
        throw( AipsError("Iteration Control un-initialized"));

      itsLoopController->incrementMajorCycleCount();
      itsLoopController->addSummaryMajor();
    } catch(AipsError &x) {
      throw( AipsError("Error in running Major Cycle : "+x.getMesg()) );
    }    
  }

  void SynthesisImager::executeMajorCycle(Record& /*controlRecord*/)
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );
    
    try
      {    
	// se.runMajorCycle(xxxxx . modeltoms = usescratch)
	itsSkyEquation.runMajorCycle( itsMappers );

	/* The first time, when PSFs are made, all mappers need to compute 
           PSF parameters ( peak sidelobe level, etc ) 0 and store it inside 
           the mappers. */

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in running Major Cycle : "+x.getMesg()) );
      }    
  }// end of runMajorCycle
  
  
  Record SynthesisImager::getSubIterBot()
  {
    Record returnRecord;

    LogIO os( LogOrigin("SynthesisImager","getSubIterBot",WHERE) );
    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

        Float peakResidual = itsMappers.findPeakResidual();

        itsLoopController->setMaxPsfSidelobe( itsMappers.findMaxPsfSidelobe());
        itsLoopController->updateCycleThreshold(peakResidual);

        os << "Start Minor-Cycle iterations with peak residual = " << peakResidual;
        os << " and model flux = " << itsMappers.addIntegratedFlux() << LogIO::POST;
        
        os << " [ cyclethreshold = " << itsLoopController->getCycleThreshold() ;
        os << " max iter per field/chan/pol = " << itsLoopController->getCycleNiter() ;
        os << " loopgain = " << itsLoopController->getLoopGain() ;
        os << " ]" << LogIO::POST;

        if (itsLoopController->interactiveInputRequired(peakResidual)) {
          pauseForUserInteraction();
        }

        returnRecord = itsLoopController->getSubIterBotRecord();
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
      }
    return returnRecord;
  }

  Record SynthesisImager::executeMinorCycle(Record& subIterBotRecord) {
    LogIO os( LogOrigin("SynthesisImager","executeMinorCycle",WHERE) );
    Record returnRecord;
    
    try {
      SISubIterBot loopController(subIterBotRecord);
      itsSkyModel.runMinorCycle( itsMappers , loopController );
      returnRecord = loopController.serialize();
    } catch(AipsError &x) {
      throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
    }
    return returnRecord;
  }

  void SynthesisImager::endMinorCycle(Record& subIterBotRecord) {
    try {
      if (itsLoopController.get() == NULL) 
        throw( AipsError("Iteration Control un-initialized"));

      SISubIterBot loopController(subIterBotRecord);
      LogIO os( LogOrigin("SynthesisImager",__FUNCTION__,WHERE) );
      itsLoopController->mergeSubIterBot(loopController);
    } catch(AipsError &x) {
      throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
    }
  }

  void SynthesisImager::pauseForUserInteraction()
  {
    LogIO os( LogOrigin("SISkyModel","pauseForUserInteraction",WHERE) );

    os << "Waiting for interactive clean feedback" << LogIO::POST;

    /* This call will make sure that the current values of loop control are
       available in the GUI and will not return until the user hits the
       button */
    itsLoopController->waitForInteractiveInput();
    // UUU comment out the above line to test if plumbing around interaction is OK.
    
    Int nmappers = itsMappers.nMappers();
    for(Int mp=0;mp<nmappers;mp++)
      {
	TempImage<Float> dispresidual, dispmask;
	// Memory for these image copies are allocated inside the SIMapper
	itsMappers.getMapper(mp)->getCopyOfResidualAndMask( dispresidual, dispmask );

	///// Send dispresidual and dispmask to the GUI.
	///// Receive dispmask back from the GUI ( on click-to-set-mask for this field )

	itsMappers.getMapper(mp)->setMask( dispmask );
      }
    
  }// end of pauseForUserInteraction


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  
  // Build the Image coordinate system.  TODO : Replace with Imager2::imagecoordinates2()
  CountedPtr<CoordinateSystem> SynthesisImager::buildImageCoordinateSystem(String phasecenter, 
                                                                           Double cellx, Double celly, 
                                                                           uInt imx, uInt imy,
                                                                           uInt npol, uInt nchan)
  {
    
    // Set up Direction Coordinate

    // Cell Size
    Vector<Double> deltas(2);
    deltas(0) = -1* cellx / 3600.0 * 3.14158 / 180.0; // 10 arcsec in radians
    deltas(1) = celly / 3600.0 * 3.14158 / 180.0; // 10 arcsec in radians

    // Direction of Image Center
    MDirection mDir;
    String tmpA,tmpB,tmpC;
    std::istringstream iss(phasecenter);
    iss >> tmpA >> tmpB >> tmpC;
    casa::Quantity tmpQA, tmpQB;
    casa::Quantity::read(tmpQA, tmpA);
    casa::Quantity::read(tmpQB, tmpB);
    if(tmpC.length() > 0){
      casa::MDirection::Types theRF;
      casa::MDirection::getType(theRF, tmpC);
      mDir = casa::MDirection (tmpQA, tmpQB, theRF);
    } else {
      mDir = casa::MDirection (tmpQA, tmpQB);
    }
    //    MVDirection mvPhaseCenter( mDir.getAngle() );
    // MVAngle ra = mvPhaseCenter.get()(0);
    //  //ra(0.0);
      // MVAngle dec = mvPhaseCenter.get()(1);
    //    MVAngle ra(mDir.getAngle().getValue()(0));
    // MVAngle dec(mDir.getAngle().getValue()(1));

    Vector<Double> refCoord(2);
    //    refCoord(0) = ra.get().getValue();
    //  refCoord(1) = dec;
    refCoord(0) = mDir.getAngle().getValue()(0);
    refCoord(1) = mDir.getAngle().getValue()(1);

    // Reference pixel
    Vector<Double> refPixel(2); 
    refPixel(0) = Double(imx / 2);
    refPixel(1) = Double(imy / 2);

    // Projection
    Projection projection(Projection::SIN);

    // Not sure....
    Matrix<Double> xform(2,2);
    xform=0.0;
    xform.diagonal()=1.0;

    DirectionCoordinate myRaDec(MDirection::Types(mDir.getRefPtr()->getType()),
                                projection,
                                refCoord(0), refCoord(1),
                                deltas(0), deltas(1),
                                xform,
                                refPixel(0), refPixel(1));


    // Set up Stokes Coordinate
    Vector<Int> whichStokes(npol);
    whichStokes[0] = Stokes::I;
    if(npol>1) whichStokes[1] = Stokes::V;
    StokesCoordinate myStokes(whichStokes);
    
    // Set up Spectral Coordinate
    //SpectralCoordinate* mySpectral=0;
    MFrequency::Types imfreqref=MFrequency::REST;
    Vector<Double> chanFreq( nchan );
    for(uInt ch=0;ch<nchan;ch++)
      {
        chanFreq[ch] = 1.0e+09 + (Double)ch * 1.0e+06;
      }
    Double restFreq = 1.0e+09;
    //mySpectral = new SpectralCoordinate(imfreqref, chanFreq, restFreq);
    SpectralCoordinate mySpectral(imfreqref, chanFreq, restFreq);
    

    CountedPtr<CoordinateSystem> coordSys;
    coordSys = new CoordinateSystem();
    coordSys->addCoordinate(myRaDec);
    coordSys->addCoordinate(myStokes);
    coordSys->addCoordinate(mySpectral);
    //if(mySpectral) delete mySpectral;

    return coordSys;
  }// end of buildImageCoordinateSystem


} //# NAMESPACE CASA - END

