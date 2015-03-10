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


#ifndef REGION_POLYGON_H_
#define REGION_POLYGON_H_

#include <display/region/Region.qo.h>
#include <display/region/QtRegionSource.qo.h>
#include <casa/BasicSL/String.h>
#include <vector>

namespace casa {

	class AnnotationBase;
	class DisplayData;

	namespace viewer {

		// carry over from QtRegion... hopefully, removed soon...
		class QtRegionSourceKernel;

		class Polygon : public Region {
		public:
			typedef std::pair<double,double> pt;
			typedef std::vector<pt> point_list;

			Polygon( WorldCanvas *wc, QtRegionDock *d, double x1, double y1 );
			Polygon( WorldCanvas *wc, QtRegionDock *d, const std::vector<std::pair<double,double> > &pts );
			// carry over from QtRegion... hopefully, removed soon...
			Polygon( QtRegionSourceKernel *, WorldCanvas *wc,
			         const std::vector<std::pair<double,double> > &pts,
			         bool hold_signals=false );
			Polygon( QtRegionSourceKernel *rs, WorldCanvas *wc, double x1,
			         double y1, bool hold_signals=false );


			~Polygon( ) { }

			bool clickWithin( double x, double y ) const;
			int clickHandle( double x, double y ) const;
			// for rectangles, resizing can change the handle...
			// for rectangles, moving a handle is resizing...
			int moveHandle( int handle, double x, double y );
			void move( double dx, double dy );
			void resize( double /*width_delta*/, double /*height_delta*/ );
			bool valid_translation( double dx, double dy, double width_delta, double height_delta );

			// returns point state (Region::PointLocation)
			region::PointInfo checkPoint( double x, double y ) const;

			// returns mouse state (Region::MouseState)
			unsigned int mouseMovement( double x, double y, bool other_selected );

			void linearCenter( double &x, double &y ) const;
			void pixelCenter( double &x, double &y ) const;

			void closeFigure( bool signal_complete=true );
			void polygonComplete( );

			void addVertex( double x, double y, bool rewrite_last_point=false );

			AnnotationBase *annotation( ) const;

			// return the *drawing* bounding rectangle...
			// in "linear" coordinates...
			void boundingRectangle( double &blcx, double &blcy, double &trcx, double &trcy ) const;

			int numVertices( ) const {
				return drawing_points( ).size( );
			}

			void output( ds9writer &out ) const;

			// fetch region type...
			region::RegionTypes type( ) const {
				return region::PolyRegion;
			}

		protected:
			unsigned int check_handle( double x, double y ) const;

			enum YScaleTo { ScaleTop, ScaleBottom };
			enum XScaleTo { ScaleLeft, ScaleRight };
			enum Tranformations { FLIP_X = 1 << 0, FLIP_Y = 1 << 1 };
			std::list<SHARED_PTR<RegionInfo> > *generate_dds_centers( );
			ImageRegion *get_image_region( DisplayData* ) const;

			void drawRegion( bool );

			virtual void fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
			                                   std::vector<std::pair<double,double> > &world_pts ) const;

			const point_list &drawing_points( ) const {
				return _drawing_points_;
			}

		private:
			bool within_vertex_handle( double x, double y ) const;

			int move_sizing_rectangle_handle( int handle, double x, double y );
			int move_vertex( int handle, double x, double y );

			void update_drawing_bounds_rectangle( );
			void update_reference_bounds_rectangle( );

			void update_drawing_state( );
			void update_reference_state( int transformations, int handle, int new_handle );

			virtual void setCenter(double &x, double &y, double &deltx, double &delty) {
				_center_x=x;
				_center_y=y;
				_center_delta_x=deltx;
				_center_delta_y=delty;
			};

			double handle_delta_x, handle_delta_y;

			point_list _ref_points_;
			double _ref_blc_x_, _ref_blc_y_;
			double _ref_trc_x_, _ref_trc_y_;
			double _ref_width_, _ref_height_;

			point_list _drawing_points_;
			double _drawing_blc_x_, _drawing_blc_y_;
			double _drawing_trc_x_, _drawing_trc_y_;
			double _drawing_width_, _drawing_height_;

			double _center_x, _center_y;
			double _center_delta_x, _center_delta_y;

			XScaleTo _x_origin_;
			YScaleTo _y_origin_;
		};
	}
}

#endif
