//# RegionSource.cc: base region factory for generating regions
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
//# $Id: $
#include <display/region/RegionSource.h>
#include <display/region/Region.qo.h>
#include <algorithm>

namespace casa {
	namespace viewer {
		RegionSourceKernel::~RegionSourceKernel( ) {
			for ( std::list<Region*>::iterator it = created_regions.begin( ); it != created_regions.end( ); ++it )
				(*it)->removeNotifiee(this);
		}

		void RegionSourceKernel::dtorCalled( const dtorNotifier *dtor ) {
			std::list<Region*>::iterator it = std::find(created_regions.begin( ), created_regions.end( ), dtor);
			if ( it != created_regions.end( ) ) {
				created_regions.erase(it);
			}
		}

		// this should be declared within RegionSourceKernel::generateExistingRegionUpdates( ),
		// but it chokes gcc version 4.2.1 (Apple Inc. build 5666) (dot 3)
		struct functor {
			void operator( )( Region *element ) {
				element->emitUpdate( );
			}
		};
		void RegionSourceKernel::generateExistingRegionUpdates( ) {
			std::for_each( created_regions.begin( ), created_regions.end( ), functor( ) );
		}

		void RegionSourceKernel::register_new_region( Region *region ) {
			created_regions.push_back(region);
			region->addNotifiee(this);
		}

	}
}
