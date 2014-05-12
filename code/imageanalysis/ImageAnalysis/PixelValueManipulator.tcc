//# Copyright (C) 1998,1999,2000,2001,2003
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
//# $Id: $

#include <imageanalysis/ImageAnalysis/PixelValueManipulator.h>

// #include <casa/Arrays/ArrayMath.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>

namespace casa {

template<class T> PixelValueManipulator<T>::PixelValueManipulator(
	const SPCIIT image,
	const Record *const regionRec,
	const String& mask
) : ImageTask<T>(
	image, "", regionRec, "", "", "",
	mask, "", False
), _axes() {
	this->_construct();
}

template<class T> void PixelValueManipulator<T>::setAxes(
	const IPosition& axes, Bool invert
) {
	uInt ndim = this->_getImage()->ndim();
	ThrowIf(
		axes.nelements() > ndim,
		"Too many axes, image only has "
		+ String::toString(ndim)
		+ " dimensions"
	);
	if (! axes.empty()) {
		Vector<Int> t = axes.asVector();
		ThrowIf(
			max(t) >= (Int)ndim,
			"image does not have axis " + String::toString(max(t))
		);
		ThrowIf(
			min(t) < 0, "Axis cannot be negative"
		);
	}
	_axes = invert
		? IPosition::otherAxes(ndim, axes)
		: axes;
}

template<class T> Record PixelValueManipulator<T>::get () const {
	SPCIIT subImage = SubImageFactory<T>::createImage(
		*this->_getImage(), "", *this->_getRegion(), this->_getMask(),
		False, False,
		(this->_getVerbosity() > ImageTask<T>::QUIET ? this->_getLog().get() : 0),
		this->_getStretch()
	);
	if (! _axes.empty()) {
		ImageCollapser<T> collapser(
			subImage, _axes, False, ImageCollapserData::MEAN,
			"", False
		);
		subImage = collapser.collapse();
	}
    Array<T> values;
    Array<Bool> pixelMask;
    values = subImage->get(this->_getDropDegen());
    pixelMask = Array<Bool>(values.shape(), True);
    if (subImage->hasPixelMask()) {
    	pixelMask = pixelMask && subImage->pixelMask().get(this->_getDropDegen());
    }
    Record ret;
    ret.define("values", values);
    ret.define("mask", pixelMask);
    return ret;
}

template<class T> Record PixelValueManipulator<T>::getProfile(
	uInt axis, const String& function,
	const String& unit, PixelValueManipulatorData::SpectralType specType,
	const Quantity *const restFreq, const String& frame
) const {
	 ImageCollapser<T> collapser(
		function, this->_getImage(), this->_getRegion(),
		this->_getMask(), IPosition(1, axis), True, "", ""
	 );
	 collapser.setStretch(this->_getStretch());
	 SPIIT collapsed = collapser.collapse();
	 Record ret;
	 Array<T> values = collapsed->get(True);
	 ret.define("values", values);
	 Array<Bool> mask(values.shape(), True);
	 if (collapsed->isMasked()) {
		 mask = mask && collapsed->getMask(True);
	 }
	 if (collapsed->hasPixelMask()) {
		 mask = mask && collapsed->pixelMask().get(True);
	 }
	 ret.define("mask", mask);
	 String tunit = unit;
	 tunit.downcase();
	 if (tunit.startsWith("pix")) {
		 Vector<Double> pix(collapsed->ndim(), 0);
		 Vector<Double> outputRef = collapsed->coordinates().toWorld(pix);
		 Vector<Double> inputRef = this->_getImage()->coordinates().referenceValue();
		 inputRef[axis] = outputRef[axis];
		 Vector<Double> inputPixel = this->_getImage()->coordinates().toPixel(inputRef);
		 Int count = floor(inputPixel[axis] + 0.5);
		 uInt length = values.shape()[0];
		 Vector<Int> coords = indgen(length, count, 1);
		 ret.define("coords", coords);
		 ret.define("xUnit", "pixel");
	 }
	 else {
		 ret.merge(
			_doWorld(
				collapsed, unit, specType,
				restFreq, frame, axis
			)
		 );
	 }
	 return ret;
}

template<class T> Record PixelValueManipulator<T>::_doWorld(
	SPIIT collapsed, const String& unit,
	PixelValueManipulatorData::SpectralType specType,
	const Quantity *const restFreq, const String& frame,
	uInt axis
) const {
	SPIIT tmp = SubImageFactory<T>::createImage(
		*collapsed, "", Record(), "", True, False, False, False
	);
	const CoordinateSystem csys = tmp->coordinates();
	Quantity t(0, unit);
	String axisUnit = csys.worldAxisUnits()[0];
	if (! unit.empty()) {
		_checkUnit(unit, csys, specType);
	}
	uInt length = tmp->shape()[0];
	Vector<Double> coords(length);
	Matrix<Double> pixel;
	Matrix<Double> world;
	Vector<Bool> failures;
	if (
		! frame.empty() && (unit.empty() || t.isConform(axisUnit))
		&& csys.hasSpectralAxis()
	) {
		SPCIIT subimage = SubImageFactory<T>::createImage(
			*this->_getImage(), "", *this->_getRegion(), this->_getMask(),
			False, False, False, this->_getStretch()
		);
		CoordinateSystem mycsys = subimage->coordinates();
		pixel.resize(IPosition(2, mycsys.nPixelAxes(), length));
		pixel.set(0);
		pixel.row(axis) = indgen(length, 0.0, 1.0);
		mycsys.setSpectralConversion(frame);
		mycsys.toWorldMany(world, pixel, failures);
		coords = world.row(axis);
	}
	else {
		pixel.resize(IPosition(2, 1, length));
		pixel.set(0);
		pixel.row(0) = indgen(length, 0.0, 1.0);
		csys.toWorldMany(world, pixel, failures);
		coords = world.row(0);
		if (! unit.empty()) {
			if (t.isConform(axisUnit)) {
				Quantum<Vector<Double> > q(coords, axisUnit);
				coords = q.getValue(unit);
			}
			else {
				_doNoncomformantUnit(
					coords, csys, unit, specType, restFreq, axisUnit
				);
			}
		}
	}
	Record ret;
	ret.define("coords", coords);
	ret.define("xUnit", unit.empty() ? axisUnit : unit);
	return ret;
}

template<class T> void PixelValueManipulator<T>::_doNoncomformantUnit(
	Vector<Double> coords, const CoordinateSystem& csys,
	const String& unit, PixelValueManipulatorData::SpectralType specType,
	const Quantity *const restFreq, const String& axisUnit
) const {
	ThrowIf(
		! csys.hasSpectralAxis(),
		"Units must be conformant with" + axisUnit
	);
	SpectralCoordinate sp = csys.spectralCoordinate();
	if (restFreq) {
		Double value = restFreq->getValue(axisUnit);
		sp.setRestFrequency(value, False);
		sp.selectRestFrequency(value);
	}
	Quantity t(0, unit);
	if (t.isConform("m/s")) {
		MDoppler::Types doppler;
		if (
			specType == PixelValueManipulatorData::DEFAULT
			|| specType == PixelValueManipulatorData::RELATIVISTIC
		) {
			doppler = MDoppler::RELATIVISTIC;
		}
		else if (specType == PixelValueManipulatorData::RADIO_VELOCITY) {
			doppler = MDoppler::RADIO;
		}
		else if (specType == PixelValueManipulatorData::OPTICAL_VELOCITY) {
			doppler = MDoppler::OPTICAL;
		}
		else {
			ThrowCc("Spectral type not compatible with velocity units");
		}
		sp.setVelocity(unit ,doppler);
		sp.frequencyToVelocity(coords, coords);
	}
	else {
		// unit must be conformant with meters
		sp.setWavelengthUnit(unit);
		if (
			specType == PixelValueManipulatorData::DEFAULT
			|| specType == PixelValueManipulatorData::WAVELENGTH
		) {
			sp.frequencyToWavelength(coords, coords);
		}
		else if (specType == PixelValueManipulatorData::AIR_WAVELENGTH) {
			sp.frequencyToAirWavelength(coords, coords);
		}
	}
}

template<class T> void PixelValueManipulator<T>::_checkUnit(
	const String& unit, const CoordinateSystem& csys,
	PixelValueManipulatorData::SpectralType specType
) const {
	Quantity t(0, unit);
	String axisUnit = csys.worldAxisUnits()[0];
	if (! t.isConform(axisUnit)) {
		if (csys.hasSpectralAxis()) {
			ThrowIf(
				! (t.isConform("m/s") || t.isConform("m")),
				"Invalid spectral conversion unit " + unit
			);
			ThrowIf(
				t.isConform("m/s")
				&& (
					specType == PixelValueManipulatorData::WAVELENGTH
					|| specType == PixelValueManipulatorData::AIR_WAVELENGTH
				),
				"Inconsistent spectral type used for velocity units"
			);
			ThrowIf(
				t.isConform("m")
				&& (
					specType == PixelValueManipulatorData::OPTICAL_VELOCITY
					|| specType == PixelValueManipulatorData::RADIO_VELOCITY
				),
				"Inconsistent spectral type used for wavelength units"
			);
		}
		else {
			ThrowCc(
				"Unit " + unit
				+ " does not conform to corresponding axis unit "
				+ axisUnit
			);
		}
	}
}

template<class T> void PixelValueManipulator<T>::put(
	SPIIT image, const Array<T>& pixelsArray, const Vector<Int>& blc,
	const Vector<Int>& inc, Bool list, Bool locking, Bool replicate
) {
	IPosition imageShape = image->shape();
	uInt ndim = imageShape.nelements();
	ThrowIf(
		pixelsArray.ndim() > ndim,
		"Pixel array cannot have more dimensions than the image!"
	);

	// Verify blc value. Fill in values for blc and inc.  trc set to shape-1
	IPosition iblc = IPosition(blc);
	IPosition itrc = imageShape - 1;
	IPosition iinc(inc.size());
	for (uInt i = 0; i < inc.size(); i++) {
		iinc(i) = inc[i];
	}
	LCBox::verify(iblc, itrc, iinc, imageShape);

	// Create two slicers; one describing the region defined by blc + shape-1
	// with extra axes given length 1. The other we extend with the shape
	IPosition len = pixelsArray.shape();
	len.resize(ndim, True);
	for (uInt i = pixelsArray.shape().nelements(); i < ndim; i++) {
		len(i) = 1;
		itrc(i) = imageShape(i) - 1;
	}
	Slicer sl(iblc, len, iinc, Slicer::endIsLength);
	ThrowIf(
		sl.end() + 1 > imageShape,
		"Pixels array, including inc, extends beyond edge of image."
	);
	Slicer sl2(iblc, itrc, iinc, Slicer::endIsLast);

	if (list) {
		LogIO log;
		log << LogOrigin("PixelValueManipulator", __func__)
			<< LogIO::NORMAL << "Selected bounding box " << sl.start()
			<< " to " << sl.end() << LogIO::POST;
	}

	// Put the pixels
	if (pixelsArray.ndim() == ndim) {
		// _setCache(pixelsArray.shape());
		if (replicate) {
			LatticeUtilities::replicate(*image, sl2, pixelsArray);
		}
		else {
			image->putSlice(pixelsArray, iblc, iinc);
		}
	}
	else {
		// Pad with extra degenerate axes if necessary (since it is somewhat
		// costly).
		Array<T> pixelsref(
			pixelsArray.addDegenerate(
				ndim - pixelsArray.ndim()
			)
		);
		// _setCache(pixelsref.shape());
		if (replicate) {
			LatticeUtilities::replicate(*image, sl2, pixelsref);
		}
		else {
			image->putSlice(pixelsref, iblc, iinc);
		}
	}
	// Ensure that we reconstruct the  histograms objects
	// now that the data have changed
	if (locking) {
		image->unlock();
	}
}

}


