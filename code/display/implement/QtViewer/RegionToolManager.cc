//# RegionToolManager.cc: class designed to unify the behavior of all of the mouse tools
//# Copyright (C) 2011
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
//# $Id$

#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/QtViewer/RegionToolManager.qo.h>
#include <display/QtPlotter/QtMWCTools.qo.h>
#include <display/QtViewer/QtMouseToolState.qo.h>

#include <imageanalysis/Annotations/RegionTextList.h>
#include <imageanalysis/Annotations/AnnEllipse.h>

namespace casa {
    namespace viewer {

	RegionToolManager::RegionToolManager( QtRegionSourceFactory *rsf, PanelDisplay *pd_ ) : pd(pd_), moving_handle(false),
							moving_handle_info(0,0,Region::PointOutside), moving_handle_region(0) {
	    // register for world canvas events...
	    pd->myWCLI->toStart( );
	    while ( ! pd->myWCLI->atEnd( ) ) {
		WorldCanvas* wc = pd->myWCLI->getRight( );
		wc->addPositionEventHandler(*this);
		wc->addMotionEventHandler(*this);
		wc->addRefreshEventHandler(*this);
		(*(pd->myWCLI))++;
	    }

	    // MultiWCTool *_this_is_it_ = new QtCrossTool(rsf,pd);
	    // QtMouseTool *_this_is_it_ = new QtCrossTool(rsf,pd);

	    //rtregion_  = new QtRectTool(region_source_factory,pd_);   pd_->addTool(RECTANGLE, rtregion_);
	    RegionTool *tool = 0;
	    tool = new QtCrossTool(rsf,pd);
	    tools.insert(tool_map::value_type(PointTool,tool));
	    pd->addTool(QtMouseToolNames::POSITION, tool);

	    tool = new QtPolyTool(rsf,pd);
	    tools.insert(tool_map::value_type(PolyTool,tool));
	    pd->addTool(QtMouseToolNames::POLYGON, tool);

	    tool = new QtRectTool(rsf,pd);
	    tools.insert(tool_map::value_type(RectTool,tool));
	    pd->addTool(QtMouseToolNames::RECTANGLE, tool);

	    tool = new QtEllipseTool(rsf,pd);
	    tools.insert(tool_map::value_type(EllipseTool,tool));
	    pd->addTool(QtMouseToolNames::ELLIPSE, tool);

	}

	bool RegionToolManager::add_mark_select( RegionTool::State &state ) {
	    if ( state.count( viewer::Region::PointInside ) > 0 ) {
		const region_list_type &new_marked_regions = state.regions(viewer::Region::PointInside);
		for ( region_list_type::iterator it=new_marked_regions.begin( );
		      it != new_marked_regions.end( ); ++it ) {
		    (*it)->mark_toggle( );
		    marked_regions.insert(*it);
		}
		state.refresh( );
		return true;
	    }
	    return false;
	}

	void RegionToolManager::clear_mark_select( RegionTool::State &state ) {
	    const region_list_type &all_regions = state.regions( );
	    // shift-click outside of all regions (but perhaps inside of a handle?)
	    for ( region_list_type::const_iterator it = marked_regions.begin( ); it != marked_regions.end( ); ++it ) {
		region_list_type::const_iterator contains = all_regions.find(*it);
		if ( contains != all_regions.end( ) ) (*it)->mark(false);
	    }
	    marked_regions.clear( );
	    return;
	}

	void RegionToolManager::setup_moving_regions_state( double linx, double liny ) {
	    // prevent sliding regions out of the viewing area...
	    bool first_trip = true;
	    for ( region_list_type::iterator it = moving_regions.begin( ); it != moving_regions.end( ); ++it ) {
		double blc_x, blc_y;
		double trc_x, trc_y;
		(*it)->boundingRectangle( blc_x, blc_y, trc_x, trc_y );
		if ( blc_x < moving_blc.first || first_trip ) moving_blc.first = blc_x;
		if ( blc_y < moving_blc.second || first_trip ) moving_blc.second = blc_y;
		if ( trc_x > moving_trc.first || first_trip ) moving_trc.first = trc_x;
		if ( trc_y > moving_trc.second || first_trip ) moving_trc.second = trc_y;
		first_trip = false;
	    }
	    moving_ref_point = linear_point_type(linx,liny);
	}

