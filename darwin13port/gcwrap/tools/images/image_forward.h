#define NO_INITIALIZE_STATICS 1
#include <coordsys_cmpt.h>
#undef NO_INITIALIZE_STATICS
#include <stdcasa/StdCasa/CasacSupport.h>
#include <images/Images/ImageInterface.h>
#include <measures/Measures/Stokes.h>
#include <components/ComponentModels/ComponentType.h>
#include <memory>
#include <tr1/memory.hpp>

namespace casa 
{
	class LogIO;
	class ImageAnalysis;
	class ImageStatsCalculator;
	template<class T> class ImageStatistics;
	template<class T> class ImageHistograms;
	template<class T> class SubImage;
	class ImageRegion;
	class LatticeExprNode;
	template<class T> class PtrHolder;
    class String;
	class SkyComponent;
	class DirectionCoordinate;
}
