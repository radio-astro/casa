//# SynthesisParSync.cc: Implementation of Gather/Scatter for parallel major cycles
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

#include <synthesis/ImagerObjects/SynthesisParSync.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisParSync::SynthesisParSync() : 
				       itsImages(CountedPtr<SIImageStore>()),
				       itsPartImages(Vector<CountedPtr<SIImageStore> >()),
                                       itsImageName(""),
                                       itsPartImageNames(Vector<String>(0))
  {
    
  }
  
  SynthesisParSync::~SynthesisParSync() 
  {
    LogIO os( LogOrigin("SynthesisParSync","descructor",WHERE) );
    os << "SynthesisParSync destroyed" << LogIO::POST;
  }
  
  
  void SynthesisParSync::setupParSync(Record syncpars)
  {
    LogIO os( LogOrigin("SynthesisParSync","setupParSync",WHERE) );

    try
      {

      if( syncpars.isDefined("imagename") )  // A single string
	{ itsImageName = syncpars.asString( RecordFieldId("imagename")); }
      else
	{throw( AipsError("imagename not specified")); }

      if( syncpars.isDefined("partimagenames") )  // A vector of strings
	{ syncpars.get( RecordFieldId("partimagenames") , itsPartImageNames ); }
      else
	{ itsPartImageNames.resize(0); }

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading gather/scatter parameters: "+x.getMesg()) );
      }
    
  }//end of setupParSync


  void SynthesisParSync::gatherImages(Bool dopsf, Bool doresidual)
  {

    Bool needToGatherImages = setupImagesOnDisk();

    if( needToGatherImages )
      {
	LogIO os( LogOrigin("SynthesisParSync", "gatherImages",WHERE) );
	
	os << "Gather "<< (doresidual?"residual":"") << ( (dopsf&&doresidual)?",":"")  << (dopsf?"psf, weight":"") << " images : " << itsPartImageNames << " onto :" << itsImageName << LogIO::POST;
	
	AlwaysAssert( itsPartImages.nelements()>0 , AipsError );
	
	// Add intelligence to modify all only the first time, but later, only residual;
	itsImages->resetImages( /*psf*/dopsf, /*residual*/doresidual, /*weight*/dopsf ); 
	
	for( uInt part=0;part<itsPartImages.nelements();part++)
	  {
	    itsImages->addImages( itsPartImages[part], /*psf*/dopsf, /*residual*/doresidual, /*weight*/dopsf );
	  }

      }// end of image gathering.
    
    // Normalize by the weight image.
    //    divideResidualByWeight();

  }// end of gatherImages

  void SynthesisParSync::scatterModel()
  {

    divideModelByWeight(); // This is currently a no-op

    LogIO os( LogOrigin("SynthesisParSync", "scatterModel",WHERE) );

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

  

  void SynthesisParSync::divideResidualByWeight()
  {
    LogIO os( LogOrigin("SynthesisParSync", "divideResidualByWeight",WHERE) );

    itsImages->divideResidualByWeight(/* weightlimit */);

  }

  void SynthesisParSync::dividePSFByWeight()
  {
    LogIO os( LogOrigin("SynthesisParSync", "dividePSFByWeight",WHERE) );

    itsImages->dividePSFByWeight(/* weightlimit */);

  }

  void SynthesisParSync::divideModelByWeight()
  {
    LogIO os( LogOrigin("SynthesisParSync", "divideModelByWeight",WHERE) );

    // If Weight image is not specified, treat it as though it were filled with ones. 
    // ( i.e.  itsWeight = NULL )

    /// This is a no-op here.... Need a way to activate this only for A-Projection.
    ///    itsImages->divideModelByWeight( 0.1 );

  }


  CountedPtr<SIImageStore> SynthesisParSync::getImageStore()
  {
    LogIO os( LogOrigin("SynthesisParSync", "getImageStore", WHERE) );
    return itsImages;
  }

  void SynthesisParSync::setImageStore( SIImageStore* imstore )
  {
    LogIO os( LogOrigin("SynthesisParSync", "setImageStore", WHERE) );
    itsImages = imstore;
  }



  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  Bool SynthesisParSync::setupImagesOnDisk() 
  {
    LogIO os( LogOrigin("SynthesisParSync","setupImagesOnDisk",WHERE) );

    Bool needToGatherImages=False;

    // Check if full images exist, and open them if possible.
    Bool foundFullImage=False;
    try
      {
	itsImages = new SIImageStore( itsImageName );
	foundFullImage = True;
      }
    catch(AipsError &x)
      {
	//throw( AipsError("Error in constructing a Deconvolver : "+x.getMesg()) );
	foundFullImage = False;
      }


    // Check if part images exist
    Bool foundPartImages = itsPartImageNames.nelements()>0 ? True : False ;
    itsPartImages.resize( itsPartImageNames.nelements() );

    for ( uInt part=0; part < itsPartImageNames.nelements() ; part++ )
      {
	try
	  {
	    itsPartImages[part] = new SIImageStore( itsPartImageNames[part] );
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
	    PagedImage<Float> temppart( itsPartImageNames[0]+".residual" );
	    IPosition tempshape = temppart.shape();
	    CoordinateSystem tempcsys = temppart.coordinates();

	    itsImages = new SIImageStore( itsImageName, tempcsys, tempshape );
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
    
    return needToGatherImages;
  }// end of setupImagesOnDisk




} //# NAMESPACE CASA - END

