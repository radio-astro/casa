#ifndef IMAGES_PIXELVALUEMANIPULATORDATA_H
#define IMAGES_PIXELVALUEMANIPULATORDATA_H

#include <casa/typeinfo.h>

namespace casa {

class String;

class PixelValueManipulatorData {
	// <summary>
	// Non-templated data related bits for PixelValueManipulator.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Data for PixelValueManipulator.
	// </etymology>

	// <synopsis>
	// </synopsis>

	// <example>
	// </example>

public:

	enum SpectralType {
		DEFAULT,
		RELATIVISTIC,
		RADIO_VELOCITY,
		OPTICAL_VELOCITY,
		AIR_WAVELENGTH,
		WAVELENGTH,
	};

	// destructor
	~PixelValueManipulatorData() {}

	// convert a string to a spectral type. Acceptable values are (case insensitive,
	// minimum match supported): "default", "optical velocity", "radio velocity",
	// "wavelength", "air wavelength", and "" (which returns DEFAULT)
	static SpectralType spectralType(const String& specString);

private:

	// disallow default constructor
	PixelValueManipulatorData() {}

};
}

#endif
