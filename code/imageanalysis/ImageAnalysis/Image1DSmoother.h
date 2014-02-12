#ifndef IMAGEANALYSIS_IMAGE1DSMOOTHER_H
#define IMAGEANALYSIS_IMAGE1DSMOOTHER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <imageanalysis/ImageAnalysis/ImageDecimatorData.h>

#include <casa/namespace.h>

namespace casa {

template <class T> class Image1DSmoother : public ImageTask<T> {
	// <summary>
	// Top level interface for 1-D smoothing of images.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Smooths an image in one dimension.
	// </etymology>

	// <synopsis>
	// Top level interface for removing image planes.
	// </synopsis>

public:

	// destructor
	virtual ~Image1DSmoother() {}

	SPIIF smooth(Bool wantReturn) const;

	virtual String getClass() const { const static String s = "Image1DSmoother"; return s; }

	// Keep only every nth plane
	void setDecimate(Bool b) {_decimate = b; }

	// Set the pixel axis number along which the smoothing will occur
	void setAxis(uInt n);

	// Set the decimation function.
	inline void setDecimationFunction(ImageDecimatorData::Function f) {
		_decimationFunction = f;
	}

protected:

	Image1DSmoother(
		const SPCIIT image,
		const Record *const region,
		const String& maskInp,
		const String& outname, Bool overwrite
	);

	virtual SPIIT _smooth(const ImageInterface<T>& image) const = 0;

	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>();
	}

    inline Bool _supportsMultipleBeams() const {return False;}

    inline Bool _getDecimate() const { return _decimate; }

    inline ImageDecimatorData::Function _getDecimationFunction() const {
    	return _decimationFunction;
    }

    inline uInt _getAxis() const { return _axis; }

private:
	uInt _axis;
	Bool _decimate;
	ImageDecimatorData::Function _decimationFunction;

	// disallow default constructor
	Image1DSmoother();
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/Image1DSmoother.tcc>
#endif

#endif
