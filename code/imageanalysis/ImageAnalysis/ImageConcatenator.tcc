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

#include <casa/BasicSL/STLIO.h>

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
	), _axis(-1), _tempClose(False), _relax(False), _reorder(False) {
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
	*this->_getLog() << LogOrigin(_class, __func__, WHERE);
	// There could be wild cards embedded in our list so expand them out
	ThrowIf(
		imageNames.size() < 1,
		"You must give at least two extant images to concatenate"
	);
	*this->_getLog() << LogIO::NORMAL << "Number of images to concatenate = "
		<< (imageNames.size() + 1) << LogIO::POST;
	SPCIIT myImage = this->_getImage();
	const CoordinateSystem csys = myImage->coordinates();
	Int whichCoordinate, axisInCoordinate;
	csys.findPixelAxis(
		whichCoordinate, axisInCoordinate, _axis
	);
	Coordinate::Type ctype = csys.coordinate(whichCoordinate).type();
	DataType dataType = myImage->dataType();
	Vector<Double> pix = csys.referencePixel();
	vector<String> imageList;
	imageList.push_back(myImage->name());
	//if (! _reorder) {
		imageList.insert(
			imageList.end(), imageNames.begin(), imageNames.end()
		);
	//}
	String myName = myImage->name();
	Bool isIncreasing = False;
	//boost::scoped_array<Double> minVals, maxVals;
	vector<Double> minVals, maxVals;
	uInt n = 0;
	if (! _relax || _reorder) {
		n = imageList.size();
		minVals.resize(n);
		maxVals.resize(n);
		isIncreasing = _minMaxAxisValues(
			minVals[0], maxVals[0], myImage
		);
	}
	uInt i = 1;
	foreach_(String name, imageNames) {
		SPIIT im2 = ImageUtilities::openImage<T>(name);
		ThrowIf(
			im2->dataType() != dataType,
			"Concatenation of images of different data types is not supported"
		);
		if (! _relax || _reorder) {
			ThrowIf(
				_minMaxAxisValues(
					minVals[i], maxVals[i], im2
				) != isIncreasing,
				"Coordinate axes in different images with opposing increment signs "
				"is not permitted if relax=False or reorder=True"
			);
		}
		if (! _relax) {
			const CoordinateSystem tcsys = im2->coordinates();
			tcsys.findPixelAxis(
				whichCoordinate, axisInCoordinate, _axis
			);
			ThrowIf(
				tcsys.coordinate(whichCoordinate).type() != ctype,
				"Cannot concatenate different coordinates in different images "
				"if relax=False"
			);
		}
		i++;
	}
	if (_reorder) {
		Sort sorter;
		sorter.sortKey(
			minVals.data(), TpDouble, 0,
			isIncreasing ? Sort::Ascending : Sort::Descending
		);
		Vector<uInt> indices;
		sorter.sort(indices, n);
		vector<String> tmp = imageList;
		vector<String>::iterator iter = tmp.begin();
		vector<String>::iterator end = tmp.end();
		Vector<uInt>::const_iterator index = indices.begin();
		while (iter != end) {
			*iter++ = imageList[*index++];
		}
		imageList = tmp;
		*this->_getLog() << LogIO::NORMAL
			<< "Images will be concatenated in the order "
			<< imageList << " and the coordinate system of "
			<< imageList[0] << " will be used as the reference"
			<< LogIO::POST;
	}
	std::unique_ptr<ImageConcat<T> > pConcat(new ImageConcat<T> (_axis, _tempClose));
	ThrowIf(
		! pConcat.get(), "Failed to create ImageConcat object"
	);
	foreach_(String name, imageList) {
		_addImage(pConcat, name);
	}
	return this->_prepareOutputImage(*pConcat);
}

template <class T> void ImageConcatenator<T>::_addImage(
	std::unique_ptr<ImageConcat<T> >& pConcat, const String& name
) const {
	if (name == this->_getImage()->name()) {
		SPIIT mycopy = SubImageFactory<T>::createImage(
			*this->_getImage(), "", Record(), "", False, False, False, False
		);
		pConcat->setImage(*mycopy, _relax);
		return;
	}
	Bool doneOpen = False;
	try {
		SPIIT im2 = ImageUtilities::openImage<T>(name);
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

template <class T> Bool ImageConcatenator<T>::_minMaxAxisValues(
	Double& mymin, Double& mymax, SPCIIT image
) const {
	uInt ndim = image->ndim();
	ThrowIf(
		ndim != this->_getImage()->ndim(),
		"All images must have the same number of dimensions"
	);
	const CoordinateSystem csys = image->coordinates();
	Vector<Double> pix = csys.referencePixel();
	pix[_axis] = 0;
	mymin = csys.toWorld(pix)[_axis];
	IPosition shape = image->shape();
	if (shape[_axis] == 1) {
		mymax = mymin;
		return this->_getImage()->coordinates().increment()[_axis] > 0;
	}
	pix[_axis] = shape[_axis] - 1;
	mymax = csys.toWorld(pix)[_axis];
	Bool isIncreasing = mymax > mymin;
	if (! isIncreasing) {
        std::swap(mymin, mymax);
	}
	return isIncreasing;
}

template <class T>
String ImageConcatenator<T>::getClass() const {
	return _class;
}

}


