#ifndef IMAGEANALYSIS_IMAGEMAXFITTER_H
#define IMAGEANALYSIS_IMAGEMAXFITTER_H

#include <casa/namespace.h>

namespace casa {

template <class T> class ImageMaxFitter: public ImageTask<T> {
	// <summary>
	// Top level interface source max fitting
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// </etymology>

	// <synopsis>
	// </synopsis>

public:

	ImageMaxFitter(SPCIIT image, const Record *const &region);

	// destructor
	~ImageMaxFitter() {}

	String getClass() const { const static String s = "ImageMaxFitter"; return s; }

	Record fit(Bool point, Int width=5, Bool negfind=False, Bool list=True) const;

protected:

	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_FIRST_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>(1, Coordinate::DIRECTION);
	}

    inline Bool _supportsMultipleBeams() const {return True;}


};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageMaxFitter.tcc>
#endif

#endif
