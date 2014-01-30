//# SIMapper.cc: Implementation of SIMapper.h
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

#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBuffer2Adapter.h>
#include <synthesis/TransformMachines/BeamSkyJones.h>
#include <synthesis/TransformMachines/SkyJones.h>
#include <synthesis/TransformMachines/SimpleComponentFTMachine.h>
#include <synthesis/TransformMachines/SimpCompGridMachine.h>
#include <synthesis/ImagerObjects/SIMapper.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapper::SIMapper( CountedPtr<SIImageStore>& imagestore, 
		      CountedPtr<FTMachine>& ftm, CountedPtr<FTMachine>& iftm,
		      Int mapperid)
  : SIMapperBase( imagestore, ftm, mapperid ),
    vb_p (vi::VisBuffer2::factory (vi::VbPlain, vi::VbRekeyable))
  {
    LogIO os( LogOrigin("SIMapper","Construct a mapper",WHERE) );
    ft_p=ftm;
    ift_p=iftm;
    ejgrid_p=NULL;
    ejdegrid_p=NULL;
    itsImages=imagestore;
    itsMapperId=mapperid;
  }
  
  SIMapper::SIMapper(const ComponentList& cl, String& whichMachine, Int mapperid)
  : SIMapperBase(NULL, NULL, mapperid),
    vb_p (vi::VisBuffer2::factory (vi::VbPlain, vi::VbRekeyable))
  {
    ft_p=NULL;
    ift_p=NULL;
    itsImages=NULL;
    ejgrid_p=NULL;
    ejdegrid_p=NULL;
    itsMapperId=mapperid;
    if(whichMachine=="SimpleComponentFTMachine")
      cft_p=new SimpleComponentFTMachine();
    else
      //SD style component gridding
      cft_p=new SimpleComponentGridMachine();
    cl_p=cl;
    clCorrupted_p=cl;
      
  }
  
  SIMapper::~SIMapper() 
  {
      delete ejgrid_p;
      delete ejdegrid_p;
      delete vb_p;
  }
  
  // #############################################
  // #############################################
  // #######  Gridding / De-gridding functions ###########
  // #############################################
  // #############################################


  /// All these take in vb's, and just pass them on.


   void SIMapper::initializeGrid(const vi::VisBuffer2& vb)
    {

      LogIO os( LogOrigin("SIMapper","initializeGrid",WHERE) );
      // itsFTM->initializeToSky( itsResidual, vb )
      //Componentlist FTM has nothing to do
      if(ift_p.null())
      	return;
      Bool dirDep= (ejgrid_p != NULL);
      //The Matrix Weight is not needed or is needed when we do not need the weight Image full
      Matrix<Float> wgt;
      ift_p->stokesToCorrelation(*(itsImages->weight()), *(itsImages->backwardGrid()));
      ift_p->initializeToSky(*(itsImages->backwardGrid()), wgt,
      				   vi::VisBuffer2Adapter(&vb));
      dirDep= dirDep || ((ift_p->name()) == "MosaicFT");

      // assertSkyJones(vb, -1);
      //vb_p is used to finalize things if vb has changed propoerties
      //vb_p->assign(vb, False);
      vb_p->copyCoordinateInfo(& vb, dirDep);


    }

   void SIMapper::initializeGrid(const VisBuffer& vb)
     {

       LogIO os( LogOrigin("SIMapper","initializeGrid",WHERE) );
       // itsFTM->initializeToSky( itsResidual, vb )
       //Componentlist FTM has nothing to do
       if(ift_p.null())
       	return;
       Bool dirDep= (ejgrid_p != NULL);
       //The Matrix Weight is not needed or is needed when we do not need the weight Image full
       Matrix<Float> wgt;
       ift_p->stokesToCorrelation(*(itsImages->weight()), *(itsImages->backwardGrid()));
       ift_p->initializeToSky(*(itsImages->backwardGrid()), wgt,
       				   vb);
       dirDep= dirDep || ((ift_p->name()) == "MosaicFT");

       // assertSkyJones(vb, -1);
       //vb_p is used to finalize things if vb has changed propoerties
       //vb_p->assign(vb, False);
       ovb_p.assign(vb, False);
       ovb_p.updateCoordInfo(&vb, dirDep);


     }

  void SIMapper::grid(const vi::VisBuffer2& vb, Bool dopsf, FTMachine::Type col)
  {
    LogIO os( LogOrigin("SIMapper","grid",WHERE) );
    //Componentlist FTM has no gridding to do
    if(ift_p.null())
    	return;
    Int nRow=vb.nRows();
    const vi::VisBuffer2Adapter vba(&vb);
    Bool internalChanges=False;  // Does this VB change inside itself?
    Bool firstOneChanges=False;  // Has this VB changed from the previous one?
    if((ift_p->name() != "MosaicFT")    && (ift_p->name() != "PBWProjectFT") &&
           (ift_p->name() != "AWProjectFT") && (ift_p->name() != "AWProjectWBFT"))
    {
            changedSkyJonesLogic(vb, firstOneChanges, internalChanges, True);
    }
    //First ft machine change should be indicative
    //anyways right now we are allowing only 1 ftmachine for GridBoth
    Bool IFTChanged=ift_p->changed(vba);


    // we might need to recompute the "sky" for every single row, but we
    // avoid this if possible.


    if(internalChanges) {
    	// Yes there are changes: go row by row.

    	for (Int row=0; row<nRow; row++) {
    		if(IFTChanged||ejgrid_p->changed(vba,row))
    		{
    			// Need to apply the SkyJones from the previous row
    			// and finish off before starting with this row
    			finalizeGrid(* vb_p, dopsf);
    			initializeGrid(vb);
    		}


    		ift_p->put(vba, row, dopsf, col);

    	}
    }
    else if (IFTChanged || firstOneChanges) {


    	//if(!isBeginingOfSkyJonesCache_p){
    	//finalizePutSlice(*vb_p, dopsf, cubeSlice, nCubeSlice);
    	// }
    	//IMPORTANT:We need to finalize here by checking that we are not at the begining of the iteration
    	//finalizeGrid(vb_p, dopsf);
    	initializeGrid(vb);

    	ift_p->put(vba, -1, dopsf, col);

    }
    else {
    	ift_p->put(vba, -1, dopsf, col);

    }

    //isBeginingOfSkyJonesCache_p=False;

  }

  /////////////////OLD vi/vb version
  void SIMapper::grid(const VisBuffer& vb, Bool dopsf, FTMachine::Type col)
   {
     LogIO os( LogOrigin("SIMapper","grid",WHERE) );
     //Componentlist FTM has no gridding to do
     if(ift_p.null())
     	return;
     Int nRow=vb.nRow();
     Bool internalChanges=False;  // Does this VB change inside itself?
     Bool firstOneChanges=False;  // Has this VB changed from the previous one?
     if((ift_p->name() != "MosaicFT")    && (ift_p->name() != "PBWProjectFT") &&
            (ift_p->name() != "AWProjectFT") && (ift_p->name() != "AWProjectWBFT"))
     {
             changedSkyJonesLogic(vb, firstOneChanges, internalChanges, True);
     }
     //First ft machine change should be indicative
     //anyways right now we are allowing only 1 ftmachine for GridBoth
     Bool IFTChanged=ift_p->changed(vb);


     // we might need to recompute the "sky" for every single row, but we
     // avoid this if possible.


     if(internalChanges) {
     	// Yes there are changes: go row by row.

     	for (Int row=0; row<nRow; row++) {
     		if(IFTChanged||ejgrid_p->changed(vb,row))
     		{
     			// Need to apply the SkyJones from the previous row
     			// and finish off before starting with this row
     			finalizeGrid(ovb_p, dopsf);
     			initializeGrid(vb);
     		}


     		ift_p->put(vb, row, dopsf, col);

     	}
     }
     else if (IFTChanged || firstOneChanges) {


     	//if(!isBeginingOfSkyJonesCache_p){
     	//finalizePutSlice(*vb_p, dopsf, cubeSlice, nCubeSlice);
     	// }
     	//IMPORTANT:We need to finalize here by checking that we are not at the begining of the iteration
     	//finalizeGrid(vb_p, dopsf);
     	initializeGrid(vb);

     	ift_p->put(vb, -1, dopsf, col);

     }
     else {
     	ift_p->put(vb, -1, dopsf, col);

     }

     //isBeginingOfSkyJonesCache_p=False;

   }


  //// The function that makes the PSF should check its validity, and fit the beam,
  void SIMapper::finalizeGrid(const vi::VisBuffer2& vb, const Bool dopsf)
  {
    LogIO os( LogOrigin("SIMapper","finalizeGrid",WHERE) );

    // TODO : Fill in itsResidual, itsPsf, itsWeight.
    // Do not normalize the residual by the weight. 
    //   -- Normalization happens later, via 'divideResidualImageByWeight' called from SI.divideImageByWeight()
    //   -- This will ensure that normalizations are identical for the single-node and parallel major cycles. 
    if(ift_p.null())
    	return;
    // Actually do the transform. Update weights as we do so.
    ift_p->finalizeToSky();
    // 1. Now get the (unnormalized) image and add the
    // weight to the summed weight
    Matrix<Float> delta;
    //get the image in itsImage->backwardGrid() which ift is holding by reference
    ift_p->getImage(delta, False);
    //iftm_p[field]->finalizeToSky( imPutSliceVec , gSSliceVec , ggSSliceVec , fluxScaleVec, dopsf , weightSliceVec );
    //weightSlice_p[model]+=delta;
    if(dopsf)
    	ift_p->correlationToStokes(*(itsImages->backwardGrid()), *(itsImages->psf()), True);
    else{
    	if(ejgrid_p != NULL){
    		// Note we apply the state of the previously saved visbuffer vb_p
    		// We might have to carry over the row for internal changes
    		ejgrid_p->apply(*(itsImages->backwardGrid()),*(itsImages->backwardGrid()),
    		                vi::VisBuffer2Adapter(vb_p), -1, False);
    		TempImage<Float> temp((itsImages->residual())->shape(), (itsImages->residual())->coordinates());
    		ift_p->correlationToStokes(*(itsImages->backwardGrid()), temp, False);
    		LatticeExpr<Float> addToRes( *(itsImages->residual()) + temp );
    		(itsImages->residual())->copyData(addToRes);
    	}
    	else
    		ift_p->correlationToStokes(*(itsImages->backwardGrid()), *(itsImages->residual()), False);
    }
    Matrix<Float> wgt;

    if(ejgrid_p != NULL && !dopsf){
    	TempImage<Float> temp((itsImages->weight())->shape(), (itsImages->weight())->coordinates());
    	ift_p->getWeightImage(temp, wgt);
    	ejgrid_p->applySquare(temp, temp, vi::VisBuffer2Adapter(&vb), -1);
    	LatticeExpr<Float> addToWgt( *(itsImages->weight()) + temp );
    	(itsImages->weight())->copyData(addToWgt);
    }
    else
    {
    	ift_p->getWeightImage(*(itsImages->weight()), wgt);
    }


  }

  //////////////OLD VI/VB version
  void SIMapper::finalizeGrid(const VisBuffer& vb, const Bool dopsf)
    {
      LogIO os( LogOrigin("SIMapper","finalizeGrid",WHERE) );

      // TODO : Fill in itsResidual, itsPsf, itsWeight.
      // Do not normalize the residual by the weight.
      //   -- Normalization happens later, via 'divideResidualImageByWeight' called from SI.divideImageByWeight()
      //   -- This will ensure that normalizations are identical for the single-node and parallel major cycles.
      if(ift_p.null())
      	return;
      // Actually do the transform. Update weights as we do so.
      ift_p->finalizeToSky();
      // 1. Now get the (unnormalized) image and add the
      // weight to the summed weight
      Matrix<Float> delta;
      //get the image in itsImage->backwardGrid() which ift is holding by reference
      ift_p->getImage(delta, False);
      //iftm_p[field]->finalizeToSky( imPutSliceVec , gSSliceVec , ggSSliceVec , fluxScaleVec, dopsf , weightSliceVec );
      //weightSlice_p[model]+=delta;
      if(dopsf)
      	ift_p->correlationToStokes(*(itsImages->backwardGrid()), *(itsImages->psf()), True);
      else{
      	if(ejgrid_p != NULL){
      		// Note we apply the state of the previously saved visbuffer vb_p
      		// We might have to carry over the row for internal changes
      		ejgrid_p->apply(*(itsImages->backwardGrid()),*(itsImages->backwardGrid()), ovb_p, -1, False);
      		TempImage<Float> temp((itsImages->residual())->shape(), (itsImages->residual())->coordinates());
      		ift_p->correlationToStokes(*(itsImages->backwardGrid()), temp, False);
      		LatticeExpr<Float> addToRes( *(itsImages->residual()) + temp );
      		(itsImages->residual())->copyData(addToRes);
      	}
      	else
      		ift_p->correlationToStokes(*(itsImages->backwardGrid()), *(itsImages->residual()), False);
      }
      Matrix<Float> wgt;

      if(ejgrid_p != NULL && !dopsf){
      	TempImage<Float> temp((itsImages->weight())->shape(), (itsImages->weight())->coordinates());
      	ift_p->getWeightImage(temp, wgt);
      	ejgrid_p->applySquare(temp, temp, vb, -1);
      	LatticeExpr<Float> addToWgt( *(itsImages->weight()) + temp );
      	(itsImages->weight())->copyData(addToWgt);
      }
      else
      {
      	ift_p->getWeightImage(*(itsImages->weight()), wgt);
      }


    }


  void SIMapper::initializeDegrid(const vi::VisBuffer2& vb, const Int row)
  {
    LogIO os( LogOrigin("SIMapper", "initializeDegrid",WHERE) );
    if(ft_p.null() && cft_p.null())
    	return;

    if(!ft_p.null()) {
    	ft_p->stokesToCorrelation(*(itsImages->model()), *(itsImages->forwardGrid()));
    	if(ejdegrid_p != NULL)
    		ejgrid_p->apply(*(itsImages->forwardGrid()),*(itsImages->forwardGrid()), vi::VisBuffer2Adapter(&vb), row, True);
    	// Change the model polarization frame
    	if(vb.polarizationFrame()==MSIter::Linear) {
    		StokesImageUtil::changeCStokesRep(*(itsImages->forwardGrid()),
    						  StokesImageUtil::LINEAR);
    	}
    	else {
    		StokesImageUtil::changeCStokesRep(*(itsImages->forwardGrid()),
    						  StokesImageUtil::CIRCULAR);
    	}
    	ft_p->initializeToVis(*(itsImages->forwardGrid()), vi::VisBuffer2Adapter(&vb));
      }
      else if(!cft_p.null()) {
    	if(ejdegrid_p != NULL){
    		clCorrupted_p=ComponentList();
    		for (uInt k=0; k < cl_p.nelements(); ++k){
    			SkyComponent comp=cl_p.component(k).copy();
    			if(vb.polarizationFrame()==MSIter::Linear) {
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
    			ejdegrid_p->apply(comp, comp, vi::VisBuffer2Adapter(&vb),row, True);
    			clCorrupted_p.add(comp);
    		}
    	}
  }


    
  }

  //////////////////OLD vi/vb version
  void SIMapper::initializeDegrid(const VisBuffer& vb, const Int row)
  {
	  LogIO os( LogOrigin("SIMapper", "initializeDegrid",WHERE) );
      if(ft_p.null() && cft_p.null())
      	return;

      if(!ft_p.null()) {
      	ft_p->stokesToCorrelation(*(itsImages->model()), *(itsImages->forwardGrid()));
      	if(ejdegrid_p != NULL)
      		ejgrid_p->apply(*(itsImages->forwardGrid()),*(itsImages->forwardGrid()), vb, row, True);
      	// Change the model polarization frame
      	if(vb.polFrame()==MSIter::Linear) {
      		StokesImageUtil::changeCStokesRep(*(itsImages->forwardGrid()),
      						  StokesImageUtil::LINEAR);
      	}
      	else {
      		StokesImageUtil::changeCStokesRep(*(itsImages->forwardGrid()),
      						  StokesImageUtil::CIRCULAR);
      	}
      	ft_p->initializeToVis(*(itsImages->forwardGrid()), vb);
        }
        else if(!cft_p.null()) {
      	if(ejdegrid_p != NULL){
      		clCorrupted_p=ComponentList();
      		for (uInt k=0; k < cl_p.nelements(); ++k){
      			SkyComponent comp=cl_p.component(k).copy();
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
    }



    }

  void SIMapper::degrid(vi::VisBuffer2& vb)
  {
    LogIO os( LogOrigin("SIMapper","degrid",WHERE) );
    ///This should not be called even but heck let's ignore
    if(ft_p.null() and cft_p.null())
    	return;
    Int nRow=vb.nRows();
    Cube<Complex> origCube;
    origCube.assign(vb.visCubeModel());
    vi::VisBuffer2Adapter vba(&vb);
    Bool internalChanges=False;  // Does this VB change inside itself?
    Bool firstOneChanges=False;  // Has this VB changed from the previous one?

    	if((!ft_p.null() && (ft_p->name() != "MosaicFT")    && (ft_p->name() != "PBWProjectFT") &&
    		(ft_p->name() != "AWProjectFT") && (ft_p->name() != "AWProjectWBFT")) || (!cft_p.null())) {
    		changedSkyJonesLogic(vb, firstOneChanges, internalChanges, False);
    	}
    	//anyways right now we are allowing only 1 ftmachine for GridBoth
    	Bool FTChanged=ft_p->changed(vba);

    	if(internalChanges)
    	{
    		// Yes there are changes within this buffer: go row by row.
    		// This will automatically catch a change in the FTMachine so
    		// we don't have to check for that.
    		for (Int row=0; row<nRow; row++)
    		{
    			if(FTChanged||ejdegrid_p->changed(vba,row))
    			{
    				// Need to apply the SkyJones from the previous row
    				// and finish off before starting with this row
    				finalizeDegrid();
    				initializeDegrid(vb, row);
    			}
    			ft_p.null() ? cft_p->get(vba, clCorrupted_p, row) : ft_p->get(vba, row);

    		}

    	}
    	else if (FTChanged||firstOneChanges) {
    		// This buffer has changed wrt the previous buffer, but
    		// this buffer has no changes within it. Again we don't need to
    		// check for the FTMachine changing.
    		finalizeDegrid();
    		initializeDegrid(vb, 0);
    		ft_p.null() ? cft_p->get(vba, clCorrupted_p) : ft_p->get(vba);
    	}
    	else {
    		ft_p.null() ? cft_p->get(vba, clCorrupted_p) : ft_p->get(vba);
    	}


     vba.modelVisCube()+=origCube;

  }


  ////////////////////Old vi/Vb version

  void SIMapper::degrid(VisBuffer& vb)
    {
      LogIO os( LogOrigin("SIMapper","degrid",WHERE) );
      ///This should not be called even but heck let's ignore
      if(ft_p.null() and cft_p.null())
      	return;
      Int nRow=vb.nRow();
      Cube<Complex> origCube;
      origCube.assign(vb.modelVisCube());
      Bool internalChanges=False;  // Does this VB change inside itself?
      Bool firstOneChanges=False;  // Has this VB changed from the previous one?

      	if((!ft_p.null() && (ft_p->name() != "MosaicFT")    && (ft_p->name() != "PBWProjectFT") &&
      		(ft_p->name() != "AWProjectFT") && (ft_p->name() != "AWProjectWBFT")) || (!cft_p.null())) {
      		changedSkyJonesLogic(vb, firstOneChanges, internalChanges, False);
      	}
      	//anyways right now we are allowing only 1 ftmachine for GridBoth
      	Bool FTChanged=ft_p->changed(vb);

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
      				finalizeDegrid();
      				initializeDegrid(vb, row);
      			}
      			ft_p.null() ? cft_p->get(vb, clCorrupted_p, row) : ft_p->get(vb, row);

      		}

      	}
      	else if (FTChanged||firstOneChanges) {
      		// This buffer has changed wrt the previous buffer, but
      		// this buffer has no changes within it. Again we don't need to
      		// check for the FTMachine changing.
      		finalizeDegrid();
      		initializeDegrid(vb, 0);
      		ft_p.null() ? cft_p->get(vb, clCorrupted_p) : ft_p->get(vb);
      	}
      	else {
      		ft_p.null() ? cft_p->get(vb, clCorrupted_p) : ft_p->get(vb);
      	}


       vb.modelVisCube()+=origCube;

    }


  void SIMapper::finalizeDegrid()
  {
    LogIO os( LogOrigin("SIMapper","finalizeDegrid",WHERE) );
  }

  Bool SIMapper::getFTMRecord(Record& rec)
  {
    LogIO os( LogOrigin("SIMapper","getFTMRecord",WHERE) );
    if(ft_p.null())
    	return False;
    String err;
    return ft_p->toRecord(err, rec, True);
    // rec = itsFTM->toRecord();

  }
  Bool SIMapper::getCLRecord(Record& rec)
  {
	  if(cft_p.null())
	      	return False;
	  String err;
	  return cl_p.toRecord(err, rec);

  }
  Bool SIMapper::changedSkyJonesLogic(const vi::VisBuffer2& vb, Bool& firstRow, Bool& internalRow, const Bool grid){
      firstRow=False;
      internalRow=False;
      SkyJones* ej= grid ? ejgrid_p : ejdegrid_p;
      if(ej ==NULL)
    	  return False;
	  if(ej->changed(vi::VisBuffer2Adapter(&vb),0))
		  firstRow=True;
	  Int row2temp=0;
	  if(ej->changedBuffer(vi::VisBuffer2Adapter(&vb),0,row2temp)) {
	     internalRow=True;
	   }
	   return (firstRow || internalRow) ;
  }
  ////////////Old VB version
  Bool SIMapper::changedSkyJonesLogic(const VisBuffer& vb, Bool& firstRow, Bool& internalRow, const Bool grid){
        firstRow=False;
        internalRow=False;
        SkyJones* ej= grid ? ejgrid_p : ejdegrid_p;
        if(ej ==NULL)
      	  return False;
  	  if(ej->changed(vb,0))
  		  firstRow=True;
  	  Int row2temp=0;
  	  if(ej->changedBuffer(vb,0,row2temp)) {
  	     internalRow=True;
  	   }
  	   return (firstRow || internalRow) ;
    }
} //# NAMESPACE CASA - END

