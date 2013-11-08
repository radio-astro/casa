
#ifndef REGION_QTREGIONSOURCEFACTORY_H_
#define REGION_QTREGIONSOURCEFACTORY_H_
#include <display/region/RegionSourceFactory.h>

namespace casa {

	class QtDisplayPanelGui;

	namespace viewer {

		class QtRegionDock;

		class QtRegionSourceFactory : public RegionSourceFactory {
		public:
			RegionSource *newSource( RegionCreator *rc );
			QtRegionSourceFactory( QtDisplayPanelGui *panel ) : panel_(panel) { }
			~QtRegionSourceFactory( ) { }
			QtRegionDock *regionDock( );
		private:
			RegionSource::shared_kernel_ptr_type kernel_;
			QtDisplayPanelGui *panel_;
		};
	}
}

#endif
