//# SIMapperCollection.cc: Implementation of Imager.h
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

#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <synthesis/ImagerObjects/SIMapperCollection.h>

#include <synthesis/TransformMachines/VisModelData.h>
#include <images/Regions/WCBox.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  
  SIMapperCollection::SIMapperCollection() 
  {
    LogIO os( LogOrigin("SIMapperCollection","Construct a mapperCollection",WHERE) );
    
    itsMappers.resize(0);
    oldMsId_p=-1;
    itsIsNonZeroModel=False;

  }
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  SIMapperCollection::~SIMapperCollection() 
  {
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  Bool SIMapperCollection::releaseImageLocks() 
  {
    Bool validflag=True;
    for(Int mapperid=0;mapperid<nMappers();mapperid++ )
      {
	validflag &= itsMappers[mapperid]->releaseImageLocks();
      }
    return validflag;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  
  /*
  // Allocate Memory and open images.
  void SIMapperCollection::addMapper( String mappertype,  
				      CountedPtr<SIImageStore> imagestore,
				      CountedPtr<FTMachine> ftmachine,
				      CountedPtr<FTMachine> iftmachine)
  {

    LogIO os( LogOrigin("SIMapperCollection","addMapper",WHERE) );

    CountedPtr<SIMapper> localMapper=NULL;
    Int nMappers = itsMappers.nelements();
    // Check 'mappertype' for valid types....
    if( mappertype == "default" )
      {
	localMapper = new SIMapperSingle( imagestore, ftmachine, iftmachine, nMappers );
      }
    
    else if( mappertype == "multiterm" )
      {
	localMapper = new SIMapperMultiTerm( imagestore, ftmachine, iftmachine,nMappers, ntaylorterms );
      }
    
    else
      {
	throw ( AipsError("Internal Error : Unrecognized Mapper Type in MapperCollection.addMapper") );
      }

    // If all is well, add to the list.
    itsMappers.resize(nMappers+1, True);
    itsMappers[nMappers] = localMapper;

  }
  */

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  void SIMapperCollection::addMapper( CountedPtr<SIMapper> map){
    Int nMappers = itsMappers.nelements();
    itsMappers.resize(nMappers+1, True);
    itsMappers[nMappers]=map;
  } 

  Int SIMapperCollection::nMappers()
  {
    return itsMappers.nelements();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  Vector<String> SIMapperCollection::getImageNames()
  {
    Vector<String> names( nMappers() );

    for(Int mapperid=0;mapperid<nMappers();mapperid++ )
      {
	names[mapperid] = itsMappers[mapperid]->getImageName();
      }

    return names;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////  Start VB dependent code /////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////OLD vi/vb //////////////////////////////////////////////
  void SIMapperCollection::initializeGrid(VisBuffer& vb, Bool dopsf)
  {
	  for (uInt k=0; k < itsMappers.nelements(); ++k)
	  {
	    (itsMappers[k])->initializeGrid(vb,dopsf,True);

  	  }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////OLD VI/VB ///////////////////////////////////
  void SIMapperCollection::grid(VisBuffer& vb, Bool dopsf, FTMachine::Type col)
  {
    if( itsIsNonZeroModel == True ) // Try to subtract model visibilities only if a model exists.
	{
	  if(col==FTMachine::CORRECTED){
		  if(vb.msColumns().correctedData().isNull()){
			  col=FTMachine::OBSERVED;
			  //			  cerr << "Max of visCube" << max(vb.visCube()) << " model " << max(vb.modelVisCube())<< endl;
			  vb.visCube()-=vb.modelVisCube();
		  }
		  else{
			  vb.correctedVisCube()-=vb.modelVisCube();
		  }
	  } 
	  else if (col==FTMachine::OBSERVED) {
			  vb.visCube()-=vb.modelVisCube();
	    }
	}// if non zero model

	  for (uInt k=0; k < itsMappers.nelements(); ++k)
	  {
		  (itsMappers[k])->grid(vb, dopsf, col);

	  }
  }
  ///////////////////////////////
  ////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////OLD VI/VB////////////////////////////////////////////////
    void SIMapperCollection::finalizeGrid(VisBuffer& vb, Bool dopsf)
        {
      	  for (uInt k=0; k < itsMappers.nelements(); ++k)
      	  {
        		  (itsMappers[k])->finalizeGrid(vb, dopsf);

      	  }
        }


//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////OLD VI/VB ///////////////////////////////////////////////////
  void SIMapperCollection::initializeDegrid(VisBuffer& vb)
    {

      itsIsNonZeroModel = anyNonZeroModels();

      if( itsIsNonZeroModel == True )
	{
	  vb.setModelVisCube( Complex(0.0,0.0) );
	  

  	  for (uInt k=0; k < itsMappers.nelements(); ++k)
    	  {
    		  (itsMappers[k])->initializeDegrid(vb);

    	  }
	}// if non zero model 
    }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////OLD VI/VB ////////////////////////////////////////////////////
  void SIMapperCollection::degrid(VisBuffer& vb, Bool saveVirtualMod)
    {
      if( itsIsNonZeroModel == True )
	{
	  	  for (uInt k=0; k < itsMappers.nelements(); ++k)
	    				(itsMappers[k])->degrid(vb);

  		  if(saveVirtualMod){
		    saveVirtualModel(vb);
  		  }
	}// if non zero model
    }


  /////
  /////////////
  void SIMapperCollection::saveVirtualModel(VisBuffer& vb){



	  if(vb.msId() != oldMsId_p){
		  oldMsId_p=vb.msId();
		  /*Block< Vector<Int> > blockNGroup;
    				Block< Vector<Int> > blockStart;
    				Block< Vector<Int> > blockWidth;
    				Block< Vector<Int> > blockIncr;
    				Block< Vector<Int> > blockSpw;
    				vb.getChannelSelection(blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);
    				Vector<Int> fields = vb.msColumns().fieldId().getColumn();
    				const Int option = Sort::HeapSort | Sort::NoDuplicates;
    				const Sort::Order order = Sort::Ascending;
    				Int nfields = GenSort<Int>::sort (fields, order, option);

    				// Make sure  we have the right size

    				fields.resize(nfields, True);
		   */
		  //Int msid = vb.msId();
		  ROVisibilityIterator *viloc=vb.getVisibilityIterator();
		  for (uInt k=0; k < itsMappers.nelements(); ++k){
			  Record rec;
			  String modImage=viloc->ms().getPartNames()[0];
			  if(!((viloc->ms()).source().isNull()))
			    modImage=(viloc->ms()).source().tableName();
			  modImage=File::newUniqueName(modImage, "FT_MODEL").absoluteName();
			  Bool iscomp=itsMappers[k]->getCLRecord(rec);
			  if(iscomp || itsMappers[k]->getFTMRecord(rec)){

				  //VisModelData::putModel(vb.getVisibilityIterator()->ms(), rec, fields, blockSpw[msid], blockStart[msid],
				  //		blockWidth[msid], blockIncr[msid],
				  //                         iscomp, True);
				  VisibilityIterator * elvi=(dynamic_cast<VisibilityIterator* >(vb.getVisibilityIterator()));
				  if(elvi)
					  elvi->putModel(rec, iscomp, True);
				  //				  VisModelData::listModel(vb.getVisibilityIterator()->ms());
			  }

		  }



	  }



  }


  /////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  void SIMapperCollection::finalizeDegrid(VisBuffer& /*vb*/)
    {
      if( itsIsNonZeroModel == True )
	{
  	  for (uInt k=0; k < itsMappers.nelements(); ++k)
  	  {
  		  (itsMappers[k])->finalizeDegrid();

  	  }
	}// if non zero model
    }


  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////// End of VB dependent code.
  ///////////////////////////////////////////////////////////////////////////////////////////////////////



  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////
  CountedPtr<SIImageStore> SIMapperCollection::imageStore(Int id)
  {
	if(uInt(id) < itsMappers.nelements())
	{
		return (itsMappers[id])->imageStore();
	}
	return NULL;
  }
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
  Record SIMapperCollection::getFTMRecord(Int mapperid)
  {
    AlwaysAssert( mapperid >=0 && mapperid < nMappers() , AipsError );
    //return itsMappers[mapperid]->getFTMRecord();
    Record rec;
    return rec;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIMapperCollection::checkOverlappingModels(String action)
  {
    // If nothing that overlaps, don't check.
    if(nMappers()==1) return;

    Int nmodels = nMappers();

    // If there is no model image (i.e. first major cycle with no starting model), don't check.
    Bool hasmodel=True;
    for (Int model=0;model<(nmodels-1); ++model) 
      { hasmodel = hasmodel && ((itsMappers[model])->imageStore())->hasModel();  }
    if( hasmodel==False ) { 
      //cout << "No model images to check overlap for." << endl; 
      return; 
    }

    // Internal check
    AlwaysAssert( action=="blank" || action=="restore" , AipsError );

    for (Int model=0;model<(nmodels-1); ++model) 
      {
	// Connect to one image for aux info.
	SubImage<Float> modelimage( *(((itsMappers[model])->imageStore())->model()), True );

	uInt nTaylor0 = ((itsMappers[model])->imageStore())->getNTaylorTerms();

	CoordinateSystem cs0=modelimage.coordinates();
	IPosition iblc0(modelimage.shape().nelements(),0);
	IPosition itrc0(modelimage.shape());
	itrc0=itrc0-Int(1);
	LCBox lbox0(iblc0, itrc0, modelimage.shape());
	ImageRegion imagreg0(WCBox(lbox0, cs0));

	for (Int nextmodel=model+1; nextmodel < nmodels; ++nextmodel)
	  {
	    SubImage<Float> nextmodelimage( *(((itsMappers[nextmodel])->imageStore())->model()) , True);

	    uInt nTaylor1 = ((itsMappers[nextmodel])->imageStore())->getNTaylorTerms();
	    
	    CoordinateSystem cs=nextmodelimage.coordinates();
	    IPosition iblc(nextmodelimage.shape().nelements(),0);
	    IPosition itrc(nextmodelimage.shape());
	    itrc=itrc-Int(1);
	    LCBox lbox(iblc, itrc, nextmodelimage.shape());
	    ImageRegion imagreg(WCBox(lbox, cs));
	    
	    try{

	      if( action.matches("blank") )
		{
		  LatticeRegion latReg=imagreg.toLatticeRegion(modelimage.coordinates(), modelimage.shape());

		  for(uInt taylor=0;taylor<min(nTaylor0,nTaylor1);taylor++)
		    { // loop for taylor term
		      SubImage<Float> modelim( *(((itsMappers[model])->imageStore())->model(taylor)), True );
		      SubImage<Float> partToMask(modelim, imagreg, True);
		      ArrayLattice<Bool> pixmask(latReg.get());
		      LatticeExpr<Float> myexpr(iif(pixmask, 0.0, partToMask) );
		      partToMask.copyData(myexpr);
		    }


		}
	      else // "restore"
		{
		LatticeRegion latReg0=imagreg0.toLatticeRegion(nextmodelimage.coordinates(), nextmodelimage.shape());
		LatticeRegion latReg=imagreg.toLatticeRegion(modelimage.coordinates(), modelimage.shape());
		ArrayLattice<Bool> pixmask(latReg.get());

		
		for(uInt taylor=0;taylor<min(nTaylor0,nTaylor1);taylor++)
		  {// loop for taylor term
		    SubImage<Float> modelim( *(((itsMappers[model])->imageStore())->model(taylor)), True );
		    SubImage<Float> nextmodelim( *(((itsMappers[nextmodel])->imageStore())->model(taylor)), True );

		    SubImage<Float> partToMerge(nextmodelim, imagreg0, True);
		    SubImage<Float> partToUnmask(modelim, imagreg, True);
		    LatticeExpr<Float> myexpr0(iif(pixmask,partToMerge,partToUnmask));
		    partToUnmask.copyData(myexpr0);
		  }
		
		}
	    }
	    catch(AipsError &x){
	      //	      cout << "Hmm.... in here : "<< x.getMesg() << endl;
	      //no overlap you think ?
	      /*
		os << LogIO::WARN
		<< "no overlap or failure of copying the clean components"
		<< x.getMesg()
		<< LogIO::POST;
	*/
	      continue;
	    }
	  }
      }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  Bool SIMapperCollection::anyNonZeroModels()
  {
    Bool validmodel=False;
    // If any one Mapper has a valid and nonzero model, return True.
    for (Int model=0;model<nMappers(); ++model) 
      { 
	//	validmodel = ((itsMappers[model])->imageStore())->hasModel() && 
	//	                        ( ! ( ((itsMappers[model])->imageStore())->isModelEmpty() ));
	validmodel =  ! ( ((itsMappers[model])->imageStore())->isModelEmpty() );
      }
    //cout << "anyNonZeroModel : " << validmodel << endl;
    return validmodel;
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END

