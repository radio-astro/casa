#include <display/region/Point.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>

#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/PanelDisplay.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <images/Regions/WCEllipsoid.h>

#include <imageanalysis/Annotations/AnnRectBox.h>
#include <coordinates/Coordinates/CoordinateUtil.h>

namespace casa {
    namespace viewer {

	const int Point::radius = 10;

	Point::~Point( ) { }


	AnnRegion *Point::annotation( ) const {
	    if ( wc_ == 0 ) return 0;

	    const CoordinateSystem &cs = wc_->coordinateSystem( );

	    double wblc_x, wblc_y, wtrc_x, wtrc_y;
	    linear_to_world( wc_, blc_x, blc_y, trc_x, trc_y, wblc_x, wblc_y, wtrc_x, wtrc_y );
	    const Vector<String> &units = wc_->worldAxisUnits( );

	    Quantity qblc_x( wblc_x, units[0] );
	    Quantity qblc_y( wblc_y, units[1] );
	    Quantity qtrc_x( wtrc_x, units[0] );
	    Quantity qtrc_y( wtrc_y, units[1] );

	    const DisplayData *dd = wc_->displaylist().front();


	    Vector<Stokes::StokesTypes> stokes;
	    Int polaxis = CoordinateUtil::findStokesAxis(stokes, cs);

	    AnnRectBox *box = new AnnRectBox( qblc_x, qblc_y, qtrc_x, qtrc_y, cs, dd->dataShape(), stokes );

	    return box;
	}

	void Point::drawRegion( bool selected ) {
	    if ( wc_ == 0 ) return;

	    PixelCanvas *pc = wc_->pixelCanvas();
	    if(pc==0) return;

	    double center_x, center_y;
	    regionCenter( center_x, center_y );

	    int x, y;
	    linear_to_screen( wc_, blc_x, blc_y, x, y );
	    // drawing symbols would slot in here...
	    pc->drawFilledRectangle( x-1, y-1, x+1, y+1 );

	    if ( selected ) {
		// draw outline rectangle for resizing the point...
		pushDrawingEnv(DotLine);
		// While a circle would be a better choice, drawing a dotted circle
		// leaves terrible gaps in the circumference...  currently... <drs>
		// pc->drawEllipse(x, y, radius, radius, 0.0, True, 1.0, 1.0);
		pc->drawRectangle( x-radius, y-radius, x+radius, y+radius );
		popDrawingEnv( );
	    }

	}

	bool Point::clickWithin( double xd, double yd ) const {
	    int x, y, ptx, pty;
	    linear_to_screen( wc_, xd, yd, blc_x, blc_y, x, y, ptx, pty );
	    if ( x >  ptx - radius && x < ptx + radius  && y > pty - radius && y < pty + radius )
		return true;
	    else
		return false;
	}

	int Point::mouseMovement( double xd, double yd, bool other_selected ) {
	    int result = 0;

	    if ( visible_ == false ) return result;

	    int x, y, ptx, pty;
	    linear_to_screen( wc_, xd, yd, blc_x, blc_y, x, y, ptx, pty );

	    if ( x >  ptx - radius && x < ptx + radius  && y > pty - radius && y < pty + radius ) {
		result |= MouseSelected;
		result |= MouseRefresh;
		selected_ = true;
		draw( );
		if ( other_selected == false ) {
		    // mark flag as this is the region (how to mix in other shapes)
		    // of interest for statistics updates...
		    selectedInCanvas( );
		}
	    } else if ( selected_ == true ) {
		selected_ = false;
		draw( );
		result |= MouseRefresh;
	    }
	    return result;
	}

    }

}
