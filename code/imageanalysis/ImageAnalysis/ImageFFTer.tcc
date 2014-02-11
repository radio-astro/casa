#include <imageanalysis/ImageAnalysis/ImageFFTer.h>

#include <stdcasa/cboost_foreach.h>

#include <tr1/memory>

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
	std::auto_ptr<ImageInterface<T> > clone(this->_getImage()->cloneII());
	SubImage<T> subImage = SubImageFactory<T>::createSubImage(
		*clone, *this->_getRegion(), this->_getMask(), this->_getLog().get(),
		False, AxesSpecifier(), this->_getStretch()
	);
	ImageFFT fft;
	if (_axes.size() == 0) {
		*this->_getLog() << LogIO::NORMAL << "FFT the direction coordinate" << LogIO::POST;
		fft.fftsky(subImage);
	}
	else {
		// Set vector of bools specifying axes
		Vector<Bool> which(subImage.ndim(), False);
		foreach_(uInt i, _axes) {
			which(_axes(i)) = True;
		}
		*this->_getLog() << LogIO::NORMAL << "FFT zero-based axes " << _axes << LogIO::POST;
		fft.fft(subImage, which);
	}

	String maskName("");
	if (! _real.empty()) {
		PagedImage<Float> x = _createFloatImage(_real, subImage);
		fft.getReal(x);
	}
	if (! _imag.empty()) {
		PagedImage<Float> x = _createFloatImage(_imag, subImage);
		fft.getImaginary(x);
	}
	if (! _amp.empty()) {
		PagedImage<Float> x = _createFloatImage(_amp, subImage);
		fft.getAmplitude(x);
	}
	if (! _phase.empty()) {
		PagedImage<Float> x = _createFloatImage(_phase, subImage);
		fft.getPhase(x);
	}
	if (! _complex.empty()) {
		PagedImage<Complex> x = _createComplexImage(_complex, subImage);
		fft.getComplex(x);
	}
}

template<class T> PagedImage<Float> ImageFFTer<T>::_createFloatImage(
	const String& name, const SubImage<T>& subImage
) const {
	*this->_getLog() << LogIO::NORMAL << "Creating image '"
		<< name << "'" << LogIO::POST;
	PagedImage<Float> image(
		subImage.shape(), subImage.coordinates(),
		name
	);
	if (subImage.isMasked()) {
		String x;
		ImageMaskAttacher::makeMask(
			image, x, False, True, *this->_getLog(), True
		);
	}
	return image;
}

template<class T> PagedImage<Complex> ImageFFTer<T>::_createComplexImage(
	const String& name, const SubImage<T>& subImage
) const {
	*this->_getLog() << LogIO::NORMAL << "Creating image '"
		<< name << "'" << LogIO::POST;
	PagedImage<Complex> image(
		subImage.shape(), subImage.coordinates(),
		name
	);
	if (subImage.isMasked()) {
		String x;
		ImageMaskAttacher::makeMask(
			image, x, False, True, *this->_getLog(), True
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
