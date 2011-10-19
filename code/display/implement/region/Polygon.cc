//# polygon.h: base class for statistical regions
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


#include <display/region/Polygon.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>

#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <images/Regions/WCPolygon.h>

#include <imageanalysis/Annotations/AnnPolygon.h>
#include <coordinates/Coordinates/CoordinateUtil.h>

namespace casa {
    namespace viewer {

	Polygon::Polygon( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts) :
	  Region( wc ), closed(false), _ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
	  _drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
	    for ( int i=0; i < pts.size(); ++i ) {
		_ref_points_.push_back(pt(pts[i].first,pts[i].second));
		_drawing_points_.push_back(pt(pts[i].first,pts[i].second));
	    }
	    closeFigure( );
	}

	void Polygon::closeFigure( ) {
	    closed = true;
	    unsigned int size = _ref_points_.size( );
	    if ( size > 1 && _ref_points_[size-1].first == _ref_points_[size-2].first &&
		 _ref_points_[size-1].second == _ref_points_[size-2].second ) {
		_ref_points_.pop_back( );
		_drawing_points_.pop_back( );
	    }
	    update_reference_bounds_rectangle( );
	    update_drawing_bounds_rectangle( );
	}

	void Polygon::addVertex( double x, double y, bool rewrite_last_point ) {
	    if ( rewrite_last_point == false ) {
		_ref_points_.push_back(pt(x,y));
		_drawing_points_.push_back(pt(x,y));
	    } else {
		if ( _ref_points_.size( ) < 1 || _drawing_points_.size( ) < 1 )
		    throw internal_error("polygon inconsistency");
		_ref_points_[_ref_points_.size( ) - 1] = pt(x,y);
		_drawing_points_[_drawing_points_.size( ) - 1] = pt(x,y);
	    }
	    update_drawing_bounds_rectangle( );
	}

	void Polygon::move( double dx, double dy ) {

	    if ( _drawing_points_.size( ) == 0 ) return;

	    _drawing_points_[0].first += dx;
	    _drawing_points_[0].second += dy;
	    _drawing_blc_x_ = _drawing_trc_x_ = _drawing_points_[0].first;
	    _drawing_blc_y_ = _drawing_trc_y_ = _drawing_points_[0].second;

	    for ( unsigned int i=1; i < _drawing_points_.size( ); ++i ) {
		_drawing_points_[i].first += dx;
		_drawing_points_[i].second += dy;

		// moving the bounding box will be required for redrawing,
		// so update as we move the points...
		if ( _drawing_points_[i].first < _drawing_blc_x_ )
		    _drawing_blc_x_ = _drawing_points_[i].first;
		if ( _drawing_points_[i].first > _drawing_trc_x_ )
		    _drawing_trc_x_ = _drawing_points_[i].first;

		if ( _drawing_points_[i].second < _drawing_blc_y_ )
		    _drawing_blc_y_ = _drawing_points_[i].second;
		if ( _drawing_points_[i].second > _drawing_trc_y_ )
		    _drawing_trc_y_ = _drawing_points_[i].second;
	    }

	    updateStateInfo( true ); 
	}

