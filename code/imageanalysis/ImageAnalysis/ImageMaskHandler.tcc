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

#include <imageanalysis/ImageAnalysis/ImageMaskHandler.h>

#include <casa/Exceptions/Error.h>

namespace casa {

template <class T> ImageMaskHandler<T>::ImageMaskHandler(SPIIT image)
	: _image(image) {}

template <class T> ImageMaskHandler<T>::~ImageMaskHandler() {}

template <class T> void ImageMaskHandler<T>::set(const String& name) {
	_image->setDefaultMask(name);
}

template <class T> String ImageMaskHandler<T>::defaultMask() const {
	return _image->getDefaultMask();
}

template <class T> void ImageMaskHandler<T>::deleteMasks(
	const std::set<String>& masks
) {
	ThrowIf(masks.empty(), "You have not supplied any mask names");
	for (const auto& mask: masks) {
		_image->removeRegion(mask, RegionHandler::Masks, False);
	}
}

template <class T> void ImageMaskHandler<T>::rename(
	const String& oldName, const String& newName
) {
	_image->renameRegion(newName, oldName, RegionHandler::Masks, False);
}

template <class T> Vector<String> ImageMaskHandler<T>::get() const {
	return _image->regionNames(RegionHandler::Masks);
}

template <class T> void ImageMaskHandler<T>::copy(
	const String& currentName, const String& newName
) {
	ThrowIf(_image->hasRegion(
		newName, RegionHandler::Any),
		"Mask " + newName + " already exists"
	);
	Vector<String> mask2 = stringToVector(currentName, ':');
	ThrowIf(mask2.size() > 2, "Illegal mask specification " + currentName);
	auto external = mask2.size() == 2;
	_image->makeMask(newName, True, False);

	if (external) {
		ImageProxy proxy(Vector<String>(1, mask2[0]), 0);
		ThrowIf(
			! proxy.shape().isEqual(_image->shape()),
			"Images have different shapes"
		);
		auto imagePair = ImageFactory::fromFile(mask2[0]);
		if (imagePair.first) {
			ImageUtilities::copyMask(
				*_image, *imagePair.first,
				newName, mask2[1], AxesSpecifier()
			);
		}
		else {
			ImageUtilities::copyMask(
				*_image, *imagePair.second,
				newName, mask2[1], AxesSpecifier()
			);
		}
	}
	else {
		ImageUtilities::copyMask(
			*_image, *_image,
			newName, mask2[0], AxesSpecifier()
		);
	}
}


}

