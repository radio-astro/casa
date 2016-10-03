#ifndef IMAGES_BEAMMANIPULATOR_H
#define IMAGES_BEAMMANIPULATOR_H

#include <imageanalysis/ImageTypedefs.h>

#include <casa/Quanta/Quantum.h>

namespace casa {

template <class T> class BeamManipulator {
	// <summary>
	// Manipulate beams associated with images.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Manipulates beams.
	// </etymology>

	// <synopsis>
	// Manipulate beams associated with images.
	// </synopsis>

public:

	BeamManipulator() = delete;
	
	BeamManipulator(SPIIT image);

	BeamManipulator operator=(const BeamManipulator& other) = delete;
	
	~BeamManipulator() {}

	// remove all existing beam(s)
	void remove();

	// rotate all the beams counterclockwise by the specified angle
	void rotate(const casacore::Quantity& angle);

	void set(
		const casacore::Quantity& major, const casacore::Quantity& minor,
		const casacore::Quantity& pa, const casacore::Record& rec,
	    casacore::Int channel, casacore::Int polarization
	);

	// set all beams in one go
	void set(const casacore::ImageBeamSet& beamSet);

	void setVerbose(casacore::Bool v);

private:
	SPIIT _image;
	SHARED_PTR<casacore::LogIO> _log;
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/BeamManipulator.tcc>
#endif

#endif
