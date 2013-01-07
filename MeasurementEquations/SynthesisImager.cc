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
				       itsCurrentCoordSys(NULL),
                                       itsCurrentImageShape(IPosition()),
                                       itsCurrentImageName(""),
				       itsSkyEquation(SISkyEquation()),
				       itsUseScratch(True)
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
	
	if( selpars.isDefined("usescratch") ) { selpars.get( RecordFieldId("usescratch"), itsUseScratch ); }
	
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
    uInt nchan=1,npol=1; 
    //imx=1,imy=1;
    Vector<Int> imsize(2);
    String phasecenter =  "19:59:28.500 +40.44.01.50";
    Double cellx=10.0,celly=10.0;

    std::string imagename, partimagename;
    try {
      // TODO : If critical params are unspecified, throw exceptions.
      // TODO : If they're the wrong data type, throw exceptions.

      if( impars.isDefined("imagename") )  // A single string
	{ imagename = impars.asString( RecordFieldId("imagename")); }
      else
	{throw( AipsError("imagename not specified")); }

      if( impars.isDefined("nchan") ) // A single integer
	{ impars.get( RecordFieldId("nchan") , nchan ); }
      else
	{throw( AipsError("nchan not specified")); }

      if( impars.isDefined("imsize") ) // An array with 2 integers
	{ 
          impars.get( RecordFieldId("imsize") , imsize ); 
        }
      else
	{throw( AipsError("imsize not specified")); }

      // Read and interpret input parameters.
    } catch(AipsError &x)
      {
	throw( AipsError("Error in reading input image-parameters: "+x.getMesg()) );
      }

    try
      {
	
	itsCurrentCoordSys = buildImageCoordinateSystem(phasecenter, 
                                                        cellx, celly, (uInt)imsize[0], (uInt)imsize[1], npol, nchan );
        itsCurrentImageShape = IPosition(4,(uInt)imsize[0],(uInt)imsize[1],npol,nchan);

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
  
  void SynthesisImager::initMapper()
  {
    LogIO os( LogOrigin("SynthesisImager","initMapper", WHERE) );
    os << "Construct a Mapper from the current FTMachine and Deconvolver, and allocate Image Memory" << LogIO::POST;
    try
      {

	itsMappers.addMapper( itsCurrentImageName, itsCurrentFTMachine, itsCurrentCoordSys, itsCurrentImageShape);

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing Mapper : "+x.getMesg()) );
      }
    
    
  }//end of initMapper
  
   /* This method makes all decisions about how the major cycle should 
       execute.  The output of this is passed to the runMajorCycle method
    */
  // This needs to be re-implemented somewhere else.
  /*
  Record SynthesisImager::getMajorCycleControls()
  {
     LogIO os( LogOrigin("SynthesisImager","getMajorCycleControls",WHERE) );
    Record returnRecord;

    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

        // TODO : Check if input model images exist, and setUpdatedModelFlag.
        //             This may be unnecessary now, but check.
        Bool startmodel = False;

 	if(itsLoopController->getCompletedNiter()==0 &&
           startmodel )
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
	    if(itsUseScratch==True)
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
  */

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
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  
  // Build the Image coordinate system.  TODO : Replace with Imager2::imagecoordinates2()
  CountedPtr<CoordinateSystem> SynthesisImager::buildImageCoordinateSystem(String phasecenter, 
                                                                           Double cellx, Double celly, 
                                                                           uInt imx, uInt imy,
                                                                           uInt npol, uInt nchan)
  {
    
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

    Vector<Double> refCoord(2);
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

    // Set up direction coordinate
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
    MFrequency::Types imfreqref=MFrequency::REST;
    Vector<Double> chanFreq( nchan );
    for(uInt ch=0;ch<nchan;ch++)
      {
        chanFreq[ch] = 1.0e+09 + (Double)ch * 1.0e+06;
      }
    Double restFreq = 1.0e+09;
    SpectralCoordinate mySpectral(imfreqref, chanFreq, restFreq);
    

    CountedPtr<CoordinateSystem> coordSys;
    coordSys = new CoordinateSystem();
    coordSys->addCoordinate(myRaDec);
    coordSys->addCoordinate(myStokes);
    coordSys->addCoordinate(mySpectral);

    return coordSys;
  }// end of buildImageCoordinateSystem


} //# NAMESPACE CASA - END

