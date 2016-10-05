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

#include <imageanalysis/ImageAnalysis/ImageFactory.h>

namespace casa {

template <class T>
const casacore::String ImageConcatenator<T>::_class = "ImageConcatenator";

template <class T>
ImageConcatenator<T>::ImageConcatenator(
	SPCIIT image, const casacore::String& outname,
	casacore::Bool overwrite
) : ImageTask<T>(
		image, "", 0, "", "", "",
		"", outname, overwrite
	), _axis(-1), _tempClose(false), _relax(false), _reorder(false) {
	this->_construct();
}

template <class T>
ImageConcatenator<T>::~ImageConcatenator() {}

template <class T>
void ImageConcatenator<T>::setAxis(int axis) {
	ThrowIf(
		axis >= (casacore::Int)this->_getImage()->ndim(),
		"Specified zero-based value of axis exceeds number of dimensions in image"
	);
	if (axis < 0) {
		ThrowIf(!
			this->_getImage()->coordinates().hasSpectralAxis(),
			"This image has no spectral axis"
		);
		_axis = this->_getImage()->coordinates().spectralAxisNumber(false);
	}
	else {
		_axis = axis;
	}
}

template <class T>
SPIIT ImageConcatenator<T>::concatenate(
	const vector<casacore::String>& imageNames
) {
	if (_axis < 0) {
		setAxis(-1);
	}
	*this->_getLog() << casacore::LogOrigin(_class, __func__, WHERE);
	// There could be wild cards embedded in our list so expand them out
	ThrowIf(
		imageNames.size() < 1,
		"You must give at least two extant images to concatenate"
	);
	*this->_getLog() << casacore::LogIO::NORMAL << "Number of images to concatenate = "
		<< (imageNames.size() + 1) << casacore::LogIO::POST;
	SPCIIT myImage = this->_getImage();
	const casacore::CoordinateSystem csys = myImage->coordinates();
	casacore::Int whichCoordinate, axisInCoordinate;
	csys.findPixelAxis(
		whichCoordinate, axisInCoordinate, _axis
	);
	casacore::Coordinate::Type ctype = csys.coordinate(whichCoordinate).type();
	DataType dataType = myImage->dataType();
	casacore::Vector<casacore::Double> pix = csys.referencePixel();
	vector<casacore::String> imageList;
	imageList.push_back(myImage->name());
	//if (! _reorder) {
		imageList.insert(
			imageList.end(), imageNames.begin(), imageNames.end()
		);
	//}
	casacore::String myName = myImage->name();
	casacore::Bool isIncreasing = false;

	vector<casacore::Double> minVals, maxVals;
	casacore::uInt n = 0;
	if (! _relax || _reorder) {
		n = imageList.size();
		minVals.resize(n);
		maxVals.resize(n);
		isIncreasing = _minMaxAxisValues(
			minVals[0], maxVals[0], myImage->ndim(),
			csys, myImage->shape()
		);
	}
	casacore::uInt i = 1;
	for(casacore::String name: imageNames) {
		auto mypair = ImageFactory::fromFile(name);
		auto oDType = mypair.first ? mypair.first->dataType() : mypair.second->dataType();
		const auto& oCsys = mypair.first
			? mypair.first->coordinates()
			: mypair.second->coordinates();
		ThrowIf(
			oDType != dataType,
			"Concatenation of images of different data types is not supported"
		);
		if (! _relax || _reorder) {
			ThrowIf(
				_minMaxAxisValues(
					minVals[i], maxVals[i],
					mypair.first ? mypair.first->ndim() : mypair.second->ndim(),
					oCsys,
					mypair.first
						? mypair.first->shape()
						: mypair.second->shape()
				) != isIncreasing,
				"Coordinate axes in different images with opposing increment signs "
				"is not permitted if relax=false or reorder=true"
			);
		}
		if (! _relax) {
			oCsys.findPixelAxis(
				whichCoordinate, axisInCoordinate, _axis
			);
			ThrowIf(
				oCsys.coordinate(whichCoordinate).type() != ctype,
				"Cannot concatenate different coordinates in different images "
				"if relax=false"
			);
		}
		++i;
	}
	if (_reorder) {
		casacore::Sort sorter;
		sorter.sortKey(
			minVals.data(), TpDouble, 0,
			isIncreasing ? casacore::Sort::Ascending : casacore::Sort::Descending
		);
		casacore::Vector<casacore::uInt> indices;
		sorter.sort(indices, n);
		vector<casacore::String> tmp = imageList;
		vector<casacore::String>::iterator iter = tmp.begin();
		vector<casacore::String>::iterator end = tmp.end();
		casacore::Vector<casacore::uInt>::const_iterator index = indices.begin();
		while (iter != end) {
			*iter++ = imageList[*index++];
		}
		imageList = tmp;
		*this->_getLog() << casacore::LogIO::NORMAL
			<< "Images will be concatenated in the order "
			<< imageList << " and the coordinate system of "
			<< imageList[0] << " will be used as the reference"
			<< casacore::LogIO::POST;
	}
	std::unique_ptr<casacore::ImageConcat<T> > pConcat(new casacore::ImageConcat<T> (_axis, _tempClose));
	ThrowIf(
		! pConcat.get(), "Failed to create casacore::ImageConcat object"
	);
	for(casacore::String name: imageList) {
		_addImage(pConcat, name);
	}
	return this->_prepareOutputImage(*pConcat);
}

template <class T> void ImageConcatenator<T>::_addImage(
	std::unique_ptr<casacore::ImageConcat<T> >& pConcat, const casacore::String& name
) const {
	if (name == this->_getImage()->name()) {
		SPIIT mycopy = SubImageFactory<T>::createImage(
			*this->_getImage(), "", casacore::Record(), "", false, false, false, false
		);
		pConcat->setImage(*mycopy, _relax);
		return;
	}
	casacore::Bool doneOpen = false;
	try {
		SPIIT im2 = casacore::ImageUtilities::openImage<T>(name);
		doneOpen = true;
		pConcat->setImage(*im2, _relax);
	}
	catch (const casacore::AipsError& x) {
		ThrowIf(doneOpen, x.getMesg());
		ThrowCc(
			"Failed to open file " + name
			+ "This may mean you have too many files open simultaneously. "
			"Try using tempclose=T in the imageconcat constructor. "
			"Exception message " + x.getMesg()
		);
	}
}

template <class T> casacore::Bool ImageConcatenator<T>::_minMaxAxisValues(
	casacore::Double& mymin, casacore::Double& mymax, casacore::uInt ndim, const casacore::CoordinateSystem& csys,
	const casacore::IPosition& shape
) const {
	ThrowIf(
		ndim != this->_getImage()->ndim(),
		"All images must have the same number of dimensions"
	);
	casacore::Vector<casacore::Double> pix = csys.referencePixel();
	pix[_axis] = 0;
	mymin = csys.toWorld(pix)[_axis];
	if (shape[_axis] == 1) {
		mymax = mymin;
		return this->_getImage()->coordinates().increment()[_axis] > 0;
	}
	pix[_axis] = shape[_axis] - 1;
	mymax = csys.toWorld(pix)[_axis];
	casacore::Bool isIncreasing = mymax > mymin;
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


