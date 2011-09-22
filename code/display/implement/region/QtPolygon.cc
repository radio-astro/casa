#include <display/region/QtPolygon.qo.h>
#include <display/region/QtRegionSource.qo.h>


namespace casa {
    namespace viewer {

	QtPolygon::QtPolygon( QtRegionSource *factory, WorldCanvas *wc, double x1, double y1, bool hold_signals ) :
				QtRegion("polygon name",factory, hold_signals), Polygon(wc, x1, y1) { }
    }
}
