#include <imageanalysis/ImageAnalysis/ImageHanningSmoother.h>

#include<stdcasa/cboost_foreach.h>

namespace casa {

template<class T> ImageHanningSmoother<T>::ImageHanningSmoother(
	const SPCIIT image,
	const Record *const region,
	const String& mask,
	const String& outname, Bool overwrite
) : Image1DSmoother<T>(
		image, region, mask, outname, overwrite
	) {
	this->_construct();
}

template<class T> SPIIT ImageHanningSmoother<T>::_smooth(
	const ImageInterface<T>& image
) const {
	IPosition inTileShape = image.niceCursorShape();
	uInt axis = this->_getAxis();
	TiledLineStepper inNav(image.shape(), inTileShape, axis);
	RO_MaskedLatticeIterator<Float> inIter(image, inNav);
	IPosition sliceShape(image.ndim(), 1);
	sliceShape[axis] = image.shape()[axis];
	Array<Float> slice(sliceShape);
	Array<Bool> maskIn(sliceShape, True);
	Array<Bool> maskOut(sliceShape);
	//std::auto_ptr<ImageInterface<T> > out(image.cloneII());
	String empty;
	Record emptyRecord;
	SPIIT out(
		SubImageFactory<T>::createImage(
			image, empty, emptyRecord, empty, False, False, False, False
		)
	);

	Bool isMasked = image.hasPixelMask();
	while (!inIter.atEnd()) {
		if (isMasked) {
			maskIn = inIter.getMask(False);
		}
		slice = (isMasked && ! allTrue(maskIn))
			? _hanningSmooth(inIter.cursor(), maskIn)
			: _hanningSmooth(inIter.cursor());

		out->putSlice(slice, inIter.position());
		inIter++;
	}
	if (this->_getDecimate()) {
		// remove the first plane from _axis
		IPosition shape = out->shape();
		IPosition blc(shape.size(), 0);
		blc[axis] = 1;
		IPosition trc = shape - 1;
		if (shape[axis] % 2 == 0) {
			trc[axis]--;
		}
		LCBox lcbox(blc, trc, shape);
		Record x = lcbox.toRecord("");
		ImageDecimator<T> decimator(
			SPIIT(out->cloneII()), &x,
			String(""), String(""), False
		);
		decimator.setFunction(ImageDecimatorData::NONE);
		decimator.setAxis(axis);
		decimator.setFactor(2);
		out = decimator.decimate(True);
	}
	return out;
}

template<class T> Array<T> ImageHanningSmoother<T>::_hanningSmooth(
	const Array<T>& in, const Array<Bool>& mask
) const {
	Array<T> xx = in.copy();
	typename Array<T>::iterator cur = xx.begin();
	typename Array<T>::const_iterator end = xx.end();
	Array<Bool>::const_iterator curMask = mask.begin();
	while (cur != end) {
		if (! *curMask) {
			*cur = 0;
		}
		cur++;
		curMask++;
	}
	return _hanningSmooth(xx);
}

template<class T> Array<T> ImageHanningSmoother<T>::_hanningSmooth(
	const Array<T>& in
) const {
	// although the passed in array may have more than one
	// dimensions, only one of those will have length > 1
	uInt size = in.size();
	Array<T> out(in.shape(), T(0.0));

	uInt count = 1;
	uInt end = size - 1;
	typename Array<T>::const_iterator prev = in.begin();
	typename Array<T>::const_iterator cur = in.begin();
	cur++;
	typename Array<T>::const_iterator next = in.begin();
	next++;
	next++;
	Bool skip = this->_getDecimate()
		&& this->_getDecimationFunction() == ImageDecimatorData::NONE;
	uInt inc = skip ? 2 : 1;
	if (skip && size % 2 == 0) {
		end--;
	}
	typename Array<T>::iterator outIter = out.begin();
	if (! skip) {
		*outIter = 0.5*(*cur + *prev);
	}
	outIter++;
	while (count < end) {
		*outIter = 0.25*(*prev + *next)
			+ 0.5 * (*cur);
		for (uInt i=0; i<inc; i++) {
			outIter++;
			prev++;
			cur++;
			next++;
		}
		count += inc;
	}
	if (! skip) {
		 *outIter = 0.5 * (*cur + *prev);
	}
	return out;
}

}
