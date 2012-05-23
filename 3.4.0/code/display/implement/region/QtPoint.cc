#include <display/region/QtPoint.qo.h>
#include <display/region/RegionSource.h>

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/Display/WorldCanvas.h>
#include <display/ds9/ds9writer.h>

namespace casa {
    namespace viewer {
	// QtPoint::QtPoint( RegionSource *factory, double blc_x, double blc_y, double trc_x, double trc_y ) : 
	// 			QtRegion("point",factory), Point(blc_x, blc_y, trc_x, trc_y) { }

	QtPoint::QtPoint( QtRegionSourceKernel *factory, WorldCanvas *wc, double x, double y, bool hold_signals ) :
				QtRegion("point", factory, hold_signals), Point(wc, x, y) {
	    mystate->init( );
	    mystate->disableAnnotation(true);
	}

	QtPoint::~QtPoint( ) { }

	void QtPoint::output( ds9writer &out ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    std::string path = QtDisplayData::path(wc_->csMaster());
	    out.setCsysSource(path.c_str( ));
	    std::vector<std::pair<double,double> > pts(2);
	    pts[0].first = blc_x;
	    pts[0].second = blc_y;
	    pts[1].first = trc_x;
	    pts[1].second = trc_y;
	    out.point(wc_,pts);

	}
    }
}
