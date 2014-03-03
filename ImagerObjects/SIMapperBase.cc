//# SIMapperBase.cc: Implementation of Imager.h
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

#include <synthesis/ImagerObjects/SIMapperBase.h>
#include <synthesis/ImagerObjects/SIIterBot.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapperBase::SIMapperBase( CountedPtr<VPSkyJones>& vp)
  {
    LogIO os( LogOrigin("SIMapperBase","Construct a mapper",WHERE) );
    
    itsIsModelUpdated = False;
    
    if ( !vp.null() )
      {
	ejgrid_p = vp;
	ejdegrid_p = vp;
	itsDoImageMosaic = True;
      }
    else
      {
	ejgrid_p = NULL;
	ejdegrid_p = NULL;
	itsDoImageMosaic = False;
      }
    
  }
  
  SIMapperBase::~SIMapperBase() 
  {
    LogIO os( LogOrigin("SIMapperBase","destructor",WHERE) );
  }
  
  /*  
      Bool SIMapperBase::releaseImageLocks() 
      {
      LogIO os( LogOrigin("SIMapperBase","releaseImageLocks",WHERE) );
      return itsImages->releaseLocks();
      }
  */
  
  //------------------------------------------------------------------------------------------------------------
  /// NEW
  void SIMapperBase::initializeGridCore2(const vi::VisBuffer2& vb, 
					 CountedPtr<FTMachine>&  ftm,
					 ImageInterface<Complex>& complexImage){
    //					 Matrix<Float>& sumWeights){
    try   {
      LogIO os( LogOrigin("SIMapperBase","initializeGridCore2",WHERE) );
      if( ftm.null() ) return;
      Matrix<Float> sumWeights;
      /// no need ?  ftm->stokesToCorrelation(*(itsImages->weight()), *(itsImages->backwardGrid()));
      ftm->initializeToSky( complexImage, sumWeights, vi::VisBuffer2Adapter(&vb) );
      
    } catch(AipsError &x){
      throw(AipsError("initGridCore : "+x.getMesg()));
    }
  }
  /// OLD
  void SIMapperBase::initializeGridCore(const VisBuffer& vb, 
					CountedPtr<FTMachine>&  ftm,
					ImageInterface<Complex>& complexImage) {
    //					Matrix<Float>& sumWeights){
    try {
      LogIO os( LogOrigin("SIMapperBase","initializeGridCore",WHERE) );
      if( ftm.null() ) return;
      Matrix<Float> sumWeights;
      /// no need ?  ftm->stokesToCorrelation(*(itsImages->weight()), *(itsImages->backwardGrid()));
      ftm->initializeToSky( complexImage, sumWeights, vb );
      
    } catch(AipsError &x){
      throw(AipsError("initGridCore : "+x.getMesg()));
    }
  }// end of initializeGridCore
  
  
  
  //------------------------------------------------------------------------------------------------------------
  
  // NEW
  void SIMapperBase::gridCore2(const vi::VisBuffer2& vb, Bool dopsf, FTMachine::Type col,
			       CountedPtr<FTMachine>&  ftm, Int row){
    try   {
      if( ftm.null() ) return;
      ftm->put( vi::VisBuffer2Adapter(&vb), row, dopsf, col );
      
    } catch(AipsError &x){
      throw(AipsError("gridCore : "+x.getMesg()));
    }
  }
  // OLD
  void SIMapperBase::gridCore(const VisBuffer& vb, Bool dopsf, FTMachine::Type col,
			      CountedPtr<FTMachine>&  ftm, Int row){
    try   {
      if( ftm.null() ) return;
      ftm->put( vb, row, dopsf, col );
      
    } catch(AipsError &x){
      throw(AipsError("gridCore : "+x.getMesg()));
    }
  }
  
  //------------------------------------------------------------------------------------------------------------
  
  void SIMapperBase::finalizeGridCore(const Bool dopsf, 
				      CountedPtr<FTMachine>&  ftm,
				      ImageInterface<Float>& targetImage,
				      ImageInterface<Float>& weightImage,
				      Bool fillWeightImage){
    try   {
      LogIO os( LogOrigin("SIMapperBase","finalizeGridCore",WHERE) );
      if( ftm.null() ) return;

      Matrix<Float> sumWeights;

      /////// Uses internally held complex grids.
      ftm->finalizeToSky();
      ftm->correlationToStokes(   ftm->getImage( sumWeights, False )  , targetImage , dopsf );

      if( fillWeightImage) { ftm->getWeightImage( weightImage, sumWeights ); }

      addImageMiscInfo( targetImage, ftm, sumWeights );      

    } catch(AipsError &x){
      throw(AipsError("finalizeGridCore : "+x.getMesg()));
    }
  }
  
  //------------------------------------------------------------------------------------------------------------
  
  /// NEW
  void SIMapperBase::initializeDegridCore2(const vi::VisBuffer2& vb, 
					   CountedPtr<FTMachine>&  ftm,
					   ImageInterface<Float>& modelImage,
					   ImageInterface<Complex>& complexImage){
    try   {
      LogIO os( LogOrigin("SIMapperBase","initializeDegridCore",WHERE) );
      if( ftm.null() ) return;
      // ( Call findConvFunction for AWP here )
      ftm->stokesToCorrelation( modelImage, complexImage );
      if(vb.polarizationFrame()==MSIter::Linear) {
	StokesImageUtil::changeCStokesRep( complexImage , StokesImageUtil::LINEAR);
      } else {
	StokesImageUtil::changeCStokesRep( complexImage , StokesImageUtil::CIRCULAR);
      }
      ftm->initializeToVis( complexImage, vi::VisBuffer2Adapter(&vb) ); // Init only grid image, not component image....
      
    } catch(AipsError &x){
      throw(AipsError("initDegridCore : "+x.getMesg()));
    }
  }
  /// OLD
  void SIMapperBase::initializeDegridCore(const VisBuffer& vb, 
					  CountedPtr<FTMachine>&  ftm,
					  ImageInterface<Float>& modelImage,
					  ImageInterface<Complex>& complexImage){
    try{
      LogIO os( LogOrigin("SIMapperBase","initializeDegridCore",WHERE) );
      if( ftm.null() ) return;
      // ( Call findConvFunction for AWP here )
      ftm->stokesToCorrelation( modelImage, complexImage );
      if(vb.polFrame()==MSIter::Linear) {
	StokesImageUtil::changeCStokesRep( complexImage , StokesImageUtil::LINEAR);
      } else {
	StokesImageUtil::changeCStokesRep( complexImage , StokesImageUtil::CIRCULAR);
      }
      ftm->initializeToVis( complexImage, vb ); // Init only grid image, not component image....
      
    } catch(AipsError &x){
      throw(AipsError("initDegridCore : "));
    }
  }
  
  //------------------------------------------------------------------------------------------------------------
  
  /// NEW
  void SIMapperBase::degridCore2(vi::VisBuffer2& vb, CountedPtr<FTMachine>& ftm, 
				 CountedPtr<ComponentFTMachine>& cftm, ComponentList& cl) {
    try   {
      if(ftm.null() and cftm.null())
	return;
      
      Cube<Complex> origCube;
      origCube.assign(vb.visCubeModel()); 
      
      vi::VisBuffer2Adapter vba(&vb);
      
      if( ! ftm.null() ) { ftm->get( vba ); }
      if( ! cftm.null() ) { cftm->get( vba , cl); }
      
      vba.modelVisCube()+=origCube; // ( USE vb.visCubeModel() with new vi/vb , once this is allowed)
      
    } catch(AipsError &x){
      throw(AipsError("degridCore : "+x.getMesg()));
    }
  }
  /// OLD
  void SIMapperBase::degridCore(VisBuffer& vb, CountedPtr<FTMachine>& ftm, 
				CountedPtr<ComponentFTMachine>& cftm, ComponentList& cl) {
    try   {
      if(ftm.null() and cftm.null())
	return;
      
      Cube<Complex> origCube;
      origCube.assign(vb.modelVisCube()); 
      
      if( ! ftm.null() ) { ftm->get(vb); }
      if( ! cftm.null() ) { cftm->get(vb, cl); }
      
      vb.modelVisCube()+=origCube;
      
    } catch(AipsError &x){
      throw(AipsError("degridCore : "+x.getMesg()));
    }
  }

  void SIMapperBase::addImageMiscInfo(ImageInterface<Float>& target, 
				      CountedPtr<FTMachine>& ftm, 
				      Matrix<Float>& sumwt)
  {
    Record miscinfo = target.miscInfo();

    miscinfo.define("sumwt", sumwt);
    miscinfo.define("useweightimage", useWeightImage(ftm));

    target.setMiscInfo( miscinfo );
  }

  Bool SIMapperBase::useWeightImage(CountedPtr<FTMachine>& ftm)
  {
    if( ftm->name() == "GridFT" || ftm->name() == "WProjectFT" )  
      { return False; }
    else
      { return True; }
  }

 
  //------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------
  // -------------------------------  Image Domain Mosaics ---------------------------------------
  //------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------
  
  void SIMapperBase::initializeGridCoreMos(const VisBuffer& vb, 
					   CountedPtr<FTMachine>&  ftm,
					   ImageInterface<Complex>& complexImage)
  //					   Matrix<Float>& sumWeights)
  {
    try   {
      initializeGridCore( vb, ftm, complexImage );
      
      Bool dirDep= (!ejgrid_p.null());
      dirDep= dirDep || ((ftm->name()) == "MosaicFT");
      ovb_p.assign(vb, False);
      ovb_p.updateCoordInfo(&vb, dirDep);
      
    } catch(AipsError &x){
      throw(AipsError("initGridCoreMos : "+x.getMesg()));
    }
  }
  
  void SIMapperBase::gridCoreMos(const VisBuffer& vb, Bool dopsf, FTMachine::Type col,
				 CountedPtr<FTMachine>&  ftm, Int /*dummyrow*/, 
				 ImageInterface<Float>& targetImage,
				 ImageInterface<Float>& weightImage,
				 //				 Matrix<Float>& sumWeights,
				 ImageInterface<Complex>& complexImage)
  {
    try   {
      Int nRow=vb.nRow();
      Bool internalChanges=False;  // Does this VB change inside itself?
      Bool firstOneChanges=False;  // Has this VB changed from the previous one?
      if((ftm->name() != "MosaicFT")    && (ftm->name() != "PBWProjectFT") &&
	 (ftm->name() != "AWProjectFT") && (ftm->name() != "AWProjectWBFT")) {
	changedSkyJonesLogic(vb, firstOneChanges, internalChanges, True);
      }
      //First ft machine change should be indicative
      //anyways right now we are allowing only 1 ftmachine for GridBoth
      Bool IFTChanged=ftm->changed(vb);
      
      Matrix<Float> wgt;
      
      if(internalChanges) {
	// Yes there are changes: go row by row.
	for (Int row=0; row<nRow; row++) {
	  if(IFTChanged||ejgrid_p->changed(vb,row)) {
	    // Need to apply the SkyJones from the previous row
	    // and finish off before starting with this row
	    finalizeGridCoreMos(dopsf, ftm, targetImage, weightImage, complexImage, ovb_p);
	    initializeGridCoreMos(vb,ftm,complexImage);
	  }
	  //ftm->put(vb, row, dopsf, col);
	  gridCore( vb, dopsf, col, ftm, row );
	}
      } else if (IFTChanged || firstOneChanges) {
	//if(!isBeginingOfSkyJonesCache_p){
	//finalizePutSlice(*vb_p, dopsf, cubeSlice, nCubeSlice);
	// }
	//IMPORTANT:We need to finalize here by checking that we are not at the begining of the iteration
	//finalizeGrid(vb_p, dopsf);
	initializeGridCoreMos(vb,ftm,complexImage);
	///ftm->put(vb, -1, dopsf, col);
	gridCore( vb, dopsf, col, ftm, -1 );
      } else  {
	gridCore( vb, dopsf, col, ftm, -1 );
	//       throw(AipsError("Internal error : unsupported image-domain mosaic option"));
      }
      
    } catch(AipsError &x){
      throw(AipsError("gridCoreMos : "+x.getMesg()));
    }
  }// end of gridCoreMos
  
  
  void SIMapperBase::finalizeGridCoreMos(const Bool dopsf,
					 CountedPtr<FTMachine>&  ftm,
					 ImageInterface<Float>& targetImage,
					 ImageInterface<Float>& weightImage,
					 ImageInterface<Complex>& complexImage,
					 VisBuffer& vb)
  {
    try   {
      
      if(ejgrid_p.null()) throw(AipsError("Internal error : null ejgrid..."));
      if(dopsf) throw(AipsError("Internal error : call non-mosaic finalizeGridCore for psf"));
      
      // Actually do the transform. Update weights as we do so.
      ftm->finalizeToSky();
      // 1. Now get the (unnormalized) image and add the
      // weight to the summed weight
      Matrix<Float> sumWeights;
      //get the image in itsImage->backwardGrid() which ift is holding by reference
      ftm->getImage(sumWeights, False);
      
      ////if(ejgrid_p != NULL)
      {
	// Note we apply the state of the previously saved visbuffer vb_p
	// We might have to carry over the row for internal changes
	ejgrid_p->apply(complexImage,complexImage, ovb_p, -1, False);
	TempImage<Float> temp(targetImage.shape(), targetImage.coordinates());
	ftm->correlationToStokes( complexImage, temp, False);
	LatticeExpr<Float> addToRes( targetImage + temp );
	targetImage.copyData(addToRes);
      }
      
      //if(ejgrid_p != NULL && !dopsf)
      {
	TempImage<Float> temp(weightImage.shape(), weightImage.coordinates());
	ftm->getWeightImage(temp, sumWeights);
	ejgrid_p->applySquare(temp, temp, vb, -1);
	LatticeExpr<Float> addToWgt( weightImage + temp );
	weightImage.copyData(addToWgt);
      }
      
      addImageMiscInfo( targetImage, ftm, sumWeights );      
      
    } catch(AipsError &x){
      throw(AipsError("finalizeGridCoreMos : "+x.getMesg()));
    }
    
  }// end of finalizeGridCoreMos
  
  
  
  void SIMapperBase::initializeDegridCoreMos(const VisBuffer& vb, 
					     CountedPtr<FTMachine>&  ftm,
					     ImageInterface<Float>& modelImage,
					     ImageInterface<Complex>& complexImage,
					     CountedPtr<ComponentFTMachine>& cftm,
					     ComponentList& cl)
  {
    try   {
      if(ejdegrid_p.null()) throw(AipsError("Internal error : null ejdegrid..."));
      
      Int row=0;
      
      ftm->stokesToCorrelation(modelImage, complexImage);
      ejgrid_p->apply(complexImage, complexImage, vb, row, True);
      if(vb.polFrame()==MSIter::Linear) {
	StokesImageUtil::changeCStokesRep(complexImage,
					  StokesImageUtil::LINEAR);
      } else {
	StokesImageUtil::changeCStokesRep(complexImage,
					  StokesImageUtil::CIRCULAR);
      }
      ftm->initializeToVis(complexImage, vb);
      
      
      if(!cftm.null()) {
	clCorrupted_p=ComponentList();
	for (uInt k=0; k < cl.nelements(); ++k){
	  SkyComponent comp=cl.component(k).copy();
	  if(vb.polFrame()==MSIter::Linear) {
	    if(comp.flux().pol()==ComponentType::STOKES) {
	      comp.flux().convertPol(ComponentType::LINEAR);
	    }
	  }
	  else {
	    if(comp.flux().pol()==ComponentType::STOKES) {
	      comp.flux().convertPol(ComponentType::CIRCULAR);
	    }
	  }
	  ////We might have to deal with the right row here if the visbuffer is has changed internally
	  ejdegrid_p->apply(comp, comp, vb,row, True);
	  clCorrupted_p.add(comp);
	}
      }
      
    } catch(AipsError &x){
      throw(AipsError("initDegridCoreMos : "+x.getMesg()));
    }
    
  }// end of initDegridCoreMos
  
  void SIMapperBase::degridCoreMos(VisBuffer& vb, CountedPtr<FTMachine>& ftm, 
				   CountedPtr<ComponentFTMachine>& cftm,
				   ImageInterface<Float>& modelImage,
				   ImageInterface<Complex>& complexImage)
  {
    try   {
      
      if(ftm.null() and cftm.null())
	return;
      Int nRow=vb.nRow();
      Cube<Complex> origCube;
      origCube.assign(vb.modelVisCube());
      Bool internalChanges=False;  // Does this VB change inside itself?
      Bool firstOneChanges=False;  // Has this VB changed from the previous one?
      
      if((!ftm.null() && (ftm->name() != "MosaicFT")    && (ftm->name() != "PBWProjectFT") &&
	  (ftm->name() != "AWProjectFT") && (ftm->name() != "AWProjectWBFT")) || (!cftm.null())) {
	changedSkyJonesLogic(vb, firstOneChanges, internalChanges, False);
      }
      //anyways right now we are allowing only 1 ftmachine for GridBoth
      Bool FTChanged=ftm->changed(vb);
      
      if(internalChanges)
	{
	  // Yes there are changes within this buffer: go row by row.
	  // This will automatically catch a change in the FTMachine so
	  // we don't have to check for that.
	  for (Int row=0; row<nRow; row++)
	    {
	      if(FTChanged||ejdegrid_p->changed(vb,row))
		{
		  // Need to apply the SkyJones from the previous row
		  // and finish off before starting with this row
		  finalizeDegridCoreMos();
		  initializeDegridCoreMos(vb, ftm, modelImage, complexImage, cftm, clCorrupted_p );
		}
	      ftm.null() ? cftm->get(vb, clCorrupted_p, row) : ftm->get(vb, row);
	      
	    }
	  
	}
      else if (FTChanged||firstOneChanges) {
	// This buffer has changed wrt the previous buffer, but
	// this buffer has no changes within it. Again we don't need to
	// check for the FTMachine changing.
	finalizeDegridCoreMos();
	initializeDegridCoreMos(vb, ftm, modelImage, complexImage, cftm,clCorrupted_p);
	ftm.null() ? cftm->get(vb, clCorrupted_p) : ftm->get(vb);
      }
      else {
	ftm.null() ? cftm->get(vb, clCorrupted_p) : ftm->get(vb);
      }
      vb.modelVisCube()+=origCube;
      
      
    } catch(AipsError &x){
      throw(AipsError("degridCoreMos : "+x.getMesg()));
    }
    
  }// end of degridCore
  
  
  Bool SIMapperBase::changedSkyJonesLogic(const VisBuffer& vb, 
					  Bool& firstRow, 
					  Bool& internalRow, 
					  const Bool grid){
    try   {
      firstRow=False;
      internalRow=False;
      CountedPtr<VPSkyJones> ej= grid ? ejgrid_p : ejdegrid_p;
      if(ej.null())
	return False;
      if(ej->changed(vb,0))
	firstRow=True;
      Int row2temp=0;
      if(ej->changedBuffer(vb,0,row2temp)) {
	internalRow=True;
      }
      return (firstRow || internalRow) ;
      
    } catch(AipsError &x){
      throw(AipsError("changedSkyJonedLogic : "+x.getMesg()));
    }
  }
  
  
  
} //# NAMESPACE CASA - END

