#include <imageanalysis/ImageAnalysis/ImageFFTer.h>

#include <imageanalysis/ImageAnalysis/ImageFFT.h>

namespace casa {

template<class T> ImageFFTer<T>::ImageFFTer(
	const SPCIIT image,
	const casacore::Record *const region,
	const casacore::String& maskInp, const casacore::Vector<casacore::uInt>& axes
) : ImageTask<T>(
		image, "", region, "", "", "",
		maskInp, "", false
	),
	_axes(axes), _real(), _imag(), _amp(),
	_phase(), _complex() {
	this->_construct();
}

template<class T> void ImageFFTer<T>::fft() const {
	*this->_getLog() << casacore::LogOrigin(getClass(), __FUNCTION__);
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
	SHARED_PTR<const casacore::SubImage<T> > subImage = SubImageFactory<T>::createSubImageRO(
		*this->_getImage(), *this->_getRegion(), this->_getMask(), this->_getLog().get(),
		casacore::AxesSpecifier(), this->_getStretch()
	);
	ImageFFT fft;
	if (_axes.size() == 0) {
		*this->_getLog() << casacore::LogIO::NORMAL << "FFT the direction coordinate" << casacore::LogIO::POST;
		fft.fftsky(*subImage);
	}
	else {
		// Set vector of bools specifying axes
		casacore::Vector<casacore::Bool> which(subImage->ndim(), false);
		for(casacore::uInt i: _axes) {
			which(_axes(i)) = true;
		}
		*this->_getLog() << casacore::LogIO::NORMAL << "FFT zero-based axes " << _axes << casacore::LogIO::POST;
		fft.fft(*subImage, which);
	}

	casacore::String maskName("");
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
	const casacore::String& name, const casacore::SubImage<T>& subImage
) const {
	*this->_getLog() << casacore::LogIO::NORMAL << "Creating image '"
		<< name << "'" << casacore::LogIO::POST;
	SPIIF image(
	    new casacore::PagedImage<casacore::Float>(
	        subImage.shape(), subImage.coordinates(),
	        name
	    )
	);
	if (subImage.isMasked()) {
		casacore::String x;
		ImageMaskAttacher::makeMask(
			*image, x, false, true, *this->_getLog(), true
		);
	}
	return image;
}

template<class T> SPIIC ImageFFTer<T>::_createComplexImage(
	const casacore::String& name, const casacore::SubImage<T>& subImage
) const {
	*this->_getLog() << casacore::LogIO::NORMAL << "Creating image '"
		<< name << "'" << casacore::LogIO::POST;
	SPIIC image(
	    new casacore::PagedImage<casacore::Complex>(
	        subImage.shape(), subImage.coordinates(),
	        name
	    )
	);
	if (subImage.isMasked()) {
		casacore::String x;
		ImageMaskAttacher::makeMask(
			*image, x, false, true, *this->_getLog(), true
		);
	}
	return image;
}

template<class T> void ImageFFTer<T>::_checkExists(
	const casacore::String& name
) {
	if (! name.empty()) {
		casacore::File f(name);
		ThrowIf (
			f.exists(), "casacore::File " + name + " already exists"
		);
	}
}
}
