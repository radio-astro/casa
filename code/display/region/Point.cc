#include <display/region/Point.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>

#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/PanelDisplay.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <images/Regions/WCEllipsoid.h>

#include <imageanalysis/Annotations/AnnSymbol.h>
#include <coordinates/Coordinates/CoordinateUtil.h>

namespace casa {
    namespace viewer {

	const int Point::radius = 10;

	Point::~Point( ) { }


	AnnotationBase *Point::annotation( ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

	    const CoordinateSystem &cs = wc_->coordinateSystem( );

	    double wx, wy;
	    try { linear_to_world( wc_, blc_x, blc_y, wx, wy ); } catch (...) { return 0; }
	    const Vector<String> &units = wc_->worldAxisUnits( );

	    Quantity qx( wx, units[0] );
	    Quantity qy( wy, units[1] );

// 	    const DisplayData *dd = wc_->displaylist().front();

	    // Vector<Stokes::StokesTypes> stokes;
	    // Int polaxis = CoordinateUtil::findStokesAxis(stokes, cs);

	    AnnSymbol *symbol = 0;
	    try {
		symbol = new AnnSymbol( qx, qy, cs,
					marker_ == QtMouseToolNames::SYM_DOWN_RIGHT_ARROW ? AnnSymbol::TRIANGLE_DOWN :
					marker_ == QtMouseToolNames::SYM_UP_LEFT_ARROW    ? AnnSymbol::TRIANGLE_UP :
					marker_ == QtMouseToolNames::SYM_DOWN_LEFT_ARROW  ? AnnSymbol::TRIANGLE_LEFT :
					marker_ == QtMouseToolNames::SYM_UP_RIGHT_ARROW   ? AnnSymbol::TRIANGLE_RIGHT :
					marker_ == QtMouseToolNames::SYM_PLUS             ? AnnSymbol::PLUS :
					marker_ == QtMouseToolNames::SYM_X                ? AnnSymbol::X :
					marker_ == QtMouseToolNames::SYM_CIRCLE           ? AnnSymbol::CIRCLE :
					marker_ == QtMouseToolNames::SYM_DIAMOND          ? AnnSymbol::DIAMOND :
					marker_ == QtMouseToolNames::SYM_SQUARE           ? AnnSymbol::SQUARE :
					AnnSymbol::POINT );
	    } catch ( AipsError &e ) {
		cerr << "Error encountered creating an AnnSymbol:" << endl;
		cerr << "\t\"" << e.getMesg( ) << "\"" << endl;
	    } catch ( ... ) {
		cerr << "Error encountered creating an AnnSymbol..." << endl;
	    }

	    return symbol;
	}

	void Point::fetch_region_details( Region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts, 
					  std::vector<std::pair<double,double> > &world_pts ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

	    type = PointRegion;

	    double wblc_x, wblc_y;
	    try { linear_to_world( wc_, blc_x, blc_y, wblc_x, wblc_y ); } catch(...) { return; }

	    double pblc_x, pblc_y;
	    try { linear_to_pixel( wc_, blc_x, blc_y, pblc_x, pblc_y ); } catch(...) { return; }

	    pixel_pts.resize(1);
	    pixel_pts[0].first = (int) pblc_x;
	    pixel_pts[0].second = (int) pblc_y;

	    world_pts.resize(1);
	    world_pts[0].first = wblc_x;
	    world_pts[0].second = wblc_y;
	}

