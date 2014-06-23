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

	BeamManipulator(SPIIT image);

	~BeamManipulator() {}

	void set(
		const Quantity& major, const Quantity& minor,
		const Quantity& pa, const Record& rec,
	    Int channel, Int polarization
	);

	// set all beams in one go
	void set(const ImageBeamSet& beamSet);

	// remove existing beam(s)
	void remove();

	void setVerbose(Bool v);

private:
	SPIIT _image;
	std::tr1::shared_ptr<LogIO> _log;

	BeamManipulator() {}

	BeamManipulator operator=(const BeamManipulator& other) {}
};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/BeamManipulator.tcc>
#endif

#endif
