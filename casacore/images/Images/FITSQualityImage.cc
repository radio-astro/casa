//# FITSQualityImage.cc: Class providing native access to FITS images
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
//# $Id: FITSImage.cc 20859 2010-02-03 13:14:15Z gervandiepen $

#include <images/Images/FITSQualityImage.h>

#include <images/Images/FITSImage.h>
#include <images/Images/FITSErrorImage.h>

#include <images/Images/FITSImgParser.h>
#include <fits/FITS/hdu.h>
#include <fits/FITS/fitsio.h>
#include <fits/FITS/FITSKeywordUtil.h>
#include <images/Images/ImageInfo.h>
#include <images/Images/ImageFITSConverter.h>
#include <images/Images/MaskSpecifier.h>
#include <images/Images/ImageOpener.h>
#include <lattices/Lattices/TiledShape.h>
#include <lattices/Lattices/TempLattice.h>
#include <lattices/Lattices/FITSMask.h>
#include <tables/Tables/TiledFileAccess.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/QualityCoordinate.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/Record.h>
#include <tables/LogTables/LoggerHolder.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicMath/Math.h>
#include <casa/OS/File.h>
#include <casa/Quanta/Unit.h>
#include <measures/Measures/Quality.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Utilities/ValType.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>

#include <casa/iostream.h>



