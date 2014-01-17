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

    itsImageShape=IPosition();
    itsImageName=String("");
    itsCoordSys=CoordinateSystem();
    itsMiscInfo=Record();

    itsValidity = False;

  }

  SIImageStore::SIImageStore(String imagename, 
			     CoordinateSystem &imcoordsys, 
			     IPosition imshape, const Bool /*overwrite*/)
  {
    LogIO os( LogOrigin("SIImageStore","Open new Images",WHERE) );

    itsPsf=NULL;
    itsModel=NULL;
    itsResidual=NULL;
    itsWeight=NULL;
    itsImage=NULL;

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
    itsMiscInfo=Record();

    itsImageName = imagename;

    // The PSF or Residual images must exist. 
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
  SIImageStore::SIImageStore(ImageInterface<Float>* modelim, ImageInterface<Float>* residim,
			     ImageInterface<Float>* psfim, ImageInterface<Float>* weightim, ImageInterface<Float>* restoredim)
  {
    itsPsf=psfim;
    itsModel=modelim;
    itsResidual=residim;
    itsWeight=weightim;
    itsImage=restoredim;
    itsImageShape=modelim->shape();
	
    // TODO : Modify to support a subset of valid images ? 
    itsValidity=((!itsPsf.null()) &&  (!itsModel.null()) &&   (!itsResidual.null()) &&  (!itsWeight.null()) &&
		 (!itsImage.null()));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////make a facet image store which refers to a sub section of images 
  ///////////////////////in this storage
  SIImageStore* SIImageStore::getFacetImageStore(const Int facet, const Int nfacets){
    SubImage<Float>* facetPSF=makeFacet(facet, nfacets, *itsPsf);
    SubImage<Float>* facetModel=makeFacet(facet, nfacets, *itsModel);
    SubImage<Float>* facetResidual=makeFacet(facet, nfacets, *itsResidual);
    SubImage<Float>* facetWeight=makeFacet(facet, nfacets, *itsWeight);
    SubImage<Float>* facetImage=makeFacet(facet, nfacets, *itsImage);
    return new SIImageStore(facetModel, facetResidual, facetPSF, facetWeight, facetImage);

  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //// Either read an image from disk, or construct one. 
  CountedPtr<ImageInterface<Float> > SIImageStore::openImage(const String imagenamefull, const Bool overwrite)
  {

    CountedPtr<ImageInterface<Float> > imPtr;

    if( overwrite || !Table::isWritable( imagenamefull ) )
      {
	imPtr=new PagedImage<Float> (itsImageShape, itsCoordSys, imagenamefull);
      }
    else
      {
	if(Table::isWritable( imagenamefull ))
	  {
	    cerr << "Trying to open "<< imagenamefull << endl;
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
  Bool SIImageStore::checkValidity(const Bool psf, const Bool residual, const Bool weight, const Bool model, const Bool restored)
  {

    Bool valid = True;

    if(psf==True){ valid = valid & ( !itsPsf.null() && itsPsf->shape()==itsImageShape ); }
    if(residual==True){ valid = valid & ( !itsResidual.null()&& itsResidual->shape()==itsImageShape ); }
    if(weight==True){ valid = valid & ( !itsWeight.null()&& itsWeight->shape()==itsImageShape ); }
    if(model==True){ valid = valid & ( !itsModel.null() && itsModel->shape()==itsImageShape); }
    if(restored==True){ valid = valid & ( !itsImage.null() && itsImage->shape()==itsImageShape); }

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

    return True; // do something more intelligent here.
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIImageStore::setModelImage( String modelname )
  {
    LogIO os( LogOrigin("SIImageStore","setModelImage",WHERE) );

    os << LogIO::WARN << "TODO : Need to check shapes with the store and regrid if needed." << LogIO::POST;
    
    Directory immodel( modelname+String(".model") );
    if( !immodel.exists() ) 
      {
	os << "Starting model image does not exist. No initial prediction will be done" << LogIO::POST;
	return;
      }

    CountedPtr<PagedImage<Float> > model = new PagedImage<Float>( modelname+String(".model") );
    // Check shapes, coordsys with those of other images.  If different, try to re-grid here.

    if( model->shape() != itsModel->shape() )
      {
	// For now, throw an exception.
	throw( AipsError( "Input model image "+modelname+".model is not the same shape as that defined for output in "+ itsImageName + ".model" ) );
      }

    os << "Setting " << modelname << " as model " << LogIO::POST;
    // Then, add its contents to itsModel.
    //itsModel->put( itsModel->get() + model->get() );
    itsModel->put( model->get() );
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

  CountedPtr<ImageInterface<Float> > SIImageStore::psf()
  {
    if( !itsPsf.null() && itsPsf->shape() == itsImageShape ) { return itsPsf; }
    itsPsf = openImage( itsImageName+String(".psf") , False );
    return itsPsf;
  }
  CountedPtr<ImageInterface<Float> > SIImageStore::residual()
  {
    if( !itsResidual.null() && itsResidual->shape() == itsImageShape ) { return itsResidual; }
    itsResidual = openImage( itsImageName+String(".residual") , False );
    return itsResidual;
  }
  CountedPtr<ImageInterface<Float> > SIImageStore::weight()
  {
    if( !itsWeight.null() && itsWeight->shape() == itsImageShape ) { return itsWeight; }
    itsWeight = openImage( itsImageName+String(".weight") , False );

    /*  /// TODO : Do something here, to support absence of weight image when they contain a single number ! 
    if( itsWeight.null() )
      {
	throw( AipsError("Internal error : Weight Image does not exist. Please check with SIImageStore.hasWeight() before accessing the weight image. If not present, treat it as a scalar = 1.0") );
      }
    */
    return itsWeight;
  }
  CountedPtr<ImageInterface<Float> > SIImageStore::model()
  {
    if( !itsModel.null() && itsModel->shape() == itsImageShape ) { return itsModel; }

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
  CountedPtr<ImageInterface<Float> > SIImageStore::image()
  {
    if( !itsImage.null() && itsImage->shape() == itsImageShape ) { return itsImage; }
    itsImage = openImage( itsImageName+String(".image") , False );
    itsImage->setUnits("Jy/beam");
    return itsImage;
  }
  CountedPtr<ImageInterface<Complex> > SIImageStore::forwardGrid(){
	  if(!itsForwardGrid.null() && (itsForwardGrid->shape() == itsImageShape))
		  return itsForwardGrid;
	  itsForwardGrid=new TempImage<Complex>(TiledShape(itsImageShape, tileShape()), itsCoordSys, memoryBeforeLattice());
	  return itsForwardGrid;
  }
  CountedPtr<ImageInterface<Complex> > SIImageStore::backwardGrid(){
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

    /*
    if( itsWeight.null() )
      {
	throw( AipsError("Internal Error : Weight image from major cycle is not present. Cannot gather a weighted sum from all nodes") );
      }
    */

    if(addpsf)
      {
	LatticeExpr<Float> adderPsf( *(psf()) + *(imagestoadd->psf()) ); 
	itsPsf->copyData(adderPsf);
      }
    if(addresidual)
      {
	LatticeExpr<Float> adderRes( *(residual()) + *(imagestoadd->residual()) ); 
	itsResidual->copyData(adderRes);
      }
    if(addweight)
      {
	LatticeExpr<Float> adderWeight( *(weight()) + *(imagestoadd->weight()) ); 
	itsWeight->copyData(adderWeight);
      }
    ///cout << "Res : " << itsResidual->getAt( IPosition(4,0,0,0,0) ) << "  Wt : " << itsWeight->getAt( IPosition(4,0,0,0,0) ) << endl;
  }

  // Make another for the PSF too.
  void SIImageStore::divideResidualByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStore","divideResidualByWeight",WHERE) );

    /*
    if( itsWeight.null() )
      {
	os << "Weights are 1.0. Not dividing " << itsImageName+String(".residual") << LogIO::POST;
      }
    else
    */
      {
	//	if( itsResNormed==False )
	  {
	    os << "Dividing " << itsImageName+String(".residual") << " by the weight image " << itsImageName+String(".weight") << LogIO::POST;
	    
	    ///cout << " Dividing : " << itsResidual->getAt( IPosition(4,0,0,0,0) ) << " by " << itsWeight->getAt( IPosition(4,0,0,0,0) ) << endl;
	    
	    LatticeExpr<Float> mask( iif( (*(weight())) > weightlimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (*(weight())) > weightlimit , 0.0, 1.0 ) );
	    
	    LatticeExpr<Float> ratio( ( (*(residual())) * mask ) / ( (*(weight())) + maskinv) );
	    itsResidual->copyData(ratio);
	    //   itsResNormed=True;
	  }
	  /*
	else
	  {
	    os << itsImageName+String(".residual") << " is already normalized." << LogIO::POST;
	  }
	  */
      }
    // createMask
  }

  void SIImageStore::dividePSFByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStore","dividePSFByWeight",WHERE) );

    /*
    if( itsWeight.null() )
      {
	os << "Weights are 1.0. Not dividing " << itsImageName+String(".psf") << LogIO::POST;
      }
    else
    */
      {
	//	if ( itsPsfNormed==False )
	  {
	    os << "Dividing " << itsImageName+String(".psf") << " by the weight image " << itsImageName+String(".weight") << LogIO::POST;
	    
	    ///cout << " Dividing : " << itsResidual->getAt( IPosition(4,0,0,0,0) ) << " by " << itsWeight->getAt( IPosition(4,0,0,0,0) ) << endl;
	    //	    cerr << "weight limit " <<  weightlimit << endl;
	    LatticeExpr<Float> mask( iif( (*(weight())) > weightlimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (*(weight())) > weightlimit , 0.0, 1.0 ) );
	    
	    LatticeExpr<Float> ratio( ( (*(psf())) * mask ) / ( (*(weight())) + maskinv) );
	    itsPsf->copyData(ratio);
	    // itsPsfNormed=True;
	  }
	  /*	else
	  {
	    os << itsImageName+String(".psf") << " is already normalized." << LogIO::POST;
	  }
	  */
      }
    // createMask
  }

  void SIImageStore::divideModelByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStore","divideModelByWeight",WHERE) );

    /*
    if( itsWeight.null() )
      {
	os << "Weights are 1.0. Not dividing " << itsImageName+String(".residual") << LogIO::POST;
      }
    else
    */
      {
	os << "Dividing " << itsImageName+String(".model") << " by the weight image " << itsImageName+String(".weight") << LogIO::POST;
	
	LatticeExpr<Float> mask( iif( (*(weight())) > weightlimit , 1.0, 0.0 ) );
	LatticeExpr<Float> maskinv( iif( (*(weight())) > weightlimit , 0.0, 1.0 ) );
	
	LatticeExpr<Float> ratio( ( (*(model())) * mask ) / ( (*(weight())) + maskinv) );
	itsModel->copyData(ratio);
      }    
    // createMask
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END

