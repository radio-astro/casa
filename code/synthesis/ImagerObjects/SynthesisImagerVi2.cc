//# SynthesisImagerVi2.cc: Implementation of SynthesisImager.h
//# Copyright (C) 1997-2016
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
#include <casa/System/ProgressMeter.h>

#include <casa/OS/DirectoryIterator.h>
#include <casa/OS/File.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/Path.h>
//#include <casa/OS/Memory.h>

#include <lattices/LRegions/LCBox.h>

#include <measures/Measures/MeasTable.h>

#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MSSel/MSSelection.h>


#include <synthesis/ImagerObjects/SIIterBot.h>
#include <synthesis/ImagerObjects/SynthesisImagerVi2.h>

#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>
#include <synthesis/ImagerObjects/SIImageStore.h>
#include <synthesis/ImagerObjects/SIImageStoreMultiTerm.h>

#include <synthesis/MeasurementEquations/ImagerMultiMS.h>
#include <synthesis/MeasurementEquations/VPManager.h>
#include <msvis/MSVis/MSUtil.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <msvis/MSVis/VisImagingWeight.h>

#include <synthesis/TransformMachines2/GridFT.h>
#include <synthesis/TransformMachines2/WPConvFunc.h>
#include <synthesis/TransformMachines2/WProjectFT.h>
#include <synthesis/TransformMachines2/VisModelData.h>
#include <synthesis/TransformMachines2/AWProjectFT.h>
#include <synthesis/TransformMachines2/HetArrayConvFunc.h>
#include <synthesis/TransformMachines2/MosaicFTNew.h>
#include <synthesis/TransformMachines2/MultiTermFTNew.h>
#include <synthesis/TransformMachines2/AWProjectWBFTNew.h>
#include <synthesis/TransformMachines2/AWConvFunc.h>
#include <synthesis/TransformMachines2/AWConvFuncEPJones.h>
#include <synthesis/TransformMachines2/NoOpATerm.h>
#include <synthesis/TransformMachines/WProjectFT.h>
#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casacore/casa/Utilities/Regex.h>
#include <casacore/casa/OS/Directory.h>
#include <msvis/MSVis/VisibilityIteratorImpl2.h>
//#include <casadbus/utilities/BusAccess.h>
//#include <casadbus/session/DBusSession.h>

#include <sys/types.h>
#include <unistd.h>