namespace casa { //# NAMESPACE CASA - BEGIN

FITSQualityImage::FITSQualityImage(const String& name, uInt whichDataHDU, uInt whichErrorHDU)
: ImageInterface<Float>(),
  name_p          (name),
  fitsdata_p      (0),
  fitserror_p     (0),
  whichDataHDU_p  (whichDataHDU),
  whichErrorHDU_p (whichErrorHDU),
  isClosed_p      (False),
  isDataClosed_p  (False),
  isErrorClosed_p (False)
{
  setup();
}


FITSQualityImage::FITSQualityImage (const FITSQualityImage& other)
: ImageInterface<Float>(other),
  name_p          (other.name_p),
  fitsdata_p      (0),
  fitserror_p     (0),
  shape_p         (other.shape_p),
  whichDataHDU_p  (other.whichDataHDU_p),
  whichErrorHDU_p (other.whichErrorHDU_p),
  isClosed_p      (other.isClosed_p),
  isDataClosed_p  (other.isDataClosed_p),
  isErrorClosed_p (other.isErrorClosed_p)
{
	if (other.fitsdata_p != 0) {
		fitsdata_p = dynamic_cast<FITSImage *>(other.fitsdata_p->cloneII());
	}
	if (other.fitserror_p != 0) {
		fitserror_p = dynamic_cast<FITSErrorImage *>(other.fitserror_p->cloneII());
	}
}
 
FITSQualityImage& FITSQualityImage::operator=(const FITSQualityImage& other)
// 
// Assignment. Uses reference semantics
//
{
   if (this != &other) {
      ImageInterface<Float>::operator= (other);
      delete fitsdata_p;
      fitsdata_p = 0;
      if (other.fitsdata_p != 0) {
    	  fitsdata_p = dynamic_cast<FITSImage *>(other.fitsdata_p->cloneII());

      }
      delete fitserror_p;
      fitserror_p = 0;
      if (other.fitserror_p != 0) {
    	  fitserror_p = dynamic_cast<FITSErrorImage *>(other.fitserror_p->cloneII());
      }
      name_p          = other.name_p;
      shape_p         = other.shape_p;
      whichDataHDU_p  = other.whichDataHDU_p;
      whichErrorHDU_p = other.whichErrorHDU_p;
      isClosed_p      = other.isClosed_p;
      isDataClosed_p  = other.isDataClosed_p;
      isErrorClosed_p = other.isErrorClosed_p;
   }
   return *this;
} 


FITSQualityImage::~FITSQualityImage()
{
	if (fitsdata_p) {
		delete fitsdata_p;
		fitsdata_p=0;
	}
	if (fitserror_p){
		delete fitserror_p;
		fitserror_p=0;
	}
}


ImageInterface<Float>* FITSQualityImage::cloneII() const
{
   return new FITSQualityImage (*this);
}


String FITSQualityImage::imageType() const
{
   return "FITSQualityImage";
}

void FITSQualityImage::resize(const TiledShape&)
{
   throw (AipsError ("FITSQualityImage::resize - a FITSQualityImage is not writable"));
}

Bool FITSQualityImage::isMasked() const
{
   return True;
}

Bool FITSQualityImage::hasPixelMask() const
{
   return True;
}

const LatticeRegion* FITSQualityImage::getRegionPtr() const
{
   return 0;
}
/*
void FITSQualityImage::fillData(void)
{
	IPosition myShape = shape();
	IPosition imShape(myShape.nelements()-1);
	IPosition imStart(myShape.nelements()-1, 0);
	IPosition imStride(myShape.nelements()-1, 1);

	Array<Float> subData;
	Array<Float> subErr;

	// set the shape
	for (uInt index=0; index < imShape.nelements(); index++)
		imShape(index)  = myShape(index);

	Slicer allImg(imStart, imShape, Slicer::endIsLength);
	fitsdata_p->doGetSlice(subData, allImg);
	fitserror_p->doGetSlice(subErr, allImg);
	if (subData.ndim() == 3){
		for (uInt ix=0; ix < subData.shape()(0); ix++)
			for (uInt iy=0; iy < subData.shape()(1); iy++)
				for (uInt iz=0; iz < subData.shape()(2); iz++){
					IPosition ifrom(3, ix, iy, iz);
					IPosition itoI(4, ix, iy, iz, 0);
					IPosition itoII(4, ix, iy, iz, 1);
					allData(itoI) = subData(ifrom);
					allData(itoII) = subErr(ifrom);
				}
	}
	else if (subData.ndim() == 2){
		for (uInt ix=0; ix < subData.shape()(0); ix++)
			for (uInt iy=0; iy < subData.shape()(1); iy++){
				//buffer(ix, iy, 0) = subData(ix, iy);
				IPosition ifrom(2, ix, iy);
				IPosition itoI(4, ix, iy, 0);
				IPosition itoII(3, ix, iy, 1);
				allData(itoI) = subData(ifrom);
				allData(itoII) = subErr(ifrom);
			}
	}

}

void FITSQualityImage::fillMask(void)
{
	IPosition myShape = shape();
	IPosition imShape(myShape.nelements()-1);
	IPosition imStart(myShape.nelements()-1, 0);
	IPosition imStride(myShape.nelements()-1, 1);

	Array<Bool> subData;
	Array<Bool> subErr;

	// set the shape
	for (uInt index=0; index < imShape.nelements(); index++)
		imShape(index)  = myShape(index);

	Slicer allImg(imStart, imShape, Slicer::endIsLength);

	fitsdata_p->doGetMaskSlice(subData, allImg);
	fitserror_p->doGetMaskSlice(subErr, allImg);
	if (subData.ndim() == 3){
		for (uInt ix=0; ix < subData.shape()(0); ix++)
			for (uInt iy=0; iy < subData.shape()(1); iy++)
				for (uInt iz=0; iz < subData.shape()(2); iz++){
					IPosition ifrom(3, ix, iy, iz);
					IPosition itoI(4, ix, iy, iz, 0);
					IPosition itoII(4, ix, iy, iz, 1);
					allMask(itoI) = subData(ifrom);
					allMask(itoII) = subErr(ifrom);
				}
	}
	else if (subData.ndim() == 2){
		for (uInt ix=0; ix < subData.shape()(0); ix++)
			for (uInt iy=0; iy < subData.shape()(1); iy++){
				//buffer(ix, iy, 0) = subData(ix, iy);
				IPosition ifrom(2, ix, iy);
				IPosition itoI(4, ix, iy, 0);
				IPosition itoII(3, ix, iy, 1);
				allMask(itoI) = subData(ifrom);
				allMask(itoII) = subErr(ifrom);
			}
	}

}
*/
/*
Bool FITSQualityImage::doGetSliceII(Array<Float>& buffer,
         	               const Slicer& section)
{
	buffer.reference(allData(section.start(), section.end(), section.stride()));
	return False;
}
Bool FITSQualityImage::doGetMaskSliceII(Array<Bool>& buffer,
         	               const Slicer& section)
{
	buffer.reference(allMask(section.start(), section.end(), section.stride()));
	return False;
}
*/
Bool FITSQualityImage::doGetSlice(Array<Float>& buffer, const Slicer& section)
{
	// get the section dimension
	IPosition shp = section.length();
	uInt ndim=section.ndim();

	// resize the buffer
	if (!buffer.shape().isEqual(shp)) buffer.resize(shp);

	// set the in all except the last dimension
	IPosition tmpStart(ndim-1);
	IPosition tmpEnd(ndim-1);
	IPosition tmpStride(ndim-1);
	for (uInt index=0; index<ndim-1; index++) {
		tmpStart(index)  = section.start()(index);
		tmpEnd(index)    = section.end()(index);
		tmpStride(index) = section.stride()(index);
	}

	// generate a slicer for all except the last dimension;
	// used for getting the data from the individual extensions
	Slicer subSection(tmpStart, tmpEnd, tmpStride, Slicer::endIsLast);

	// analyze the request
	if (section.start()(ndim-1) != section.end()(ndim-1)){

		// data and error is requested
		Array<Float> subData;
		Array<Float> subError;
		Array<Float> tmp;

		// prepare the call
		// for data values
		IPosition subStart(ndim);
		IPosition subEnd(ndim);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subData.shape().isEqual(subSection.length())) subData.resize(subSection.length());

		// get the data values
		reopenDataIfNeeded();
		fitsdata_p->doGetSlice(subData, subSection);
		tempCloseData();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subData.addDegenerate(1);


		// prepare the call
		// for error values
		subStart(ndim-1) = 1;
		subEnd(ndim-1)   = 1;

		// re-size the buffer
		if (!subError.shape().isEqual(subSection.length())) subError.resize(subSection.length());

		// get the error values
		reopenErrorIfNeeded();
		fitserror_p->doGetSlice(subError, subSection);
		tempCloseError();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subError.addDegenerate(1);
	}
	else if (section.start()(ndim-1)==0) {

		// only data is requested
		Array<Float> subData;
		Array<Float> tmp;

		// prepare the call
		// for data values
		IPosition subStart(ndim);
		IPosition subEnd(ndim);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subData.shape().isEqual(subSection.length())) subData.resize(subSection.length());