	void Point::drawRegion( bool selected ) {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

	    PixelCanvas *pc = wc_->pixelCanvas();
	    if(pc==0) return;

	    double center_x, center_y;
	    regionCenter( center_x, center_y );

	    int x, y;
	    try { linear_to_screen( wc_, blc_x, blc_y, x, y ); } catch(...) { return; }
	    // drawing symbols would slot in here...
	    switch ( marker_ ) {
		case QtMouseToolNames::SYM_DOWN_RIGHT_ARROW:
		    pc->drawLine( x-5-1, y+1, x-1, y+1 );
		    pc->drawLine( x-1, y+1, x-1, y+5+1 );
		    pc->drawLine( x-7-1, y+7+1, x-1, y+1);
		    break;
		case QtMouseToolNames::SYM_UP_RIGHT_ARROW:
		    pc->drawLine( x-5-1, y-1, x-1, y-1 );
		    pc->drawLine( x-1, y-1, x-1, y-5-1 );
		    pc->drawLine( x-7-1, y-7-1, x-1, y-1);
		    break;
		case QtMouseToolNames::SYM_DOWN_LEFT_ARROW:
		    pc->drawLine( x+5+1, y+1, x+1, y+1 );
		    pc->drawLine( x+1, y+1, x+1, y+5+1 );
		    pc->drawLine( x+7+1, y+7+1, x+1, y+1);
		    break;
		case QtMouseToolNames::SYM_UP_LEFT_ARROW:
		    pc->drawLine( x+5+1, y-1, x+1, y-1 );
		    pc->drawLine( x+1, y-1, x+1, y-5-1 );
		    pc->drawLine( x+7+1, y-7-1, x+1, y-1);
		    break;
		case QtMouseToolNames::SYM_PLUS:
		    pc->drawLine( x, y+5, x, y-5 );
		    pc->drawLine( x+5, y, x-5, y );
		    break;
		case QtMouseToolNames::SYM_X:
		    pc->drawLine( x-5, y+5, x+5, y-5 );
		    pc->drawLine( x-5, y-5, x+5, y+5 );
		    break;
		case QtMouseToolNames::SYM_CIRCLE:
		    pc->drawEllipse( x, y, 6, 6, 0 );
		    break;
		case QtMouseToolNames::SYM_DIAMOND:
		  { const int minor=4;
		    const int major=6;
		    pc->drawLine( x-minor, y, x, y+major );
		    pc->drawLine( x, y+major, x+minor, y );
		    pc->drawLine( x+minor, y, x, y-major );
		    pc->drawLine( x, y-major, x-minor, y ); }
		    break;
		case QtMouseToolNames::SYM_SQUARE:
		    pc->drawRectangle(x-4,y-4,x+4,y+4);
		    break;
		case QtMouseToolNames::SYM_DOT:
		default:
		    pc->drawFilledRectangle( x-1, y-1, x+1, y+1 );
	    }

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

	bool Point::setMarker( QtMouseToolNames::PointRegionSymbols sym ) {
	    int isym = (int) sym;
	    if ( isym < 0 || isym > QtMouseToolNames::SYM_POINT_REGION_COUNT )
		return false;
	    marker_ = sym;
	    return true;
	}

	bool Point::clickWithin( double xd, double yd ) const {
	    int x, y, ptx, pty;
	    try { linear_to_screen( wc_, xd, yd, blc_x, blc_y, x, y, ptx, pty ); } catch(...) { return false; }
	    if ( x >  ptx - radius && x < ptx + radius  && y > pty - radius && y < pty + radius )
		return true;
	    else
		return false;
	}

	// returns point state (Region::PointLocation)
	Region::PointInfo Point::checkPoint( double xd, double yd ) const {
	    unsigned int result = 0;
	    int x, y, ptx, pty;
	    try { linear_to_screen( wc_, xd, yd, blc_x, blc_y, x, y, ptx, pty ); } catch(...) { return Region::PointInfo(0,0,PointOutside); }

	    if ( x >  ptx - radius && x < ptx + radius  && y > pty - radius && y < pty + radius )
		result |= PointInside;

	    return PointInfo(xd,yd,result == 0 ? (unsigned int) PointOutside : result);
	}

        // returns mouse state (Region::MouseState)
	unsigned int Point::mouseMovement( double xd, double yd, bool other_selected ) {
	    unsigned int result = 0;

	    if ( visible_ == false ) return result;

	    int x, y, ptx, pty;
	    try { linear_to_screen( wc_, xd, yd, blc_x, blc_y, x, y, ptx, pty ); } catch(...) { return 0; }

	    if ( x >  ptx - radius && x < ptx + radius  && y > pty - radius && y < pty + radius ) {
		result |= MouseSelected;
		result |= MouseRefresh;
		selected_ = true;
		draw( other_selected );
		if ( other_selected == false ) {
		    // mark flag as this is the region (how to mix in other shapes)
		    // of interest for statistics updates...
		    selectedInCanvas( );
		}
	    } else if ( selected_ == true ) {
		selected_ = false;
		draw( other_selected );
		result |= MouseRefresh;
	    }
	    return result;
	}

	std::list<RegionInfo> * Point::generate_dds_centers(bool /* skycomp */){
		// In principle there is no need to implement this,
		// it would go to Rectangle::generate_dds_centers() otherwise
		// and really try to fit a Gaussian to a point, certainly
		// without success. Implementing it here as an empty method
		// just accelerates matters (MK)
		return new std::list<RegionInfo>( );
	}
    }

}
