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
#include <display/DisplayErrors.h>

namespace casa {
	void RegionTool::keyPressed(const WCPositionEvent &/*ev*/) {  }
	void RegionTool::keyReleased(const WCPositionEvent &) {  }
	void RegionTool::otherKeyPressed(const WCPositionEvent &) {  }
	void RegionTool::otherKeyReleased(const WCPositionEvent &) {  }
	void RegionTool::moved(const WCMotionEvent &/*ev*/, const viewer::region::region_list_type & /*selected_regions*/) {  }
	void RegionTool::draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type & /*selected_regions*/) {  }

#define CHECK_MOUSE_STATE(STATE)											\
	if ( state & viewer::region::STATE ) {									\
		ct_it = count_map.find(viewer::region::STATE);						\
		if ( ct_it != count_map.end( ) )									\
			(*ct_it).second->insert(region);								\
		else {																\
			count_map.insert( state_count_type::value_type( viewer::region::STATE,	\
				std::tr1::shared_ptr<viewer::region::region_list_type>(new viewer::region::region_list_type( )) ) ); \
			ct_it = count_map.find(viewer::region::STATE);					\
			if ( ct_it != count_map.end( ) )								\
				(*ct_it).second->insert(region);							\
		}																	\
	}

	void RegionTool::State::insert( RegionTool *tool, viewer::Region *region, const viewer::region::PointInfo &state ) {

		all_regions.insert(region);

		region_map_type::iterator sm_it = state_map.find(region);
		if ( sm_it == state_map.end( ) )
			state_map.insert( region_map_type::value_type(region,state) );
		else
			(*sm_it).second = state;

		tool_regions_type::iterator tr_it = tool_map.find(tool);
		if ( tr_it == tool_map.end( ) ) {
			tool_map.insert( tool_regions_type::value_type(tool,viewer::region::region_list_type( )) );
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

	unsigned int RegionTool::State::count( viewer::region::PointLocation loc ) {
		state_count_type::iterator ct_it = count_map.find(loc);
		return ct_it != count_map.end( ) ? (*ct_it).second->size() : 0;
	}

	std::tr1::shared_ptr<viewer::region::region_list_type> RegionTool::State::regions( viewer::region::PointLocation loc,
	        viewer::region::RegionSelect select ) {
		state_count_type::iterator ct_it = count_map.find(loc);
		if ( ct_it != count_map.end( ) ) {
			return select == viewer::region::SelectAny ? (*ct_it).second : filter((*ct_it).second,select );
		} else {
			return std::tr1::shared_ptr<viewer::region::region_list_type>(new viewer::region::region_list_type( ));
		}
	}

	bool RegionTool::State::exists( viewer::Region *region ) const {
		region_map_type::const_iterator it = state_map.find(region);
		return it == state_map.end( ) ? false : true;
	}

	viewer::region::region_list_type &RegionTool::State::regions( RegionTool *tool ) {
		tool_regions_type::iterator rt_it = tool_map.find(tool);
		if ( rt_it != tool_map.end( ) )
			return (*rt_it).second;
		else {
			default_region_list.clear( );
			return default_region_list;
		}
	}

	viewer::region::PointInfo RegionTool::State::state( viewer::Region *region ) {
		region_map_type::iterator st_it = state_map.find(region);
		return st_it != state_map.end( ) ? (*st_it).second : viewer::region::PointInfo(0,0,viewer::region::PointOutside);
	}

	RegionTool::State::~State( ) {
		if ( refresh_count > 0 ) wc_->refresh( );
	}

	std::tr1::shared_ptr<viewer::region::region_list_type> RegionTool::State::filter( std::tr1::shared_ptr<viewer::region::region_list_type> orig,
	        viewer::region::RegionSelect select ) {
		std::tr1::shared_ptr<viewer::region::region_list_type> result(new viewer::region::region_list_type( ));
		for ( viewer::region::region_list_type::iterator it=orig->begin( ); it != orig->end( ); ++it ) {
			viewer::region::RegionTypes region_type = viewer::region::select_to_region(select);
			if ( select == viewer::region::SelectAny || region_type == (*it)->type( ) ) {
				result->insert(*it);
			}
		}
		return result;
	}
}
