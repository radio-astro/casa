//# point.h: base class for statistical regions
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


#ifndef REGION_POINT_H_
#define REGION_POINT_H_

#include <display/Display/MouseToolState.h>
#include <display/region/Rectangle.h>
#include <casa/BasicSL/String.h>
#include <list>

namespace casa {

	class PanelDisplay;
	class AnnRegion;
	class PixelCanvas;

	namespace viewer {

		// carry over from QtRegion... hopefully, removed soon...
		class QtRegionSourceKernel;

		// All regions are specified in "linear coordinates", not "pixel coordinates". This is necessary
		// because "linear coordinates" scale with zooming whereas "pixel coordinates" do not. Unfortunately,
		// this means that coordinate transformation is required each time the region is drawn.
		class Point : public Rectangle {
		public:
			~Point( );
			Point( WorldCanvas *wc, QtRegionDock *d, double x, double y, QtMouseToolNames::PointRegionSymbols sym ) :
				Rectangle( wc, d, x, y, x, y ), marker_(sym) {
				/***updateStatistics***/
			}

			// carry over from QtRegion... hopefully, removed soon...
			Point( QtRegionSourceKernel *factory, WorldCanvas *wc, double x, double y, bool hold_signals=false, QtMouseToolNames::PointRegionSymbols sym=QtMouseToolNames::SYM_DOT ) :
				Rectangle( "point", wc, factory->dock( ), x, y, x, y, hold_signals, sym ), marker_(sym) {
				/***updateStatistics***/
			}

			bool setMarker( QtMouseToolNames::PointRegionSymbols sym );

			int clickHandle( double /*x*/, double /*y*/ ) const {
				return 0;
			}

			bool clickWithin( double x, double y ) const;

			// returns point state (Region::PointLocation)
			region::PointInfo checkPoint( double x, double y ) const;

			// returns mouse movement state
			unsigned int mouseMovement( double x, double y, bool other_selected );
			void resize( double, double ) { }

			AnnotationBase *annotation( ) const;

			// points cannot be degenerate...
			bool degenerate( ) const {
				return false;
			}

			// fetch region type...
			region::RegionTypes type( ) const {
				return region::PointRegion;
			}

			void releaseSignals( );

		protected:

			static const int radius;

			virtual void fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
			                                   std::vector<std::pair<double,double> > &world_pts ) const;

			void drawRegion( bool );

			std::list<std::tr1::shared_ptr<RegionInfo> > *generate_dds_centers( );

			QtMouseToolNames::PointRegionSymbols marker_;

		private:
			virtual bool output_region( ds9writer &out, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) const;
			void draw_arrow( PixelCanvas *, int /*x*/, int /*y*/, int /*xsign*/, int /*ysign*/,
			                 int /*scale_unit*/, int /*scale*/ );

		};
	}
}

#endif
