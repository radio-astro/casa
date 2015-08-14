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
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>
#include <images/Images/ImageRegrid.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

  //
  //===========================================================================
  // Global method that I (SB) could make work in SynthesisUtilsMethods.
  //
  template <class T>
  void openImage(const String& imagenamefull,SHARED_PTR<ImageInterface<T> >& imPtr )
  {
    LogIO logIO ( LogOrigin("SynthesisImager","openImage(name)") );
    try
      {
	if (Table::isReadable(imagenamefull))
	  imPtr.reset( new PagedImage<T>( imagenamefull ) );
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
  void openImage(const String& imagenamefull, SHARED_PTR<ImageInterface<Float> >& img );
  template 
  void openImage(const String& imagenamefull, SHARED_PTR<ImageInterface<Complex> >& img );
  //
  //===========================================================================


  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////       START of SIIMAGESTORE implementation
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  
  SIImageStore::SIImageStore() 
  {
    itsPsf.reset( );
    itsModel.reset( );
    itsResidual.reset( );
    itsWeight.reset( );
    itsImage.reset( );
    itsMask.reset( );
    itsGridWt.reset( );
    itsPB.reset( );

    itsSumWt.reset( );
    itsUseWeight=False;
    itsPBScaleFactor=1.0;

    itsNFacets=1;
    itsFacetId=0;
    itsNChanChunks = 1;
    itsChanId = 0;
    itsNPolChunks = 1;
    itsPolId = 0;

    itsImageShape=IPosition(4,0,0,0,0);
    itsImageName=String("");
    itsCoordSys=CoordinateSystem();
    itsMiscInfo=Record();
    init();
    
    //    validate();

  }

  SIImageStore::SIImageStore(String imagename, 
			     CoordinateSystem &imcoordsys, 
			     IPosition imshape, 
			     //			     const Int nfacets, 
			     const Bool /*overwrite*/,
			     const Bool useweightimage)
  // TODO : Add parameter to indicate weight image shape. 
  {
    LogIO os( LogOrigin("SIImageStore","Open new Images",WHERE) );

    itsPsf.reset( );
    itsModel.reset( );
    itsResidual.reset( );
    itsWeight.reset( );
    itsImage.reset( );
    itsMask.reset( );
    itsGridWt.reset( );
    itsPB.reset( );

    itsSumWt.reset( );
    itsUseWeight=useweightimage;
    itsPBScaleFactor=1.0;

    itsNFacets=1;
    itsFacetId=0;
    itsNChanChunks = 1;
    itsChanId = 0;
    itsNPolChunks = 1;
    itsPolId = 0;

    itsImageName = imagename;
    itsImageShape = imshape;
    itsCoordSys = imcoordsys;

    itsMiscInfo=Record();

    init();

    validate();
  }

  SIImageStore::SIImageStore(String imagename,const Bool ignorefacets) 
  {
    LogIO os( LogOrigin("SIImageStore","Open existing Images",WHERE) );

    /*
    init();
    String fname( imagename + ".info" );
    recreate( fname );
    */

   
    itsPsf.reset( );
    itsModel.reset( );
    itsResidual.reset( );
    itsWeight.reset( );   
    itsImage.reset( );
    itsMask.reset( );
    itsGridWt.reset( );
    itsPB.reset( );
    itsMiscInfo=Record();

    itsSumWt.reset( );
    itsNFacets=1;
    itsFacetId=0;
    itsNChanChunks = 1;
    itsChanId = 0;
    itsNPolChunks = 1;
    itsPolId = 0;

    itsImageName = imagename;

    // The PSF or Residual images must exist. ( TODO : and weight )
    if( doesImageExist(itsImageName+String(".residual")) || 
	doesImageExist(itsImageName+String(".psf")) ||
	doesImageExist(itsImageName+String(".gridwt"))  )
      {
	SHARED_PTR<ImageInterface<Float> > imptr;
	if( doesImageExist(itsImageName+String(".psf")) )
	  imptr.reset( new PagedImage<Float> (itsImageName+String(".psf")) );
	else if ( doesImageExist(itsImageName+String(".residual")) )
	  imptr.reset( new PagedImage<Float> (itsImageName+String(".residual")) );
	else 
	  imptr.reset( new PagedImage<Float> (itsImageName+String(".gridwt")) );
	  
	itsImageShape = imptr->shape();
	itsCoordSys = imptr->coordinates();
      }
    else
      {
	throw( AipsError( "PSF or Residual Image (or sumwt) do not exist. Please create one of them." ) );
      }
    
    if( doesImageExist(itsImageName+String(".residual")) || 
	doesImageExist(itsImageName+String(".psf")) )
      {

	
    if( doesImageExist(itsImageName+String(".sumwt"))  )
      {
	SHARED_PTR<ImageInterface<Float> > imptr;
	imptr.reset( new PagedImage<Float> (itsImageName+String(".sumwt")) );
	itsNFacets = imptr->shape()[0];
	itsFacetId = 0;
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

      }// if psf or residual exist...

    if( ignorefacets==True ) itsNFacets= 1;

    init();
    
    validate();
  }

  SIImageStore::SIImageStore(SHARED_PTR<ImageInterface<Float> > modelim, 
			     SHARED_PTR<ImageInterface<Float> > residim,
			     SHARED_PTR<ImageInterface<Float> > psfim, 
			     SHARED_PTR<ImageInterface<Float> > weightim, 
			     SHARED_PTR<ImageInterface<Float> > restoredim, 
			     SHARED_PTR<ImageInterface<Float> > maskim,
			     SHARED_PTR<ImageInterface<Float> > sumwtim,
			     CoordinateSystem& csys,
			     IPosition imshape,
			     String imagename,
			     const Int facet, const Int nfacets,
			     const Int chan, const Int nchanchunks,
			     const Int pol, const Int npolchunks)
  {

    itsPsf=psfim;
    itsModel=modelim;
    itsResidual=residim;
    itsWeight=weightim;
    itsImage=restoredim;
    itsMask=maskim;

    itsSumWt=sumwtim;

    itsPBScaleFactor=1.0;// No need to set properly here as it will be computed in makePSF.

    itsNFacets = nfacets;
    itsFacetId = facet;
    itsNChanChunks = nchanchunks;
    itsChanId = chan;
    itsNPolChunks = npolchunks;
    itsPolId = pol;

    itsParentImageShape = imshape; 
    itsImageShape = imshape;

    itsCoordSys = csys; // Hopefully this doesn't change for a reference image
    itsImageName = imagename;

    //-----------------------
    init(); // Connect parent pointers to the images.
    //-----------------------

    // Set these to null, to be set later upon first access.
    itsPsf.reset( );
    itsModel.reset( );
    itsResidual.reset( );
    itsWeight.reset( );
    itsImage.reset( );
    itsMask.reset( );
    itsSumWt.reset( );


    validate();
  }
  
   void SIImageStore::validate()
  {
    /// There are two valid states. Check for at least one of them. 
    Bool state = False;
    
    stringstream oss;
    oss << "shape:" << itsImageShape << " parentimageshape:" << itsParentImageShape 
	<< " nfacets:" << itsNFacets << "x" << itsNFacets << " facetid:" << itsFacetId 
	<< " nchanchunks:" << itsNChanChunks << " chanid:" << itsChanId 
	<< " npolchunks:" << itsNPolChunks << " polid:" << itsPolId 
	<< " coord-dim:" << itsCoordSys.nPixelAxes() 
	<< " psf/res:" << (hasPsf() || hasResidual()) ;
    if( hasSumWt() ) oss << " sumwtshape : " << sumwt()->shape() ; 
	oss << endl;


    try {

    if( itsCoordSys.nPixelAxes() != 4 ) state=False;
    
    /// (1) Regular imagestore 
    if( itsNFacets==1 && itsFacetId==0 
	&& itsNChanChunks==1 && itsChanId==0
	&& itsNPolChunks==1 && itsPolId==0 )  {
      Bool check1 = hasSumWt() && sumwt()->shape()[0]==1;
      if(  (itsImageShape.isEqual(itsParentImageShape) ) && ( check1 || !hasSumWt() )
	   && itsParentImageShape.product() > 0 ) state=True;
      }
    /// (2) Reference Sub Imagestore 
    else if ( ( itsNFacets>1 && itsFacetId >=0 )
	      || ( itsNChanChunks>1 && itsChanId >=0 ) 
	      || ( itsNPolChunks>1 && itsPolId >=0 )   ) {
      // If shape is still unset, even when the first image has been made....
      Bool check1 = ( itsImageShape.product() > 0 && ( hasPsf() || hasResidual() ) );
      Bool check2 = ( itsImageShape.isEqual(IPosition(4,0,0,0,0)) && ( !hasPsf() && !hasResidual() ) );
      Bool check3 = hasSumWt() && sumwt()->shape()[0]==1; // One facet only.

      if( ( check1 || check2 ) && ( itsParentImageShape.product()>0 ) 
	  && ( itsFacetId < itsNFacets*itsNFacets ) 
	  && ( itsChanId < itsNChanChunks ) && ( itsPolId < itsNPolChunks ) 
	  && ( check3 || !hasSumWt() ) )  state = True;
    }

    } catch( AipsError &x )  {
      state = False;
      oss << "  |||||  " << x.getMesg() << endl;
    }

    //cout << " SIIM:validate : " << oss.str() << endl;

    if( state == False )  throw(AipsError("Internal Error : Invalid ImageStore state : "+ oss.str()) );
    
    return;
  }


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  SHARED_PTR<SIImageStore> SIImageStore::getSubImageStore(const Int facet, const Int nfacets, 
							  const Int chan, const Int nchanchunks, 
							  const Int pol, const Int npolchunks)
  {
    return SHARED_PTR<SIImageStore>(new SIImageStore(itsModel, itsResidual, itsPsf, itsWeight, itsImage, itsMask, itsSumWt, itsCoordSys,itsImageShape, itsImageName, facet, nfacets,chan,nchanchunks,pol,npolchunks));
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //// Either read an image from disk, or construct one. 

  SHARED_PTR<ImageInterface<Float> > SIImageStore::openImage(const String imagenamefull, 
							     const Bool overwrite, 
							     const Bool dosumwt, const Int nfacetsperside)
  {

    SHARED_PTR<ImageInterface<Float> > imPtr;

    IPosition useShape( itsParentImageShape );

    if( dosumwt ) // change shape to sumwt image shape.
      {
	useShape[0] = nfacetsperside;
	useShape[1] = nfacetsperside;
	//	cout << "openImage : Making sumwt grid : using shape : " << useShape << endl;
      }

    if( overwrite || !Table::isWritable( imagenamefull ) )
      {
	imPtr.reset( new PagedImage<Float> (useShape, itsCoordSys, imagenamefull) );
	// initialize to zeros...
	imPtr->set(0.0);
      }
    else
      {
	if(Table::isWritable( imagenamefull ))
	  {
	    //cerr << "Trying to open "<< imagenamefull << endl;
	    try{
	      imPtr.reset( new PagedImage<Float>( imagenamefull ) );
	    }
	    catch (AipsError &x){
	      cerr << "Writable table exists, but cannot open. Creating temp image. : " << x.getMesg() << endl;
	      imPtr.reset( new TempImage<Float> (useShape, itsCoordSys) );
	      //  imPtr=new PagedImage<Float> (useShape, itsCoordSys, imagenamefull);
	      imPtr->set(0.0);
	    }
	  }
	else
	  {
	    cerr << "Table " << imagenamefull << " is not writeable. Creating temp image." << endl;
	    imPtr.reset( new TempImage<Float> (useShape, itsCoordSys) );
	    imPtr->set(0.0);
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

  SHARED_PTR<ImageInterface<Float> > SIImageStore::makeSubImage(const Int facet, const Int nfacets,
								const Int chan, const Int nchanchunks,
								const Int pol, const Int npolchunks,
								ImageInterface<Float>& image)
  {
    //assuming n x n facets
    Int nx_facets=Int(sqrt(Double(nfacets)));
    LogIO os( LogOrigin("SynthesisImager","makeFacet") );
     // Make the output image
    Slicer imageSlicer;

    // Add checks for all dimensions..
    if((facet>(nfacets-1))||(facet<0)) {
      os << LogIO::SEVERE << "Illegal facet " << facet << LogIO::POST;
      return SHARED_PTR<ImageInterface<Float> >();
    }
    IPosition imshp=image.shape();
    IPosition blc(imshp.nelements(), 0);
    IPosition trc=imshp-1;
    IPosition inc(imshp.nelements(), 1);

    /// Facets
    Int facetx = facet % nx_facets; 
    Int facety = (facet - facetx) / nx_facets;
    Int sizex = imshp(0) / nx_facets;
    Int sizey = imshp(1) / nx_facets;
    blc(1) = facety * sizey; 
    trc(1) = blc(1) + sizey - 1;
    blc(0) = facetx * sizex; 
    trc(0) = blc(0) + sizex - 1;

    /// Pol Chunks
    Int sizepol = imshp(2) / npolchunks;
    blc(2) = pol * sizepol;
    trc(2) = blc(2) + sizepol - 1;

    /// Chan Chunks
    Int sizechan = imshp(3) / nchanchunks;
    blc(3) = chan * sizechan;
    trc(3) = blc(3) + sizechan - 1;

    LCBox::verify(blc, trc, inc, imshp);
    Slicer imslice(blc, trc, inc, Slicer::endIsLast);

    // Now create the sub image
    SHARED_PTR<ImageInterface<Float> >  referenceImage( new SubImage<Float>(image, imslice, True) );
    referenceImage->setMiscInfo(image.miscInfo());
    referenceImage->setUnits(image.units());

    // cout << "Made Ref subimage of shape : " << referenceImage->shape() << endl;

    return referenceImage;
    
  }



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

    if( itsPsf ) releaseImage( itsPsf );
    if( itsModel ) releaseImage( itsModel );
    if( itsResidual ) releaseImage( itsResidual );
    if( itsImage ) releaseImage( itsImage );
    if( itsWeight ) releaseImage( itsWeight );
    if( itsMask ) releaseImage( itsMask );
    if( itsSumWt ) releaseImage( itsSumWt );
    if( itsGridWt ) releaseImage( itsGridWt );
    if( itsPB ) releaseImage( itsPB );

    return True; // do something more intelligent here.
  }

  void SIImageStore::releaseImage( SHARED_PTR<ImageInterface<Float> > im )
  {
    im->unlock();
    im->tempClose();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIImageStore::setModelImage( String modelname )
  {
    LogIO os( LogOrigin("SIImageStore","setModelImage",WHERE) );

    Directory immodel( modelname  +String(".model") );
    if( !immodel.exists() ) 
      {
	os << "Starting model image does not exist. No initial prediction will be done" << LogIO::POST;
	return;
      }

    SHARED_PTR<PagedImage<Float> > newmodel( new PagedImage<Float>( modelname +String(".model") ) );
    // Check shapes, coordsys with those of other images.  If different, try to re-grid here.

    if( (newmodel->shape() != model()->shape()) ||  (! itsCoordSys.near(newmodel->coordinates() )) )
      {
	os << "Regridding input model to target coordinate system" << LogIO::POST;
	regridToModelImage( *newmodel );

	// For now, throw an exception.
	//throw( AipsError( "Input model image "+modelname+".model is not the same shape as that defined for output in "+ itsImageName + ".model" ) );
      }
    else
      {
	os << "Setting " << modelname << " as model " << LogIO::POST;
	// Then, add its contents to itsModel.
	//itsModel->put( itsModel->get() + model->get() );
	itsModel->put( newmodel->get() );
      }
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

  /*
  void SIImageStore::checkRef( SHARED_PTR<ImageInterface<Float> > ptr, const String label )
  {
    if( ! ptr && itsImageName==String("reference") ) 
      {throw(AipsError("Internal Error : Attempt to access null subImageStore "+label + " by reference."));}
  }
  */

  void SIImageStore::accessImage( SHARED_PTR<ImageInterface<Float> > &ptr, 
		    SHARED_PTR<ImageInterface<Float> > &parentptr, 
		    const String label )
  {
    // if ptr is not null, assume it's OK. Perhaps add more checks.

    Bool sw=False;
    if( label.contains(imageExts(SUMWT)) ) sw=True;
    
    if( !ptr )
      {
	//cout << itsNFacets << " " << itsNChanChunks << " " << itsNPolChunks << endl;
	if( itsNFacets>1 || itsNChanChunks>1 || itsNPolChunks>1 )
	  {
	    if( ! parentptr ) 
	      {
		//cout << "Making parent : " << label << "    sw : " << sw << endl; 
		parentptr = openImage(itsImageName+label , False, sw, itsNFacets );  
	      }
	    //cout << "Making facet " << itsFacetId << " out of " << itsNFacets << endl;
	    //ptr = makeFacet( itsFacetId, itsNFacets*itsNFacets, *parentptr );
	    ptr = makeSubImage( itsFacetId, itsNFacets*itsNFacets, 
				itsChanId, itsNChanChunks, 
				itsPolId, itsNPolChunks, 
				*parentptr );
	    if( ! sw )
	      {
		itsImageShape = ptr->shape(); // over and over again.... FIX.
		itsCoordSys = ptr->coordinates();
		itsMiscInfo = ptr->miscInfo();
	      }

	    //cout << "accessImage : " << label << " : sumwt : " << sw << " : shape : " << itsImageShape << endl;
    
	  }
	else
	  {
	    ptr = openImage(itsImageName+label , False, sw, 1 ); 
	    //cout << "Opening image : " << itsImageName+label << " of shape " << ptr->shape() << endl;
	  }
      }
    
  }


  SHARED_PTR<ImageInterface<Float> > SIImageStore::psf(uInt /*nterm*/)
  {
    accessImage( itsPsf, itsParentPsf, imageExts(PSF) );
    return itsPsf;
  }
  SHARED_PTR<ImageInterface<Float> > SIImageStore::residual(uInt /*nterm*/)
  {
    accessImage( itsResidual, itsParentResidual, imageExts(RESIDUAL) );
    return itsResidual;
  }
  SHARED_PTR<ImageInterface<Float> > SIImageStore::weight(uInt /*nterm*/)
  {
    accessImage( itsWeight, itsParentWeight, imageExts(WEIGHT) );
    return itsWeight;
  }

  SHARED_PTR<ImageInterface<Float> > SIImageStore::sumwt(uInt /*nterm*/)
  {

    accessImage( itsSumWt, itsParentSumWt, imageExts(SUMWT) );
    
    if( itsNFacets>1 || itsNChanChunks>1 || itsNPolChunks>1 ) 
      { itsUseWeight = getUseWeightImage( *itsParentSumWt );}
    setUseWeightImage( *itsSumWt , itsUseWeight); // Sets a flag in the SumWt image. 
    
    return itsSumWt;
  }

  SHARED_PTR<ImageInterface<Float> > SIImageStore::model(uInt /*nterm*/)
  {
    accessImage( itsModel, itsParentModel, imageExts(MODEL) );

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
  SHARED_PTR<ImageInterface<Float> > SIImageStore::image(uInt /*nterm*/)
  {
    accessImage( itsImage, itsParentImage, imageExts(IMAGE) );

    itsImage->setUnits("Jy/beam");
    return itsImage;
  }

  SHARED_PTR<ImageInterface<Float> > SIImageStore::mask(uInt /*nterm*/)
  {
    accessImage( itsMask, itsParentMask, imageExts(MASK) );
    return itsMask;
  }
  SHARED_PTR<ImageInterface<Float> > SIImageStore::gridwt(uInt /*nterm*/)
  {
    accessImage( itsGridWt, itsParentGridWt, imageExts(GRIDWT) );
    /// change the coordinate system here, to uv.
    return itsGridWt;
  }
  SHARED_PTR<ImageInterface<Float> > SIImageStore::pb(uInt /*nterm*/)
  {
    accessImage( itsPB, itsParentPB, imageExts(PB) );
    /// change the coordinate system here, to uv.
    return itsPB;
  }

  SHARED_PTR<ImageInterface<Complex> > SIImageStore::forwardGrid(uInt /*nterm*/){
    if( itsForwardGrid ) // && (itsForwardGrid->shape() == itsImageShape))
      {
	//	cout << "Forward grid has shape : " << itsForwardGrid->shape() << endl;
	return itsForwardGrid;
      }
    Vector<Int> whichStokes(0);
    IPosition cimageShape;
    cimageShape=itsImageShape;
    CoordinateSystem cimageCoord = StokesImageUtil::CStokesCoord( itsCoordSys,
								  whichStokes, itsDataPolRep);
    cimageShape(2)=whichStokes.nelements();
    //cout << "Making forward grid of shape : " << cimageShape << " for imshape : " << itsImageShape << endl;
    itsForwardGrid.reset( new TempImage<Complex>(TiledShape(cimageShape, tileShape()), cimageCoord, memoryBeforeLattice()) );

    return itsForwardGrid;
  }

  SHARED_PTR<ImageInterface<Complex> > SIImageStore::backwardGrid(uInt /*nterm*/){
    if( itsBackwardGrid ) //&& (itsBackwardGrid->shape() == itsImageShape))
      {
	//	cout << "Backward grid has shape : " << itsBackwardGrid->shape() << endl;
	return itsBackwardGrid;
      }
    Vector<Int> whichStokes(0);
    IPosition cimageShape;
    cimageShape=itsImageShape;
    CoordinateSystem cimageCoord = StokesImageUtil::CStokesCoord( itsCoordSys,
								  whichStokes, itsDataPolRep);
    cimageShape(2)=whichStokes.nelements();
    //cout << "Making backward grid of shape : " << cimageShape << " for imshape : " << itsImageShape << endl;
    itsBackwardGrid.reset( new TempImage<Complex>(TiledShape(cimageShape, tileShape()), cimageCoord, memoryBeforeLattice()) );
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
  Bool SIImageStore::doesImageExist(String imagename)
  {
    LogIO os( LogOrigin("SIImageStore","doesImageExist",WHERE) );
    Directory image( imagename );
    return image.exists();
  }


  void SIImageStore::resetImages( Bool resetpsf, Bool resetresidual, Bool resetweight )
  {
    if( resetpsf ) psf()->set(0.0);
    if( resetresidual ) residual()->set(0.0);
    if( resetweight && itsWeight ) weight()->set(0.0);
    if( resetweight ) sumwt()->set(0.0);
  }

  void SIImageStore::addImages( SHARED_PTR<SIImageStore> imagestoadd,
				Bool addpsf, Bool addresidual, Bool addweight, Bool adddensity)
  {

    if(addpsf)
      {
	LatticeExpr<Float> adderPsf( *(psf()) + *(imagestoadd->psf()) ); 
	psf()->copyData(adderPsf);
      }
    if(addresidual)
      {
	LatticeExpr<Float> adderRes( *(residual()) + *(imagestoadd->residual()) ); 
	residual()->copyData(adderRes);
      }
    if(addweight)
      {
	if( getUseWeightImage( *(imagestoadd->sumwt()) ) ) // Access and add weight only if it is needed.
	  {
	    LatticeExpr<Float> adderWeight( *(weight()) + *(imagestoadd->weight()) ); 
	    weight()->copyData(adderWeight);
	  }

	/*
	Array<Float> qqq, www;
	imagestoadd->sumwt()->get(qqq,True);
	sumwt()->get(www,True);
	cout << "SUMWT : Adding : " << qqq << " to " << www << endl;
	*/

	LatticeExpr<Float> adderSumWt( *(sumwt()) + *(imagestoadd->sumwt()) ); 
	sumwt()->copyData(adderSumWt);
	setUseWeightImage( *sumwt(),  getUseWeightImage(*(imagestoadd->sumwt()) ) );

      }
    if(adddensity)
      {
	LatticeExpr<Float> adderDensity( *(gridwt()) + *(imagestoadd->gridwt()) ); 
	gridwt()->copyData(adderDensity);
      }

  }

void SIImageStore::setWeightDensity( SHARED_PTR<SIImageStore> imagetoset )
  {
    LogIO os( LogOrigin("SIImageStore","setWeightDensity",WHERE) );

    gridwt()->copyData( LatticeExpr<Float> ( *(imagetoset->gridwt()) ) );

  }

  // TODO
  //    cout << "WARN :   get getPbMax right for cube.... weight is indexed on chan and pol." << endl;
  Double SIImageStore::getPbMax()
  {

    //// Don't do any extra norm. Minor cycle will operate with native PB.
    //return 1.0;

    //// Normalize PB to 1 at the center of the image OF CHANNEL ZERO
    
    //        IPosition shp = weight(0)->shape();
    //     IPosition center(4, shp[0]/2, shp[1]/2,0,0);
    //    return sqrt(   weight(0)->getAt( center )   );
    

    //// Normalize PB to 1 at the location of the maximum (across all chans..)
    
    LatticeExprNode le( sqrt(max( *(weight(0)) )) );
    return le.getFloat();
    
  }


  Double SIImageStore::getPbMax(Int pol,Int chan)
  {

    //// Normalize PB to 1 at the location of the maximum (per pol,chan)
    
    CountedPtr<ImageInterface<Float> > subim=makeSubImage(0,1, 
							  chan, itsImageShape[3],
							  pol, itsImageShape[2], 
							  *weight(0) );

    LatticeExprNode le( sqrt(max( *subim )) );
    return le.getFloat();
  }

  void  SIImageStore::makePBFromWeight(const Float pblimit)
  {
   LogIO os( LogOrigin("SIImageStore","makePBFromWeight",WHERE) );

    	for(Int pol=0; pol<itsImageShape[2]; pol++)
	  {
	       for(Int chan=0; chan<itsImageShape[3]; chan++)
	      {

		itsPBScaleFactor = getPbMax(pol,chan);
		
		if(itsPBScaleFactor<=0){os << LogIO::NORMAL1 << "Skipping normalization for C:" << chan << " P:" << pol << " because pb max is zero " << LogIO::POST;}
		else {

		  CountedPtr<ImageInterface<Float> > wtsubim=makeSubImage(0,1, 
									  chan, itsImageShape[3],
									  pol, itsImageShape[2], 
									  *weight() );
		  CountedPtr<ImageInterface<Float> > pbsubim=makeSubImage(0,1, 
									  chan, itsImageShape[3],
									  pol, itsImageShape[2], 
									  *pb() );
		  
		  LatticeExpr<Float> normed( sqrt(abs(*wtsubim)) / itsPBScaleFactor  );
		  LatticeExpr<Float> limited( iif( normed > pblimit , normed, 0.0 ) );
		  pbsubim->copyData( limited );
		}// if not zero
	      }
	  }
	
	//MSK//	LatticeExpr<Bool> pbmask( iif( *pb() > pblimit , True , False ) );
	//MSK// createMask( pbmask, pb() );
  }

  Bool SIImageStore::createMask(LatticeExpr<Bool> &lemask, 
				CountedPtr<ImageInterface<Float> > outimage)
  {

    //   LogIO os( LogOrigin("SIImageStore","createMask",WHERE) );
    //   os << "Making mask for image : " << outimage->name() << LogIO::POST;

    if( (outimage->getDefaultMask()).matches("mask0") ) 
      { outimage->removeRegion("mask0");}
    ImageRegion outreg = outimage->makeMask("mask0",False,True);
    LCRegion& outmask=outreg.asMask();
    outmask.copyData(lemask);
    outimage->defineRegion("mask0",outreg, RegionHandler::Masks, True);
    outimage->setDefaultMask("mask0");
    return True;
  }

  Bool SIImageStore::copyMask(CountedPtr<ImageInterface<Float> > inimage,
				CountedPtr<ImageInterface<Float> > outimage)
  {
    if( (inimage->getDefaultMask()).matches("mask0") ) // input mask exists.
      {

	// clear output image mask
		if( (outimage->getDefaultMask()).matches("mask0") ) 
		  { outimage->removeRegion("mask0");}

	// get mask from input image
		
		ImageRegion* outreg = inimage->getImageRegionPtr("mask0");
		outimage->defineRegion("mask0",*outreg, RegionHandler::Masks,True);
		outimage->setDefaultMask("mask0");
      }
    
    return True;
  }
  

  void SIImageStore::dividePSFByWeight(const Float pblimit)
  {
    LogIO os( LogOrigin("SIImageStore","dividePSFByWeight",WHERE) );

    normPSF();

    //    cout << "In dividePSFByWeight : itsUseWeight : " << itsUseWeight << endl;
    if( itsUseWeight )
    { 
	
	divideImageByWeightVal( *weight() ); 

	for(Int pol=0; pol<itsImageShape[2]; pol++)
	  {
	    for(Int chan=0; chan<itsImageShape[3]; chan++)
	      {
		os << LogIO::NORMAL1 << "Scale factor for [C" +String::toString(chan) + ":P" + String::toString(pol) + "] to keep the model image w.r.to a PB of max=1 is " << getPbMax(pol,chan) << LogIO::POST;
	      }//chan
	  }//pol

	makePBFromWeight(pblimit);
	
    }
    
   }

  // Make another for the PSF too.
  void SIImageStore::divideResidualByWeight(Float pblimit,String normtype)
  {
    LogIO os( LogOrigin("SIImageStore","divideResidualByWeight",WHERE) );

    // Normalize by the sumwt, per plane. 
    Bool didNorm = divideImageByWeightVal( *residual() );

    if( itsUseWeight )
      {
	
	for(Int pol=0; pol<itsImageShape[2]; pol++)
	  {
	    for(Int chan=0; chan<itsImageShape[3]; chan++)
	      {
		
		itsPBScaleFactor = getPbMax(pol,chan);
		//	cout << " pbscale : " << itsPBScaleFactor << endl;
		if(itsPBScaleFactor<=0){os << LogIO::NORMAL1 << "Skipping normalization for C:" << chan << " P:" << pol << " because pb max is zero " << LogIO::POST;}
		else {

		CountedPtr<ImageInterface<Float> > wtsubim=makeSubImage(0,1, 
								      chan, itsImageShape[3],
								      pol, itsImageShape[2], 
								      *weight() );
		CountedPtr<ImageInterface<Float> > ressubim=makeSubImage(0,1, 
								      chan, itsImageShape[3],
								      pol, itsImageShape[2], 
								      *residual() );

		
		LatticeExpr<Float> deno;
		if( normtype=="flatnoise"){
		  deno = LatticeExpr<Float> ( sqrt( abs(*(wtsubim)) ) * itsPBScaleFactor );
		  os << LogIO::NORMAL1 ;
		  os <<  "[C" +String::toString(chan) + ":P" + String::toString(pol) + "] ";
		  os << "Dividing " << itsImageName+String(".residual") ;
		  os << " by [ sqrt(weightimage) * " << itsPBScaleFactor ;
		  os << " ] to get flat noise with unit pb peak."<< LogIO::POST;
		  
		}
		if( normtype=="flatsky") {
		  deno = LatticeExpr<Float> ( *(wtsubim) );
		  os << LogIO::NORMAL1 ;
		  os <<  "[C" +String::toString(chan) + ":P" + String::toString(pol) + "] ";
		  os << "Dividing " << itsImageName+String(".residual") ;
		  os << " by [ weight ] to get flat sky"<< LogIO::POST;
		}

		//		IPosition ip(4,itsImageShape[0]/2,itsImageShape[1]/2,0,0);
		//Float resval = ressubim->getAt(ip);

		Float scalepb = pblimit * itsPBScaleFactor * itsPBScaleFactor ;
		LatticeExpr<Float> mask( iif( (deno) > scalepb , 1.0, 0.0 ) );
		LatticeExpr<Float> maskinv( iif( (deno) > scalepb , 0.0, 1.0 ) );
		LatticeExpr<Float> ratio( ( (*(ressubim)) * mask ) / ( deno + maskinv ) );
		
		ressubim->copyData(ratio);

		//cout << "Val of residual before|after normalizing at center for pol " << pol << " chan " << chan << " : " << resval << "|" << ressubim->getAt(ip) << " weight : " << wtsubim->getAt(ip) << endl;
		}// if not zero
	      }//chan
	  }//pol
	
      }
    
    // If no normalization happened, print a warning. The user must check if it's right or not.
    // Or... later if we get a gridder that does pre-norms, this warning can go. 
    if( (didNorm | itsUseWeight) != True ) 
      os << LogIO::WARN << "No normalization done to residual" << LogIO::POST;

    // createMask
    //MSK//    if(hasPB()){copyMask(pb(),residual());}
  }
  

  void SIImageStore::divideModelByWeight(Float pblimit, const String normtype)
  {
    LogIO os( LogOrigin("SIImageStore","divideModelByWeight",WHERE) );

        if(itsUseWeight // only when needed
       	&& hasSensitivity() )// i.e. only when possible. For an initial starting model, don't need wt anyway.
      {

	if( normtype=="flatsky") {
	  Array<Float> arrmod;
	  model()->get( arrmod, True );

	  os << LogIO::NORMAL1 << "Model is already flat sky with peak flux : " << max(arrmod);
	  os << ". No need to divide before prediction" << LogIO::POST;
	  
	  return;
	  }
	else if( normtype=="flatnoise"){

	  for(Int pol=0; pol<itsImageShape[2]; pol++)
	    {
	      for(Int chan=0; chan<itsImageShape[3]; chan++)
		{
		  
		  itsPBScaleFactor = getPbMax(pol,chan);
		  //	cout << " pbscale : " << itsPBScaleFactor << endl;
		if(itsPBScaleFactor<=0){os << LogIO::NORMAL1 << "Skipping normalization for C:" << chan << " P:" << pol << " because pb max is zero " << LogIO::POST;}
		else {
		  
		  CountedPtr<ImageInterface<Float> > wtsubim=makeSubImage(0,1, 
									  chan, itsImageShape[3],
									  pol, itsImageShape[2], 
									  *weight() );
		  CountedPtr<ImageInterface<Float> > modsubim=makeSubImage(0,1, 
									   chan, itsImageShape[3],
									   pol, itsImageShape[2], 
									   *model() );
		  os << LogIO::NORMAL1 ;
		  os <<  "[C" +String::toString(chan) + ":P" + String::toString(pol) + "] ";
		  os << "Dividing " << itsImageName+String(".model") ;
		  os << " by [ sqrt(weight) / " << itsPBScaleFactor ;
		  os <<" ] to get to flat sky model before prediction" << LogIO::POST;
		  
		  LatticeExpr<Float> deno( sqrt( abs(*(wtsubim)) ) / itsPBScaleFactor );
		  
		  LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
		  LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
		  LatticeExpr<Float> ratio( ( (*(modsubim)) * mask ) / ( deno + maskinv ) );
		  
		  IPosition ip(4,itsImageShape[0]/2,itsImageShape[1]/2,0,0);
		  Float modval = modsubim->getAt(ip);
		  //LatticeExprNode aminval( min(*modsubim) );
		  //LatticeExprNode amaxval( max(*modsubim) );
		  //cout << "Before ---- min : " << aminval.getFloat() << " max : " << amaxval.getFloat() << endl;

		  modsubim->copyData(ratio);
		  
		  cout << "Val of model before|after flattening at center for pol " << pol << " chan " << chan << " : " << modval << "|" << modsubim->getAt(ip) << " weight : " << wtsubim->getAt(ip) << endl;
		  //LatticeExprNode minval( min(*modsubim) );
		  //LatticeExprNode maxval( max(*modsubim) );
		  //cout << "After ---- min : " << minval.getFloat() << " max : " << maxval.getFloat() << endl;
		}// if not zero
		}//chan
	    }//pol

	}

	storeImg(String("flatmodel.im"), *model());
	
      }
    // createMask
    }
  
  void SIImageStore::multiplyModelByWeight(Float pblimit, const String normtype)
  {
   LogIO os( LogOrigin("SIImageStore","multiplyModelByWeight",WHERE) );

        if(itsUseWeight // only when needed
    	&& hasSensitivity() )// i.e. only when possible. For an initial starting model, don't need wt anyway.
      {
	if( normtype=="flatsky") {
	  os << "Model is already flat sky. No need to multiply back after prediction" << LogIO::POST;
	  return;
	  }
	else if( normtype=="flatnoise"){

	  for(Int pol=0; pol<itsImageShape[2]; pol++)
	    {
	      for(Int chan=0; chan<itsImageShape[3]; chan++)
		{
		  
		  itsPBScaleFactor = getPbMax(pol,chan);
		  //	cout << " pbscale : " << itsPBScaleFactor << endl;
		if(itsPBScaleFactor<=0){os << LogIO::NORMAL1 << "Skipping normalization for C:" << chan << " P:" << pol << " because pb max is zero " << LogIO::POST;}
		else {
		  
		  CountedPtr<ImageInterface<Float> > wtsubim=makeSubImage(0,1, 
									  chan, itsImageShape[3],
									  pol, itsImageShape[2], 
									  *weight() );
		  CountedPtr<ImageInterface<Float> > modsubim=makeSubImage(0,1, 
									   chan, itsImageShape[3],
									   pol, itsImageShape[2], 
									   *model() );
		  os << LogIO::NORMAL1 ;
		  os <<  "[C" +String::toString(chan) + ":P" + String::toString(pol) + "] ";
		  os << "Multiplying " << itsImageName+String(".model") ;
		  os << " by [ sqrt(weight) / " << itsPBScaleFactor;
		  os <<  " ] to take model back to flat noise with unit pb peak." << LogIO::POST;
		  
		  LatticeExpr<Float> deno( sqrt( abs(*(wtsubim)) ) / itsPBScaleFactor );
		  
		  LatticeExpr<Float> mask( iif( (deno) > pblimit , 1.0, 0.0 ) );
		  LatticeExpr<Float> maskinv( iif( (deno) > pblimit , 0.0, 1.0 ) );
		  LatticeExpr<Float> ratio( ( (*(modsubim)) * mask ) * ( deno + maskinv ) );
		  
		  modsubim->copyData(ratio);
		}// if not zero
		}//chan
	    }//pol
	}
	
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

  void SIImageStore::makeImageBeamSet()
  {
    LogIO os( LogOrigin("SIImageStore","getPSFGaussian",WHERE) );
    // For all chans/pols, call getPSFGaussian() and put it into ImageBeamSet(chan,pol).
    AlwaysAssert( itsImageShape.nelements() == 4, AipsError );
    Int nx = itsImageShape[0];
    Int ny = itsImageShape[1];
    Int npol = itsImageShape[2];
    Int nchan = itsImageShape[3];
    itsPSFBeams.resize( nchan, npol );

    //    cout << "makeImBeamSet : imshape : " << itsImageShape << endl;

    String blankpsfs="";

    for( Int chanid=0; chanid<nchan;chanid++) {
      for( Int polid=0; polid<npol; polid++ ) {

	IPosition substart(4,0,0,polid,chanid);
	IPosition substop(4,nx-1,ny-1,polid,chanid);
	Slicer psfslice(substart, substop,Slicer::endIsLast);
	SubImage<Float> subPsf( *psf() , psfslice, True );
	GaussianBeam beam;

	Bool tryfit=True;
	LatticeExprNode le( max(subPsf) );
	tryfit = le.getFloat()>0.0;

	if(tryfit)
	  {
	    try
	      {
		StokesImageUtil::FitGaussianPSF( subPsf, beam );
		itsPSFBeams.setBeam( chanid, polid, beam );
	      }
	    catch(AipsError &x)
	      {
	    	os << LogIO::WARN << "[Chan" << chanid << ":Pol" << polid << "] Error Gaussian fit to PSF : " << x.getMesg() ;
		//		os << LogIO::POST;
		os << " :  Setting restoring beam to largest valid beam." << LogIO::POST;
	      }
	  }
	else
	  {
	    //	    os << LogIO::WARN << "[Chan" << chanid << ":Pol" << polid << "] PSF is blank. Setting null restoring beam." << LogIO::POST ;
	    blankpsfs += "[C" +String::toString(chanid) + ":P" + String::toString(polid) + "] ";
	  }

      }// end of pol loop
    }// end of chan loop

    if( blankpsfs.length() >0 )
      os << LogIO::WARN << "PSF is blank for" << blankpsfs << LogIO::POST;

    //// Replace null (and bad) beams with the good one. 
    ////GaussianBeam maxbeam = findGoodBeam(True);

    //// Replace null beams by a tiny tiny beam, just to get past the ImageInfo restriction that
    //// all planes must have non-null beams.
    Quantity majax(1e-06,"arcsec"),minax(1e-06,"arcsec"),pa(0.0,"deg");
    GaussianBeam defaultbeam;
    defaultbeam.setMajorMinor(majax,minax);
    defaultbeam.setPA(pa);
    for( Int chanid=0; chanid<nchan;chanid++) {
      for( Int polid=0; polid<npol; polid++ ) {
	if( (itsPSFBeams.getBeam(chanid, polid)).isNull() ) 
	  { itsPSFBeams.setBeam( chanid, polid, defaultbeam ); }
      }// end of pol loop
    }// end of chan loop
    
    /*        
    //// Fill in gaps if there are any --- with the MAX Area beam. 
    /////    GaussianBeam maxbeam = itsPSFBeams.getMaxAreaBeam();
    if( maxbeam.isNull() ) {
	os << LogIO::WARN << "No planes have non-zero restoring beams. Forcing artificial 1.0arcsec beam." << LogIO::POST;
	Quantity majax(1.0,"arcsec"),minax(1.0,"arcsec"),pa(0.0,"deg");
	maxbeam.setMajorMinor(majax,minax);
	maxbeam.setPA(pa);
      }
    else  {
	for( Int chanid=0; chanid<nchan;chanid++) {
	  for( Int polid=0; polid<npol; polid++ ) {
	    if( (itsPSFBeams.getBeam(chanid, polid)).isNull() ) 
	      { itsPSFBeams.setBeam( chanid, polid, maxbeam ); }
	  }// end of pol loop
	}// end of chan loop
      }
    */


    /// For lack of a better place, store this inside the PSF image. To be read later and used to restore
    ImageInfo ii = psf()->imageInfo();
    ii.setBeams( itsPSFBeams );
    psf()->setImageInfo(ii);
    
  }// end of make beam set



  ImageBeamSet SIImageStore::getBeamSet()
  { 
    IPosition beamshp = itsPSFBeams.shape();
    AlwaysAssert( beamshp.nelements()==2 , AipsError );
    if( beamshp[0]==0 || beamshp[1]==0 ) {makeImageBeamSet();}
    return itsPSFBeams; 
  }

  void SIImageStore::printBeamSet()
  {
    LogIO os( LogOrigin("SIImageStore","printBeamSet",WHERE) );
    AlwaysAssert( itsImageShape.nelements() == 4, AipsError );
    if( itsImageShape[3] == 1 && itsImageShape[2]==1 )
      {
	GaussianBeam beam = itsPSFBeams.getBeam();
	os << "Beam : " << beam.getMajor(Unit("arcsec")) << " arcsec, " << beam.getMinor(Unit("arcsec"))<< " arcsec, " << beam.getPA(Unit("deg")) << " deg" << LogIO::POST; 
 }
    else
      {
	// TODO : Enable this, when this function doesn't complain about 0 rest freq.
	//                                 or when rest freq is never zero !
	try{
		itsPSFBeams.summarize( os, False, itsCoordSys );
	}
	catch(AipsError &x)
	  {
	    os << LogIO::WARN << "Error while printing (compact) restoring beam summary : " <<  x.getMesg() << LogIO::POST;
	    os << "Printing long summary" << LogIO::POST;
	    
	    AlwaysAssert( itsImageShape.nelements() == 4, AipsError );
	    //Int npol = itsImageShape[2];
	    Int nchan = itsImageShape[3];
	    for( Int chanid=0; chanid<nchan;chanid++) {
	      Int polid=0;
	      //	  for( Int polid=0; polid<npol; polid++ ) {
	      GaussianBeam beam = itsPSFBeams.getBeam( chanid, polid );
	      os << "Beam [C" << chanid << "]: " << beam.getMajor(Unit("arcsec")) << " arcsec, " << beam.getMinor(Unit("arcsec"))<< " arcsec, " << beam.getPA(Unit("deg")) << " deg" << LogIO::POST; 
	      //}//for polid
	    }//for chanid
	  }// catch
      }
  }
  
  /////////////////////////////// Restore all planes.

  void SIImageStore::restore(GaussianBeam& rbeam, String& usebeam, uInt term)
  {

    LogIO os( LogOrigin("SIImageStore","restore",WHERE) );
    //     << ". Optionally, PB-correct too." << LogIO::POST;

    AlwaysAssert( itsImageShape.nelements() == 4, AipsError );
    Int nx = itsImageShape[0];
    Int ny = itsImageShape[1];
    Int npol = itsImageShape[2];
    Int nchan = itsImageShape[3];

    //// Get/fill the beamset
    IPosition beamset = itsPSFBeams.shape();
    AlwaysAssert( beamset.nelements()==2 , AipsError );
    if( beamset[0] != nchan || beamset[1] != npol )
      {
	
	// Get PSF Beams....
	ImageInfo ii = psf()->imageInfo();
	itsPSFBeams = ii.getBeamSet();

	IPosition beamset2 = itsPSFBeams.shape();

	AlwaysAssert( beamset2.nelements()==2 , AipsError );
	if( beamset2[0] != nchan || beamset2[1] != npol )
	  {
	    // Make new beams.
	    os << LogIO::WARN << "Couldn't find pre-computed restoring beams. Re-fitting." << LogIO::POST;
	    makeImageBeamSet();
	  }
      }

    //// Modify the beamset if needed
    //// if ( rbeam is Null and usebeam=="" ) Don't do anything.
    //// If rbeam is Null but usebeam=='common', calculate a common beam and set 'rbeam'
    //// If rbeam is given (or exists due to 'common'), just use it.
    if( rbeam.isNull() && usebeam=="common") {
      rbeam = findGoodBeam();
    }
    if( !rbeam.isNull() ) {
      for( Int chanid=0; chanid<nchan;chanid++) {
	for( Int polid=0; polid<npol; polid++ ) {
	  itsPSFBeams.setBeam( chanid, polid, rbeam );
	  /// Still need to add the 'common beam' option.
	}//for chanid
      }//for polid
    }// if rbeam not NULL
    //// Done modifying beamset if needed
    
    //// Use beamset to restore
    for( Int chanid=0; chanid<nchan;chanid++) {
      for( Int polid=0; polid<npol; polid++ ) {
	
	IPosition substart(4,0,0,polid,chanid);
	IPosition substop(4,nx-1,ny-1,polid,chanid);
	Slicer imslice(substart, substop,Slicer::endIsLast);
	SubImage<Float> subRestored( *image(term) , imslice, True );
	SubImage<Float> subModel( *model(term) , imslice, True );
	SubImage<Float> subResidual( *residual(term) , imslice, True );
	
	GaussianBeam beam = itsPSFBeams.getBeam( chanid, polid );;
	
	try
	  {
	    // Initialize restored image
	    subRestored.set(0.0);
	    // Copy model into it
	    subRestored.copyData( LatticeExpr<Float>( subModel )  );
	    // Smooth model by beam
	    StokesImageUtil::Convolve( subRestored, beam);
	    // Add residual image
	    subRestored.copyData( LatticeExpr<Float>( subRestored + subResidual  ) );
	    
	  }
	catch(AipsError &x)
	  {
	    throw( AipsError("Restoration Error in chan" + String::toString(chanid) + ":pol" + String::toString(polid) + " : " + x.getMesg() ) );
	  }
	
      }// end of pol loop
    }// end of chan loop
    
    try
      {
	//MSK//	if(hasPB()){copyMask(pb(),image());}
      }
    catch(AipsError &x)
      {
	throw( AipsError("Restoration Error (setting mask) : "  + x.getMesg() ) );
      }
	
  }// end of restore()

  GaussianBeam SIImageStore::findGoodBeam()
  {
    LogIO os( LogOrigin("SIImageStore","findGoodBeam",WHERE) );
    IPosition beamshp = itsPSFBeams.shape();
    AlwaysAssert( beamshp.nelements()==2 , AipsError );

    /*
    if( beamshp[0] != nchan || beamshp[1] != npol )
      {
	// Make new beams.
	os << LogIO::WARN << "Couldn't find pre-computed restoring beams. Re-fitting." << LogIO::POST;
	makeImageBeamSet();
      }
    */

    Vector<Float> areas(beamshp[0]*beamshp[1]);
    Vector<Float> axrat(beamshp[0]*beamshp[1]);
    areas=0.0; axrat=1.0;
    Vector<Bool> flags( areas.nelements() );
    flags=False;
    
    Int cnt=0;
    for( Int chanid=0; chanid<beamshp[0];chanid++) {
      for( Int polid=0; polid<beamshp[1]; polid++ ) {
	GaussianBeam beam = itsPSFBeams(chanid, polid);
	if( !beam.isNull() && beam.getMajor(Unit("arcsec"))>1.1e-06  )  // larger than default filler beam.
	  {
	    areas[cnt] = beam.getArea( Unit("arcsec2") );
	    axrat[cnt] = beam.getMajor( Unit("arcsec") ) / beam.getMinor( Unit("arcsec") );
	  }
	else {
	  flags[cnt] = True;
	}
	cnt++;
      }//for chanid
    }//for polid
    
    Vector<Float> fit( areas.nelements() );
    Vector<Float> fitaxr( areas.nelements() );
    for (Int loop=0;loop<5;loop++)  {
      /// Filter on outliers in PSF beam area
      lineFit( areas, flags, fit, 0, areas.nelements()-1 );
      Float sd = calcStd( areas , flags, fit );
      for (uInt  i=0;i<areas.nelements();i++) {
	if( fabs( areas[i] - fit[i] ) > 3*sd ) flags[i]=True;
      }
      /// Filter on outliers in PSF axial ratio
      lineFit( axrat, flags, fitaxr, 0, areas.nelements()-1 );
      Float sdaxr = calcStd( axrat , flags, fitaxr );
      for (uInt  i=0;i<areas.nelements();i++) {
	if( fabs( axrat[i] - fitaxr[i] ) > 3*sdaxr ) flags[i]=True;
      }
    }
    //    cout << "Original areas : " << areas << endl;
    //    cout << "Original axrats : " << axrat << endl;
    //    cout << "Flags : " << flags << endl;

    // Find max area good beam.
    GaussianBeam goodbeam;
    Int cid=0,pid=0;
    Float maxval=0.0;
    cnt=0;
    for( Int chanid=0; chanid<beamshp[0];chanid++) {
      for( Int polid=0; polid<beamshp[1]; polid++ ) {
	if( flags[cnt] == False ){ 
	  if( areas[cnt] > maxval ) {maxval = areas[cnt]; goodbeam = itsPSFBeams.getBeam(chanid,polid);cid=chanid;pid=polid;}
	}
	cnt++;
      }//polid
    }//chanid

    os << "Picking common beam from C"<<cid<<":P"<<pid<<" : " << goodbeam.getMajor(Unit("arcsec")) << " arcsec, " << goodbeam.getMinor(Unit("arcsec"))<< " arcsec, " << goodbeam.getPA(Unit("deg")) << " deg" << LogIO::POST; 

    Bool badbeam=False;
    for(uInt i=0;i<flags.nelements();i++){if(flags[i]==True) badbeam=True;}

    if( badbeam == True ) 
      { 
	os << "(Ignored beams from :";
	cnt=0;
	for( Int chanid=0; chanid<beamshp[0];chanid++) {
	  for( Int polid=0; polid<beamshp[1]; polid++ ) {
	    if( flags[cnt] == True ){ 
	      os << " C"<<chanid<<":P"<<polid;
	    }
	    cnt++;
	  }//polid
	}//chanid
	os << " as outliers either by area or by axial ratio)" << LogIO::POST;
      } 


    /*
    // Replace 'bad' psfs with the chosen one.
    if( goodbeam.isNull() ) {
      os << LogIO::WARN << "No planes have non-zero restoring beams. Forcing artificial 1.0arcsec beam." << LogIO::POST;
      Quantity majax(1.0,"arcsec"),minax(1.0,"arcsec"),pa(0.0,"deg");
      goodbeam.setMajorMinor(majax,minax);
      goodbeam.setPA(pa);
    }
    else  {
      cnt=0;
      for( Int chanid=0; chanid<nchan;chanid++) {
	for( Int polid=0; polid<npol; polid++ ) {
	  if( flags[cnt]==True ) 
	    { itsPSFBeams.setBeam( chanid, polid, goodbeam ); }
	  cnt++;
	}// end of pol loop
      }// end of chan loop
    }
    */

    return goodbeam;
  }// end of findGoodBeam

  ///////////////////////// Funcs to calculate robust mean and fit, taking into account 'flagged' points.
void SIImageStore :: lineFit(Vector<Float> &data, Vector<Bool> &flag, Vector<Float> &fit, uInt lim1, uInt lim2)
{
  float Sx = 0, Sy = 0, Sxx = 0, Sxy = 0, S = 0, a, b, sd, mn;
  
  mn = calcMean(data, flag);
  sd = calcStd (data, flag, mn);
  
  for (uInt i = lim1; i <= lim2; i++)
    {
      if (flag[i] == False) // if unflagged
	{
	  S += 1 / (sd * sd);
	  Sx += i / (sd * sd);
	  Sy += data[i] / (sd * sd);
	  Sxx += (i * i) / (sd * sd);
	  Sxy += (i * data[i]) / (sd * sd);
	}
    }
  a = (Sxx * Sy - Sx * Sxy) / (S * Sxx - Sx * Sx);
  b = (S * Sxy - Sx * Sy) / (S * Sxx - Sx * Sx);
  
  for (uInt i = lim1; i <= lim2; i++)
    fit[i] = a + b * i;
  
}
/* Calculate the MEAN of 'vect' ignoring values flagged in 'flag' */
Float SIImageStore :: calcMean(Vector<Float> &vect, Vector<Bool> &flag)
{
  Float mean=0;
  Int cnt=0;
  for(uInt i=0;i<vect.nelements();i++)
    if(flag[i]==False)
      {
	mean += vect[i];
	cnt++;
      }
  if(cnt==0) cnt=1;
  return mean/cnt;
}
Float SIImageStore :: calcStd(Vector<Float> &vect, Vector<Bool> &flag, Vector<Float> &fit)
{
  Float std=0;
  uInt n=0,cnt=0;
  n = vect.nelements() < fit.nelements() ? vect.nelements() : fit.nelements();
  for(uInt i=0;i<n;i++)
    if(flag[i]==False)
      {
	cnt++;
	std += (vect[i]-fit[i])*(vect[i]-fit[i]);
      }
  if(cnt==0) cnt=1;
  return sqrt(std/cnt);

}
Float SIImageStore :: calcStd(Vector<Float> &vect, Vector<Bool> &flag, Float mean)
{
  Float std=0;
  uInt cnt=0;
  for(uInt i=0;i<vect.nelements();i++)
    if(flag[i]==False)
      {
	cnt++;
	std += (vect[i]-mean)*(vect[i]-mean);
      }
  return sqrt(std/cnt);
}

  ///////////////////////// End of Funcs to calculate robust mean and fit.



/*
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
	    //os << "[" << itsImageName << "] " ;  // Add when parent image name is available.
	    //os << "Restore with beam : " << beam.getMajor(Unit("arcmin")) << " arcmin, " << beam.getMinor(Unit("arcmin"))<< " arcmin, " << beam.getPA(Unit("deg")) << " deg" << LogIO::POST; 
	    
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
*/

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
  /*
  Bool SIImageStore::getUseWeightImage()
  {
    if( ! itsParentSumWt )
      return False;
    else 
     return  getUseWeightImage( *itsParentSumWt );
  }
  */
  void SIImageStore::setUseWeightImage(ImageInterface<Float>& target, Bool useweightimage)
  {
    Record miscinfo = target.miscInfo();
    miscinfo.define("useweightimage", useweightimage);
    target.setMiscInfo( miscinfo );
  }
  


  Bool SIImageStore::divideImageByWeightVal( ImageInterface<Float>& target )
  {

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
		//		SubImage<Float>* subim=makePlane(  chan, True ,pol, True, target );
		SHARED_PTR<ImageInterface<Float> > subim=makeSubImage(0,1, 
								      chan, lsumwt.shape()[3],
								      pol, lsumwt.shape()[2], 
								      target );
		if ( lsumwt(pos) > 1e-07 ) {
		    LatticeExpr<Float> le( (*subim)/lsumwt(pos) );
		    subim->copyData( le );
		  }
		else  {
		    subim->set(0.0);
		  }
		div=True;
	      }
	  }
      }

    //    if( div==True ) cout << "Div image by sumwt : " << lsumwt << endl;
    //    else cout << "Already normalized" << endl;

    //    lsumwt = 1.0; setSumWt( target , lsumwt );

    return div;
  }

  void  SIImageStore::normPSF(Int term)
  {

    for(Int pol=0; pol<itsImageShape[2]; pol++)
      {
	for(Int chan=0; chan<itsImageShape[3]; chan++)
	  {
	    ///	    IPosition center(4,itsImageShape[0]/2,itsImageShape[1]/2,pol,chan);
	    
	    SHARED_PTR<ImageInterface<Float> > subim=makeSubImage(0,1, 
								  chan, itsImageShape[3],
								  pol, itsImageShape[2], 
								  (*psf(term)) );

	    SHARED_PTR<ImageInterface<Float> > subim0=makeSubImage(0,1, 
								  chan, itsImageShape[3],
								  pol, itsImageShape[2], 
								  (*psf(0)) );

	    LatticeExpr<Float> normed( (*(subim)) / max(*(subim0)) );
	    subim->copyData( normed );
	  }
      }

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////   Utility Functions to gather statistics on the imagestore.

Float SIImageStore::getPeakResidual()
{
    LogIO os( LogOrigin("SIImageStore","getPeakResidual",WHERE) );

    Float maxresidual = max( residual()->get() );

    return maxresidual;
  }

Float SIImageStore::getPeakResidualWithinMask()
  {
    LogIO os( LogOrigin("SIImageStore","getPeakResidualWithinMask",WHERE) );
    Float minresmask, maxresmask, minres, maxres;
    findMinMax( residual()->get(), mask()->get(), minres, maxres, minresmask, maxresmask );

    return maxresmask;
  }

  // Calculate the total model flux
Float SIImageStore::getModelFlux(uInt term)
  {
    //    LogIO os( LogOrigin("SIImageStore","getModelFlux",WHERE) );

    Float modelflux = sum( model(term)->get() );

    return modelflux;
  }

  // Check for non-zero model (this is different from getting model flux, for derived SIIMMT)
Bool SIImageStore::isModelEmpty()
  {
    /// There MUST be a more efficient way to do this !!!!!  I hope. 
    return  ( fabs( sum( model()->get() ) ) < 1e-08 );
  }

  // Calculate the PSF sidelobe level...
  Float SIImageStore::getPSFSidelobeLevel()
  {
    LogIO os( LogOrigin("SIImageStore","getPSFSidelobeLevel",WHERE) );

    /// Calculate only once, store and return for all subsequent calls.

    Float psfsidelobe = fabs(min( psf()->get() ));

    if(psfsidelobe == 1.0)
      {
	//os << LogIO::WARN << "For testing only. Set psf sidelobe level to 0.01" << LogIO::POST;
	psfsidelobe = 0.01;
      }

    return psfsidelobe;
  }

  void SIImageStore::findMinMax(const Array<Float>& lattice,
					const Array<Float>& mask,
					Float& minVal, Float& maxVal,
					Float& minValMask, Float& maxValMask)
  {
    IPosition posmin(lattice.shape().nelements(), 0);
    IPosition posmax(lattice.shape().nelements(), 0);

    if( sum(mask) <1e-06 ) {minValMask=0.0; maxValMask=0.0;}
    else { minMaxMasked(minValMask, maxValMask, posmin, posmax, lattice,mask); }

    minMax( minVal, maxVal, posmin, posmax, lattice );
  }

  void SIImageStore::printImageStats()
  {
    LogIO os( LogOrigin("SIImageStore","printImageStats",WHERE) );
    Float minresmask, maxresmask, minres, maxres;
    findMinMax( residual()->get(), mask()->get(), minres, maxres, minresmask, maxresmask );

    os << "[" << itsImageName << "]" ;
    os << " Peak residual (max,min) " ;
    if( minresmask!=0.0 || maxresmask!=0.0 )
      { os << "within mask : (" << maxresmask << "," << minresmask << ") "; }
    os << "over full image : (" << maxres << "," << minres << ")" << LogIO::POST;

    os << "[" << itsImageName << "] Total Model Flux : " << getModelFlux() << LogIO::POST; 

  }

  // Calculate the total model flux
  Float SIImageStore::getMaskSum()
  {
    LogIO os( LogOrigin("SIImageStore","getMaskSum",WHERE) );

    Float masksum = sum( mask()->get() );

    return masksum;
  }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    imageExts(GRIDWT)=".gridwt";
    imageExts(PB)=".pb";
    imageExts(FORWARDGRID)=".forward";
    imageExts(BACKWARDGRID)=".backward";

    itsParentPsf = itsPsf;
    itsParentModel=itsModel;
    itsParentResidual=itsResidual;
    itsParentWeight=itsWeight;
    itsParentImage=itsImage;
    itsParentSumWt=itsSumWt;
    itsParentMask=itsMask;

    //    cout << "parent shape : " << itsParentImageShape << "   shape : " << itsImageShape << endl;
    itsParentImageShape = itsImageShape;

    if( itsNFacets>1 || itsNChanChunks>1 || itsNPolChunks>1 ) { itsImageShape=IPosition(4,0,0,0,0); }

 
  }


void SIImageStore::regridToModelImage( ImageInterface<Float> &inputimage, Int term )
  {
    try 
      {

    //output coords
	IPosition outshape = itsImageShape;
	CoordinateSystem outcsys = itsCoordSys;
	DirectionCoordinate outDirCsys = outcsys.directionCoordinate();
	SpectralCoordinate outSpecCsys = outcsys.spectralCoordinate();
     
	// do regrid   
	IPosition axes(3,0, 1, 2);
	IPosition inshape = inputimage.shape();
	CoordinateSystem incsys = inputimage.coordinates(); 
	DirectionCoordinate inDirCsys = incsys.directionCoordinate();
	SpectralCoordinate inSpecCsys = incsys.spectralCoordinate();

	Vector<Int> dirAxes = CoordinateUtil::findDirectionAxes(incsys);
	axes(0) = dirAxes(0); 
	axes(1) = dirAxes(1);
	axes(2) = CoordinateUtil::findSpectralAxis(incsys);
	try {
	  ImageRegrid<Float> imregrid;
	  imregrid.regrid( *(model(term)), Interpolate2D::LINEAR, axes, inputimage ); 
	} catch (AipsError &x) {
	  throw(AipsError("ImageRegrid error : "+ x.getMesg()));
	}
	
      }catch(AipsError &x)
      {
	throw("Error in regridding input model image to target coordsys : " + x.getMesg());
      }
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
    outFile << "itsUseWeight: " << itsUseWeight << endl;
    

    // Misc Information to go into the header. 
    //    Record itsMiscInfo; 
    itsMiscInfo.print(outFile);
    
    // SHARED_PTR<ImageInterface<Float> > itsMask, itsPsf, itsModel, itsResidual, itsWeight, itsImage, itsSumWt;
    // SHARED_PTR<ImageInterface<Complex> > itsForwardGrid, itsBackwardGrid;

    Vector<Bool> ImageExists(MAX_IMAGE_IDS);
    if ( ! itsMask )     ImageExists(MASK)=False;
    if ( ! itsPsf )      ImageExists(PSF)=False;
    if ( ! itsModel )    ImageExists(MODEL)=False;
    if ( ! itsResidual ) ImageExists(RESIDUAL)=False;
    if ( ! itsWeight )   ImageExists(WEIGHT)=False;
    if ( ! itsImage )    ImageExists(IMAGE)=False;
    if ( ! itsSumWt )    ImageExists(SUMWT)=False;
    if ( ! itsGridWt )   ImageExists(GRIDWT)=False;
    if ( ! itsPB )       ImageExists(PB)=False;

    if ( ! itsForwardGrid )    ImageExists(FORWARDGRID)=False;
    if ( ! itsBackwardGrid )   ImageExists(BACKWARDGRID)=False;
    
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
    inFile >> token; if (token=="itsUseWeight:") inFile >> itsUseWeight;

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

