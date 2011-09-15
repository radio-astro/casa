#include <display/region/QtPoint.qo.h>
#include <display/region/QtRegionSource.qo.h>

namespace casa {
    namespace viewer {
	// QtPoint::QtPoint( QtRegionSource *factory, double blc_x, double blc_y, double trc_x, double trc_y ) : 
	// 			QtRegion("point name",factory), Point(blc_x, blc_y, trc_x, trc_y) { }

	QtPoint::QtPoint( QtRegionSource *factory, WorldCanvas *wc, double x, double y ) :
				QtRegion("point name",factory), Point(wc, x, y) { }

	QtPoint::~QtPoint( ) { }

    }
}
