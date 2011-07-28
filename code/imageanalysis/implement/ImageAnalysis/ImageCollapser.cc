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

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/HashMap.h>
#include <casa/Containers/HashMapIter.h>
#include <casa/OS/Directory.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Images/TempImage.h>

#include <imageanalysis/ImageAnalysis/ImageInputProcessor.h>

namespace casa {

HashMap<uInt, String> *ImageCollapser::_funcNameMap = 0;
HashMap<uInt, String> *ImageCollapser::_minMatchMap = 0;
HashMap<uInt, Float (*)(const Array<Float>&)> *ImageCollapser::_funcMap = 0;

const String ImageCollapser::_class = "ImageCollapser";

ImageCollapser::ImageCollapser(
	String aggString, const ImageInterface<Float> *const image,
	const String& region, const Record *const regionRec,
	const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const Vector<uInt> axes,
	const String& outname, const Bool overwrite
) : ImageTask(
		image, region, regionRec, box, chanInp, stokes,
		maskInp, outname, overwrite
	),
	_invertAxesSelection(False),
	_axes(axes), _aggType(UNKNOWN) {
	_aggType = aggregateType(aggString);
	_construct();
	_finishConstruction();
}

ImageCollapser::ImageCollapser(
	const ImageInterface<Float> *const image,
	const Vector<uInt>& axes, const Bool invertAxesSelection,
	const AggregateType aggregateType,
	const String& outname, const Bool overwrite
) : ImageTask(
		image, "", 0, "", "", "",
		"", outname, overwrite
	),
	_invertAxesSelection(invertAxesSelection),
	_axes(axes), _aggType(aggregateType) {
	*_log << LogOrigin(_class, __FUNCTION__);
	if (_aggType == UNKNOWN) {
		*_log << "UNKNOWN aggregateType not allowed" << LogIO::EXCEPTION;
	}
	if (! image) {
		*_log << "Cannot use a null image pointer with this constructor"
			<< LogIO::EXCEPTION;
	}
	_construct();
	_finishConstruction();
}

ImageCollapser::~ImageCollapser() {}

ImageInterface<Float>* ImageCollapser::collapse(const Bool wantReturn) const {
	*_log << LogOrigin(_class, __FUNCTION__);
	std::auto_ptr<ImageInterface<Float> > clone(_getImage()->cloneII());
	SubImage<Float> subImage = SubImage<Float>::createSubImage(
		*clone, *_getRegion(), _getMask(), _log.get(), False
	);
	clone.reset(0);
	IPosition inShape = subImage.shape();
	// Set the compressed axis reference pixel and reference value
	CoordinateSystem outCoords(subImage.coordinates());
	Vector<Double> blc, trc;
	IPosition pixblc(inShape.nelements(), 0);
	IPosition pixtrc = inShape - 1;
	if(
		! outCoords.toWorld(blc, pixblc)
		|| ! outCoords.toWorld(trc, pixtrc)
	) {
		*_log << "Could not set new coordinate values" << LogIO::EXCEPTION;
	}
	Vector<Double> refValues = outCoords.referenceValue();
	Vector<Double> refPixels = outCoords.referencePixel();
	IPosition outShape = inShape;
	IPosition shape(outShape.nelements(), 1);

	for (Vector<uInt>::const_iterator iter=_axes.begin(); iter != _axes.end(); iter++) {
		uInt i = *iter;
		refValues[i] = (blc[i] + trc[i])/2;
		refPixels[i] = 0;
		outShape[i] = 1;
		shape[i] = inShape[i];
	}

	if (! outCoords.setReferenceValue(refValues)) {
		*_log << "Unable to set reference value" << LogIO::EXCEPTION;
	}
	if (! outCoords.setReferencePixel(refPixels)) {
		*_log << "Unable to set reference pixel" << LogIO::EXCEPTION;
	}

	std::auto_ptr<ImageInterface<Float> > outImage(0);
	if (_getOutname().empty()) {
		outImage.reset(new TempImage<Float>(outShape, outCoords));
	}
	else {
		_removeExistingOutfileIfNecessary();
		outImage.reset(
			new PagedImage<Float>(outShape, outCoords, _getOutname())
		);
	}
	if (_aggType == ZERO) {
		Array<Float> zeros(outShape, 0.0);
		outImage->put(zeros);
	}
	else {
		Float (*function)(const Array<Float>&) = (*funcMap())(_aggType);
		Array<Float> data = subImage.get(False);
		for (uInt i=0; i<outShape.product(); i++) {
			IPosition start = toIPositionInArray(i, outShape);
			IPosition end = start + shape - 1;
			Slicer s(start, end, Slicer::endIsLast);
			outImage->putAt(function(data(s)), start);
		}
	}
	if (! _getOutname().empty()) {
		outImage->flush();
	}
	if (! wantReturn) {
		outImage.reset(0);
	}
	return outImage.release();
}

const HashMap<uInt, Float (*)(const Array<Float>&)>* ImageCollapser::funcMap() {
	if (! _funcMap) {
		_funcMap = new HashMap<uInt, Float (*)(const Array<Float>&)>(casa::mean<Float>);
		_funcMap->define((uInt)AVDEV, casa::avdev);
		_funcMap->define((uInt)MAX, casa::max);
		_funcMap->define((uInt)MEAN, casa::mean);
		_funcMap->define((uInt)MEDIAN, casa::median);
		_funcMap->define((uInt)MIN, casa::min);
		_funcMap->define((uInt)RMS, casa::rms);
		_funcMap->define((uInt)STDDEV, casa::stddev);
		_funcMap->define((uInt)SUM, casa::sum);
		_funcMap->define((uInt)VARIANCE, casa::variance);
	}
	return _funcMap;
}

const HashMap<uInt, String>* ImageCollapser::funcNameMap() {
	if (! _funcNameMap) {
		_funcNameMap = new HashMap<uInt, String>;
		_funcNameMap->define((uInt)AVDEV, "avdev");
		_funcNameMap->define((uInt)MAX, "max");
		_funcNameMap->define((uInt)MEAN, "mean");
		_funcNameMap->define((uInt)MEDIAN, "median");
		_funcNameMap->define((uInt)MIN, "min");
		_funcNameMap->define((uInt)RMS, "rms");
		_funcNameMap->define((uInt)STDDEV, "stddev");
		_funcNameMap->define((uInt)SUM, "sum");
		_funcNameMap->define((uInt)VARIANCE, "variance");
		_funcNameMap->define((uInt)ZERO, "zero");
	}
	return _funcNameMap;
}

const HashMap<uInt, String>* ImageCollapser::minMatchMap() {
	if (! _minMatchMap) {
		_minMatchMap = new HashMap<uInt, String>;
		_minMatchMap->define((uInt)AVDEV, "a");
		_minMatchMap->define((uInt)MAX, "ma");
		_minMatchMap->define((uInt)MEAN, "mea");
		_minMatchMap->define((uInt)MEDIAN, "med");
		_minMatchMap->define((uInt)MIN, "mi");
		_minMatchMap->define((uInt)RMS, "r");
		_minMatchMap->define((uInt)STDDEV, "st");
		_minMatchMap->define((uInt)SUM, "su");
		_minMatchMap->define((uInt)VARIANCE, "v");
		_minMatchMap->define((uInt)ZERO, "z");
	}
	return _minMatchMap;
}

void ImageCollapser::_finishConstruction() {
	for (
		Vector<uInt>::const_iterator iter=_axes.begin();
			iter != _axes.end(); iter++
		) {
		if (*iter >= _getImage()->ndim()) {
			*_log << "Specified zero-based axis (" << *iter
				<< ") must be less than the number of axes in " << _getImage()->name()
				<< "(" << _getImage()->ndim() << LogIO::EXCEPTION;
		}
	}
	_invert();
}


ImageCollapser::AggregateType ImageCollapser::aggregateType(
	String& aggString
) {
	LogIO log;
	log << LogOrigin(_class, __FUNCTION__);
	if (aggString.empty()) {
		log << "Aggregate function name is not specified and it must be."
			<< LogIO::EXCEPTION;
	}
	aggString.downcase();
	const HashMap<uInt, String> *funcNamePtr = funcNameMap();
	ConstHashMapIter<uInt, String> iter = *minMatchMap();
	for (iter.toStart(); ! iter.atEnd(); iter++) {
		uInt key = iter.getKey();
		String minMatch = iter.getVal();
		String funcName = (*funcNamePtr)(key);
		if (
			aggString.startsWith(minMatch)
			&& funcName.startsWith(aggString)
		) {
			return (AggregateType)key;
		}
	}
	log << "Unknown aggregate function specified by " << aggString << LogIO::EXCEPTION;
}

void ImageCollapser::_invert() {
	if (_invertAxesSelection) {
		Vector<uInt> newAxes(_getImage()->ndim() - _axes.size(), 0);
		uInt index=0;
		for (uInt i=0; i<_getImage()->ndim(); i++) {
			Bool found = False;
			for (uInt j=0; j<_axes.size(); j++) {
				if (i == _axes[j]) {
					found = True;
					break;
				}
			}
			if (! found) {
				newAxes[index] = i;
				index++;
			}
		}
		_axes.resize(newAxes.size());
		_axes = newAxes;
	}
}
}

