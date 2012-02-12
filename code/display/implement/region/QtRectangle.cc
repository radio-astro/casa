#include <display/region/QtRectangle.qo.h>
#include <display/region/QtRegionSource.qo.h>

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/Display/WorldCanvas.h>
#include <display/ds9/ds9writer.h>

namespace casa {
    namespace viewer {

	QtRectangle::QtRectangle( QtRegionSource *factory, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y, bool hold_signals ) :
				QtRegion("rectangle name",factory, hold_signals), Rectangle(wc, blc_x, blc_y, trc_x, trc_y) { mystate->init( ); }

	QtRectangle::~QtRectangle( ) { }

	void QtRectangle::output( ds9writer &out ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
	    std::string path = QtDisplayData::path(wc_->csMaster());
	    out.setCsysSource(path.c_str( ));
	    std::vector<std::pair<double,double> > pts(2);
	    pts[0].first = blc_x;
	    pts[0].second = blc_y;
	    pts[1].first = trc_x;
	    pts[1].second = trc_y;
	    out.rectangle(wc_,pts);

	}

    }
}