using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

  SynthesisImagerVi2::SynthesisImagerVi2() : SynthesisImager(), mss_p(0), vi_p(0) {


  }

  SynthesisImagerVi2::~SynthesisImagerVi2(){
    for (uInt k=0; k < mss_p.nelements(); ++k){
      if(mss_p[k])
	delete mss_p[k];
    }
  }

  Bool SynthesisImagerVi2::selectData(const SynthesisParamsSelect& selpars){
 LogIO os( LogOrigin("SynthesisImager","selectData",WHERE) );

    try
      {

    //Respect the readonly flag...necessary for multi-process access
    MeasurementSet thisms(selpars.msname, TableLock(TableLock::AutoNoReadLocking),
				selpars.readonly ? Table::Old : Table::Update);
    thisms.setMemoryResidentSubtables (MrsEligibility::defaultEligible());

    useScratch_p=selpars.usescratch;
    readOnly_p = selpars.readonly;
    //    cout << "**************** usescr : " << useScratch_p << "     readonly : " << readOnly_p << endl;
    //if you want to use scratch col...make sure they are there
    if(selpars.usescratch && !selpars.readonly){
      VisSetUtil::addScrCols(thisms, True, False, True, False);
      refim::VisModelData::clearModel(thisms);
    }
    if(!selpars.incrmodel && !selpars.usescratch && !selpars.readonly)
      refim::VisModelData::clearModel(thisms, selpars.field, selpars.spw);

    os << "MS : " << selpars.msname << " | ";

    //Some MSSelection 
    //If everything is empty (which is valid) it will throw an exception..below
    //So make sure the main defaults are not empy i.e field and spw
    MSSelection thisSelection;
    if(selpars.field != ""){
      thisSelection.setFieldExpr(selpars.field);
      os << "Selecting on fields : " << selpars.field << " | " ;//LogIO::POST;
    }else
      thisSelection.setFieldExpr("*");
    if(selpars.spw != ""){
	thisSelection.setSpwExpr(selpars.spw);
	os << "Selecting on spw :"<< selpars.spw  << " | " ;//LogIO::POST;
    }else
      thisSelection.setSpwExpr("*");
    
    if(selpars.antenna != ""){
      Vector<String> antNames(1, selpars.antenna);
      // thisSelection.setAntennaExpr(MSSelection::nameExprStr( antNames));
      thisSelection.setAntennaExpr(selpars.antenna);
      os << "Selecting on antenna names : " << selpars.antenna << " | " ;//LogIO::POST;
	
    }            
    if(selpars.timestr != ""){
	thisSelection.setTimeExpr(selpars.timestr);
	os << "Selecting on time range : " << selpars.timestr << " | " ;//LogIO::POST;	
      }
    if(selpars.uvdist != ""){
      thisSelection.setUvDistExpr(selpars.uvdist);
      os << "Selecting on uvdist : " << selpars.uvdist << " | " ;//LogIO::POST;	
    }
    if(selpars.scan != ""){
      thisSelection.setScanExpr(selpars.scan);
      os << "Selecting on scan : " << selpars.scan << " | " ;//LogIO::POST;	
    }
    if(selpars.obs != ""){
      thisSelection.setObservationExpr(selpars.obs);
      os << "Selecting on Observation Expr : " << selpars.obs << " | " ;//LogIO::POST;	
    }
    if(selpars.state != ""){
      thisSelection.setStateExpr(selpars.state);
      os << "Selecting on Scan Intent/State : " << selpars.state << " | " ;//LogIO::POST;	
    }
    // if(selpars.taql != ""){
    // 	thisSelection.setTaQLExpr(selpars.taql);
    // 	os << "Selecting via TaQL : " << selpars.taql << " | " ;//LogIO::POST;	
    // }
    os << "[Opened " << (readOnly_p?"in readonly mode":(useScratch_p?"with scratch model column":"with virtual model column"))  << "]" << LogIO::POST;
    TableExprNode exprNode=thisSelection.toTableExprNode(&thisms);
    if(!(exprNode.isNull()))
      {
	mss_p.resize(mss_p.nelements()+1, False, True);
    
	MeasurementSet thisMSSelected0 = MeasurementSet(thisms(exprNode));

	if(selpars.taql != "")
	  {
	    MSSelection mss0;
	    mss0.setTaQLExpr(selpars.taql);
	    os << "Selecting via TaQL : " << selpars.taql << " | " ;//LogIO::POST;	

	    TableExprNode tenWithTaQL=mss0.toTableExprNode(&thisMSSelected0);
	    MeasurementSet thisMSSelected1 = MeasurementSet(thisMSSelected0(tenWithTaQL));
	    //mss4vi_p[mss4vi_p.nelements()-1]=MeasurementSet(thisms(exprNode));
	    mss_p[mss_p.nelements()-1]=new MeasurementSet(thisMSSelected1);
	  }
	else
	  mss_p[mss_p.nelements()-1]=new MeasurementSet(thisMSSelected0);
	  
	os << "  NRows selected : " << (mss_p[mss_p.nelements()-1])->nrow() << LogIO::POST;
      }
    else{
      throw(AipsError("Selection for given MS "+selpars.msname+" is invalid"));
    }
    
    ///Channel selection
    {
      Matrix<Int> chanlist = thisSelection.getChanList(mss_p[mss_p.nelements()-1]);
      
      IPosition shape = chanlist.shape();
      uInt nSelections = shape[0];
      if(selpars.freqbeg==""){
	vi::FrequencySelectionUsingChannels channelSelector;

	channelSelector.add(thisSelection, mss_p[mss_p.nelements()-1]);

	fselections_p.add(channelSelector);
	 
      }
      else{
    	  Quantity freq;
    	  Quantity::read(freq, selpars.freqbeg);
    	  Double lowfreq=freq.getValue("Hz");
    	  Quantity::read(freq, selpars.freqend);
    	  Double topfreq=freq.getValue("Hz");
	  vi::FrequencySelectionUsingFrame channelSelector(selpars.freqframe);
    	  for(uInt k=0; k < nSelections; ++k)
    		  channelSelector.add(chanlist(k,0), lowfreq, topfreq);
    	  fselections_p.add(channelSelector);

      }
    }
    writeAccess_p=writeAccess_p && !selpars.readonly;
    createVisSet(writeAccess_p);

   
    //// Set the data column on which to operate
    // TT: added checks for the requested data column existace 
    //    cout << "Using col : " << selpars.datacolumn << endl;
    if( selpars.datacolumn.contains("data") || selpars.datacolumn.contains("obs") ) 
      {    if( thisms.tableDesc().isColumn("DATA") ) { datacol_p = FTMachine::OBSERVED; }
           else { os << LogIO::SEVERE <<"DATA column does not exist" << LogIO::EXCEPTION;}
      }
    else if( selpars.datacolumn.contains("corr") ) {    
      if( thisms.tableDesc().isColumn("CORRECTED_DATA") ) { datacol_p = FTMachine::CORRECTED; } 
      else 
	{
	  if( thisms.tableDesc().isColumn("DATA") ) { 
	    datacol_p = FTMachine::OBSERVED;
	    os << "CORRECTED_DATA column does not exist. Using DATA column instead" << LogIO::POST; 
	  }
	  else { 
	    os << LogIO::SEVERE <<"Neither CORRECTED_DATA nor DATA columns exist" << LogIO::EXCEPTION;
	  }
	}
	
      }
   
    else { os << LogIO::WARN << "Invalid data column : " << datacol_p << ". Using corrected (or observed if corrected doesn't exist)" << LogIO::POST;  datacol_p = thisms.tableDesc().isColumn("CORRECTED_DATA") ? FTMachine::CORRECTED : FTMachine::OBSERVED; }

    dataSel_p.resize(dataSel_p.nelements()+1, True);

    dataSel_p[dataSel_p.nelements()-1]=selpars;


    unlockMSs();
      }
    catch(AipsError &x)
      {
	unlockMSs();
	throw( AipsError("Error in selectData() : "+x.getMesg()) );
      }

    return True;



  }

