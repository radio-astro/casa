#ifndef IMAGEANALYSIS_IMAGEREBINNER_H
#define IMAGEANALYSIS_IMAGEREBINNER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/namespace.h>

namespace casa {

template <class T> class ImageRebinner : public ImageTask<T> {
	// <summary>
	// Top level interface for image rebinning
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Rebins an image to larger pixels.
	// </etymology>

	// <synopsis>
	// Top level interface for rebinning images.
	// </synopsis>

public:


	ImageRebinner(
		const SPCIIT image, const Record *const region,
		const String& maskInp,
		const String& outname, Bool overwrite
	);

	// destructor
	virtual ~ImageRebinner() {}

	SPIIT rebin() const;

	virtual String getClass() const { const static String s = "ImageRebinner"; return s; }

	// Set the factors (1 for each axis) to rebin by.
	void setFactors(const Vector<Int>& f);

	void setCrop(Bool c) { _crop = c; }

protected:


	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>();
	}

    inline Bool _supportsMultipleBeams() const {return False;}

private:
	IPosition _factors;
	Bool _crop;

	// disallow default constructor
	ImageRebinner();
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageRebinner.tcc>
#endif

#endif
