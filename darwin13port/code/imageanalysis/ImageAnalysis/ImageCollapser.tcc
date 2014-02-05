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

template<class T> map<uInt, T (*)(const Array<T>&)> ImageCollapser<T>::_funcMap;

template<class T> ImageCollapser<T>::ImageCollapser(
	String aggString, const SPCIIT image,
	const String& region, const Record *const regionRec,
	const String& box,
	const String& chanInp, const String& stokes,
	const String& maskInp, const IPosition& axes,
	const String& outname, const Bool overwrite
) : ImageTask<T>(
		image, region, regionRec, box, chanInp, stokes,
		maskInp, outname, overwrite
	),
	_invertAxesSelection(False),
	_axes(axes), _aggType(ImageCollapserData::UNKNOWN) {
	_aggType = ImageCollapserData::aggregateType(aggString);
	this->_construct();
	_finishConstruction();
}

template<class T> ImageCollapser<T>::ImageCollapser(
	const SPCIIT image,
	const IPosition& axes, const Bool invertAxesSelection,
	const ImageCollapserData::AggregateType aggregateType,
	const String& outname, const Bool overwrite
) : ImageTask<T>(
		image, "", 0, "", "", "",
		"", outname, overwrite
	),
	_invertAxesSelection(invertAxesSelection),
	_axes(axes), _aggType(aggregateType) {
	ThrowIf (
		_aggType == ImageCollapserData::UNKNOWN,
		"UNKNOWN aggregateType not allowed"
	);
	ThrowIf(
		! image,
		"Cannot use a null image pointer with this constructor"
	);
	this->_construct();
	_finishConstruction();
}

