#include <display/region/RegionCreator.h>

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
	std::map< RegionCreator::Types, RegionCreator::creator_list_type*> RegionCreator::creator_map;
	RegionCreator::creator_list_type RegionCreator::unsorted_creators;

	RegionCreator::RegionCreator( ) {
	    unsorted_creators.push_back(this);
	}

	RegionCreator::~RegionCreator( ) {
	    // remove all the references 
	    for ( creator_map_type::iterator it = creator_map.begin( );
		  it != creator_map.end(); ++it ) {
		Types tt = (*it).first;
		creator_list_type::iterator me_in_list = std::find((*it).second->begin(),(*it).second->end(),this);
		if ( me_in_list != (*it).second->end( ) ) {
		    (*it).second->erase(me_in_list);
		}
	    }
	}

	const RegionCreator::creator_list_type &RegionCreator::findCreator( RegionCreator::Types type ) {

	    // sort newly constructed creators...
	    for ( creator_list_type::iterator creator_it=unsorted_creators.begin( );
		  creator_it != unsorted_creators.end( ); ++creator_it ) {
		const std::set<Types> &types = (*creator_it)->regionsCreated( );
		for( std::set<Types>::iterator types_it=types.begin( );
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
	  

