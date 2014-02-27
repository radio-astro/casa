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
#include <list>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <stdexcept>

namespace casa {
    namespace dbus {	
	// this function is a little odd... it gets a connection with a unique name
	// (via a suffix) using 'name' and upon success returns 'path', otherwise
	// it throws an exception...
	std::string adaptor_object( const std::string &name, const std::string &path );
	std::string object( const std::string &name );
	std::string path( const std::string &name );

	class address {
	    public:
		    address( const std::string &bus_address, bool unique=true );
			address( );
			virtual ~address( );
			std::string busName( ) const { return name_; }
	    private:
			std::string generate_name( const std::string &base );
			std::string name_;
	};

	char *launch_casa_proxy( bool unique_name, const std::string &dbusname, const std::string &default_name,  const std::list<std::string> &args );

	template<class proxy> proxy *launch( const std::list<std::string> &args=std::list<std::string>( ), bool unique_name=true,
					     const std::string &name="", int trys=60, unsigned long delay=500000 ) {
	    char *dbname = launch_casa_proxy( unique_name, name, proxy::dbusName( ), proxy::execArgs(args) );
	    if ( dbname == 0 ) { throw  std::runtime_error( "launch failed" ); }
	    proxy *result = 0;
	    int count = trys;
	    while ( count > 0 ) {
		usleep(delay);
		try {
		  result = new proxy(dbname);
		  break;
		} catch (...) { }
		--count;
	    }
	    delete [] dbname;
	    return result;
	}

    }
}

#endif
