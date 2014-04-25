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
/*
template<class T> Bool ImageAnalysis::_getchunk(
	Array<T>& pixels, Array<Bool>& pixelMask,
	const ImageInterface<T>& image,
	const Vector<Int>& blc, const Vector<Int>& trc, const Vector<Int>& inc,
	const Vector<Int>& axes, const Bool list, const Bool dropdeg,
	const Bool getmask
) {
	*_log << LogOrigin(className(), __func__);

	IPosition iblc = IPosition(Vector<Int> (blc));
	IPosition itrc = IPosition(Vector<Int> (trc));
	IPosition imshape = image.shape();

	// Verify region.
	IPosition iinc = IPosition(inc.size());
	for (uInt i = 0; i < inc.size(); i++) {
		iinc(i) = inc[i];
	}
	LCBox::verify(iblc, itrc, iinc, imshape);
	if (list) {
		*_log << LogIO::NORMAL << "Selected bounding box " << iblc << " to "
				<< itrc << LogIO::POST;
	}

	// Get the chunk.  The mask is not returned. Leave that to getRegion
	IPosition curshape = (itrc - iblc + iinc) / iinc;
	Slicer sl(iblc, itrc, iinc, Slicer::endIsLast);
	SubImage<T> subImage(image, sl);
	IPosition iAxes = IPosition(Vector<Int> (axes));
	if (getmask) {
		LatticeUtilities::collapse(pixels, pixelMask, iAxes, subImage, dropdeg);
		return True;
	} else {
		LatticeUtilities::collapse(pixels, iAxes, subImage, dropdeg);
		return True;
	}
}
*/

}


