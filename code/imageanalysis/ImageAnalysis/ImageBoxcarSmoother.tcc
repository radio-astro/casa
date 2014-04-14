#include <imageanalysis/ImageAnalysis/ImageBoxcarSmoother.h>

namespace casa {

template<class T> ImageBoxcarSmoother<T>::ImageBoxcarSmoother(
	const SPCIIT image,
	const Record *const region,
	const String& mask,
	const String& outname, Bool overwrite
) : Image1DSmoother<T>(
		image, region, mask, outname, overwrite
	), _width(2) {
	this->_construct();
    this->_setNMinPixels(_width);
}

template<class T> SPIIT ImageBoxcarSmoother<T>::_smooth(
	const ImageInterface<T>& image
) const {
    IPosition shape = image.shape();
	uInt axis = this->_getAxis();
	ThrowIf(
		this->_getDecimate()
		&& this->_getDecimationFunction() == ImageDecimatorData::MEAN
		&& shape[axis] < 2*_width,
		"The convolution axis of selected image region must have length "
		"of at least 2*width when using the mean decimation function"
	);
	IPosition inTileShape = image.niceCursorShape();
	TiledLineStepper inNav(image.shape(), inTileShape, axis);
	RO_MaskedLatticeIterator<T> inIter(image, inNav);
	IPosition sliceShape(image.ndim(), 1);
	sliceShape[axis] = shape[axis];
	String empty;
	Record emptyRecord;
    IPosition blc(image.ndim(), 0);
    IPosition trc = shape - 1;
    // integer division
    trc[axis] = this->_getDecimate()
		&& this->_getDecimationFunction() == ImageDecimatorData::COPY
		? shape[axis]/_width*_width - _width
		: shape[axis] - _width;
    LCBox lcbox(blc, trc, shape);
    Record region = lcbox.toRecord("");
	SPIIT out(
		SubImageFactory<T>::createImage(
			image, empty, region, empty,
            False, False, False, False
		)
	);
	out->set(T(0));
	sliceShape[axis] = out->shape()[axis];
	Array<T> slice(sliceShape);
	while (!inIter.atEnd()) {
		_boxcarSmooth(slice, inIter.cursor());
		out->putSlice(slice, inIter.position());
		inIter++;
	}
	if (this->_getDecimate() && out->shape()[axis] > 1) {
		ImageDecimator<T> decimator(
			SPIIT(out->cloneII()), 0,
			String(""), String(""), False
		);
		decimator.setFunction(this->_getDecimationFunction());
		decimator.setAxis(axis);
		decimator.setFactor(_width);
		decimator.suppressHistoryWriting(True);
		out = decimator.decimate();
		this->addHistory(decimator.getHistory());
	}
	return out;
}

template<class T> void ImageBoxcarSmoother<T>::setWidth(uInt w) {
	ThrowIf(w == 0, "Boxcar width must be positive");
	_width = w;
	this->_setNMinPixels(_width);
}

template<class T> void ImageBoxcarSmoother<T>::_boxcarSmooth(
	Array<T>& out, const Array<T>& in
) const {
	// although the passed in array may have more than one
	// dimension, only one of those will have length > 1

	out.set(T(0.0));
	typename Array<T>::const_iterator cur = in.begin();
	typename Array<T>::const_iterator end = in.end();
	Bool skip = this->_getDecimate()
		&& this->_getDecimationFunction() == ImageDecimatorData::COPY;
	typename Array<T>::iterator outIter = out.begin();
	if (skip) {
		uInt count = 0;
		uInt nelem = in.size();
		while (count < nelem - _width + 1) {
			uInt intraCount = 0;
			while (intraCount < _width) {
				*outIter += *cur;
				cur++;
				intraCount++;
			}
			*outIter /= _width;
			for (uInt i=0; i<_width; i++) {
				outIter++;
			}
			count += _width;
		}
	}
	else {
		typename Array<T>::const_iterator windowEnd = cur;
		T windowSum(0.0);
		for (uInt i=0; i<_width; i++) {
			windowSum += *windowEnd;
			windowEnd++;
		}
		Float width = _width;
		*outIter = windowSum/width;
		*outIter++;
		while (windowEnd != end) {
			windowSum += (*windowEnd - *cur);
			*outIter = windowSum/width;
			cur++;
			windowEnd++;
			outIter++;
		}
	}
}

}
