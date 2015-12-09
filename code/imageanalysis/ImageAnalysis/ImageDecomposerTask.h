#ifndef IMAGEANALYSIS_IMAGEDECOMPOSERTASK_H
#define IMAGEANALYSIS_IMAGEDECOMPOSERTASK_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/namespace.h>

namespace casa {

template <class T> class ImageDecomposerTask : public ImageTask<T> {
	// <summary>
	// Top level interface for decomposing of images.
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

	ImageDecomposerTask() = delete;

	ImageDecomposerTask(
		SPCIIT image, const Record *const region, const String& mask
	);

	// destructor
	~ImageDecomposerTask() {}

	Matrix<T> decompose(Matrix<Int>& blcs, Matrix<Int>& trcs);

	String getClass() const { const static String s = "ImageDecomposerTask"; return s; }

	void setSimple(Bool b) { _simple = b; }

	void setDeblendOptions(Double threshold, Int nContour, Int minRange, Int nAxis);

	void setFit(Bool b) { _fit = b; }

	void setFitOptions(Double maxrms, Int maxRetry, Int maxIter, Double convCriteria);

protected:

	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>();
	}

    inline Bool _supportsMultipleBeams() const {return True;}

private:
    Bool _simple = False;
    Double _threshold = -1;
    Int _ncontour = 11;
    Int _minrange = 1;
    Int _naxis = 2;
    Bool _fit = True;
    Double _maxrms = -1;
    Int _maxretry = -1;
    Int _maxiter = 256;
    Double _convcriteria = 0.0001;
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageDecomposerTask.tcc>
#endif

#endif
