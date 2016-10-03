#define NO_INITIALIZE_STATICS 1
#include <coordsys_cmpt.h>
#undef NO_INITIALIZE_STATICS
#include <stdcasa/StdCasa/CasacSupport.h>
#include <casa/Utilities/CountedPtr.h>
#include <imageanalysis/ImageTypedefs.h>
#include <imageanalysis/ImageAnalysis/ImageDecimatorData.h>
#include <measures/Measures/Stokes.h>
#include <components/ComponentModels/ComponentType.h>

namespace casacore{

	class GaussianBeam;
	class ImageRegion;
	class LogIO;
	template<class T> class ImageStatistics;
	template<class T> class PtrHolder;
	template<class T> class SubImage;
	class LatticeExprNode;
    class String;
	class DirectionCoordinate;
}

namespace casa {
	class ImageAnalysis;
	class ImageStatsCalculator;

	template<class T> class ImageHistograms;
	template<class T> class ImageRegridderBase;
	class SkyComponent;
}