	bool RegionToolManager::setup_moving_regions( RegionTool::State &state ) {
	    region_list_type &point_inside = state.regions( viewer::Region::PointInside );
	    region_list_type intersection;
	    // intersection of marked regions and regions inclosing the current point...
	    std::set_intersection( marked_regions.begin( ), marked_regions.end( ),
				   point_inside.begin( ), point_inside.end( ),
				   std::insert_iterator<region_list_type>(intersection,intersection.begin( )) );

	    if ( intersection.size( ) > 0 ) {
		region_list_type new_marked_regions;
		region_list_type &all_regions = state.regions( );
		std::set_intersection( marked_regions.begin( ), marked_regions.end( ),
				       all_regions.begin( ), all_regions.end( ),
				       std::insert_iterator<region_list_type>(new_marked_regions,new_marked_regions.begin( )) );
		moving_regions.clear( );
		std::set_union( new_marked_regions.begin( ), new_marked_regions.end( ),
				point_inside.begin( ), point_inside.end( ),
				std::insert_iterator<region_list_type>(moving_regions,moving_regions.begin( )) );

		setup_moving_regions_state( state.x( ), state.y( ) );
		return true;
	    } else if ( point_inside.size( ) > 0 ) {
		moving_regions = point_inside;
		setup_moving_regions_state( state.x( ), state.y( ) );
		return true;
	    }
	    return false;
	}

	void RegionToolManager::translate_moving_regions( WorldCanvas *wc, double dx, double dy ) {

	    linear_point_type new_blc(moving_blc);
	    linear_point_type new_trc(moving_trc);

	    new_blc.first += dx;
	    new_blc.second += dy;
	    new_trc.first += dx;
	    new_trc.second += dy;

	    if ( ! inDrawArea(wc, new_blc,new_trc) ) return;

	    moving_blc = new_blc;
	    moving_trc = new_trc;

	    // we do not verify moving_regions (against the regions returned by
	    // Region::checkPoint(...) because the moving_regions list is created
	    // when the user clicks and cleared when the click is released (so
	    // there is no opportunity for the user to delete regions (thus
	    // invalidating the pointers in moving_regions) in between)
	    for ( region_list_type::iterator it = moving_regions.begin( );
		  it != moving_regions.end( ); ++it ) {
		(*it)->move(dx,dy);
	    }
	    moving_ref_point.first += dx;
	    moving_ref_point.second += dy;
	}

