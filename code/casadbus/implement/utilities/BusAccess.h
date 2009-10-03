//# BusAccess.h: tools for getting access to a dbus object
//# Copyright (C) 2009
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

#ifndef DBUS_BUSACCESS_H_
#define DBUS_BUSACCESS_H_
#include <string>
#include <unistd.h>

namespace casa {
    namespace dbus {

	template<class proxy> proxy *launch( const std::string &name, int trys=60, unsigned long delay=500000 ) {
	    proxy *result = 0;
	    try {
		result = new proxy( name );
	    } catch (...) {
		const char **args = proxy::execArgs( );
		if ( ! fork( ) ) {
		    execvp( args[0], (char* const*) args );
		    perror( "launch<>(...) child process exec failed" );
		}
		int count = trys;
		while ( count > 0 ) {
 		    usleep(delay);
		    try {
		      result = new proxy( name );
		      break;
		    } catch (...) { }
		    --count;
		}
	    }
	    return result;
	}

	std::string object( const std::string &name );
	std::string path( const std::string &name );
    }
}

#endif
