//# SIImageStore.cc: Implementation of Imager.h
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
#include <images/Images/PagedImage.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <synthesis/ImagerObjects/SIImageStore.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  
  SIImageStore::SIImageStore() 
  {
    itsPsf=NULL;
    itsModel=NULL;
    itsResidual=NULL;
    itsWeight=NULL;
    itsImage=NULL;
    itsMask=NULL;

    itsImageShape=IPosition();
    itsImageName=String("");
    itsCoordSys=CoordinateSystem();
    itsMiscInfo=Record();

    itsValidity = False;

  }

  SIImageStore::SIImageStore(String imagename, 
			     CoordinateSystem &imcoordsys, 
			     IPosition imshape, const Bool /*overwrite*/)
  // TODO : Add parameter to indicate weight image shape. 
  {
    LogIO os( LogOrigin("SIImageStore","Open new Images",WHERE) );

    itsPsf=NULL;
    itsModel=NULL;
    itsResidual=NULL;
    itsWeight=NULL;
    itsImage=NULL;
    itsMask=NULL;

    itsImageName = imagename;
    itsImageShape = imshape;
    itsCoordSys = imcoordsys;

    itsMiscInfo=Record();

  }

  SIImageStore::SIImageStore(String imagename) 
  {
    LogIO os( LogOrigin("SIImageStore","Open existing Images",WHERE) );

    itsPsf=NULL;
    itsModel=NULL;
    itsResidual=NULL;
    itsWeight=NULL;   
    itsImage=NULL;
    itsMask=NULL;
    itsMiscInfo=Record();

    itsImageName = imagename;

    // The PSF or Residual images must exist. ( TODO : and weight )
    if( doesImageExist(itsImageName+String(".residual")) || doesImageExist(itsImageName+String(".psf")) )
      {
	CountedPtr<ImageInterface<Float> > imptr;
	if( doesImageExist(itsImageName+String(".psf")) )
	  imptr = new PagedImage<Float> (itsImageName+String(".psf"));
	else
	  imptr = new PagedImage<Float> (itsImageName+String(".residual"));
	  
	itsImageShape = imptr->shape();
	itsCoordSys = imptr->coordinates();
      }
    else
      {
	throw( AipsError( "PSF or Residual Images do not exist. Please create one of them." ) );
      }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////Constructor with pointers already created else where but taken over here
  SIImageStore::SIImageStore(ImageInterface<Float>* modelim, 
			     ImageInterface<Float>* residim,
			     ImageInterface<Float>* psfim, 
			     ImageInterface<Float>* weightim, 
			     ImageInterface<Float>* restoredim,
			     ImageInterface<Float>* maskim
			     )
  {
    itsPsf=psfim;
    itsModel=modelim;
    itsResidual=residim;
    itsWeight=weightim;
    itsImage=restoredim;
    itsMask=maskim;

    itsImageShape=psfim->shape();
    itsCoordSys = psfim->coordinates();
    itsMiscInfo = psfim->miscInfo();

    itsImageName = String("reference");  // This is what the access functions use to guard against allocs...
	
    itsValidity=((!itsPsf.null()) &&   (!itsResidual.null()) && (!itsWeight.null()));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////make a facet image store which refers to a sub section of images 
  ///////////////////////in this storage
  CountedPtr<SIImageStore> SIImageStore::getFacetImageStore(const Int facet, const Int nfacets)
  {
    SubImage<Float>* facetPSF= itsPsf.null()?NULL:makeFacet(facet, nfacets, *itsPsf);
    SubImage<Float>* facetModel=itsModel.null()?NULL:makeFacet(facet, nfacets, *itsModel);
    SubImage<Float>* facetResidual=itsResidual.null()?NULL:makeFacet(facet, nfacets, *itsResidual);
    SubImage<Float>* facetWeight=itsWeight.null()?NULL:makeFacet(facet, nfacets, *itsWeight);
    SubImage<Float>* facetImage=itsImage.null()?NULL:makeFacet(facet, nfacets, *itsImage);
    SubImage<Float>* facetMask=itsMask.null()?NULL:makeFacet(facet, nfacets, *itsMask);
    return new SIImageStore(facetModel, facetResidual, facetPSF, facetWeight, facetImage, facetMask);

  }

  void SIImageStore::getNSubImageStores(const Bool onechan, const Bool onepol, uInt& nSubChans, uInt& nSubPols)
  {
    nSubChans = ( (onechan)?itsImageShape[3]:1 );
    nSubPols = ( (onepol)?itsImageShape[2]:1 );
  }



  CountedPtr<SIImageStore> SIImageStore::getSubImageStore(const Int chan, const Bool onechan,
							  const Int pol, const Bool onepol)
  {

    if( !onechan && !onepol ) {return this;}    // No slicing is required. 

    SubImage<Float>* subPSF=itsPsf.null()?NULL:makePlane( chan,onechan,pol,onepol, *itsPsf);
    SubImage<Float>* subModel=itsModel.null()?NULL:makePlane(  chan,onechan,pol,onepol, *itsModel);
    SubImage<Float>* subResidual=itsResidual.null()?NULL:makePlane(  chan,onechan,pol,onepol, *itsResidual);
    SubImage<Float>* subWeight=itsWeight.null()?NULL:makePlane(  chan,onechan,pol,onepol, *itsWeight);
    SubImage<Float>* subImage=itsImage.null()?NULL:makePlane(  chan,onechan,pol,onepol, *itsImage);
    SubImage<Float>* subMask=itsMask.null()?NULL:makePlane(  chan,onechan,pol,onepol, *itsMask);
    return new SIImageStore(subModel, subResidual, subPSF, subWeight, subImage, subMask);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //// Either read an image from disk, or construct one. 
  CountedPtr<ImageInterface<Float> > SIImageStore::openImage(const String imagenamefull, 
							     const Bool overwrite)
  {

    CountedPtr<ImageInterface<Float> > imPtr;

    if( overwrite || !Table::isWritable( imagenamefull ) )
      {
	imPtr=new PagedImage<Float> (itsImageShape, itsCoordSys, imagenamefull);
	// initialize to zeros...
	imPtr->set(0.0);

	// TODO : Add special case for itsWeightShape ?  A flag inputted to openImage to trigger this...

      }
    else
      {
	if(Table::isWritable( imagenamefull ))
	  {
	    //cerr << "Trying to open "<< imagenamefull << endl;
	    try{
	      imPtr=new PagedImage<Float>( imagenamefull );
	    }
	    catch (AipsError &x){
	      cerr << "Writable table exists, but cannot open. Overwriting." << endl;
	      imPtr=new PagedImage<Float> (itsImageShape, itsCoordSys, imagenamefull);
	    }
	  }
	else
	  {
	    cerr << "Table " << imagenamefull << " is not writeable. Creating temp image." << endl;
	    imPtr=new TempImage<Float> (itsImageShape, itsCoordSys);
	  }
      }
    
    return imPtr;
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void SIImageStore::setImageInfo(const Record miscinfo)
  {
    itsMiscInfo = miscinfo;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  // Check if images that are asked-for are ready and all have the same shape.
  Bool SIImageStore::checkValidity(const Bool ipsf, const Bool iresidual, const Bool iweight, 
				   const Bool imodel, const Bool irestored,const Bool imask, 
				   const Bool /*ialpha*/, const Bool /*ibeta*/)
  {

    Bool valid = True;

    try
      {

    if(ipsf==True)
      { psf(); valid = valid & ( !itsPsf.null() && itsPsf->shape()==itsImageShape ); }
    if(iresidual==True)
      { residual(); valid = valid & ( !itsResidual.null()&& itsResidual->shape()==itsImageShape ); }
    if(iweight==True)
      { weight(); valid = valid & ( !itsWeight.null()&& itsWeight->shape()==itsImageShape ); }
    if(imodel==True)
      { model(); valid = valid & ( !itsModel.null() && itsModel->shape()==itsImageShape); }
    if(irestored==True)
      { image(); valid = valid & ( !itsImage.null() && itsImage->shape()==itsImageShape); }
    if(imask==True)
      { mask(); valid = valid & ( !itsMask.null() && itsMask->shape()==itsImageShape); }

      }
    catch(AipsError &x)
      {
	throw(AipsError("Error in SIImageStore::checkValidity : " + x.getMesg()));
      }

    return valid;

    //    return ( (!itsPsf.null()) == psf && (!itsResidual.null()) == residual &&
    //	     (!itsWeight.null()) == weight  && (!itsModel.null()) == model && 
    //	     (!itsImage.null()) == restored );
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  SubImage<Float>* 
  SIImageStore::makeFacet(const Int facet, const Int nfacets, ImageInterface<Float>& image){
    //assuming n x n facets
    Int nx_facets=Int(sqrt(Double(nfacets)));
    LogIO os( LogOrigin("SynthesisImager","makeFacet") );
     // Make the output image
    Slicer imageSlicer;
    if((facet>(nfacets-1))||(facet<0)) {
      os << LogIO::SEVERE << "Illegal facet " << facet << LogIO::POST;
      return NULL;
    }
    IPosition imshp=image.shape();
    IPosition blc(imshp.nelements(), 0);
    IPosition trc=imshp-1;
    IPosition inc(imshp.nelements(), 1);
    Int facetx = facet % nx_facets; 
    Int facety = (facet - facetx) / nx_facets;
    Int sizex = imshp(0) / nx_facets;
    Int sizey = imshp(1) / nx_facets;
    blc(0) = facetx * sizex; 
    trc(0) = blc(0) + sizex - 1;
    blc(1) = facety * sizey; 
    trc(1) = blc(1) + sizey - 1;
    LCBox::verify(blc, trc, inc, imshp);
    Slicer imslice(blc, trc, inc, Slicer::endIsLast);
    // Now create the facet image
    SubImage<Float>*  facetImage = new SubImage<Float>(image, imslice, True);
    facetImage->setMiscInfo(image.miscInfo());
    facetImage->setUnits(image.units());
    return facetImage;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  SubImage<Float>* 
  SIImageStore::makePlane(const Int chan, const Bool onechan, 
			  const Int pol, const Bool onepol,
			  ImageInterface<Float>& image)
  {
    LogIO os( LogOrigin("SIImageStore","makePlane") );

    uInt nx = itsImageShape[0];
    uInt ny = itsImageShape[1];
    uInt npol = itsImageShape[2];
    uInt nchan = itsImageShape[3];

    //    uInt nSubImages = ( (onechan)?itsImageShape[3]:1 ) * ( (onepol)?itsImageShape[2]:1 ) ;
    uInt polstep = (onepol)?1:npol;
    uInt chanstep = (onechan)?1:nchan;
    
    IPosition substart(4,0,0,pol,chan);
    IPosition substop(4,nx-1,ny-1, pol+polstep-1, chan+chanstep-1);
    Slicer imslice(substart, substop, Slicer::endIsLast);

    // Now create the reference image
    SubImage<Float>*  sliceImage = new SubImage<Float>(image, imslice, True);
    sliceImage->setMiscInfo(image.miscInfo());
    sliceImage->setUnits(image.units());
    return sliceImage;
  }


  
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  SIImageStore::~SIImageStore() 
  {
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  Bool SIImageStore::releaseLocks() 
  {
    LogIO os( LogOrigin("SIImageStore","releaseLocks",WHERE) );

    if( ! itsPsf.null() ) itsPsf->unlock();
    if( ! itsModel.null() ) itsModel->unlock();
    if( ! itsResidual.null() ) itsResidual->unlock();
    if( ! itsImage.null() ) itsImage->unlock();
    if( ! itsWeight.null() ) itsWeight->unlock();
    if( ! itsMask.null() ) itsMask->unlock();

    return True; // do something more intelligent here.
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIImageStore::setModelImage( String modelname )
  {
    LogIO os( LogOrigin("SIImageStore","setModelImage",WHERE) );

    Directory immodel( modelname+String(".model") );
    if( !immodel.exists() ) 
      {
	os << "Starting model image does not exist. No initial prediction will be done" << LogIO::POST;
	return;
      }

    CountedPtr<PagedImage<Float> > newmodel = new PagedImage<Float>( modelname+String(".model") );
    // Check shapes, coordsys with those of other images.  If different, try to re-grid here.

    if( newmodel->shape() != model()->shape() )
      {
	// For now, throw an exception.
	throw( AipsError( "Input model image "+modelname+".model is not the same shape as that defined for output in "+ itsImageName + ".model" ) );
      }

    os << "Setting " << modelname << " as model " << LogIO::POST;
    // Then, add its contents to itsModel.
    //itsModel->put( itsModel->get() + model->get() );
    itsModel->put( newmodel->get() );
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  IPosition SIImageStore::getShape()
  {
    return itsImageShape;
  }

  String SIImageStore::getName()
  {
    return itsImageName;
  }

  void SIImageStore::checkRef( CountedPtr<ImageInterface<Float> > ptr, const String label )
  {
    if( ptr.null() && itsImageName==String("reference") ) 
      {throw(AipsError("Internal Error : Attempt to access null subImageStore "+label + " by reference."));}
  }

  CountedPtr<ImageInterface<Float> > SIImageStore::psf(uInt /*nterm*/)
  {
    if( !itsPsf.null() && itsPsf->shape() == itsImageShape ) { return itsPsf; }
    checkRef( itsPsf, "psf" );
    itsPsf = openImage( itsImageName+String(".psf") , False );
    return itsPsf;
  }
  CountedPtr<ImageInterface<Float> > SIImageStore::residual(uInt /*nterm*/)
  {
    if( !itsResidual.null() && itsResidual->shape() == itsImageShape ) { return itsResidual; }
    checkRef( itsResidual, "residual" );
    itsResidual = openImage( itsImageName+String(".residual") , False );
    return itsResidual;
  }
  CountedPtr<ImageInterface<Float> > SIImageStore::weight(uInt /*nterm*/)
  {
    if( !itsWeight.null() && itsWeight->shape() == itsImageShape ) { return itsWeight; }
    checkRef( itsWeight, "weight" );
    itsWeight = openImage( itsImageName+String(".weight") , False );

    /// TODO :  itsWeight's shape should reflect what type of weight image it is....

    return itsWeight;
  }
  CountedPtr<ImageInterface<Float> > SIImageStore::model(uInt /*nterm*/)
  {
    if( !itsModel.null() && itsModel->shape() == itsImageShape ) { return itsModel; }

    checkRef( itsModel, "model" );
    itsModel = openImage( itsImageName+String(".model") , False );

    // Set up header info the first time.
    ImageInfo info = itsModel->imageInfo();
    String objectName("");
    if( itsMiscInfo.isDefined("OBJECT") ){ itsMiscInfo.get("OBJECT", objectName); }
    info.setObjectName(objectName);
    itsModel->setImageInfo( info );
    itsModel->setMiscInfo( itsMiscInfo );
    itsModel->setUnits("Jy/pixel");

    return itsModel;
  }
  CountedPtr<ImageInterface<Float> > SIImageStore::image(uInt /*nterm*/)
  {
    if( !itsImage.null() && itsImage->shape() == itsImageShape ) { return itsImage; }
    checkRef( itsImage, "image" );
    
    itsImage = openImage( itsImageName+String(".image") , False );
    itsImage->setUnits("Jy/beam");
    return itsImage;
  }
  CountedPtr<ImageInterface<Float> > SIImageStore::mask(uInt /*nterm*/)
  {
    if( !itsMask.null() && itsMask->shape() == itsImageShape ) { return itsMask; }
    checkRef( itsMask, "mask" );
    itsMask = openImage( itsImageName+String(".mask") , False );
    //    cout << itsImageName << " has mask of shape : " << itsMask->shape() << endl;
    return itsMask;
  }
  CountedPtr<ImageInterface<Complex> > SIImageStore::forwardGrid(uInt /*nterm*/){
	  if(!itsForwardGrid.null() && (itsForwardGrid->shape() == itsImageShape))
		  return itsForwardGrid;
	  itsForwardGrid=new TempImage<Complex>(TiledShape(itsImageShape, tileShape()), itsCoordSys, memoryBeforeLattice());
	  return itsForwardGrid;
  }
  CountedPtr<ImageInterface<Complex> > SIImageStore::backwardGrid(uInt /*nterm*/){
  	  if(!itsBackwardGrid.null() && (itsBackwardGrid->shape() == itsImageShape))
  		  return itsBackwardGrid;
  	  itsBackwardGrid=new TempImage<Complex>(TiledShape(itsImageShape, tileShape()), itsCoordSys, memoryBeforeLattice());
  	  return itsBackwardGrid;
    }
  Double SIImageStore::memoryBeforeLattice(){
	  //Calculate how much memory to use per temporary images before disking
	  return 1000.0;
  }
  IPosition SIImageStore::tileShape(){
	  //Need to have settable stuff here or algorith to determine this
	  return IPosition(4, min(itsImageShape[0],1000), min(itsImageShape[1],1000), 1, 1);
  }

  /*
  // TODO : Move to an image-wrapper class ? Same function exists in SynthesisDeconvolver.
  Bool SIImageStore::doImagesExist()
  {
    LogIO os( LogOrigin("SIImageStore","doImagesExist",WHERE) );
    // Check if imagename.residual, imagename.psf. imagename.weight
    // exist on disk and if they're the right shape.
    // If the shape is not right, complain here and throw an exception (or just say it will get overwritten)

    Directory impsf( itsImageName+String(".psf") );
    Directory imresidual( itsImageName+String(".residual") );
    Directory imweight( itsImageName+String(".weight") );
    Directory immodel( itsImageName+String(".model") );

    return impsf.exists() & imresidual.exists() & imweight.exists() & immodel.exists();
  }
  */

  // TODO : Move to an image-wrapper class ? Same function exists in SynthesisDeconvolver.
  Bool SIImageStore::doesImageExist(String imagename)
  {
    LogIO os( LogOrigin("SIImageStore","doesImageExist",WHERE) );
    Directory image( imagename );
    return image.exists();
  }

  /*
   void SIImageStore::allocateRestoredImage()
  {
    if( itsImage.null() )
      {
	itsImage = new PagedImage<Float> (itsImageShape, itsResidual->coordinates(), itsImageName+String(".image"));
      }
    else
      {
	AlwaysAssert( itsImage->shape() == itsModel->shape() , AipsError );
      }
  }
  */


  void SIImageStore::resetImages( Bool resetpsf, Bool resetresidual, Bool resetweight )
  {
    if( resetpsf ) psf()->set(0.0);
    if( resetresidual ) residual()->set(0.0);
    if( resetweight && !itsWeight.null() ) weight()->set(0.0);
  }

  void SIImageStore::addImages( CountedPtr<SIImageStore> imagestoadd,
				Bool addpsf, Bool addresidual, Bool addweight)
  {

    if(addpsf)
      {
	LatticeExpr<Float> adderPsf( *(psf()) + *(imagestoadd->psf()) ); 
	psf()->copyData(adderPsf);

	addSumWts( *psf(), *(imagestoadd->psf()) );
	
      }
    if(addresidual)
      {
	LatticeExpr<Float> adderRes( *(residual()) + *(imagestoadd->residual()) ); 
	residual()->copyData(adderRes);

	addSumWts( *residual(), *(imagestoadd->residual()) );
	
      }
    if(addweight)
      {
	if( getUseWeightImage( *(imagestoadd->psf()) ) ) // Access and add weight only if it is needed.
	  {
	    LatticeExpr<Float> adderWeight( *(weight()) + *(imagestoadd->weight()) ); 
	    weight()->copyData(adderWeight);
	    
	    addSumWts( *weight(), *(imagestoadd->weight()) );
	
	  }
      }
    ///cout << "Res : " << itsResidual->getAt( IPosition(4,0,0,0,0) ) << "  Wt : " << itsWeight->getAt( IPosition(4,0,0,0,0) ) << endl;
  }

  void SIImageStore::addSumWts(ImageInterface<Float>& target, ImageInterface<Float>& toadd)
  {
    Matrix<Float> addsumwt = getSumWt( target ) + getSumWt( toadd ) ;
    setSumWt( target , addsumwt );
    Bool useweightimage = getUseWeightImage( toadd );
    setUseWeightImage( target, useweightimage );  // last one will override. Ok since all must be same.

    cout << "in addSumWts, useweightimage : " << getUseWeightImage( target ) << endl;
  }


  // Make another for the PSF too.
  void SIImageStore::divideResidualByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStore","divideResidualByWeight",WHERE) );

    // Normalize by the sumwt, per plane. 
    Bool didNorm = divideImageByWeightVal( *residual() );
    Bool useweightimage = getUseWeightImage( *residual() );

    //    cout << "SIIM div residual by weight : useweightimage : " << useweightimage << endl;

    if( useweightimage == True )
      {
	divideImageByWeightVal( *weight() ); // no-op if already normalized, with sumwt set to 1.0
	
	os << "Dividing " << itsImageName+String(".residual") << " by the weight image " << itsImageName+String(".weight") << LogIO::POST;
	
	LatticeExpr<Float> mask( iif( (*(weight())) > weightlimit , 1.0, 0.0 ) );
	LatticeExpr<Float> maskinv( iif( (*(weight())) > weightlimit , 0.0, 1.0 ) );
	
	LatticeExpr<Float> ratio( ( (*(residual())) * mask ) / sqrt( (*(weight())) + maskinv ) );
	residual()->copyData(ratio);
      }
    
    // If no normalization happened, print a warning. The user must check if it's right or not.
    // Or... later if we get a gridder that does pre-norms, this warning can go. 
    if( (didNorm | useweightimage) != True ) 
      os << LogIO::WARN << "No normalization done to residual" << LogIO::POST;

    // createMask
  }
  
  void SIImageStore::dividePSFByWeight()
  {
    LogIO os( LogOrigin("SIImageStore","dividePSFByWeight",WHERE) );

    // Normalize by the sumwt, per plane. 
    divideImageByWeightVal( *psf() );

    divideImageByWeightVal( *weight() ); // no-op if already normalized, with sumwt set to 1.0

    /*    
    if( getUseWeightImage( *psf() ) == True )
      {
	    os << "Dividing " << itsImageName+String(".psf") << " by the weight image " << itsImageName+String(".weight") << LogIO::POST;
	    
	    LatticeExpr<Float> mask( iif( (*(weight())) > weightlimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (*(weight())) > weightlimit , 0.0, 1.0 ) );
	    
	    LatticeExpr<Float> ratio( ( (*(psf())) * mask ) / sqrt( (*(weight())) + maskinv ) );
	    psf()->copyData(ratio);
      }
    */
    // createMask
  }

  /*
  void SIImageStore::divideSensitivityPatternByWeight()
  {
    LogIO os( LogOrigin("SIImageStore","dividePSFByWeight",WHERE) );

    if( getUseWeightImage( *psf() ) == True )// i.e. only when needed.
      {
	// Normalize by the sumwt, per plane. 
	divideImageByWeightVal( *weight() );
      }

  }
  */

  void SIImageStore::divideModelByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStore","divideModelByWeight",WHERE) );

    if( !itsResidual.null() // information exists on whether weight image is needed or not
	&& getUseWeightImage( *residual() ) == True // only when needed
	&& hasSensitivity() )// i.e. only when possible. For an initial starting model, don't need wt anyway.
      {
        os << "Dividing " << itsImageName+String(".model") << " by the weight image " << itsImageName+String(".weight") << LogIO::POST;
	
	LatticeExpr<Float> mask( iif( (*(weight())) > weightlimit , 1.0, 0.0 ) );
	LatticeExpr<Float> maskinv( iif( (*(weight())) > weightlimit , 0.0, 1.0 ) );
	
	LatticeExpr<Float> ratio( ( (*(model())) * mask ) / sqrt( (*(weight())) + maskinv ) );
	model()->copyData(ratio);
      }
    // createMask
  }
  
  void SIImageStore::multiplyModelByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStore","multiplyModelByWeight",WHERE) );

    //cout << "In multiplymodelbyweight : model, usewt, weight " << itsModel.null() << " " << getUseWeightImage( *residual() ) << " " << doesWeightExist() << endl;

    if( //!itsModel.null() // anything to do ? 
	//&& 
	getUseWeightImage( *residual() ) == True // only when needed
	&& hasSensitivity() )// i.e. only when possible. For an initial starting model, don't need wt anyway.
      {
        os << "Multiplying " << itsImageName+String(".model") << " by the weight image " << itsImageName+String(".weight") << LogIO::POST;
	
	LatticeExpr<Float> mask( iif( (*(weight())) > weightlimit , 1.0, 0.0 ) );
	LatticeExpr<Float> maskinv( iif( (*(weight())) > weightlimit , 0.0, 1.0 ) );
	
	LatticeExpr<Float> ratio( ( (*(model())) * mask ) * sqrt( (*(weight())) + maskinv ) );
	model()->copyData(ratio);
      }
    // createMask
  }
  
  GaussianBeam SIImageStore::getPSFGaussian()
  {

    GaussianBeam beam;
    try
      {
	if( psf()->ndim() > 0 )
	  {
	    StokesImageUtil::FitGaussianPSF( *(psf()), beam );
	  }
      }
    catch(AipsError &x)
      {
	LogIO os( LogOrigin("SIImageStore","getPSFGaussian",WHERE) );
	os << "Error in fitting a Gaussian to the PSF : " << x.getMesg() << LogIO::POST;
	throw( AipsError("Error in fitting a Gaussian to the PSF" + x.getMesg()) );
      }

    return beam;
  }

  void SIImageStore::restorePlane()
  {

    LogIO os( LogOrigin("SIImageStore","restorePlane",WHERE) );
    //     << ". Optionally, PB-correct too." << LogIO::POST;

    try
      {
	// Fit a Gaussian to the PSF.
	GaussianBeam beam = getPSFGaussian();

	os << "Restore with beam : " << beam.getMajor(Unit("arcmin")) << " arcmin, " << beam.getMinor(Unit("arcmin"))<< " arcmin, " << beam.getPA(Unit("deg")) << " deg" << LogIO::POST; 
	
	// Initialize restored image
	image()->set(0.0);
	// Copy model into it
	image()->copyData( LatticeExpr<Float>( *(model()) )  );
	// Smooth model by beam
	StokesImageUtil::Convolve( *(image()), beam);
	// Add residual image
	image()->copyData( LatticeExpr<Float>( *(image()) + *(residual())  ) );
	
	// Set restoring beam into the image
	ImageInfo ii = image()->imageInfo();
	ii.setRestoringBeam(beam);
	image()->setImageInfo(ii);

      }
    catch(AipsError &x)
      {
	throw( AipsError("Restoration Error : " + x.getMesg() ) );
      }

  }


  Matrix<Float> SIImageStore::getSumWt(ImageInterface<Float>& target)
  {
    Record miscinfo = target.miscInfo();
    
    Matrix<Float> sumwt;
    sumwt.resize();
    if( miscinfo.isDefined("sumwt") 
	&& (miscinfo.dataType("sumwt")==TpArrayFloat || miscinfo.dataType("sumwt")==TpArrayDouble  )  ) 
      { miscinfo.get( "sumwt" , sumwt ); } 
    else   { sumwt.resize( IPosition(2, target.shape()[2], target.shape()[3] ) ); sumwt = 1.0;  }
    
    return sumwt;
  }
  
  void SIImageStore::setSumWt(ImageInterface<Float>& target, Matrix<Float>& sumwt)
  {
    Record miscinfo = target.miscInfo();
    miscinfo.define("sumwt", sumwt);
    target.setMiscInfo( miscinfo );
  }

  Bool SIImageStore::getUseWeightImage(ImageInterface<Float>& target)
  {
    Record miscinfo = target.miscInfo();
    Bool useweightimage;
    if( miscinfo.isDefined("useweightimage") && miscinfo.dataType("useweightimage")==TpBool )
      { miscinfo.get( "useweightimage", useweightimage );  }
    else { useweightimage = False; }

    return useweightimage;
  }

  void SIImageStore::setUseWeightImage(ImageInterface<Float>& target, Bool useweightimage)
  {
    Record miscinfo = target.miscInfo();
    miscinfo.define("useweightimage", useweightimage);
    target.setMiscInfo( miscinfo );
  }
  


  Bool SIImageStore::divideImageByWeightVal( ImageInterface<Float>& target )
  {

    Matrix<Float> sumwt = getSumWt( target );

    IPosition imshape = target.shape();

    //    cout << " SumWt  : " << sumwt << " image shape : " << imshape << endl;

    AlwaysAssert( sumwt.shape()[0] == imshape[2] , AipsError ); // polplanes
    AlwaysAssert( sumwt.shape()[1] == imshape[3] , AipsError ); // chanplanes

    Bool div=False; // flag to signal if division actually happened, or weights are all 1.0

    for(Int pol=0; pol<sumwt.shape()[0]; pol++)
      {
	for(Int chan=0; chan<sumwt.shape()[1]; chan++)
	  {
	    if( sumwt(pol,chan) != 1.0 )
	      { 
		SubImage<Float>* subim=makePlane(  chan, True ,pol, True, target );
		LatticeExpr<Float> le( (*subim)/sumwt(pol,chan) );
		subim->copyData( le );
		div=True;
	      }
	  }
      }

    //    if( div==True ) cout << "Div image by sumwt : " << sumwt << endl;
    //    else cout << "Already normalized" << endl;

    sumwt = 1.0; setSumWt( target , sumwt );

    return div;
  }


  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END

