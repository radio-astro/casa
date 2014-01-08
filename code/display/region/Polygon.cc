//# Polygon.cc: non-GUI polygon region
//# Copyright (C) 2012
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
#include <images/Images/SubImage.h>

#include <imageanalysis/Annotations/AnnPolygon.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <display/DisplayErrors.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/ds9/ds9writer.h>

#include <tr1/memory>

namespace casa {
	namespace viewer {

		Polygon::Polygon( WorldCanvas *wc, QtRegionDock *d, double x1, double y1 ) :
			Region( "polygon", wc, d, 0 ),
			_ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
			_drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
			_ref_points_.push_back(pt(x1,y1));
			_drawing_points_.push_back(pt(x1,y1));
		}


		Polygon::Polygon( QtRegionSourceKernel *rs, WorldCanvas *wc, double x1, double y1, bool hold_signals ) :
			Region( "polygon", wc, rs->dock( ), hold_signals ),
			_ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
			_drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
			_ref_points_.push_back(pt(x1,y1));
			_drawing_points_.push_back(pt(x1,y1));
			initHistogram();
		}


		Polygon::Polygon( WorldCanvas *wc, QtRegionDock *d, const std::vector<std::pair<double,double> > &pts) :
			Region( "polygon", wc, d, 0 ), _ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
			_drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
			for ( size_t i=0; i < pts.size(); ++i ) {
				_ref_points_.push_back(pt(pts[i].first,pts[i].second));
				_drawing_points_.push_back(pt(pts[i].first,pts[i].second));
			}
			initHistogram();
			closeFigure(false);
		}

