#define NO_INITIALIZE_STATICS 1
#include <coordsys_cmpt.h>
#undef NO_INITIALIZE_STATICS
#include <stdcasa/StdCasa/CasacSupport.h>
#include <casa/Utilities/CountedPtr.h>
#include <imageanalysis/ImageTypedefs.h>
#include <imageanalysis/ImageAnalysis/ImageDecimatorData.h>
#include <measures/Measures/Stokes.h>
#include <components/ComponentModels/ComponentType.h>

namespace casa {
	class GaussianBeam;
	class ImageAnalysis;
	class ImageRegion;
	class ImageStatsCalculator;
	class LogIO;

	template<class T> class ImageHistograms;
	template<class T> class ImageRegridderBase;
	template<class T> class ImageStatistics;
	template<class T> class PtrHolder;
	template<class T> class SubImage;
	class LatticeExprNode;
    class String;
	class SkyComponent;
	class DirectionCoordinate;
}
