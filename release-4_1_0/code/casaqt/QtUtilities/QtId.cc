//# QtId.cc: base class which allows mixing in id generation
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
//# $Id: $

#include <stdlib.h>
#include <climits>
#include <algorithm>
#include <casaqt/QtUtilities/QtId.h>

namespace casa {

    std::list<int> *QtId::used_ids = 0;

    int QtId::get_id( ) {

	if ( used_ids == 0 ) { used_ids = new std::list<int>( ); }

	static bool initialized = false;
#if defined(__APPLE___)
	if ( ! initialized ) {
	    initialized = true;
	    srandomdev( );
	}
#else
	if ( ! initialized ) {
	    initialized = true;
	    union {
	      void *foo;
	      unsigned bar;
	    };
	    foo = &initialized;
	    srandom(bar);
	}
#endif
	int rn = (int) random( );
	while ( rn <= 0 || rn == INT_MAX || rn == SHRT_MAX || rn == CHAR_MAX ||
		std::find(used_ids->begin( ), used_ids->end( ), rn) != used_ids->end() ) {
	    rn = (int) random( );
	}
// 	used_ids.insert(rn);
	used_ids->push_back(rn);
	return rn;
    }

}

