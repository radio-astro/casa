#include <display/region/QtPolygon.qo.h>
#include <display/region/QtRegionSource.qo.h>


namespace casa {
    namespace viewer {

	QtPolygon::QtPolygon( QtRegionSource *factory, WorldCanvas *wc, double x1, double y1, bool hold_signals ) :
				QtRegion("polygon name",factory, hold_signals), Polygon(wc, x1, y1) { mystate->init( ); }

	QtPolygon::QtPolygon( QtRegionSource *factory, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts, bool hold_signals ) :
				QtRegion("polygon name",factory, hold_signals), Polygon(wc, pts) { mystate->init( ); }

    }
}