		// get the data values
		reopenDataIfNeeded();
		fitsdata_p->doGetSlice(subData, subSection);
		tempCloseData();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subData.addDegenerate(1);
	}
	else if (section.start()(ndim-1)==1) {

		// only error values are requested
		Array<Float> subError;
		Array<Float> tmp;

		// prepare the call
		// for error values
		IPosition subStart(ndim, 1);
		IPosition subEnd(ndim, 1);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subError.shape().isEqual(subSection.length())) subError.resize(subSection.length());

		// get the error values
		reopenErrorIfNeeded();
		fitserror_p->doGetSlice(subError, subSection);
		tempCloseError();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subError.addDegenerate(1);
	}

	return False;
}

Bool FITSQualityImage::doGetMaskSlice(Array<Bool>& buffer, const Slicer& section)
{
	// get the section dimension
	IPosition shp = section.length();
	uInt ndim=section.ndim();

	// resize the buffer
	if (!buffer.shape().isEqual(shp)) buffer.resize(shp);

	// set the in all except the last dimension
	IPosition tmpStart(ndim-1);
	IPosition tmpEnd(ndim-1);
	IPosition tmpStride(ndim-1);
	for (uInt index=0; index<ndim-1; index++) {
		tmpStart(index)  = section.start()(index);
		tmpEnd(index)    = section.end()(index);
		tmpStride(index) = section.stride()(index);
	}

	// generate a slicer for all except the last dimension;
	// used for getting the data from the individual extensions
	Slicer subSection(tmpStart, tmpEnd, tmpStride, Slicer::endIsLast);

	// analyze the request
	if (section.start()(ndim-1) != section.end()(ndim-1)){

		// data and error is requested
		Array<Bool> subData;
		Array<Bool> subError;
		Array<Bool> tmp;

		// prepare the call
		// for data mask
		IPosition subStart(ndim);
		IPosition subEnd(ndim);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subData.shape().isEqual(subSection.length())) subData.resize(subSection.length());

		// get the data mask
		reopenDataIfNeeded();
		fitsdata_p->doGetMaskSlice(subData, subSection);
		tempCloseData();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subData.addDegenerate(1);

		// prepare the call
		// for error mask values
		subStart(ndim-1) = 1;
		subEnd(ndim-1)   = 1;

		// re-size the buffer
		if (!subError.shape().isEqual(subSection.length())) subError.resize(subSection.length());

		// get the error mask
		reopenErrorIfNeeded();
		fitserror_p->doGetMaskSlice(subError, subSection);
		tempCloseError();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subError.addDegenerate(1);
	}

	else if (section.start()(ndim-1)==0) {

		// only data is requested
		Array<Bool> subData;
		Array<Bool> tmp;

		// prepare the call
		// for data mask values
		IPosition subStart(ndim);
		IPosition subEnd(ndim);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subData.shape().isEqual(subSection.length())) subData.resize(subSection.length());

		// get the data mask
		reopenDataIfNeeded();
		fitsdata_p->doGetMaskSlice(subData, subSection);
		tempCloseData();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subData.addDegenerate(1);

	}

	else if (section.start()(ndim-1)==1) {

		// only error is requested
		Array<Bool> subError;
		Array<Bool> tmp;

		// prepare the call
		// for error mask values
		IPosition subStart(ndim);
		IPosition subEnd(ndim);
		for (uInt index=0; index<ndim-1; index++) {
			subStart(index)  = 0;
			subEnd(index)    = shp(index)-1;
		}
		subStart(ndim-1) = 0;
		subEnd(ndim-1)   = 0;

		// re-size the buffer
		if (!subError.shape().isEqual(subSection.length())) subError.resize(subSection.length());

		// get the error mask
		reopenErrorIfNeeded();
		fitserror_p->doGetMaskSlice(subError, subSection);
		tempCloseError();

		// insert the retrieved data
		// into the output buffer
		tmp.reference(buffer(subStart, subEnd));
		tmp=subError.addDegenerate(1);
	}

	return False;
}
/*
Bool FITSQualityImage::doGetSliceIII(Array<Float>& buffer,
	         	               const Slicer& section)
	{
	//reopenIfNeeded();
	cerr << "Get Data Slice" << section.start() << " " << section.end()<< endl;
	IPosition shp = section.length();
	if (!buffer.shape().isEqual(shp)) buffer.resize(shp);
	cerr << "Buffer shape: " << buffer.shape() << endl;

	uInt ndim=section.ndim();

	IPosition tmpStart(ndim-1);
	IPosition tmpEnd(ndim-1);
	IPosition tmpStride(ndim-1);

	// set the shape
	for (uInt index=0; index<tmpStart.nelements(); index++) {
		tmpStart(index)  = section.start()(index);
		tmpEnd(index)    = section.end()(index);
		tmpStride(index) = section.stride()(index);
	}

	Slicer subSection(tmpStart, tmpEnd, tmpStride, Slicer::endIsLast);
	//cerr << "IPos: " << tmpStart << tmpEnd << tmpStride << endl;
	//cout << "Original section: " << section << endl;
	//cout << "Start: " << tmpStart << " End: "<< tmpEnd << " Stride:" << tmpStride << endl;
	//cout << "Now: " << subSection << endl;
	//cerr << "sub section" <<  subSection.start() << " " << subSection.end()<< endl;
	//cerr << "buffer " <<  buffer.ndim() << endl;

	// get the shape of the buffer
	IPosition arrShape = buffer.shape();
	uInt ndimb=buffer.ndim();

	IPosition firstBlc(ndimb-1);
	IPosition secondBlc(ndimb-1);
	IPosition firstTrc(ndimb-1);
	IPosition secondTrc(ndimb-1);

	// set the shape for the sub-arrays
	for (uInt index=0; index<ndimb-1; index++) {
		firstBlc(index)=0;
		secondBlc(index)=0;
		firstTrc(index)=arrShape(index);
		secondTrc(index)=arrShape(index);
	}

	if (section.start()(ndim-1) != section.end()(ndim-1)){
		Array<Float> subData;
		Array<Float> subErr;

		//IPosition shp = subSection.length();
		//subData.resize(shp);
		//subErr.resize(shp);

		//cout << "Subsection data-error: " << subSection << endl;
		fitsdata_p->doGetSlice(subData, subSection);
		//fitserror_p->doGetSlice(subErr, subSection);
		fitsdata_p->doGetSlice(subErr, subSection);
		if (subData.ndim() == 3){
			for (uInt ix=0; ix < subData.shape()(0); ix++)
				for (uInt iy=0; iy < subData.shape()(1); iy++)
					for (uInt iz=0; iz < subData.shape()(2); iz++){
						IPosition ifrom(3, ix, iy, iz);
						IPosition itoI(4, ix, iy, iz, 0);
						IPosition itoII(4, ix, iy, iz, 1);
						buffer(itoI) = subData(ifrom);
						buffer(itoII) = subErr(ifrom);
					}
		}
		else if (subData.ndim() == 2){
			for (uInt ix=0; ix < subData.shape()(0); ix++)
				for (uInt iy=0; iy < subData.shape()(1); iy++){
					//buffer(ix, iy, 0) = subData(ix, iy);
					IPosition ifrom(2, ix, iy);
					IPosition itoI(2, ix, iy, 0);
					IPosition itoII(3, ix, iy, 1);
					buffer(itoI) = subData(ifrom);
					buffer(itoII) = subErr(ifrom);
				}
		}
		//cerr << "Sub-data: " << subData.ndim() << endl;
		//cerr << "Sub-error: " << subErr.ndim() << endl;
	}
	else if (section.start()(ndim-1) == 0)
	{
		Array<Float> subData;
		//IPosition shp = subSection.length();
		//subData.resize(shp);

		//cout << "Subsection data: " << subSection << endl;
		//cout << "Data Shape" << subData.shape() << endl;
		fitsdata_p->doGetSlice(subData, subSection);
		//cout << "got the data" << endl;
		//cerr << "Sub-data: " << subData.ndim() << endl;
		if (subData.ndim() == 3){
			for (uInt ix=0; ix < subData.shape()(0); ix++)
				for (uInt iy=0; iy < subData.shape()(1); iy++)
					for (uInt iz=0; iz < subData.shape()(2); iz++){
						IPosition ifrom(3, ix, iy, iz);
						IPosition ito(4, ix, iy, iz, 0);
						buffer(ito) = subData(ifrom);
					}
		}
		else if (subData.ndim() == 2){
			for (uInt ix=0; ix < subData.shape()(0); ix++)
				for (uInt iy=0; iy < subData.shape()(1); iy++){
					//buffer(ix, iy, 0) = subData(ix, iy);
					IPosition ifrom(2, ix, iy);
					IPosition ito(3, ix, iy, 0);
					buffer(ito) = subData(ifrom);
				}
		}
	}
	else if (section.start()(ndim-1) == 1)
	{
		Array<Float> subErr;
		//IPosition shp = subSection.length();
		//subErr.resize(shp);

		//cout << "Subsection error: " << subSection << endl;
		//fitserror_p->doGetSlice(subErr, subSection);
		fitsdata_p->doGetSlice(subErr, subSection);
		//cout << "got the error" << endl;
		//cerr << "Sub-error: " << subErr.ndim() << endl;
		if (subErr.ndim() == 3){
			for (uInt ix=0; ix < subErr.shape()(0); ix++)
				for (uInt iy=0; iy < subErr.shape()(1); iy++)
					for (uInt iz=0; iz < subErr.shape()(2); iz++){
						//buffer(ix, iy, iz, 0) = subErr(ix, iy, iz);
						IPosition ifrom(3, ix, iy, iz);
						IPosition ito(4, ix, iy, iz, 0);
						buffer(ito) = subErr(ifrom);
					}
		}
		else if (subErr.ndim() == 2){
			for (uInt ix=0; ix < subErr.shape()(0); ix++)
				for (uInt iy=0; iy < subErr.shape()(1); iy++){
					IPosition ifrom(2, ix, iy);
					IPosition ito(3, ix, iy, 0);
					buffer(ito) = subErr(ifrom);
					//buffer(ix, iy, 0) = subErr(ix, iy);
				}
		}
	}
	//cout << "Got a junk!" << endl;
	//cout << "buffer: " << buffer.shape() << endl;
	//tempClose();
   return False;
}
*/
void FITSQualityImage::doPutSlice (const Array<Float>&, const IPosition&,
                            const IPosition&)
{
   throw (AipsError ("FITSQualityImage::putSlice - "
		     "is not possible as FITSQualityImage is not writable"));
}
/*
Bool FITSQualityImage::doGetMaskSliceIII (Array<Bool>& buffer, const Slicer& section)
{
	//reopenIfNeeded();
	cerr << "Get Mask Slice" << section.start() << " " << section.end()<< endl;
	IPosition shp = section.length();
	if (!buffer.shape().isEqual(shp)) buffer.resize(shp);

	uInt ndim=section.ndim();

	IPosition tmpStart(ndim-1);
	IPosition tmpEnd(ndim-1);
	IPosition tmpStride(ndim-1);

	// set the shape
	for (uInt index=0; index<tmpStart.nelements(); index++) {
		tmpStart(index)  = section.start()(index);
		tmpEnd(index)    = section.end()(index);
		tmpStride(index) = section.stride()(index);
	}

	Slicer subSection(tmpStart, tmpEnd, tmpStride, Slicer::endIsLast);
	//cerr << "IPos: " << tmpStart << tmpEnd << tmpStride << endl;
	//cerr << "original section" << section.start() << " " << section.end()<< endl;
	//cerr << "sub section" <<  subSection.start() << " " << subSection.end()<< endl;
	//cerr << "buffer " <<  buffer.ndim() << endl;

	// get the shape of the buffer
	IPosition arrShape = buffer.shape();
	uInt ndimb=buffer.ndim();

	IPosition firstBlc(ndimb-1);
	IPosition secondBlc(ndimb-1);
	IPosition firstTrc(ndimb-1);
	IPosition secondTrc(ndimb-1);

	// set the shape for the sub-arrays
	for (uInt index=0; index<ndimb-1; index++) {
		firstBlc(index)=0;
		secondBlc(index)=0;
		firstTrc(index)=arrShape(index);
		secondTrc(index)=arrShape(index);
	}

	if (section.start()(ndim-1) != section.end()(ndim-1)){
		Array<Bool> subData;
		Array<Bool> subErr;

		fitsdata_p->doGetMaskSlice(subData, subSection);
		//fitserror_p->doGetMaskSlice(subErr, subSection);
		fitsdata_p->doGetMaskSlice(subErr, subSection);
		if (subData.ndim() == 3){
			for (uInt ix=0; ix < subData.shape()(0); ix++)
				for (uInt iy=0; iy < subData.shape()(1); iy++)
					for (uInt iz=0; iz < subData.shape()(2); iz++){
						IPosition ifrom(3, ix, iy, iz);
						IPosition itoI(4, ix, iy, iz, 0);
						IPosition itoII(4, ix, iy, iz, 1);
						buffer(itoI) = subData(ifrom);
						buffer(itoII) = subErr(ifrom);
					}
		}
		else if (subData.ndim() == 2){
			for (uInt ix=0; ix < subData.shape()(0); ix++)
				for (uInt iy=0; iy < subData.shape()(1); iy++){
					//buffer(ix, iy, 0) = subData(ix, iy);
					IPosition ifrom(2, ix, iy);
					IPosition itoI(4, ix, iy, 0);
					IPosition itoII(3, ix, iy, 1);
					buffer(itoI) = subData(ifrom);
					buffer(itoII) = subErr(ifrom);
				}
		}
		//cerr << "Sub-data: " << subData.ndim() << endl;
		//cerr << "Sub-error: " << subErr.ndim() << endl;
	}
	else if (section.start()(ndim-1) == 0)
	{
		Array<Bool> subData;

		fitsdata_p->doGetMaskSlice(subData, subSection);
		//cerr << "Sub-data: " << subData.ndim() << endl;
		if (subData.ndim() == 3){
			for (uInt ix=0; ix < subData.shape()(0); ix++)
				for (uInt iy=0; iy < subData.shape()(1); iy++)
					for (uInt iz=0; iz < subData.shape()(2); iz++){
						IPosition ifrom(3, ix, iy, iz);
						IPosition ito(4, ix, iy, iz, 0);
						buffer(ito) = subData(ifrom);
					}
		}
		else if (subData.ndim() == 2){
			for (uInt ix=0; ix < subData.shape()(0); ix++)
				for (uInt iy=0; iy < subData.shape()(1); iy++){
					//buffer(ix, iy, 0) = subData(ix, iy);
					IPosition ifrom(2, ix, iy);
					IPosition ito(3, ix, iy, 0);
					buffer(ito) = subData(ifrom);
				}
		}
	}
	else if (section.start()(ndim-1) == 1)
	{
		Array<Bool> subErr;

		//fitserror_p->doGetMaskSlice(subErr, subSection);
		fitsdata_p->doGetMaskSlice(subErr, subSection);
		//cerr << "Sub-error: " << subErr.ndim() << endl;
		if (subErr.ndim() == 3){
			for (uInt ix=0; ix < subErr.shape()(0); ix++)
				for (uInt iy=0; iy < subErr.shape()(1); iy++)
					for (uInt iz=0; iz < subErr.shape()(2); iz++){
						//buffer(ix, iy, iz, 0) = subErr(ix, iy, iz);
						IPosition ifrom(3, ix, iy, iz);
						IPosition ito(4, ix, iy, iz, 0);
						buffer(ito) = subErr(ifrom);
					}
		}
		else if (subErr.ndim() == 2){
			for (uInt ix=0; ix < subErr.shape()(0); ix++)
				for (uInt iy=0; iy < subErr.shape()(1); iy++){
					IPosition ifrom(2, ix, iy);
					IPosition ito(3, ix, iy, 0);
					buffer(ito) = subErr(ifrom);
					//buffer(ix, iy, 0) = subErr(ix, iy);
				}
		}
	}
	//cout << buffer << endl;
	//tempClose();
	cout << "Finished" << endl;
   return False;
}
*/

