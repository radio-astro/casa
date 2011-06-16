//# FITSErrorImage.cc: Class providing native access to FITS images
//# Copyright (C) 2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: FITSErrorImage.cc 20859 2010-02-03 13:14:15Z gervandiepen $

#include <images/Images/FITSErrorImage.h>
#include <images/Images/FITSImage.h>
#include <lattices/Lattices/FITSMask.h>

#include <casa/iostream.h>



namespace casa { //# NAMESPACE CASA - BEGIN

FITSErrorImage::FITSErrorImage (const String& name, uInt whichRep, uInt whichHDU, ErrorType errtype)
: FITSImage(name, whichRep, whichHDU),
  errtype_p(errtype)
{
	setupMask();
}

FITSErrorImage::FITSErrorImage (const String& name, const MaskSpecifier& maskSpec, uInt whichRep, uInt whichHDU, ErrorType errtype)
: FITSImage (name, maskSpec, whichRep, whichHDU),
  errtype_p(errtype)
{
	setupMask();
}

FITSErrorImage::FITSErrorImage (const FITSErrorImage& other)
: FITSImage(other),
  errtype_p(other.errtype_p)
{
	setupMask();
}
 
FITSErrorImage& FITSErrorImage::operator=(const FITSErrorImage& other)
// 
// Assignment. Uses reference semantics
//
{
   if (this != &other) {
      FITSImage::operator= (other);

      errtype_p = other.errtype_p;
      setupMask();
   }
   return *this;
} 

FITSErrorImage::~FITSErrorImage()
{
}

ImageInterface<Float>* FITSErrorImage::cloneII() const
{
   return new FITSErrorImage (*this);
}


String FITSErrorImage::imageType() const
{
   return "FITSErrorImage";
}

Bool FITSErrorImage::doGetSlice(Array<Float>& buffer,
                           const Slicer& section)
{
	// set up the arrays
	IPosition shp = section.length();
	if (!buffer.shape().isEqual(shp)) buffer.resize(shp);
	if (!buffer_p.shape().isEqual(shp)) buffer_p.resize(shp);

	// get the data+....////
	FITSImage::doGetSlice(buffer_p, section);

	//
	Bool deletePtrD;
	const Float* pData = buffer_p.getStorage(deletePtrD);
	Bool deletePtrM;
	Float* pBuffer = buffer.getStorage(deletePtrM);

	// depending on the error type,
	// fill the resulting array with variance values
	switch (errtype_p)
	{
	case VARIANCE:
		for (uInt i=0; i<buffer.nelements(); i++)
			pBuffer[i] = pData[i];
		break;
	case SIGMA:
		for (uInt i=0; i<buffer.nelements(); i++)
			pBuffer[i] = pData[i]*pData[i];
		break;
	case INVVARIANCE:
		for (uInt i=0; i<buffer.nelements(); i++)
			if (pData[i])
				pBuffer[i] = 1.0/pData[i];
			else
				pBuffer[i] = NAN;
		break;
	case INVSIGMA:
		for (uInt i=0; i<buffer.nelements(); i++)
			if (pData[i])
				pBuffer[i] = 1.0/(pData[i]*pData[i]);
			else
				pBuffer[i] = NAN;
		break;

	}

	// re-shuffle the arrays
	buffer_p.freeStorage(pData, deletePtrD);
	buffer.putStorage(pBuffer, deletePtrM);

	return False;                            // Not a reference
} 
   

void FITSErrorImage::doPutSlice (const Array<Float>&, const IPosition&,
                            const IPosition&)
{
	// the image is read-only
	throw (AipsError ("FITSErrorImage::putSlice - "
			"is not possible as FITSErrorImage is not writable"));
}


Bool FITSErrorImage::doGetMaskSliceII (Array<Bool>& mask, const Slicer& section)
{
   IPosition shp = section.length();
   if (!mask.shape().isEqual(shp)) mask.resize(shp);
   if (!buffer_p.shape().isEqual(shp)) buffer_p.resize(shp);

   doGetSlice(buffer_p, section);

   //
   Bool deletePtrD;
   const Float* pData = buffer_p.getStorage(deletePtrD);
   Bool deletePtrM;
   Bool* pMask = mask.getStorage(deletePtrM);
   //
   for (uInt i=0; i<mask.nelements(); i++) {

   // Blanked values are NaNs.
	   pMask[i] = True;
	   if (isNaN(pData[i])) pMask[i] = False;
   }
   //
   buffer_p.freeStorage(pData, deletePtrD);
   mask.putStorage(pMask, deletePtrM);
   //

   return False;
}

void FITSErrorImage::setupMask()
{
	// for the inverse error types, switch on
	// the masking of values 0.0 (in the FITS file)
	if (errtype_p == INVVARIANCE || errtype_p == INVSIGMA){
		setMaskZero(True);
	}
}
} //# NAMESPACE CASA - END

