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

#include <casa/Arrays/MatrixMath.h>
#include <scimath/Mathematics/MatrixMathLA.h>

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
    itsSumWts.resize(0);
    itsMask=NULL;

    itsForwardGrids.resize(0);
    itsBackwardGrids.resize(0);

    itsNTerms=0;
    itsNFacets=1;
    itsUseWeight=False;

    itsImageShape=IPosition();
    itsImageName=String("");
    itsCoordSys=CoordinateSystem();
    itsMiscInfo=Record();

    //    itsValidity = False;

  }
  
  SIImageStoreMultiTerm::SIImageStoreMultiTerm(String imagename, 
					       CoordinateSystem &imcoordsys, 
					       IPosition imshape, 
					       const Int nfacets,
					       const Bool /*overwrite*/, 
					       uInt ntaylorterms,
					       Bool useweightimage)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","Open new Images",WHERE) );

    itsNTerms = ntaylorterms;

    itsPsfs.resize(2 * itsNTerms - 1);
    itsModels.resize(itsNTerms);
    itsResiduals.resize(itsNTerms);
    itsWeights.resize(2 * itsNTerms - 1);
    itsImages.resize(itsNTerms);
    itsSumWts.resize(2 * itsNTerms - 1);

    itsMask=NULL;

    itsForwardGrids.resize(itsNTerms);
    itsBackwardGrids.resize(2 * itsNTerms - 1);

    itsImageName = imagename;
    itsImageShape = imshape;
    itsCoordSys = imcoordsys;
    itsNFacets = nfacets;
    itsUseWeight = useweightimage;

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
    itsSumWts.resize(2 * itsNTerms - 1);
    itsMask=NULL;

    itsMiscInfo=Record();

    itsForwardGrids.resize(itsNTerms);
    itsBackwardGrids.resize(2 * itsNTerms - 1);

    itsImageName = imagename;


    Bool exists=True;
    Bool sumwtexists=True;
    for(uInt tix=0;tix<2*itsNTerms-1;tix++) 
      {
	if( tix<itsNTerms ) {
	    exists &= ( doesImageExist( itsImageName+String(".residual.tt")+String::toString(tix) ) ||
			doesImageExist( itsImageName+String(".psf.tt")+String::toString(tix) ) );
	  }else {
	    exists &= ( doesImageExist( itsImageName+String(".psf.tt")+String::toString(tix) ) );
	    sumwtexists &= ( doesImageExist( itsImageName+String(".sumwt.tt")+String::toString(tix) ) );
	  }
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

    if( sumwtexists )
      {
	CountedPtr<ImageInterface<Float> > imptr;
	imptr = new PagedImage<Float> (itsImageName+String(".sumwt.tt0"));
	itsNFacets = imptr->shape()[0];
	itsUseWeight = getUseWeightImage( *imptr );
	if( itsUseWeight && ! doesImageExist(itsImageName+String(".weight.tt0")) )
	  {
	    throw(AipsError("Internal error : MultiTerm Sumwt has a useweightimage=True but the weight image does not exist."));
	  }
      }
    else
      {
	throw( AipsError( "Multi-term SumWt does not exist. Please create PSFs or Residuals." ) );
      }

  }


  /////////////Constructor with pointers already created else where but taken over here
  SIImageStoreMultiTerm::SIImageStoreMultiTerm(Block<CountedPtr<ImageInterface<Float> > > modelims, 
					       Block<CountedPtr<ImageInterface<Float> > >residims,
					       Block<CountedPtr<ImageInterface<Float> > >psfims, 
					       Block<CountedPtr<ImageInterface<Float> > >weightims, 
					       Block<CountedPtr<ImageInterface<Float> > >restoredims,
					       Block<CountedPtr<ImageInterface<Float> > >sumwtims, 
					       CountedPtr<ImageInterface<Float> > newmask,
					       CountedPtr<ImageInterface<Float> > newalpha,
					       CountedPtr<ImageInterface<Float> > newbeta)
  {
    
    itsPsfs=psfims;
    itsModels=modelims;
    itsResiduals=residims;
    itsWeights=weightims;
    itsImages=restoredims;
    itsSumWts=sumwtims;
    itsMask = newmask;
    itsAlpha = newalpha;
    itsBeta = newbeta;

    itsNTerms = itsResiduals.nelements();
    itsMiscInfo=Record();

    AlwaysAssert( itsPsfs.nelements() == 2*itsNTerms-1 , AipsError ); 
    AlwaysAssert( itsPsfs.nelements()>0 && !itsPsfs[0].null() , AipsError );
    AlwaysAssert( itsSumWts.nelements()>0 && !itsSumWts[0].null() , AipsError );

    itsForwardGrids.resize( itsNTerms );
    itsBackwardGrids.resize( 2 * itsNTerms - 1 );

    itsImageShape=psfims[0]->shape();
    itsCoordSys = psfims[0]->coordinates();
    itsMiscInfo = psfims[0]->miscInfo();

    itsNFacets=sumwtims[0]->shape()[0];
    itsUseWeight=getUseWeightImage( *(sumwtims[0]) );

    itsImageName = String("reference");  // This is what the access functions use to guard against allocs...
	
    //    itsValidity = checkValidity(True/*psf*/, True/*res*/,False/*wgt*/,False/*model*/,False/*image*/);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  uInt SIImageStoreMultiTerm::getNTaylorTerms(Bool dopsf)
  {
    return dopsf ? (2*itsNTerms-1) : itsNTerms;
   }

  // Check if images that are asked-for are ready and all have the same shape.
  Bool SIImageStoreMultiTerm::checkValidity(const Bool ipsf, const Bool iresidual, 
					    const Bool iweight, const Bool imodel, const Bool irestored, 
					    const Bool imask,const Bool isumwt,
					    const Bool ialpha, const Bool ibeta)
  {

    //    cout << "In MT::checkValidity imask is " << imask << endl;

    Bool valid = True;

    for(uInt tix=0; tix<2*itsNTerms-1; tix++)
      {
	
	if(ipsf==True)
	  { psf(tix); 
	    valid = valid & ( !itsPsfs[tix].null() && itsPsfs[tix]->shape()==itsImageShape ); }
	if(iweight==True)
	  { weight(tix);  
	    valid = valid & ( !itsWeights[tix].null()&& itsWeights[tix]->shape()==itsImageShape ); }

	if(isumwt==True) {
	    IPosition useShape(itsImageShape);
	    useShape[0]=itsNFacets; useShape[1]=itsNFacets;
	    sumwt(tix);  
	    valid = valid & ( !itsSumWts[tix].null()&& itsSumWts[tix]->shape()==useShape ); 
	  }
	
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
    
    if(imask==True)
      { mask(); valid = valid & ( !itsMask.null() && itsMask->shape()==itsImageShape); 
	//	cout << " Mask.null() ? " << itsMask.null() << endl;
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

    for(uInt tix=0; tix<2*itsNTerms-1; tix++)
      {
	if( ! itsPsfs[tix].null() ) itsPsfs[tix]->unlock();
	if( ! itsWeights[tix].null() ) itsWeights[tix]->unlock();
	if( ! itsSumWts[tix].null() ) itsSumWts[tix]->unlock();
	if( tix < itsNTerms )
	  {
	    if( ! itsModels[tix].null() ) itsModels[tix]->unlock();
	    if( ! itsResiduals[tix].null() ) itsResiduals[tix]->unlock();
	    if( ! itsImages[tix].null() ) itsImages[tix]->unlock();
	  }
      }
    if( !itsMask.null() ) itsMask->unlock();
    if( !itsAlpha.null() ) itsAlpha->unlock();
    if( !itsBeta.null() ) itsBeta->unlock();
    
    return True; // do something more intelligent here.
  }

  Double SIImageStoreMultiTerm::getReferenceFrequency()
  {
    Double theRefFreq;

    Vector<Double> refpix = (itsCoordSys.spectralCoordinate()).referencePixel();
    AlwaysAssert( refpix.nelements()>0, AipsError );
    (itsCoordSys.spectralCoordinate()).toWorld( theRefFreq, refpix[0] );
    //    cout << "Reading ref freq as : " << theRefFreq << endl;
    return theRefFreq;
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

    return itsWeights[term];
  }
  CountedPtr<ImageInterface<Float> > SIImageStoreMultiTerm::sumwt(uInt term)
  {
    IPosition useShape( itsImageShape );
    useShape[0] = itsNFacets;
    useShape[1] = itsNFacets;

    if( !itsSumWts[term].null() && itsSumWts[term]->shape() == useShape ) { return itsSumWts[term]; }
    checkRef( itsSumWts[term], "sumwt.tt"+String::toString(term) );
    itsSumWts[term] = openImage( itsImageName+String(".sumwt.tt")+String::toString(term) , False, True/*dosumwt*/ ); 

    setUseWeightImage( *(itsSumWts[term]) , itsUseWeight); // Sets a flag in the SumWt image. 

    return itsSumWts[term];
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
    for(uInt tix=0;tix<2*itsNTerms-1;tix++)
      {
	if( resetpsf ) psf(tix)->set(0.0);
	if( resetweight && !itsWeights[tix].null() ) weight(tix)->set(0.0);

	if( tix < itsNTerms ) {
	    if( resetresidual ) residual(tix)->set(0.0);
	  }
      }
  }

  void SIImageStoreMultiTerm::addImages( CountedPtr<SIImageStore> imagestoadd,
				Bool addpsf, Bool addresidual, Bool addweight)
  {

    for(uInt tix=0;tix<2*itsNTerms-1;tix++)
      {
	
	if(addpsf)
	  {
	    LatticeExpr<Float> adderPsf( *(psf(tix)) + *(imagestoadd->psf(tix)) ); 
	    psf(tix)->copyData(adderPsf);	

	    //	    addSumWts( *psf(tix), *(imagestoadd->psf(tix)) );

	  }
	if(addweight)
	  {

	    if( itsUseWeight ) //getUseWeightImage( *(imagestoadd->psf(tix)) ) ) // Access and add weight only if it is needed.
	      {
		LatticeExpr<Float> adderWeight( *(weight(tix)) + *(imagestoadd->weight(tix)) ); 
		weight(tix)->copyData(adderWeight);
		
		//		addSumWts( *weight(tix), *(imagestoadd->weight(tix)) );
	      }

	    LatticeExpr<Float> adderSumWt( *(sumwt(tix)) + *(imagestoadd->sumwt(tix)) ); 
	    sumwt(tix)->copyData(adderSumWt);

	  }

	if(tix < itsNTerms && addresidual)
	  {
	    LatticeExpr<Float> adderRes( *(residual(tix)) + *(imagestoadd->residual(tix)) ); 
	    residual(tix)->copyData(adderRes);

	    //	    addSumWts( *residual(tix), *(imagestoadd->residual(tix)) );

	  }

      }
  }

  void SIImageStoreMultiTerm::dividePSFByWeight()
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","dividePSFByWeight",WHERE) );

    // Fill all sumwts by that of the first term.
    Matrix<Float> sumwt = getSumWt( *psf(0) );
    for(uInt tix=1;tix<2*itsNTerms-1;tix++)
      { setSumWt( *psf(tix) , sumwt ); }
    
    //    Bool useweightimage = getUseWeightImage( *psf() ) ;
    
    for(uInt tix=0;tix<2*itsNTerms-1;tix++)
      {
	divideImageByWeightVal( *psf(tix) );
	if( itsUseWeight ) { divideImageByWeightVal( *weight(tix) ); }
	//	if( useweightimage ) { divideImageByWeightVal( *weight(tix) ); }
	
	/*
	if( getUseWeightImage( *psf(tix) ) == True )
	  {
	    os << "Dividing " << itsImageName+String(".psf.tt")+String::toString(tix) << " by the weight image " << itsImageName+String(".weight.tt0") << LogIO::POST;
	    //	    cerr << "weight limit " <<  weightlimit << endl;
	    LatticeExpr<Float> mask( iif( (*(weight(0))) > weightlimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (*(weight(0))) > weightlimit , 0.0, 1.0 ) );
	    
	    LatticeExpr<Float> ratio( ( (*(psf(tix))) * mask ) / sqrt( (*(weight(0))) + maskinv) );
	    itsPsfs[tix]->copyData(ratio);
	  }
	*/
      }

    calcSensitivity();

    // createMask
  }


  // Make another for the PSF too.
  void SIImageStoreMultiTerm::divideResidualByWeight(Float pblimit, const String normtype)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","divideResidualByWeight",WHERE) );

    // Fill all sumwts by that of the first term.
    Matrix<Float> sumwt = getSumWt( *residual(0) );
    for(uInt tix=1;tix<itsNTerms;tix++)  { setSumWt( *residual(tix) , sumwt ); }

    Bool useweightimage = itsUseWeight; // getUseWeightImage( *residual(0) );

    if( useweightimage )
      {
	sumwt = getSumWt( *weight(0) );
	for(uInt tix=1;tix<itsNTerms;tix++) { setSumWt( *weight(tix) , sumwt ); }

	itsPBScaleFactor = getPbMax();

      }

    for(uInt tix=0;tix<itsNTerms;tix++)
      {

	divideImageByWeightVal( *residual(tix) );

	if( useweightimage == True )
	  {
	    
	    LatticeExpr<Float> deno;
	    if( normtype=="flatnoise"){
	      deno = LatticeExpr<Float> ( sqrt( abs(*(weight(0)) ) ) * itsPBScaleFactor );
	      os << "Dividing " << itsImageName+String(".residual.tt")+String::toString(tix) ;
	      os << " by [ sqrt(weightimage) * " << itsPBScaleFactor ;
	      os << " ] to get flat noise with unit pb peak."<< LogIO::POST;
	      
	    }
	    if( normtype=="flatsky") {
	      deno = LatticeExpr<Float> ( *(weight(0)) );
	      os << "Dividing " << itsImageName+String(".residual.tt")+String::toString(tix) ;
	      os << " by [ weight ] to get flat sky"<< LogIO::POST;
	    }
	    
	    LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
	    LatticeExpr<Float> ratio( ( (*(residual(tix))) * mask ) / ( deno + maskinv ) );

	/*	
	    os << "Dividing " << itsImageName+String(".residual.tt")+String::toString(tix) << " by the weight image " << itsImageName+String(".weight.tt0") << LogIO::POST;
	    
	    LatticeExpr<Float> mask( iif( (*(weight(0))) > weightlimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (*(weight(0))) > weightlimit , 0.0, 1.0 ) );
	    
	    LatticeExpr<Float> ratio( ( (*(residual(tix))) * mask ) / sqrt( (*(weight(0))) + maskinv) );
	*/

	    residual(tix)->copyData(ratio);
	  }
      }
    // createMask
  }

  /*
  void SIImageStoreMultiTerm::divideSensitivityPatternByWeight()
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","dividePSFByWeight",WHERE) );

    // Fill all sumwts by that of the first term.
    Matrix<Float> sumwt = getSumWt( *weight(0) );
    for(uInt tix=1;tix<2*itsNTerms-1;tix++)
      { setSumWt( *weight(tix) , sumwt ); }

    for(uInt tix=0;tix<2*itsNTerms-1;tix++)
      {
	divideImageByWeightVal( *weight(tix) );
      }
    // createMask
  }
  */

  void SIImageStoreMultiTerm::divideModelByWeight(Float pblimit, const String normtype)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","divideModelByWeight",WHERE) );

    if( //!itsResiduals[0].null() // information exists on whether weight image is needed or not
	itsUseWeight // only when needed
	//	&& getUseWeightImage( *residual(0) ) == True // only when needed
	&& hasSensitivity() )// i.e. only when possible. For an initial starting model, don't need wt anyway.
      {

	if( normtype=="flatsky") {
	  Array<Float> arrmod;
	  model(0)->get( arrmod, True );

	  os << "Model is already flat sky with peak flux : " << max(arrmod);
	  os << ". No need to divide before prediction" << LogIO::POST;
	  
	  return;
	  }

	  itsPBScaleFactor = getPbMax();

	for(uInt tix=0;tix<itsNTerms;tix++)
	  {
	    os << "Dividing " << itsImageName+String(".model")+String::toString(tix);
	    os << " by [ sqrt(weight) / " << itsPBScaleFactor ;
	    os <<" ] to get to flat sky model before prediction" << LogIO::POST;
	    
	    LatticeExpr<Float> deno( sqrt( abs(*(weight(0))) ) / itsPBScaleFactor );
	    
	    LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
	    LatticeExpr<Float> ratio( ( (*(model(tix))) * mask ) / ( deno + maskinv ) );


	    /*
	    os << "Dividing " << itsImageName+String(".model")+String::toString(tix) << " by the weight image " << itsImageName+String(".weight.tt0") << LogIO::POST;
	    
	    LatticeExpr<Float> mask( iif( (*(weight(0))) > pblimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (*(weight(0))) > pblimit , 0.0, 1.0 ) );
	    
	    LatticeExpr<Float> ratio( ( (*(model(tix))) * mask ) / sqrt( (*(weight(0))) + maskinv) );
	    */
	    itsModels[tix]->copyData(ratio);
	  }    
      }
    // createMask
  }


  void SIImageStoreMultiTerm::multiplyModelByWeight(Float pblimit, const String normtype)
  {
    LogIO os( LogOrigin("SIImageStoreMultiTerm","multiplyModelByWeight",WHERE) );

    if( //!itsResiduals[0].null() // information exists on whether weight image is needed or not
	//&& 
       itsUseWeight // only when needed
	//	&& getUseWeightImage( *residual(0) ) == True // only when needed
	&& hasSensitivity() )// i.e. only when possible. For an initial starting model, don't need wt anyway.
      {

	if( normtype=="flatsky") {
	  os << "Model is already flat sky. No need to multiply back after prediction" << LogIO::POST;
	  return;
	  }

	  itsPBScaleFactor = getPbMax();

	for(uInt tix=0;tix<itsNTerms;tix++)
	  {

	    os << "Multiplying " << itsImageName+String(".model")+String::toString(tix);
	  os << " by [ sqrt(weight) / " << itsPBScaleFactor;
	  os <<  " ] to take model back to flat noise with unit pb peak." << LogIO::POST;
	  
	  LatticeExpr<Float> deno( sqrt( abs(*(weight(0)) ) ) / itsPBScaleFactor );

	  LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
	  LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
	  LatticeExpr<Float> ratio( ( (*(model(tix))) * mask ) * ( deno + maskinv ) );

	  /*
	    os << "Multiplying " << itsImageName+String(".model")+String::toString(tix) << " by the weight image " << itsImageName+String(".weight.tt0") << LogIO::POST;
	    
	    LatticeExpr<Float> mask( iif( (*(weight(0))) > pblimit , 1.0, 0.0 ) );
	    LatticeExpr<Float> maskinv( iif( (*(weight(0))) > pblimit , 0.0, 1.0 ) );
	    
	    LatticeExpr<Float> ratio( ( (*(model(tix))) * mask ) * sqrt( (*(weight(0))) + maskinv) );
	  */
	    itsModels[tix]->copyData(ratio);
	  }    
      }
    // createMask
  }




  GaussianBeam SIImageStoreMultiTerm::restorePlane()
  {

    LogIO os( LogOrigin("SIImageStoreMultiTerm","restorePlane",WHERE) );

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
	    os << "Restore with beam : " 
	       << beam.getMajor(Unit("arcmin")) << " arcmin, " 
	       << beam.getMinor(Unit("arcmin"))<< " arcmin, " 
	       << beam.getPA(Unit("deg")) << " deg" << LogIO::POST; 
	    os << "With " << itsNTerms << " Taylor coefficient(s), spectral index " 
	       << ((itsNTerms>2)?String("and curvature"):String("")) << " will "
	       << ((itsNTerms>1)?String(""):String("not")) << " be computed." << LogIO::POST;
	    
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
	    //ii.setRestoringBeam( beam );
	    ii.setBeams( beam );
	    
	    for(uInt tix=0; tix<itsNTerms; tix++)
	      {
		(image(tix))->set(0.0);
		(image(tix))->copyData( LatticeExpr<Float>(*(model(tix))) );
		StokesImageUtil::Convolve( *(image(tix)) , beam);
		(image(tix))->copyData( LatticeExpr<Float>
					( *(image(tix)) + *(residual(tix)) )   );
		//image()->setImageInfo(ii); // no use.... its a reference subimage.
	      }	
	    
	    if( itsNTerms > 1)
	      {
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
	    
	  }
      }
    catch(AipsError &x)
      {
	throw( AipsError("Multi-Term Restoration Error : " + x.getMesg() ) );
      }
    return beam;
  }

  Bool SIImageStoreMultiTerm::createMask(LatticeExpr<Bool> &lemask, 
					 CountedPtr<ImageInterface<Float> > outimage)
{
  ImageRegion outreg = outimage->makeMask("mask0",False,True);
  //ImageRegion outreg = ((PagedImage<Float> *)(&*outimage))->makeMask("mask0",False,True);
  LCRegion& outmask=outreg.asMask();
  outmask.copyData(lemask);
  outimage->defineRegion("mask0",outreg, RegionHandler::Masks, True);
  outimage->setDefaultMask("mask0");
  return True;
}

  void SIImageStoreMultiTerm::pbcorPlane()
  {

    LogIO os( LogOrigin("SIImageStoreMultiTerm","pbcorPlane",WHERE) );

  }

  void SIImageStoreMultiTerm::calcSensitivity()
  {
    LogIO os( LogOrigin("SIImageStore","calcSensitivity",WHERE) );

    // Construct Hessian.
    
    Matrix<Float> hess(IPosition(2,itsNTerms,itsNTerms));
    for(uInt tay1=0;tay1<itsNTerms;tay1++)
      for(uInt tay2=0;tay2<itsNTerms;tay2++)
	{
	  uInt taymin = (tay1<=tay2)? tay1 : tay2;
	  uInt taymax = (tay1>=tay2)? tay1 : tay2;
	  uInt ind = (taymax*(taymax+1)/2)+taymin;
	  AlwaysAssert( ind < 2*itsNTerms-1, AipsError );

	  Array<Float> lsumwt;
	  sumwt( ind )->get( lsumwt, True );
	  AlwaysAssert( lsumwt.shape().nelements()==1, AipsError );
	  AlwaysAssert( lsumwt.shape()[0]>0, AipsError );

	  hess(tay1,tay2) = lsumwt(IPosition(1,0));
	}

    os << "Multi-Term Hessian Matrix : " << hess << LogIO::POST;

    // Invert Hessian. 
    try
      {
	Float deter=0.0;
	Matrix<Float> invhess;
	invertSymPosDef(invhess,deter,hess);
	os << "Multi-Term Covariance Matrix : " << invhess << LogIO::POST;

	// Just print the sqrt of the diagonal elements. 
	
	for(uInt tix=0;tix<itsNTerms;tix++)
	  {
	    os << "[" << itsImageName << "][Taylor"<< tix << "] Theoretical sensitivity (Jy/bm):" ;
	    if( invhess(tix,tix) > 0.0 ) { os << sqrt(invhess(tix,tix)) << LogIO::POST; }
	    else { os << "none" << LogIO::POST; }
	  }
      }
    catch(AipsError &x)
      {
	os << LogIO::WARN << "Cannot invert Hessian Matrix : " << x.getMesg()  << " || Calculating approximate sensitivity " << LogIO::POST;
	
	// Approximate : 1/h^2
	for(uInt tix=0;tix<itsNTerms;tix++)
	  {
	    Array<Float> lsumwt;
	    AlwaysAssert( 2*tix < 2*itsNTerms-1, AipsError );
	    sumwt(2*tix)->get( lsumwt , False ); 
	    
	    IPosition shp( lsumwt.shape() );
	    //cout << "Sumwt shape : " << shp << " : " << lsumwt << endl;
	    //AlwaysAssert( shp.nelements()==4 , AipsError );
	    
	    os << "[" << itsImageName << "][Taylor"<< tix << "] Approx Theoretical sensitivity (Jy/bm):" ;
	    
	    IPosition it(4,0,0,0,0);
	    for ( it[0]=0; it[0]<shp[0]; it[0]++)
	      for ( it[1]=0; it[1]<shp[1]; it[1]++)
		for ( it[2]=0; it[2]<shp[2]; it[2]++)
		  for ( it[3]=0; it[3]<shp[3]; it[3]++)
		    {
		      if( shp[0]>1 ){os << "f"<< it[0]+(it[1]*shp[0]) << ":" ;}
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
	  }
      }
  }
  

  
  
  CountedPtr<SIImageStore> SIImageStoreMultiTerm::getFacetImageStore(const Int facet, const Int nfacets)
  {
    Block<CountedPtr<ImageInterface<Float> > > psflist(2*itsNTerms-1);
    Block<CountedPtr<ImageInterface<Float> > > modellist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > residuallist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > weightlist(2*itsNTerms-1);
    Block<CountedPtr<ImageInterface<Float> > > imagelist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > sumwtlist(2*itsNTerms-1);
    CountedPtr<ImageInterface<Float> > newmask;
    CountedPtr<ImageInterface<Float> > newalpha;
    CountedPtr<ImageInterface<Float> > newbeta;

    for(uInt tix=0; tix<2*itsNTerms-1;tix++)
      {
	psflist[tix] = itsPsfs[tix].null()?NULL:makeFacet(facet, nfacets, *itsPsfs[tix]);	
	weightlist[tix] = itsWeights[tix].null()?NULL:makeFacet(facet, nfacets, *itsWeights[tix]);
	sumwtlist[tix] = itsSumWts[tix].null()?NULL:makeFacet(facet, nfacets, *itsSumWts[tix]);

	if( tix<itsNTerms )
	  {
	    modellist[tix] = itsModels[tix].null()?NULL:makeFacet(facet, nfacets, *itsModels[tix]);
	    residuallist[tix] = itsResiduals[tix].null()?NULL:makeFacet(facet, nfacets, *itsResiduals[tix]);
	    imagelist[tix] = itsImages[tix].null()?NULL:makeFacet(facet, nfacets, *itsImages[tix]);
	  }
      }
    newmask = itsMask.null()?NULL:makeFacet(facet, nfacets, *itsMask);
    newalpha = itsAlpha.null()?NULL:makeFacet(facet, nfacets, *itsAlpha);
    newbeta = itsBeta.null()?NULL:makeFacet(facet, nfacets, *itsBeta);

    return new SIImageStoreMultiTerm(modellist, residuallist, psflist, weightlist, imagelist,sumwtlist, newmask,newalpha,newbeta);

  }

  CountedPtr<SIImageStore> SIImageStoreMultiTerm::getSubImageStore(const Int chan, const Bool onechan,
							  const Int pol, const Bool onepol)
  {

    ///    cout << "in MT subImStor : " << chan << " " << onechan << " " << pol << " " << onepol << endl;

    if( !onechan && !onepol ) {return this;}    // No slicing is required. 

    Block<CountedPtr<ImageInterface<Float> > > psflist(2*itsNTerms-1);
    Block<CountedPtr<ImageInterface<Float> > > modellist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > residuallist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > weightlist(2*itsNTerms-1);
    Block<CountedPtr<ImageInterface<Float> > > imagelist(itsNTerms);
    Block<CountedPtr<ImageInterface<Float> > > sumwtlist(2*itsNTerms-1);
    CountedPtr<ImageInterface<Float> > newmask;
    CountedPtr<ImageInterface<Float> > newalpha;
    CountedPtr<ImageInterface<Float> > newbeta;

    for(uInt tix=0; tix<2*itsNTerms-1;tix++)
      {
	psflist[tix] = itsPsfs[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsPsfs[tix]);
	weightlist[tix] = itsWeights[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsWeights[tix]);
	sumwtlist[tix] = itsSumWts[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsSumWts[tix]);
	
	if( tix<itsNTerms )
	  {
	    modellist[tix] = itsModels[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsModels[tix]);
	    residuallist[tix] = itsResiduals[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsResiduals[tix]);
	    imagelist[tix] = itsImages[tix].null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsImages[tix]);
	  }
      }
    newmask = itsMask.null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsMask);
    newalpha = itsAlpha.null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsAlpha);
    newbeta = itsBeta.null()?NULL:makePlane( chan,onechan,pol,onepol,  *itsBeta);
    
    return new SIImageStoreMultiTerm(modellist, residuallist, psflist, weightlist, imagelist, sumwtlist, newmask, newalpha,newbeta);

  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END

