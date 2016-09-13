#ifndef IMAGEANALYSIS_IMAGEMOMENTSTASK_H
#define IMAGEANALYSIS_IMAGEMOMENTSTASK_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <casa/namespace.h>

namespace casa {

class ImageMomentsProgressMonitor;

template <class T> class ImageMomentsTask : public ImageTask<T> {
	// <summary>
	// Top level interface for creating image moments
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

    ImageMomentsTask() = delete;

    // overwrite only applies to the smooth image. The moment images should
    // be removed if they exist.
	ImageMomentsTask(
		const SPCIIT image, const Record *const region,
		const String& mask, const String& smoothImageName,
		Bool overwrite
	);

	~ImageMomentsTask() {}

	String getClass() const { const static String s = "ImageMomentsTask"; return s; }

	// the first requested moments image is returned.
	SPIIT makeMoments() const;

	// set moments axis number
	void setAxis(Int axis);

	// set the include or exclude range. An empty vector means no range is set,
	// a one element vector means -range[0] to range[0] is set. If the vector
	// contains more than two elements, an exception is thrown. If isInclude is
	// True, then the range is the range to be included, excluded if False.
	void setIncludeExcludeRange(const std::vector<T>& range, Bool isInclude);

	// Set smoothing kernels
	void setKernels(const std::vector<String>& kernels) { _kernels = kernels; }

	// set smoothing kernel widths
	void setKernelWidths(const vector<Quantity>& kernelWidths) {
	    _kernelWidths = kernelWidths;
	}

	// set moment methods
	void setMethods(const vector<String>& methods) { _methods = methods; }

	// set moments to compute
	void setMoments(const Vector<Int>& moments);

	void setMomentsProgressMonitor(ImageMomentsProgressMonitor* progressMonitor ) {
	    _imageMomentsProgressMonitor = progressMonitor;
	}

	// set the base name for the moment images.
	void setMomentImageName(const String& name) { _momentName = name; }

	// Should the moment axis be removed. These rules will be followed:
	// If the moment axis is associated with a coordinate with one axis only,
	// the axis and its coordinate are physically removed from the output image.  Otherwise,
	// if <src>remove=True</src> then the output axis is logically removed from the
	// the output CoordinateSystem.  If <src>remove=False</src> then the axis
	// is retained with shape=1 and with its original coordinate information (which
	// is probably meaningless).
	void setRemoveAxis(Bool remove) { _removeAxis = remove; }

	// set smoothing axes
	void setSmoothAxes(const std::vector<uInt>& axes);

	// set desired signal-to-noise. Less than or equal to zero means
	// use the default value.
	void setSNR(Double snr) { _snr = snr; }

	// set desired standard deviation. Less than or equal to zero means
	// set to 0.
	void setStdDev(Double stddev) { _stddev = stddev; }

	// set the velocity type.
	void setVelocityType(const String& type) { _velocityType = type; }

protected:

	CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>();
	}

    Bool _supportsMultipleBeams() const {return True;}

    Bool _supportsMultipleRegions() const {return True;}

private:
    Vector<Int> _moments;
    Int _axis;
    std::vector<String> _methods, _kernels;
    std::vector<uInt> _smoothAxes;
    std::vector<Quantity> _kernelWidths;
    std::vector<T> _range;
    Bool _isIncludeRange, _removeAxis;
    Double _snr, _stddev;
    String _velocityType, _momentName;
    ImageMomentsProgressMonitor* _imageMomentsProgressMonitor;

    void _deleteTempImage(const String& tmpImage) const;

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageMomentsTask.tcc>
#endif

#endif
