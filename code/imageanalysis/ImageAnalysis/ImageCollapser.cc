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
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/TempImage.h>
#include <lattices/Lattices/LatticeUtilities.h>

#include <memory>

namespace casa {

map<uInt, Float (*)(const Array<Float>&)> ImageCollapser::_funcMap;

map<uInt, String> *ImageCollapser::_funcNameMap = 0;
map<uInt, String> *ImageCollapser::_minMatchMap = 0;

const String ImageCollapser::_class = "ImageCollapser";

ImageCollapser::ImageCollapser(
	String aggString, const ImageTask::shCImFloat image,
	const String& region, const Record *const regionRec,
	const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const IPosition& axes,
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
		const ImageTask::shCImFloat image,
	const IPosition& axes, const Bool invertAxesSelection,
	const AggregateType aggregateType,
	const String& outname, const Bool overwrite
) : ImageTask(
		image, "", 0, "", "", "",
		"", outname, overwrite
	),
	_invertAxesSelection(invertAxesSelection),
	_axes(axes), _aggType(aggregateType) {
    *_getLog() << LogOrigin(_class, __FUNCTION__);
	if (_aggType == UNKNOWN) {
		*_getLog() << "UNKNOWN aggregateType not allowed" << LogIO::EXCEPTION;
	}
	if (! image) {
		*_getLog() << "Cannot use a null image pointer with this constructor"
			<< LogIO::EXCEPTION;
	}
	_construct();
	_finishConstruction();
}

ImageCollapser::~ImageCollapser() {}

ImageInterface<Float>* ImageCollapser::collapse(const Bool wantReturn) const {
	std::auto_ptr<ImageInterface<Float> > clone(_getImage()->cloneII());
	SubImage<Float> subImage = SubImageFactory<Float>::createSubImage(
		*clone, *_getRegion(), _getMask(), _getLog().get(),
		False, AxesSpecifier(), _getStretch()
	);
	*_getLog() << LogOrigin(_class, __FUNCTION__);
	if (! anyTrue(subImage.getMask())) {
		*_getLog() << "All selected pixels are masked" << LogIO::EXCEPTION;
	}
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
		*_getLog() << "Could not set new coordinate values" << LogIO::EXCEPTION;
	}
	Vector<Double> refValues = outCoords.referenceValue();
	Vector<Double> refPixels = outCoords.referencePixel();
	IPosition outShape = inShape;
	IPosition shape(outShape.nelements(), 1);
	for (
		IPosition::const_iterator iter=_axes.begin();
		iter != _axes.end(); iter++
	) {
		uInt i = *iter;
		refValues[i] = (blc[i] + trc[i])/2;
		refPixels[i] = 0;
		outShape[i] = 1;
		shape[i] = inShape[i];
	}
	if (! outCoords.setReferenceValue(refValues)) {
		*_getLog() << "Unable to set reference value" << LogIO::EXCEPTION;
	}
	if (! outCoords.setReferencePixel(refPixels)) {
		*_getLog() << "Unable to set reference pixel" << LogIO::EXCEPTION;
	}
	TempImage<Float> tmpIm(outShape, outCoords);
	if (_aggType == ZERO) {
		Array<Float> zeros(outShape, 0.0);
		tmpIm.put(zeros);
	}
	else if (_aggType == MEDIAN) {
		_doMedian(subImage, tmpIm);
	}
	else {
		if (subImage.getMask().size() > 0 && ! allTrue(subImage.getMask())) {
			// mask with one or more False values, must use lower performance methods
			LatticeStatsBase::StatisticsTypes lattStatType = LatticeStatsBase::NACCUM;
			switch(_aggType) {
			case MAX:
				lattStatType = LatticeStatsBase::MAX;
				break;
			case MEAN:
				lattStatType = LatticeStatsBase::MEAN;
				break;
			case MIN:
				lattStatType = LatticeStatsBase::MIN;
				break;
			case RMS:
				lattStatType = LatticeStatsBase::RMS;
				break;
			case STDDEV:
				lattStatType = LatticeStatsBase::SIGMA;
				break;
			case SUM:
				lattStatType = LatticeStatsBase::SUM;
				break;
			case VARIANCE:
				lattStatType = LatticeStatsBase::VARIANCE;
				break;
			case MEDIAN:
			case ZERO:
			case UNKNOWN:
			default:
				*_getLog() << "Logic error. Should never have gotten the the bottom of the switch statement"
					<< LogIO::EXCEPTION;
				break;
			}
			Array<Float> data;
			Array<Bool> mask;
			LatticeUtilities::collapse(
				data, mask, _axes, subImage, False,
				True, True, lattStatType
			);
			Array<Float> dataCopy = (_axes.size() <= 1)
				? data
				: data.addDegenerate(_axes.size() - 1);
			IPosition newOrder(tmpIm.ndim(), -1);
			uInt nAltered = _axes.size();
			uInt nUnaltered = tmpIm.ndim() - nAltered;
			uInt alteredCount = nUnaltered;
			uInt unAlteredCount = 0;
			for (uInt i=0; i<tmpIm.ndim(); i++) {
				for (uInt j=0; j<_axes.size(); j++) {
					if (i == _axes[j]) {
						newOrder[i] = alteredCount;
						alteredCount++;
						break;
					}
				}
				if (newOrder[i] < 0) {
					newOrder[i] = unAlteredCount;
					unAlteredCount++;
				}
			}
			tmpIm.put(reorderArray(dataCopy, newOrder));
			if (! allTrue(mask)) {
				Array<Bool> maskCopy = (
					_axes.size() <= 1)
					? mask
					: mask.addDegenerate(_axes.size() - 1
				);
				Array<Bool> mCopy = reorderArray(maskCopy, newOrder);
				_attachOutputMask(tmpIm, mCopy);
			}
		}
		else {
			// no mask, can use higher performance method
			Float (*function)(const Array<Float>&) = _getFuncMap().find(_aggType)->second;
			Array<Float> data = subImage.get(False);
			for (uInt i=0; i<outShape.product(); i++) {
				IPosition start = toIPositionInArray(i, outShape);
				IPosition end = start + shape - 1;
				Slicer s(start, end, Slicer::endIsLast);
				tmpIm.putAt(function(data(s)), start);
			}
		}
	}
	if (subImage.imageInfo().hasMultipleBeams()) {
		*_getLog() << LogIO::WARN << "Input image has per plane beams. "
			<< "The output image will arbitrarily have a single beam which "
			<< "is the first beam available in the subimage."
			<< "Thus, the image planes will not be convolved to a common "
			<< "restoring beam before collapsing. If, however, this is desired, "
			<< "then run the task imsmooth or the tool method ia.convolve2d() first, "
			<< "and use the output image of that as the input for collapsing."
			<< LogIO::POST;
		ImageUtilities::copyMiscellaneous(tmpIm, subImage, False);
		ImageInfo info = subImage.imageInfo();
		vector<Vector<Quantity> > out;
		GaussianBeam beam = *(info.getBeamSet().getBeams().begin());
        info.removeRestoringBeam();
		info.setRestoringBeam(beam);
		tmpIm.setImageInfo(info);
	}
	else {
		ImageUtilities::copyMiscellaneous(tmpIm, subImage, True);
	}
	std::tr1::shared_ptr<ImageInterface<Float> > outImage = _prepareOutputImage(tmpIm);
	if (wantReturn) {
		return outImage->cloneII();
	}
	else {
		return 0;
	}

}