	void RegionToolManager::operator()(const WCPositionEvent& ev) {

	    Int x = ev.pixX( );
	    Int y = ev.pixY( );
	    WorldCanvas *wc = ev.worldCanvas( );

	    if ( ! wc->inDrawArea(x,y) ) return;

	    double linx, liny;
	    viewer::screen_to_linear( wc, x, y, linx, liny );

	    // checkPixel( ): inside, outside, handle
	    RegionTool::State state(wc,linx,liny);
	    for ( tool_map::iterator it = tools.begin( ); it != tools.end( ); ++it ) {
		(*it).second->checkPoint( wc, state );
	    }

	    if ( ev.keystate() ) {
		if ( ev.key( ) == Display::K_Pointer_Button1 ) {

		    if ( ev.modifiers( ) & Display::KM_Shift ) {
			// shift-click within a region
			if ( add_mark_select( state ) ) return;
			else return clear_mark_select( state );

		    } else {
			region_list_type &handles = state.regions( viewer::Region::PointHandle );
			if ( handles.size( ) > 0 ) {
			    moving_handle = true;
			    moving_handle_info = state.state(*handles.begin());
			    moving_handle_region = *handles.begin();
			    return;
			} else {

			    if ( setup_moving_regions( state ) ) {
				return;
			    } else {
				// click outside of all regions...
				if ( marked_regions.size( ) > 0 ) {
				    clear_mark_select(state);
				    return;
				}	// go on to (perhaps) create a new region if no regions are marked...
			    }
			}
		    }
		} else if ( ev.key( ) == Display::K_Escape ) {
		    if ( marked_regions.size( ) > 0 ) {
			// escape clears marked regions...
			clear_mark_select(state);
			return;
		    } else if ( moving_regions.size( ) > 0 ) {
			// escape while moving regions has no effect...
			return;
		    }
		} else if ( ev.key() == Display::K_Left ||
			    ev.key() == Display::K_Right ||
			    ev.key() == Display::K_Up ||
			    ev.key() == Display::K_Down ) {
		    if ( setup_moving_regions( state) ) {
			const int pixel_step = 1;

			double dx=0, dy=0;
			switch ( ev.key( ) ) {
			    case Display::K_Left:
				screen_offset_to_linear_offset( wc, -pixel_step, 0, dx, dy );
				break;
			    case Display::K_Right:
				screen_offset_to_linear_offset( wc, pixel_step, 0, dx, dy );
				break;
			    case Display::K_Down:
				screen_offset_to_linear_offset( wc, 0, -pixel_step, dx, dy );
				break;
			    case Display::K_Up:
				screen_offset_to_linear_offset( wc, 0, pixel_step, dx, dy );
				break;
			    default:
				break;
			}
			
			translate_moving_regions( wc, dx, dy );
			wc->refresh( );
			moving_regions.clear( );
			moving_handle = false;
			return;
		    }
		}
	    } else {
		// key-release clear list of regions being moved...
		moving_regions.clear( );
		moving_handle = false;
	    }

	    for ( tool_map::iterator it = tools.begin( ); it != tools.end( ); ++it ) {
		if ( ev.key() != (*it).second->getKey() ||
		     (*it).second->getKey( ) == Display::K_None ) {
		    if (ev.keystate()) {
			(*it).second->otherKeyPressed(ev);
		    } else {
			(*it).second->otherKeyReleased(ev);
		    }
		} else {
		    if (ev.keystate()) {
			(*it).second->keyPressed(ev);
		    } else {
			(*it).second->keyReleased(ev);
		    }
		}
	    }
	}

	void RegionToolManager::operator()(const WCMotionEvent& ev) {

	    Int x = ev.pixX( );
	    Int y = ev.pixY( );
	    WorldCanvas *wc = ev.worldCanvas( );

	    if ( ! wc->inDrawArea(x,y) ) return;

	    if ( moving_handle ) {

		if ( ! wc->inDrawArea(x,y) ) return;

		double linx, liny;
		viewer::screen_to_linear( wc, x, y, linx, liny );

		moving_handle_info.handle( ) = moving_handle_region->moveHandle( moving_handle_info.handle( ), linx, liny );
		moving_handle_info.x( ) = linx;
		moving_handle_info.y( ) = liny;

		wc->refresh( );
		return;

	    } else if ( moving_regions.size( ) > 0 ) {

		double linx, liny;
		viewer::screen_to_linear( wc, x, y, linx, liny );

		double dx = linx - moving_ref_point.first;
		double dy = liny - moving_ref_point.second;

		translate_moving_regions( wc, dx, dy );
		wc->refresh( );
		return;
	    }

	    for ( tool_map::iterator it = tools.begin( ); it != tools.end( ); ++it )
		(*it).second->moved(ev);
	}

	void RegionToolManager::operator()(const WCRefreshEvent& ev) {
	    if ( /*itsCurrentWC != 0 && ev.worldCanvas() == itsCurrentWC &&*/
		 ev.reason() == Display::BackCopiedToFront &&
		 ev.worldCanvas( )->pixelCanvas()->drawBuffer()==Display::FrontBuffer  ) {
		for ( tool_map::iterator it = tools.begin( ); it != tools.end( ); ++it )
		    (*it).second->draw(ev);
	    }
	}

