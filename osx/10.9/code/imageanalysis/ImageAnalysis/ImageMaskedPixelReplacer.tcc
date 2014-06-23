//# tSubImage.cc: Test program for class SubImage
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

#include <imageanalysis/ImageAnalysis/ImageMaskedPixelReplacer.h>

#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

namespace casa {

template <class T>
const String ImageMaskedPixelReplacer<T>::_class = "ImageMaskedPixelReplacer";

template <class T>
ImageMaskedPixelReplacer<T>::ImageMaskedPixelReplacer(
	const SPIIT image,
	const Record *const &region,
	const String& mask
) : ImageTask<T>(
		image, "", region, "", "", "", mask, "", False
	),
	_image(image) {
	this->_construct();
}

template <class T>
void ImageMaskedPixelReplacer<T>::replace(
	const String& expr, Bool updateMask, Bool verbose
) {
	LogOrigin lor(_class, __func__);
	*this->_getLog() << lor;
	ThrowIf(
		expr.empty(),
		"You must specify an expression"
	);

	// Modify in place by writing to the image passed to the constructor.
	SubImage<T> subImage = SubImageFactory<T>::createSubImage(
		*_image, *this->_getRegion(), this->_getMask(),
		(verbose ? this->_getLog().get() : 0), True,
		AxesSpecifier(), this->_getStretch(), True
	);
	ThrowIf(
		! subImage.isWritable(),
		"This image is not writable.  It is probably "
		"a reference or expression virtual image"
	);
	ThrowIf(
		! subImage.isMasked() && ! subImage.hasPixelMask(),
		"Selected region of image has no mask"
	);
	Array<Bool> mymask(subImage.shape(), True);
	if (subImage.isMasked()) {
		mymask = mymask && subImage.getMask();
	}
	if (subImage.hasPixelMask()) {
		mymask && subImage.pixelMask().get();
	}
	ThrowIf(
		allTrue(mymask),
		"Mask for selected region has no bad pixels"
	);
	Block<LatticeExprNode> temps;
	Record tempRegions;
	PtrBlock<const ImageRegion*> tempRegs;
	_makeRegionBlock(tempRegs, tempRegions);
	LatticeExprNode node = ImageExprParse::command(expr, temps, tempRegs);
	// Delete the ImageRegions (by using an empty Record).
	_makeRegionBlock(tempRegs, Record());
	// Create the LEL expression we need.  It's like  replace(lattice, pixels)
	// where pixels is an expression itself.
	LatticeExprNode node2 = casa::replace(subImage, node);
	// Do it
	subImage.copyData(LatticeExpr<T> (node2));

	// Update the mask if desired
	if (updateMask) {
		Lattice<Bool>& mask = subImage.pixelMask();
		LatticeExprNode node(iif(!mask, True, mask));
		LatticeExpr<Bool> expr(node);
		mask.copyData(expr);
	}
	this->addHistory(lor, "Replaced values of masked pixels by " + expr);
}

template <class T>void ImageMaskedPixelReplacer<T>::_makeRegionBlock(
	PtrBlock<const ImageRegion*>& imageRegions,
	const Record& regions
) {
	for (uInt j = 0; j < imageRegions.nelements(); j++) {
		delete imageRegions[j];
	}
	imageRegions.resize(0, True, True);
	uInt nreg = regions.nfields();
	if (nreg > 0) {
		imageRegions.resize(nreg);
		imageRegions.set(static_cast<ImageRegion*> (0));
		for (uInt i = 0; i < nreg; i++) {
			imageRegions[i] = ImageRegion::fromRecord(regions.asRecord(i), "");
		}
	}
}

template <class T>
String ImageMaskedPixelReplacer<T>::getClass() const {
	return _class;
}

}