Bool SynthesisImagerVi2::defineImage(SynthesisParamsImage& impars, 
			   const SynthesisParamsGrid& gridpars)
  {

    LogIO os( LogOrigin("SynthesisImagerVi2","defineImage",WHERE) );
    if(mss_p.nelements() ==0)
      os << "SelectData has to be run before defineImage" << LogIO::EXCEPTION;

    CoordinateSystem csys;
    CountedPtr<refim::FTMachine> ftm, iftm;


    try
      {

	os << "Define image coordinates for [" << impars.imageName << "] : " << LogIO::POST;

	csys = impars.buildCoordinateSystem( *vi_p );
	IPosition imshape = impars.shp();

	os << "Impars : start " << impars.start << LogIO::POST;
	os << "Shape : " << imshape << "Spectral : " << csys.spectralCoordinate().referenceValue() << " at " << csys.spectralCoordinate().referencePixel() << " with increment " << csys.spectralCoordinate().increment() << LogIO::POST;

	if( (itsMappers.nMappers()==0) || 
	    (impars.imsize[0]*impars.imsize[1] > itsMaxShape[0]*itsMaxShape[1]))
	  {
	    itsMaxShape=imshape;
	    itsMaxCoordSys=csys;
	  }
        itsNchan = imshape[3];
        itsCsysRec = impars.getcsys();
	/*
	os << "Define image  [" << impars.imageName << "] : nchan : " << impars.nchan 
	   //<< ", freqstart:" << impars.freqStart.getValue() << impars.freqStart.getUnit() 
	   << ", start:" << impars.start
	   <<  ", imsize:" << impars.imsize 
	   << ", cellsize: [" << impars.cellsize[0].getValue() << impars.cellsize[0].getUnit() 
	   << " , " << impars.cellsize[1].getValue() << impars.cellsize[1].getUnit() 
	   << LogIO::POST;
	*/
      }
    catch(AipsError &x)
      {
	os << "Error in building Coordinate System and Image Shape : " << x.getMesg() << LogIO::EXCEPTION;
      }

	
    try
      {
	os << "Set Gridding options for [" << impars.imageName << "] with ftmachine : " << gridpars.ftmachine << LogIO::POST;

	createFTMachine(ftm, iftm, gridpars.ftmachine, impars.nTaylorTerms, gridpars.mType, 
			gridpars.facets, gridpars.wprojplanes,
			gridpars.padding,gridpars.useAutoCorr,gridpars.useDoublePrec,
			gridpars.convFunc,
			gridpars.aTermOn,gridpars.psTermOn, gridpars.mTermOn,
			gridpars.wbAWP,gridpars.cfCache,gridpars.doPointing,
			gridpars.doPBCorr,gridpars.conjBeams,
			gridpars.computePAStep,gridpars.rotatePAStep,
			gridpars.interpolation, impars.freqFrameValid, 1000000000,  16, impars.stokes,
			impars.imageName);

      }
    catch(AipsError &x)
      {
	os << "Error in setting up FTMachine() : " << x.getMesg() << LogIO::EXCEPTION;
      }

    try
      {

		appendToMapperList(impars.imageName,  csys,  impars.shp(),
			   ftm, iftm,
			   gridpars.distance, gridpars.facets, gridpars.chanchunks,impars.overwrite,
			   gridpars.mType, impars.nTaylorTerms, impars.startModel);
	
	imageDefined_p=True;
      }
    catch(AipsError &x)
      {
	os << "Error in adding Mapper : "+x.getMesg() << LogIO::EXCEPTION;
      }

    return True;
  }


 Bool SynthesisImagerVi2::weight(const String& type, const String& rmode,
			       const Quantity& noise, const Double robust,
			       const Quantity& fieldofview,
			       const Int npixels, const Bool multiField,
			       const String& filtertype, const Quantity& filterbmaj,
			       const Quantity& filterbmin, const Quantity& filterbpa   )
  {
    LogIO os(LogOrigin("SynthesisImager", "weight()", WHERE));

       try {
    	//Int nx=itsMaxShape[0];
    	//Int ny=itsMaxShape[1];
	 Quantity cellx=Quantity(itsMaxCoordSys.increment()[0], itsMaxCoordSys.worldAxisUnits()[0]);
	 Quantity celly=Quantity(itsMaxCoordSys.increment()[1], itsMaxCoordSys.worldAxisUnits()[1]);
	 os << LogIO::NORMAL // Loglevel INFO
	    << "Set imaging weights : " ; //<< LogIO::POST;
	 
	 if (type=="natural") {
	   os << LogIO::NORMAL // Loglevel INFO
	      << "Natural weighting" << LogIO::POST;
	   imwgt_p=VisImagingWeight("natural");
	 }
      else if (type=="radial") {
	os << "Radial weighting" << LogIO::POST;
    	  imwgt_p=VisImagingWeight("radial");
      }
      else{
    	  if(!imageDefined_p)
    		  throw(AipsError("Need to define image"));
    	  Int nx=itsMaxShape[0];
    	  Int ny=itsMaxShape[1];
    	  Quantity cellx=Quantity(itsMaxCoordSys.increment()[0], itsMaxCoordSys.worldAxisUnits()[0]);
    	  Quantity celly=Quantity(itsMaxCoordSys.increment()[1], itsMaxCoordSys.worldAxisUnits()[1]);
    	  if(type=="superuniform"){
    		  if(!imageDefined_p) throw(AipsError("Please define image first"));
    		  Int actualNpix=npixels;
    		  if(actualNpix <=0)
    			  actualNpix=3;
    		  os << LogIO::NORMAL // Loglevel INFO
    				  << "SuperUniform weighting over a square cell spanning ["
    				  << -actualNpix
    				  << ", " << actualNpix << "] in the uv plane" << LogIO::POST;
    		  imwgt_p=VisImagingWeight(*vi_p, rmode, noise, robust, nx,
    				  ny, cellx, celly, actualNpix,
    				  actualNpix, multiField);
    	  }
    	  else if ((type=="robust")||(type=="uniform")||(type=="briggs")) {
    		  if(!imageDefined_p) throw(AipsError("Please define image first"));
    		  Quantity actualFieldOfView(fieldofview);
    		  Int actualNPixels(npixels);
    		  String wtype;
    		  if(type=="briggs") {
    			  wtype = "Briggs";
    		  }
    		  else {
    			  wtype = "Uniform";
    		  }
    		  if(actualFieldOfView.get().getValue()==0.0&&actualNPixels==0) {
    			  actualNPixels=nx;
    			  actualFieldOfView=Quantity(actualNPixels*cellx.get("rad").getValue(),
    					  "rad");
    			  os << LogIO::NORMAL // Loglevel INFO
    					  << wtype
    					  << " weighting: sidelobes will be suppressed over full image"
    					  << LogIO::POST;
    		  }
    		  else if(actualFieldOfView.get().getValue()>0.0&&actualNPixels==0) {
    			  actualNPixels=nx;
    			  os << LogIO::NORMAL // Loglevel INFO
    					  << wtype
    					  << " weighting: sidelobes will be suppressed over specified field of view: "
    					  << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
    		  }
    		  else if(actualFieldOfView.get().getValue()==0.0&&actualNPixels>0) {
    			  actualFieldOfView=Quantity(actualNPixels*cellx.get("rad").getValue(),
    					  "rad");
    			  os << LogIO::NORMAL // Loglevel INFO
    					  << wtype
    					  << " weighting: sidelobes will be suppressed over full image field of view: "
    					  << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
    		  }
    		  else {
    			  os << LogIO::NORMAL // Loglevel INFO
    					  << wtype
    					  << " weighting: sidelobes will be suppressed over specified field of view: "
    					  << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
    		  }
    		  os << LogIO::DEBUG1
    				  << "Weighting used " << actualNPixels << " uv pixels."
    				  << LogIO::POST;
    		  Quantity actualCellSize(actualFieldOfView.get("rad").getValue()/actualNPixels, "rad");

		  //		  cerr << "rmode " << rmode << " noise " << noise << " robust " << robust << " npixels " << actualNPixels << " cellsize " << actualCellSize << " multifield " << multiField << endl;
		  //		  Timer timer;
		  //timer.mark();
		  //Construct imwgt_p with old vi for now if old vi is in use as constructing with vi2 is slower 


		  imwgt_p=VisImagingWeight(*vi_p, wtype=="Uniform" ? "none" : rmode, noise, robust,
                                 actualNPixels, actualNPixels, actualCellSize,
                                 actualCellSize, 0, 0, multiField);

		  /*
		  if(rvi_p !=NULL){
		    imwgt_p=VisImagingWeight(*rvi_p, rmode, noise, robust,
                                 actualNPixels, actualNPixels, actualCellSize,
                                 actualCellSize, 0, 0, multiField);
		  }
		  else{
		    ////This is slower by orders of magnitude as of 2014/06/25
		    imwgt_p=VisImagingWeight(*vi_p, rmode, noise, robust,
                                 actualNPixels, actualNPixels, actualCellSize,
                                 actualCellSize, 0, 0, multiField);
		  }
		  */
		    //timer.show("After making visweight ");

    	  }
    	  else {
    		  //this->unlock();
    		  os << LogIO::SEVERE << "Unknown weighting " << type
    				  << LogIO::EXCEPTION;
    		  return False;
    	  }
      }
	 
	 //// UV-Tapering
	 //cout << "Taper type : " << filtertype << " : " << (filtertype=="gaussian") <<  endl;
	 if( filtertype == "gaussian" ) {
	   //	   os << "Setting uv-taper" << LogIO::POST;
	   imwgt_p.setFilter( filtertype,  filterbmaj, filterbmin, filterbpa );
	 }
	 vi_p->useImagingWeight(imwgt_p);
      ///////////////////////////////
	 
	 
	 ///	 return True;
	 
       }
       catch(AipsError &x)
	 {
	   throw( AipsError("Error in Weighting : "+x.getMesg()) );
	 }
       
       return True;
  }

