
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
		QtDisplayPanelGui *panel_;
	};
    }
}

#endif
