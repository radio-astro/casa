//# Point.cc: non-GUI point region
//# with surrounding Gui functionality
//# Copyright (C) 2011,2012
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $

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


		void Point::releaseSignals( ) {
			Region::releaseSignals( );
			refresh_state_gui( );
		}

		AnnotationBase *Point::annotation( ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

			const DisplayCoordinateSystem &cs = wc_->coordinateSystem( );

			double wx, wy;
			try {
				linear_to_world( wc_, blc_x, blc_y, wx, wy );
			} catch (...) {
				return 0;
			}
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
				                        AnnSymbol::POINT, Vector<Stokes::StokesTypes>(0) );

				int scale = markerScale( );
				if ( scale >= 0 )
					symbol->setSymbolSize((unsigned int)scale);

			} catch ( AipsError &e ) {
				cerr << "Error encountered creating an AnnSymbol:" << endl;
				cerr << "\t\"" << e.getMesg( ) << "\"" << endl;
			} catch ( ... ) {
				cerr << "Error encountered creating an AnnSymbol..." << endl;
			}

			return symbol;
		}

		void Point::fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
		                                  std::vector<std::pair<double,double> > &world_pts ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			type = region::PointRegion;

			double wblc_x, wblc_y;
			try {
				linear_to_world( wc_, blc_x, blc_y, wblc_x, wblc_y );
			} catch(...) {
				return;
			}

			double pblc_x, pblc_y;
			try {
				linear_to_pixel( wc_, blc_x, blc_y, pblc_x, pblc_y );
			} catch(...) {
				return;
			}

			pixel_pts.resize(1);
			pixel_pts[0].first = (int) pblc_x;
			pixel_pts[0].second = (int) pblc_y;

			world_pts.resize(1);
			world_pts[0].first = wblc_x;
			world_pts[0].second = wblc_y;
		}


		void Point::draw_arrow( PixelCanvas *pc, int x, int y, int xsign, int ysign, int scale_unit, int scale ) {
			int off = 5 + (scale * scale_unit);
			int off2 = 7 + (scale * (int)((double)scale_unit*(7.0/5.0)));
			Vector<int> xv(3),yv(3);

			int ptx = x+xsign;		// arrow point
			int pty = y+ysign;
			xv[2] = ptx+xsign*off2;
			yv[2] = pty+ysign*off2;

			xv[0] = ptx+xsign*off;
			yv[0] = pty;
			xv[1] = ptx;
			yv[1] = pty;
			pc->drawPolyline(xv,yv);
			xv[0] = ptx;
			yv[0] = pty+ysign*off;
			pc->drawPolyline(xv,yv);
		}

		void Point::drawRegion( bool /*selected*/ ) {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;

			double center_x, center_y;
			linearCenter( center_x, center_y );

			int x, y;
			try {
				linear_to_screen( wc_, blc_x, blc_y, x, y );
			} catch(...) {
				return;
			}

			const int scale_unit=2;
			const int scale = markerScale( )-1;
			const int inc = scale*scale_unit;
			switch ( marker_ ) {
			case QtMouseToolNames::SYM_DOWN_RIGHT_ARROW:
				draw_arrow( pc, x, y, -1, 1, scale_unit, scale );
				break;
			case QtMouseToolNames::SYM_UP_RIGHT_ARROW:
				draw_arrow( pc, x, y, -1, -1, scale_unit, scale );
				break;
			case QtMouseToolNames::SYM_DOWN_LEFT_ARROW:
				draw_arrow( pc, x, y, 1, 1, scale_unit, scale );
				break;
			case QtMouseToolNames::SYM_UP_LEFT_ARROW:
				draw_arrow( pc, x, y, 1, -1, scale_unit, scale );
				break;
			case QtMouseToolNames::SYM_PLUS:
				pc->drawLine( x, y+5+inc, x, y-5-inc );
				pc->drawLine( x+5+inc, y, x-5-inc, y );
				break;
			case QtMouseToolNames::SYM_X: {
				pc->drawLine( x-5-inc, y+5+scale*scale_unit, x+5+scale*scale_unit, y-5-scale*scale_unit );
				pc->drawLine( x-5-inc, y-5-inc, x+5+inc, y+5+inc );
			}
			break;
			case QtMouseToolNames::SYM_CIRCLE:
				pc->drawEllipse( x, y, 6+inc, 6+inc, 0 );
				break;
			case QtMouseToolNames::SYM_DIAMOND: {
				const int minor=4;
				const int major=6;
				const int mjr = scale * (int)((double)scale_unit*(6.0/4.0));
				pc->drawLine( x-minor-inc, y, x, y+major+mjr );
				pc->drawLine( x, y+major+mjr, x+minor+inc, y );
				pc->drawLine( x+minor+inc, y, x, y-major-mjr );
				pc->drawLine( x, y-major-mjr, x-minor-inc, y );
			}
			break;
			case QtMouseToolNames::SYM_SQUARE:
				pc->drawRectangle(x-4-inc,y-4-inc,x+4+inc,y+4+inc);
				break;
			case QtMouseToolNames::SYM_DOT:
			default:
				pc->drawFilledRectangle( x-1-inc, y-1-inc, x+1+inc, y+1+inc );
			}

			if ( memory::anullptr.check( creating_region ) ) {
				pushDrawingEnv(region::DotLine,1);
				if ( weaklySelected( ) || marked( ) ) {
					// Switching to a circle seemed good, but the rendering of a circle
					// with a dotted line is atrocious...
					pc->drawRectangle( x-radius, y-radius, x+radius, y+radius );
					if ( marked( ) ) {
						// draw outline rectangle for resizing the point...
						// While a circle would be a better choice, drawing a dotted circle
						// leaves terrible gaps in the circumference...  currently... <drs>
						// pc->drawEllipse(x, y, radius, radius, 0.0, True, 1.0, 1.0);
						pc->drawRectangle( x-radius+4, y-radius+4, x+radius-4, y+radius-4 );
					}
				}
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
			try {
				linear_to_screen( wc_, xd, yd, blc_x, blc_y, x, y, ptx, pty );
			} catch(...) {
				return false;
			}
			if ( x >  ptx - radius && x < ptx + radius  && y > pty - radius && y < pty + radius )
				return true;
			else
				return false;
		}

		// returns point state (Region::PointLocation)
		region::PointInfo Point::checkPoint( double xd, double yd ) const {
			unsigned int result = 0;
			int x, y, ptx, pty;

			try {
				linear_to_screen( wc_, xd, yd, blc_x, blc_y, x, y, ptx, pty );
			} catch(...) {
				return region::PointInfo(0,0,region::PointOutside);
			}

			if ( x >  ptx - radius && x < ptx + radius  && y > pty - radius && y < pty + radius )
				result |= region::PointInside;

			return region::PointInfo(xd,yd,result == 0 ? (unsigned int) region::PointOutside : result);
		}

		// returns mouse state (Region::MouseState)
		unsigned int Point::mouseMovement( double xd, double yd, bool other_selected ) {
			unsigned int result = 0;

			if ( visible_ == false ) return result;

			int x, y, ptx, pty;
			try {
				linear_to_screen( wc_, xd, yd, blc_x, blc_y, x, y, ptx, pty );
			} catch(...) {
				return 0;
			}

			if ( x >  ptx - radius && x < ptx + radius  && y > pty - radius && y < pty + radius ) {
				weaklySelect( mouse_in_region == false );
				mouse_in_region = true;
				result |= region::MouseSelected;
				result |= region::MouseRefresh;
				selected_ = true;
				draw( other_selected );
				if ( other_selected == false ) {
					// mark flag as this is the region (how to mix in other shapes)
					// of interest for statistics updates...
					selectedInCanvas( );
				}
			} else if ( selected_ == true ) {
				weaklyUnselect( );
				mouse_in_region = false;
				if ( selected_ == true ) {
					selected_ = false;
					draw( other_selected );
					result |= region::MouseRefresh;
				}
			}
			return result;
		}

		std::list<std::tr1::shared_ptr<RegionInfo> > * Point::generate_dds_centers( ) {
			// In principle there is no need to implement this,
			// it would go to Rectangle::generate_dds_centers() otherwise
			// and really try to fit a Gaussian to a point, certainly
			// without success. Implementing it here as an empty method
			// just accelerates matters (MK)
			return new std::list<std::tr1::shared_ptr<RegionInfo> >( );
		}
	}

}
