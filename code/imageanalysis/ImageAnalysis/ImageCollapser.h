#ifndef IMAGES_IMAGECOLLAPSER_H
#define IMAGES_IMAGECOLLAPSER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <imageanalysis/ImageAnalysis/ImageCollapserData.h>

#include <casa/namespace.h>

namespace casacore{

template <class T> class TempImage;
template <class T> class SubImage;
}

namespace casa {


template <class T> class ImageCollapser : public ImageTask<T> {
	// <summary>
	// Top level interface which allows collapsing of images along a single axis. An aggregate method
	// (average, sum, etc) is applied to the collapsed pixels.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Collapses image.
	// </etymology>

	// <synopsis>
	// High level interface for collapsing an image along a single axis.
	// </synopsis>

	// <example>
	// <srcblock>
	// ImageCollapser collapser(...);
	// collapser.collapse();
	// </srcblock>
	// </example>

public:

	// if <src>outname</src> is empty, no image will be written
	// if <src>overwrite</src> is true, if image already exists it will be removed
	// if <src>overwrite</src> is false, if image already exists exception will be thrown
	//
	// <group>

	ImageCollapser(
		const casacore::String& aggString, SPCIIT image,
		const casacore::Record *const regionRec,
		const casacore::String& maskInp, const casacore::IPosition& axes,
		casacore::Bool invertAxesSelection,
		const casacore::String& outname, casacore::Bool overwrite
	);

	ImageCollapser(
		const SPCIIT image,
		const casacore::IPosition& axes, casacore::Bool invertAxesSelection,
		const ImageCollapserData::AggregateType aggregateType,
		const casacore::String& outname, casacore::Bool overwrite
	);
	// </group>

	~ImageCollapser() {}

	// perform the collapse and return the resulting image.
	SPIIT collapse() const;

	static const map<casacore::uInt, T (*)(const casacore::Array<T>&)>* funcMap();

	casacore::String getClass() const { const static casacore::String name = "ImageCollapser"; return name; }

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<casacore::Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<casacore::Coordinate::Type>();
	}

	casacore::Bool _supportsMultipleRegions() const {return true;}

private:
	casacore::Bool _invertAxesSelection;
	casacore::IPosition _axes;
	ImageCollapserData::AggregateType _aggType;

	static map<casacore::uInt, T (*)(const casacore::Array<T>&)> _funcMap;

	// disallow default constructor
	ImageCollapser();

	void _invert();

	void _finishConstruction();

	// necessary to improve performance
	void _doMedian(
		SPCIIT image,
		casacore::TempImage<T>& outImage
	) const;

	void _attachOutputMask(
		casacore::TempImage<T>& outImage,
		const casacore::Array<casacore::Bool>& outMask
	) const;

	static void _zeroNegatives(casacore::Array<T>& arr);

	static const map<casacore::uInt, T (*)(const casacore::Array<T>&)>& _getFuncMap();
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageCollapser.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
