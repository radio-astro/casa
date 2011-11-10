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

	RegionToolManager::RegionToolManager( QtRegionSourceFactory *rsf, PanelDisplay *pd_ ) : pd(pd_) {
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

	void RegionToolManager::operator()(const WCPositionEvent& ev) {
	    for ( tool_map::iterator it = tools.begin( ); it != tools.end( ); ++it ) {
		if ( ev.key() != (*it).second->getKey() ) {
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
				for ( int i = 0; i < points.size( ); ++i ) {
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
    }
}

