//# RegionTool.h: base class for event-based tools which create and manage regions
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

#ifndef DISPLAY_REGIONTOOL_H__
#define DISPLAY_REGIONTOOL_H__

#include <map>
#include <set>
#include <list>
#include <display/region/RegionEnums.h>
#include <display/DisplayEvents/MultiWCTool.h>
#include <display/Utilities/VOID.h>

namespace casa {
	class RegionTool : public MultiWCTool {
	public:
		typedef std::map<viewer::Region*,viewer::region::PointInfo> region_map_type;

		enum RegionToolTypes { POLYTOOL, RECTTOOL, POINTTOOL, ELLIPSETOOL, POLYLINETOOL, PVLINETOOL };

		class State {
		public:
			State( WorldCanvas *wc, double X, double Y ) : wc_(wc), x_(X), y_(Y), refresh_count(0) {  }
			void insert( RegionTool *tool, viewer::Region *region, const viewer::region::PointInfo &state );
			// get the number of regions in a particular state
			unsigned int count( viewer::region::PointLocation state );
			bool exists( viewer::Region * ) const;
			// get the number of regions by a specific tool
			/* region_map_type &regions( ) { return state_map; } */
			viewer::region::region_list_type &regions( ) {
				return all_regions;
			}
			viewer::region::region_list_type &regions( RegionTool *tool );
			SHARED_PTR<viewer::region::region_list_type> regions( viewer::region::PointLocation loc,
			        viewer::region::RegionSelect select );
			viewer::region::PointInfo state( viewer::Region *region );

			double x( ) const {
				return x_;
			}
			double y( ) const {
				return y_;
			}
			WorldCanvas *wc( ) {
				return wc_;
			}

			void refresh( ) {
				++refresh_count;
			}

			~State( );

		private:

			SHARED_PTR<viewer::region::region_list_type> filter( SHARED_PTR<viewer::region::region_list_type>,
			        viewer::region::RegionSelect );

			void *operator new( size_t ); // { /* refresh( ) with automatic creation/deletion in mind */ }
			State( const State & ) { }
			State( ) { }

			typedef std::map< RegionTool*, viewer::region::region_list_type > tool_regions_type;
			typedef std::map< viewer::region::PointLocation, SHARED_PTR<viewer::region::region_list_type> > state_count_type;

			WorldCanvas *wc_;
			double x_, y_;
			unsigned int refresh_count;

			region_map_type state_map;
			tool_regions_type tool_map;
			state_count_type count_map;
			viewer::region::region_list_type all_regions;
			viewer::region::region_list_type default_region_list;
		};
		// Constructor taking the primary key to which this tool will respond.
		RegionTool( Display::KeySym keysym = Display::K_Pointer_Button1) : MultiWCTool( keysym, false ) { }
		~RegionTool( ) { }

		void keyPressed(const WCPositionEvent &ev);
		void keyReleased(const WCPositionEvent &);
		void otherKeyPressed(const WCPositionEvent &);
		void otherKeyReleased(const WCPositionEvent &);
		void moved(const WCMotionEvent &ev, const viewer::region::region_list_type & /*selected_regions*/);
		void draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type & /*selected_regions*/);

		// this is a non-const function because a non-const RegionTool ptr is
		// returned in 'state' which can then be used for setting the state of
		// the regions managed by the various tools....
		virtual void checkPoint( WorldCanvas *wc, State &state ) = 0;

		// create regions of various types (e.g. point, rect, poly, etc.). For use when
		// loading casa region files...
		virtual bool create( viewer::region::RegionTypes, WorldCanvas */*wc*/, const std::vector<std::pair<double,double> > &/*pts*/,
		                     const std::string &/*label*/, viewer::region::TextPosition /*label_pos*/, const std::vector<int> &/*label_off*/,
		                     const std::string &/*font*/, int /*font_size*/, int /*font_style*/, const std::string &/*font_color*/,
		                     const std::string &/*line_color*/, viewer::region::LineStyle /*line_style*/, unsigned int /*line_width*/,
		                     bool /*annotation*/, VOID */*region_specific_state*/ ) = 0;
//			DISPLAY_PURE_VIRTUAL(RegionTool::create,true);

		// defeat the registering/unregistering done by MultiWCTool (because polygon regions stop working
		// because they see a double-click from both the RegionToolManager and MultiWCTool)...
		void enable() { }
		void disable() { }

		virtual RegionToolTypes type( ) const = 0;

		// called when the user (read GUI user) indicates that a region should be deleted...
		virtual void revokeRegion( viewer::Region * ) = 0;

	};
}

#endif