// carry over from QtRegion... hopefully, removed soon...
		Polygon::Polygon( QtRegionSourceKernel *rs, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts, bool hold_signals ) :
			Region( "polygon", wc, rs->dock( ), hold_signals ), _ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
			_drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
			for ( size_t i=0; i < pts.size(); ++i ) {
				_ref_points_.push_back(pt(pts[i].first,pts[i].second));
				_drawing_points_.push_back(pt(pts[i].first,pts[i].second));
			}
			initHistogram();
			closeFigure(false);
		}

		void Polygon::closeFigure( bool signal_complete ) {
			complete = true;
			unsigned int size = _ref_points_.size( );
			if ( size > 1 && _ref_points_[size-1].first == _ref_points_[size-2].first &&
			        _ref_points_[size-1].second == _ref_points_[size-2].second ) {
				_ref_points_.pop_back( );
				_drawing_points_.pop_back( );
			}
			update_reference_bounds_rectangle( );
			update_drawing_bounds_rectangle( );
			if ( signal_complete ) {
				try {
					polygonComplete( );
				} catch (...) {
					/*fprintf( stderr, "******\tregion selection errors - %s, %d \t******\n", __FILE__, __LINE__ );*/
				}
			}
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

		void Polygon::polygonComplete( ) {
			// for polygons signals remain blocked until polygon is complete...
			releaseSignals( );
			updateStateInfo( true, region::RegionChangeModified );
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

			updateStateInfo( true, region::RegionChangeModified );
			setDrawCenter(false);
			invalidateCenterInfo();
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

// returns point state (Region::PointLocation)
		region::PointInfo Polygon::checkPoint( double x, double y )  const {

			if ( complete == false )
				return region::PointInfo( x, y, region::PointOutside );

			unsigned int result = 0;
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );
			if ( x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y )
				result |= region::PointInside;
			unsigned int handle = check_handle( x, y );
			if ( handle )
				result |= region::PointHandle;
			return region::PointInfo( x, y, result == 0 ? (unsigned int) region::PointOutside : result, handle );
		}

// returns mouse movement state
		unsigned int Polygon::mouseMovement( double x, double y, bool other_selected ) {
			unsigned int result = 0;

			if ( visible_ == false ) return result;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );
			if ( x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y || within_vertex_handle( x, y ) ) {
				weaklySelect( mouse_in_region == false );
				mouse_in_region = true;
				result |= region::MouseSelected;
				result |= region::MouseRefresh;
				selected_ = true;
				draw( other_selected );		// this draw may not be necessary (?)...
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

		bool Polygon::clickWithin( double x, double y ) const {
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );
			return x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y;
		}


		void Polygon::output( ds9writer &out ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
			std::string path = QtDisplayData::path(wc_->csMaster());
			out.setCsysSource(path.c_str( ));
			out.polygon(wc_,drawing_points( ));
		}

		unsigned int Polygon::check_handle( double x, double y ) const {
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );
			if ( visible_ == false ) return 0;
			bool blc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= blc_y && y <= (blc_y + handle_delta_y);
			bool tlc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= (trc_y - handle_delta_y) && y <= trc_y;
			bool brc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= blc_y && y <= (blc_y + handle_delta_y);
			bool trc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= (trc_y - handle_delta_y) && y <= trc_y;

			// state for resizing (upon click & moving within a handle)...
			// may not be needed with the new RegionToolManager... or might be...
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

		int Polygon::clickHandle( double x, double y ) const {
			return check_handle( x, y );
		}

		bool Polygon::valid_translation( double dx, double dy, double width_delta, double height_delta ) {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return false;

			const double lxmin = wc_->linXMin( );
			const double lxmax = wc_->linXMax( );
			const double lymin = wc_->linYMin( );
			const double lymax = wc_->linYMax( );

			const double x_delta = width_delta  / 2.0;
			const double y_delta = height_delta  / 2.0;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			double pt = blc_x + dx - x_delta;
			if ( pt < lxmin || pt > lxmax ) return false;
			pt = trc_x + dx + x_delta;
			if ( pt < lxmin || pt > lxmax ) return false;
			pt = blc_y + dy - y_delta;
			if ( pt < lymin || pt > lymax ) return false;
			pt = trc_y + dy + y_delta;
			if ( pt < lymin || pt > lymax ) return false;
			return true;
		}

		void Polygon::resize( double width_delta, double height_delta ) {
			double dx = width_delta / 2.0;
			double dy = height_delta / 2.0;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			blc_x -= dx;
			blc_y -= dy;
			trc_x += dx;
			trc_y += dy;

			update_reference_bounds_rectangle( );

			// set the drawing bounds to the new bounds...
			// update scaling
			_drawing_blc_x_ = blc_x;
			_drawing_blc_y_ = blc_y;
			_drawing_trc_x_ = trc_x;
			_drawing_trc_y_ = trc_y;

			update_drawing_state( );
			updateStateInfo( true, region::RegionChangeModified );
		}

		int Polygon::moveHandle( int handle, double x, double y ) {

			if ( handle >= 1 && handle <= 4 )
				handle = move_sizing_rectangle_handle( handle, x, y );
			else if ( handle >= 5 )
				handle = move_vertex( handle, x, y );
			else
				return 0;

			updateStateInfo( true, region::RegionChangeModified );
			setDrawCenter(false);
			invalidateCenterInfo();
			return handle;
		}

		int Polygon::move_vertex( int handle, double x, double y ) {
			int vertex = handle - 5;
			if ( vertex >= (int) _ref_points_.size( ) || vertex < 0 )
				return 0;

			// use current x & y scaling to translate drawing vertex
			// back to reference vertex...
			double x_scale = _ref_width_ / _drawing_width_;
			double y_scale = _ref_height_ / _drawing_height_;

			// moving a vertex can change drawing bounding rectangle...
			// calculate any adjustment...
			double new_blc_x = x, new_blc_y = y, new_trc_x = x, new_trc_y = y;
			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
				if ( (int) i != vertex ) {
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
			refresh_state_gui( ); /***updateStatistics***/

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
			refresh_state_gui( ); /***updateStatistics***/

			return new_handle;
		}

		void Polygon::linearCenter( double &x, double &y ) const {
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			x = linear_average(blc_x,trc_x);
			y = linear_average(blc_y,trc_y);
		}

		void Polygon::pixelCenter( double &x, double &y ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			double lx = linear_average(blc_x,trc_x);
			double ly = linear_average(blc_y,trc_y);

			try {
				linear_to_pixel( wc_, lx, ly, x, y );
			} catch(...) {
				return;
			}
		}

		void Polygon::drawRegion( bool selected ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;

			if ( _drawing_points_.size( ) == 0 ) return;

			int x1, y1, x2, y2;
			try {
				linear_to_screen( wc_, _drawing_points_[0].first, _drawing_points_[0].second, x1, y1 );
			} catch(...) {
				return;
			}

			// draw the center
			if (getDrawCenter())
				drawCenter( _center_x, _center_y, _center_delta_x, _center_delta_y);

			int first_x = x1, first_y = y1;
			for ( unsigned int i=1; i < _drawing_points_.size( ); ++i ) {
				try {
					linear_to_screen( wc_, _drawing_points_[i].first, _drawing_points_[i].second, x2, y2 );
				} catch(...) {
					return;
				}
				pc->drawLine(x1,y1,x2,y2);
				x1 = x2;
				y1 = y2;
			}
			if ( complete ) pc->drawLine( x1, y1, first_x, first_y );

			if ( selected && memory::nullptr.check( creating_region ) ) {

				// get bounding rectangle...
				double blc_x, blc_y, trc_x, trc_y;
				boundingRectangle( blc_x, blc_y, trc_x, trc_y );
				try {
					linear_to_screen( wc_, blc_x, blc_y, trc_x, trc_y, x1, y1, x2, y2 );
				} catch(...) {
					return;
				}

				// compute handle size...
				Int w = x2 - x1;
				Int h = y2 - y1;

				Int s = 0;
				if (w>=18 && h>=18) s = 6;
				else if (w>=15 && h>=15) s = 5;
				else if (w>=12 && h>=12) s = 4;
				else if (w>=9 && h>=9) s = 3;

				// get handle size in linear coordinates...
				double xdx, ydy;
				try {
					screen_to_linear( wc_, x1 + s, y1 + s, xdx, ydy );
				} catch(...) {
					return;
				}
				handle_delta_x = xdx - blc_x;
				handle_delta_y = ydy - blc_y;

				// draw outline rectangle for resizing whole polygon...
				pushDrawingEnv(region::DotLine);
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
					pushDrawingEnv( region::SolidLine);
					if ( weaklySelected( ) ) {
						if ( marked_region_count( ) > 0 && mouse_in_region ) {
							pc->drawRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
							pc->drawRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
							pc->drawRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
							pc->drawRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
						} else {
							pc->drawFilledRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
							pc->drawFilledRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
							pc->drawFilledRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
							pc->drawFilledRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
						}

						for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
							int h_blc_x, h_blc_y, h_trc_x, h_trc_y;
							try {
								linear_to_screen( wc_, _drawing_points_[i].first - handle_delta_x / 2.0,
								                  _drawing_points_[i].second - handle_delta_y / 2.0,
								                  _drawing_points_[i].first + handle_delta_x / 2.0,
								                  _drawing_points_[i].second + handle_delta_y / 2.0,
								                  h_blc_x, h_blc_y, h_trc_x, h_trc_y );
							} catch(...) {
								return;
							}

							pc->drawFilledRectangle( h_blc_x, h_blc_y, h_trc_x, h_trc_y );
						}

					} else if ( marked( ) ) {
						pc->drawRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
						pc->drawRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
						pc->drawRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
						pc->drawRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
					} else {
						pc->drawFilledRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
						pc->drawFilledRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
						pc->drawFilledRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
						pc->drawFilledRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);

						for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
							int h_blc_x, h_blc_y, h_trc_x, h_trc_y;
							try {
								linear_to_screen( wc_, _drawing_points_[i].first - handle_delta_x / 2.0,
								                  _drawing_points_[i].second - handle_delta_y / 2.0,
								                  _drawing_points_[i].first + handle_delta_x / 2.0,
								                  _drawing_points_[i].second + handle_delta_y / 2.0,
								                  h_blc_x, h_blc_y, h_trc_x, h_trc_y );
							} catch(...) {
								return;
							}

							pc->drawFilledRectangle( h_blc_x, h_blc_y, h_trc_x, h_trc_y );
						}
					}

					popDrawingEnv( );
				}
			}
		}

		AnnotationBase *Polygon::annotation( ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

			const DisplayCoordinateSystem &cs = wc_->coordinateSystem( );
			const Vector<String> &units = wc_->worldAxisUnits( );

			Vector<Quantity> xv(_drawing_points_.size( ));
			Vector<Quantity> yv(_drawing_points_.size( ));
			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
				double x,y;
				try {
					linear_to_world( wc_, _drawing_points_[i].first, _drawing_points_[i].second, x, y );
				} catch(...) {
					return 0;
				}
				xv[i] = Quantity(x,units[0]);
				yv[i] = Quantity(y,units[1]);
			}

			Vector<Stokes::StokesTypes> stokes;
			/*Int polaxis =*/
			CoordinateUtil::findStokesAxis(stokes, cs);
			const DisplayData *dd = wc_->displaylist().front();

			AnnPolygon *poly = 0;
			try {
				std::vector<int> axes = dd->displayAxes( );
				IPosition shape(cs.nPixelAxes( ));
				for ( size_t i=0; i < shape.size( ); ++i )
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

		void Polygon::fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
		                                    std::vector<std::pair<double,double> > &world_pts ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			type = region::PolyRegion;

			pixel_pts.resize(_drawing_points_.size( ));
			world_pts.resize(_drawing_points_.size( ));

			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {

				double wx, wy;
				try {
					linear_to_world( wc_, _drawing_points_[i].first, _drawing_points_[i].second, wx, wy );
				} catch(...) {
					return;
				}
				world_pts[i].first = wx;
				world_pts[i].second = wy;

				double px, py;
				try {
					linear_to_pixel( wc_, _drawing_points_[i].first, _drawing_points_[i].second, px, py );
				} catch(...) {
					return;
				}
				pixel_pts[i].first  = static_cast<int>(px);
				pixel_pts[i].second = static_cast<int>(py);
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

		std::list<std::tr1::shared_ptr<RegionInfo> > *Polygon::generate_dds_centers() {
			std::list<std::tr1::shared_ptr<RegionInfo> > *region_centers = new std::list<std::tr1::shared_ptr<RegionInfo> >( );

			if( wc_==0 ) return region_centers;

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
				if ( ! wc_->linToWorld(wld, lin)) return region_centers;
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

					std::tr1::shared_ptr<ImageInterface<Float> > image ( padd->imageinterface( ));

					if ( ! image  ) continue;

					String description = image->name(false);
					String name = image->name(true);
					std::map<String,bool>::iterator repeat = processed.find(description);
					if (repeat != processed.end()) continue;
					processed.insert(std::map<String,bool>::value_type(description,true));

					Int nAxes = image->ndim( );
					IPosition shp = image->shape( );
					const DisplayCoordinateSystem &cs = image->coordinates( );

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

					// select the visible layer in the third and all
					// hidden axes with a WCBox and a SubImage
					Quantum<Double> px0(0.,"pix");
					Vector<Quantum<Double> > blcq(nAxes,px0), trcq(nAxes,px0);
					for (Int ax = 0; ax < nAxes; ax++) {
						if ( ax == dispAxes[0] || ax == dispAxes[1]) {
							trcq[ax].setValue(shp[ax]-1);
						} else  {
							blcq[ax].setValue(pos[ax]);
							trcq[ax].setValue(pos[ax]);
						}
					}
					WCBox box(blcq, trcq, cs, Vector<Int>());
					ImageRegion     *imgbox = new ImageRegion(box);
					std::tr1::shared_ptr<SubImage<Float> > boxImg(new SubImage<Float>(*image, *imgbox));

					// technically (I guess), WorldCanvasHolder::worldAxisUnits( ) should be
					// used here, because it references the "CSmaster" DisplayData which all
					// of the display options are referenced from... lets hope all of the
					// coordinate systems are kept in sync...      <drs>
					const Vector<String> &units = wc_->worldAxisUnits( );

					Quantum<Vector<Double> > qx(x, units[0]), qy(y, units[1]);
					WCPolygon poly(qx, qy, IPosition(dispAxes), cs);

					ImageRegion *imageregion = new ImageRegion(poly);

					region_centers->push_back(std::tr1::shared_ptr<RegionInfo>(new ImageRegionInfo(name,description,getLayerCenter(padd,boxImg,*imageregion))));

					delete imgbox;
					delete imageregion;
				} catch (const casa::AipsError& err) {
					errMsg_ = err.getMesg();
					fprintf( stderr, "Polygon::generate_dds_centers( ): %s\n", errMsg_.c_str() );
					continue;
				} catch (...) {
					errMsg_ = "Unknown error converting region";
					fprintf( stderr, "Polygon::generate_dds_centers( ): %s\n", errMsg_.c_str() );
					continue;
				}
			}
			return region_centers;
		}

