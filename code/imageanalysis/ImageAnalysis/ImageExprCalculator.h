#ifndef IMAGEANALYSIS_ImageExprCalculator_H
#define IMAGEANALYSIS_ImageExprCalculator_H

#include <casa/namespace.h>

namespace casa {

class String;

template <class T> class ImageExprCalculator {
	// <summary>
	// Top level interface for computing the results of an image expression
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

    ImageExprCalculator() = delete;

    // expression is the lattice expression to compute
	ImageExprCalculator(
		const String& expression, const String& outname, Bool overwrite
	);

	~ImageExprCalculator() {}

	// the image from which to copy metadata, including the coordinate system,
	// imageInfo() and miscInfo()
	void setCopyMetaDataFromImage(const String& name) { _copyMetaDataFromImage = name; }

	String getClass() const { const static String s = "ImageExprCalculator"; return s; }

	// Compute the expression and return the image.
	SPIIT compute() const;

private:
    String _expr, _copyMetaDataFromImage, _outname;
    Bool _overwrite;
    mutable LogIO _log;

    SPIIT _imagecalc(
    	const LatticeExprNode& node, const IPosition& shape,
    	const CoordinateSystem& csys, const LELImageCoord* const imCoord
    ) const;

    static void _makeRegionBlock(
    	PtrBlock<const ImageRegion*>& regions,
    	const Record& Regions
    );

    void _checkImages() const;
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageExprCalculator.tcc>
#endif

#endif
