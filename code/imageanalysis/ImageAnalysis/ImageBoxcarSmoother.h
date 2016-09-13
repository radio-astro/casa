#ifndef IMAGEANALYSIS_IMAGEBOXCARSMOOTHER_H
#define IMAGEANALYSIS_IMAGEBOXCARSMOOTHER_H

#include <imageanalysis/ImageAnalysis/Image1DSmoother.h>

#include <casa/namespace.h>

namespace casa {

template <class T> class ImageBoxcarSmoother : public Image1DSmoother<T> {
	// <summary>
	// Top level interface for 1-D boxcar smoothing of images.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Boxcar smooths an image in one dimension.
	// </etymology>

	// <synopsis>
	// </synopsis>

public:

	ImageBoxcarSmoother(
		const SPCIIT image,
		const Record *const region,
		const String& maskInp,
		const String& outname, Bool overwrite
	);

	// destructor
	~ImageBoxcarSmoother() {}

	String getClass() const { const static String s = "ImageBoxcarSmoother"; return s; }

    // set witdth of boxcar
    void setWidth(uInt w);

protected:

	SPIIT _smooth(
		const ImageInterface<T>& image
	) const;

private:

    uInt _width;

	// disallow default constructor
	ImageBoxcarSmoother();

	void _boxcarSmooth(Array<T>& out, const Array<T>& in) const ;
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageBoxcarSmoother.tcc>
#endif

#endif
