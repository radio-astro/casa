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
#include <casa/OS/HostInfo.h>
#include <casa/OS/Path.h>

#include <lattices/Lattices/LCBox.h>

#include <measures/Measures/MeasTable.h>

#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>


#include <synthesis/ImagerObjects/SIIterBot.h>
#include <synthesis/ImagerObjects/SynthesisImager.h>
#include <synthesis/ImagerObjects/SIMapper.h>
#include <synthesis/MSVis/VisSetUtil.h>
#include <synthesis/TransformMachines/GridFT.h>
#include <synthesis/TransformMachines/WPConvFunc.h>
#include <synthesis/TransformMachines/WProjectFT.h>


#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
//#include <casadbus/utilities/BusAccess.h>
//#include <casadbus/session/DBusSession.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  SynthesisImager::SynthesisImager() : itsMappers(SIMapperCollection()), 
				       itsCurrentFTMachine(NULL), 
				       itsCurrentImages(NULL),
				       writeAccess_p(True), mss_p(0), vi_p(0)
  {
     nx_p=1000;
     ny_p=1000;
     cellx_p=Quantity(1, "arcsec");
     celly_p=Quantity(1,"arcsec");
     distance_p=Quantity(0, "m");
     stokes_p="I";
     phasecenter_p=MDirection(Quantity(0.0, "deg"),Quantity(0.0, "deg")) ;
     nchan_p=1;
     freqStart_p=Quantity(0, "Hz");
     freqStep_p=Quantity(1, "Hz");
     freqFrame_p=MFrequency::LSRK;
     freqFrameValid_p=False;
     //mLocation_p=MPosition(Quantity(0., "deg"), Quantity(0, "deg"), Quantity(0.0,"m"));
     mLocation_p=MPosition();
     wprojPlanes_p=1;
     useAutocorr_p=False;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  SynthesisImager::~SynthesisImager() 
  {
    LogIO os( LogOrigin("SynthesisImager","destructor",WHERE) );
    os << LogIO::DEBUGGING << "SynthesisImager destroyed" << LogIO::POST;
    for (uInt k=0; k < mss_p.nelements(); ++k){
      delete mss_p[k];
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  Bool SynthesisImager::selectData(const String& msname, const String& spw, const String& field, const String& taql,  const String& antenna, const String& uvdist, const String& scan, const String& obs, const String& timestr, const Bool usescratch, const Bool readonly){
    LogIO os( LogOrigin("SynthesisImager","selectData",WHERE) );
    //Respect the readonly flag...necessary for multi-process access
    MeasurementSet thisms(msname, TableLock(TableLock::AutoNoReadLocking), 
			  readonly ? Table::Old : Table::Update);
    thisms.setMemoryResidentSubtables (MrsEligibility::defaultEligible());
    //if you want to use scratch col...make sure they are there
    if(usescratch && !readonly){
      VisSetUtil::addScrCols(thisms, True, False, True, False);
      VisModelData::clearModel(thisms);
    }
    //Some MSSelection 
    MSSelection thisSelection;
    if(field != ""){
      thisSelection.setFieldExpr(field);
      os << "Selecting on fields : " << field << LogIO::POST;
    }
    if(spw != ""){
	thisSelection.setSpwExpr(spw);
	os << "Selecting on spectral windows expression :"<< spw  << LogIO::POST;
    }
    
    if(antenna != ""){
      Vector<String> antNames(1, antenna);
      // thisSelection.setAntennaExpr(MSSelection::nameExprStr( antNames));
      thisSelection.setAntennaExpr(antenna);
      os << "Selecting on antenna names : " << antenna << LogIO::POST;
	
    }            
    if(timestr != ""){
	thisSelection.setTimeExpr(timestr);
	os << "Selecting on time range : " << timestr << LogIO::POST;	
      }
    if(uvdist != ""){
      thisSelection.setUvDistExpr(uvdist);
      os << "Selecting on uvdist : " << uvdist << LogIO::POST;	
    }
    if(scan != ""){
      thisSelection.setScanExpr(scan);
      os << "Selecting on scan : " << scan << LogIO::POST;	
    }
    if(obs != ""){
      thisSelection.setObservationExpr(obs);
      os << "Selecting on Observation Expr : " << obs << LogIO::POST;	
    }
    if(taql != ""){
	thisSelection.setTaQLExpr(taql);
	os << "Selecting via TaQL : " << taql << LogIO::POST;	
    }
    TableExprNode exprNode=thisSelection.toTableExprNode(&thisms);
    if(!(exprNode.isNull())){
      mss_p.resize(mss_p.nelements()+1, False, True);
      mss_p[mss_p.nelements()-1]=new const MeasurementSet(thisms(exprNode));
    }
    else{
      throw(AipsError("Selection for given MS "+msname+" is invalid"));
    }
    //We should do the select channel here for  the VI construction later
    //Need a cross check between channel selection and ms
    // replace below if/when viFrquencySelectionUsingChannels takes in a MSSelection
    // rather than the following gymnastics
    {
      Matrix<Int> chanlist = thisSelection.getChanList(mss_p[mss_p.nelements()-1]);
      
      IPosition shape = chanlist.shape();
      uInt nSelections = shape[0];
      Int spw,chanStart,chanEnd,chanStep,nchan;
      vi::FrequencySelectionUsingChannels channelSelector;
      for(uInt k=0; k < nSelections; ++k)
	{
	  
	  spw = chanlist(k,0);
	  
	  // channel selection 
	  chanStart = chanlist(k,1);
	  chanEnd = chanlist(k,2);
	  chanStep = chanlist(k,3);
	  nchan = chanEnd-chanStart+1;
	  channelSelector.add (spw, chanStart, nchan,chanStep);
	}

      fselections_p.add(channelSelector);

    }
    writeAccess_p=writeAccess_p && !readonly;
    return True;

  }

 


  // Make this read in a list of MS's and selection pars....
  void  SynthesisImager::selectData(Record selpars)
  {
    LogIO os( LogOrigin("SynthesisImager","selectData",WHERE) );
    
    Vector<String> mslist(0),fieldlist(0),spwlist(0);
    Bool useScratch=False;

    try
      {
	// TODO : If critical params are unspecified, throw exceptions.
	if( selpars.isDefined("vis") ) { selpars.get( RecordFieldId("vis") , mslist ); }
	if( selpars.isDefined("field") ) { selpars.get( RecordFieldId("field") , fieldlist ); }
	if( selpars.isDefined("spw") ) { selpars.get( RecordFieldId("spw") , spwlist ); }
	
	
	uInt nms = mslist.nelements();
	if(fieldlist.nelements() != nms){os << LogIO::EXCEPTION << "Need " << nms << " field selection strings, one for each specified MS" << LogIO::POST; }
	if(spwlist.nelements() != nms){os << LogIO::EXCEPTION << "Need " << nms << " spw selection strings, one for each specified MS" << LogIO::POST; }
	if( selpars.isDefined("usescratch") ) { selpars.get( RecordFieldId("usescratch"), useScratch ); }
	for(uInt sel=0; sel<nms; sel++)
	  {
	    os << "MS : " << mslist[sel];
	    os << "   Selection : spw='" << spwlist[sel] << "'";
	    os << " field='" << fieldlist[sel] << "'" << LogIO::POST;
	    selectData(mslist[sel], spwlist[sel], fieldlist[sel],""/*taql*/,  ""/*antenna*/, ""/*uvdist*/, ""/*scan*/, 
		       ""/*obs*/, ""/*timestr*/, useScratch, False);
	  }
	
	
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading selection parameter record : "+x.getMesg()) );
      }

    // Set up Visibility Iterator here.
    try
      {
	os << "Do MS-Selection and Setup vi/vb" << LogIO::POST;
	//itsVisSet = createVisSet(/* parameters */);
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in creating SkyEquation : "+x.getMesg()) );
      }
    
  }// end of selectData()


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Bool SynthesisImager::defineImage(const String& imagename, const Int nx, const Int ny,
				    const Quantity& cellx, const Quantity& celly,
				    const String& stokes,
				    const MDirection& phaseCenter, 
				    const Int nchan,
				    const Quantity&freqStart,
				    const Quantity& freqStep, 
				    const Vector<Quantity>& restFreq,
				    const Int facets,
				    const String& ftmachine, 
				    const Projection& projection,
				    const Quantity& distance,
				    const MFrequency::Types& freqFrame,
				    const Bool trackSource, const MDirection& 
				    trackDir){
    LogIO os( LogOrigin("SynthesisImager","selectData",WHERE) );
    if(mss_p.nelements() ==0)
      throw(AipsError("SelectData has to be run before defineImage"));
    nx_p=nx;
    ny_p=ny;
    cellx_p=cellx;
    celly_p=celly;
    distance_p=distance;
    stokes_p=stokes;
    nchan_p=nchan;
    freqStart_p=freqStart;
    freqStep_p=freqStep;
    freqFrame_p=freqFrame;
    phasecenter_p=phaseCenter;
    CoordinateSystem csys=buildCoordSys(phaseCenter, cellx, celly, nx, ny, stokes, projection, nchan, freqStart, freqStep, restFreq);
    appendToMapperList(imagename,  csys,  ftmachine, distance, facets); 
    
    return True;
  }
 

  // Construct Image Coordinates
  void  SynthesisImager::defineImage(Record impars)
  {
    LogIO os( LogOrigin("SynthesisImager","defineImage",WHERE) );
    
    /* Use the image name to create a unique service name */
    uInt nchan=1,npol=1; 
    //imx=1,imy=1;
    Vector<Int> imsize(2);
    String phasecenter =  "19:59:28.500 +40.44.01.50";
    Double cellx=10.0,celly=10.0;

    String imagename("");
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
	  if(imsize.nelements() != 2) 
	    {
	      throw( AipsError("imsize must be an array of two integers") );
	    }
        }
      else
	{throw( AipsError("imsize not specified")); }

      // Read and interpret input parameters.
    } catch(AipsError &x)
      {
	throw( AipsError("Error in reading input image-parameters: "+x.getMesg()) );
      }

    os << "Define Image Coordinates and allocate Memory for : " << imagename<< LogIO::POST;

    try
      {
	
	CountedPtr<CoordinateSystem> coordsys = buildImageCoordinateSystem(phasecenter, 
									   cellx, celly, (uInt)imsize[0], (uInt)imsize[1], npol, nchan );
        IPosition imageshape(4,(uInt)imsize[0],(uInt)imsize[1],npol,nchan);
	itsCurrentImages = new SIImageStore(imagename, *coordsys, imageshape);

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing image coordinate system and allocating memory : "+x.getMesg()) );
      }
    
    
  }// end of defineImage

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  void  SynthesisImager::setupImaging(Record gridpars)
  {
    LogIO os( LogOrigin("SynthesisImager","setupImaging",WHERE) );

    String ftmname("");
    String modelname("");
    try
      {
	// TODO : Parse FT-machine-related parameters.
	if( gridpars.isDefined("ftmachine") )  // A single string
	  { ftmname = gridpars.asString( RecordFieldId("ftmachine")); }
	
	if( gridpars.isDefined("modelname") )  // A single string
	  { modelname = gridpars.asString( RecordFieldId("modelname")); }


      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading Imaging Parameters : "+x.getMesg()) );
      }
    
    os << "Set Imaging Options. Construct FTMachine of type " << ftmname << LogIO::POST;
    
    try
      {
	// Set the model image for prediction
	if( modelname.length()>0 && !itsCurrentImages.null() )
	  {
	    os << "Setting " << modelname << " as starting model for prediction " << LogIO::POST;
	    itsCurrentImages->setModelImage( modelname );
	  }

	// TODO : Set up the FT-Machine. Send in parameters here...
	//itsCurrentFTMachine = createFTMachine(/* parameters */);



      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing FTMachine : "+x.getMesg()) );
      }
    
  }// end of setupImaging
  

  //////////////////////Reset the Mapper
  ////////////////////
  void SynthesisImager::resetMapper(){
    ////reset code
  }



  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void SynthesisImager::initMapper()
  {
    LogIO os( LogOrigin("SynthesisImager","initMapper", WHERE) );
    os << "Construct mapper " << itsMappers.nMappers() 
       << " for image : " << itsCurrentImages->getName();
    os << " and ftm : " << "FTMNAME"; // itsCurrentFTMachine->name() 
    os << LogIO::POST;
    try
      {

	itsMappers.addMapper( String("basetype"), itsCurrentImages, itsCurrentFTMachine);

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing Mapper : "+x.getMesg()) );
      }
    
    
  }//end of initMapper
  

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  void SynthesisImager::executeMajorCycle(Record& /*controlRecord*/)
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );

    os << "-------------------------------------------------------------------------------------------------------------" << LogIO::POST;

    try
      {    
	runMajorCycle();

	itsMappers.releaseImageLocks();

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in running Major Cycle : "+x.getMesg()) );
      }    

    os << "-------------------------------------------------------------------------------------------------------------" << LogIO::POST;

  }// end of runMajorCycle
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////
  ////////////This should be called  at each defineimage
    void SynthesisImager::appendToMapperList(String imagename,  CoordinateSystem& csys, String ftmachine,  Quantity distance, Int facets){
    if(facets <1)
      facets=1;
    Int nIm=facets*facets;
    facets_p=facets;
    CountedPtr<SIImageStore> imstor;
    


    if (nIm < 2){
      imstor=new SIImageStore(imagename, csys, IPosition(4, nx_p, ny_p, nstokes_p, nchan_p)); 
    }
    else{
      if(!unFacettedImStore_p.null())
	throw(AipsError("A facetted Image has already been set"));
      unFacettedImStore_p=new SIImageStore(imagename, csys, IPosition(4, nx_p, ny_p, nstokes_p, nchan_p)); 
    }

     for (Int facet=0; facet< nIm; ++facet){
       if(nIm > 1)
	 imstor=unFacettedImStore_p->getFacetImageStore(facet, nIm);
       CountedPtr<FTMachine> ftm, iftm;
       createFTMachine(ftm, iftm, ftmachine);
       Int id=itsMappers.nMappers();
       CountedPtr<SIMapperBase> thismap=new SIMapper(imstor, ftm, iftm, id);
       itsMappers.addMapper(thismap);
     }

   
    
  }

  /////////////////////////
  ////////////////////////
  CoordinateSystem SynthesisImager::buildCoordSys(const MDirection& phasecenter, const Quantity& cellx, const Quantity& celly, const Int nx, const Int ny, const String& stokes, const Projection& projection, const Int nchan, const Quantity& freqStart, const Quantity& freqStep, const Vector<Quantity>& restFreq){
    LogIO os( LogOrigin("SynthesisImager","build",WHERE) );
    // At this stage one ms at least should have been assigned
    ROMSColumns msc(*mss_p[0]);
    MVDirection mvPhaseCenter(phasecenter.getAngle());
    // Normalize correctly
    MVAngle ra=mvPhaseCenter.get()(0);
    ra(0.0);
    MVAngle dec=mvPhaseCenter.get()(1);
    Vector<Double> refCoord(2);
    refCoord(0)=ra.get().getValue();    
    refCoord(1)=dec;
    Vector<Double> refPixel(2); 
    refPixel(0) = Double(nx / 2);
    refPixel(1) = Double(ny/ 2);
    //defining observatory...needed for position on earth
    String telescop = msc.observation().telescopeName()(0);
    MEpoch obsEpoch = msc.timeMeas()(0);
    MPosition obsPosition;
    if(!(MeasTable::Observatory(obsPosition, telescop))){
      os << LogIO::WARN << "Did not get the position of " << telescop 
	 << " from data repository" << LogIO::POST;
      os << LogIO::WARN 
	 << "Please contact CASA to add it to the repository."
	 << LogIO::POST;
      os << LogIO::WARN << "Frequency conversion will not work " << LogIO::POST;
      freqFrameValid_p = False;
    }
    else{
      mLocation_p = obsPosition;
      freqFrameValid_p = True;
    }
     //Make sure frame conversion is switched off for REST frame data.
    freqFrameValid_p=freqFrameValid_p && (freqFrame_p != MFrequency::REST);
    Vector<Double> deltas(2);
    deltas(0)=-cellx_p.get("rad").getValue();
    deltas(1)=celly_p.get("rad").getValue();
    Matrix<Double> xform(2,2);
    xform=0.0;xform.diagonal()=1.0;
    DirectionCoordinate
      myRaDec(MDirection::Types(phasecenter_p.getRefPtr()->getType()),
	      projection,
	      refCoord(0), refCoord(1),
	      deltas(0), deltas(1),
	      xform,
	      refPixel(0), refPixel(1));

    SpectralCoordinate mySpectral(freqFrameValid_p ? MFrequency::LSRK : freqFrame_p, freqStart, freqStep, 0, restFreq.nelements() >0 ? restFreq[0]: Quantity(0.0, "Hz"));
    for (uInt k=1 ; k < restFreq.nelements(); ++k)
      mySpectral.setRestFrequency(restFreq[k].getValue("Hz"));
    
    Vector<Int> whichStokes = decideNPolPlanes(stokes);
    if(whichStokes.nelements()==0)
      throw(AipsError("Stokes selection of " +stokes+ " is invalid"));
    nstokes_p=whichStokes.nelements();
    StokesCoordinate myStokes(whichStokes);
    //Set Observatory info
    ObsInfo myobsinfo;
    myobsinfo.setTelescope(telescop);
    myobsinfo.setPointingCenter(mvPhaseCenter);
    myobsinfo.setObsDate(obsEpoch);
    myobsinfo.setObserver(msc.observation().observer()(0));

    CoordinateSystem csys;
    csys.addCoordinate(myRaDec);
    csys.addCoordinate(myStokes);
    csys.addCoordinate(mySpectral);
    csys.setObsInfo(myobsinfo);
    return csys;
  }

  //////////////////////
  ////////////////////Function to get the pol string to stokes vector
  Vector<Int> SynthesisImager::decideNPolPlanes(const String& stokes){
    Vector<Int> whichStokes(0);
    if(stokes=="I" || stokes=="Q" || stokes=="U" || stokes=="V" || 
       stokes=="RR" ||stokes=="LL" || 
       stokes=="XX" || stokes=="YY" ) {
      whichStokes.resize(1);
      whichStokes(0)=Stokes::type(stokes);
    }
    else if(stokes=="IV" || stokes=="IQ" || 
              stokes=="RRLL" || stokes=="XXYY" ||
	    stokes=="QU" || stokes=="UV"){
      whichStokes.resize(2);
      
      if(stokes=="IV"){ whichStokes[0]=Stokes::I; whichStokes[1]=Stokes::V;}
      else if(stokes=="IQ"){whichStokes[0]=Stokes::I; whichStokes[1]=Stokes::Q;}
      else if(stokes=="RRLL"){whichStokes[0]=Stokes::RR; whichStokes[1]=Stokes::LL;}
      else if(stokes=="XXYY"){whichStokes[0]=Stokes::XX; whichStokes[1]=Stokes::YY; }
      else if(stokes=="QU"){whichStokes[0]=Stokes::Q; whichStokes[1]=Stokes::U; }
      else if(stokes=="UV"){ whichStokes[0]=Stokes::U; whichStokes[1]=Stokes::V; }
	
    }
  
    else if(stokes=="IQU" || stokes=="IUV") {
      whichStokes.resize(3);
      if(stokes=="IUV")
	{whichStokes[0]=Stokes::I; whichStokes[1]=Stokes::U; whichStokes[2]=Stokes::V;}
      else
	{whichStokes[0]=Stokes::I; whichStokes[1]=Stokes::Q; whichStokes[2]=Stokes::U;}
    }
    else if(stokes=="IQUV"){
      whichStokes.resize(4);
      whichStokes(0)=Stokes::I; whichStokes(1)=Stokes::Q;
      whichStokes(2)=Stokes::U; whichStokes(3)=Stokes::V;
    }
      
    return whichStokes;
  }

  ////////////////////////////////////////////
  //////////////////////////////////////////////
  
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


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  // Make the FT-Machine and related objects (cfcache, etc.)
  void SynthesisImager::createFTMachine(CountedPtr<FTMachine>& theFT, CountedPtr<FTMachine>& theIFT, const String& ftname)
  {
    LogIO os( LogOrigin("SynthesisImager","createFTMachine",WHERE));
    if(ftname=="GridFT"){
      if(facets_p >1){
	theFT=new GridFT(cache_p, tile_p, gridFunction_p, mLocation_p, phasecenter_p, padding_p, useAutocorr_p, useDoublePrec_p);
	theIFT=new GridFT(cache_p, tile_p, gridFunction_p, mLocation_p, phasecenter_p, padding_p, useAutocorr_p, useDoublePrec_p);

      }
      else{
	theFT=new GridFT(cache_p, tile_p, gridFunction_p, mLocation_p, padding_p, useAutocorr_p, useDoublePrec_p);
      theIFT=new GridFT(cache_p, tile_p, gridFunction_p, mLocation_p, padding_p, useAutocorr_p, useDoublePrec_p);
      }
    }
    else if(ftname== "WProjectFT"){
      theFT=new WProjectFT(wprojPlanes_p,  mLocation_p,
			   cache_p/2, tile_p, useAutocorr_p, padding_p, useDoublePrec_p);
      theIFT=new WProjectFT(wprojPlanes_p,  mLocation_p,
			   cache_p/2, tile_p, useAutocorr_p, padding_p, useDoublePrec_p);
      CountedPtr<WPConvFunc> sharedconvFunc= new WPConvFunc();
      static_cast<WProjectFT &>(*theFT).setConvFunc(sharedconvFunc);
      static_cast<WProjectFT &>(*theFT).setConvFunc(sharedconvFunc);
      
    }
    /* else if(ftname== "MosaicFT"){

       }*/
    
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  // Do MS-Selection and set up vi/vb. 
  // Only this functions needs to know anything about the MS 
  void SynthesisImager::createVisSet()
  {
    LogIO os( LogOrigin("SynthesisImager","createVisSet",WHERE) );
    if(mss_p.nelements() != uInt(fselections_p.size()) && (fselections_p.size() !=0)){
      throw(AipsError("Discrepancy between Number of MSs and Frequency selections"));
    }
    vi_p=new vi::VisibilityIterator2(mss_p, vi::SortColumns(), writeAccess_p); 
    if(fselections_p.size() !=0)
      vi_p->setFrequencySelection (fselections_p);
    //return *vi_p;
  }// end of createVisSet

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  void SynthesisImager::runMajorCycle()
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );

    Int nmappers = itsMappers.nMappers();
    
    os << "Run major cycle for " << nmappers << " image(s) : " 
       << itsMappers.getImageNames() << LogIO::POST;

    ///////// (1) Initialize all the FTMs.
    for(Int mp=0;mp<nmappers;mp++)
      {
	// vb.selectChannel(....)
	itsMappers.initializeDegrid(mp);
	itsMappers.initializeGrid(mp);
      }

    ////////// (2) Iterate through visbuffers, degrid, subtract, grid

    //for ( vi.originChunks(); vi.moreChunks(); vi.nextChunk() )
      {
	//for( vi.origin(); vi.more(); vi++ )
	  {
	    for(Int mp=0;mp<nmappers;mp++)
	      {
		itsMappers.degrid(mp /* ,vb */);
		// resultvb.modelVisCube += vb.modelVisCube()
	      }
	    
	    // resultvb.visCube -= resultvb.modelvisCube()
	    
	    // save model either in the column, or in the record. 
	    // Access the FTM record as    rec=mappers[mp]->getFTMRecord();
	    
	    for(Int mp=0;mp<nmappers;mp++)
	      {
		itsMappers.grid(mp /* ,vb */);
	      }
	  }// end of for vb.
      }// end of vi.chunk iterations
      

      /////////// (3) Finalize Mappers.
      for(Int mp=0;mp<nmappers;mp++)
	{
	  itsMappers.finalizeDegrid(mp);
	  itsMappers.finalizeGrid(mp);
	}
      
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


} //# NAMESPACE CASA - END

