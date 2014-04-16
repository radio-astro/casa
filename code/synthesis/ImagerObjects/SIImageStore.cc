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
#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

  //
  //===========================================================================
  // Global method that I (SB) could make work in SynthesisUtilsMethods.
  //
  template <class T>
  void openImage(const String& imagenamefull,CountedPtr<ImageInterface<T> >& imPtr )
  {
    LogIO logIO ( LogOrigin("SynthesisImager","openImage(name)") );
    try
      {
	if (Table::isReadable(imagenamefull))
	  imPtr=new PagedImage<T>( imagenamefull );
      }
    catch (AipsError &x)
      {
	logIO << "Error in reading image \"" << imagenamefull << "\"" << LogIO::EXCEPTION;
      }
  }
  //
  //--------------------------------------------------------------
  //
  template 
  void openImage(const String& imagenamefull, CountedPtr<ImageInterface<Float> >& img );
  template 
  void openImage(const String& imagenamefull, CountedPtr<ImageInterface<Complex> >& img );
  //
  //===========================================================================
  //

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

    itsSumWt=NULL;
    itsNFacets=1;
    itsUseWeight=False;
    itsPBScaleFactor=1.0;

    itsImageShape=IPosition();
    itsImageName=String("");
    itsCoordSys=CoordinateSystem();
    itsMiscInfo=Record();
    init();
    //    itsValidity = False;

  }

  SIImageStore::SIImageStore(String imagename, 
			     CoordinateSystem &imcoordsys, 
			     IPosition imshape, 
			     const Int nfacets, 
			     const Bool /*overwrite*/,
			     const Bool useweightimage)
  // TODO : Add parameter to indicate weight image shape. 
  {
    LogIO os( LogOrigin("SIImageStore","Open new Images",WHERE) );

    itsPsf=NULL;
    itsModel=NULL;
    itsResidual=NULL;
    itsWeight=NULL;
    itsImage=NULL;
    itsMask=NULL;

    itsSumWt=NULL;
    itsNFacets=nfacets;
    itsUseWeight=useweightimage;
    itsPBScaleFactor=1.0;

    itsImageName = imagename;
    itsImageShape = imshape;
    itsCoordSys = imcoordsys;

    itsMiscInfo=Record();

    init();
  }

  SIImageStore::SIImageStore(String imagename) 
  {
    LogIO os( LogOrigin("SIImageStore","Open existing Images",WHERE) );

    /*
    init();
    String fname( imagename + ".info" );
    recreate( fname );
    */

   
    itsPsf=NULL;
    itsModel=NULL;
    itsResidual=NULL;
    itsWeight=NULL;   
    itsImage=NULL;
    itsMask=NULL;
    itsMiscInfo=Record();

    itsSumWt=NULL;
    itsNFacets=1;

    itsImageName = imagename;

    // The PSF or Residual images must exist. ( TODO : and weight )
    if( doesImageExist(itsImageName+String(".residual")) || 
	doesImageExist(itsImageName+String(".psf"))  )
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
	throw( AipsError( "PSF or Residual Image (or sumwt) do not exist. Please create one of them." ) );
      }
    
    if( doesImageExist(itsImageName+String(".sumwt"))  )
      {
	CountedPtr<ImageInterface<Float> > imptr;
	imptr = new PagedImage<Float> (itsImageName+String(".sumwt"));
	itsNFacets = imptr->shape()[0];
	itsUseWeight = getUseWeightImage( *imptr );
	itsPBScaleFactor=1.0; ///// No need to set properly here as it will be calc'd in dividePSF...()
	if( itsUseWeight && ! doesImageExist(itsImageName+String(".weight")) )
	  {
	    throw(AipsError("Internal error : Sumwt has a useweightimage=True but the weight image does not exist."));
	  }
      }
    else
      {
	throw( AipsError( "SumWt information does not exist. Please create either a PSF or Residual" ) );
      }
    init();
    
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////Constructor with pointers already created else where but taken over here
  SIImageStore::SIImageStore(ImageInterface<Float>* modelim, 
			     ImageInterface<Float>* residim,
			     ImageInterface<Float>* psfim, 
			     ImageInterface<Float>* weightim, 
			     ImageInterface<Float>* restoredim,
			     ImageInterface<Float>* maskim,
			     ImageInterface<Float>* sumwtim  )
  {
    itsPsf=psfim;
    itsModel=modelim;
    itsResidual=residim;
    itsWeight=weightim;
    itsImage=restoredim;
    itsMask=maskim;

    AlwaysAssert( sumwtim != NULL , AipsError );
    AlwaysAssert( psfim != NULL , AipsError );

    itsSumWt=sumwtim;
    itsNFacets=sumwtim->shape()[0];
    itsUseWeight=getUseWeightImage( *sumwtim );
    itsPBScaleFactor=1.0;// No need to set properly here as it will be computed in makePSF.

    itsImageShape=psfim->shape();
    itsCoordSys = psfim->coordinates();
    itsMiscInfo = psfim->miscInfo();

    itsImageName = String("reference");  // This is what the access functions use to guard against allocs...

    //    cout << " In ref imstore, hasSensitivity : " << hasSensitivity() << endl;
	
    //    itsValidity=((!itsPsf.null()) &&   (!itsResidual.null()) && (!itsWeight.null()));
    init();
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
    SubImage<Float>* facetSumWt=itsSumWt.null()?NULL:makeFacet(facet, nfacets, *itsSumWt);
    return new SIImageStore(facetModel, facetResidual, facetPSF, facetWeight, facetImage, facetMask,facetSumWt);

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
    SubImage<Float>* subSumWt=itsSumWt.null()?NULL:makePlane(  chan,onechan,pol,onepol, *itsSumWt);
    return new SIImageStore(subModel, subResidual, subPSF, subWeight, subImage, subMask, subSumWt);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //// Either read an image from disk, or construct one. 

  CountedPtr<ImageInterface<Float> > SIImageStore::openImage(const String imagenamefull, 
							     const Bool overwrite, const Bool dosumwt)
  {

    CountedPtr<ImageInterface<Float> > imPtr;

    IPosition useShape( itsImageShape );

    if( dosumwt ) // change shape to sumwt image shape.
      {
	useShape[0] = itsNFacets;
	useShape[1] = itsNFacets;
      }
    
    if( overwrite || !Table::isWritable( imagenamefull ) )
      {
	imPtr=new PagedImage<Float> (useShape, itsCoordSys, imagenamefull);
	// initialize to zeros...
	imPtr->set(0.0);

	// TODO : Add special case for itsWeightShape ?  A flag inputted to openImage to trigger this...

	//	cout << "made  " << imagenamefull << " of shape : " << useShape << endl;

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
	      imPtr=new PagedImage<Float> (useShape, itsCoordSys, imagenamefull);
	    }
	  }
	else
	  {
	    cerr << "Table " << imagenamefull << " is not writeable. Creating temp image." << endl;
	    imPtr=new TempImage<Float> (useShape, itsCoordSys);
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
				   const Bool isumwt, const Bool /*ialpha*/, const Bool /*ibeta*/)
  {

    Bool valid = True;

    try
      {

	if(isumwt==True) { 
	    IPosition useShape(itsImageShape);
	    useShape[0]=itsNFacets; useShape[1]=itsNFacets;
	    sumwt(); valid = valid & ( !itsSumWt.null()&& itsSumWt->shape()==useShape ); 
	  }
	// add a check for if sumwt has useweightimage=true, then weight() needs to exist too.....
	
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

    IPosition imshape = image.shape();
    uInt nx = imshape[0];
    uInt ny = imshape[1];
    uInt npol = imshape[2];
    uInt nchan = imshape[3];

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

    //    String fname( itsImageName+String(".info") );
    //    makePersistent( fname );

    if( ! itsPsf.null() ) itsPsf->unlock();
    if( ! itsModel.null() ) itsModel->unlock();
    if( ! itsResidual.null() ) itsResidual->unlock();
    if( ! itsImage.null() ) itsImage->unlock();
    if( ! itsWeight.null() ) itsWeight->unlock();
    if( ! itsMask.null() ) itsMask->unlock();
    if( ! itsSumWt.null() ) itsSumWt->unlock();

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

  uInt SIImageStore::getNTaylorTerms(Bool /*dopsf*/)
  {
    return 1;
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
    return itsWeight;
  }

  CountedPtr<ImageInterface<Float> > SIImageStore::sumwt(uInt /*nterm*/)
  {

    IPosition useShape( itsImageShape );
    useShape[0] = itsNFacets;
    useShape[1] = itsNFacets;

    if( !itsSumWt.null() && itsSumWt->shape() == useShape ) { return itsSumWt; }
    checkRef( itsSumWt, "sumwt" );
    itsSumWt = openImage( itsImageName+String(".sumwt") , False, True/*dosumwt*/ ); 

    setUseWeightImage( *itsSumWt , itsUseWeight); // Sets a flag in the SumWt image. 
    // if( itsUseWeight ){ 
      //      weight(); // Since it needs the weight image, make it. 
    //} 

    return itsSumWt;
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
    if( resetweight ) sumwt()->set(0.0);
  }

  void SIImageStore::addImages( CountedPtr<SIImageStore> imagestoadd,
				Bool addpsf, Bool addresidual, Bool addweight)
  {

    if(addpsf)
      {
	LatticeExpr<Float> adderPsf( *(psf()) + *(imagestoadd->psf()) ); 
	psf()->copyData(adderPsf);

	//	addSumWts( *psf(), *(imagestoadd->psf()) );
	//	setUseWeightImage( *psf(),  getUseWeightImage(*(imagestoadd->psf()) ) );
	
      }
    if(addresidual)
      {
	LatticeExpr<Float> adderRes( *(residual()) + *(imagestoadd->residual()) ); 
	residual()->copyData(adderRes);

	//	addSumWts( *residual(), *(imagestoadd->residual()) );
	//	setUseWeightImage( *residual(),  getUseWeightImage(*(imagestoadd->residual()) ) );
	
      }
    if(addweight)
      {
	if( itsUseWeight ) // getUseWeightImage( *(imagestoadd->psf()) ) ) // Access and add weight only if it is needed.
	  {
	    LatticeExpr<Float> adderWeight( *(weight()) + *(imagestoadd->weight()) ); 
	    weight()->copyData(adderWeight);
	    
	    //addSumWts( *weight(), *(imagestoadd->weight()) );
	    //	    setUseWeightImage( *weight(),  getUseWeightImage(*(imagestoadd->weight()) ) );
	  }

	LatticeExpr<Float> adderSumWt( *(sumwt()) + *(imagestoadd->sumwt()) ); 
	sumwt()->copyData(adderSumWt);
	setUseWeightImage( *sumwt(),  getUseWeightImage(*(imagestoadd->sumwt()) ) );

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


  Double SIImageStore::getPbMax()
  {
    //LatticeExprNode pbmax( max ( sqrt( *weight() ) ) );
    //return pbmax.getDouble();

    Array<Float> pbmat;
    ( weight(0) )->get( pbmat, True );
    //    Float minval, maxval;
    //    IPosition posmin,posmax;
    //    minMax(minval, maxval, posmin, posmax);
    //    return maxval;

    return max(sqrt(fabs(pbmat)));

  }


  void SIImageStore::dividePSFByWeight()
  {
    LogIO os( LogOrigin("SIImageStore","dividePSFByWeight",WHERE) );

    // Normalize by the sumwt, per plane. 
    divideImageByWeightVal( *psf() );
    if( itsUseWeight ) 
      { 
	divideImageByWeightVal( *weight() ); 

	// Get the scale factor that will make the peak PB gain 1.
	// This will be used to divide the residual image, and multiply the model image.
	// It will let the minor cycle see as close to the 'principal solution' as possible.
	itsPBScaleFactor = getPbMax();
	cout << " PB Scale factor : " << itsPBScaleFactor << endl;
      }

    /// Calculate and print point source sensitivity (per plane).
    calcSensitivity();
    // createMask
  }

  // Make another for the PSF too.
  void SIImageStore::divideResidualByWeight(Float pblimit,String normtype)
  {
    LogIO os( LogOrigin("SIImageStore","divideResidualByWeight",WHERE) );

    // Normalize by the sumwt, per plane. 
    Bool didNorm = divideImageByWeightVal( *residual() );
    Bool useweightimage = itsUseWeight; //getUseWeightImage( *residual() );

    //    cout << "SIIM div residual by weight : useweightimage : " << useweightimage << endl;

    if( useweightimage == True )
      {
	//	divideImageByWeightVal( *weight() ); // Assume already normalized by PSF making.

	itsPBScaleFactor = getPbMax();
	//	cout << " pbscale : " << itsPBScaleFactor << endl;

	LatticeExpr<Float> deno;
	if( normtype=="flatnoise"){
	  deno = LatticeExpr<Float> ( sqrt( abs(*(weight())) ) * itsPBScaleFactor );
	  os << "Dividing " << itsImageName+String(".residual") ;
	  os << " by [ sqrt(weightimage) * " << itsPBScaleFactor ;
	  os << " ] to get flat noise with unit pb peak."<< LogIO::POST;
	  
	  }
	if( normtype=="flatsky") {
	  deno = LatticeExpr<Float> ( *(weight()) );
	  os << "Dividing " << itsImageName+String(".residual") ;
	  os << " by [ weight ] to get flat sky"<< LogIO::POST;
	}
	
	LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
	LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
	LatticeExpr<Float> ratio( ( (*(residual())) * mask ) / ( deno + maskinv ) );
	
	////LatticeExpr<Float> mask( iif( (*(weight())) > pblimit , 1.0, 0.0 ) );
	////LatticeExpr<Float> maskinv( iif( (*(weight())) > pblimit , 0.0, 1.0 ) );
	////	LatticeExpr<Float> ratio( ( (*(residual())) * mask ) / sqrt( (*(weight())) + maskinv ) );
	residual()->copyData(ratio);
      }
    
    // If no normalization happened, print a warning. The user must check if it's right or not.
    // Or... later if we get a gridder that does pre-norms, this warning can go. 
    if( (didNorm | useweightimage) != True ) 
      os << LogIO::WARN << "No normalization done to residual" << LogIO::POST;

    // createMask
  }
  

  void SIImageStore::divideModelByWeight(Float pblimit, const String normtype)
  {
    LogIO os( LogOrigin("SIImageStore","divideModelByWeight",WHERE) );

    if( ///!itsModel.null() 
	//	&& getUseWeightImage( *residual() ) == True // only when needed
	//&& 
	itsUseWeight // only when needed
	&& hasSensitivity() )// i.e. only when possible. For an initial starting model, don't need wt anyway.
      {

	if( normtype=="flatsky") {
	  Array<Float> arrmod;
	  model()->get( arrmod, True );

	  os << "Model is already flat sky with peak flux : " << max(arrmod);
	  os << ". No need to divide before prediction" << LogIO::POST;
	  
	  return;
	  }
	else if( normtype=="flatnoise"){

	  itsPBScaleFactor = getPbMax();
	  //	  cout << " pbscale : " << itsPBScaleFactor << endl;

	  os << "Dividing " << itsImageName+String(".model") ;
	  os << " by [ sqrt(weight) / " << itsPBScaleFactor ;
	  os <<" ] to get to flat sky model before prediction" << LogIO::POST;
	  
	  LatticeExpr<Float> deno( sqrt( abs(*(weight())) ) / itsPBScaleFactor );

	  LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
	  LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
	  LatticeExpr<Float> ratio( ( (*(model())) * mask ) / ( deno + maskinv ) );
	  
	  //	LatticeExpr<Float> mask( iif( (*(weight())) > pblimit , 1.0, 0.0 ) );
	  //	LatticeExpr<Float> maskinv( iif( (*(weight())) > pblimit , 0.0, 1.0 ) );
	  //	LatticeExpr<Float> ratio( ( (*(model())) * mask ) / sqrt( (*(weight())) + maskinv ) );
	  model()->copyData(ratio);
	}
	
      }
    // createMask
  }
  
  void SIImageStore::multiplyModelByWeight(Float pblimit, const String normtype)
  {
    LogIO os( LogOrigin("SIImageStore","multiplyModelByWeight",WHERE) );

    if( //!itsModel.null() // anything to do ? 
       //	getUseWeightImage( *residual() ) == True // only when needed
	//&& 
	itsUseWeight // only when needed
	&& hasSensitivity() )// i.e. only when possible. For an initial starting model, don't need wt anyway.
      {
	if( normtype=="flatsky") {
	  os << "Model is already flat sky. No need to multiply back after prediction" << LogIO::POST;
	  return;
	  }
	else if( normtype=="flatnoise"){

	  itsPBScaleFactor = getPbMax();
	  cout << " pbscale : " << itsPBScaleFactor << endl;

	  os << "Multiplying " << itsImageName+String(".model") ;
	  os << " by [ sqrt(weight) / " << itsPBScaleFactor;
	  os <<  " ] to take model back to flat noise with unit pb peak." << LogIO::POST;
	  
	  LatticeExpr<Float> deno( sqrt( abs(*(weight())) ) / itsPBScaleFactor );

	  LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
	  LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
	  LatticeExpr<Float> ratio( ( (*(model())) * mask ) * ( deno + maskinv ) );
	  
	  //	LatticeExpr<Float> mask( iif( (*(weight())) > pblimit , 1.0, 0.0 ) );
	  //	LatticeExpr<Float> maskinv( iif( (*(weight())) > pblimit , 0.0, 1.0 ) );
	  //	LatticeExpr<Float> ratio( ( (*(model())) * mask ) / sqrt( (*(weight())) + maskinv ) );
	  model()->copyData(ratio);
	}

	/*
        os << "Multiplying " << itsImageName+String(".model") << " by the weight image " << itsImageName+String(".weight") << LogIO::POST;

	if( normtype=="flatnoise"){
	    LatticeExpr<Float> deno( sqrt( *(weight()) ) / itsPBScaleFactor );
	  }
	if( normtype=="flatsky") {
	    LatticeExpr<Float> deno( 1.0 );
	  }

	LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
	LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
	LatticeExpr<Float> ratio( ( (*(model())) * mask ) * ( deno + maskinv ) );

	
	//	LatticeExpr<Float> mask( iif( (*(weight())) > pblimit , 1.0, 0.0 ) );
	//	LatticeExpr<Float> maskinv( iif( (*(weight())) > pblimit , 0.0, 1.0 ) );
	//	LatticeExpr<Float> ratio( ( (*(model())) * mask ) * sqrt( (*(weight())) + maskinv ) );
	model()->copyData(ratio);
	*/
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
	//	LogIO os( LogOrigin("SIImageStore","getPSFGaussian",WHERE) );
	//	os << "Error in fitting a Gaussian to the PSF : " << x.getMesg() << LogIO::POST;
	throw( AipsError("Error in fitting a Gaussian to the PSF : " + x.getMesg()) );
      }

    return beam;
  }

  GaussianBeam SIImageStore::restorePlane()
  {

    LogIO os( LogOrigin("SIImageStore","restorePlane",WHERE) );
    //     << ". Optionally, PB-correct too." << LogIO::POST;

    Bool validbeam=False;
    GaussianBeam beam;
    try
      {
	// Fit a Gaussian to the PSF.
	beam = getPSFGaussian();
	validbeam = True;
      }
    catch(AipsError &x)
      {
	os << LogIO::WARN << "Beam fit error : " + x.getMesg() << LogIO::POST;
      }
    
    try
      {
	if( validbeam==True )
	  {
	    //	    os << "[" << itsImageName << "] " ;  // Add when parent image name is available.
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
	    //ii.setRestoringBeam(beam);
	    ii.setBeams(beam);
	    image()->setImageInfo(ii);
	  }
      }
    catch(AipsError &x)
      {
	throw( AipsError("Restoration Error : " + x.getMesg() ) );
      }
	
    return beam;

  }

  void SIImageStore::pbcorPlane()
  {

    LogIO os( LogOrigin("SIImageStore","pbcorPlane",WHERE) );
    //     << ". Optionally, PB-correct too." << LogIO::POST;

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

    //    Matrix<Float> lsumwt = getSumWt( target );

    Array<Float> lsumwt;
    sumwt()->get( lsumwt , False ); // For MT, this will always pick the zeroth sumwt, which it should.

    IPosition imshape = target.shape();

    //cout << " SumWt  : " << lsumwt << " sumwtshape : " << lsumwt.shape() << " image shape : " << imshape << endl;

    AlwaysAssert( lsumwt.shape()[2] == imshape[2] , AipsError ); // polplanes
    AlwaysAssert( lsumwt.shape()[3] == imshape[3] , AipsError ); // chanplanes

    Bool div=False; // flag to signal if division actually happened, or weights are all 1.0

    for(Int pol=0; pol<lsumwt.shape()[2]; pol++)
      {
	for(Int chan=0; chan<lsumwt.shape()[3]; chan++)
	  {
	    IPosition pos(4,0,0,pol,chan);
	    if( lsumwt(pos) != 1.0 )
	      { 
		SubImage<Float>* subim=makePlane(  chan, True ,pol, True, target );
		LatticeExpr<Float> le( (*subim)/lsumwt(pos) );
		subim->copyData( le );
		div=True;
	      }
	  }
      }

    //    if( div==True ) cout << "Div image by sumwt : " << lsumwt << endl;
    //    else cout << "Already normalized" << endl;

    //    lsumwt = 1.0; setSumWt( target , lsumwt );

    return div;
  }

  void SIImageStore::calcSensitivity()
  {
    LogIO os( LogOrigin("SIImageStore","calcSensitivity",WHERE) );

    Array<Float> lsumwt;
    sumwt()->get( lsumwt , False ); // For MT, this will always pick the zeroth sumwt, which it should.

    IPosition shp( lsumwt.shape() );
    //cout << "Sumwt shape : " << shp << " : " << lsumwt << endl;
    //AlwaysAssert( shp.nelements()==4 , AipsError );
    
    os << "[" << itsImageName << "] Theoretical sensitivity (Jy/bm):" ;
    
    IPosition it(4,0,0,0,0);
    for ( it[0]=0; it[0]<shp[0]; it[0]++)
      for ( it[1]=0; it[1]<shp[1]; it[1]++)
	for ( it[2]=0; it[2]<shp[2]; it[2]++)
	  for ( it[3]=0; it[3]<shp[3]; it[3]++)
	    {
	      if( shp[0]>1 ){os << "f"<< it[0]+(it[1]*shp[0]) << ":" ;}
	      if( shp[3]>1 ) { os << "c"<< it[3] << ":"; }
	      if( shp[2]>1 ) { os << "p"<< it[2]<< ":" ; }
	      if( lsumwt( it )  > 1e-07 ) 
		{ 
		  os << 1.0/(sqrt(lsumwt(it))) << " " ;
		}
	      else
		{
		  os << "none" << " ";
		}
	    }
    
    os << LogIO::POST;

    //    Array<Float> sens = 1.0/sqrtsumwt;


  }


  //
  //-------------------------------------------------------------
  // Initialize the internals of the object.  Perhaps other such
  // initializations in the constructors can be moved here too.
  //
  void SIImageStore::init()
  {
    imageExts.resize(MAX_IMAGE_IDS);
    
    imageExts(MASK)=".mask";
    imageExts(PSF)=".psf";
    imageExts(MODEL)=".model";
    imageExts(RESIDUAL)=".residual";
    imageExts(WEIGHT)=".weight";
    imageExts(IMAGE)=".image";
    imageExts(SUMWT)=".sumwt";
    imageExts(FORWARDGRID)=".forward";
    imageExts(BACKWARDGRID)=".backward";
  }
  //
  //---------------------------------------------------------------
  //
  void SIImageStore::makePersistent(String& fileName)
  {
    LogIO logIO(LogOrigin("SIImageStore", "makePersistent"));
    ofstream outFile; outFile.open(fileName.c_str(),std::ofstream::out);
    if (!outFile) logIO << "Failed to open filed \"" << fileName << "\"" << LogIO::EXCEPTION;
    //  String itsImageName;
    outFile << "itsImageNameBase: " << itsImageName << endl;

    //IPosition itsImageShape;
    outFile << "itsImageShape: " << itsImageShape.nelements() << " ";
    for (uInt i=0;i<itsImageShape.nelements(); i++) outFile << itsImageShape(i) << " "; outFile << endl;

    // Don't know what to do with this.  Looks like this gets
    // filled-in from one of the images.  So load this from one of the
    // images if they exist?
    //CoordinateSystem itsCoordSys; 

    // Int itsNFacets;
    outFile << "itsNFacets: " << itsNFacets << endl;
    //Bool itsUseWeight, itsValidity;
    outFile << "itsUseWeight: " << itsUseWeight << endl;
    outFile << "itsValidity: " << itsValidity << endl;
    

    // Misc Information to go into the header. 
    //    Record itsMiscInfo; 
    itsMiscInfo.print(outFile);
    
    // CountedPtr<ImageInterface<Float> > itsMask, itsPsf, itsModel, itsResidual, itsWeight, itsImage, itsSumWt;
    // CountedPtr<ImageInterface<Complex> > itsForwardGrid, itsBackwardGrid;

    Vector<Bool> ImageExists(MAX_IMAGE_IDS);
    if (itsMask.null())     ImageExists(MASK)=False;
    if (itsPsf.null())      ImageExists(PSF)=False;
    if (itsModel.null())    ImageExists(MODEL)=False;
    if (itsResidual.null()) ImageExists(RESIDUAL)=False;
    if (itsWeight.null())   ImageExists(WEIGHT)=False;
    if (itsImage.null())    ImageExists(IMAGE)=False;
    if (itsSumWt.null())    ImageExists(SUMWT)=False;

    if (itsForwardGrid.null())    ImageExists(FORWARDGRID)=False;
    if (itsBackwardGrid.null())    ImageExists(BACKWARDGRID)=False;
    
    outFile << "ImagesExist: " << ImageExists << endl;
  }
  //
  //---------------------------------------------------------------
  //
  void SIImageStore::recreate(String& fileName)
  {
    LogIO logIO(LogOrigin("SIImageStore", "recreate"));
    ifstream inFile; inFile.open(fileName.c_str(),std::ofstream::out);
    if (!inFile) logIO << "Failed to open filed \"" << fileName << "\"" << LogIO::EXCEPTION;
      
    String token;
    inFile >> token; if (token == "itsImageNameBase:") inFile >> itsImageName;

    inFile >> token; 
    if (token=="itsImageShape:")
      {
	Int n;
	inFile >> n;
	itsImageShape.resize(n);
	for (Int i=0;i<n; i++) inFile >> itsImageShape(i);
      }

    // Int itsNFacets;
    inFile >> token; if (token=="itsNFacets:") inFile >> itsNFacets;
    //Bool itsUseWeight, itsValidity;
    inFile >> token; if (token=="itsUseWeight:") inFile >> itsUseWeight;
    inFile >> token; if (token=="itsValidity:") inFile >> itsValidity;

    Bool coordSysLoaded=False;
    String itsName;
    try 
      {
	itsName=itsImageName+imageExts(PSF);casa::openImage(itsName,      itsPsf);
	if (coordSysLoaded==False) {itsCoordSys=itsPsf->coordinates(); itsMiscInfo=itsPsf->miscInfo();coordSysLoaded=True;}
      } catch (AipsIO& x) {logIO << "\"" << itsName << "\" not found." << LogIO::WARN;};
    try 
      {
	itsName=itsImageName+imageExts(MASK);casa::openImage(itsName,     itsMask);
	if (coordSysLoaded==False) {itsCoordSys=itsMask->coordinates(); itsMiscInfo=itsImage->miscInfo();coordSysLoaded=True;}
      } catch (AipsIO& x) {logIO << "\"" << itsName << "\" not found." << LogIO::WARN;};
    try 
      {
	itsName=itsImageName+imageExts(MODEL);casa::openImage(itsName,    itsModel);
	if (coordSysLoaded==False) {itsCoordSys=itsModel->coordinates(); itsMiscInfo=itsModel->miscInfo();coordSysLoaded=True;}
      } catch (AipsIO& x) {logIO << "\"" << itsName << "\" not found." << LogIO::WARN;};
    try 
      {
	itsName=itsImageName+imageExts(RESIDUAL);casa::openImage(itsName, itsResidual);
	if (coordSysLoaded==False) {itsCoordSys=itsResidual->coordinates(); itsMiscInfo=itsResidual->miscInfo();coordSysLoaded=True;}
      } catch (AipsIO& x) {logIO << "\"" << itsName << "\" not found." << LogIO::WARN;};
    try 
      {
	itsName=itsImageName+imageExts(WEIGHT);casa::openImage(itsName,   itsWeight);
	if (coordSysLoaded==False) {itsCoordSys=itsWeight->coordinates(); itsMiscInfo=itsWeight->miscInfo();coordSysLoaded=True;}
      } catch (AipsIO& x) {logIO << "\"" << itsName << "\" not found." << LogIO::WARN;};
    try 
      {
	itsName=itsImageName+imageExts(IMAGE);casa::openImage(itsName,    itsImage);
	if (coordSysLoaded==False) {itsCoordSys=itsImage->coordinates(); itsMiscInfo=itsImage->miscInfo();coordSysLoaded=True;}
      } catch (AipsIO& x) {logIO << "\"" << itsName << "\" not found." << LogIO::WARN;};
    try 
      {
	itsName=itsImageName+imageExts(SUMWT);casa::openImage(itsName,    itsSumWt);
	if (coordSysLoaded==False) {itsCoordSys=itsSumWt->coordinates(); itsMiscInfo=itsSumWt->miscInfo();coordSysLoaded=True;}
      } catch (AipsIO& x) {logIO << "\"" << itsName << "\" not found." << LogIO::WARN;};
    try
      {
	casa::openImage(itsImageName+imageExts(FORWARDGRID),  itsForwardGrid);
	casa::openImage(itsImageName+imageExts(BACKWARDGRID), itsBackwardGrid);
      }
    catch (AipsError& x)
      {
	logIO << "Did not find forward and/or backward grid.  Just say'n..." << LogIO::POST;
      }

  }


  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END

