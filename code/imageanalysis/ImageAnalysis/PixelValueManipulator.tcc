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
	SPCIIT myimage = this->_getImage();
	SPCIIT subImage = SubImageFactory<T>::createImage(
		*myimage, "", *this->_getRegion(), this->_getMask(),
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


