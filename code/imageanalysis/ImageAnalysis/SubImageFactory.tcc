//# Copyright (C) 1998,1999,2000,2001,2003
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

#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

#include <casa/BasicSL/String.h>
#include <tables/LogTables/NewFile.h>
#include <images/Images/ExtendImage.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/SubImage.h>
#include <images/Images/PagedImage.h>
#include <images/Regions/WCLELMask.h>
#include <lattices/LRegions/LCMask.h>
#include <lattices/Lattices/LatticeUtilities.h>

#include <imageanalysis/ImageAnalysis/ImageMaskAttacher.h>

namespace casa {

template<class T> SubImageFactory<T>::SubImageFactory() {}

template<class T> SHARED_PTR<SubImage<T> > SubImageFactory<T>::createSubImageRW(
	CountedPtr<ImageRegion>& outRegion, CountedPtr<ImageRegion>& outMask,
	ImageInterface<T>& inImage, const Record& region,
	const String& mask, LogIO *const &os,
	const AxesSpecifier& axesSpecifier,
	Bool extendMask, Bool preserveAxesOrder
) {
	if (! mask.empty()) {
		_getMask(outMask, mask, extendMask, inImage.shape(), inImage.coordinates());
	}
	SHARED_PTR<SubImage<T> > subImage;
	// We can get away with no region processing if the region record
	// is empty and the user is not dropping degenerate axes
	if (region.nfields() == 0 && axesSpecifier.keep()) {
        subImage.reset(
        	! outMask
			? new SubImage<T>(inImage, True, axesSpecifier, preserveAxesOrder)
			: new SubImage<T>(
				inImage, *outMask,
				True, axesSpecifier, preserveAxesOrder
			)
		);
	}
	else {
		outRegion = ImageRegion::fromRecord(
			os, inImage.coordinates(),
			inImage.shape(), region
		);
		if (! outMask) {
            subImage.reset(
            	new SubImage<T>(
            		inImage, *outRegion,
            		True, axesSpecifier,
            		preserveAxesOrder
            	)
            );
		}
		else {
            // on the first pass, we need to keep all axes, the second
            // SubImage construction after this one will properly account
            // for the axes specifier
            SubImage<T> subImage0(
				inImage, *outMask, True,
				AxesSpecifier(), preserveAxesOrder
			);
			subImage.reset(
				new SubImage<T>(
					subImage0, *outRegion,
					True, axesSpecifier,
					preserveAxesOrder
				)
			);
		}
	}
	return subImage;
}

template<class T> SHARED_PTR<SubImage<T> > SubImageFactory<T>::createSubImageRW(
	ImageInterface<T>& inImage, const Record& region,
	const String& mask, LogIO *const &os,
	const AxesSpecifier& axesSpecifier,
	Bool extendMask, Bool preserveAxesOrder
) {
	CountedPtr<ImageRegion> pRegion;
	CountedPtr<ImageRegion> pMask;
	return createSubImageRW(
		pRegion, pMask, inImage, region,
		mask, os, axesSpecifier,
		extendMask, preserveAxesOrder
	);
}

template<class T> SHARED_PTR<const SubImage<T> > SubImageFactory<T>::createSubImageRO(
	CountedPtr<ImageRegion>& outRegion, CountedPtr<ImageRegion>& outMask,
	const ImageInterface<T>& inImage, const Record& region,
	const String& mask, LogIO *const &os,
	const AxesSpecifier& axesSpecifier,
	Bool extendMask, Bool preserveAxesOrder
) {
	if (! mask.empty()) {
		_getMask(outMask, mask, extendMask, inImage.shape(), inImage.coordinates());
	}
	SHARED_PTR<SubImage<T> > subImage;
	// We can get away with no region processing if the region record
	// is empty and the user is not dropping degenerate axes
	if (region.nfields() == 0 && axesSpecifier.keep()) {
        subImage.reset(
        	! outMask
			? new SubImage<T>(inImage, axesSpecifier, preserveAxesOrder)
			: new SubImage<T>(
				inImage, *outMask,
				axesSpecifier, preserveAxesOrder
			)
		);
	}
	else {
		outRegion = ImageRegion::fromRecord(
			os, inImage.coordinates(),
			inImage.shape(), region
		);
		if (! outMask) {
            subImage.reset(
            	new SubImage<T>(
            		inImage, *outRegion,
            		axesSpecifier,
            		preserveAxesOrder
            	)
            );
		}
		else {
            // on the first pass, we need to keep all axes, the second
            // SubImage construction after this one will properly account
            // for the axes specifier
            SubImage<T> subImage0(
				inImage, *outMask,
				AxesSpecifier(),
				preserveAxesOrder
			);
			subImage.reset(
				new SubImage<T>(
					subImage0, *outRegion,
					axesSpecifier,
					preserveAxesOrder
				)
			);
		}
	}
	return subImage;
}

template<class T> SHARED_PTR<const SubImage<T> > SubImageFactory<T>::createSubImageRO(
	const ImageInterface<T>& inImage, const Record& region,
	const String& mask, LogIO *const &os,
	const AxesSpecifier& axesSpecifier,
	Bool extendMask, Bool preserveAxesOrder
) {
	CountedPtr<ImageRegion> pRegion;
	CountedPtr<ImageRegion> pMask;
	return createSubImageRO(
		pRegion, pMask, inImage, region,
		mask, os, axesSpecifier,
		extendMask, preserveAxesOrder
	);
}

template<class T> SPIIT SubImageFactory<T>::createImage(
	const ImageInterface<T>& image,
	const String& outfile, const Record& region,
	const String& mask, Bool dropDegenerateAxes,
	Bool overwrite, Bool list, Bool extendMask, Bool attachMask
) {
	return createImage(
		image, outfile, region, mask, AxesSpecifier(! dropDegenerateAxes),
		overwrite, list, extendMask, attachMask
	);
}

template<class T> SPIIT SubImageFactory<T>::createImage(
	const ImageInterface<T>& image,
	const String& outfile, const Record& region,
	const String& mask, const AxesSpecifier& axesSpec,
	Bool overwrite, Bool list, Bool extendMask, Bool attachMask
) {
	LogIO log;
	log << LogOrigin("SubImageFactory", __func__);
	// Copy a portion of the image
	// Verify output file
	if (! overwrite && ! outfile.empty()) {
		NewFile validfile;
		String errmsg;
		ThrowIf(
			! validfile.valueOK(outfile, errmsg), errmsg
		);
	}
	SHARED_PTR<const SubImage<T> > x = createSubImageRO(
		image, region, mask, list ? &log : 0,
		axesSpec, extendMask
	);
	SPIIT outImage;
	if (outfile.empty()) {
		outImage.reset(
			new TempImage<T>(x->shape(), x->coordinates())
		);
	}
	else {
		outImage.reset(
			new PagedImage<T>(
				x->shape(), x->coordinates(), outfile
			)
		);
		if (list) {
			log << LogIO::NORMAL << "Creating image '" << outfile
				<< "' of shape " << outImage->shape() << LogIO::POST;
		}
	}
	ImageUtilities::copyMiscellaneous(*outImage, *x);
	if (
		attachMask
		|| (x->isMasked() && ! allTrue(x->getMask()))
		|| (x->hasPixelMask() && ! allTrue(x->pixelMask().get()))
	) {
		// if we don't already have a mask, but the user has specified that one needs to
		// be present, attach it. This needs to be done prior to the copyDataAndMask() call
		// because in that implementation, the image to which the mask is to be copied must
		// have already have a mask; that call does not create one if it does not exist.
		String maskName = "";
		ImageMaskAttacher::makeMask(*outImage, maskName, False, True, log, list);
	}
	LatticeUtilities::copyDataAndMask(log, *outImage, *x);
    outImage->flush();
    return outImage;
}

template<class T> void SubImageFactory<T>::_getMask(
	CountedPtr<ImageRegion>& outMask, const String& mask,
	Bool extendMask, const IPosition& imageShape,
	const CoordinateSystem& csys
) {
	String mymask = mask;
	for (uInt i=0; i<2; i++) {
		try {
			outMask = ImageRegion::fromLatticeExpression(mymask);
			break;
		}
		catch (const AipsError& x) {
			if (i == 0) {
				// not an LEL expression, perhaps it's a clean mask image name
				mymask += ">=0.5";
				continue;
			}
			ThrowCc("Input mask specification is incorrect: " + x.getMesg());
		}
	}
    if (outMask && outMask->asWCRegion().type() == "WCLELMask") {
    	const ImageExpr<Bool> *myExpression = dynamic_cast<const WCLELMask*>(
    		outMask->asWCRegionPtr()
    	)->getImageExpr();
    	if (
    		myExpression
    		&& ! myExpression->shape().isEqual(imageShape)
    	) {
    		ThrowIf(
    			! extendMask,
    			"The input image shape and mask shape are different, and it was specified "
    			"that the mask should not be extended, so the mask cannot be applied to the "
    			"(sub)image. Specifying that the mask should be extended may resolve the issue"
    		);
    		try {
    			ExtendImage<Bool> exIm(*myExpression, imageShape, csys);
    			outMask = new ImageRegion(LCMask(exIm));
    		}
    		catch (const AipsError& x) {
    			ThrowCc("Unable to extend mask: " + x.getMesg());
    		}
    	}
    }
}

}
