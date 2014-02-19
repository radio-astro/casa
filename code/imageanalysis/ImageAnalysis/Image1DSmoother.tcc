#include <imageanalysis/ImageAnalysis/Image1DSmoother.h>

#include <imageanalysis/ImageAnalysis/ImageDecimator.h>

#include<stdcasa/cboost_foreach.h>

namespace casa {

template<class T> Image1DSmoother<T>::Image1DSmoother(
	const SPCIIT image,
	const Record *const region,
	const String& maskInp,
	const String& outname, Bool overwrite
) : ImageTask<T>(
		image, "", region, "", "", "",
		maskInp, outname, overwrite
	),
	_axis(0), _decimate(False),
	_decimationFunction(ImageDecimatorData::NONE) {}

template<class T> void Image1DSmoother<T>::setAxis(uInt n) {
	uInt ndim = this->_getImage()->ndim();
	ThrowIf(
		n >= ndim,
		"The axis number along which the smoothing "
		"is to occur must be less than the number of "
		"axes in the image which is " + String::toString(ndim)
	);
	_axis = n;
}

template<class T> SPIIF Image1DSmoother<T>::smooth(Bool wantReturn) const {
	*this->_getLog() << LogOrigin(getClass(), __FUNCTION__);

	SPIIT subImage(
		SubImageFactory<T>::createImage(
			*this->_getImage(), "", *this->_getRegion(),
			this->_getMask(), False, False, False, this->_getStretch()
		)
	);
	SPIIT out(_smooth(*subImage));
	SPIIF tmp = this->_prepareOutputImage(*out);
	if (! wantReturn) {
		tmp.reset();
	}
	return tmp;
}

}