void SynthesisImagerVi2::appendToMapperList(String imagename,  
					   CoordinateSystem& csys, 
					   IPosition imshape,
					    CountedPtr<refim::FTMachine>& ftm,
					    CountedPtr<refim::FTMachine>& iftm,
					   Quantity distance, 
					   Int facets,
					   Int chanchunks,
					   const Bool overwrite,
					   String mappertype,
					   uInt ntaylorterms,
					   Vector<String> startmodel)
    {
      LogIO log_l(LogOrigin("SynthesisImagerVi2", "appendToMapperList(ftm)"));
      //---------------------------------------------
      // Some checks..
      if(facets > 1 && itsMappers.nMappers() > 0)
	log_l << "Facetted image has to be the first of multifields" << LogIO::EXCEPTION;

      if( imshape.nelements()==4 && imshape[3]<chanchunks )
	{
	  log_l << LogIO::WARN << "An image with " << imshape[3] << " channel(s) cannot be divided into " << chanchunks << " chunks. Please set chanchunks=1 or choose chanchunks<nchan." << LogIO::EXCEPTION;
	}

      if(chanchunks > 1 && itsMappers.nMappers() > 0)
	log_l << "Channel chunking is currently not supported with multi(outlier)-fields. Please submit a feature request if needed." << LogIO::EXCEPTION;

      if(chanchunks > 1) itsDataLoopPerMapper=True;
      
      AlwaysAssert( ( ( ! (ftm->name()=="MosaicFTNew" && mappertype=="imagemosaic") )  && 
      		      ( ! (ftm->name()=="AWProjectWBFTNew" && mappertype=="imagemosaic") )) ,
		    AipsError );
      //---------------------------------------------

      // Create the ImageStore object
      CountedPtr<SIImageStore> imstor;
      ROMSColumns msc(*(mss_p[0]));
      imstor = createIMStore(imagename, csys, imshape, overwrite,msc, mappertype, ntaylorterms, distance,facets, iftm->useWeightImage(), startmodel );

      // Create the Mappers
      if( facets<2 && chanchunks<2) // One facet. Just add the above imagestore to the mapper list.
	{
	  itsMappers.addMapper(  createSIMapper( mappertype, imstor, ftm, iftm, ntaylorterms) );
	}
      else // This field is facetted. Make a list of reference imstores, and add all to the mapper list.
	{

	  if ( facets>1 && chanchunks==1 )
	    {
	      // Make and connect the list.
	      Block<CountedPtr<SIImageStore> > imstorList = createFacetImageStoreList( imstor, facets );
	      for( uInt facet=0; facet<imstorList.nelements(); facet++)
		{
		  CountedPtr<refim::FTMachine> new_ftm, new_iftm;
		  if(facet==0){ new_ftm = ftm;  new_iftm = iftm; }
		  else{ new_ftm=ftm->cloneFTM();  new_iftm=iftm->cloneFTM(); }
		  itsMappers.addMapper(createSIMapper( mappertype, imstorList[facet], new_ftm, new_iftm, ntaylorterms));
		}
	    }// facets
	  else if ( facets==1 && chanchunks>1 )
	    {
	      // Make and connect the list.
	      Block<CountedPtr<SIImageStore> > imstorList = createChanChunkImageStoreList( imstor, chanchunks );
	      for( uInt chunk=0; chunk<imstorList.nelements(); chunk++)
		{
		  CountedPtr<refim::FTMachine> new_ftm, new_iftm;
		  if(chunk==0){ new_ftm = ftm;  new_iftm = iftm; }
		  else{ new_ftm=ftm->cloneFTM();  new_iftm=iftm->cloneFTM(); }
		  itsMappers.addMapper(createSIMapper( mappertype, imstorList[chunk], new_ftm, new_iftm, ntaylorterms));
		}
	    }// chanchunks
	  else
	    {
	      throw( AipsError("Error in requesting "+String::toString(facets)+" facets on a side with " + String::toString(chanchunks) + " channel chunks.  Support for faceting along with channel chunking is not yet available. Please submit a feature-request if you need multiple facets as well as chanchunks. ") );
	    }

	}// facets or chunks

    }

  /////////////////////////
 void SynthesisImagerVi2::runMajorCycle(const Bool dopsf, 
				      const Bool savemodel)
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );

    //    cout << "Savemodel : " << savemodel << "   readonly : " << readOnly_p << "   usescratch : " << useScratch_p << endl;

    Bool savemodelcolumn = savemodel && !readOnly_p && useScratch_p;
    Bool savevirtualmodel = savemodel && !readOnly_p && !useScratch_p;

    if( savemodelcolumn ) os << "Saving model column" << LogIO::POST;
    if( savevirtualmodel ) os << "Saving virtual model" << LogIO::POST;

    SynthesisUtilMethods::getResource("Start Major Cycle");

    itsMappers.checkOverlappingModels("blank");

    {
      vi::VisBuffer2* vb=vi_p->getVisBuffer();
      vi_p->originChunks();
      vi_p->origin();
      Double numcoh=0;
      for (uInt k=0; k< mss_p.nelements(); ++k)
	numcoh+=Double(mss_p[k]->nrow());
      ProgressMeter pm(1.0, numcoh, 
			 dopsf?"Gridding Weights and PSF":"Major Cycle", "","","",True);
	Int cohDone=0;


    	if(!dopsf)itsMappers.initializeDegrid(*vb);
    	itsMappers.initializeGrid(*vb,dopsf);
	SynthesisUtilMethods::getResource("After initGrid for all mappers");

    	for (vi_p->originChunks(); vi_p->moreChunks();vi_p->nextChunk())
    	{

	  for (vi_p->origin(); vi_p->more(); vi_p->next())
    		{
		  //if (SynthesisUtilMethods::validate(*vb)==SynthesisUtilMethods::NOVALIDROWS) break; // No valid rows in this VB
		  //		  cerr << "nRows "<< vb->nRow() << "   " << max(vb->visCube()) <<  endl;
		  if (SynthesisUtilMethods::validate(*vb)!=SynthesisUtilMethods::NOVALIDROWS)
		    {
    			if(!dopsf) {
			  { Cube<Complex> mod(vb->nCorrelations(), vb->nChannels(), vb->nRows(), Complex(0.0));
			    vb->setVisCubeModel(mod); 
			  }
			  itsMappers.degrid(*vb, savevirtualmodel );
			  if(savemodelcolumn && writeAccess_p ){
			    //Darn not implented
			    //vi_p->writeVisModel(vb->visCubeModel());
			    static_cast<VisibilityIteratorImpl2 *> (vi_p->getImpl())->writeVisModel(vb->visCubeModel());
			  }
    			}
    			itsMappers.grid(*vb, dopsf, (refim::FTMachine::Type)datacol_p);
			cohDone += vb->nRows();
			pm.update(Double(cohDone));
		    }
    		}
    	}
    	//cerr << "IN SYNTHE_IMA" << endl;
    	//VisModelData::listModel(rvi_p->getMeasurementSet());
	SynthesisUtilMethods::getResource("Before finalize for all mappers");
    	if(!dopsf) itsMappers.finalizeDegrid(*vb);
    	itsMappers.finalizeGrid(*vb, dopsf);

    }

    itsMappers.checkOverlappingModels("restore");

    unlockMSs();

    SynthesisUtilMethods::getResource("End Major Cycle");

  }// end runMajorCycle

 
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /// The mapper loop is outside the data iterator loop.
  /// This is for cases where the image size is large compared to the RAM and
  /// where data I/O is the relatively minor cost.
  void SynthesisImagerVi2::runMajorCycle2(const Bool dopsf, 
				      const Bool savemodel)
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle2",WHERE) );

    //    cout << "Savemodel : " << savemodel << "   readonly : " << readOnly_p << "   usescratch : " << useScratch_p << endl;

    Bool savemodelcolumn = savemodel && !readOnly_p && useScratch_p;
    Bool savevirtualmodel = savemodel && !readOnly_p && !useScratch_p;

    if( savemodelcolumn ) os << "Saving model column" << LogIO::POST;
    if( savevirtualmodel ) os << "Saving virtual model" << LogIO::POST;

    itsMappers.checkOverlappingModels("blank");

    for(Int gmap=0;gmap<itsMappers.nMappers();gmap++)
       {
	 SynthesisUtilMethods::getResource("Start Major Cycle for mapper"+String::toString(gmap));

	 vi::VisBuffer2* vb=vi_p->getVisBuffer();
	 vi_p->originChunks();
	 vi_p->origin();
	 Double numcoh=0;
	 for (uInt k=0; k< mss_p.nelements(); ++k)
	   numcoh+=Double(mss_p[k]->nrow());


	 ProgressMeter pm(1.0, numcoh, 
			  dopsf?"Gridding Weights and PSF":"Major Cycle", "","","",True);
	Int cohDone=0;


    	if(!dopsf){
	  itsMappers.initializeDegrid(*vb, gmap);
		  //itsMappers.getMapper(gmap)->initializeDegrid(*vb);
	}
	itsMappers.initializeGrid(*vb,dopsf, gmap);
		//itsMappers.getMapper(gmap)->initializeGrid(*vb,dopsf);

	SynthesisUtilMethods::getResource("After initialize for mapper"+String::toString(gmap));

    	for (vi_p->originChunks(); vi_p->moreChunks();vi_p->nextChunk())
    	{

	  for (vi_p->origin(); vi_p->more(); vi_p->next())
	    {
	      //if (SynthesisUtilMethods::validate(*vb)==SynthesisUtilMethods::NOVALIDROWS) break; // No valid rows in this VB
	      //		  cerr << "nRows "<< vb->nRow() << "   " << max(vb->visCube()) <<  endl;
	      if (SynthesisUtilMethods::validate(*vb)!=SynthesisUtilMethods::NOVALIDROWS)
		{
		  if(!dopsf) {
		    { Cube<Complex> mod(vb->nCorrelations(), vb->nChannels(), vb->nRows(), Complex(0.0));
		      vb->setVisCubeModel(mod); 
		    }
		    itsMappers.degrid(*vb, savevirtualmodel, gmap );
		    //itsMappers.getMapper(gmap)->degrid(*vb); //, savevirtualmodel );
		    if(savemodelcolumn && writeAccess_p ){
		      //vi_p->writeVisModel(vb->visCubeModel());
		      //vi_p->writeBackChanges(vb);
		      static_cast<VisibilityIteratorImpl2 *> (vi_p->getImpl())->writeVisModel(vb->visCubeModel());
		    }

		  }
		  itsMappers.grid(*vb, dopsf, (refim::FTMachine::Type)(datacol_p), gmap);
		  //itsMappers.getMapper(gmap)->grid(*vb, dopsf, datacol_p);
		  cohDone += vb->nRows();
		  pm.update(Double(cohDone));
		}
	    }
    	}
    	//cerr << "IN SYNTHE_IMA" << endl;
    	//VisModelData::listModel(rvi_p->getMeasurementSet());

	SynthesisUtilMethods::getResource("Before finalize for mapper"+String::toString(gmap));
	
    	if(!dopsf) 
	  {
	    itsMappers.finalizeDegrid(*vb,gmap);
	    //itsMappers.getMapper(gmap)->finalizeDegrid();
	  }
	itsMappers.finalizeGrid(*vb, dopsf,gmap);
    	//itsMappers.getMapper(gmap)->finalizeGrid(*vb, dopsf);
	
	//	itsMappers.getMapper(gmap)->releaseImageLocks();
	
	SynthesisUtilMethods::getResource("End Major Cycle for mapper"+String::toString(gmap));
       }// end of mapper loop

    itsMappers.checkOverlappingModels("restore");

    unlockMSs();

    SynthesisUtilMethods::getResource("End Major Cycle");

  }// end runMajorCycle2


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CountedPtr<SIMapper> SynthesisImagerVi2::createSIMapper(String mappertype,  
							   CountedPtr<SIImageStore> imagestore,
							CountedPtr<refim::FTMachine> ftmachine,
							CountedPtr<refim::FTMachine> iftmachine,
						       uInt /*ntaylorterms*/)
  {
    LogIO os( LogOrigin("SynthesisImager","createSIMapper",WHERE) );
    
    CountedPtr<SIMapper> localMapper;

    try
      {
	
	if( mappertype == "default" || mappertype == "multiterm" )
	  {
	    localMapper = new SIMapper( imagestore, ftmachine, iftmachine );
	  }
	else if( mappertype == "imagemosaic") // || mappertype == "mtimagemosaic" )
	  {
	    localMapper = new SIMapperImageMosaic( imagestore, ftmachine, iftmachine );
	  }
	else
	  {
	    throw(AipsError("Unknown mapper type : " + mappertype));
	  }

      }
    catch(AipsError &x) {
	throw(AipsError("Error in createSIMapper : " + x.getMesg() ) );
      }
    return localMapper;
  }
  