Bool FITSQualityImage::isPersistent() const
{
  return True;
}

Bool FITSQualityImage::isPaged() const
{
  return True;
}

Bool FITSQualityImage::isWritable() const
{
// Its too hard to implement putMaskSlice becuase
// magic blanking is used. It means we lose
// the data values if the mask is put somewhere

   return False;
}

String FITSQualityImage::name (Bool stripPath) const
{
   return fitsdata_p->name(stripPath);
}


IPosition FITSQualityImage::shape() const
{
   return shape_p.shape();
}

uInt FITSQualityImage::advisedMaxPixels() const
{
   return shape_p.tileShape().product();
}

IPosition FITSQualityImage::doNiceCursorShape (uInt) const
{
   return shape_p.tileShape();
}

Bool FITSQualityImage::ok() const
{
   return True;
}  

void FITSQualityImage::tempClose()
{
	//cout << "close!" << endl;
   if (! isClosed_p) {
	   fitsdata_p->tempClose();
	   fitserror_p->tempClose();
   }
}
void FITSQualityImage::tempCloseData()
{
   if (! isDataClosed_p) {
		//cout << "Data closed!" << endl;
	   fitsdata_p->tempClose();
   }
   isDataClosed_p = True;
}
void FITSQualityImage::tempCloseError()
{
   if (! isErrorClosed_p) {
	   //cout << "Error closed!" << endl;
	   fitserror_p->tempClose();
   }
   isErrorClosed_p = True;
}

