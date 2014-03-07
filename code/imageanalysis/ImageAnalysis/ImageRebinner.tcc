#include <imageanalysis/ImageAnalysis/ImageRebinner.h>

namespace casa {

template<class T> ImageRebinner<T>::ImageRebinner(
	const SPCIIT image,
	const Record *const region,
	const String& maskInp,
	const String& outname, Bool overwrite
) : ImageTask<T>(
		image, "", region, "", "", "",
		maskInp, outname, overwrite
	),
	_factors(), _crop(False) {}

template<class T> void ImageRebinner<T>::setFactors(const Vector<Int>& f) {
	ThrowIf(
		f.empty(),
		"Rebinning factor vector cannot be empty"
	);
	ThrowIf(
		anyTrue(f <= 0),
		"All factors must be positive"
	);
	ThrowIf(
		allTrue(f == 1),
		"All rebinning factors are 1, which "
		"means rebinning cannot occur"
	);
	uInt ndim = this->_getImage()->ndim();
	ThrowIf(
		f.size() > ndim,
		"Factor vector length must be less than or "
		"equal to the number of input image axes"
	);
	Vector<Int> fcopy = f.copy();
	uInt mysize = f.size();
	if (mysize < ndim) {
		fcopy.resize(ndim, True);
		for (uInt i=mysize; i<ndim; i++) {
			fcopy[i] = 1;
		}
		*this->_getLog() << LogOrigin(getClass(), __func__)
			<< LogIO::NORMAL << "Not all rebinning factors were specified, "
			<< "filling in those not specified with factors of "
			<< "1, so will rebin according to factor=" << fcopy
			<< LogIO::POST;
	}
	const CoordinateSystem& csys = this->_getImage()->coordinates();
	ThrowIf(
		csys.hasPolarizationCoordinate() && fcopy[csys.polarizationAxisNumber()] > 1,
		"A polarization axis cannot be rebinned"
	);
	_factors = fcopy;
}

template<class T> SPIIT ImageRebinner<T>::rebin() const {
	ThrowIf(_factors.empty(), "Logic Error: factors have not been set");
	IPosition myFactors = _factors;
	if (this->_getDropDegen()) {
		uInt ndim = this->_getImage()->ndim();
		IPosition shape = this->_getImage()->shape();
		vector<uInt> fac;
		for (Int i=ndim-1; i>=0; i--) {
			if (shape[i] == 1) {
				myFactors.removeAxes(IPosition(1, i));
			}
		}
	}
	SPIIT subImage(
		SubImageFactory<T>::createImage(
			*this->_getImage(), "", *this->_getRegion(),
			this->_getMask(), this->_getDropDegen(),
			False, False, this->_getStretch()
		)
	);
	if (_crop) {
		uInt ndim = subImage->ndim();
		IPosition shape = subImage->shape();
		IPosition trc = shape - 1;
		Vector<Int> mods(ndim);
		for (uInt i=0; i<ndim; i++) {
			mods[i] = shape[i] % myFactors[i];
			if (mods[i] > 0) {
				trc[i] -= mods[i];
			}
		}
		if (anyTrue(mods > 0)) {
			LCBox box(IPosition(ndim, 0), trc, shape);
			subImage.reset(
				SubImageFactory<T>::createImage(
					*subImage, "", box.toRecord(""),
					"", False, False, False, False
				)
			);
		}
	}
	RebinImage<T> binIm(*subImage, myFactors);
	return this->_prepareOutputImage(binIm);
}

}
