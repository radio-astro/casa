#ifndef IMAGEANALYSIS_PIXELVALUEMANIPULATOR_H
#define IMAGEANALYSIS_PIXELVALUEMANIPULATOR_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

namespace casa {

template <class T> class PixelValueManipulator : public ImageTask<T> {
	// <summary>
	// Top level interface for getting and setting image pixel values.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Manipulates pixel values.
	// </etymology>

	// <synopsis>
	// Top level interface for getting and setting image pixel values.
	// </synopsis>

	// <example>
	// </example>

public:

	PixelValueManipulator(
		const SPCIIT image,
		const Record *const regionRec,
		const String& mask
	);

	~PixelValueManipulator() {}

	// set axes to average over. If invert is True, select all axes other than
	// the specified axes to average over.
	void setAxes(const IPosition& axes, Bool invert=False);

	// Get pixel values, pixel mask values, etc.
	// The return Record has the following fields:
	// 'values' => Array<T> of pixel values
	// 'mask'   => Array<Bool> of pixel mask values
	Record get() const;

	String getClass() const { const static String name = "PixelValueManipulator"; return name; }

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline std::vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return std::vector<Coordinate::Type>();
	}

    inline Bool _supportsMultipleRegions() {return True;}

private:
    IPosition _axes;

	// disallow default constructor
	PixelValueManipulator();

};

}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/PixelValueManipulator.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
