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
#include <images/Images/ExtendImage.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/SubImage.h>
#include <images/Regions/WCLELMask.h>
#include <lattices/Lattices/LCMask.h>

namespace casa {

template<class T> SubImageFactory<T>::SubImageFactory() {}

template<class T> SubImage<T> SubImageFactory<T>::createSubImage(
	ImageRegion*& outRegion, ImageRegion*& outMask,
	ImageInterface<T>& inImage, const Record& region,
	const String& mask, LogIO *const &os,
	Bool writableIfPossible, const AxesSpecifier& axesSpecifier,
	const Bool extendMask
) {
	// The ImageRegion pointers must be null on entry
	// either pointer may be null on exit
	std::auto_ptr<ImageRegion> outMaskMgr(0);
    if (! mask.empty()) {
    	String mymask = mask;
    	for (uInt i=0; i<2; i++) {
    		try {
    			outMaskMgr.reset(ImageRegion::fromLatticeExpression(mymask));
    			break;
    		}
    		catch (const AipsError& x) {
    			if (i == 0) {
    				// not an LEL expression, perhaps it's a clean mask image name
    				mymask += ">=0.5";
    				continue;
    			}
    			LogIO *myos = os;
    			std::auto_ptr<LogIO> localLogMgr(0);
    			if (! myos) {
    				myos = new LogIO();
    				localLogMgr.reset(myos);
    			}
    			*myos << LogOrigin("SubImage", __FUNCTION__);
    			*myos << "Input mask specification is incorrect: "
    				<< x.getMesg() << LogIO::EXCEPTION;
    		}
    	}
	}
    if (
		extendMask && outMaskMgr.get() != 0
		&& outMaskMgr->asWCRegionPtr()->type() == "WCLELMask"
		&& ! dynamic_cast<const WCLELMask *>(
			outMaskMgr->asWCRegionPtr()
		)->getImageExpr()->shape().isEqual(inImage.shape())
	) {
		try {
			const WCRegion *wcptr = outMaskMgr->asWCRegionPtr();
			const WCLELMask *mymask = dynamic_cast<const WCLELMask *>(wcptr);
			const ImageExpr<Bool> *const imEx = mymask->getImageExpr();
			ExtendImage<Bool> exIm(*imEx, inImage.shape(), inImage.coordinates());
			outMaskMgr.reset(new ImageRegion(LCMask(exIm)));
		}
		catch (const AipsError& x) {
			LogIO *myos = os;
			std::auto_ptr<LogIO> localLogMgr(0);
			if (! myos) {
				myos = new LogIO();
				localLogMgr.reset(myos);
			}
			*myos << LogOrigin("SubImage", __FUNCTION__);
			*myos << "Unable to extend mask: " << x.getMesg() << LogIO::EXCEPTION;
		}
	}
	SubImage<T> subImage;
	// We can get away with no region processing if the region record
	// is empty and the user is not dropping degenerate axes
	if (region.nfields() == 0 && axesSpecifier.keep()) {
		subImage = (outMaskMgr.get() == 0)
			? SubImage<T>(inImage, True)
			: SubImage<T>(
				inImage, *outMaskMgr,
				writableIfPossible
			);
	}
	else {
		std::auto_ptr<ImageRegion> outRegionMgr(
			ImageRegion::fromRecord(
				os, inImage.coordinates(),
				inImage.shape(), region
			)
		);
		if (outMaskMgr.get() == 0) {
            subImage = SubImage<T>(
				inImage, *outRegionMgr,
				writableIfPossible, axesSpecifier
			);
		}
		else {
            SubImage<T> subImage0(
				inImage, *outMaskMgr, writableIfPossible
			);
			subImage = SubImage<T>(
				subImage0, *outRegionMgr,
				writableIfPossible, axesSpecifier
			);
		}
		outRegion = outRegionMgr.release();
	}
	outMask = outMaskMgr.release();
	return subImage;
}

template<class T> SubImage<T> SubImageFactory<T>::createSubImage(
	ImageInterface<T>& inImage, const Record& region,
	const String& mask, LogIO *const &os,
	Bool writableIfPossible, const AxesSpecifier& axesSpecifier,
	const Bool extendMask
) {
	ImageRegion *pRegion = 0;
	ImageRegion *pMask = 0;
	SubImage<T> mySubim = createSubImage(
		pRegion, pMask, inImage, region,
		mask, os, writableIfPossible, axesSpecifier,
		extendMask
	);
	delete pRegion;
	delete pMask;
    return mySubim;
}

} //# NAMESPACE CASA - END

