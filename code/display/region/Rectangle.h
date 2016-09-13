//# rectangle.h: base class for statistical regions
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


#ifndef REGION_RECTANGLE_H_
#define REGION_RECTANGLE_H_

#include <display/region/Region.qo.h>
#include <display/region/QtRegionSource.qo.h>
#include <casa/BasicSL/String.h>
#include <list>

namespace casa {

	class PanelDisplay;
	class AnnotationBase;
	class MSAsRaster;
	class DisplayData;

	namespace viewer {

		// carry over from QtRegion... hopefully, removed soon...
		class QtRegionSourceKernel;

		// All regions are specified in "linear coordinates", not "pixel coordinates". This is necessary
		// because "linear coordinates" scale with zooming whereas "pixel coordinates" do not. Unfortunately,
		// this means that coordinate transformation is required each time the region is drawn.
		class Rectangle : public Region {
		public:
			~Rectangle( );
			Rectangle( WorldCanvas *wc, QtRegionDock *d, double x1, double y1, double x2, double y2,
			           bool hold_signals=false );

			// carry over from QtRegion... hopefully, removed soon...
			Rectangle( QtRegionSourceKernel *rs, WorldCanvas *wc, double x1, double y1, double x2, double y2,
			           bool hold_signals=false);

			bool clickWithin( double x, double y ) const {
				return x > blc_x && x < trc_x && y > blc_y && y < trc_y;
			}

			int clickHandle( double x, double y ) const;

			bool doubleClick( double /*x*/, double /*y*/ );

			// returns point state (Region::PointLocation)
			region::PointInfo checkPoint( double x, double y ) const;

			// returns mouse state (Region::MouseState)
			unsigned int mouseMovement( double x, double y, bool other_selected );

			// for rectangles, resizing can change the handle...
			// for rectangles, moving a handle is resizing...
			int moveHandle( int handle, double x, double y );
			void move( double dx, double dy );

			void resize( double /*width_delta*/, double /*height_delta*/ );
			bool valid_translation( double dx, double dy, double width_delta, double height_delta );

			void linearCenter( double &x, double &y ) const;
			void pixelCenter( double &x, double &y ) const;

			AnnotationBase *annotation( ) const;

			virtual bool flag( MSAsRaster *msar );

			// in "linear" coordinates...
			void boundingRectangle( double &blcx, double &blcy, double &trcx, double &trcy ) const;

			void output( ds9writer &out ) const;

			// fetch region type...
			region::RegionTypes type( ) const {
				return region::RectRegion;
			}

		protected:
			Rectangle( const std::string &name, WorldCanvas *wc, QtRegionDock *d, double x1,
			           double y1, double x2, double y2,
			           bool hold_signals=false, QtRegionState *supplied_state=0,
			           QtMouseToolNames::PointRegionSymbols sym=QtMouseToolNames::SYM_UNKNOWN ) :
				Region( name, wc, d, hold_signals, supplied_state, sym ), blc_x(x1<x2?x1:x2),
				blc_y(y1<y2?y1:y2), trc_x(x1<x2?x2:x1), trc_y(y1<y2?y2:y1) {
				initHistogram();
				complete = true;
			}

			RegionInfo::stats_t *get_ms_stats( MSAsRaster *msar, double x, double y );
			void generate_nonimage_statistics( DisplayData*, std::list<RegionInfo> * );
			std::list<SHARED_PTR<RegionInfo> > *generate_dds_centers( );
			ImageRegion *get_image_region( DisplayData* ) const;

			virtual void fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
			                                   std::vector<std::pair<double,double> > &world_pts ) const;

			void drawRegion( bool );
			/* void drawHandles( ); */

			virtual void setCenter(double &x, double &y, double &deltx, double &delty) {
				center_x_=x;
				center_y_=y;
				center_delta_x_=deltx;
				center_delta_y_=delty;
			};

			double blc_x, blc_y;
			double trc_x, trc_y;
			double center_x_, center_y_;
			double center_delta_x_, center_delta_y_;
			double handle_delta_x, handle_delta_y;

		private:
			virtual bool output_region( ds9writer &out, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) const;

			bool within_vertex_handle( double x, double y ) const;
			unsigned int check_handle( double x, double y ) const;

		};
	}
}

#endif
