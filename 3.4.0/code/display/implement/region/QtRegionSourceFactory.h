
#ifndef REGION_QTREGIONSOURCEFACTORY_H_
#define REGION_QTREGIONSOURCEFACTORY_H_
#include <display/region/RegionSourceFactory.h>

namespace casa {

    class QtDisplayPanelGui;

    namespace viewer {

	class QtRegionSourceFactory : public RegionSourceFactory {
	    public:
		RegionSource *newSource( RegionCreator *rc );
		QtRegionSourceFactory( QtDisplayPanelGui *panel ) : panel_(panel) { }
		~QtRegionSourceFactory( ) { }
	    private:
		RegionSource::shared_kernel_ptr_type kernel_;
		QtDisplayPanelGui *panel_;
	};
    }
}

#endif
