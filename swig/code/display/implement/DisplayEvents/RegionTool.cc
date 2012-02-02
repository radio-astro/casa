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
#include <display/Display/WorldCanvas.h>

namespace casa {
    void RegionTool::keyPressed(const WCPositionEvent &ev) {  }
    void RegionTool::keyReleased(const WCPositionEvent &) {  }
    void RegionTool::otherKeyPressed(const WCPositionEvent &) {  }
    void RegionTool::otherKeyReleased(const WCPositionEvent &) {  }
    void RegionTool::moved(const WCMotionEvent &) {  }
    void RegionTool::draw(const WCRefreshEvent &) {  }

#define CHECK_MOUSE_STATE(STATE)					\
	if ( state & viewer::Region::STATE ) {				\
	    ct_it = count_map.find(viewer::Region::STATE);		\
	    if ( ct_it != count_map.end( ) )				\
		(*ct_it).second.insert(region);				\
	    else {							\
		count_map.insert( state_count_type::value_type( viewer::Region::STATE, region_list_type( ) ) ); \
		ct_it = count_map.find(viewer::Region::STATE);		\
  		if ( ct_it != count_map.end( ) )			\
		    (*ct_it).second.insert(region);			\
	    }								\
	}

    void RegionTool::State::insert( RegionTool *tool, viewer::Region *region, const viewer::Region::PointInfo &state ) {

	all_regions.insert(region);

	region_map_type::iterator sm_it = state_map.find(region);
	if ( sm_it == state_map.end( ) )
	    state_map.insert( region_map_type::value_type(region,state) );
	else
	    (*sm_it).second = state;

	tool_regions_type::iterator tr_it = tool_map.find(tool);
	if ( tr_it == tool_map.end( ) ) {
	    tool_map.insert( tool_regions_type::value_type(tool,region_list_type( )) );
	    tr_it = tool_map.find(tool);
	    if ( tr_it == tool_map.end( ) )
		throw viewer::internal_error( "region state insertion failed" );
	}
	(*tr_it).second.insert(region);

	state_count_type::iterator ct_it;
	CHECK_MOUSE_STATE(PointInside)
	CHECK_MOUSE_STATE(PointHandle)
	CHECK_MOUSE_STATE(PointOutside)
    }

    unsigned int RegionTool::State::count( viewer::Region::PointLocation loc ) {
	state_count_type::iterator ct_it = count_map.find(loc);
	return ct_it != count_map.end( ) ? (*ct_it).second.size() : 0;
    }

    RegionTool::region_list_type &RegionTool::State::regions( viewer::Region::PointLocation loc ) {
	state_count_type::iterator ct_it = count_map.find(loc);
	if ( ct_it != count_map.end( ) ) {
	    return (*ct_it).second;
	} else {
	    default_region_list.clear( );
	    return default_region_list;
	}
    }

    bool RegionTool::State::exists( viewer::Region *region ) const {
	region_map_type::const_iterator it = state_map.find(region);
	return it == state_map.end( ) ? false : true;
    }

    RegionTool::region_list_type &RegionTool::State::regions( RegionTool *tool ) {
	tool_regions_type::iterator rt_it = tool_map.find(tool);
	if ( rt_it != tool_map.end( ) )
	    return (*rt_it).second;
	else {
	    default_region_list.clear( );
	    return default_region_list;
	}
    }

    const viewer::Region::PointInfo &RegionTool::State::state( viewer::Region *region ) {
	region_map_type::iterator st_it = state_map.find(region);
	return st_it != state_map.end( ) ? (*st_it).second : viewer::Region::PointInfo(0,0,viewer::Region::PointOutside);
    }

    RegionTool::State::~State( ) { if ( refresh_count > 0 ) wc_->refresh( ); }

}
