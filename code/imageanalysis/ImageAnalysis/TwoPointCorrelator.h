#ifndef IMAGEANALYSIS_TWOPOINTCORRELATOR_H
#define IMAGEANALYSIS_TWOPOINTCORRELATOR_H

#include <lattices/LatticeMath/LatticeTwoPtCorr.h>

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/namespace.h>

namespace casa {

template <class T> class TwoPointCorrelator : public ImageTask<T> {
	// <summary>
	// Top level interface for two point correlation of images.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Smooths an image in one dimension.
	// </etymology>

	// <synopsis>
	// </synopsis>

public:

	TwoPointCorrelator() = delete;

	TwoPointCorrelator(
		SPCIIT image, const Record *const region, const String& mask,
		const String& outname, Bool overwrite
	);

	// destructor
	~TwoPointCorrelator() {}

	SPIIT correlate();

	String getClass() const { const static String s = "TwoPointCorrelator"; return s; }

	// Set the pixel axes over which to do correlation
	void setAxes(const IPosition& axes);

	void setMethod(const String& method) {
		_method = LatticeTwoPtCorr<T>::fromString(method);
	}

protected:

	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>();
	}

    inline Bool _supportsMultipleBeams() const {return True;}

private:
	IPosition _axes = IPosition();
	typename LatticeTwoPtCorr<T>::Method _method = LatticeTwoPtCorr<T>::STRUCTUREFUNCTION;

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/TwoPointCorrelator.tcc>
#endif

#endif
