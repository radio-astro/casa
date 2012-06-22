#include <display/region/QtRegionSourceFactory.h>
#include <display/region/QtRegionSource.qo.h>

namespace casa {
    namespace viewer {
	RegionSource *QtRegionSourceFactory::newSource( RegionCreator *rc ) {
	    // passing kernel_ here ensures that all regions created by panel_ will
	    // be managed by the same RegionSource...
	    if ( kernel_.get( ) == 0 ) kernel_ = RegionSource::shared_kernel_ptr_type(new QtRegionSourceKernel(panel_));
	    return new QtRegionSource(rc,panel_,kernel_);
	}
	  
    }
}
