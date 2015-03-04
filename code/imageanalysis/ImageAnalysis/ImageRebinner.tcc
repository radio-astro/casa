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
	_factors(), _crop(False) {
        this->_construct();
    }

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
    SPCIIT image = this->_getImage();
    uInt ndim = image->ndim();
    ThrowIf(
        ndim != _factors.size(),
        "You have provided " + String::toString(_factors.size())
        + " factors. You must provide exactly " + String::toString(ndim)
        + (
            this->_getDropDegen()
                ? ". If you wish to drop degenerate axes, specify binning factors of 1 for them"
                : ""
        )
    );
    IPosition myFactors;
    if (this->_getDropDegen()) {
        IPosition shape = image->shape();
        IPosition degenAxes;
        for (uInt i=0; i<ndim; i++) {
            if (shape[i] == 1) {
                degenAxes.append(IPosition(1, i));
            }
        }
        myFactors = _factors.removeAxes(degenAxes);
    }
    else { 
        myFactors = _factors;
    }
    SPIIT subImage(
		SubImageFactory<T>::createImage(
			*this->_getImage(), "", *this->_getRegion(),
			this->_getMask(), this->_getDropDegen(),
			False, False, this->_getStretch()
		)
	);
	if (_crop) {
        ndim = subImage->ndim();
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
			subImage = 	SubImageFactory<T>::createImage(
				*subImage, "", box.toRecord(""),
				"", False, False, False, False
			);
		}
	}
	RebinImage<T> binIm(*subImage, myFactors);
	SPIIT outIm = this->_prepareOutputImage(binIm, this->_getDropDegen());
	/*
	// remove any axes that have been binned into a remaining degenerate axis,
	// CAS-5836
	if (this->_getDropDegen()) {
		IPosition outShape = outIm->shape();
		uInt outDim = outIm->ndim();
		for (uInt i=0; i<outDim; ++i) {
			if (outShape[i] == 1) {
				Record empty;
				outIm = SubImageFactory<T>::createImage(
					*outIm, this->_getOutname(), Record(),
					"", True, True, False, False
				);
				break;
			}
		}
	}
	*/
	return outIm;
}

}
