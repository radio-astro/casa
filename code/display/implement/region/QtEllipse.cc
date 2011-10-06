#include <display/region/QtEllipse.qo.h>
#include <display/region/QtRegionSource.qo.h>

namespace casa {
    namespace viewer {
	// QtEllipse::QtEllipse( QtRegionSource *factory, double blc_x, double blc_y, double trc_x, double trc_y ) : 
	// 			QtRegion("ellipse name",factory), Ellipse(blc_x, blc_y, trc_x, trc_y) { }

	QtEllipse::QtEllipse( QtRegionSource *factory, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y, bool hold_signals ) :
				QtRegion("ellipse name",factory, hold_signals), Ellipse(wc, blc_x, blc_y, trc_x, trc_y) { mystate->init( ); }

	QtEllipse::~QtEllipse( ) { }

    }
}
