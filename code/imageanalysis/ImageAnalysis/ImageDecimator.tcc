#include <imageanalysis/ImageAnalysis/ImageDecimator.h>

#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <imageanalysis/ImageAnalysis/ImageHistory.h>

#include<stdcasa/cboost_foreach.h>

namespace casa {

template<class T> ImageDecimator<T>::ImageDecimator(
	const SPCIIT image,
	const Record *const region,
	const String& maskInp,
	const String& outname, Bool overwrite
) : ImageTask<T>(
		image, "", region, "", "", "",
		maskInp, outname, overwrite
	),
	_axis(0), _factor(1), _function(ImageDecimatorData::NONE) {
	this->_construct();
}

template<class T> void ImageDecimator<T>::setFactor(uInt n) {
	ThrowIf(n == 0, "The decimation factor must be positive");
	_factor = n;
}

template<class T> void ImageDecimator<T>::setAxis(uInt n) {
	uInt ndim = this->_getImage()->ndim();
	ThrowIf(
		n >= ndim,
		"The axis number along which the decimation "
		"is to occur must be less than the number of "
		"axes in the image which is " + String::toString(ndim)
	);
	_axis = n;
}

template<class T> SPIIT ImageDecimator<T>::decimate(Bool wantReturn) const {
	LogOrigin lor = LogOrigin(getClass(), __FUNCTION__);
	*this->_getLog() << lor;
	SPIIT clone(this->_getImage()->cloneII());
	std::tr1::shared_ptr<SubImage<T> >subImage(
		new SubImage<T>(
			SubImageFactory<T>::createSubImage(
				*clone, *this->_getRegion(), this->_getMask(), 0,
				True, AxesSpecifier(), this->_getStretch()
			)
		)
	);
	clone.reset();
	if (_factor == 1) {
		*this->_getLog() << LogIO::WARN << "A decimation factor "
			<< "of 1 has been specified which means no planes will "
			<< "be removed. The resulting image will be a straight "
			<< "copy of the selected image." << LogIO::POST;
		SPIIT tmp(this->_prepareOutputImage(*subImage));
		if (wantReturn) {
			return tmp;
		}
		else {
			return SPIIT();
		}
	}
	CoordinateSystem csys = subImage->coordinates();
	Vector<Double> refPix = csys.referencePixel();
	refPix[_axis] /= _factor;
	csys.setReferencePixel(refPix);
	Vector<Double> inc = csys.increment();
	inc[_axis] *= _factor;
	csys.setIncrement(inc);

	IPosition subShape = subImage->shape();
	IPosition shape = subShape;
	// integer division
	shape[_axis] = shape[_axis]/_factor;
	if (
		_function == ImageDecimatorData::NONE
		&& subShape[_axis] % 2 == 1
	) {
		shape[_axis]++;
	}
	TempImage<T> out(shape, csys);
	IPosition cursorShape = shape;
	cursorShape[_axis] = _factor;
	LatticeStepper stepper(subShape, cursorShape);
	RO_MaskedLatticeIterator<T> inIter(*subImage, stepper);
	inIter.reset();
	Bool isMasked = subImage->isMasked();
	uInt ndim = subImage->ndim();
	IPosition begin(ndim, 0);
	IPosition end = cursorShape - 1;
	std::tr1::shared_ptr<ArrayLattice<Bool> > outMask(
		isMasked ? new ArrayLattice<Bool>(out.shape()) : 0
	);
	IPosition outPos = begin;

	if (_function == ImageDecimatorData::NONE) {
		end[_axis] = 0;
		while(! inIter.atEnd() && outPos[_axis]<shape[_axis]) {
			if (isMasked) {
				const Array<Bool> mask = inIter.getMask();
				Array<Bool> maskSlice = mask(begin, end);
				outMask->putSlice(maskSlice, outPos);
			}
			const Array<T> data = inIter.cursor();
			Array<T> dataSlice = data(begin, end);
			out.putSlice(dataSlice, outPos);
			inIter++;
			outPos[_axis]++;
		}
	}
	else if (_function == ImageDecimatorData::MEAN) {
		String comment;
		ImageCollapser<T> collapser(
			subImage, IPosition(1, _axis), False,
			ImageCollapserData::MEAN, "", False
		);
		std::auto_ptr<Record> reg;

		Vector<Double> start(ndim);
		Vector<Int> lattShape(ndim);
		Vector<Double> stop(ndim);
		for (uInt i=0; i<ndim; i++) {
			start[i] = begin[i];
			stop[i] = begin[i] + cursorShape[i] - 1;
			lattShape[i] = subShape[i];
		}
		SPIIT collapsed;
		while(! inIter.atEnd() && outPos[_axis]<shape[_axis]) {
			reg.reset(
				RegionManager::box(
					start, stop,
					lattShape, comment
				)
			);
			collapser.setRegion(*reg);
			collapsed.reset(collapser.collapse(True));
			if (isMasked) {
				if (collapsed->isMasked()) {
					Array<Bool> mask = collapsed->pixelMask().get();
					outMask->putSlice(mask, outPos);
				}
				else {
					Array<Bool> mask(collapsed->shape(), True);
					outMask->putSlice(mask, outPos);
				}
			}
			const Array<T> data = collapsed->get();
			out.putSlice(data, outPos);
			inIter++;
			outPos[_axis]++;
			start[_axis] += _factor;
			stop[_axis] += _factor;
		}
	}
	SPIIT tmp(
		this->_prepareOutputImage(out, 0, outMask.get())
	);
	ImageHistory<T> hist(tmp);
	LogIO log = hist.getLogSink();
	log << lor << LogIO::NORMAL << "Decimated axis " << _axis
		<< " by keeping only every " << _factor << " planes.";
	if (wantReturn) {
		return tmp;
	}
	else {
		return SPIIT();
	}
}



}
