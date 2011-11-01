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
#include <list>
#include <display/region/Region.h>
#include <display/DisplayEvents/MultiWCTool.h>

namespace casa {
    class RegionTool : public MultiWCTool {
	public:
	    class State {
		public:
		    void insert( const RegionTool *tool, const viewer::Region *region, unsigned int state );
		    // get the number of regions in a particular state
		    unsigned int count( viewer::Region::MouseState state );
		    // get the number of regions by a specific tool
		    std::list<const viewer::Region*> &regions( const RegionTool *tool );
		    unsigned int state( const viewer::Region *region );
		private:
		    typedef std::map< const viewer::Region*, unsigned int > region_state_type;
		    typedef std::list<const viewer::Region*> tool_regions_type_value;
		    typedef std::map< const RegionTool*, tool_regions_type_value > tool_regions_type;
		    typedef std::map< viewer::Region::MouseState, unsigned int > state_count_type;

		    region_state_type state_map;
		    tool_regions_type tool_map;
		    state_count_type count_map;
		    std::list<const viewer::Region*> default_region_list;
		    
	    };
	    // Constructor taking the primary key to which this tool will respond.
	    RegionTool(const Display::KeySym &keysym = Display::K_Pointer_Button1) : MultiWCTool( keysym ) { }
	    ~RegionTool( ) { }

	    void keyPressed(const WCPositionEvent &ev);
	    void keyReleased(const WCPositionEvent &);
	    void otherKeyPressed(const WCPositionEvent &);
	    void otherKeyReleased(const WCPositionEvent &);
	    void moved(const WCMotionEvent &);
	    void draw(const WCRefreshEvent &);

	    // create regions of various types (e.g. point, rect, poly, etc.). For use when
	    // loading casa region files...
	    virtual bool create( WorldCanvas */*wc*/, const std::vector<std::pair<double,double> > &/*pts*/,
				 const std::string &/*label*/, const std::string &/*font*/, int /*font_size*/,
				 int /*font_style*/, const std::string &/*font_color*/,
				 const std::string &/*line_color*/, viewer::Region::LineStyle /*line_style*/ )
			DISPLAY_PURE_VIRTUAL(MultiWCTool::create,true);
    };
}

#endif
