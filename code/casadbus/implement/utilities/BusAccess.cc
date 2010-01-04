//# BusAccess.cc: tools for getting access to a dbus object
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
//# $Id: $
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>
#include <casa/Exceptions/Error.h>
#include <sys/time.h>
#include <vector>

#define CASA_PREFIX "edu.nrao.casa."
#define CASA_PATH "/casa/"
#define CACHE_INTERVAL 5

namespace casa {
    namespace dbus {

	std::string path( const std::string &name ) {

	    static double cache_time = 0;
	    // these could be expanded to be a map of previous i/o, but mainly we want
	    // to insure that multiple calls to path()/object() in the same immediate
	    // interval, e.g. for multiple calls for arguments of a ctor, are consistent...
	    static std::string cache_input;
	    static std::string cache_output;

	    struct timeval tv = { 0, 0 };
	    gettimeofday( &tv, 0 );

	    double current_time = (double) tv.tv_sec + (double) tv.tv_usec / (double) 1000000000;
	    if ( current_time > (cache_time + double(CACHE_INTERVAL)) || cache_input != name ) {

		casa::DBusSession &session = casa::DBusSession::instance( );
		std::vector<std::string> name_list(session.listNames( ));
		std::string prefix( CASA_PREFIX + name + "_");
		std::vector<std::string> objects;
		for ( std::vector<std::string>::iterator iter = name_list.begin(); iter != name_list.end( ); ++iter ) {
		    if ( ! iter->compare(0,prefix.size(),prefix) ) {
			objects.push_back(*iter);
		    }
		}
		if ( objects.size( ) <= 0 ) {
		    throw AipsError("no " + name + "s available");
		}
		cache_time = current_time;
		cache_input = name;
		cache_output = objects[objects.size( )-1];
	    }
	    return cache_output;
	}

	std::string object( const std::string &name ) {
	    std::string opath = path( name );
	    std::string oname = opath.substr(strlen(CASA_PREFIX));
	    return std::string( CASA_PATH + oname );
	}
    }
}