	void RegionToolManager::loadRegions( const std::string &path, const std::string &datatype, const std::string &displaytype ) {

	    bool first_trip = true;
	    ListIter<WorldCanvas* >* wcs = pd->wcs();
	    for ( wcs->toStart(); ! wcs->atEnd(); wcs->step( ) ) {
		WorldCanvas *wc = wcs->getRight();
		if ( wc == 0 ) continue;
		const CoordinateSystem &test = wc->coordinateSystem();
		const Vector<String> &units = wc->worldAxisUnits( );
		IPosition shape(2);
		shape[0] = wc->canvasXSize( );
		shape[1] = wc->canvasYSize( );
		RegionTextList rlist( path, test, shape );
		Vector<AsciiAnnotationFileLine> aaregions = rlist.getLines( );
		for ( unsigned int i=0; i < aaregions.size( ); ++i ) {
		    if ( aaregions[i].getType( ) != AsciiAnnotationFileLine::ANNOTATION ) continue;
		    const AnnotationBase* ann = aaregions[i].getAnnotationBase();
		    AnnotationBase::Direction points = ann->getDirections( );
		    switch ( ann->getType( ) ) {
			case AnnotationBase::SYMBOL:
			    {
				if ( points.size( ) != 1 ) {
				    fprintf( stderr, "QtDisplayPanel::loadRegions(symbol): wrong number of points returned...\n" );
				    continue;
				}

				double lcx, lcy;
				viewer::world_to_linear( wc, points[0].first.getValue(units[0]), points[0].second.getValue(units[1]), lcx, lcy );
				int px, py;
				viewer::linear_to_pixel( wc, lcx, lcy, px, py );

				// region is outside of our pixel canvas area
				if ( ! wc->inPC(px,py) ) continue;

				std::vector<std::pair<double,double> > linear_pts(2);
				linear_pts[0].first = lcx;
				linear_pts[0].second = lcy;
				linear_pts[1].first = lcx;
				linear_pts[1].second = lcy;
				AnnotationBase::LineStyle ls = ann->getLineStyle( );
				AnnotationBase::FontStyle fs = ann->getFontStyle( );
				tool_map::iterator ptit = tools.find(PointTool);
				if ( ptit == tools.end( ) ) continue;
				(*ptit).second->create( wc, linear_pts, ann->getLabel( ), ann->getFont( ), ann->getFontSize( ),
						    (fs == AnnotationBase::BOLD ? viewer::Region::BoldText : 0) |
						    (fs == AnnotationBase::ITALIC ? viewer::Region::ItalicText : 0) |
						    (fs == AnnotationBase::ITALIC_BOLD ? (viewer::Region::BoldText | viewer::Region::ItalicText) : 0 ),
						    ann->getLabelColorString( ), ann->getColorString( ),
						    ( ls == AnnotationBase::DASHED ? viewer::Region::DashLine :
						      ls == AnnotationBase::DOTTED ? viewer::Region::DotLine : viewer::Region::SolidLine ) );

			    }
			    break;
			case AnnotationBase::RECT_BOX:
			    {
				if ( points.size( ) != 2 ) {
				    fprintf( stderr, "QtDisplayPanel::loadRegions(rect_box): wrong number of points returned...\n" );
				    continue;
				}

				double lblcx, lblcy, ltrcx, ltrcy;
				viewer::world_to_linear( wc, points[0].first.getValue(units[0]), points[0].second.getValue(units[1]),
							 points[1].first.getValue(units[0]), points[1].second.getValue(units[1]),
							 lblcx, lblcy, ltrcx, ltrcy );
				int pblcx, pblcy, ptrcx, ptrcy;
				viewer::linear_to_pixel( wc, lblcx, lblcy, ltrcx, ltrcy, pblcx, pblcy, ptrcx, ptrcy );

				// region is outside of our pixel canvas area
				if ( ! wc->inPC(pblcx,pblcy) || ! wc->inPC(ptrcx,ptrcy) ) continue;

				std::vector<std::pair<double,double> > linear_pts(2);
				linear_pts[0].first = lblcx;
				linear_pts[0].second = lblcy;
				linear_pts[1].first = ltrcx;
				linear_pts[1].second = ltrcy;
				AnnotationBase::LineStyle ls = ann->getLineStyle( );
				AnnotationBase::FontStyle fs = ann->getFontStyle( );
				tool_map::iterator rtit = tools.find(RectTool);
				if ( rtit == tools.end( ) ) continue;
				(*rtit).second->create( wc, linear_pts, ann->getLabel( ), ann->getFont( ), ann->getFontSize( ),
						   (fs == AnnotationBase::BOLD ? viewer::Region::BoldText : 0) |
						   (fs == AnnotationBase::ITALIC ? viewer::Region::ItalicText : 0) |
						   (fs == AnnotationBase::ITALIC_BOLD ? (viewer::Region::BoldText | viewer::Region::ItalicText) : 0 ),
						   ann->getLabelColorString( ), ann->getColorString( ),
						   ( ls == AnnotationBase::DASHED ? viewer::Region::DashLine :
						     ls == AnnotationBase::DOTTED ? viewer::Region::DotLine : viewer::Region::SolidLine ) );
			    }

			    break;
			case AnnotationBase::ELLIPSE:
			    {
				if ( points.size( ) != 1 ) {
				    fprintf( stderr, "QtDisplayPanel::loadRegions(ellipse): wrong number of points returned...\n" );
				    continue;
				}

				const AnnEllipse *el = dynamic_cast<const AnnEllipse*>(ann);

				double pos_angle = el->getPositionAngle( ).getValue("deg");

				while ( pos_angle < 0 ) pos_angle += 360;
				while ( pos_angle >= 360 ) pos_angle -= 360;

				// 90 deg around 0 & 180 deg
				bool x_is_major = ((pos_angle > 45.0 && pos_angle < 135.0) ||
						   (pos_angle > 225.0 && pos_angle < 315.0));

				Quantity qblcx, qblcy, qtrcx, qtrcy;
				Quantity major_inc = el->getMajorAxis( ) / 2.0;
				Quantity minor_inc = el->getMinorAxis( ) / 2.0;
				Quantity centerx = points[0].first;
				Quantity centery = points[0].second;
				if ( x_is_major ) {
				    qblcx = centerx - major_inc;
				    qblcy = centery - minor_inc;
				    qtrcx = centerx + major_inc;
				    qtrcy = centery + minor_inc;
				} else { 
				    qblcx = centerx - minor_inc;
				    qblcy = centery - major_inc;
				    qtrcx = centerx + minor_inc;
				    qtrcy = centery + major_inc;
				}

				double lblcx, lblcy, ltrcx, ltrcy;
				viewer::world_to_linear( wc, qblcx.getValue(units[0]), qblcy.getValue(units[1]),
							 qtrcx.getValue(units[0]), qtrcy.getValue(units[1]),
							 lblcx, lblcy, ltrcx, ltrcy );
				int pblcx, pblcy, ptrcx, ptrcy;
				viewer::linear_to_pixel( wc, lblcx, lblcy, ltrcx, ltrcy, pblcx, pblcy, ptrcx, ptrcy );

				// region is outside of our pixel canvas area
				if ( ! wc->inPC(pblcx,pblcy) || ! wc->inPC(ptrcx,ptrcy) ) continue;

				std::vector<std::pair<double,double> > linear_pts(2);
				linear_pts[0].first = lblcx;
				linear_pts[0].second = lblcy;
				linear_pts[1].first = ltrcx;
				linear_pts[1].second = ltrcy;
				AnnotationBase::LineStyle ls = ann->getLineStyle( );
				AnnotationBase::FontStyle fs = ann->getFontStyle( );
				tool_map::iterator elit = tools.find(EllipseTool);
				if ( elit == tools.end( ) ) continue;
				(*elit).second->create( wc, linear_pts, ann->getLabel( ), ann->getFont( ), ann->getFontSize( ),
						   (fs == AnnotationBase::BOLD ? viewer::Region::BoldText : 0) |
						   (fs == AnnotationBase::ITALIC ? viewer::Region::ItalicText : 0) |
						   (fs == AnnotationBase::ITALIC_BOLD ? (viewer::Region::BoldText | viewer::Region::ItalicText) : 0 ),
						   ann->getLabelColorString( ), ann->getColorString( ),
						   ( ls == AnnotationBase::DASHED ? viewer::Region::DashLine :
						     ls == AnnotationBase::DOTTED ? viewer::Region::DotLine : viewer::Region::SolidLine ) );
			    }
			    break;
			case AnnotationBase::POLYGON:
			    {
				if ( points.size( ) <= 2 ) {
				    fprintf( stderr, "QtDisplayPanel::loadRegions(polygon): wrong number of points returned...\n" );
				    continue;
				}

				std::vector<std::pair<double,double> > linear_pts(points.size( ));

				bool error = false;
				for ( unsigned int i = 0; i < points.size( ); ++i ) {
				    double lx, ly;
				    viewer::world_to_linear( wc, points[i].first.getValue(units[0]), points[i].second.getValue(units[1]), lx, ly );
				    int px, py;
				    viewer::linear_to_pixel( wc, lx, ly, px, py );

				    // region is outside of our pixel canvas area
				    if ( ! wc->inPC(px,py) ) {
					error = false;
					break;
				    }

				    linear_pts[i].first = lx;
				    linear_pts[i].second = ly;
				}

				AnnotationBase::LineStyle ls = ann->getLineStyle( );
				AnnotationBase::FontStyle fs = ann->getFontStyle( );

				tool_map::iterator plyit = tools.find(PolyTool);
				if ( plyit == tools.end( ) ) continue;
				(*plyit).second->create( wc, linear_pts, ann->getLabel( ), ann->getFont( ), ann->getFontSize( ),
						   (fs == AnnotationBase::BOLD ? viewer::Region::BoldText : 0) |
						   (fs == AnnotationBase::ITALIC ? viewer::Region::ItalicText : 0) |
						   (fs == AnnotationBase::ITALIC_BOLD ? (viewer::Region::BoldText | viewer::Region::ItalicText) : 0 ),
						   ann->getLabelColorString( ), ann->getColorString( ),
						   ( ls == AnnotationBase::DASHED ? viewer::Region::DashLine :
						     ls == AnnotationBase::DOTTED ? viewer::Region::DotLine : viewer::Region::SolidLine ) );
			    }
			    break;

			case AnnotationBase::CIRCLE:
			    if ( first_trip ) fprintf( stderr, "QtDisplayPanel::loadRegions(): unsupported region type (circle) encountered...\n" );
			    break;
			case AnnotationBase::CENTER_BOX:
			    if ( first_trip ) fprintf( stderr, "QtDisplayPanel::loadRegions(): unsupported region type (center box) encountered...\n" );
			    break;
			case AnnotationBase::LINE:
			    if ( first_trip ) fprintf( stderr, "QtDisplayPanel::loadRegions(): unsupported region type (line) encountered...\n" );
			    break;
			case AnnotationBase::VECTOR:
			    if ( first_trip ) fprintf( stderr, "QtDisplayPanel::loadRegions(): unsupported region type (vector) encountered...\n" );
			    break;
			case AnnotationBase::TEXT:
			    if ( first_trip ) fprintf( stderr, "QtDisplayPanel::loadRegions(): unsupported region type (text) encountered...\n" );
			    break;
			case AnnotationBase::ROTATED_BOX:
			    if ( first_trip ) fprintf( stderr, "QtDisplayPanel::loadRegions(): unsupported region type (rotated box) encountered...\n" );
			    break;
			case AnnotationBase::ANNULUS:
			    if ( first_trip ) fprintf( stderr, "QtDisplayPanel::loadRegions(): unsupported region type (annulus) encountered...\n" );
			    break;
			default:
			    if ( first_trip ) fprintf( stderr, "QtDisplayPanel::loadRegions(): unsupported region (of unknown type) encountered...\n" );
		    }
		}
		first_trip = false;
	    }
	}

	bool RegionToolManager::inDrawArea( WorldCanvas *wc, const linear_point_type &new_blc, const linear_point_type &new_trc ) const {
	    return new_blc.first >= wc->linXMin( ) && new_blc.second >= wc->linYMin( ) &&
		   new_trc.first <= wc->linXMax( ) && new_trc.second <= wc->linYMax( );
	}
    }
}