	bool Polygon::within_vertex_handle( double x, double y ) const {
	    double half_handle_delta_x = handle_delta_x / 2.0;
	    double half_handle_delta_y = handle_delta_y / 2.0;
	    for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
		if ( x >= (_drawing_points_[i].first - half_handle_delta_x) &&
		     x <= (_drawing_points_[i].first + half_handle_delta_x) &&
		     y >= (_drawing_points_[i].second - half_handle_delta_y) &&
		     y <= (_drawing_points_[i].second + half_handle_delta_y) )
		    return true;
	    }
	    return false;
	}

        // returns mouse movement state
	int Polygon::mouseMovement( double x, double y, bool other_selected ) {
	    int result = 0;

	    if ( visible_ == false ) return result;

	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );
	    if ( x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y || within_vertex_handle( x, y ) ) {
		result |= MouseSelected;
		result |= MouseRefresh;
		selected_ = true;
		draw( );		// this draw may not be necessary (?)...
		if ( other_selected == false ) {
		    // mark flag as this is the region (how to mix in other shapes)
		    // of interest for statistics updates...
		    selectedInCanvas( );
		}
	    } else if ( selected_ == true ) {
		selected_ = false;
		draw( );		// this draw may not be necessary (?)...
		result |= MouseRefresh;
	    }
	    return result;
	}

	bool Polygon::clickWithin( double x, double y ) const {
	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );
	    return x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y;
	}


	int Polygon::clickHandle( double x, double y ) const {

	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );
	    if ( visible_ == false ) return 0;
	    bool blc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= blc_y && y <= (blc_y + handle_delta_y);
	    bool tlc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= (trc_y - handle_delta_y) && y <= trc_y;
	    bool brc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= blc_y && y <= (blc_y + handle_delta_y);
	    bool trc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= (trc_y - handle_delta_y) && y <= trc_y;

	    if ( blc ) {
		((Polygon*)this)->_x_origin_ = ScaleRight;
		((Polygon*)this)->_y_origin_ = ScaleTop;
	    } else if ( tlc ) {
		((Polygon*)this)->_x_origin_ = ScaleRight;
		((Polygon*)this)->_y_origin_ = ScaleBottom;
	    } else if ( brc ) {
		((Polygon*)this)->_x_origin_ = ScaleLeft;
		((Polygon*)this)->_y_origin_ = ScaleTop;
	    } else if ( trc ) {
		((Polygon*)this)->_x_origin_ = ScaleLeft;
		((Polygon*)this)->_y_origin_ = ScaleBottom;
	    }

	    if ( blc || tlc || brc || trc )
		return trc ? 1 : brc ? 2 : blc ? 3 : tlc ? 4 : 0;

	    double half_handle_delta_x = handle_delta_x / 2.0;
	    double half_handle_delta_y = handle_delta_y / 2.0;
	    for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
		if ( x >= (_drawing_points_[i].first - half_handle_delta_x) &&
		     x <= (_drawing_points_[i].first + half_handle_delta_x) &&
		     y >= (_drawing_points_[i].second - half_handle_delta_y) &&
		     y <= (_drawing_points_[i].second + half_handle_delta_y) )
		    return i + 5;
	    }
	    return 0;
	}

	int Polygon::moveHandle( int handle, double x, double y ) {

	    if ( handle >= 1 && handle <= 4 )
		handle = move_sizing_rectangle_handle( handle, x, y );
	    else if ( handle >= 5 )
		handle = move_vertex( handle, x, y );
	    else
		return 0;

	    updateStateInfo( true ); 
	    return handle;
	}

	int Polygon::move_vertex( int handle, double x, double y ) {
	    int vertex = handle - 5;
	    if ( vertex >= _ref_points_.size( ) || vertex < 0 )
		return 0;

	    // use current x & y scaling to translate drawing vertex
	    // back to reference vertex...
	    double x_scale = _ref_width_ / _drawing_width_;
	    double y_scale = _ref_height_ / _drawing_height_;

	    // moving a vertex can change drawing bounding rectangle...
	    // calculate any adjustment...
	    double new_blc_x = x, new_blc_y = y, new_trc_x = x, new_trc_y = y;
	    for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
		if ( i != vertex ) {
		    if ( _drawing_points_[i].first < new_blc_x ) new_blc_x = _drawing_points_[i].first;
		    if ( _drawing_points_[i].second < new_blc_y ) new_blc_y = _drawing_points_[i].second;
		    if ( _drawing_points_[i].first > new_trc_x ) new_trc_x = _drawing_points_[i].first;
		    if ( _drawing_points_[i].second > new_trc_y ) new_trc_y = _drawing_points_[i].second;
		}
	    }

	    // reflect any drawing bounding rectangle back to reference points...
	    if ( new_blc_x != _drawing_blc_x_ ) {
		_ref_blc_x_ = _ref_blc_x_ + (new_blc_x - _drawing_blc_x_) * x_scale;
		_drawing_blc_x_ = new_blc_x;
	    }
	    if ( new_blc_y != _drawing_blc_y_ ) {
		_ref_blc_y_ = _ref_blc_y_ + (new_blc_y - _drawing_blc_y_) * y_scale;
		_drawing_blc_y_ = new_blc_y;
	    }
	    if ( new_trc_x != _drawing_trc_x_ ) {
		_ref_trc_x_ = _ref_trc_x_ + (new_trc_x - _drawing_trc_x_) * x_scale;
		_drawing_trc_x_ = new_trc_x;
	    }
	    if ( new_trc_y != _drawing_trc_y_ ) {
		_ref_trc_y_ = _ref_trc_y_ + (new_trc_y - _drawing_trc_y_) * y_scale;
		_drawing_trc_y_ = new_trc_y;
	    }

	    // update drawing rectangle for new scaling values...
	    _drawing_blc_x_ = new_blc_x;
	    _drawing_blc_y_ = new_blc_y;
	    _drawing_trc_x_ = new_trc_x;
	    _drawing_trc_y_ = new_trc_y;

	    // use new scaling values to translate moved (drawing) vertex
	    // back to reference vertex...
	    x_scale = (_ref_trc_x_ - _ref_blc_x_) / (_drawing_trc_x_ - _drawing_blc_x_);
	    y_scale = (_ref_trc_y_ - _ref_blc_y_) / (_drawing_trc_y_ - _drawing_blc_y_);

	    // calculate offset between drawn image and reference image...
	    double x_offset = _ref_blc_x_ - _drawing_blc_x_;
	    double y_offset = _ref_blc_y_ - _drawing_blc_y_;

	    // translate modified drawing vertex back to a reference vertex...
	    _ref_points_[vertex] = pt( (x - _drawing_blc_x_) * x_scale + _drawing_blc_x_ + x_offset,
				       (y - _drawing_blc_y_) * y_scale + _drawing_blc_y_ + y_offset );

	    // update reference state...
	    update_reference_bounds_rectangle( );

	    // recalculate drawing vertices based upon updated reference vertices...
	    update_drawing_state( );
	    update_drawing_bounds_rectangle( );

	    return handle;
	}

	int Polygon::move_sizing_rectangle_handle( int handle, double x, double y ) {
	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );

	    int translation = 0;
	    int new_handle = handle;

	    switch ( handle ) {
	      case 1:				// trc handle becomes:
		if ( x < blc_x ) {
		    if ( y < blc_y ) {
			trc_x = blc_x;
			trc_y = blc_y;
			blc_x = x;
			blc_y = y;		// blc...
			translation = FLIP_X | FLIP_Y;
			new_handle = 3;		// flip-x, flip-y
		    } else {
			trc_x = blc_x;
			trc_y = y;
			blc_x = x;		// tlc...
			translation = FLIP_X;
			new_handle = 4;		// flip-x (x values change)
		    }
		} else if ( y < blc_y ) {
		    trc_y = blc_y;
		    blc_y = y;			// brc...
		    translation = FLIP_Y;
		    new_handle = 2;		// flip-y (y values change)
		} else {
		    trc_x = x;
		    trc_y = y;
		}
		break;
	      case 2:				// brc handle becomes:
		if ( x < blc_x ) {
		    if ( y > trc_y ) {
			blc_y = trc_y;
			trc_x = blc_x;
			trc_y = y;
			blc_x = x;		// tlc...
			translation = FLIP_X | FLIP_Y;
			new_handle = 4;		// flip-x, flip-y
		    } else {
			trc_x = blc_x;
			blc_x = x;
			blc_y = y;		// blc...
			translation = FLIP_X;
			new_handle = 3;		// flip-x
		    }
		} else if ( y > trc_y ) {
		    blc_y = trc_y;
		    trc_x = x;
		    trc_y = y;			// trc...
		    translation = FLIP_Y;
		    new_handle = 1;		// flip-y
		} else {
		    trc_x = x;
		    blc_y = y;
		}
		break;
	      case 3:				// blc handle becomes:
		if ( x > trc_x ) {
		    if ( y > trc_y ) {
			blc_x = trc_x;
			blc_y = trc_y;
			trc_x = x;
			trc_y = y;		// trc...
			translation = FLIP_X | FLIP_Y;
			new_handle = 1;		// flip-x, flip-y
		    } else {
			blc_x = trc_x;
			trc_x = x;
			blc_y = y;		// brc...
			translation = FLIP_X;
			new_handle = 2;		// flip-x
		    }
		} else if ( y > trc_y ) {
		    blc_y = trc_y;
		    blc_x = x;
		    trc_y = y;			// tlc...
		    translation = FLIP_Y;
		    new_handle = 4;		// flip-y
		} else {
		    blc_x = x;
		    blc_y = y;
		}
		break;
	      case 4:				// tlc handle becomes:
		if ( x > trc_x ) {
		    if ( y < blc_y ) {
			blc_x = trc_x;
			trc_y = blc_y;
			blc_y = y;
			trc_x = x;		// brc...
			translation = FLIP_X | FLIP_Y;
			new_handle = 2;		// flip-x, flip-y
		    } else {
			blc_x = trc_x;
			trc_x = x;
			trc_y = y;		// trc...
			translation = FLIP_X;
			new_handle = 1;		// flip-x
		    }
		} else if ( y < blc_y ) {
		    trc_y = blc_y;
		    blc_x = x;
		    blc_y = y;			// blc...
		    translation = FLIP_Y;
		    new_handle = 3;		// flip-y
		} else {
		    blc_x = x;
		    trc_y = y;
		}
		break;
	    }

	    if ( blc_x > trc_x || blc_y > trc_y ) throw internal_error("rectangle inconsistency");

	    update_reference_state(  translation, handle, new_handle );

	    // set the drawing bounds to the new bounds...
	    // update scaling
	    _drawing_blc_x_ = blc_x;
	    _drawing_blc_y_ = blc_y;
	    _drawing_trc_x_ = trc_x;
	    _drawing_trc_y_ = trc_y;

	    update_drawing_state( );

	    return new_handle;
	}

	void Polygon::regionCenter( double &x, double &y ) const {
	    double blc_x, blc_y, trc_x, trc_y;
	    boundingRectangle( blc_x, blc_y, trc_x, trc_y );

	    x = linear_average(blc_x,trc_x);
	    y = linear_average(blc_y,trc_y);
	}

	void Polygon::drawRegion( bool selected ) {
	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

	    PixelCanvas *pc = wc_->pixelCanvas();
	    if(pc==0) return;

	    if ( _drawing_points_.size( ) == 0 ) return;

	    int x1, y1, x2, y2;
	    linear_to_screen( wc_, _drawing_points_[0].first, _drawing_points_[0].second, x1, y1 );
	      
	    int first_x = x1, first_y = y1;
	    for ( unsigned int i=1; i < _drawing_points_.size( ); ++i ) {
		linear_to_screen( wc_, _drawing_points_[i].first, _drawing_points_[i].second, x2, y2 );
		pc->drawLine(x1,y1,x2,y2);
		x1 = x2;
		y1 = y2;
	    }
	    if ( closed ) pc->drawLine( x1, y1, first_x, first_y );


	    if ( selected ) {

		// get bounding rectangle...
		double blc_x, blc_y, trc_x, trc_y;
		boundingRectangle( blc_x, blc_y, trc_x, trc_y );
		linear_to_screen( wc_, blc_x, blc_y, trc_x, trc_y, x1, y1, x2, y2 );

		// compute handle size...
		Int w = x2 - x1;
		Int h = y2 - y1;

		Int s = 0;
		if (w>=35 && h>=35) s = 6;
		else if (w>=20 && h>=20) s = 4;
		else if (w>= 9 && h>= 9) s = 3;

		// get handle size in linear coordinates...
		double xdx, ydy;
		screen_to_linear( wc_, x1 + s, y1 + s, xdx, ydy );
		handle_delta_x = xdx - blc_x;
		handle_delta_y = ydy - blc_y;

		// draw outline rectangle for resizing whole polygon...
		pushDrawingEnv(DotLine);
		pc->drawRectangle( x1, y1, x2, y2 );
		popDrawingEnv( );

		// get resizing rectangle handle bounding values...
		int hx0 = x1;
		int hx1 = x1 + s;
		int hx2 = x2 - s;
		int hx3 = x2;
		int hy0 = y1;
		int hy1 = y1 + s;
		int hy2 = y2 - s;
		int hy3 = y2;	// set handle coordinates

		if (s) {
		    // draw handles of outline rectangle for resizing whole polygon...
		    pushDrawingEnv( Region::SolidLine);
		    pc->drawFilledRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
		    pc->drawFilledRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
		    pc->drawFilledRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
		    pc->drawFilledRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);

		    for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
			int h_blc_x, h_blc_y, h_trc_x, h_trc_y;
			linear_to_screen( wc_, _drawing_points_[i].first - handle_delta_x / 2.0, _drawing_points_[i].second - handle_delta_y / 2.0,
					       _drawing_points_[i].first + handle_delta_x / 2.0, _drawing_points_[i].second + handle_delta_y / 2.0,
					       h_blc_x, h_blc_y, h_trc_x, h_trc_y );

			pc->drawFilledRectangle( h_blc_x, h_blc_y, h_trc_x, h_trc_y );
		    }
		    popDrawingEnv( );
		}
	    }
	}

	AnnotationBase *Polygon::annotation( ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

	    const CoordinateSystem &cs = wc_->coordinateSystem( );
	    const Vector<String> &units = wc_->worldAxisUnits( );

	    Vector<Quantity> xv(_drawing_points_.size( ));
	    Vector<Quantity> yv(_drawing_points_.size( ));
	    for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
		double x,y;
		linear_to_world( wc_, _drawing_points_[i].first, _drawing_points_[i].second, x, y );
		xv[i] = Quantity(x,units[0]);
		yv[i] = Quantity(y,units[1]);
	    }

	    Vector<Stokes::StokesTypes> stokes;
	    /*Int polaxis =*/ CoordinateUtil::findStokesAxis(stokes, cs);
	    const DisplayData *dd = wc_->displaylist().front();

	    AnnPolygon *poly = 0;
	    try {
		std::vector<int> axes = dd->displayAxes( );
		IPosition shape(cs.nPixelAxes( ));
		for ( int i=0; i < shape.size( ); ++i )
		    shape(i) = dd->dataShape( )[axes[i]];
		poly = new AnnPolygon( xv, yv, cs, shape, stokes );
	    } catch ( AipsError &e ) {
		cerr << "Error encountered creating an AnnPolygon:" << endl;
		cerr << "\t\"" << e.getMesg( ) << "\"" << endl;
	    } catch ( ... ) {
		cerr << "Error encountered creating an AnnPolygon..." << endl;
	    }

	    return poly;
	}

	// return the *drawing* bounding rectangle...
	void Polygon::boundingRectangle( double &blcx, double &blcy, double &trcx, double &trcy ) const {
	    blcx = _drawing_blc_x_;
	    blcy = _drawing_blc_y_;
	    trcx = _drawing_trc_x_;
	    trcy = _drawing_trc_y_;
	}

	void Polygon::fetch_region_details( RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts, 
					    std::vector<std::pair<double,double> > &world_pts ) const {

	    if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

	    type = PolyRegion;

	    pixel_pts.resize(_drawing_points_.size( ));
	    world_pts.resize(_drawing_points_.size( ));

	    for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {

		double wx, wy;
		linear_to_world( wc_, _drawing_points_[i].first, _drawing_points_[i].second, wx, wy );
		world_pts[i].first = wx;
		world_pts[i].second = wy;

		int px, py;
		linear_to_pixel( wc_, _drawing_points_[i].first, _drawing_points_[i].second, px, py );
		pixel_pts[i].first = px;
		pixel_pts[i].second = py;
	    }
	}

	void Polygon::update_drawing_bounds_rectangle( ) {

	    _drawing_blc_x_ = _drawing_points_[0].first;
	    _drawing_blc_y_ = _drawing_points_[0].second;
	    _drawing_trc_x_ = _drawing_points_[0].first;
	    _drawing_trc_y_ = _drawing_points_[0].second;

	    for ( unsigned int i=1; i < _drawing_points_.size( ); ++i ) {
		if ( _drawing_points_[i].first < _drawing_blc_x_ ) _drawing_blc_x_ = _drawing_points_[i].first;
		if ( _drawing_points_[i].second < _drawing_blc_y_ ) _drawing_blc_y_ = _drawing_points_[i].second;
		if ( _drawing_points_[i].first > _drawing_trc_x_ ) _drawing_trc_x_ = _drawing_points_[i].first;
		if ( _drawing_points_[i].second > _drawing_trc_y_ ) _drawing_trc_y_ = _drawing_points_[i].second;
	    }
	    _drawing_width_ = _drawing_trc_x_ - _drawing_blc_x_;
	    _drawing_height_ = _drawing_trc_y_ - _drawing_blc_y_;
	}

	void Polygon::update_reference_bounds_rectangle( ) {
	    _ref_blc_x_ = _ref_points_[0].first;
	    _ref_blc_y_ = _ref_points_[0].second;
	    _ref_trc_x_ = _ref_points_[0].first;
	    _ref_trc_y_ = _ref_points_[0].second;

	    for ( unsigned int i=1; i < _ref_points_.size( ); ++i ) {
		if ( _ref_points_[i].first < _ref_blc_x_ ) _ref_blc_x_ = _ref_points_[i].first;
		if ( _ref_points_[i].second < _ref_blc_y_ ) _ref_blc_y_ = _ref_points_[i].second;
		if ( _ref_points_[i].first > _ref_trc_x_ ) _ref_trc_x_ = _ref_points_[i].first;
		if ( _ref_points_[i].second > _ref_trc_y_ ) _ref_trc_y_ = _ref_points_[i].second;
	    }
	    _ref_width_ = _ref_trc_x_ - _ref_blc_x_;
	    _ref_height_ = _ref_trc_y_ - _ref_blc_y_;
	}

	void Polygon::update_drawing_state( ) {
	    _drawing_width_ = _drawing_trc_x_ - _drawing_blc_x_;
	    _drawing_height_ = _drawing_trc_y_ - _drawing_blc_y_;

	    double x_scale = _drawing_width_ / _ref_width_;
	    double y_scale = _drawing_height_ / _ref_height_;

	    double x_offset = _drawing_blc_x_ - _ref_blc_x_;
	    double y_offset = _drawing_blc_y_ - _ref_blc_y_;

	    for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
		_drawing_points_[i] = pt( (_ref_points_[i].first - _ref_blc_x_) * x_scale + _ref_blc_x_ + x_offset,
					  (_ref_points_[i].second - _ref_blc_y_) * y_scale + _ref_blc_y_ + y_offset );
	    }	    
	}

	void Polygon::update_reference_state(  int transformations, int handle, int new_handle ) {
	    // transform reference image to match the new drawing image orientation...
	    if ( transformations & FLIP_X || transformations & FLIP_Y ) {
		for ( unsigned int i=0; i < _ref_points_.size( ); ++i ) {
		    // (1) translate to the proper axis
		    // (2) flip
		    // (3) translate back
		    if ( transformations & FLIP_X ) {
			if ( handle == 1 || handle == 2 ) {		// right outline handles
			    _ref_points_[i].first = (_ref_points_[i].first - _ref_blc_x_) * -1.0 + _ref_blc_x_;
			    // _x_origin_ = ScaleLeft;
			} else {					// left outline handles
			    _ref_points_[i].first = (_ref_points_[i].first - _ref_trc_x_) * -1.0 + _ref_trc_x_;
			    // _x_origin_ = ScaleRight;
			}
		    }
		    if ( transformations & FLIP_Y ) {
			if ( handle == 1 || handle == 4 ) {		// top outline handles
			    _ref_points_[i].second = (_ref_points_[i].second - _ref_blc_y_) * -1.0 + _ref_blc_y_;
			    // _y_origin_ = ScaleBottom;
			} else {					// bottom outline handles
			    _ref_points_[i].second = (_ref_points_[i].second - _ref_trc_y_) * -1.0 + _ref_trc_y_;
			    // _y_origin_ = ScaleTop;
			}
		    }
		}
	    }

	    // update reference bounds, width & height... just in case...
	    update_reference_bounds_rectangle( );

	    // reflection (as a result of resizing) happens to the reference points...
	    if ( new_handle == 1 ) {
		_x_origin_ = ScaleLeft;
		_y_origin_ = ScaleBottom;
	    } else if ( new_handle == 2 ) {
		_x_origin_ = ScaleLeft;
		_y_origin_ = ScaleTop;
	    } else if ( new_handle == 3 ) {
		_x_origin_ = ScaleRight;
		_y_origin_ = ScaleTop;
	    } else if ( new_handle == 4 ) {
		_x_origin_ = ScaleRight;
		_y_origin_ = ScaleBottom;
	    }

	}


	std::list<RegionInfo> *Polygon::generate_dds_statistics(  ) {
	    std::list<RegionInfo> *region_statistics = new std::list<RegionInfo>( );

	    if( wc_==0 ) return region_statistics;

	    Int zindex = 0;
	    if (wc_->restrictionBuffer()->exists("zIndex")) {
		wc_->restrictionBuffer()->getValue("zIndex", zindex);
	    }

	    DisplayData *dd = 0;
	    const std::list<DisplayData*> &dds = wc_->displaylist( );
	    Vector<Double> lin(2), wld(2);

	    Vector<Double> x(_drawing_points_.size( ));
	    Vector<Double> y(_drawing_points_.size( ));
	    for ( unsigned int i = 0; i < _drawing_points_.size( ); ++i ) {
		lin(0) = _drawing_points_[i].first;
		lin(1) = _drawing_points_[i].second;
		if ( ! wc_->linToWorld(wld, lin)) return region_statistics;
		x[i] = wld[0];
		y[i] = wld[1];
	    }

	    std::string errMsg_;
	    std::map<String,bool> processed;
	    for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
		dd = *ddi;

		PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
		if (padd==0) continue;

		try {
		    if ( ! padd->conformsTo(*wc_) ) continue;

		    ImageInterface<Float> *image = padd->imageinterface( );

		    if ( image == 0 ) continue;

		    String full_image_name = image->name(false);
		    std::map<String,bool>::iterator repeat = processed.find(full_image_name);
		    if (repeat != processed.end()) continue;
		    processed.insert(std::map<String,bool>::value_type(full_image_name,true));

		    Int nAxes = image->ndim( );
		    IPosition shp = image->shape( );
		    const CoordinateSystem &cs = image->coordinates( );

		    int zIndex = padd->activeZIndex( );
		    IPosition pos = padd->fixedPosition( );
		    Vector<Int> dispAxes = padd->displayAxes( );

		    if ( nAxes == 2 ) dispAxes.resize(2,True);

		    if ( nAxes < 2 || Int(shp.nelements()) != nAxes ||
			 Int(pos.nelements()) != nAxes ||
			 anyLT(dispAxes,0) || anyGE(dispAxes,nAxes) )
			continue;

		    if ( dispAxes.nelements() > 2u )
			pos[dispAxes[2]] = zIndex;

		    dispAxes.resize(2,True);
#if 0
		    // WCBox dummy;
		    Quantum<Double> px0(0.,"pix");
		    Vector<Quantum<Double> > blcq(nAxes,px0), trcq(nAxes,px0);

		    Int spaxis = getAxisIndex( image, String("Spectral") );
		    for (Int ax = 0; ax < nAxes; ax++) {
			if ( ax == dispAxes[0] || ax == dispAxes[1] || ax == spaxis) {
			    trcq[ax].setValue(shp[ax]-1);
			} else  {
			    blcq[ax].setValue(pos[ax]);
			    trcq[ax].setValue(pos[ax]);
			}
		    }

		    // technically (I guess), WorldCanvasHolder::worldAxisUnits( ) should be
		    // used here, because it references the "CSmaster" DisplayData which all
		    // of the display options are referenced from... lets hope all of the
		    // coordinate systems are kept in sync...      <drs>
		    const Vector<String> &units = wc_->worldAxisUnits( );

		    for (Int i = 0; i < 2; i++) {
			Int ax = dispAxes[i];

			blcq[ax].setValue(blc[i]);
			trcq[ax].setValue(trc[i]);

			blcq[ax].setUnit(units[i]);
			trcq[ax].setUnit(units[i]);
		    }

		    // WCBox box(blcq, trcq, cs, Vector<Int>());
#else
		    // technically (I guess), WorldCanvasHolder::worldAxisUnits( ) should be
		    // used here, because it references the "CSmaster" DisplayData which all
		    // of the display options are referenced from... lets hope all of the
		    // coordinate systems are kept in sync...      <drs>
		    const Vector<String> &units = wc_->worldAxisUnits( );

#endif
		    Quantum<Vector<Double> > qx(x, units[0]), qy(y, units[1]);

		    WCPolygon poly(qx, qy, IPosition(dispAxes), cs);
		    ImageRegion *imageregion = new ImageRegion(poly);

		    region_statistics->push_back(ImageRegionInfo(full_image_name,getLayerStats(padd,image,*imageregion)));
		    delete imageregion;

		} catch (const casa::AipsError& err) {
		    errMsg_ = err.getMesg();
		    continue;
		} catch (...) {
		    errMsg_ = "Unknown error converting region";
		    continue;
		}
	    }
	    return region_statistics;
	}

    }
}
