//# dtor.cc: classes for managing the notification of deleted objects
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
#include <display/Utilities/dtor.h>
#include <algorithm>

namespace casa {
	namespace viewer {

		void dtorNotifier::addNotifiee( dtorNotifiee *notifiee ) {
			registrants.push_back(notifiee);
		}

		void dtorNotifier::removeNotifiee( dtorNotifiee *notifiee ) {
			std::list<dtorNotifiee*>::iterator it = std::find( registrants.begin( ), registrants.end( ), notifiee );
			if ( it != registrants.end( ) ) registrants.erase(it);
		}

		// this should be declared within dtorNotifier::~dtorNotifier( ),
		// but it chokes gcc version 4.2.1 (Apple Inc. build 5666) (dot 3)
		struct functor {
			functor( const dtorNotifier *notifier ) : self(notifier) { }
			void operator( )( dtorNotifiee *element ) {
				element->dtorCalled(self);
			}
			const dtorNotifier *self;
		};

		dtorNotifier::~dtorNotifier( ) {
			std::for_each( registrants.begin( ), registrants.end( ), functor(this) );
		}
	}
}