void SynthesisImagerVi2::unlockMSs()
  {
    LogIO os( LogOrigin("SynthesisImager","unlockMSs",WHERE) );
    for(uInt i=0;i<mss_p.nelements();i++)
      { 
	os << LogIO::NORMAL2 << "Unlocking : " << (mss_p[i])->tableName() << LogIO::POST;
	const_cast<MeasurementSet* >(mss_p[i])->unlock(); 
      }
  }
  void SynthesisImagerVi2::createFTMachine(CountedPtr<refim::FTMachine>& theFT, 
					   CountedPtr<refim::FTMachine>& theIFT, 
					   const String& ftname,
					   const uInt nTaylorTerms,
					   const String mType,
					   const Int facets,            //=1
					   //------------------------------
					   const Int wprojplane,        //=1,
					   const Float padding,         //=1.0,
					   const Bool useAutocorr,      //=False,
					   const Bool useDoublePrec,    //=True,
					   const String gridFunction,   //=String("SF"),
					//------------------------------
					   const Bool aTermOn,          //= True,
					   const Bool psTermOn,         //= True,
					   const Bool mTermOn,          //= False,
					const Bool wbAWP,            //= True,
					   const String cfCache,        //= "",
					   const Bool doPointing,       //= False,
					   const Bool doPBCorr,         //= True,
					   const Bool conjBeams,        //= True,
					const Float computePAStep,         //=360.0
					   const Float rotatePAStep,          //=5.0
					   const String interpolation,  //="linear"
					   const Bool freqFrameValid, //=True
					   const Int cache,             //=1000000000,
					   const Int tile,               //=16
					   const String stokes, //=I
					   const String imageNamePrefix
					   )

  {
    LogIO os( LogOrigin("SynthesisImager","createFTMachine",WHERE));

    if(ftname=="gridft"){
      if(facets >1){
	theFT=new refim::GridFT(cache, tile, gridFunction, mLocation_p, phaseCenter_p, padding, useAutocorr, useDoublePrec);
	theIFT=new refim::GridFT(cache, tile, gridFunction, mLocation_p, phaseCenter_p, padding, useAutocorr, useDoublePrec);

      }
      else{
	theFT=new refim::GridFT(cache, tile, gridFunction, mLocation_p, padding, useAutocorr, useDoublePrec);
	theIFT=new refim::GridFT(cache, tile, gridFunction, mLocation_p, padding, useAutocorr, useDoublePrec);
      }
    }
    else if(ftname== "wprojectft"){
     Double maxW=-1.0;
     Double minW=-1.0;
     Double rmsW=-1.0;
     if(wprojplane <1)
       casa::WProjectFT::wStat(*rvi_p, minW, maxW, rmsW);
    if(facets >1){
      theFT=new refim::WProjectFT(wprojplane,  phaseCenter_p, mLocation_p,
			   cache/2, tile, useAutocorr, padding, useDoublePrec, minW, maxW, rmsW);
      theIFT=new refim::WProjectFT(wprojplane,  phaseCenter_p, mLocation_p,
			    cache/2, tile, useAutocorr, padding, useDoublePrec, minW, maxW, rmsW);
    }
    else{
      theFT=new refim::WProjectFT(wprojplane,  mLocation_p,
			   cache/2, tile, useAutocorr, padding, useDoublePrec, minW, maxW, rmsW);
      theIFT=new refim::WProjectFT(wprojplane,  mLocation_p,
			    cache/2, tile, useAutocorr, padding, useDoublePrec, minW, maxW, rmsW);
    }
    CountedPtr<refim::WPConvFunc> sharedconvFunc=static_cast<refim::WProjectFT &>(*theFT).getConvFunc();
      //static_cast<WProjectFT &>(*theFT).setConvFunc(sharedconvFunc);
    static_cast<refim::WProjectFT &>(*theIFT).setConvFunc(sharedconvFunc);
    }
    else if ((ftname == "awprojectft") || (ftname== "mawprojectft") || (ftname == "protoft")) {
      createAWPFTMachine(theFT, theIFT, ftname, facets, wprojplane, 
			 padding, useAutocorr, useDoublePrec, gridFunction,
			 aTermOn, psTermOn, mTermOn, wbAWP, cfCache, 
			 doPointing, doPBCorr, conjBeams, computePAStep,
			 rotatePAStep, cache,tile,imageNamePrefix);
    }
    else if ( ftname == "mosaic" || ftname== "mosft" || ftname == "mosaicft" || ftname== "MosaicFT"){

      createMosFTMachine(theFT, theIFT, padding, useAutocorr, useDoublePrec, rotatePAStep, stokes);
    }
    else
      {
	throw( AipsError( "Invalid FTMachine name : " + ftname ) );
      }
    /* else if(ftname== "MosaicFT"){

       }*/



    ///////// Now, clone and pack the chosen FT into a MultiTermFT if needed.
    if( mType=="multiterm" )
      {
	AlwaysAssert( nTaylorTerms>=1 , AipsError );

	CountedPtr<refim::FTMachine> theMTFT = new refim::MultiTermFTNew( theFT , nTaylorTerms, True/*forward*/ );
	CountedPtr<refim::FTMachine> theMTIFT = new refim::MultiTermFTNew( theIFT , nTaylorTerms, False/*forward*/ );

	theFT = theMTFT;
	theIFT = theMTIFT;
      }




    ////// Now, set the SkyJones if needed, and if not internally generated.
    if( mType=="imagemosaic" && 
	(ftname != "awprojectft" && ftname != "mawprojectft" && ftname != "proroft") )
      {
	CountedPtr<refim::SkyJones> vp;
	ROMSColumns msc(*(mss_p[0]));
	Quantity parang(0.0,"deg");
	Quantity skyposthreshold(0.0,"deg");
	vp = new refim::VPSkyJones(msc, True,  parang, BeamSquint::NONE,skyposthreshold);

	Vector<CountedPtr<refim::SkyJones> > skyJonesList(1);
	skyJonesList(0) = vp;
	theFT->setSkyJones(  skyJonesList );
	theIFT->setSkyJones(  skyJonesList );

      }

    //// For mode=cubedata, set the freq frame to invalid..
    // get this info from buildCoordSystem
    //theFT->setSpw( tspws, False );
    //theIFT->setSpw( tspws, False );
    theFT->setFrameValidity( freqFrameValid );
    theIFT->setFrameValidity( freqFrameValid );

    //// Set interpolation mode
    theFT->setFreqInterpolation( interpolation );
    theIFT->setFreqInterpolation( interpolation );
    /* vi_p has chanselection now
    //channel selections from spw param
    theFT->setSpwChanSelection(chanSel_p);
    theIFT->setSpwChanSelection(chanSel_p);
    */
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void SynthesisImagerVi2::createAWPFTMachine(CountedPtr<refim::FTMachine>& theFT, CountedPtr<refim::FTMachine>& theIFT, 
					   const String&,// ftmName,
					   const Int,// facets,            //=1
					   //------------------------------
					   const Int wprojPlane,        //=1,
					   const Float,// padding,         //=1.0,
					   const Bool,// useAutocorr,      //=False,
					   const Bool useDoublePrec,    //=True,
					   const String,// gridFunction,   //=String("SF"),
					   //------------------------------
					   const Bool aTermOn,          //= True,
					   const Bool psTermOn,         //= True,
					   const Bool mTermOn,          //= False,
					   const Bool wbAWP,            //= True,
					   const String cfCache,        //= "",
					   const Bool doPointing,       //= False,
					   const Bool doPBCorr,         //= True,
					   const Bool conjBeams,        //= True,
					   const Float computePAStep,   //=360.0
					   const Float rotatePAStep,    //=5.0
					   const Int cache,             //=1000000000,
					   const Int tile,               //=16
					   const String imageNamePrefix
					)

  {
    LogIO os( LogOrigin("SynthesisImager","createAWPFTMachine",WHERE));

    if (wprojPlane<=1)
      {
	os << LogIO::NORMAL
	   << "You are using wprojplanes=1. Doing co-planar imaging (no w-projection needed)" 
	   << LogIO::POST;
	os << LogIO::NORMAL << "Performing WBA-Projection" << LogIO::POST; // Loglevel PROGRESS
      }
    if((wprojPlane>1)&&(wprojPlane<64)) 
      {
	os << LogIO::WARN
	   << "No. of w-planes set too low for W projection - recommend at least 128"
	   << LogIO::POST;
	os << LogIO::NORMAL << "Performing WBAW-Projection" << LogIO::POST; // Loglevel PROGRESS
      }

    // CountedPtr<ATerm> apertureFunction = createTelescopeATerm(mss4vi_p[0], aTermOn);
    // CountedPtr<PSTerm> psTerm = new PSTerm();
    // CountedPtr<WTerm> wTerm = new WTerm();
    
    // //
    // // Selectively switch off CFTerms.
    // //
    // if (aTermOn == False) {apertureFunction->setOpCode(CFTerms::NOOP);}
    // if (psTermOn == False) psTerm->setOpCode(CFTerms::NOOP);

    // //
    // // Construct the CF object with appropriate CFTerms.
    // //
    // CountedPtr<ConvolutionFunction> tt;
    // tt = AWProjectFT::makeCFObject(aTermOn, psTermOn, True, mTermOn, wbAWP);
    // CountedPtr<ConvolutionFunction> awConvFunc;
    // //    awConvFunc = new AWConvFunc(apertureFunction,psTerm,wTerm, !wbAWP);
    // if ((ftmName=="mawprojectft") || (mTermOn))
    //   awConvFunc = new AWConvFuncEPJones(apertureFunction,psTerm,wTerm,wbAWP);
    // else
    //   awConvFunc = new AWConvFunc(apertureFunction,psTerm,wTerm,wbAWP);

    ROMSObservationColumns msoc((mss_p[0])->observation());
    String telescopeName=msoc.telescopeName()(0);
    CountedPtr<refim::ConvolutionFunction> awConvFunc = refim::AWProjectFT::makeCFObject(telescopeName, 
									   aTermOn,
									   psTermOn, True, mTermOn, wbAWP);
    //
    // Construct the appropriate re-sampler.
    //
    CountedPtr<refim::VisibilityResamplerBase> visResampler;
    //    if (ftmName=="protoft") visResampler = new ProtoVR();
    //elsef
    visResampler = new refim::AWVisResampler();
    //    CountedPtr<VisibilityResamplerBase> visResampler = new VisibilityResampler();

    //
    // Construct and initialize the CF cache object.
    //


    // CountedPtr<CFCache> cfCacheObj = new CFCache();
    // cfCacheObj->setCacheDir(cfCache.data());
    // //    cerr << "Setting wtImagePrefix to " << imageNamePrefix.c_str() << endl;
    // cfCacheObj->setWtImagePrefix(imageNamePrefix.c_str());
    // cfCacheObj->initCache2();

    CountedPtr<refim::CFCache> cfCacheObj;
      

    //
    // Finally construct the FTMachine with the CFCache, ConvFunc and
    // Re-sampler objects.  
    //
    Float pbLimit_l=1e-3;
    theFT = new refim::AWProjectWBFTNew(wprojPlane, cache/2, 
			      cfCacheObj, awConvFunc, 
			      visResampler,
			      /*True */doPointing, doPBCorr, 
			      tile, computePAStep, pbLimit_l, True,conjBeams,
			      useDoublePrec);

    cfCacheObj = new refim::CFCache();
    cfCacheObj->setCacheDir(cfCache.data());
    //    cerr << "Setting wtImagePrefix to " << imageNamePrefix.c_str() << endl;
    cfCacheObj->setWtImagePrefix(imageNamePrefix.c_str());
    cfCacheObj->initCache2();

    theFT->setCFCache(cfCacheObj);
    

    Quantity rotateOTF(rotatePAStep,"deg");
    static_cast<refim::AWProjectWBFTNew &>(*theFT).setObservatoryLocation(mLocation_p);
    static_cast<refim::AWProjectWBFTNew &>(*theFT).setPAIncrement(Quantity(computePAStep,"deg"),rotateOTF);

    // theIFT = new AWProjectWBFT(wprojPlane, cache/2, 
    // 			       cfCacheObj, awConvFunc, 
    // 			       visResampler,
    // 			       /*True */doPointing, doPBCorr, 
    // 			       tile, computePAStep, pbLimit_l, True,conjBeams,
    // 			       useDoublePrec);

    // static_cast<AWProjectWBFT &>(*theIFT).setObservatoryLocation(mLocation_p);
    // static_cast<AWProjectWBFT &>(*theIFT).setPAIncrement(Quantity(computePAStep,"deg"),rotateOTF);

    theIFT = new refim::AWProjectWBFTNew(static_cast<refim::AWProjectWBFTNew &>(*theFT));

    

  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void SynthesisImagerVi2:: createMosFTMachine(CountedPtr<refim::FTMachine>& theFT,CountedPtr<refim::FTMachine>&  theIFT, const Float /*padding*/, const Bool useAutoCorr, const Bool useDoublePrec, const Float rotatePAStep, const String stokes){
    
    LogIO os(LogOrigin("SynthesisImager", "createMosFTMachine",WHERE));
   
    ROMSColumns msc(vi_p->ms());
    String telescop=msc.observation().telescopeName()(0);
    // Hack...start
    if(telescop=="EVLA"){os << LogIO::WARN << "vpmanager does not list EVLA. Using VLA beam parameters" << LogIO::POST; telescop="VLA";}
    // Hack...stop
    VPManager *vpman=VPManager::Instance();
    PBMath::CommonPB kpb;
    PBMath::enumerateCommonPB(telescop, kpb);
    Record rec;
    vpman->getvp(rec, telescop);
    refim::VPSkyJones* vps=NULL;
    if(rec.asString("name")=="COMMONPB" && kpb !=PBMath::UNKNOWN ){
      vps= new refim::VPSkyJones(msc, True, Quantity(rotatePAStep, "deg"), BeamSquint::GOFIGURE, Quantity(360.0, "deg"));
      /////Don't know which parameter has pb threshold cutoff that the user want 
      ////leaving at default
      ////vps.setThreshold(minPB);
      
    }
    
    theFT = new refim::MosaicFTNew(vps, mLocation_p, stokes, 1000000000, 16, useAutoCorr, 
		      useDoublePrec);
    PBMathInterface::PBClass pbtype=PBMathInterface::AIRY;
    if(rec.asString("name")=="IMAGE")
       pbtype=PBMathInterface::IMAGE;
    ///Use Heterogenous array mode for the following
    if((kpb == PBMath::UNKNOWN) || (kpb==PBMath::OVRO) || (kpb==PBMath::ACA)
       || (kpb==PBMath::ALMA)){
      CountedPtr<refim::SimplePBConvFunc> mospb=new refim::HetArrayConvFunc(pbtype, "");
      static_cast<refim::MosaicFTNew &>(*theFT).setConvFunc(mospb);
    }
    ///////////////////make sure both FTMachine share the same conv functions.
    theIFT= new refim::MosaicFTNew(static_cast<refim::MosaicFTNew &>(*theFT));

    
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void SynthesisImagerVi2::createVisSet(const Bool /*writeAccess*/)
  {
    LogIO os( LogOrigin("SynthesisImager","createVisSet",WHERE) );
    if(mss_p.nelements() != uInt(fselections_p.size()) && (fselections_p.size() !=0)){
      throw(AipsError("Discrepancy between Number of MSs and Frequency selections"));
    }
    vi_p=new vi::VisibilityIterator2(mss_p, vi::SortColumns(), True); //writeAccess);
    if(fselections_p.size() !=0)
      vi_p->setFrequencySelection (fselections_p);
    //
    vi_p->originChunks();
    vi_p->origin();
  }// end of createVisSet





} //# NAMESPACE CASA - END

