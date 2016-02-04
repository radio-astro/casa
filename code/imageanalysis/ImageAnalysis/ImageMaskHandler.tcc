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

#include <imageanalysis/ImageAnalysis/ImageFactory.h>

#include <casa/Exceptions/Error.h>
#include <images/Images/ImageExprParse.h>
#include <images/Images/ImageProxy.h>
#include <images/Regions/RegionHandler.h>

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

template <class T> template<class U> void ImageMaskHandler<T>::copy(
    const MaskedLattice<U>& mask
) {
    auto shape = _image->shape();
    ThrowIf (
        ! shape.isEqual(mask.shape()),
        "Mask must be the same shape as the image"
    );
    auto cursorShape = _image->niceCursorShape(4096*4096);
    LatticeStepper stepper(shape, cursorShape, LatticeStepper::RESIZE);
    if (! _image->hasPixelMask()) {
        if (ImageMask::isAllMaskTrue(mask)) {
            // the current image has no pixel mask and the mask is all true, so
            // there is no point in copying anything.
            return;
        }
        String maskname = "";
        LogIO log;
        ImageMaskAttacher::makeMask(*_image, maskname, False, True, log, False);
    }
    Lattice<Bool>& pixelMask = _image->pixelMask();
    LatticeIterator<Bool> iter(pixelMask, stepper);
    RO_MaskedLatticeIterator<U> miter(mask, stepper);
    for (iter.reset(); ! iter.atEnd(); ++iter, ++miter) {
        auto mymask = miter.getMask();
        iter.rwCursor() = mymask;
    }
}

template <class T> void ImageMaskHandler<T>::calcmask(
	const String& mask, Record& regions,
	const String& maskName, const Bool makeDefault
) {
	ThrowIf(mask.empty(), "You must specify an expression");
	ThrowIf (
		! _image->canDefineRegion(),
		"Cannot make requested mask for this type of image"
		"It is of type" + _image->imageType()
	);
	Block<LatticeExprNode> temps;
	PtrBlock<const ImageRegion*> tempRegs;
	_makeRegionBlock(tempRegs, regions);
	LatticeExprNode node = ImageExprParse::command(mask, temps, tempRegs);

	// Delete the ImageRegions
	_makeRegionBlock(tempRegs, Record());

	// Make sure the expression is Boolean
	DataType type = node.dataType();
	ThrowIf(type != TpBool, "The expression type must be Boolean");
	_calcmask(node, maskName, makeDefault);
}

template<class T> void ImageMaskHandler<T>::_calcmask(
    const LatticeExprNode& node,
    const String& maskName, const Bool makeDefault
) {
	// Get the shape of the expression and check it matches that
	// of the output image.  We don't check that the Coordinates
	// match as that would be an un-necessary restriction.
	if (
		! node.isScalar()
		&& ! _image->shape().isEqual(node.shape())
	) {
		ostringstream os;
		os << "The shape of the expression does not conform "
			<< "with the shape of the output image"
			<< "Expression shape = " << node.shape()
			<< "Image shape      = " << _image->shape();
		ThrowCc(os.str());
	}
	// Make mask and get hold of its name.   Currently new mask is forced to
	// be default because of other problems.  Cannot use the usual ImageMaskAttacher<Float>::makeMask
	// function because I cant attach/make it default until the expression
	// has been evaluated
	// Generate mask name if not given
	String maskName2 = maskName.empty()
		? _image->makeUniqueRegionName(
			String("mask"), 0
		) : maskName;

	// Make the mask if it does not exist
	if (! _image->hasRegion(maskName2, RegionHandler::Masks)) {
		_image->makeMask(maskName2, True, False);
		LogIO log;
		log << LogOrigin("ImageMaskHandler", __func__);
		log << LogIO::NORMAL << "Created mask `" << maskName2 << "'"
			<< LogIO::POST;
		ImageRegion iR = _image->getRegion(
			maskName2, RegionHandler::Masks
		);
		LCRegion& mask = iR.asMask();
		if (node.isScalar()) {
			Bool value = node.getBool();
			mask.set(value);
		}
		else {
			mask.copyData(LatticeExpr<Bool> (node));
		}
	}
	else {
		// Access pre-existing mask.
		ImageRegion iR = _image->getRegion(
			maskName2, RegionHandler::Masks
		);
		LCRegion& mask2 = iR.asMask();
		if (node.isScalar()) {
			Bool value = node.getBool();
			mask2.set(value);
		}
		else {
			mask2.copyData(LatticeExpr<Bool> (node));
		}
	}
	if (makeDefault) {
		_image->setDefaultMask(maskName2);
	}
}

template<class T> void ImageMaskHandler<T>::_makeRegionBlock(
    PtrBlock<const ImageRegion*>& regions,
    const Record& Regions
) {
    auto n = regions.size();
    for (uInt j=0; j<n; ++j) {
        delete regions[j];
    }
    regions.resize(0, True, True);
    uInt nreg = Regions.nfields();
    if (nreg > 0) {
        regions.resize(nreg);
        regions.set(static_cast<ImageRegion*> (0));
        for (uInt i=0; i<nreg; ++i) {
            regions[i] = ImageRegion::fromRecord(Regions.asRecord(i), "");
        }
    }
}

}