const map<uInt, String>* ImageCollapser::funcNameMap() {
	if (! _funcNameMap) {
		map<uInt, String> ref;
		//ref[(uInt)AVDEV] = "avdev";
		ref[(uInt)MAX] = "max";
		ref[(uInt)MEAN] = "mean";
		ref[(uInt)MEDIAN] = "median";
		ref[(uInt)MIN] = "min";
		ref[(uInt)RMS] = "rms";
		ref[(uInt)STDDEV] = "stddev";
		ref[(uInt)SUM] = "sum";
		ref[(uInt)VARIANCE] = "variance";
		ref[(uInt)ZERO] = "zero";
		_funcNameMap = new map<uInt, String>(ref);
	}
	return _funcNameMap;
}

const map<uInt, String>* ImageCollapser::minMatchMap() {
	if (! _minMatchMap) {
		map<uInt, String> ref;
		//ref[(uInt)AVDEV] = "a";
		ref[(uInt)MAX] = "ma";
		ref[(uInt)MEAN] = "mea";
		ref[(uInt)MEDIAN] = "med";
		ref[(uInt)MIN] = "mi";
		ref[(uInt)RMS] = "r";
		ref[(uInt)STDDEV] = "st";
		ref[(uInt)SUM] = "su";
		ref[(uInt)VARIANCE] = "v";
		ref[(uInt)ZERO] = "z";
		_minMatchMap = new map<uInt, String>(ref);

	}
	return _minMatchMap;
}