void FITSQualityImage::reopen()
{
	//cout << "reopen!" << endl;
   if (isClosed_p) {
	   fitsdata_p->reopen();
	   fitserror_p->reopen();
   }
}

DataType FITSQualityImage::dataType () const
{
	return fitsdata_p->dataType();
}

uInt FITSQualityImage::maximumCacheSize() const
{
   reopenIfNeeded();
   return fitsdata_p->maximumCacheSize();
}

void FITSQualityImage::setMaximumCacheSize (uInt howManyPixels)
{
   reopenIfNeeded();
   fitsdata_p->setMaximumCacheSize(howManyPixels);
   fitserror_p->setMaximumCacheSize(howManyPixels);
}

void FITSQualityImage::setCacheSizeFromPath (const IPosition& sliceShape,
				      const IPosition& windowStart,
				      const IPosition& windowLength,
				      const IPosition& axisPath)
{
   reopenIfNeeded();
   fitsdata_p->setCacheSizeFromPath(sliceShape, windowStart,
			       windowLength, axisPath);
   fitserror_p->setCacheSizeFromPath(sliceShape, windowStart,
			       windowLength, axisPath);
}

void FITSQualityImage::setCacheSizeInTiles (uInt howManyTiles)
{  
   reopenIfNeeded();
   fitsdata_p->setCacheSizeInTiles(howManyTiles);
   fitserror_p->setCacheSizeInTiles(howManyTiles);
}


