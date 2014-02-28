//# SIImageStoreMultiTerm.cc: Implementation of Imager.h
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
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>

#include <synthesis/ImagerObjects/SIImageStoreMultiTerm.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  
  SIImageStoreMultiTerm::SIImageStoreMultiTerm():SIImageStore()
  {
    itsPsfs.resize(0);
    itsModels.resize(0);
    itsResiduals.resize(0);
    itsWeights.resize(0);
    itsImages.resize(0);

    itsForwardGrids.resize(0);
    itsBackwardGrids.resize(0);

    itsNTerms=0;

    itsImageShape=IPosition();
    itsImageName=String("");
    itsCoordSys=CoordinateSystem();
    itsMiscInfo=Record();

    itsValidity = False;

  }
  
  SIImageStoreMultiTerm::SIImageStoreMultiTerm(String imagename, 
					       CoordinateSystem &imcoordsys, 
					       IPosition imshape, const Bool /*overwrite*/, 
					       uInt ntaylorterms)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","Open new Images",WHERE) );

    itsNTerms = ntaylorterms;

    itsPsfs.resize(2 * itsNTerms - 1);
    itsModels.resize(itsNTerms);
    itsResiduals.resize(itsNTerms);
    itsWeights.resize(2 * itsNTerms - 1);
    itsImages.resize(itsNTerms);

    itsForwardGrids.resize(itsNTerms);
    itsBackwardGrids.resize(2 * itsNTerms - 1);

    itsImageName = imagename;
    itsImageShape = imshape;
    itsCoordSys = imcoordsys;

    itsMiscInfo=Record();

  }

  SIImageStoreMultiTerm::SIImageStoreMultiTerm(String imagename, uInt ntaylorterms) 
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","Open existing Images",WHERE) );

    itsNTerms = ntaylorterms;

    itsPsfs.resize(2 * itsNTerms - 1);
    itsModels.resize(itsNTerms);
    itsResiduals.resize(itsNTerms);
    itsWeights.resize(2 * itsNTerms - 1);
    itsImages.resize(itsNTerms);
    itsMiscInfo=Record();

    itsForwardGrids.resize(itsNTerms);
    itsBackwardGrids.resize(2 * itsNTerms - 1);

    itsImageName = imagename;


    Bool exists=True;
    for(uInt tix=0;tix<itsNTerms;tix++)
      {
	exists &= ( doesImageExist( itsImageName+String(".residual.tt")+String::toString(tix) ) ||
		    doesImageExist( itsImageName+String(".psf.tt")+String::toString(tix) ) );
      }

    // The PSF or Residual images must exist. 
    if( exists )
      {
	CountedPtr<ImageInterface<Float> > imptr;
	if( doesImageExist(itsImageName+String(".psf.tt0")) )
	  imptr = new PagedImage<Float> (itsImageName+String(".psf.tt0"));
	else
	  imptr = new PagedImage<Float> (itsImageName+String(".residual.tt0"));
	  
	itsImageShape = imptr->shape();
	itsCoordSys = imptr->coordinates();
      }
    else
      {
	throw( AipsError( "Multi-term PSF or Residual Images do not exist. Please create one of them." ) );
      }
  }


  /////////////Constructor with pointers already created else where but taken over here
  SIImageStoreMultiTerm::SIImageStoreMultiTerm(Block<CountedPtr<ImageInterface<Float> > > modelims, 
					       Block<CountedPtr<ImageInterface<Float> > >residims,
					       Block<CountedPtr<ImageInterface<Float> > >psfims, 
					       Block<CountedPtr<ImageInterface<Float> > >weightims, 
					       Block<CountedPtr<ImageInterface<Float> > >restoredims,
					       CountedPtr<ImageInterface<Float> > newalpha,
					       CountedPtr<ImageInterface<Float> > newbeta)
  {
    
    itsPsfs=psfims;
    itsModels=modelims;
    itsResiduals=residims;
    itsWeights=weightims;
    itsImages=restoredims;
    itsAlpha = newalpha;
    itsBeta = newbeta;

    itsNTerms = itsResiduals.nelements();
    itsMiscInfo=Record();

    AlwaysAssert( itsPsfs.nelements() == 2*itsNTerms-1 , AipsError ); 
    AlwaysAssert( itsPsfs.nelements()>0 && !itsPsfs[0].null() , AipsError );

    itsForwardGrids.resize( itsNTerms );
    itsBackwardGrids.resize( 2 * itsNTerms - 1 );

    itsImageShape=psfims[0]->shape();
    itsCoordSys = psfims[0]->coordinates();
    itsMiscInfo = psfims[0]->miscInfo();

    itsImageName = String("reference");  // This is what the access functions use to guard against allocs...
	
    itsValidity = checkValidity(True/*psf*/, True/*res*/,False/*wgt*/,False/*model*/,False/*image*/);
    itsWeightExists = itsValidity;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  // Check if images that are asked-for are ready and all have the same shape.
  Bool SIImageStoreMultiTerm::checkValidity(const Bool ipsf, const Bool iresidual, 
					    const Bool iweight, const Bool imodel, const Bool irestored, 
					    const Bool ialpha, const Bool ibeta)
  {

    Bool valid = True;

    for(uInt tix=0; tix<2*itsNTerms-1; tix++)
      {
	
	if(ipsf==True)
	  { psf(tix); 
	    valid = valid & ( !itsPsfs[tix].null() && itsPsfs[tix]->shape()==itsImageShape ); }
	if(iweight==True)
	  { weight(tix);  
	    valid = valid & ( !itsWeights[tix].null()&& itsWeights[tix]->shape()==itsImageShape ); }
	
	if( tix< itsNTerms )
	  {
	    if(iresidual==True)
	      { residual(tix);  
		valid = valid & ( !itsResiduals[tix].null()&& itsResiduals[tix]->shape()==itsImageShape ); }
	    if(imodel==True)
	      { model(tix);
		valid = valid & ( !itsModels[tix].null() && itsModels[tix]->shape()==itsImageShape); }
	    if(irestored==True)
	      { image(tix);
		valid = valid & ( !itsImages[tix].null() && itsImages[tix]->shape()==itsImageShape); }
	  }
      }
    
    if(ialpha==True)
      { alpha();  valid = valid & ( !itsAlpha.null()&& itsAlpha->shape()==itsImageShape ); }
    if(ibeta==True)
      { beta();  valid = valid & ( !itsBeta.null()&& itsBeta->shape()==itsImageShape ); }

    return valid;
    
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  SIImageStoreMultiTerm::~SIImageStoreMultiTerm() 
  {
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  Bool SIImageStoreMultiTerm::releaseLocks() 
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","releaseLocks",WHERE) );

    for(uInt tix=0; tix<itsNTerms; tix++)
      {
	if( ! itsPsfs[tix].null() ) itsPsfs[tix]->unlock();
	if( ! itsModels[tix].null() ) itsModels[tix]->unlock();
	if( ! itsResiduals[tix].null() ) itsResiduals[tix]->unlock();
	if( ! itsImages[tix].null() ) itsImages[tix]->unlock();
	if( ! itsWeights[tix].null() ) itsWeights[tix]->unlock();
      }
    
    return True; // do something more intelligent here.
  }

  Double SIImageStoreMultiTerm::getReferenceFrequency()
  {
    return (itsCoordSys.spectralCoordinate()).restFrequency();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIImageStoreMultiTerm::setModelImage( String modelname )
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","setModelImage",WHERE) );

    for(uInt tix=0;tix<itsNTerms;tix++)
      {
	
	Directory immodel( modelname+String(".model.tt")+String::toString(tix) );
	if( !immodel.exists() ) 
	  {
	    os << "Starting model image does not exist for term : " << tix << LogIO::POST;
	  }
	else
	  {
	    CountedPtr<PagedImage<Float> > newmodel = new PagedImage<Float>( modelname+String(".model.tt")+String::toString(tix) );
	    // Check shapes, coordsys with those of other images.  If different, try to re-grid here.
	    
	    if( newmodel->shape() != model(tix)->shape() )
	      {
		// For now, throw an exception.
		throw( AipsError( "Input model image "+modelname+".model.tt"+String::toString(tix)+" is not the same shape as that defined for output in "+ itsImageName + ".model" ) );
	      }
	    
	    os << "Setting " << modelname << " as model for term " << tix << LogIO::POST;
	    // Then, add its contents to itsModel.
	    //itsModel->put( itsModel->get() + model->get() );
	    ( model(tix) )->put( newmodel->get() );
	  }
      }//nterms
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  CountedPtr<ImageInterface<Float> > SIImageStoreMultiTerm::psf(uInt term)
  {
    AlwaysAssert( itsPsfs.nelements() > term, AipsError );
    if( !itsPsfs[term].null() && itsPsfs[term]->shape() == itsImageShape ) { return itsPsfs[term]; }
    checkRef( itsPsfs[term] , "psf.tt"+String::toString(term) );
    itsPsfs[term] = openImage( itsImageName+String(".psf.tt")+String::toString(term) , False );
    return itsPsfs[term];
  }
  CountedPtr<ImageInterface<Float> > SIImageStoreMultiTerm::residual(uInt term)
  {
    if( !itsResiduals[term].null() && itsResiduals[term]->shape() == itsImageShape ) { return itsResiduals[term]; }
    checkRef( itsPsfs[term] , "psf.tt"+String::toString(term) );
    itsResiduals[term] = openImage( itsImageName+String(".residual.tt")+String::toString(term) , False );
    return itsResiduals[term];
  }
  CountedPtr<ImageInterface<Float> > SIImageStoreMultiTerm::weight(uInt term)
  {
    if( !itsWeights[term].null() && itsWeights[term]->shape() == itsImageShape ) { return itsWeights[term]; }
    checkRef( itsWeights[term] , "weight.tt"+String::toString(term) );
    itsWeights[term] = openImage( itsImageName+String(".weight.tt")+String::toString(term) , False );

    /*  /// TODO : Do something here, to support absence of weight image when they contain a single number ! 
    if( itsWeight.null() )
      {
	throw( AipsError("Internal error : Weight Image does not exist. Please check with SIImageStoreMultiTerm.hasWeight() before accessing the weight image. If not present, treat it as a scalar = 1.0") );
      }
    */
    return itsWeights[term];
  }
  CountedPtr<ImageInterface<Float> > SIImageStoreMultiTerm::model(uInt term)
  {
    if( !itsModels[term].null() && itsModels[term]->shape() == itsImageShape ) { return itsModels[term]; }
    checkRef( itsModels[term] , "model.tt"+String::toString(term) );
    itsModels[term] = openImage( itsImageName+String(".model.tt")+String::toString(term) , False );

    // Set up header info the first time.
    ImageInfo info = itsModels[term]->imageInfo();
    String objectName("");
    if( itsMiscInfo.isDefined("OBJECT") ){ itsMiscInfo.get("OBJECT", objectName); }
    info.setObjectName(objectName);
    itsModels[term]->setImageInfo( info );
    itsModels[term]->setMiscInfo( itsMiscInfo );
    itsModels[term]->setUnits("Jy/pixel");

    return itsModels[term];
  }

  CountedPtr<ImageInterface<Float> > SIImageStoreMultiTerm::image(uInt term)
  {
    if( !itsImages[term].null() && itsImages[term]->shape() == itsImageShape ) { return itsImages[term]; }
    checkRef( itsImages[term] , "image.tt"+String::toString(term) );
    itsImages[term] = openImage( itsImageName+String(".image.tt")+String::toString(term) , False );
    itsImages[term]->setUnits("Jy/beam");
    return itsImages[term];
  }
  CountedPtr<ImageInterface<Complex> > SIImageStoreMultiTerm::forwardGrid(uInt term){
	  if(!itsForwardGrids[term].null() && (itsForwardGrids[term]->shape() == itsImageShape))
		  return itsForwardGrids[term];
	  itsForwardGrids[term]=new TempImage<Complex>(TiledShape(itsImageShape, tileShape()), itsCoordSys, memoryBeforeLattice());
	  return itsForwardGrids[term];
  }
  CountedPtr<ImageInterface<Complex> > SIImageStoreMultiTerm::backwardGrid(uInt term){
  	  if(!itsBackwardGrids[term].null() && (itsBackwardGrids[term]->shape() == itsImageShape))
  		  return itsBackwardGrids[term];
  	  itsBackwardGrids[term]=new TempImage<Complex>(TiledShape(itsImageShape, tileShape()), itsCoordSys, memoryBeforeLattice());
  	  return itsBackwardGrids[term];
    }


  CountedPtr<ImageInterface<Float> > SIImageStoreMultiTerm::alpha()
  {
    if( !itsAlpha.null() && itsAlpha->shape() == itsImageShape ) { return itsAlpha; }
    checkRef( itsAlpha , "alpha" );
    itsAlpha = openImage( itsImageName+String(".alpha"), False );
    //    itsAlpha->setUnits("Alpha");
    return itsAlpha;
  }

  CountedPtr<ImageInterface<Float> > SIImageStoreMultiTerm::beta()
  {
    if( !itsBeta.null() && itsBeta->shape() == itsImageShape ) { return itsBeta; }
    checkRef( itsBeta , "beta" );
    itsBeta = openImage( itsImageName+String(".alpha"), False );
    //    itsBeta->setUnits("Beta");
    return itsBeta;
  }



    // TODO : Move to an image-wrapper class ? Same function exists in SynthesisDeconvolver.
  Bool SIImageStoreMultiTerm::doesImageExist(String imagename)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","doesImageExist",WHERE) );
    Directory image( imagename );
    return image.exists();
  }


  void SIImageStoreMultiTerm::resetImages( Bool resetpsf, Bool resetresidual, Bool resetweight )
  {
    for(uInt tix=0;tix<itsNTerms;tix++)
      {
	if( resetpsf ) psf(tix)->set(0.0);
	if( resetresidual ) residual(tix)->set(0.0);
	if( resetweight && !itsWeights[tix].null() ) weight(tix)->set(0.0);
      }
  }

  void SIImageStoreMultiTerm::addImages( CountedPtr<SIImageStore> imagestoadd,
				Bool addpsf, Bool addresidual, Bool addweight)
  {

    /*
    if( itsWeight.null() )
      {
	throw( AipsError("Internal Error : Weight image from major cycle is not present. Cannot gather a weighted sum from all nodes") );
      }
    */

    for(uInt tix=0;tix<itsNTerms;tix++)
      {
	
	if(addpsf)
	  {
	    LatticeExpr<Float> adderPsf( *(psf(tix)) + *(imagestoadd->psf(tix)) ); 
	    itsPsfs[tix]->copyData(adderPsf);
	  }
	if(addresidual)
	  {
	    LatticeExpr<Float> adderRes( *(residual(tix)) + *(imagestoadd->residual(tix)) ); 
	    itsResiduals[tix]->copyData(adderRes);
	  }
	if(addweight)
	  {
	    LatticeExpr<Float> adderWeight( *(weight(tix)) + *(imagestoadd->weight(tix)) ); 
	    itsWeights[tix]->copyData(adderWeight);
	  }
      }
  }

  // Make another for the PSF too.
  void SIImageStoreMultiTerm::divideResidualByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","divideResidualByWeight",WHERE) );

    /*
    if( itsWeight.null() )
      {
	os << "Weights are 1.0. Not dividing " << itsImageName+String(".residual") << LogIO::POST;
      }
    else
    */
    for(uInt tix=0;tix<itsNTerms;tix++)
      {
	
	os << "Dividing " << itsImageName+String(".residual.tt")+String::toString(tix) << " by the weight image " << itsImageName+String(".weight.tt0") << LogIO::POST;
	
	LatticeExpr<Float> mask( iif( (*(weight(0))) > weightlimit , 1.0, 0.0 ) );
	LatticeExpr<Float> maskinv( iif( (*(weight(0))) > weightlimit , 0.0, 1.0 ) );
	
	LatticeExpr<Float> ratio( ( (*(residual(tix))) * mask ) / ( (*(weight(0))) + maskinv) );
	itsResiduals[tix]->copyData(ratio);
	
      }
    // createMask
  }

  void SIImageStoreMultiTerm::dividePSFByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","dividePSFByWeight",WHERE) );

    /*
    if( itsWeight.null() )
      {
	os << "Weights are 1.0. Not dividing " << itsImageName+String(".psf") << LogIO::POST;
      }
    else
    */
    for(uInt tix=0;tix<2*itsNTerms-1;tix++)
      {
	    os << "Dividing " << itsImageName+String(".psf.tt")+String::toString(tix) << " by the weight image " << itsImageName+String(".weight.tt0") << LogIO::POST;
	    //	    cerr << "weight limit " <<  weightlimit << endl;
	    LatticeExpr<Float> mask( iif( (*(weight(0))) > weightlimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (*(weight(0))) > weightlimit , 0.0, 1.0 ) );
	    
	    LatticeExpr<Float> ratio( ( (*(psf(tix))) * mask ) / ( (*(weight(0))) + maskinv) );
	    itsPsfs[tix]->copyData(ratio);

      }
    // createMask
  }

  void SIImageStoreMultiTerm::divideModelByWeight(Float weightlimit)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","divideModelByWeight",WHERE) );

    /*
    if( itsWeight.null() )
      {
	os << "Weights are 1.0. Not dividing " << itsImageName+String(".residual") << LogIO::POST;
      }
    else
    */
    for(uInt tix=0;tix<itsNTerms;tix++)
      {
	os << "Dividing " << itsImageName+String(".model")+String::toString(tix) << " by the weight image " << itsImageName+String(".weight.tt0") << LogIO::POST;
	
	LatticeExpr<Float> mask( iif( (*(weight(0))) > weightlimit , 1.0, 0.0 ) );
	LatticeExpr<Float> maskinv( iif( (*(weight(0))) > weightlimit , 0.0, 1.0 ) );
	
	LatticeExpr<Float> ratio( ( (*(model(tix))) * mask ) / ( (*(weight(0))) + maskinv) );
	itsModels[tix]->copyData(ratio);
      }    
    // createMask
  }

  void SIImageStoreMultiTerm::restorePlane()
  {

    LogIO os( LogOrigin("SIImageStoreMultiTerm","restorePlane",WHERE) );

    try
      {
	// Fit a Gaussian to the PSF.
	GaussianBeam beam = getPSFGaussian();

	os << "Restore with beam : " 
	   << beam.getMajor(Unit("arcmin")) << " arcmin, " 
	   << beam.getMinor(Unit("arcmin"))<< " arcmin, " 
	   << beam.getPA(Unit("deg")) << " deg" << LogIO::POST; 

	/*	
	// Compute principal solution ( if it hasn't already been done to this ImageStore......  )
	itsMTCleaner.computeprincipalsolution();
	for(uInt tix=0; tix<itsNTerms; tix++)
	  {
	    Matrix<Float> tempRes;
	    itsMTCleaner.getresidual(tix,tempRes);
	    (itsImages->residual(tix))->put( tempRes );
	  }
	*/


	// Restore all terms
	ImageInfo ii = image(0)->imageInfo();
	ii.setRestoringBeam( beam );

	for(uInt tix=0; tix<itsNTerms; tix++)
	  {
	    (image(tix))->set(0.0);
	    (image(tix))->copyData( LatticeExpr<Float>(*(model(tix))) );
	    StokesImageUtil::Convolve( *(image(tix)) , beam);
	    (image(tix))->copyData( LatticeExpr<Float>
					       ( *(image(tix)) + *(residual(tix)) )   );
	    image()->setImageInfo(ii);
	  }	
	
	// Calculate alpha and beta
	LatticeExprNode leMaxRes = max( *( residual(0) ) );
	Float maxres = leMaxRes.getFloat();
	Float specthreshold = maxres/10.0;  //////////// do something better here..... 

      os << "Calculating spectral parameters for  Intensity > peakresidual/10 = " << specthreshold << " Jy/beam" << LogIO::POST;
      LatticeExpr<Float> mask1(iif(((*(image(0))))>(specthreshold),1.0,0.0));
      LatticeExpr<Float> mask0(iif(((*(image(0))))>(specthreshold),0.0,1.0));

      /////// Calculate alpha
      LatticeExpr<Float> alphacalc( (((*(image(1))))*mask1)/(((*(image(0))))+(mask0)) );
      alpha()->copyData(alphacalc);

      // Set the restoring beam for alpha
      alpha()->setImageInfo(ii);
      //alpha()->table().unmarkForDelete();

      // Make a mask for the alpha image
      LatticeExpr<Bool> lemask(iif(((*(image(0))) > specthreshold) , True, False));

      //      createMask( lemask, (alpha()) );

      }
    catch(AipsError &x)
      {
	throw( AipsError("Multi-Term Restoration Error : " + x.getMesg() ) );
      }

  }


  Bool SIImageStoreMultiTerm::createMask(LatticeExpr<Bool> &lemask, 
					 CountedPtr<ImageInterface<Float> > outimage)
{
  //      ImageRegion outreg = outimage->makeMask("mask0",False,True);
  ImageRegion outreg = ((PagedImage<Float> *)(&*outimage))->makeMask("mask0",False,True);
  LCRegion& outmask=outreg.asMask();
  outmask.copyData(lemask);
  outimage->defineRegion("mask0",outreg, RegionHandler::Masks, True);
  outimage->setDefaultMask("mask0");
  return True;
}


  CountedPtr<SIImageStore> SIImageStoreMultiTerm::getFacetImageStore(const Int facet, const Int nfacets)
  {
    Block<CountedPtr<ImageInterface<Float> > > psflist(2*itsNTerms-1);
    Block<CountedPtr<ImageInterface<Float> > > modellist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > residuallist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > weightlist(2*itsNTerms-1);
    Block<CountedPtr<ImageInterface<Float> > > imagelist(itsNTerms);
    CountedPtr<ImageInterface<Float> > newalpha;
    CountedPtr<ImageInterface<Float> > newbeta;

    for(uInt tix=0; tix<2*itsNTerms-1;tix++)
      {
	psflist[tix] = itsPsfs[tix].null()?NULL:makeFacet(facet, nfacets, *itsPsfs[tix]);	
	weightlist[tix] = itsWeights[tix].null()?NULL:makeFacet(facet, nfacets, *itsWeights[tix]);

	if( tix<itsNTerms )
	  {
	    modellist[tix] = itsModels[tix].null()?NULL:makeFacet(facet, nfacets, *itsModels[tix]);
	    residuallist[tix] = itsResiduals[tix].null()?NULL:makeFacet(facet, nfacets, *itsResiduals[tix]);
	    imagelist[tix] = itsImages[tix].null()?NULL:makeFacet(facet, nfacets, *itsImages[tix]);
	  }
      }
    newalpha = itsAlpha.null()?NULL:makeFacet(facet, nfacets, *itsAlpha);
    newbeta = itsBeta.null()?NULL:makeFacet(facet, nfacets, *itsBeta);

    return new SIImageStoreMultiTerm(modellist, residuallist, psflist, weightlist, imagelist,newalpha,newbeta);

  }

  CountedPtr<SIImageStore> SIImageStoreMultiTerm::getSubImageStore(const Int chan, const Bool onechan,
							  const Int pol, const Bool onepol)
  {

    ///    cout << "in MT subImStor : " << chan << " " << onechan << " " << pol << " " << onepol << endl;

    if( !onechan && !onepol ) {return this;}    // No slicing is required. 

    Block<CountedPtr<ImageInterface<Float> > > psflist(2*itsNTerms-1);
    Block<CountedPtr<ImageInterface<Float> > > modellist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > residuallist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > weightlist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > imagelist(itsNTerms);
    CountedPtr<ImageInterface<Float> > newalpha;
    CountedPtr<ImageInterface<Float> > newbeta;

    for(uInt tix=0; tix<2*itsNTerms-1;tix++)
      {
	psflist[tix] = itsPsfs[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsPsfs[tix]);
	
	if( tix<itsNTerms )
	  {
	    modellist[tix] = itsModels[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsModels[tix]);
	    residuallist[tix] = itsResiduals[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsResiduals[tix]);
	    weightlist[tix] = itsWeights[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsWeights[tix]);
	    imagelist[tix] = itsImages[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsImages[tix]);
	  }
      }
    newalpha = itsAlpha.null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsAlpha);
    newbeta = itsBeta.null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsBeta);

    return new SIImageStoreMultiTerm(modellist, residuallist, psflist, weightlist, imagelist,newalpha,newbeta);

  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END

