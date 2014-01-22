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
#include <synthesis/MeasurementEquations/ImagerMultiMS.h>
#include <synthesis/MSVis/VisSetUtil.h>
#include <synthesis/MSVis/VisImagingWeight.h>
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
				       mss_p(0),vi_p(0), writeAccess_p(True), mss4vi_p(0)
  {


     facetsStore_p=-1;
     imwgt_p=VisImagingWeight("natural");
     imageDefined_p=False;
     useScratch_p=False;
     wvi_p=0;
     rvi_p=0;

     ftmParams_p.define("ftmachine","GridFT");
     ftmParams_p.define("wprojplanes", Int(1));
     ftmParams_p.define("padding", Float(1.0));
     ftmParams_p.define("useautocorr", False);
     ftmParams_p.define("usedoubleprec", True);
     ftmParams_p.define("gridfunc", String("SF"));

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
    if(rvi_p) delete rvi_p;
    cerr << "IN DESTR"<< endl;
    //    VisModelData::listModel(mss4vi_p[0]);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  Bool SynthesisImager::selectData(const String& msname, 
				   const String& spw, 
				   const String& freqBeg, 
				   const String& freqEnd, 
				   const MFrequency::Types freqframe, 
				   const String& field, 
				   const String& antenna, 
				   const String& timestr,
				   const String& scan, 
				   const String& obs, 
				   const String& state,
				   const String& uvdist, 
				   const String& taql,
				   const Bool usescratch, 
				   const Bool readonly, 
				   const Bool incrModel){
    LogIO os( LogOrigin("SynthesisImager","selectData",WHERE) );

    try
      {

    //Respect the readonly flag...necessary for multi-process access
    MeasurementSet thisms(msname, TableLock(TableLock::AutoNoReadLocking),
				readonly ? Table::Old : Table::Update);
    thisms.setMemoryResidentSubtables (MrsEligibility::defaultEligible());
    useScratch_p=usescratch;
    //if you want to use scratch col...make sure they are there
    if(usescratch && !readonly){
      VisSetUtil::addScrCols(thisms, True, False, True, False);
      VisModelData::clearModel(thisms);
    }
    if(!incrModel && !usescratch && !readonly)
    	VisModelData::clearModel(thisms, field, spw);

    os << "MS : " << msname << " | ";

    //Some MSSelection 
    //If everything is empty (which is valid) it will throw an exception..below
    //So make sure the main defaults are not empy i.e field and spw
    MSSelection thisSelection;
    if(field != ""){
      thisSelection.setFieldExpr(field);
      os << "Selecting on fields : " << field << " | " ;//LogIO::POST;
    }else
      thisSelection.setFieldExpr("*");
    if(spw != ""){
	thisSelection.setSpwExpr(spw);
	os << "Selecting on spectral windows expression :"<< spw  << " | " ;//LogIO::POST;
    }else
      thisSelection.setSpwExpr("*");
    
    if(antenna != ""){
      Vector<String> antNames(1, antenna);
      // thisSelection.setAntennaExpr(MSSelection::nameExprStr( antNames));
      thisSelection.setAntennaExpr(antenna);
      os << "Selecting on antenna names : " << antenna << " | " ;//LogIO::POST;
	
    }            
    if(timestr != ""){
	thisSelection.setTimeExpr(timestr);
	os << "Selecting on time range : " << timestr << " | " ;//LogIO::POST;	
      }
    if(uvdist != ""){
      thisSelection.setUvDistExpr(uvdist);
      os << "Selecting on uvdist : " << uvdist << " | " ;//LogIO::POST;	
    }
    if(scan != ""){
      thisSelection.setScanExpr(scan);
      os << "Selecting on scan : " << scan << " | " ;//LogIO::POST;	
    }
    if(obs != ""){
      thisSelection.setObservationExpr(obs);
      os << "Selecting on Observation Expr : " << obs << " | " ;//LogIO::POST;	
    }
    if(state != ""){
      thisSelection.setStateExpr(state);
      os << "Selecting on Scan Intent/State : " << state << " | " ;//LogIO::POST;	
    }
    if(taql != ""){
	thisSelection.setTaQLExpr(taql);
	os << "Selecting via TaQL : " << taql << " | " ;//LogIO::POST;	
    }
    TableExprNode exprNode=thisSelection.toTableExprNode(&thisms);
    if(!(exprNode.isNull())){
      mss_p.resize(mss_p.nelements()+1, False, True);
      mss4vi_p.resize(mss4vi_p.nelements()+1, False, True);
      mss_p[mss_p.nelements()-1]=new const  MeasurementSet(thisms(exprNode));
      mss4vi_p[mss_p.nelements()-1]=MeasurementSet(thisms(exprNode));
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

      ///////////////Temporary revert to using Vi/vb
      Int msin=mss_p.nelements();
      blockNChan_p.resize(msin, False, True);
      blockStart_p.resize(msin, False, True);
      blockStep_p.resize(msin, False, True);
      blockSpw_p.resize(msin, False, True);
      msin-=1;
      blockNChan_p[msin].resize(nSelections);
      blockStart_p[msin].resize(nSelections);
      blockStep_p[msin].resize(nSelections);
      blockSpw_p[msin].resize(nSelections);
      ///////////////////////

      if(freqBeg==""){
    	  vi::FrequencySelectionUsingChannels channelSelector;
    	  //////////This is not implemented
    	  //channelSelector.add(thisSelection);
    	  /////So this gymnastic is needed
    	  for(uInt k=0; k < nSelections; ++k)
    	  {
	  
    		  spw = chanlist(k,0);
	  
    		  // channel selection
    		  chanStart = chanlist(k,1);
    		  chanEnd = chanlist(k,2);
    		  chanStep = chanlist(k,3);
    		  nchan = chanEnd-chanStart+1;
    		  //channelSelector.add (spw, chanStart, nchan,chanStep);
    		  ///////////////Temporary revert to using Vi/vb
    		  blockNChan_p[msin][k]=nchan;
    		  blockStart_p[msin][k]=chanStart;
    		  blockStep_p[msin][k]=chanStep;
    		  blockSpw_p[msin][k]=spw;
    		  /////////////////////////////////////////

    	  }
    	  fselections_p.add(channelSelector);
      }
      else{
    	  vi::FrequencySelectionUsingFrame channelSelector(freqframe);
    	  Quantity freq;
    	  Quantity::read(freq, freqBeg);
    	  Double lowfreq=freq.getValue("Hz");
    	  Quantity::read(freq, freqEnd);
    	  Double topfreq=freq.getValue("Hz");
    	  for(uInt k=0; k < nSelections; ++k)
    		  channelSelector.add(chanlist(k,0), lowfreq, topfreq);
    	  fselections_p.add(channelSelector);
    	  //////////OLD VI/VB
    	  ImagerMultiMS im;
    	  Vector<Vector<Int> >elspw, elstart, elnchan;
    	  Vector<Int>fields=thisSelection.getFieldList(mss_p[mss_p.nelements()-1]);
    	  Int fieldid=fields.nelements() ==0 ? 0: fields[0];
    	  im.adviseChanSelex(lowfreq, topfreq, 1.0, freqframe, elspw, elstart, elnchan, msname, fieldid, False);
    	  blockNChan_p[msin].resize(elspw[0].nelements());
    	  blockStart_p[msin].resize(elspw[0].nelements());
    	  blockStep_p[msin].resize(elspw[0].nelements());
    	  blockSpw_p[msin].resize(elspw[0].nelements());
    	  blockNChan_p[msin]=elnchan[0];
    	  blockStart_p[msin]=elstart[0];
    	  blockStep_p[msin].set(1);
    	  blockSpw_p[msin]=elspw[0];
    	  //////////////////////
      }


    }
    writeAccess_p=writeAccess_p && !readonly;
    createVisSet(writeAccess_p);


      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in selectData() : "+x.getMesg()) );
      }

    return True;

  }

 

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
				    const String ftmachine, 
				    const Int /*nTaylorTerms*/,
				    const Quantity& /*refFreq*/,
				    const Projection& projection,
				    const Quantity& distance,
				    const MFrequency::Types& freqFrame,
				    const Bool /*trackSource*/, 
				    const MDirection& /*trackDir*/, 
				    const Bool overwrite,
				     const Float padding, 
				     const Bool useAutocorr, 
				     const Bool useDoublePrec, 
				     const Int wprojplanes, 
				     const String convFunc, 
				    const String /*startmodel*/)
{
    LogIO os( LogOrigin("SynthesisImager","defineImage",WHERE) );
    if(mss_p.nelements() ==0)
      throw(AipsError("SelectData has to be run before defineImage"));

    CoordinateSystem csys;

    try
      {

	os << "Adding " << imagename << " (nchan : " << nchan << ", freqstart:" << freqStart.getValue() << freqStart.getUnit() <<  ", nx:" << nx << ",ny:" << ny << ", cellx:" << cellx.getValue() << cellx.getUnit() << ", celly:" << celly.getValue() << celly.getUnit() << " ) to imager list " << LogIO::POST;

	csys=buildCoordSys(phaseCenter, cellx, celly, nx, ny, stokes, projection, nchan,freqStart, freqStep, restFreq, freqFrame);
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in building Coordinate System() : "+x.getMesg()) );
      }


    try
      {

    /// Accumulate FTM parameters, to get used in createFTMachine..... 
    ftmParams_p.define("ftmachine",ftmachine);
    ftmParams_p.define("wprojplanes", wprojplanes);
    ftmParams_p.define("padding", padding );
    ftmParams_p.define("useautocorr", useAutocorr);
    ftmParams_p.define("usedoubleprec", useDoublePrec);
    ftmParams_p.define("gridfunc", convFunc);

    // NOTE to Kumar : If possible, can 'createFTMachine()' be called directly from here, 
    //                      with a full list of parameters, instead of passing them through appendToMapperList ? 
    // The other defineimage() already does this...... and can be augmented with the full FTM parameter list too.

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in setting up FTMachine() : "+x.getMesg()) );
      }

    try
      {

	appendToMapperList(imagename,  csys,  ftmParams_p.asString("ftmachine"), distance, facets, overwrite);
	imageDefined_p=True;
	
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in adding Mapper : "+x.getMesg()) );
      }


    // Set the model image for prediction -- Call an SIImageStore function that does the REGRIDDING.
    /*
      if( startmodel.length()>0 && !itsCurrentImages.null() )
      {
      os << "Setting " << startmodel << " as starting model for prediction " << LogIO::POST;
      itsCurrentImages->setModelImage( startmodel );
      }
    */
 

    
    return True;
  }
 
  Bool SynthesisImager::defineImage(CountedPtr<SIImageStore> imstor, const String& ftmachine){


	  CountedPtr<FTMachine> ftm;
	  CountedPtr<FTMachine> iftm;
	  createFTMachine(ftm, iftm, ftmachine);
	  Int id=itsMappers.nMappers();
	  itsCurrentCoordSys=imstor->residual()->coordinates();
	  itsCurrentShape=imstor->residual()->shape();
	  Int nx=itsCurrentShape[0]; Int ny=itsCurrentShape[1];
	  if( (id==0) || (nx*ny > itsMaxShape[0]*itsMaxShape[1]))
	  {
		  itsMaxShape=itsCurrentShape;
		  itsMaxCoordSys=itsCurrentCoordSys;
	  }

	  CountedPtr<SIMapperBase> thismap=new SIMapper(imstor, ftm, iftm, id);
	  itsMappers.addMapper(thismap);

	 return True;
  }

   ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  void SynthesisImager::setComponentList(const ComponentList& cl, Bool sdgrid){
	  String cft="SimpleComponentFTMachine";
	  if(sdgrid)
		  cft="SimpCompGridFTMachine";
	  Int mapperid=itsMappers.nMappers();
	  CountedPtr<SIMapperBase> sm=new SIMapper(cl, cft, mapperid);
	  itsMappers.addMapper(sm);

  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////Reset the Mapper
  ////////////////////
  void SynthesisImager::resetMappers(){
    ////reset code
	itsMappers=SIMapperCollection();
	unFacettedImStore_p=NULL;
  }
//////////////////////////////////////////////////////////////////
/////////////////////////////////////////////
  CountedPtr<SIImageStore> SynthesisImager::imageStore(const Int id)
  {
	  if(facetsStore_p >1)
	  {
		  if(id==0)
			  return unFacettedImStore_p;
		  else
		  {
			  return itsMappers.imageStore(facetsStore_p*facetsStore_p+id-1);
		  }
	  }
	  return itsMappers.imageStore(id);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  void SynthesisImager::executeMajorCycle(Record& /*controlRecord*/, const Bool useViVb2)
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );

    os << "-------------------------------------------------------------------------------------------------------------" << LogIO::POST;

    try
      {    
	runMajorCycle(False, useViVb2);

	itsMappers.releaseImageLocks();

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in running Major Cycle : "+x.getMesg()) );
      }    

  }// end of runMajorCycle
  //////////////////////////////////////////////
  /////////////////////////////////////////////

  void SynthesisImager::makePSF(const Bool useViVb2)
    {
      LogIO os( LogOrigin("SynthesisImager","makePSF",WHERE) );

      os << "-------------------------------------------------------------------------------------------------------------" << LogIO::POST;
    
      try
      {
    	  runMajorCycle(True, useViVb2);
    	  if(facetsStore_p >1){
    		  //Facetted image should
    		  IPosition shape=(unFacettedImStore_p->psf())->shape();
    		  IPosition blc(4, 0, 0, 0, 0);
    		  IPosition trc=shape-1;
    		  TempImage<Float> onepsf((itsMappers.imageStore(0)->psf())->shape(), (itsMappers.imageStore(0)->psf())->coordinates());
    		  onepsf.copyData(*(itsMappers.imageStore(0)->psf()));
    		  //now set the original to 0 as we have a copy of one facet psf
    		  (unFacettedImStore_p->psf())->set(0.0);
    		  blc[0]=(shape[0]-(onepsf.shape()[0]))/2;
    		  trc[0]=onepsf.shape()[0]+blc[0]-1;
    		  blc[1]=(shape[1]-(onepsf.shape()[1]))/2;
    		  trc[1]=onepsf.shape()[1]+blc[1]-1;
    		  Slicer sl(blc, trc, Slicer::endIsLast);
    		  SubImage<Float> sub(*(unFacettedImStore_p->psf()), sl, True);
    		  sub.copyData(onepsf);
    	  }

    	  itsMappers.releaseImageLocks();

      }
      catch(AipsError &x)
      {
    	  throw( AipsError("Error in making PSF : "+x.getMesg()) );
      }

    }
  //////////////////////
  //////////////////
  void SynthesisImager::predictModel(const Bool useViVb2){
	  LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );

	      if(useViVb2){
	      	vi_p->originChunks();
	      	vi_p->origin();
	      	vi::VisBuffer2* vb=vi_p->getVisBuffer();
	      	itsMappers.initializeDegrid(*vb);
	      	for (vi_p->originChunks(); vi_p->moreChunks();vi_p->nextChunk())
	      	{

	      		for (vi_p->origin(); vi_p->more();vi_p->next())
	      		{

	      			vb->setVisCubeModel(Cube<Complex>(vb->visCubeModel().shape(), Complex(0.0, 0.0)));
	      			itsMappers.degrid(*vb, useScratch_p);
	      			if(vi_p->isWritable() && useScratch_p)
	      				vi_p->writeVisModel(vb->visCubeModel());
	      		}
	      	}
	      	itsMappers.finalizeDegrid(*vb);
	      }

	      else{
	      	VisBufferAutoPtr vb(rvi_p);
	      	rvi_p->originChunks();
	      	rvi_p->origin();
	      	itsMappers.initializeDegrid(*vb);
	      	for (rvi_p->originChunks(); rvi_p->moreChunks();rvi_p->nextChunk())
	      	{

	      		for (rvi_p->origin(); rvi_p->more(); (*rvi_p)++)
	      		{
			  //			  cerr << "nRows "<< vb->nRow() << "   " << max(vb->visCube()) <<  endl;
	      			//if !usescratch ...just save
	      			vb->setModelVisCube(Complex(0.0, 0.0));
	      			itsMappers.degrid(*vb, useScratch_p);
	      			if(writeAccess_p && useScratch_p)
	      				wvi_p->setVis(vb->modelVisCube(),VisibilityIterator::Model);

	      		}
	      	}
	      	itsMappers.finalizeDegrid(*vb);
	      }

  }

  //////////////////
  ////////////////////
  Bool SynthesisImager::weight(const String& type, const String& rmode,
                   const Quantity& noise, const Double robust,
                   const Quantity& fieldofview,
  		    const Int npixels, const Bool multiField)
  {
    LogIO os(LogOrigin("SynthesisImager", "weight()", WHERE));

       try {
    	//Int nx=itsMaxShape[0];
    	//Int ny=itsMaxShape[1];
    	Quantity cellx=Quantity(itsMaxCoordSys.increment()[0], itsMaxCoordSys.worldAxisUnits()[0]);
    	Quantity celly=Quantity(itsMaxCoordSys.increment()[1], itsMaxCoordSys.worldAxisUnits()[1]);
      os << LogIO::NORMAL // Loglevel INFO
         << "Imaging weights : " ; //<< LogIO::POST;

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

    		  imwgt_p=VisImagingWeight(*vi_p, rmode, noise, robust,
                                 actualNPixels, actualNPixels, actualCellSize,
                                 actualCellSize, 0, 0, multiField);

    	  }
    	  else {
    		  //this->unlock();
    		  os << LogIO::SEVERE << "Unknown weighting " << type
    				  << LogIO::EXCEPTION;
    		  return False;
    	  }
      }

      vi_p->useImagingWeight(imwgt_p);
      ///////////////revert to vi/vb
      rvi_p->useImagingWeight(imwgt_p);
      ///////////////////////////////


      return True;
 
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in Weighting : "+x.getMesg()) );
      }


   return True;
  }
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////
  ////////////This should be called  at each defineimage
    void SynthesisImager::appendToMapperList(String imagename,  CoordinateSystem& csys, String ftmachine,
    		Quantity distance, Int facets, const Bool overwrite)
    {
    	if(facets > 1 && itsMappers.nMappers() > 0)
    		throw(AipsError("Facetted image has to be first of multifields"));
    	if(facets <1)
    		facets=1;
    	Int nIm=facets*facets;

    CountedPtr<SIImageStore> imstor;
    

    if (nIm < 2){
      imstor=new SIImageStore(imagename, csys, itsCurrentShape, overwrite);
    }
    else{
      if(!unFacettedImStore_p.null())
    	  throw(AipsError("A facetted Image has already been set"));
      unFacettedImStore_p=new SIImageStore(imagename, csys, itsCurrentShape, overwrite);
      facetsStore_p=facets;
    }

     for (Int facet=0; facet< nIm; ++facet){
       if(nIm > 1)
    	   imstor=unFacettedImStore_p->getFacetImageStore(facet, nIm);
       CountedPtr<FTMachine> ftm, iftm;

       createFTMachine(ftm, iftm, ftmachine, facets);
       Int id=itsMappers.nMappers();
       // Fill in miscellaneous information needed by FITS
         ROMSColumns msc(*mss_p[0]);
         Record info;
         String objectName=msc.field().name()(msc.fieldId()(0));
         //ImageInfo iinfo=(imstor->model())->imageInfo();
         //iinfo.setObjectName(objectName);
         //(imstor->model())->setImageInfo(iinfo);
         String telescop=msc.observation().telescopeName()(0);
         info.define("OBJECT", objectName);
         info.define("TELESCOP", telescop);
         info.define("INSTRUME", telescop);
         info.define("distance", distance.get("m").getValue());
	 ////////////// Send misc info into ImageStore. 
	 imstor->setImageInfo( info );
	 /////////////
         //(imstor->model())->setMiscInfo(info);
         ////((imstor->model())->table()).tableInfo().setSubType("GENERIC");
         //(imstor->model())->setUnits(Unit("Jy/pixel"));
         //(imstor->image())->setUnits(Unit("Jy/beam"));
       CountedPtr<SIMapperBase> thismap=new SIMapper(imstor, ftm, iftm, id);
       itsMappers.addMapper(thismap);
     }

   
    
  }

  /////////////////////////
  ////////////////////////
  CoordinateSystem SynthesisImager::buildCoordSys(const MDirection& phasecenter, const Quantity& cellx,
		  const Quantity& celly, const Int nx, const Int ny,
		  const String& stokes, const Projection& projection, const Int nchan,
		  const Quantity& freqStart, const Quantity& freqStep, const Vector<Quantity>& restFreq, const MFrequency::Types freqFrame)
  {
    LogIO os( LogOrigin("SynthesisImager","build",WHERE) );

    // At this stage one ms at least should have been assigned
    ROMSColumns msc(*mss_p[0]);
    phaseCenter_p=phasecenter;
    MVDirection mvPhaseCenter(phasecenter.getAngle());
    // Normalize correctly
    MVAngle ra=mvPhaseCenter.get()(0);
    ra(0.0);
    itsCurrentShape=IPosition(4, nx, ny, 0, nchan);

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
    Bool freqFrameValid=False;
    if(!(MeasTable::Observatory(obsPosition, telescop))){
      os << LogIO::WARN << "Did not get the position of " << telescop 
	 << " from data repository" << LogIO::POST;
      os << LogIO::WARN 
	 << "Please contact CASA to add it to the repository."
	 << LogIO::POST;
      os << LogIO::WARN << "Frequency conversion will not work " << LogIO::POST;

    }
    else{
    	mLocation_p=obsPosition;
    	freqFrameValid = True;
    }
     //Make sure frame conversion is switched off for REST frame data.
    freqFrameValid=freqFrameValid && (freqFrame != MFrequency::REST);
    Vector<Double> deltas(2);
    deltas(0)=-cellx.get("rad").getValue();
    deltas(1)=celly.get("rad").getValue();
    Matrix<Double> xform(2,2);
    xform=0.0;xform.diagonal()=1.0;
    DirectionCoordinate
      myRaDec(MDirection::Types(phasecenter.getRefPtr()->getType()),
	      projection,
	      refCoord(0), refCoord(1),
	      deltas(0), deltas(1),
	      xform,
	      refPixel(0), refPixel(1));

    SpectralCoordinate mySpectral(freqFrameValid ? MFrequency::LSRK : freqFrame, freqStart, freqStep, 0, restFreq.nelements() >0 ? restFreq[0]: Quantity(0.0, "Hz"));
    for (uInt k=1 ; k < restFreq.nelements(); ++k)
      mySpectral.setRestFrequency(restFreq[k].getValue("Hz"));
    
    Vector<Int> whichStokes = decideNPolPlanes(stokes);
    if(whichStokes.nelements()==0)
      throw(AipsError("Stokes selection of " +stokes+ " is invalid"));
    Int nstokes=whichStokes.nelements();
    itsCurrentShape(2)=nstokes;
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
    itsCurrentCoordSys=csys;
    if( (itsMappers.nMappers()==0) || (nx*ny > itsMaxShape[0]*itsMaxShape[1]))
    {
    	itsMaxShape=itsCurrentShape;
    	itsMaxCoordSys=itsCurrentCoordSys;
    }

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

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  // Make the FT-Machine and related objects (cfcache, etc.)
  void SynthesisImager::createFTMachine(CountedPtr<FTMachine>& theFT, CountedPtr<FTMachine>& theIFT, const String& ftname,
		  const Int facets)
  {



	Int cache=1000000000;
	Int tile=16;
    LogIO os( LogOrigin("SynthesisImager","createFTMachine",WHERE));
    ////////////these has to be defined by setupImaging it seems and passed here

    Int wprojplane=ftmParams_p.asInt("wprojplanes");
    Float padding=ftmParams_p.asFloat("padding");
    Bool useAutocorr=ftmParams_p.asBool("useautocorr");
    Bool useDoublePrec=ftmParams_p.asBool("usedoubleprec");
    String gridFunction=ftmParams_p.asString("gridfunc");
           //////////////////////////////////////////////
    if(ftname=="GridFT"){
      if(facets >1){
    	  theFT=new GridFT(cache, tile, gridFunction, mLocation_p, phaseCenter_p, padding, useAutocorr, useDoublePrec);
    	  theIFT=new GridFT(cache, tile, gridFunction, mLocation_p, phaseCenter_p, padding, useAutocorr, useDoublePrec);

      }
      else{
    	  theFT=new GridFT(cache, tile, gridFunction, mLocation_p, padding, useAutocorr, useDoublePrec);
    	  theIFT=new GridFT(cache, tile, gridFunction, mLocation_p, padding, useAutocorr, useDoublePrec);
      }
    }
    else if(ftname== "WProjectFT"){
      theFT=new WProjectFT(wprojplane,  mLocation_p,
			   cache/2, tile, useAutocorr, padding, useDoublePrec);
      theIFT=new WProjectFT(wprojplane,  mLocation_p,
			   cache/2, tile, useAutocorr, padding, useDoublePrec);
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
  void SynthesisImager::createVisSet(const Bool writeAccess)
  {
    LogIO os( LogOrigin("SynthesisImager","createVisSet",WHERE) );
    if(mss_p.nelements() != uInt(fselections_p.size()) && (fselections_p.size() !=0)){
      throw(AipsError("Discrepancy between Number of MSs and Frequency selections"));
    }
    vi_p=new vi::VisibilityIterator2(mss_p, vi::SortColumns(), writeAccess);
    if(fselections_p.size() !=0)
      vi_p->setFrequencySelection (fselections_p);
    //return *vi_p;
    ////////////Temporary revert to vi/vb
    Block<Int> sort(0);
    Block<MeasurementSet> msblock(mss_p.nelements());
    //for (uInt k=0; k< msblock.nelements(); ++k){
    //	msblock[k]=*mss_p[k];
    //}

    //vs_p= new VisSet(blockMSSel_p, sort, noChanSel, useModelCol_p);
    if(!writeAccess){

    	rvi_p=new ROVisibilityIterator(mss4vi_p, sort);

    }
    else{
    	wvi_p=new VisibilityIterator(mss4vi_p, sort);
    	rvi_p=wvi_p;
    }
    Block<Vector<Int> > blockGroup(msblock.nelements());
    for (uInt k=0; k < msblock.nelements(); ++k){
    	blockGroup[k].resize(blockSpw_p[k].nelements());
    	blockGroup[k].set(1);
    	cerr << "start " << blockStart_p[k] << " nchan " << blockNChan_p[k] << " step " << blockStep_p[k] << " spw "<< blockSpw_p[k] <<endl;
    }

    rvi_p->selectChannel(blockGroup, blockStart_p, blockNChan_p,
    			  blockStep_p, blockSpw_p);
    rvi_p->useImagingWeight(VisImagingWeight("natural"));
    ////////////////////end of revert vi/vb

  }// end of createVisSet

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  void SynthesisImager::runMajorCycle(const Bool dopsf, const Bool useViVb2)
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );

    if(useViVb2){
    	vi_p->originChunks();
    	vi_p->origin();
    	vi::VisBuffer2* vb=vi_p->getVisBuffer();
    	if(!dopsf) itsMappers.initializeDegrid(*vb);
    	itsMappers.initializeGrid(*vb);
    	for (vi_p->originChunks(); vi_p->moreChunks();vi_p->nextChunk())
    	{

    		for (vi_p->origin(); vi_p->more();vi_p->next())
    		{
    			if(!dopsf){
    				vb->setVisCubeModel(Cube<Complex>(vb->visCubeModel().shape(), Complex(0.0, 0.0)));
    				itsMappers.degrid(*vb, useScratch_p);
    				if(vi_p->isWritable() && useScratch_p)
    					vi_p->writeVisModel(vb->visCubeModel());
    			}
    			itsMappers.grid(*vb, dopsf);
    		}
    	}
    	if(!dopsf) itsMappers.finalizeDegrid(*vb);
    	itsMappers.finalizeGrid(*vb, dopsf);
    }

    else{
    	VisBufferAutoPtr vb(rvi_p);
    	rvi_p->originChunks();
    	rvi_p->origin();
    	if(!dopsf)itsMappers.initializeDegrid(*vb);
    	itsMappers.initializeGrid(*vb);
    	for (rvi_p->originChunks(); rvi_p->moreChunks();rvi_p->nextChunk())
    	{

    		for (rvi_p->origin(); rvi_p->more(); (*rvi_p)++)
    		{
		  //		  cerr << "nRows "<< vb->nRow() << "   " << max(vb->visCube()) <<  endl;
    			if(!dopsf) {
    				vb->setModelVisCube(Complex(0.0, 0.0));
    				itsMappers.degrid(*vb, useScratch_p);
    				if(writeAccess_p && useScratch_p)
    					wvi_p->setVis(vb->modelVisCube(),VisibilityIterator::Model);
    			}
    			itsMappers.grid(*vb, dopsf);
    		}
    	}
    	//cerr << "IN SYNTHE_IMA" << endl;
    	VisModelData::listModel(rvi_p->getMeasurementSet());
    	if(!dopsf) itsMappers.finalizeDegrid(*vb);
    	itsMappers.finalizeGrid(*vb, dopsf);

    }
/*
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
//	    for(Int mp=0;mp<nmappers;mp++)
//	      {
		// itsMappers.degrid(mp /* ,vb );
		// resultvb.modelVisCube += vb.modelVisCube()
//	      }
	    
	    // resultvb.visCube -= resultvb.modelvisCube()
	    
	    // save model either in the column, or in the record. 
	    // Access the FTM record as    rec=mappers[mp]->getFTMRecord();
	    
//	    for(Int mp=0;mp<nmappers;mp++)
//	      {
//		itsMappers.grid(mp /* ,vb );
//	      }
//	  }// end of for vb.
 //     }// end of vi.chunk iterations
      

      /////////// (3) Finalize Mappers.
//      for(Int mp=0;mp<nmappers;mp++)
//	{
//	  itsMappers.finalizeDegrid(mp);
//	  itsMappers.finalizeGrid(mp);
//	}
*/
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


} //# NAMESPACE CASA - END

