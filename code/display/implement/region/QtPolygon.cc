#include <display/region/QtPolygon.qo.h>
#include <display/region/QtRegionSource.qo.h>


namespace casa {
    namespace viewer {

	QtPolygon::QtPolygon( QtRegionSource *factory, WorldCanvas *wc, double x1, double y1 ) :
				QtRegion("polygon name",factory), Polygon(wc, x1, y1) { }
    }
}
