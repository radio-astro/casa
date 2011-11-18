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
#include <display/region/Region.h>
#include <display/DisplayEvents/MultiWCTool.h>

namespace casa {
    class RegionTool : public MultiWCTool {
	public:
	    typedef std::set<viewer::Region*> region_list_type;
	    typedef std::map<viewer::Region*,viewer::Region::PointInfo> region_map_type;

	    class State {
		public:
		    State( WorldCanvas *wc, double X, double Y ) : wc_(wc), x_(X), y_(Y), refresh_count(0) {  }
		    void insert( RegionTool *tool, viewer::Region *region, const viewer::Region::PointInfo &state );
		    // get the number of regions in a particular state
		    unsigned int count( viewer::Region::PointLocation state );
		    bool exists( viewer::Region * ) const;
		    // get the number of regions by a specific tool
		    /* region_map_type &regions( ) { return state_map; } */
		    region_list_type &regions( ) { return all_regions; }
		    region_list_type &regions( RegionTool *tool );
		    region_list_type &regions( viewer::Region::PointLocation loc );
		    const viewer::Region::PointInfo &state( viewer::Region *region );

		    double x( ) const { return x_; }
		    double y( ) const { return y_; }
		    WorldCanvas *wc( ) { return wc_; }

		    void refresh( ) { ++refresh_count; }

		    ~State( );

		private:
		    void *operator new( unsigned long ); // { /* refresh( ) with automatic creation/deletion in mind */ }
		    State( const State & ) { }
		    State( ) { }

		    typedef std::map< RegionTool*, region_list_type > tool_regions_type;
		    typedef std::map< viewer::Region::PointLocation, region_list_type > state_count_type;

		    WorldCanvas *wc_;
		    double x_, y_;
		    unsigned int refresh_count;

		    region_map_type state_map;
		    tool_regions_type tool_map;
		    state_count_type count_map;
		    region_list_type all_regions;
		    region_list_type default_region_list;
	    };
	    // Constructor taking the primary key to which this tool will respond.
	    RegionTool( Display::KeySym keysym = Display::K_Pointer_Button1) : MultiWCTool( keysym, false ) { }
	    ~RegionTool( ) { }

	    void keyPressed(const WCPositionEvent &ev);
	    void keyReleased(const WCPositionEvent &);
	    void otherKeyPressed(const WCPositionEvent &);
	    void otherKeyReleased(const WCPositionEvent &);
	    void moved(const WCMotionEvent &);
	    void draw(const WCRefreshEvent &);

	    // this is a non-const function because a non-const RegionTool ptr is
	    // returned in 'state' which can then be used for setting the state of
	    // the regions managed by the various tools....
	    virtual void checkPoint( WorldCanvas *wc, State &state ) = 0;

	    // create regions of various types (e.g. point, rect, poly, etc.). For use when
	    // loading casa region files...
	    virtual bool create( WorldCanvas */*wc*/, const std::vector<std::pair<double,double> > &/*pts*/,
				 const std::string &/*label*/, const std::string &/*font*/, int /*font_size*/,
				 int /*font_style*/, const std::string &/*font_color*/,
				 const std::string &/*line_color*/, viewer::Region::LineStyle /*line_style*/ )
			DISPLAY_PURE_VIRTUAL(MultiWCTool::create,true);

	    // defeat the registering/unregistering done by MultiWCTool (because polygon regions stop working
	    // because they see a double-click from both the RegionToolManager and MultiWCTool)...
	    void enable() { }
	    void disable() { }

    };
}

#endif
