
#ifndef REGION_REGIONSOURCEFACTORY_H_
#define REGION_REGIONSOURCEFACTORY_H_
#include <display/region/RegionCreator.h>
#include <display/region/RegionSource.h>
#include <casa/Utilities/CountedPtr.h>

namespace casa {

	namespace viewer {

		class RegionSourceFactory {
		public:
			virtual RegionSource *newSource( RegionCreator * ) = 0;
			virtual ~RegionSourceFactory( ) { }
			RegionSourceFactory( ) { }
		};
	}
}

#endif
