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

#include <imageanalysis/ImageAnalysis/ImageCropper.h>

#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

namespace casa {

template <class T>
const String ImageCropper<T>::_class = "ImageCropper";

template <class T>
ImageCropper<T>::ImageCropper(
	const shared_ptr<const ImageInterface<T> > image,
	const Record *const &regionRec, const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const String& outname,
	const Bool overwrite
) : ImageTask<T>(
		image, "", regionRec, box, chanInp, stokes,
		maskInp, outname, overwrite
	), _axes(std::set<uInt>()) {
	this->_construct();
}


template <class T>
ImageCropper<T>::~ImageCropper() {}

template <class T>
void ImageCropper<T>::setAxes(const std::set<uInt>& axes) {
	uInt ndim = this->_getImage()->ndim();

	if (axes.size() == 0) {
		for (uInt i=0; i< ndim; i++) {
			_axes.insert(i);
		}
	}
	else {
		std::set<uInt>::const_iterator end = axes.end();
		for (
			std::set<uInt>::const_iterator i=axes.begin();
			i!=end; i++
		) {
			if (*i >= ndim) {
				LogOrigin(_class, __FUNCTION__);
				*this->_getLog() << "Illegal axis number " << *i
					<< ". Image has only " << ndim << " dimensions"
					<< LogIO::EXCEPTION;
			}
		}
		_axes = axes;
	}
}

template <class T>
shared_ptr<ImageInterface<T> > ImageCropper<T>::crop(
	const Bool wantReturn
) const {
	*this->_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);

	auto_ptr<ImageInterface<T> > myClone(this->_getImage()->cloneII());
	SubImage<T> subImage = SubImageFactory<T>::createSubImage(
		*myClone, *this->_getRegion(), this->_getMask(),
		this->_getLog().get(), False, AxesSpecifier(), this->_getStretch(), True
	);
	*this->_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);
	Array<Bool> mask = subImage.getMask();
	if (! anyTrue(mask)) {
		*this->_getLog() << "(Sub)image is completely masked." << LogIO::EXCEPTION;
	}
	std::set<uInt>::const_iterator end = _axes.end();
	IPosition shape = mask.shape();
	IPosition blc = IPosition(shape.size(), 0);
	IPosition trc = shape - 1;

	for (
		std::set<uInt>::const_iterator i=_axes.begin();
		i!=end; i++
	) {
		Bool minFound = False;
		Bool maxFound = False;
		IPosition testblc = IPosition(shape.size(), 0);
		IPosition testtrc = shape - 1;
		uInt end1 = (uInt)(shape[*i]/2 + 1);
        IPosition step = testtrc - testblc + 1;
        step[*i] = 1;
		for (uInt j=0; j<end1; j++) {
			testblc[*i] = j;
			testtrc[*i] = j;
			Slicer slicer(testblc, testtrc, Slicer::endIsLast);
            if (! minFound && anyTrue(mask(slicer))) {
				blc[*i] = j;
				minFound = True;
			}
			testblc[*i] = shape[*i] - j - 1;
			testtrc[*i] = testblc[*i];
            slicer = Slicer(testblc, testtrc, Slicer::endIsLast);
			if (! maxFound && anyTrue(mask(slicer))) {
				trc[*i] = testblc[*i];
				maxFound = True;
			}
			if (maxFound && minFound) {
				break;
			}
		}
		AlwaysAssert(minFound && maxFound, AipsError);
	}
	LCBox lcbox(blc, trc, shape);
	SubImage<T> cropped = SubImageFactory<T>::createSubImage(
		subImage, lcbox.toRecord(""), "",
		this->_getLog().get(), False, AxesSpecifier(), False, True
	);
	shared_ptr<ImageInterface<T> > outImage( this->_prepareOutputImage(cropped));
	if (! wantReturn) {
		outImage.reset();
	}
	return outImage;
}

template <class T>
String ImageCropper<T>::getClass() const {
	return _class;
}

}


