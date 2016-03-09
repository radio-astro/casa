#include <imageanalysis/ImageAnalysis/ImageFFTer.h>

#include <imageanalysis/ImageAnalysis/ImageFFT.h>

namespace casa {

template<class T> ImageFFTer<T>::ImageFFTer(
	const SPCIIT image,
	const Record *const region,
	const String& maskInp, const Vector<uInt>& axes
) : ImageTask<T>(
		image, "", region, "", "", "",
		maskInp, "", False
	),
	_axes(axes), _real(), _imag(), _amp(),
	_phase(), _complex() {
	this->_construct();
}

template<class T> void ImageFFTer<T>::fft() const {
	*this->_getLog() << LogOrigin(getClass(), __FUNCTION__);
	ThrowIf(
		_real.empty() && _imag.empty() && _amp.empty()
		&& _phase.empty() && _complex.empty(),
		"No output image names have been specified"
	);
	_checkExists(_real);
	_checkExists(_imag);
	_checkExists(_amp);
	_checkExists(_phase);
	_checkExists(_complex);
	SHARED_PTR<const SubImage<T> > subImage = SubImageFactory<T>::createSubImageRO(
		*this->_getImage(), *this->_getRegion(), this->_getMask(), this->_getLog().get(),
		AxesSpecifier(), this->_getStretch()
	);
	ImageFFT fft;
	if (_axes.size() == 0) {
		*this->_getLog() << LogIO::NORMAL << "FFT the direction coordinate" << LogIO::POST;
		fft.fftsky(*subImage);
	}
	else {
		// Set vector of bools specifying axes
		Vector<Bool> which(subImage->ndim(), False);
		for(uInt i: _axes) {
			which(_axes(i)) = True;
		}
		*this->_getLog() << LogIO::NORMAL << "FFT zero-based axes " << _axes << LogIO::POST;
		fft.fft(*subImage, which);
	}

	String maskName("");
	if (! _real.empty()) {
		auto x = _createFloatImage(_real, *subImage);
		fft.getReal(*x);
		this->_doHistory(x);
	}
	if (! _imag.empty()) {
		auto x = _createFloatImage(_imag, *subImage);
		fft.getImaginary(*x);
		this->_doHistory(x);
	}
	if (! _amp.empty()) {
		auto x = _createFloatImage(_amp, *subImage);
		fft.getAmplitude(*x);
		this->_doHistory(x);
	}
	if (! _phase.empty()) {
		auto x = _createFloatImage(_phase, *subImage);
		fft.getPhase(*x);
		this->_doHistory(x);
	}
	if (! _complex.empty()) {
		auto x = _createComplexImage(_complex, *subImage);
		fft.getComplex(*x);
		this->_doHistory(x);
	}
}

template<class T> SPIIF ImageFFTer<T>::_createFloatImage(
	const String& name, const SubImage<T>& subImage
) const {
	*this->_getLog() << LogIO::NORMAL << "Creating image '"
		<< name << "'" << LogIO::POST;
	SPIIF image(
	    new PagedImage<Float>(
	        subImage.shape(), subImage.coordinates(),
	        name
	    )
	);
	if (subImage.isMasked()) {
		String x;
		ImageMaskAttacher::makeMask(
			*image, x, False, True, *this->_getLog(), True
		);
	}
	return image;
}

template<class T> SPIIC ImageFFTer<T>::_createComplexImage(
	const String& name, const SubImage<T>& subImage
) const {
	*this->_getLog() << LogIO::NORMAL << "Creating image '"
		<< name << "'" << LogIO::POST;
	SPIIC image(
	    new PagedImage<Complex>(
	        subImage.shape(), subImage.coordinates(),
	        name
	    )
	);
	if (subImage.isMasked()) {
		String x;
		ImageMaskAttacher::makeMask(
			*image, x, False, True, *this->_getLog(), True
		);
	}
	return image;
}

template<class T> void ImageFFTer<T>::_checkExists(
	const String& name
) {
	if (! name.empty()) {
		File f(name);
		ThrowIf (
			f.exists(), "File " + name + " already exists"
		);
	}
}
}
