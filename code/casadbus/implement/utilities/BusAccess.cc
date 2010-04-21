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
#include <cstring>
#include <sys/time.h>
#include <vector>

#define CASA_PREFIX "edu.nrao.casa."
#define CASA_PATH "/casa/"
#define CACHE_INTERVAL 5

namespace casa {
    namespace dbus {

	static char *generate_proxy_suffix( int len ) {
	    char *suffix = 0;
	    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	    suffix = new char[len+1];
	    for (int i = 0; i < len; ++i) {
	      suffix[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	    }
	    suffix[len] = 0;
	    return suffix;
	}

	static char *generate_casa_proxy_name(bool unique_name, const std::string &dbusname, const std::string &defaultname ) {
	    const int suffix_length = 12;

	    if ( dbusname.size( ) == 0 && defaultname.size( ) == 0 )
		return generate_proxy_suffix( suffix_length * 2 );

	    char *suffix = ( unique_name ? generate_proxy_suffix(suffix_length) : 0 );
	    char *result = 0;

	    if ( dbusname.size( ) > 0 ) {
		result = new char [dbusname.size() + (suffix ? strlen(suffix) : 0) + 2];
		sprintf(result, (suffix ? "%s_%s" : "%s"), dbusname.c_str( ), suffix);
	    } else if ( defaultname.size( ) > 0 ) {
		result = new char [defaultname.size() + (suffix ? strlen(suffix) : 0) + 2];
		sprintf(result, (suffix ? "%s_%s" : "%s"), defaultname.c_str( ), suffix);
	    }
	    if ( suffix ) delete [] suffix;
	    return result;
	}


	char *launch_casa_proxy( bool unique_name, const std::string &dbusname,
				 const std::string &default_name,  const char **args ) {

	    if ( ! args[0] ) { return 0; }

	    // if we cannot locate the executable, return 0
	    int arg0len = strlen(args[0]);
	    char *path=getenv("PATH");
	    if ( ! path ) { return 0; }
	    char *pathptr = path = strdup(path);
	    int exe_path_len = 2048;
	    char *exe_path = (char*) malloc(sizeof(char) * exe_path_len);
	    while ( char *p = strsep(&pathptr, ":") ) {
		if ( (strlen(p) + arg0len + 4) > exe_path_len ) {
		    while ( (strlen(p) + arg0len + 4) > exe_path_len ) { exe_path_len *= 2; }
		    exe_path = (char*) realloc(exe_path,exe_path_len);
		}
		sprintf(exe_path, "%s/%s", p, args[0]);
		if ( access(exe_path, X_OK) == 0 ) break;
		else *exe_path = '\0';
	    }

	    bool found = (strlen(exe_path) > 0);
	    free(path);
	    free(exe_path);
	    if ( ! found ) { return 0; }

	    
	    int argc = 0;
	    while ( args[argc] ) ++argc;
	    const char **arguments = (const char**) malloc(sizeof(char*) * (argc + 3));
	    int count;
	    for ( count=0; count < argc; ++count )
		arguments[count] = args[count];
	    arguments[count++] = "--dbusname";
	    char *dbus_name = generate_casa_proxy_name(unique_name, dbusname, default_name);
	    arguments[count++] = dbus_name;
	    arguments[count] = '\0';
	    if ( ! fork( ) ) {
		execvp( args[0], (char* const*) arguments );
		perror( "launch<>(...) child process exec failed" );
	    }
	    free(arguments);
	    return dbus_name;
	}

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
		std::string prefix( CASA_PREFIX + name );
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
