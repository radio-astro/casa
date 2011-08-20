#include <display/region/QtRegionSourceFactory.h>
#include <display/region/QtRegionSource.qo.h>

namespace casa {
    namespace viewer {
	RegionSource *QtRegionSourceFactory::newSource( RegionCreator *rc ) {
	    return new QtRegionSource(rc,panel_);
	}
	  
    }
}
