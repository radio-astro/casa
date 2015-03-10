//# ImageFFT.cc: FFT an image
//# Copyright (C) 1995,1997,1998,1999,2000,2001,2002,2003
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
//# $Id: ImageFFT.cc 19940 2007-02-27 05:35:22Z Malte.Marquarding $


#include <imageanalysis/ImageAnalysis/ImageFFT.h>


#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/Unit.h>
#include <casa/Utilities/Assert.h>
#include <casa/iostream.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <lattices/LRegions/LCBox.h>
#include <lattices/LatticeMath/LatticeFFT.h>
#include <lattices/LEL/LatticeExpr.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/MaskedLatticeIterator.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/TempImage.h>

namespace casa {

ImageFFT::ImageFFT()
	: _tempImagePtr(),
	  _floatImage(),
	  _complexImage(),
	  _done(False) {}

ImageFFT::~ImageFFT() {}

ImageFFT::ImageFFT(const ImageFFT& other)
	: _tempImagePtr(), _floatImage(),
	  _complexImage(), _done(False) {
	if (this != &other) {
		if (other._tempImagePtr) {
			_tempImagePtr.reset(other._tempImagePtr->cloneII());
		}
		if (other._floatImage) {
			_floatImage.reset(other._floatImage->cloneII());
		}
		if (other._complexImage) {
			_complexImage.reset(other._complexImage->cloneII());
		}
		_done = other._done;
	}
}

ImageFFT& ImageFFT::operator=(const ImageFFT& other) {
	if (this != &other) {
		_tempImagePtr = other._tempImagePtr
			? SPIIC(other._tempImagePtr->cloneII())
			: SPIIC();
		_floatImage = other._floatImage
					? SPIIF(other._floatImage->cloneII())
					: SPIIF();
		_complexImage = other._complexImage
					? SPIIC(other._complexImage->cloneII())
					: SPIIC();
		_done = other._done;
   }
   return *this;
}

void ImageFFT::fftsky(const ImageInterface<Float>& in) {
	LogIO os(LogOrigin("ImageFFT", __FUNCTION__, WHERE));

	// Try and find the sky first.   Exception if not there

	Int dC;
	Vector<Int> pixelAxes, worldAxes;
	_findSky(dC, pixelAxes, worldAxes, in.coordinates(), True);

	// Set pointer for the input image

	_complexImage.reset();
	_floatImage.reset(in.cloneII());

	// Create TempImage

	_tempImagePtr.reset(
		new TempImage<Complex>(in.shape(), in.coordinates())
	);

	// Set new coordinate system in TempImage

	uInt dC2 = dC;
	_setSkyCoordinates (
		*_tempImagePtr, _floatImage->coordinates(),
		_floatImage->shape(), dC2
	);

	// Do complex FFT
  
	_fftsky2(*_tempImagePtr, *_floatImage, pixelAxes);
	_done = True;
}


void ImageFFT::fftsky(const ImageInterface<Complex>& in) {
	LogIO os(LogOrigin("ImageFFT", __FUNCTION__, WHERE));

	// Try and find the sky first.   Exception if not there

	Int dC;
	Vector<Int> pixelAxes, worldAxes;
	_findSky(dC, pixelAxes, worldAxes, in.coordinates(), True);

	// Set pointer for the input image

	_complexImage.reset(in.cloneII());
	_floatImage.reset();

	// Create TempImage

	_tempImagePtr.reset(
		new TempImage<Complex>(in.shape(), in.coordinates())
	);

	// Set new coordinate system in TempImage

	uInt dC2 = dC;
	_setSkyCoordinates(
		*_tempImagePtr, _complexImage->coordinates(),
		_complexImage->shape(), dC2
	);

	// Do complex FFT

	_fftsky2(*_tempImagePtr, *_complexImage, pixelAxes);
	_done = True;
}


void ImageFFT::fft(const ImageInterface<Float>& in,
                   const Vector<Bool>& axes)
{
   LogIO os(LogOrigin("ImageFFT", __FUNCTION__, WHERE));

// Check axes are ok 

   checkAxes (in.coordinates(), in.ndim(), axes);

// Set pointer for the input image

   _complexImage.reset();
   _floatImage.reset(in.cloneII());

// Create TempImage

   	_tempImagePtr.reset(
		   new TempImage<Complex>(in.shape(), in.coordinates())
    	);

// Set new coordinate system in TempImage

   _setCoordinates (
		   *_tempImagePtr, _floatImage->coordinates(),
		   axes, in.shape()
			);

// Do complex FFT
  
   fft2(*_tempImagePtr, *_floatImage, axes);
// 
   _done = True;
}

void ImageFFT::fft(const ImageInterface<Complex>& in,
                   const Vector<Bool>& axes)
{
   LogIO os(LogOrigin("ImageFFT", "fft(,,)", WHERE));

// Check axes are ok 

   checkAxes (in.coordinates(), in.ndim(), axes);

// Set pointer for the input image

   _floatImage.reset();
   _complexImage.reset(in.cloneII());

// Create TempImage

   _tempImagePtr.reset(
		 new TempImage<Complex>(in.shape(), in.coordinates())
		 );

// Set new coordinate system in TempImage

   _setCoordinates (
		   *_tempImagePtr, _complexImage->coordinates(),
		   axes, in.shape()
);

// Do complex FFT
  
   fft3(*_tempImagePtr, *_complexImage, axes);
// 
   _done = True;
}



void ImageFFT::getComplex(ImageInterface<Complex>& out)  const
{
   LogIO os(LogOrigin("ImageFFT", __FUNCTION__, WHERE));
   if (!_done) {
      os << "You must call function fft first" << LogIO::EXCEPTION;
   }
   if (!out.shape().isEqual(_tempImagePtr->shape())) {
      os << "Input and output images have inconsistent shapes" << LogIO::EXCEPTION;
   }
   out.copyData(*_tempImagePtr);
   copyMask(out);
   if (!out.setCoordinateInfo(_tempImagePtr->coordinates())) {
      os << "Could not replace CoordinateSystem in output real image" << LogIO::EXCEPTION;
   }
   copyMiscellaneous(out);
}


void ImageFFT::getReal(ImageInterface<Float>& out)  const {
	ThrowIf(
		! _done,
		"You must call function fft first"
	);
	ThrowIf(
		! out.shape().isEqual(_tempImagePtr->shape()),
		"Input and output images have inconsistent shapes"
	);
	out.copyData(LatticeExpr<Float>(real(*_tempImagePtr)));
	copyMask(out);
	ThrowIf(
		! out.setCoordinateInfo(_tempImagePtr->coordinates()),
		"Could not replace CoordinateSystem in output real image"
	);
	copyMiscellaneous(out);
}

void ImageFFT::getImaginary(ImageInterface<Float>& out) const
{
   LogIO os(LogOrigin("ImageFFT", __FUNCTION__, WHERE));
   if (!_done) {
      os << "You must call function fft first" << LogIO::EXCEPTION;
   }
   if (!out.shape().isEqual(_tempImagePtr->shape())) {
      os << "Input and output images have inconsistent shapes" << LogIO::EXCEPTION;
   }
//
   out.copyData(LatticeExpr<Float>(imag(*_tempImagePtr)));
   copyMask(out);
//
   if (!out.setCoordinateInfo(_tempImagePtr->coordinates())) {
      os << "Could not replace CoordinateSystem in output imaginary image" << LogIO::EXCEPTION;
   }
//
   copyMiscellaneous(out);
}


void ImageFFT::getAmplitude(ImageInterface<Float>& out) const
{
   LogIO os(LogOrigin("ImageFFT", "getAmplitude(,,)", WHERE));
   if (!_done) {
      os << "You must call function fft first" << LogIO::EXCEPTION;
   }
   if (!out.shape().isEqual(_tempImagePtr->shape())) {
      os << "Input and output images have inconsistent shapes" << LogIO::EXCEPTION;
   }
//
   out.copyData(LatticeExpr<Float>(abs(*_tempImagePtr)));
   copyMask(out);
//
   if (!out.setCoordinateInfo(_tempImagePtr->coordinates())) {
      os << "Could not replace CoordinateSystem in output amplitude image" << LogIO::EXCEPTION;
   }
//
   copyMiscellaneous(out);
}

void ImageFFT::getPhase(ImageInterface<Float>& out) const {
	ThrowIf(
		! _done,
		"You must call function fft first"
	);
	ThrowIf(
		! out.shape().isEqual(_tempImagePtr->shape()),
		"Input and output images have inconsistent shapes"
	);

  	out.copyData(LatticeExpr<Float>(arg(*_tempImagePtr)));
  	copyMask(out);
  	ThrowIf(
  		! out.setCoordinateInfo(_tempImagePtr->coordinates()),
  		"Could not replace CoordinateSystem in output phase image"
  	);
  	copyMiscellaneous(out);
  	out.setUnits(Unit("deg"));
}

void ImageFFT::checkAxes(const CoordinateSystem& cSys, uInt ndim, const Vector<Bool>& axes)
{
   LogIO os(LogOrigin("ImageFFT", "checkAxes(,)", WHERE));
//
   if (axes.nelements() != ndim) {
      os << "The length of the axes vector must be the number of image dimensions" << LogIO::EXCEPTION;
   }

// See if we have the sky.  If the user wishes to FFT the sky, they
// must have both sky axes in their list

   Int dC;
   Vector<Int> pixelAxes, worldAxes;
   Bool haveSky = _findSky(dC, pixelAxes, worldAxes, cSys, False);
   if (haveSky) {
      if (axes(pixelAxes(0)) || axes(pixelAxes(1))) {
         if (! (axes(pixelAxes(0)) && axes(pixelAxes(1)))) {
            os << "You must specify both the DirectionCoordinate (sky) axes to FFT" << LogIO::EXCEPTION;
         }
      }
   }  
//

}

void ImageFFT::copyMask (ImageInterface<Float>& out) const
{
   if (_floatImage) {
      copyMask(out, *_floatImage);
   } else {
      copyMask(out, *_complexImage);
   }
}

void ImageFFT::copyMask (ImageInterface<Complex>& out) const
{
   if (_floatImage) {
      copyMask(out, *_floatImage);
   } else {
      copyMask(out, *_complexImage);
   }
}


void ImageFFT::copyMask (ImageInterface<Float>& out, 
                         const ImageInterface<Float>& in) const
{
   if (in.isMasked()) {
      if (out.isMasked() && out.hasPixelMask()) {
         if (!out.pixelMask().isWritable()) {
            LogIO os(LogOrigin("ImageFFT", "copyMask(...)", WHERE));
            os << LogIO::WARN << "The input image is masked but the output image does "<< endl;
            os << "not have a writable mask.  Therefore no mask will be transferred" << LogIO::POST;
            return;
         }
      } else {
         return;
      }
   } else {
      return;
   }


// Use the same stepper for input and output.

   IPosition cursorShape = out.niceCursorShape();
   LatticeStepper stepper (out.shape(), cursorShape, LatticeStepper::RESIZE);
  
// Create an iterator for the output to setup the cache.
// It is not used, because using putSlice directly is faster and as easy.
      
   LatticeIterator<Float> dummyIter(out);
   RO_MaskedLatticeIterator<Float> iter(in, stepper);
   Lattice<Bool>& outMask = out.pixelMask();
   for (iter.reset(); !iter.atEnd(); iter++) {
      outMask.putSlice(iter.getMask(False), iter.position());
   }
}

void ImageFFT::copyMask (ImageInterface<Float>& out,
                         const ImageInterface<Complex>& in) const
{
   if (in.isMasked()) {
      if (out.isMasked() && out.hasPixelMask()) {
         if (!out.pixelMask().isWritable()) {
            LogIO os(LogOrigin("ImageFFT", "copyMask(...)", WHERE));
            os << LogIO::WARN << "The input image is masked but the output image does "<< endl;
            os << "not have a writable mask.  Therefore no mask will be transferred" << LogIO::POST;
            return;
         } 
      } else {
         return;
      }
   } else {
      return;
   }
   

// Use the same stepper for input and output.
      
   IPosition cursorShape = out.niceCursorShape();   
   LatticeStepper stepper (out.shape(), cursorShape, LatticeStepper::RESIZE);   

// Create an iterator for the output to setup the cache.
// It is not used, because using putSlice directly is faster and as easy.
      
   LatticeIterator<Float> dummyIter(out);   
   RO_MaskedLatticeIterator<Complex> iter(in, stepper);   
   Lattice<Bool>& outMask = out.pixelMask();
   for (iter.reset(); !iter.atEnd(); iter++) {
      outMask.putSlice(iter.getMask(False), iter.position());
   }
}

void ImageFFT::copyMask (ImageInterface<Complex>& out,
                         const ImageInterface<Float>& in) const
{
   if (in.isMasked()) {
      if (out.isMasked() && out.hasPixelMask()) {
         if (!out.pixelMask().isWritable()) {
            LogIO os(LogOrigin("ImageFFT", "copyMask(...)", WHERE));
            os << LogIO::WARN << "The input image is masked but the output image does "<< endl;
            os << "not have a writable mask.  Therefore no mask will be transferred" << LogIO::POST;
            return;
         } 
      } else {
         return;
      }
   } else {
      return;
   }
   

// Use the same stepper for input and output.
      
   IPosition cursorShape = out.niceCursorShape();   
   LatticeStepper stepper (out.shape(), cursorShape, LatticeStepper::RESIZE);   

// Create an iterator for the output to setup the cache.
// It is not used, because using putSlice directly is faster and as easy.
      
   LatticeIterator<Complex> dummyIter(out);   
   RO_MaskedLatticeIterator<Float> iter(in, stepper);   
   Lattice<Bool>& outMask = out.pixelMask();
   for (iter.reset(); !iter.atEnd(); iter++) {
      outMask.putSlice(iter.getMask(False), iter.position());
   }
}

void ImageFFT::copyMask (ImageInterface<Complex>& out,
                         const ImageInterface<Complex>& in) const
{
   if (in.isMasked()) {
      if (out.isMasked() && out.hasPixelMask()) {
         if (!out.pixelMask().isWritable()) {
            LogIO os(LogOrigin("ImageFFT", "copyMask(...)", WHERE));
            os << LogIO::WARN << "The input image is masked but the output image does "<< endl;
            os << "not have a writable mask.  Therefore no mask will be transferred" << LogIO::POST;
            return;
         } 
      } else {
         return;
      }
   } else {
      return;
   }
   

// Use the same stepper for input and output.
      
   IPosition cursorShape = out.niceCursorShape();   
   LatticeStepper stepper (out.shape(), cursorShape, LatticeStepper::RESIZE);   

// Create an iterator for the output to setup the cache.
// It is not used, because using putSlice directly is faster and as easy.
      
   LatticeIterator<Complex> dummyIter(out);   
   RO_MaskedLatticeIterator<Complex> iter(in, stepper);   
   Lattice<Bool>& outMask = out.pixelMask();
   for (iter.reset(); !iter.atEnd(); iter++) {
      outMask.putSlice(iter.getMask(False), iter.position());
   }
}



void ImageFFT::copyMiscellaneous (ImageInterface<Float>& out) const
{
   if (_floatImage) {
      out.setMiscInfo(_floatImage->miscInfo());
      out.setImageInfo(_floatImage->imageInfo());
      out.setUnits(_floatImage->units());
      out.appendLog(_floatImage->logger());
   } else {
      out.setMiscInfo(_complexImage->miscInfo());
      out.setImageInfo(_complexImage->imageInfo());
      out.setUnits(_complexImage->units());
      out.appendLog(_complexImage->logger());
   }
}


void ImageFFT::copyMiscellaneous (ImageInterface<Complex>& out) const
{
	cout << "out coords " << out.coordinates().worldAxisNames() << endl;
   if (_floatImage) {
      out.setMiscInfo(_floatImage->miscInfo());
      out.setImageInfo(_floatImage->imageInfo());
      out.setUnits(_floatImage->units());
      out.appendLog(_floatImage->logger());
   } else {
      out.setMiscInfo(_complexImage->miscInfo());
      out.setImageInfo(_complexImage->imageInfo());
      out.setUnits(_complexImage->units());
      out.appendLog(_complexImage->logger());
   }
}

void ImageFFT::_fftsky2(
	ImageInterface<Complex>& out,
	const ImageInterface<Float>& in,
	const Vector<Int>& pixelAxes
) {

	// Do the FFT.  Use in place complex because it does
	// all the unscrambling for me.  Replace masked values
	// by zero and then convert to Complex.  LEL is a marvel.

	if (in.isMasked()) {
		Float zero = 0.0;
		LatticeExpr<Complex> expr(toComplex(replace(in,zero)));
		out.copyData(expr);
	}
	else {
		LatticeExpr<Complex> expr(toComplex(in));
		out.copyData(expr);
	}
	Vector<Bool> whichAxes(in.ndim(), False);
	whichAxes(pixelAxes(0)) = True;
	whichAxes(pixelAxes(1)) = True;
	LatticeFFT::cfft(out, whichAxes, True);
}

void ImageFFT::_fftsky2(
	ImageInterface<Complex>& out,
	const ImageInterface<Complex>& in,
	const Vector<Int>& pixelAxes
) {
	if (in.isMasked()) {
		Complex zero(0, 0);
		LatticeExpr<Complex> expr(replace(in,zero));
		out.copyData(expr);
	}
	else {
		LatticeExpr<Complex> expr(in);
		out.copyData(expr);
	}
	Vector<Bool> whichAxes(in.ndim(), False);
	whichAxes(pixelAxes(0)) = True;
	whichAxes(pixelAxes(1)) = True;
	LatticeFFT::cfft(out, whichAxes, True);
}

void ImageFFT::fft2(ImageInterface<Complex>& out, 
                    const ImageInterface<Float>& in,
                    const Vector<Bool>& axes)
{

// Do the FFT.  Use in place complex because it does
// all the unscrambling for me.  Replace masked values
// by zero and then convert to Complex.  LEL is a marvel.

   Float zero = 0.0;
   LatticeExpr<Complex> expr(toComplex(replace(in,zero)));
   out.copyData(expr);
//
   LatticeFFT::cfft(out, axes, True);
}

void ImageFFT::fft3(ImageInterface<Complex>& out, 
                    const ImageInterface<Complex>& in,
                    const Vector<Bool>& axes)
{

// Do the FFT.  Use in place complex because it does
// all the unscrambling for me.  Replace masked values
// by zero.  LEL is a marvel.

   Complex zero(0.0,0.0);
   LatticeExpr<Complex> expr(replace(in,zero));
   out.copyData(expr);
//
   LatticeFFT::cfft(out, axes, True);
}


void ImageFFT::_setSkyCoordinates (
	ImageInterface<Complex>& out,
	const CoordinateSystem& csys,
	const IPosition& shape, uInt dC
) {

	// dC is the DC coordinate number

	// Find the input CoordinateSystem

	Vector<Int> pixelAxes = csys.pixelAxes(dC);
	AlwaysAssert(pixelAxes.nelements()==2,AipsError);

	// Set the DirectionCoordinate axes to True

	Vector<Bool> axes(csys.nPixelAxes(), False);
	axes(pixelAxes(0)) = True;
	axes(pixelAxes(1)) = True;

	// FT the CS

	SHARED_PTR<Coordinate> pC(
		csys.makeFourierCoordinate(
			axes, shape.asVector()
		)
	);

	// Replace TempImage CS with the new one

	CoordinateSystem* pC2 = (CoordinateSystem*)(pC.get());
	ThrowIf(
		! out.setCoordinateInfo(*pC2),
		"Could not replace Coordinate System in internal complex image"
	);
}

void ImageFFT::_setCoordinates (
	ImageInterface<Complex>& out,
	const CoordinateSystem& cSys,
	const Vector<Bool>& axes,
	const IPosition& shape
) {
	SHARED_PTR<Coordinate> pC(
		cSys.makeFourierCoordinate(axes, shape.asVector())
	);

	CoordinateSystem *pCS = (CoordinateSystem*)(pC.get());
	ThrowIf(
		! out.setCoordinateInfo(*pCS),
		"Could not replace Coordinate System in internal complex image"
	);
}

Bool ImageFFT::_findSky(
	Int& dC, Vector<Int>& pixelAxes,
	Vector<Int>& worldAxes, const CoordinateSystem& csys,
	Bool throwIt
) {
	if (! csys.hasDirectionCoordinate()) {
		ThrowIf(
			throwIt,
			"Coordinate system does not have a direction coordinate"
		);
		return False;
	}
	dC = csys.directionCoordinateNumber();
	pixelAxes = csys.directionAxesNumbers();
	worldAxes = csys.worldAxes(dC);
   return True;
}

} //# NAMESPACE CASA - END

