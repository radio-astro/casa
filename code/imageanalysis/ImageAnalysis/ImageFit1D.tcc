//# ImageFit1D.cc: Class to fit Spectral components to vectors in an image
//# Copyright (C) 2004
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
//#   $Id: ImageFit1D.tcc 20652 2009-07-06 05:04:32Z Malte.Marquarding $

#include <imageanalysis/ImageAnalysis/ImageFit1D.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions/Error.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <lattices/Lattices/LatticeUtilities.h>
#include <lattices/LatticeMath/LatticeMathUtil.h>
#include <components/SpectralComponents/SpectralEstimate.h>
#include <components/SpectralComponents/SpectralElement.h>
#include <casa/Utilities/Assert.h>

#include <memory>

namespace casa {

/*
template <class T> 
ImageFit1D<T>::ImageFit1D()
 : _image(0),
   _weights(0),
   _axis(0),
   _converged(False), _success(False), _isValid(True), _x(0)
{
   checkType();
}
*/

template <class T> 
ImageFit1D<T>::ImageFit1D(
	SHARED_PTR<const ImageInterface<T> > image, uInt pixelAxis
) : _image(image), _weights(), _axis(pixelAxis),
	_converged(False), _success(False), _isValid(True)
{
   //checkType();
   //setImage(image, pixelAxis);
	_construct();
}

template <class T> 
ImageFit1D<T>::ImageFit1D(
	SHARED_PTR<const ImageInterface<T> > image,
	SHARED_PTR<const ImageInterface<T> > weights,
	uInt pixelAxis
) : _image(image), _weights(weights),
   _axis(pixelAxis), _converged(False), _success(False), _isValid(True)
{
   //checkType();
   //setImage(image, pixelAxis);
   // setWeightsImage (weights);
	_construct();
}


template <class T> 
ImageFit1D<T>::ImageFit1D(const ImageFit1D<T>& other)
 : _image(other._image), _weights(other._weights),
   _axis(other._axis), _converged(False), _success(False), _isValid(True)
{
   //checkType();
   copy(other);
}

template <class T> 
ImageFit1D<T>& ImageFit1D<T>::operator=(const ImageFit1D<T>& other)
{
  if (this != &other) {
     copy(other);
  }
  return *this;
}

template <class T> ImageFit1D<T>::~ImageFit1D() {}

template <class T> 
void ImageFit1D<T>::_construct() {
	checkType();
	_resetFitter();
   AlwaysAssert(_axis < _image->ndim(), AipsError);

   if (_weights) {
	   AlwaysAssert (_image->shape().isEqual(_weights->shape()), AipsError);
   }
   else {
	   _unityWeights.resize(_image->shape()[_axis], False);
	   _unityWeights = 1.0;
   }
   _weightSlice.resize(_image->shape()[_axis], False);
   _sliceShape = IPosition(_image->ndim(), 1);
   _sliceShape[_axis] = _image->shape()[_axis];
}

/*
template <class T> 
void ImageFit1D<T>::setImage (const ImageInterface<T>& image,
                              const ImageInterface<T>& weights,
                              uInt pixelAxis)
{
	_resetFitter();
   setImage(image, pixelAxis);
   setWeightsImage(weights);
}
*/

template <class T>  void ImageFit1D<T>::setData (
	const IPosition& pos,
	/*const ImageFit1D<T>::AbcissaType abcissaType,
	const Bool doAbs, const Double* const &abscissaDivisor,
    Array<Double> (*xfunc)(const Array<Double>&), */
    Array<FitterType> (*yfunc)(const Array<FitterType>&)
) {
	_resetFitter();
	/*
	const uInt nDim = _image->ndim();
	IPosition start(nDim);
	start(_axis) = 0;
	for (uInt i=0; i<nDim; i++) {
		if (i!=_axis) {
			start(i) = pos(i);
		}
	}
	*/
	IPosition start = pos;
	start[_axis] = 0;
	// Get ordinate data

	Vector<T> y;
	y = _image->getSlice(start, _sliceShape, True);

	// Mask

	Vector<Bool> mask;
	mask = _image->getMaskSlice(start, _sliceShape, True);

	// Weights

	if (_weights.get()) {
		convertArray(_weightSlice, _weights->getSlice(start, _sliceShape, True));
	}
	else {
		_weightSlice = _unityWeights;
	}
	/*
	// Generate Abscissa

	Vector<Double> x = _x;
	if (x.size() == 0) {
		cout << "makeAbscissa" << endl;
		x = makeAbscissa(abcissaType, doAbs, abscissaDivisor);
		if (xfunc) {
			x = (*xfunc)(x);
		}
	}
	*/
	// Set data in fitter; we need to use a Double fitter at present
	Vector<FitterType> y2(y.shape());
	convertArray(y2, y);
	if (yfunc) {
		y2 = (*yfunc)(y2);
		// in some cases, the supplied function will return NAN values, eg
		// log(y) will return NAN for nonpositive y values. Just mask those.
		mask = mask && ! isNaN(y2);
	}
	ThrowIf(
		!_fitter.setData (_x, y2, mask, _weightSlice),
		_fitter.errorMessage()
	);
}

template <class T>  void ImageFit1D<T>::setData (
	const IPosition& pos,
	const ImageFit1D<T>::AbcissaType abcissaType,
	const Bool doAbs, const Double* const &abscissaDivisor,
    Array<Double> (*xfunc)(const Array<Double>&),
    Array<FitterType> (*yfunc)(const Array<FitterType>&)
) {
	_resetFitter();
	/*
	const uInt nDim = _image->ndim();
	IPosition start(nDim);
	start(_axis) = 0;
	for (uInt i=0; i<nDim; i++) {
		if (i!=_axis) {
			start(i) = pos(i);
		}
	}
	*/

	IPosition start = pos;
	start[_axis] = 0;

	// Get ordinate data

	Vector<T> y;
	y = _image->getSlice(start, _sliceShape, True);

	// Mask

	Vector<Bool> mask;
	mask = _image->getMaskSlice(start, _sliceShape, True);

	// Weights

	if (_weights.get()) {
		convertArray(_weightSlice, _weights->getSlice(start, _sliceShape, True));
	}
	else {
		_weightSlice = _unityWeights;
	}

	// Generate Abscissa

	Vector<Double> x = _x.copy();
	if (x.size() == 0) {
		x = makeAbscissa(abcissaType, doAbs, abscissaDivisor);
		if (xfunc) {
			x = (*xfunc)(x);
		}
	}
	// Set data in fitter; we need to use a Double fitter at present

	Vector<FitterType> y2(y.shape());
	convertArray(y2, y);
	if (yfunc) {
		y2 = (*yfunc)(y2);
		// in some cases, the supplied function will return NAN values, eg
		// log(y) will return NAN for nonpositive y values. Just mask those.
		mask = mask && ! isNaN(y2);
	}
	ThrowIf(
		!_fitter.setData (_x, y2, mask, _weightSlice),
		_fitter.errorMessage()
	);
}

/*
template <class T> Bool ImageFit1D<T>::setData (
		const ImageRegion& region,
		const ImageFit1D<T>::AbcissaType abcissaType,
		const Bool doAbs)
		{
	_resetFitter();
	// Make SubImage

	const SubImage<T> subImage(*_image, region, False);

	// Average over non-profile axes

	const uInt nDim = subImage.ndim();
	IPosition axes = IPosition::otherAxes(nDim, IPosition(1,_axis));
	Bool dropDeg = True;
	Vector<T> y;
	Vector<Bool> mask;
	LatticeMathUtil::collapse (y, mask, axes, subImage, dropDeg);

	// Weights

	Vector<T> weights(y.nelements());
	weights = 1.0;
	if (_weights.get()) {
		LatticeMathUtil::collapse (weights, axes, *_weights, dropDeg);
	}

	// Generate Abcissa

	Vector<Double> x = _x;
	if (x.size() == 0) {
		x = makeAbscissa(abcissaType, doAbs, 0);
	}

	// Set data in fitter; we need to use a Double fitter at present

	Vector<FitterType> y2(y.shape());
	convertArray(y2, y);
	Vector<Double> w2(weights.shape());
	convertArray(w2, weights);
	if (!_fitter.setData (x, y2, mask, w2)) {
		_error = _fitter.errorMessage();
		return False;
	}
	return True;
}
*/

template <class T> 
void ImageFit1D<T>::setGaussianElements (uInt nGauss) {
	if (nGauss > 0) {
		check();
		ThrowIf(
			!_fitter.setGaussianElements (nGauss),
			_fitter.errorMessage()
		);
	}
}

template <class T> 
Bool ImageFit1D<T>::fit () {
   // check();
   _converged = _fitter.fit();
   _success = True;
   return _converged;
}

template <class T> 
Bool ImageFit1D<T>::succeeded() const {
	return _success;
}

template <class T>
Bool ImageFit1D<T>::converged() const {
	return _converged;
}

template <class T>
Bool ImageFit1D<T>::setAbcissaState (
	String& errMsg, ImageFit1D<T>::AbcissaType& type,
    CoordinateSystem& cSys, const String& xUnit,
    const String& doppler, uInt pixelAxis
) {
	if (xUnit == "native") {
		type = ImageFit1D<T>::IM_NATIVE;
		return True;
	}
   if (xUnit.contains(String("pix"))) {
      type = ImageFit1D<T>::PIXEL;
      return True;
   }
   Unit unitKMS(String("km/s"));

// Is the axis Spectral ?

   Int pAxis, wAxis, sCoord;
   CoordinateUtil::findSpectralAxis (pAxis, wAxis, sCoord, cSys);
   Bool isSpectral = (Int(pixelAxis)==pAxis);

// Defer unit making until now as 'pix' not a valid unit

   Bool ok(False);
   Unit unit(xUnit);
   if (unit==unitKMS && isSpectral) {
      ok = CoordinateUtil::setSpectralState (errMsg, cSys, xUnit, doppler);
      type = ImageFit1D<T>::VELOCITY;
   } else {
      Vector<String> units = cSys.worldAxisUnits().copy();
      units(pixelAxis) = xUnit;
      ok = cSys.setWorldAxisUnits(units);
      if (!ok) errMsg = cSys.errorMessage();
      type = ImageFit1D<T>::IM_NATIVE;
   }
//
   return ok;
}

template <class T> 
Vector<Double> ImageFit1D<T>::makeAbscissa (
	ImageFit1D<T>::AbcissaType type,
	Bool doAbs, const Double* const &abscissaDivisor
) {
   const uInt n = _image->shape()(_axis);
   Vector<Double> x(n);

   const CoordinateSystem& csys = _image->coordinates();
   Double refPix = csys.referencePixel()(_axis);
   if (type==PIXEL) {
      indgen(x);
      if (!doAbs) {
    	  x -= refPix;
      }
      return x;
   }

// Find the pixel axis

   Int coord, axisInCoord;
   csys.findPixelAxis (coord, axisInCoord, _axis);
   if (type==VELOCITY) {
      AlwaysAssert(csys.type(coord)==Coordinate::SPECTRAL, AipsError);
      const SpectralCoordinate& sCoord = csys.spectralCoordinate(coord);
      Double world;
      for (uInt i=0; i<n; i++) {
         if (!sCoord.pixelToVelocity (world, Double(i))) {
            throw AipsError(sCoord.errorMessage());
         } else {
            if (doAbs) {
               x[i] = world;
            } else {
               Double worldRefVal;
               sCoord.pixelToVelocity (worldRefVal, refPix);
               x -= worldRefVal;
            }
         }
      }
   }
   else if (type==IM_NATIVE) {
      const Coordinate& gCoord = csys.coordinate(coord);
      Vector<Double> pixel(gCoord.referencePixel().copy());
      Vector<Double> world;

      for (uInt i=0; i<n; i++) {
         pixel(axisInCoord) = i;
         if (!gCoord.toWorld(world, pixel)) {
            throw AipsError(gCoord.errorMessage());
         }
//
         if (!doAbs) {
        	 gCoord.makeWorldRelative(world);
         }
         x[i] = world(axisInCoord);
      }
      if (abscissaDivisor) {
    	  x /= *abscissaDivisor;
      }
   } else {
      throw AipsError("Unrecognized abscissa type");
   }
   return x;

}


template <class T> 
void ImageFit1D<T>::check() const
{
   if (!_image.get()) {
      throw(AipsError("Image has not been set"));
   }
}

/*
template <class T> 
void ImageFit1D<T>::setWeightsImage (const ImageInterface<T>& image)
{
   AlwaysAssert (_image->shape().isEqual(image.shape()), AipsError);
   _weights.reset(image.cloneII());
}
*/

template <class T> 
void ImageFit1D<T>::copy(const ImageFit1D<T>& other)
{
	_image.reset(
		other._image.get()
			? other._image->cloneII()
			: 0
	);
	_weights.reset(
		other._weights.get()
			? other._weights->cloneII()
			: 0
	);

// These things are copies

   _axis = other._axis;
//
   _fitter = other._fitter;

   _converged = other._converged;
   _success = other._success;
   _isValid = other._isValid;
   _sliceShape = other._sliceShape;
   _unityWeights = other._unityWeights.copy();
}

template <class T> 
void ImageFit1D<T>::checkType() const
//
// At this point, ProfileFitter and SpectralFitter
// take the *Same* template type for X and Y
// To avoid precision problems we do it all in Double
// at the moment.  Later X<T> and Y<T> can be separated
 //
{
   FitterType* p=0;
   DataType tp = whatType(p);
   AlwaysAssert(tp==TpDouble, AipsError);
}


template <class T> 
Vector<T> ImageFit1D<T>::getEstimate (Int which) const 
{
   Vector<FitterType> e = _fitter.getEstimate(which);
   Vector<T> t(e.shape());
   convertArray (t, e);
   return t;
}


template <class T> 
Vector<T> ImageFit1D<T>::getFit (Int which) const 
{
   Vector<FitterType> f = _fitter.getFit(which);
   Vector<T> t(f.shape());
   convertArray (t, f);
   return t;
}

template <class T> 
Vector<T> ImageFit1D<T>::getResidual(Int which, Bool fit) const 
{
   Vector<FitterType> r = _fitter.getResidual(which, fit);
   Vector<T> t(r.shape());
   convertArray (t, r);
   return t;
}

template <class T>  void ImageFit1D<T>::invalidate() {
	_isValid = False;
}

template <class T>
Bool ImageFit1D<T>::isValid() const {
	return _isValid;
}

template <class T>
void ImageFit1D<T>::_resetFitter() {
	_fitter = ProfileFit1D<FitterType>();
	_fitter.setElements(_fitter.getList(False));
	_isValid = True;
	_converged = False;
	_success = False;
}


} //#End casa namespace
