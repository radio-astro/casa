#include <display/region/QtEllipse.qo.h>
#include <display/region/QtRegionSource.qo.h>

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/Display/WorldCanvas.h>
#include <display/ds9/ds9writer.h>

namespace casa {
    namespace viewer {
	// QtEllipse::QtEllipse( QtRegionSource *factory, double blc_x, double blc_y, double trc_x, double trc_y ) : 
	// 			QtRegion("ellipse name",factory), Ellipse(blc_x, blc_y, trc_x, trc_y) { }

	QtEllipse::QtEllipse( QtRegionSource *factory, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y, bool hold_signals ) :
				QtRegion("ellipse name",factory, hold_signals), Ellipse(wc, blc_x, blc_y, trc_x, trc_y) { mystate->init( ); }

	QtEllipse::~QtEllipse( ) { }

	void QtEllipse::output( ds9writer &out ) const {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    std::string path = QtDisplayData::path(wc_->csMaster());
	    out.setCsysSource(path.c_str( ));
	    std::vector<std::pair<double,double> > pts(2);
	    pts[0].first = blc_x;
	    pts[0].second = blc_y;
	    pts[1].first = trc_x;
	    pts[1].second = trc_y;
	    out.ellipse(wc_,pts);
	}
    }
}
