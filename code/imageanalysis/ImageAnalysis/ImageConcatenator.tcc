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

#include <imageanalysis/ImageAnalysis/ImageConcatenator.h>

#include <images/Images/ImageConcat.h>

namespace casa {

template <class T>
const String ImageConcatenator<T>::_class = "ImageConcatenator";

template <class T>
ImageConcatenator<T>::ImageConcatenator(
	SPCIIT image, const String& outname,
	Bool overwrite
) : ImageTask<T>(
		image, "", 0, "", "", "",
		"", outname, overwrite
	), _axis(-1), _tempClose(False), _relax(False) {
	this->_construct();
}

template <class T>
ImageConcatenator<T>::~ImageConcatenator() {}

template <class T>
void ImageConcatenator<T>::setAxis(int axis) {
	ThrowIf(
		axis >= (Int)this->_getImage()->ndim(),
		"Specified zero-based value of axis exceeds number of dimensions in image"
	);
	if (axis < 0) {
		ThrowIf(!
			this->_getImage()->coordinates().hasSpectralAxis(),
			"This image has no spectral axis"
		);
		_axis = this->_getImage()->coordinates().spectralAxisNumber(False);
	}
	else {
		_axis = axis;
	}
}

template <class T>
SPIIT ImageConcatenator<T>::concatenate(
	const vector<String>& imageNames
) {
	if (_axis < 0) {
		setAxis(-1);
	}
	*this->_getLog() << LogOrigin(_class, __FUNCTION__, WHERE);

	// There could be wild cards embedded in our list so expand them out
	ThrowIf(
		imageNames.size() < 1,
		"You must give at least two extant images to concatenate"
	);
	*this->_getLog() << LogIO::NORMAL << "Number of images to concatenate = "
			<< (imageNames.size() + 1) << LogIO::POST;
	DataType dataType = this->_getImage()->dataType();
	std::auto_ptr<ImageConcat<T> > pConcat(new ImageConcat<T> (_axis, _tempClose));
	SPIIT mycopy = SubImageFactory<T>::createImage(
		*this->_getImage(), "", Record(), "", False, False, False, False
	);
	pConcat->setImage(*mycopy, _relax);

	// Set the other images.  We may run into the open file limit.
	foreach_(String name, imageNames) {
		Bool doneOpen = False;
		try {
			SPIIT im2 = ImageUtilities::openImage<T>(name);
			ThrowIf(
				im2->dataType() != dataType,
				"Concatenation of images of different data types is not supported"
			);
			doneOpen = True;
			pConcat->setImage(*im2, _relax);
		}
		catch (const AipsError& x) {
			ThrowIf(doneOpen, x.getMesg());
			ThrowCc(
				"Failed to open file " + name
				+ "This may mean you have too many files open simultaneously. "
				"Try using tempclose=T in the imageconcat constructor. "
				"Exception message " + x.getMesg()
			);
		}
	}
	return this->_prepareOutputImage(*pConcat);
}

template <class T>
String ImageConcatenator<T>::getClass() const {
	return _class;
}

}


