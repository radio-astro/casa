//# SynthesisNormalizer.cc: Implementation of Gather/Scatter for parallel major cycles
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
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>

#include <synthesis/ImagerObjects/SynthesisNormalizer.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisNormalizer::SynthesisNormalizer() : 
				       itsImages(CountedPtr<SIImageStore>()),
				       itsPartImages(Vector<CountedPtr<SIImageStore> >()),
                                       itsImageName(""),
                                       itsPartImageNames(Vector<String>(0)),
				       itsPBLimit(0.1),
				       itsMapperType("default"),
				       itsNTaylorTerms(1),
                                       itsNFacets(1)
  {
    itsFacetImageStores.resize(0);
  }
  
  SynthesisNormalizer::~SynthesisNormalizer() 
  {
    LogIO os( LogOrigin("SynthesisNormalizer","descructor",WHERE) );
    os << "SynthesisNormalizer destroyed" << LogIO::POST;
  }
  
  
  void SynthesisNormalizer::setupNormalizer(Record normpars)
  {
    LogIO os( LogOrigin("SynthesisNormalizer","setupNormalizer",WHERE) );

    try
      {

      if( normpars.isDefined("imagename") )  // A single string
	{ itsImageName = normpars.asString( RecordFieldId("imagename")); }
      else
	{throw( AipsError("imagename not specified")); }

      if( normpars.isDefined("partimagenames") )  // A vector of strings
	{ normpars.get( RecordFieldId("partimagenames") , itsPartImageNames ); }
      else
	{ itsPartImageNames.resize(0); }

      if( normpars.isDefined("pblimit") )
	{
	  normpars.get( RecordFieldId("pblimit") , itsPBLimit );
	}
      else
	{ itsPBLimit = 0.1; }

      if( normpars.isDefined("normtype") )  // A single string
	{ itsNormType = normpars.asString( RecordFieldId("normtype")); }
      else
	{ itsNormType = "flatnoise";} // flatnoise, flatsky

      cout << "Chosen normtype : " << itsNormType << endl;

      // For multi-term choices. Try to eliminate, after making imstores hold aux descriptive info.
      if( normpars.isDefined("mtype") )  // A single string
	{ itsMapperType = normpars.asString( RecordFieldId("mtype")); }
      else
	{ itsMapperType = "default";}

      if( normpars.isDefined("ntaylorterms") )  // A single int
	{ itsNTaylorTerms = normpars.asuInt( RecordFieldId("ntaylorterms")); }
      else
	{ itsNTaylorTerms = 1;}

      if( normpars.isDefined("facets") )  // A single int
	{ itsNFacets = normpars.asuInt( RecordFieldId("facets")); }
      else
	{ itsNFacets = 1;}


      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading gather/scatter parameters: "+x.getMesg()) );
      }
    
  }//end of setupParSync


  void SynthesisNormalizer::gatherImages(Bool dopsf) //, Bool doresidual)
  {

    //    cout << " partimagenames :" << itsPartImageNames << endl;

    Bool needToGatherImages = setupImagesOnDisk();

    if( needToGatherImages )
      {
	LogIO os( LogOrigin("SynthesisNormalizer", "gatherImages",WHERE) );

	AlwaysAssert( itsPartImages.nelements()>0 , AipsError );
	Bool doresidual = !dopsf;
        Bool doweight = dopsf ; //|| ( doresidual && ! itsImages->hasSensitivity() );
        //	Bool doweight = dopsf || ( doresidual && itsImages->getUseWeightImage(*(itsPartImages[0]->residual())) );
	
	os << "Gather "<< (doresidual?"residual":"") << ( (dopsf&&doresidual)?",":"")  
	   << (dopsf?"psf":"") << ( (dopsf&&doweight)?",":"")  
	   << (doweight?"weight":"")<< " images : " << itsPartImageNames 
	   << " onto :" << itsImageName << LogIO::POST;
	
	// Add intelligence to modify all only the first time, but later, only residual;
	itsImages->resetImages( /*psf*/dopsf, /*residual*/doresidual, /*weight*/doweight ); 
	
	for( uInt part=0;part<itsPartImages.nelements();part++)
	  {
	    itsImages->addImages( itsPartImages[part], /*psf*/dopsf, /*residual*/doresidual, /*weight*/doweight );
	  }

      }// end of image gathering.
    
    // Normalize by the weight image.
    //    divideResidualByWeight();

  }// end of gatherImages

  void SynthesisNormalizer::scatterModel()
  {

    LogIO os( LogOrigin("SynthesisNormalizer", "scatterModel",WHERE) );

    if( itsPartImages.nelements() > 0 )
      {
	os << "Send the model from : " << itsImageName << " to all nodes :" << itsPartImageNames << LogIO::POST;
	
	for( uInt part=0;part<itsPartImages.nelements();part++)
	  {
	    itsPartImages[part]->setModelImage( itsImages->getName() );
	    itsPartImages[part]->releaseLocks();
	  }
	itsImages->releaseLocks();
      }
  }// end of gatherImages

  

  void SynthesisNormalizer::divideResidualByWeight()
  {
    LogIO os( LogOrigin("SynthesisNormalizer", "divideResidualByWeight",WHERE) );
    

    if( itsNFacets==1) {
      itsImages->divideResidualByWeight( itsPBLimit, itsNormType );
    }
    else {
      for ( uInt facet=0; facet<itsNFacets*itsNFacets; facet++ )
        { itsFacetImageStores[facet]->divideResidualByWeight( itsPBLimit , itsNormType ); }
    }
    
  }
  
  void SynthesisNormalizer::dividePSFByWeight()
  {
    LogIO os( LogOrigin("SynthesisNormalizer", "dividePSFByWeight",WHERE) );
    
    if( itsNFacets==1) {
      itsImages->dividePSFByWeight();
    }
    else {
      for ( uInt facet=0; facet<itsNFacets*itsNFacets; facet++ )
        { itsFacetImageStores[facet]->dividePSFByWeight( ); }
    }

  }


  void SynthesisNormalizer::divideModelByWeight()
  {
    LogIO os( LogOrigin("SynthesisNormalizer", "divideModelByWeight",WHERE) );
    if( itsImages.null() ) 
      {
	os << LogIO::WARN << "No imagestore yet. Do something to fix the starting model case...." << LogIO::POST;
	return;
      }
    if( itsNFacets==1) {
      itsImages->divideModelByWeight( itsPBLimit, itsNormType );
    }
    else {
      for ( uInt facet=0; facet<itsNFacets*itsNFacets; facet++ )
        { itsFacetImageStores[facet]->divideModelByWeight( itsPBLimit, itsNormType ); }
    }
 }

  void SynthesisNormalizer::multiplyModelByWeight()
  {
    //    LogIO os( LogOrigin("SynthesisNormalizer", "multiplyModelByWeight",WHERE) );
    if( itsNFacets==1) {
      itsImages->multiplyModelByWeight( itsPBLimit , itsNormType );
    }
    else {
      for ( uInt facet=0; facet<itsNFacets*itsNFacets; facet++ )
        { itsFacetImageStores[facet]->multiplyModelByWeight( itsPBLimit , itsNormType); }
    }
 }


  CountedPtr<SIImageStore> SynthesisNormalizer::getImageStore()
  {
    LogIO os( LogOrigin("SynthesisNormalizer", "getImageStore", WHERE) );
    return itsImages;
  }

  void SynthesisNormalizer::setImageStore( SIImageStore* imstore )
  {
    LogIO os( LogOrigin("SynthesisNormalizer", "setImageStore", WHERE) );
    itsImages = imstore;
  }



  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  Bool SynthesisNormalizer::setupImagesOnDisk() 
  {
    LogIO os( LogOrigin("SynthesisNormalizer","setupImagesOnDisk",WHERE) );

    Bool needToGatherImages=False;

    // Check if full images exist, and open them if possible.
    Bool foundFullImage=False;
    try
      {
	itsImages = makeImageStore( itsImageName );
	foundFullImage = True;
      }
    catch(AipsError &x)
      {
	//throw( AipsError("Error in constructing a Deconvolver : "+x.getMesg()) );
	cout << "Did not find full images : " << x.getMesg() << endl;  // This should be a debug message.
	foundFullImage = False;
      }


    // Check if part images exist
    Bool foundPartImages = itsPartImageNames.nelements()>0 ? True : False ;
    itsPartImages.resize( itsPartImageNames.nelements() );

    for ( uInt part=0; part < itsPartImageNames.nelements() ; part++ )
      {
	try
	  {
	    itsPartImages[part] = makeImageStore ( itsPartImageNames[part] );
	    foundPartImages |= True;
	  }
	catch(AipsError &x)
	  {
	    //throw( AipsError("Error in constructing a Deconvolver : "+x.getMesg()) );
	    foundPartImages = False;
	  }
      }
    if( foundPartImages == False) 
      { 
	itsPartImages.resize(0); 
      }
    else // Check that all have the same shape.
      {
	AlwaysAssert( itsPartImages.nelements() > 0 , AipsError );
	IPosition tempshape = itsPartImages[0]->getShape();
	for( uInt part=1; part<itsPartImages.nelements(); part++ )
	  {
	    if( tempshape != itsPartImages[part]->getShape() )
	      {
		throw( AipsError("Shapes of partial images to be combined, do not match") );
	      }
	  }
      }


    // Make sure all images exist and are consistent with each other. At the end, itsImages should be valid
    if( foundPartImages == True ) // Partial Images exist. Check that 'full' exists, and do the gather. 
      {
	if ( foundFullImage == True ) // Full image exists. Just check that shapes match with parts.
	  {
	    os << "Partial and Full images exist. Checking if part images have the same shape as the full image : ";
	    IPosition fullshape = itsImages->getShape();
	    
	    for ( uInt part=0; part < itsPartImages.nelements() ; part++ )
	      {
		IPosition partshape = itsPartImages[part]->getShape();
		if( partshape != fullshape )
		  {
		    os << "NO" << LogIO::POST;
		    throw( AipsError("Shapes of the partial and full images on disk do not match. Cannot gather") );
		  }
	      }
	    os << "Yes" << LogIO::POST;

	  }
	else // Full image does not exist. Need to make it, using the shape and coords of part[0]
	  {
	    os << "Only partial images exist. Need to make full images" << LogIO::POST;

	    AlwaysAssert( itsPartImages.nelements() > 0, AipsError );

	    // Find an image to open and pick csys,shape from.
	    String imopen = itsPartImageNames[0]+".residual"+((itsMapperType=="multiterm")?".tt0":"");
	    Directory imdir( imopen );
	    if( ! imdir.exists() )
	      {
		imopen = itsPartImageNames[0]+".psf"+((itsMapperType=="multiterm")?".tt0":"");
		Directory imdir2( imopen );
		if( ! imdir2.exists() )
		  throw(AipsError("Cannot find partial image psf or residual for  " + itsPartImageNames[0]));
	      }

	    PagedImage<Float> temppart( imopen );
	    IPosition tempshape = temppart.shape();
	    CoordinateSystem tempcsys = temppart.coordinates();

	    Bool useweightimage = itsPartImages[0]->getUseWeightImage( *(itsPartImages[0]->sumwt()) );

	    itsImages = makeImageStore ( itsImageName, tempcsys, tempshape, useweightimage );
	    foundFullImage = True;
	  }

	// By now, all partial images and the full images exist on disk, and have the same shape.
	needToGatherImages=True;

      }
    else // No partial images supplied. Operating only with full images.
      {
	if ( foundFullImage == True ) 
	  {
	    os << "Full images exist : " << itsImageName << LogIO::POST;
	  }
	else // No full image on disk either. Error.
	  {
	    throw( AipsError("No images named " + itsImageName + " found on disk. No partial images found either.") );
	  }
      }
    
    // Set up facet Imstores..... if needed
    if( itsNFacets>1 )
      {
        
        // First, make sure that full images have been allocated before trying to make references.....
        if( ! itsImages->checkValidity(True/*psf*/, True/*res*/,True/*wgt*/,True/*model*/,False/*image*/,False/*mask*/,True/*sumwt*/ ) ) 
	    { throw(AipsError("Internal Error : Invalid ImageStore for " + itsImages->getName())); }

        //        Array<Float> ttt;
        //        (itsImages->sumwt())->get(ttt);
        //        cout << "SUMWT full : " << ttt <<  endl;
        
        uInt nIm = itsNFacets * itsNFacets;
        itsFacetImageStores.resize( nIm );
        for( uInt facet=0; facet<nIm; ++facet )
          {
            itsFacetImageStores[facet] = itsImages->getFacetImageStore(facet, nIm);

            Array<Float> qqq;
            itsFacetImageStores[facet]->sumwt()->get(qqq);
            //            cout << "SUMWTs for " << facet << " : " << qqq << endl;

          }
      }

    return needToGatherImages;
  }// end of setupImagesOnDisk


  CountedPtr<SIImageStore> SynthesisNormalizer::makeImageStore( String imagename )
  {
    if( itsMapperType == "multiterm" )
      { return new SIImageStoreMultiTerm( imagename, itsNTaylorTerms );   }
    else
      { return new SIImageStore( imagename );   }
  }


  CountedPtr<SIImageStore> SynthesisNormalizer::makeImageStore( String imagename, 
							    CoordinateSystem& csys, 
								IPosition shp, Bool useweightimage )
  {
    if( itsMapperType == "multiterm" )
      { return new SIImageStoreMultiTerm( imagename, csys, shp, itsNFacets, False, itsNTaylorTerms, useweightimage );   }
    else
      { return new SIImageStore( imagename, csys, shp, itsNFacets, False, useweightimage );   }
  }



} //# NAMESPACE CASA - END

