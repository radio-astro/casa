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

#include <casa/Arrays/ArrayLogical.h>
//#include <casa/Arrays/ArrayMath.h>
#include <casa/IO/STLIO.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/TempImage.h>
#include <lattices/Lattices/LatticeUtilities.h>

#include <memory>

namespace casa {

template<class T> map<uInt, T (*)(const Array<T>&)> ImageCollapser<T>::_funcMap;

template<class T> ImageCollapser<T>::ImageCollapser(
	const String& aggString, const SPCIIT image,
	const Record *const regionRec,
	const String& maskInp, const IPosition& axes,
	Bool invertAxesSelection,
	const String& outname, Bool overwrite
) : ImageTask<T>(
		image, "", regionRec, "", "", "",
		maskInp, outname, overwrite
	),
	_invertAxesSelection(invertAxesSelection),
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

template<class T> SPIIT ImageCollapser<T>::collapse() const {
	SPIIT subImage = SubImageFactory<T>::createImage(
		*this->_getImage(), "", *this->_getRegion(),
		this->_getMask(), False, False, False, this->_getStretch()
	);
	*this->_getLog() << LogOrigin(getClass(), __func__);
	ThrowIf(
		! anyTrue(subImage->getMask()),
		"All selected pixels are masked"
	);
	CoordinateSystem outCoords = subImage->coordinates();
	Bool hasDir = outCoords.hasDirectionCoordinate();
	IPosition inShape = subImage->shape();
	if (_aggType == ImageCollapserData::FLUX) {
		String cant = " Cannot do flux density calculation";
		ThrowIf(
			! hasDir,
			"Image has no direction coordinate." + cant
		);
		ThrowIf(
			! subImage->imageInfo().hasBeam(),
			"Image has no beam." + cant
		);
		Vector<Int> dirAxes = outCoords.directionAxesNumbers();
		for (uInt i=0; i<_axes.nelements(); i++) {
			Int axis = _axes[i];
			ThrowIf(
				! anyTrue(dirAxes == axis)
				&& inShape[axis] > 1,
				"Specified axis " + String::toString(axis)
				+ " is not a direction axis but has length > 1." + cant
			);
		}
	}

	// Set the compressed axis reference pixel and reference value
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
		Bool lowPerf = _aggType == ImageCollapserData::FLUX;
		if (! lowPerf) {
			Array<Bool> mask = subImage->getMask();
			if (subImage->hasPixelMask()) {
				mask = mask && subImage->pixelMask().get();
			}
			lowPerf = ! allTrue(mask);
		}
		T npixPerBeam = 1;
		if (_aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM) {
			ImageInfo info = subImage->imageInfo();
			if (! info.hasBeam()) {
				*this->_getLog() << LogIO::WARN
					<< "Image has no beam, will use sqrtsum method"
					<< LogIO::POST;
			}
			else if (info.hasMultipleBeams()) {
				*this->_getLog() << LogIO::WARN
					<< "Function sqrtsum_npix_beam does not support multiple beams, will"
					<< "use sqrtsum method instead"
					<< LogIO::POST;
			}
			else {
				npixPerBeam = info.getBeamAreaInPixels(
					-1, -1, subImage->coordinates().directionCoordinate()
				);
			}
		}
		if (lowPerf) {
			// flux or mask with one or more False values, must use lower performance methods
			LatticeStatsBase::StatisticsTypes lattStatType = LatticeStatsBase::NACCUM;
			switch(_aggType) {
			case ImageCollapserData::FLUX:
				lattStatType = LatticeStatsBase::FLUX;
				break;
			case ImageCollapserData::MAX:
				lattStatType = LatticeStatsBase::MAX;
				break;
			case ImageCollapserData::MEAN:
				lattStatType = LatticeStatsBase::MEAN;
				break;
			case ImageCollapserData::MIN:
				lattStatType = LatticeStatsBase::MIN;
				break;
			case ImageCollapserData::NPTS:
				lattStatType = LatticeStatsBase::NPTS;
				break;
			case ImageCollapserData::RMS:
				lattStatType = LatticeStatsBase::RMS;
				break;
			case ImageCollapserData::STDDEV:
				lattStatType = LatticeStatsBase::SIGMA;
				break;
			case ImageCollapserData::SQRTSUM:
			case ImageCollapserData::SQRTSUM_NPIX:
			case ImageCollapserData::SQRTSUM_NPIX_BEAM:
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
			if (_aggType == ImageCollapserData::FLUX) {
				ImageStatistics<T> stats(*subImage, False);
				stats.setAxes(_axes.asVector());
				if (
					! stats.getConvertedStatistic(
						data, lattStatType, False
					)
				) {
					ostringstream oss;
					oss << "Unable to calculate flux density: "
					<< stats.getMessages();
					ThrowCc(oss.str());
				}
				mask.resize(data.shape());
				mask.set(True);
			}
			else {
				LatticeUtilities::collapse(
					data, mask, _axes, *subImage, False,
					True, True, lattStatType
				);
				if (
					_aggType == ImageCollapserData::SQRTSUM
					|| _aggType == ImageCollapserData::SQRTSUM_NPIX
					|| _aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM
				) {
					_zeroNegatives(data);
					data = sqrt(data);
					if (_aggType == ImageCollapserData::SQRTSUM_NPIX) {
						Array<T> npts = data.copy();
						LatticeUtilities::collapse(
							npts, mask, _axes, *subImage, False,
							True, True, LatticeStatsBase::NPTS
						);
						data /= npts;
					}
					else if (_aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM) {
						data /= npixPerBeam;
					}
				}
			}
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
			Array<T> data = subImage->get(False);
			Int64 nelements = outShape.product();
			for (uInt i=0; i<nelements; i++) {
				IPosition start = toIPositionInArray(i, outShape);
				IPosition end = start + shape - 1;
				Slicer s(start, end, Slicer::endIsLast);
				tmpIm.putAt(function(data(s)), start);
			}
			if (
				_aggType == ImageCollapserData::SQRTSUM
				|| _aggType == ImageCollapserData::SQRTSUM_NPIX
				|| _aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM
			) {
				Array<T> arr = tmpIm.get();
				_zeroNegatives(arr);
				arr = sqrt(arr);
				if (_aggType == ImageCollapserData::SQRTSUM_NPIX) {
					T npts = subImage->shape().product()/nelements;
					arr /= npts;

				}
				else if (_aggType == ImageCollapserData::SQRTSUM_NPIX_BEAM) {
					arr /= npixPerBeam;
				}
				tmpIm.put(arr);
			}
		}
	}
	Bool n2 = _axes.size() == 2;
	Bool dirAxesOnlyCollapse =  hasDir && n2;
	if (dirAxesOnlyCollapse) {
		Vector<Int>dirAxes = outCoords.directionAxesNumbers();
		dirAxesOnlyCollapse = (_axes[0] == dirAxes[0] && _axes[1] == dirAxes[1])
			|| (_axes[1] == dirAxes[0] && _axes[0] == dirAxes[1]);
	}
	if (subImage->imageInfo().hasMultipleBeams() && ! dirAxesOnlyCollapse) {
		*this->_getLog() << LogIO::WARN << "Input image has per plane beams "
			<< "but the collapse is not done exclusively along the direction axes. "
			<< "The output image will arbitrarily have a single beam which "
			<< "is the first beam available in the subimage."
			<< "Thus, the image planes will not be convolved to a common "
			<< "restoring beam before collapsing. If, however, this is desired, "
			<< "then run the task imsmooth or the tool method ia.convolve2d() first, "
			<< "and use the output image of that as the input for collapsing."
			<< LogIO::POST;
		ImageUtilities::copyMiscellaneous(tmpIm, *subImage, False);
		ImageInfo info = subImage->imageInfo();
		vector<Vector<Quantity> > out;
		GaussianBeam beam = *(info.getBeamSet().getBeams().begin());
        info.removeRestoringBeam();
		info.setRestoringBeam(beam);
		tmpIm.setImageInfo(info);
	}
	else {
		ImageUtilities::copyMiscellaneous(tmpIm, *subImage, True);
	}
    return this->_prepareOutputImage(tmpIm);
}

template<class T> void ImageCollapser<T>::_zeroNegatives(Array<T>& arr) {
	typename Array<T>::iterator iter = arr.begin();
	if (isComplex(whatType(&(*iter))) || allGE(arr, (T)0)) {
		return;
	}
	typename Array<T>::iterator end = arr.end();
	while (iter != end) {
		if (*iter < 0) {
			*iter = 0;
		}
		iter++;
	}
}

template<class T> void ImageCollapser<T>::_finishConstruction() {
	for (
		IPosition::const_iterator iter=_axes.begin();
		iter != _axes.end(); iter++
	) {
		ThrowIf(
			*iter >= this->_getImage()->ndim(),
			"Specified zero-based axis (" + String::toString(*iter)
			+ ") must be less than the number of axes in " + this->_getImage()->name()
			+ "(" + String::toString(this->_getImage()->ndim()) + ")"
		);
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
	SPCIIT image, TempImage<T>& outImage
) const {
	IPosition cursorShape(image->ndim(), 1);
	for (uInt i=0; i<cursorShape.size(); i++) {
		for (uInt j=0; j<_axes.size(); j++) {
			if (_axes[j] == i) {
				cursorShape[i] = image->shape()[i];
				break;
			}
		}
	}
	LatticeStepper stepper(image->shape(), cursorShape);
	Array<T> ary = image->get(False);
	Array<Bool> mask = image->getMask();
	if (image->hasPixelMask()) {
		mask = mask && image->pixelMask().get(False);
	}
	std::auto_ptr<Array<Bool> > outMask(0);
	Bool hasMaskedPixels = ! allTrue(mask);
	for (stepper.reset(); !stepper.atEnd(); stepper++) {
		Slicer slicer(stepper.position(), stepper.endPosition(), Slicer::endIsLast);
		Vector<T> kk(ary(slicer).tovector());
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
		_funcMap[(uInt)ImageCollapserData::SQRTSUM] = casa::sum;
		_funcMap[(uInt)ImageCollapserData::SQRTSUM_NPIX] = casa::sum;
		_funcMap[(uInt)ImageCollapserData::SQRTSUM_NPIX_BEAM] = casa::sum;
		_funcMap[(uInt)ImageCollapserData::STDDEV] = casa::stddev;
		_funcMap[(uInt)ImageCollapserData::SUM] = casa::sum;
		_funcMap[(uInt)ImageCollapserData::VARIANCE] = casa::variance;
	}
	return _funcMap;
}

}
