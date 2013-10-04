//# RegionCreator.cc: class which is responsible for creating regions via a RegionSource object
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

#include <display/region/RegionCreator.h>
#include <display/DisplayErrors.h>
#include <algorithm>

#define FIND_LIST(VAR,TYPE)									\
{   creator_map_type::iterator it = creator_map.find(TYPE);					\
    if ( it == creator_map.end( ) ) {								\
	creator_map.insert( creator_map_type::value_type( TYPE, new creator_list_type( ) ) );	\
    }												\
    it = creator_map.find(TYPE);								\
    if ( it == creator_map.end( ) ) {								\
	throw internal_error( "failed to find a list..." );					\
    }												\
    VAR = (*it).second;										\
}

namespace casa {
	namespace viewer {
		std::map< region::RegionTypes, RegionCreator::creator_list_type*> RegionCreator::creator_map;
		RegionCreator::creator_list_type RegionCreator::unsorted_creators;

		RegionCreator::RegionCreator( ) {
			unsorted_creators.push_back(this);
		}

		RegionCreator::~RegionCreator( ) {
			// remove all the references
			for ( creator_map_type::iterator it = creator_map.begin( );
			        it != creator_map.end(); ++it ) {
				// region::RegionTypes tt = (*it).first;
				creator_list_type::iterator me_in_list = std::find((*it).second->begin(),(*it).second->end(),this);
				if ( me_in_list != (*it).second->end( ) ) {
					(*it).second->erase(me_in_list);
				}
			}
		}

		const RegionCreator::creator_list_type &RegionCreator::findCreator( region::RegionTypes type ) {

			// sort newly constructed creators...
			for ( creator_list_type::iterator creator_it=unsorted_creators.begin( );
			        creator_it != unsorted_creators.end( ); ++creator_it ) {
				const std::set<region::RegionTypes> &types = (*creator_it)->regionsCreated( );
				for( std::set<region::RegionTypes>::iterator types_it=types.begin( );
				        types_it != types.end( ); ++types_it ) {
					creator_list_type *dlist = 0;
					FIND_LIST(dlist,*types_it)
					dlist->push_back(*creator_it);
				}
			}

			// clear newly constructed creators list...
			unsorted_creators.clear( );

			creator_list_type *result = 0;
			FIND_LIST(result,type)
			return *result;
		}


	}
}


