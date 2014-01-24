//# ImageAnalysis.cc:  Image analysis and handling tool
//# Copyright (C) 1995-2007
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
//# $Id: ImageAnalysis.cc 20491 2009-01-16 08:33:56Z gervandiepen $
//   

// PLEASE DO *NOT* ADD ADDITIONAL METHODS TO THIS CLASS

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>

#include <images/Images/ImageOpener.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/LatticeUtilities.h>

namespace casa {

template<class T> void ImageAnalysis::_destruct(ImageInterface<T>& image) {
	if((image.isPersistent()) && ((image.imageType()) == "PagedImage")) {
		ImageOpener::ImageTypes type = ImageOpener::imageType(image.name());
		if (type == ImageOpener::AIPSPP) {
			Table::relinquishAutoLocks(True);
			(static_cast<PagedImage<T>& >(image)).table().unlock();
		}
	}
}

template<class T> Bool ImageAnalysis::_getchunk(
	Array<T>& pixels, Array<Bool>& pixelMask,
	const ImageInterface<T>& image,
	const Vector<Int>& blc, const Vector<Int>& trc, const Vector<Int>& inc,
	const Vector<Int>& axes, const Bool list, const Bool dropdeg,
	const Bool getmask
) {
	*_log << LogOrigin(className(), __FUNCTION__);

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

template<class T> Bool ImageAnalysis::_putchunk(
	ImageInterface<T>& image,
	const Array<T>& pixelsArray, const Vector<Int>& blc,
	const Vector<Int>& inc, const Bool list,
	const Bool locking, const Bool replicate
) {
	*_log << LogOrigin(className(), __FUNCTION__);
	IPosition imageShape = image.shape();
	uInt ndim = imageShape.nelements();
	ThrowIf(
		pixelsArray.ndim() > ndim,
		"Pixels array has more axes than the image!"
	);

	// Verify blc value. Fill in values for blc and inc.  trc set to shape-1
	IPosition iblc = IPosition(Vector<Int> (blc));
	IPosition itrc;
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
		*_log << LogIO::NORMAL << "Selected bounding box " << sl.start()
			<< " to " << sl.end() << LogIO::POST;
	}

	// Put the pixels
	if (pixelsArray.ndim() == ndim) {
		set_cache(pixelsArray.shape());
		if (replicate) {
			LatticeUtilities::replicate(image, sl2, pixelsArray);
		}
		else {
			image.putSlice(pixelsArray, iblc, iinc);
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
		set_cache(pixelsref.shape());
		if (replicate) {
			LatticeUtilities::replicate(image, sl2, pixelsref);
		}
		else {
			image.putSlice(pixelsref, iblc, iinc);
		}
	}
	// Ensure that we reconstruct the  histograms objects
	// now that the data have changed
	deleteHist();

	Bool rstat = True;

	if (locking) {
		rstat = unlock();
	}
	return rstat;
}

} // end of  casa namespace
