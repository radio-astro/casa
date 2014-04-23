#ifndef IMAGES_IMAGECOLLAPSER_H
#define IMAGES_IMAGECOLLAPSER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <imageanalysis/ImageAnalysis/ImageCollapserData.h>

#include <casa/namespace.h>

#include <tr1/memory>

namespace casa {

template <class T> class TempImage;
template <class T> class SubImage;

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
	// if <src>overwrite</src> is True, if image already exists it will be removed
	// if <src>overwrite</src> is False, if image already exists exception will be thrown
	//
	// <group>

	ImageCollapser(
		String aggString, const SPCIIT image,
		const Record *const regionRec,
		const String& maskInp, const IPosition& axes,
		const String& outname, const Bool overwrite
	);

	ImageCollapser(
		const SPCIIT image,
		const IPosition& axes, const Bool invertAxesSelection,
		const ImageCollapserData::AggregateType aggregateType,
		const String& outname, const Bool overwrite
	);
	// </group>

	~ImageCollapser() {}

	// perform the collapse and return the resulting image.
	SPIIT collapse() const;

	static const map<uInt, T (*)(const Array<T>&)>* funcMap();

	String getClass() const { const static String name = "ImageCollapser"; return name; }

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>();
	}

private:
	Bool _invertAxesSelection;
	IPosition _axes;
	ImageCollapserData::AggregateType _aggType;

	static map<uInt, T (*)(const Array<T>&)> _funcMap;

	// disallow default constructor
	ImageCollapser();

	void _invert();

	void _finishConstruction();

	// necessary to improve performance
	void _doMedian(
		SPCIIT image,
		TempImage<T>& outImage
	) const;

	void _attachOutputMask(
		TempImage<T>& outImage,
		const Array<Bool>& outMask
	) const;

	static const map<uInt, T (*)(const Array<T>&)>& _getFuncMap();
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageCollapser.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