void FITSQualityImage::clearCache()
{
   if (! isClosed_p) {
	   fitsdata_p->clearCache();
	   fitserror_p->clearCache();
   }
}

void FITSQualityImage::showCacheStatistics (ostream& os) const
{
   reopenIfNeeded();
   os << "FITSQualityImage statistics : ";
   fitsdata_p->showCacheStatistics(os);
   fitserror_p->showCacheStatistics(os);
}



void FITSQualityImage::setup()
{
	// open the various fits extensions
	fitsdata_p  = new FITSImage(name_p, 0, whichDataHDU_p);
	fitserror_p = new FITSErrorImage(name_p, 0, whichErrorHDU_p);

	// do some checks on the input images
	Bool ok;
	ok = checkInput();

	IPosition data_shape=fitsdata_p->shape();
	IPosition mm_shape(data_shape.nelements()+1);

	// set the shape
	for (uInt index=0; index<data_shape.nelements(); index++)
		mm_shape(index) = data_shape(index);
	mm_shape(mm_shape.nelements()-1)=2;

	// grab the coo-sys of the data image image
	CoordinateSystem cSys = fitsdata_p->coordinates();

	/*
	Vector<Double> crpix(1); crpix =  0.0;
    Vector<Double> crval(1); crval =  1.0;
    Vector<Double> cdelt(1); cdelt = -1.0;
    Matrix<Double> pc(1,1); pc= 0; pc.diagonal() = 1.0;
    Vector<String> name(1);  name = "Data <-> Error";
    Vector<String> units(1); units = " ";
    LinearCoordinate lin(name, units, crval, cdelt, pc, crpix);
	 */

	Vector<Int> quality(2);
	quality(0) = Quality::DATA;
	quality(1) = Quality::ERROR;
	QualityCoordinate qualAxis(quality);
	//coords.addCoordinate(polAxis);


	cSys.addCoordinate(qualAxis);
	//cout << "Coordnates: " << cs.nCoordinates() << "pax" << cs.nPixelAxes() << " wax: " << cs.nWorldAxes() << endl;

	setCoordsMember(cSys);
	setImageInfo(fitsdata_p->imageInfo());

	// Form the tile shape.
	shape_p = TiledShape (mm_shape, TiledFileAccess::makeTileShape(mm_shape));

	// re-shape the array
	//allData.resize(shape());
	//allMask.resize(shape());

	//cout << "Filling data start" << endl;
	//fillData();
	//fillMask();
	//cout << "Filling data end" << endl;
}

