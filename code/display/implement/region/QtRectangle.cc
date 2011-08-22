#include <display/region/QtRectangle.qo.h>
#include <display/region/QtRegionSource.qo.h>

namespace casa {
    namespace viewer {
	QtRectangle::QtRectangle( QtRegionSource *factory, double blc_x, double blc_y, double trc_x, double trc_y ) : 
				QtRegion("rectangle name",factory), Rectangle(blc_x, blc_y, trc_x, trc_y) { }

	QtRectangle::QtRectangle( QtRegionSource *factory, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) :
				QtRegion("rectangle name",factory), Rectangle(wc, blc_x, blc_y, trc_x, trc_y) { }

	QtRectangle::~QtRectangle( ) { }

    }
}
