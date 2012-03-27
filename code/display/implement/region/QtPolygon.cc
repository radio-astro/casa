#include <display/region/QtPolygon.qo.h>
#include <display/region/QtRegionSource.qo.h>

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/Display/WorldCanvas.h>
#include <display/ds9/ds9writer.h>

namespace casa {
    namespace viewer {

	QtPolygon::QtPolygon( QtRegionSource *factory, WorldCanvas *wc, double x1, double y1, bool hold_signals ) :
				QtRegion("polygon",factory, hold_signals), Polygon(wc, x1, y1) { mystate->init( ); }

	QtPolygon::QtPolygon( QtRegionSource *factory, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts, bool hold_signals ) :
				QtRegion("polygon",factory, hold_signals), Polygon(wc, pts) { mystate->init( ); }

 	void QtPolygon::output( ds9writer &out ) const {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    std::string path = QtDisplayData::path(wc_->csMaster());
	    out.setCsysSource(path.c_str( ));
	    out.polygon(wc_,drawing_points( ));
	}

	void QtPolygon::polygonComplete( ) {
	    // for polygons signals remain blocked until polygon is complete...
	    releaseSignals( );
	}
    }
}