Bool FITSQualityImage::checkInput(){

	// make sure the data end error extensions
	// are NOT identical
	if (whichDataHDU_p == whichErrorHDU_p)
		throw (AipsError("Data and error extensions must be different!"));

	// make sure the data and error image have the same dimension
	if (fitsdata_p->shape() != fitserror_p->shape())
		throw (AipsError("Data and error image have different shape!"));

	// make sure the data and error image have the same coordinate system
	CoordinateSystem dataCSys  = fitsdata_p->coordinates();
	CoordinateSystem errorCSys = fitserror_p->coordinates();
	if (!dataCSys.near(errorCSys, 10e-6))
		throw (AipsError("Data and error image have different coordinate system!"));

	return True;
}

void FITSQualityImage::reopenIfNeeded() const
  { if (isClosed_p){
	  //cout << "reopening" << endl;
	  fitsdata_p->reopen();
	  fitserror_p->reopen();
	  //cout << "done" << endl;
  	  }
  }
void FITSQualityImage::reopenErrorIfNeeded()
  { if (isErrorClosed_p){
	  //cout << "Error reopening" << endl;
	  fitserror_p->reopen();
	  //cout << "done" << endl;
	  isErrorClosed_p = False;
  	  }
  }
void FITSQualityImage::reopenDataIfNeeded()
  { if (isDataClosed_p){
	  //cout << "Data reopening" << endl;
	  fitsdata_p->reopen();
	  //cout << "done" << endl;
	  isDataClosed_p = False;
  	  }
  }
} //# NAMESPACE CASA - END