void ImageCollapser::_finishConstruction() {
	for (
		IPosition::const_iterator iter=_axes.begin();
			iter != _axes.end(); iter++
		) {
		if (*iter >= _getImage()->ndim()) {
			*_getLog() << "Specified zero-based axis (" << *iter
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
	const map<uInt, String> *funcNamePtr = funcNameMap();
	map<uInt, String>::const_iterator iter;
	const map<uInt, String> *minMatch = minMatchMap();
	for (iter = minMatch->begin(); iter != minMatch->end(); iter++) {
		uInt key = iter->first;
		String minMatch = iter->second;
		String funcName = (*funcNamePtr).at(key);
		if (
			aggString.startsWith(minMatch)
			&& funcName.startsWith(aggString)
		) {
			return (AggregateType)key;
		}
	}
	log << "Unknown aggregate function specified by " << aggString << LogIO::EXCEPTION;
	// not necessary since we've thrown an exception by now but avoids compiler warning
	return UNKNOWN;
}

void ImageCollapser::_invert() {
	if (_invertAxesSelection) {
		IPosition x = IPosition::otherAxes(_getImage()->ndim(), _axes);
		_axes.resize(x.size());
		_axes = x;
	}
}

void ImageCollapser::_doMedian(
	const SubImage<Float>& subImage, TempImage<Float>& outImage
) const {
	IPosition cursorShape(subImage.ndim(), 1);
	for (uInt i=0; i<cursorShape.size(); i++) {
		for (uInt j=0; j<_axes.size(); j++) {
			if (_axes[j] == i) {
				cursorShape[i] = subImage.shape()[i];
				break;
			}
		}
	}
	LatticeStepper stepper(subImage.shape(), cursorShape);
	Array<Float> ary = subImage.get(False);
	Array<Bool> mask = subImage.getMask();
	if (subImage.hasPixelMask()) {
		mask = mask && subImage.pixelMask().get(False);
	}
	std::auto_ptr<Array<Bool> > outMask(0);
	Bool hasMaskedPixels = ! allTrue(mask);
	for (stepper.reset(); !stepper.atEnd(); stepper++) {
		Slicer slicer(stepper.position(), stepper.endPosition(), Slicer::endIsLast);
		Vector<Float> kk(ary(slicer));
		if (hasMaskedPixels) {
			Vector<Bool> maskSlice(mask(slicer));
			if (! anyTrue(maskSlice)) {
				if (outMask.get() == 0) {
					outMask.reset(new Array<Bool>(outImage.shape(), True));
				}
				(*outMask)(stepper.position()) = False;
				kk.resize(0);
			}
			else if (! allTrue(maskSlice)) {
				vector<Float> data;
				kk.tovector(data);
				vector<Float>::iterator diter = data.begin();
				Vector<Bool>::iterator miter = maskSlice.begin();
				while (diter != data.end()) {
					if (! *miter) {
						data.erase(diter);
						if (diter == data.end()) {
							break;
						}
					}
					else {
						diter++;
					}
					miter++;
				}
				kk.resize(data.size());
				kk = Vector<Float>(data);
			}
		}
		GenSort<Float>::sort(kk);
		uInt s = kk.size();

		outImage.putAt(
			s == 0
				? 0
				: s % 2 == 1
				  ? kk[s/2]
				  : (kk[s/2] + kk[s/2 - 1])/2,
			stepper.position()
		);
	}
	if (outMask.get() != 0) {
		_attachOutputMask(outImage, *outMask.get());
	}
}

void ImageCollapser::_attachOutputMask(
	TempImage<Float>& outImage,
	const Array<Bool>& outMask
) const {
	if (_getOutname().empty()) {
		outImage.attachMask(ArrayLattice<Bool>(outMask));
	}
	else {
		String maskName = outImage.makeUniqueRegionName(
			String("mask"), 0
		);
		outImage.makeMask(maskName, True, True, True, True);
		(&outImage.pixelMask())->put(outMask);
	}
}

const map<uInt, Float (*)(const Array<Float>&)>& ImageCollapser::_getFuncMap() {
	if (_funcMap.size() == 0) {
		_funcMap[(uInt)MAX] = casa::max;
		_funcMap[(uInt)MEAN] = casa::mean;
		_funcMap[(uInt)MEDIAN] = casa::median;
		_funcMap[(uInt)MIN] = casa::min;
		_funcMap[(uInt)RMS] = casa::rms;
		_funcMap[(uInt)STDDEV] = casa::stddev;
		_funcMap[(uInt)SUM] = casa::sum;
		_funcMap[(uInt)VARIANCE] = casa::variance;
	}
	return _funcMap;
}



}