//		cout << "in Polygon::generate_dds_centers()" <<endl;
//		return region_centers;
//	}

		ImageRegion *Polygon::get_image_region( DisplayData *dd ) const {

			if( wc_==0 ) return 0;

			PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
			if ( padd == 0 ) return 0;

			Vector<Double> lin(2), wld(2);
			Vector<Double> x(_drawing_points_.size( ));
			Vector<Double> y(_drawing_points_.size( ));
			for ( unsigned int i = 0; i < _drawing_points_.size( ); ++i ) {
				lin(0) = _drawing_points_[i].first;
				lin(1) = _drawing_points_[i].second;
				if ( ! wc_->linToWorld(wld, lin)) return 0;
				x[i] = wld[0];
				y[i] = wld[1];
			}

			std::tr1::shared_ptr<ImageInterface<Float> >image (padd->imageinterface( ));
			if ( ! image ) return 0;

			Vector<Int> dispAxes = padd->displayAxes( );
			dispAxes.resize(2,True);

			const Vector<String> &units = wc_->worldAxisUnits( );
			const DisplayCoordinateSystem &cs = image->coordinates( );
			Quantum<Vector<Double> > qx(x, units[0]), qy(y, units[1]);

			ImageRegion *result = 0;
			try {
				WCPolygon poly(qx, qy, IPosition(dispAxes), cs);
				result = new ImageRegion(poly);
			} catch(...) { }
			return result;
		}

	}
}
