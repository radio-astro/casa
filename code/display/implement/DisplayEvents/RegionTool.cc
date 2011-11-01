//# RegionTool.cc: base class for event-based tools which create and manage regions
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
#include <display/DisplayEvents/RegionTool.h>

namespace casa {
    void RegionTool::keyPressed(const WCPositionEvent &ev) {  }
    void RegionTool::keyReleased(const WCPositionEvent &) {  }
    void RegionTool::otherKeyPressed(const WCPositionEvent &) {  }
    void RegionTool::otherKeyReleased(const WCPositionEvent &) {  }
    void RegionTool::moved(const WCMotionEvent &) {  }
    void RegionTool::draw(const WCRefreshEvent &) {  }

#define CHECK_MOUSE_STATE(STATE)				\
	if ( state & viewer::Region::STATE ) {			\
	    ct_it = count_map.find(viewer::Region::STATE);	\
	    if ( ct_it != count_map.end( ) )			\
		(*ct_it).second += 1;				\
	}

    void RegionTool::State::insert( const RegionTool *tool, const viewer::Region *region, unsigned int state ) {
	region_state_type::iterator sm_it = state_map.find(region);
	if ( sm_it == state_map.end( ) )
	    state_map.insert( region_state_type::value_type(region,state) );
	else
	    (*sm_it).second = state;

	tool_regions_type::iterator tr_it = tool_map.find(tool);
	if ( tr_it == tool_map.end( ) ) {
	    tool_map.insert( tool_regions_type::value_type(tool,tool_regions_type_value( )) );
	    tr_it = tool_map.find(tool);
	    if ( tr_it == tool_map.end( ) )
		throw viewer::internal_error( "region state insertion failed" );
	}
	(*tr_it).second.push_back(region);

	state_count_type::iterator ct_it;
	CHECK_MOUSE_STATE(MouseRefresh)
	CHECK_MOUSE_STATE(MouseSelected)
	CHECK_MOUSE_STATE(MouseStickySelected)
	CHECK_MOUSE_STATE(MouseUnselected)
	CHECK_MOUSE_STATE(MouseHandle)
    }

    unsigned int RegionTool::State::count( viewer::Region::MouseState state ) {
	state_count_type::iterator ct_it = count_map.find(state);
	return ct_it != count_map.end( ) ? (*ct_it).second : viewer::Region::MouseUnselected;
    }

    std::list<const viewer::Region*> &RegionTool::State::regions( const RegionTool *tool ) {
	tool_regions_type::iterator rt_it = tool_map.find(tool);
	if ( rt_it != tool_map.end( ) )
	    return (*rt_it).second;
	else {
	    default_region_list.clear( );
	    return default_region_list;
	}
    }

    unsigned int RegionTool::State::state( const viewer::Region *region ) {
	region_state_type::iterator st_it = state_map.find(region);
	return st_it != state_map.end( ) ? (*st_it).second : 0;
    }

}