template<class T> SPIIT ImageCollapser<T>::collapse(Bool wantReturn) const {
	std::auto_ptr<ImageInterface<T> > clone(this->_getImage()->cloneII());
	SubImage<T> subImage = SubImageFactory<T>::createSubImage(
		*clone, *this->_getRegion(), this->_getMask(), this->_getLog().get(),
		False, AxesSpecifier(), this->_getStretch()
	);
	*this->_getLog() << LogOrigin("ImageCollapser", __FUNCTION__);
	ThrowIf(
		! anyTrue(subImage.getMask()),
		"All selected pixels are masked"
	);
	clone.reset(0);
	IPosition inShape = subImage.shape();
	// Set the compressed axis reference pixel and reference value
	CoordinateSystem outCoords(subImage.coordinates());
	Vector<Double> blc, trc;
	IPosition pixblc(inShape.nelements(), 0);
	IPosition pixtrc = inShape - 1;
	ThrowIf(
		! outCoords.toWorld(blc, pixblc)
		|| ! outCoords.toWorld(trc, pixtrc),
		"Could not set new coordinate values"
	);
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
	ThrowIf(
		! outCoords.setReferenceValue(refValues),
		"Unable to set reference value"
	);
	ThrowIf(
		! outCoords.setReferencePixel(refPixels),
		"Unable to set reference pixel"
	);
	TempImage<T> tmpIm(outShape, outCoords);
	if (_aggType == ImageCollapserData::ZERO) {
		Array<T> zeros(outShape, 0.0);
		tmpIm.put(zeros);
	}
	else if (_aggType == ImageCollapserData::MEDIAN) {
		_doMedian(subImage, tmpIm);
	}
	else {
		if (subImage.getMask().size() > 0 && ! allTrue(subImage.getMask())) {
			// mask with one or more False values, must use lower performance methods
			LatticeStatsBase::StatisticsTypes lattStatType = LatticeStatsBase::NACCUM;
			switch(_aggType) {
			case ImageCollapserData::MAX:
				lattStatType = LatticeStatsBase::MAX;
				break;
			case ImageCollapserData::MEAN:
				lattStatType = LatticeStatsBase::MEAN;
				break;
			case ImageCollapserData::MIN:
				lattStatType = LatticeStatsBase::MIN;
				break;
			case ImageCollapserData::RMS:
				lattStatType = LatticeStatsBase::RMS;
				break;
			case ImageCollapserData::STDDEV:
				lattStatType = LatticeStatsBase::SIGMA;
				break;
			case ImageCollapserData::SUM:
				lattStatType = LatticeStatsBase::SUM;
				break;
			case ImageCollapserData::VARIANCE:
				lattStatType = LatticeStatsBase::VARIANCE;
				break;
			case ImageCollapserData::MEDIAN:
			case ImageCollapserData::ZERO:
			case ImageCollapserData::UNKNOWN:
			default:
				ThrowCc(
					"Logic error. Should never have gotten the the bottom of the switch statement"
				);
				break;
			}
			Array<T> data;
			Array<Bool> mask;
			LatticeUtilities::collapse(
				data, mask, _axes, subImage, False,
				True, True, lattStatType
			);
			Array<T> dataCopy = (_axes.size() <= 1)
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
			T (*function)(const Array<T>&) = _getFuncMap().find(_aggType)->second;
			Array<T> data = subImage.get(False);
			for (uInt i=0; i<outShape.product(); i++) {
				IPosition start = toIPositionInArray(i, outShape);
				IPosition end = start + shape - 1;
				Slicer s(start, end, Slicer::endIsLast);
				tmpIm.putAt(function(data(s)), start);
			}
		}
	}
	if (subImage.imageInfo().hasMultipleBeams()) {
		*this->_getLog() << LogIO::WARN << "Input image has per plane beams. "
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
	SPIIT outImage = this->_prepareOutputImage(tmpIm);
	if (wantReturn) {
		return outImage;
	}
	else {
		return SPIIT();
	}

}

template<class T> void ImageCollapser<T>::_finishConstruction() {
	for (
		IPosition::const_iterator iter=_axes.begin();
			iter != _axes.end(); iter++
		) {
		if (*iter >= this->_getImage()->ndim()) {
			*this->_getLog() << "Specified zero-based axis (" << *iter
				<< ") must be less than the number of axes in " << this->_getImage()->name()
				<< "(" << this->_getImage()->ndim() << LogIO::EXCEPTION;
		}
	}
	_invert();
}

template<class T> void ImageCollapser<T>::_invert() {
	if (_invertAxesSelection) {
		IPosition x = IPosition::otherAxes(this->_getImage()->ndim(), _axes);
		_axes.resize(x.size());
		_axes = x;
	}
}

template<class T> void ImageCollapser<T>::_doMedian(
	const SubImage<T>& subImage, TempImage<T>& outImage
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
	Array<T> ary = subImage.get(False);
	Array<Bool> mask = subImage.getMask();
	if (subImage.hasPixelMask()) {
		mask = mask && subImage.pixelMask().get(False);
	}
	std::auto_ptr<Array<Bool> > outMask(0);
	Bool hasMaskedPixels = ! allTrue(mask);
	for (stepper.reset(); !stepper.atEnd(); stepper++) {
		Slicer slicer(stepper.position(), stepper.endPosition(), Slicer::endIsLast);
		Vector<T> kk(ary(slicer));
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
				vector<T> data;
				kk.tovector(data);
				typename vector<T>::iterator diter = data.begin();
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
				kk = Vector<T>(data);
			}
		}
		GenSort<T>::sort(kk);
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

template<class T> void ImageCollapser<T>::_attachOutputMask(
	TempImage<T>& outImage,
	const Array<Bool>& outMask
) const {
	if (this->_getOutname().empty()) {
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

template<class T> const map<uInt, T (*)(const Array<T>&)>& ImageCollapser<T>::_getFuncMap() {
	if (_funcMap.size() == 0) {
		_funcMap[(uInt)ImageCollapserData::MAX] = casa::max;
		_funcMap[(uInt)ImageCollapserData::MEAN] = casa::mean;
		_funcMap[(uInt)ImageCollapserData::MEDIAN] = casa::median;
		_funcMap[(uInt)ImageCollapserData::MIN] = casa::min;
		_funcMap[(uInt)ImageCollapserData::RMS] = casa::rms;
		_funcMap[(uInt)ImageCollapserData::STDDEV] = casa::stddev;
		_funcMap[(uInt)ImageCollapserData::SUM] = casa::sum;
		_funcMap[(uInt)ImageCollapserData::VARIANCE] = casa::variance;
	}
	return _funcMap;
}

}
