#ifndef IMAGES_IMAGEFFTER_H
#define IMAGES_IMAGEFFTER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/namespace.h>

namespace casa {

template <class T> class ImageFFTer : public ImageTask<T> {
	// <summary>
	// Top level interface which allows FFTing of images.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// FFTs image.
	// </etymology>

	// <synopsis>
	// High level interface for FFTing an image.
	// </synopsis>

public:

	ImageFFTer(
		const SPCIIT image,
		const Record *const region,
		const String& maskInp, const IPosition& axes
	);

	// destructor
	~ImageFFTer() {}

	void fft() const;

	void setReal(const String& name) { _real = name; }
	void setImag(const String& name) { _imag = name; }
	void setAmp(const String& name) { _amp = name; }
	void setPhase(const String& name) { _phase = name; }
	void setComplex(const String& name) { _complex = name; }

	String getClass() const { const static String s = "ImageFFTer"; return s; }

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>();
	}

private:
	IPosition _axes;
	String _real, _imag, _amp, _phase, _complex;

	// disallow default constructor
	ImageFFTer();

	PagedImage<Float> _createFloatImage(
		const String& name, const SubImage<T>& subimage
	) const;

	PagedImage<Complex> _createComplexImage(
		const String& name, const SubImage<T>& subimage
	) const;

	static void _checkExists(const String& name);
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageFFTer.tcc>
#endif

#endif
