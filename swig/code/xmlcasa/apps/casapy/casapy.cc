//# casapy.cc:
//# Copyright (C) 2005
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

#include <Python.h>
#include <CCM_Python/Py.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <xmlcasa/version.h>
#include <sys/errno.h>

#include <ostream>
#include <casa/System/Aipsrc.h>

int main( int argc, char **argv ) {

    char *argv_mod[argc];

    //
    //This bit of code, copies the old casapy.log file to one with a
    //date string based on the last entry and then blows away the casapy.log
    //after creating a hard link
    //
    struct stat thestats;
    char *logfile(0);
    casa::String logfileKey("user.logfile");
    casa::String logname;
    if(!casa::Aipsrc::find(logname, logfileKey))
       logname = "casapy.log";
    logfile = const_cast<char *>(logname.c_str());
    if(!stat(logfile, &thestats)){
#ifdef AIPS_DARWIN
        tm *last_mod = localtime(&thestats.st_mtimespec.tv_sec);
#else
        tm *last_mod = localtime(&thestats.st_mtime);
#endif
	std::ostringstream oldlog;
	oldlog.fill('0');
        casa::String beg = logname.before(".log");
        if (beg == "")
           beg = "casapy";
	oldlog << beg << "-" <<
		       	last_mod->tm_year+1900 << "-";
       	oldlog.width(2);
	oldlog << last_mod->tm_mon+1 << "-";
       	oldlog.width(2);
	oldlog << last_mod->tm_mday << "T";
       	oldlog.width(2);
	oldlog << last_mod->tm_hour ;
       	oldlog.width(2);
	oldlog << last_mod->tm_min ;
       	oldlog.width(2);
 	oldlog << last_mod->tm_sec;
        oldlog << ".log";
        if(rename(logfile, oldlog.str().c_str()))
		perror("Oh no...");
    }

    // --- --- --- process command line arguments first --- --- ---
    // ------------------------------------------------------------
    // move past executable name
    argv_mod[0] = argv[0];
    argv++;

    char *paths[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    int path_max = (int) (sizeof(paths) / sizeof(char*));
    int path_index = 0;

    const char *arg_prepend_path = 0;
    const char *arg_path = 0;

    while ( argc >= 3 ) {

	if ( ! strcmp( argv[0], "--prepend-path" )) {
	    // specifies load path
	    arg_prepend_path = argv[1];
	    argc -= 2;
	    argv += 2;
	} else if ( ! strcmp( argv[0], "--path" )) {
	    // specifies load path
	    arg_path = argv[1];
	    argc -= 2;
	    argv += 2;
	} else if ( ! strcmp( argv[0], "--" )) {
	    // specifies end of casapy arguments
	    argc -= 1;
	    argv += 1;
	    break;
	} else {
	    break;
	}
    }

    if ( arg_prepend_path ) {
	if ( path_index > 0 ) paths[path_index++] = strdup(":");
	paths[path_index++] = strdup(arg_prepend_path);
    }

    // move over the rest of the non-casa parameters
    for ( int i=1; i < argc; ++i ) {
	argv_mod[i] = argv[i-1];
    }

    // --- --- --- process $CASAPATH                    --- --- ---
    // ------------------------------------------------------------
    char *CASAPATH = getenv( "CASAPATH" );
    if ( CASAPATH ) {

	char *cpath = strdup( CASAPATH );
	const char *ctmp = strtok( cpath, " \t" );
	char *croot = ctmp ? strdup(ctmp) : NULL;
	ctmp = strtok( NULL, " \t");
	char *carch = ctmp ? strdup(ctmp) : NULL;

	// here we want to only find the path to casa's python scripts...
	char *buf = (char*) malloc( (croot ? strlen(croot) : 0) + (carch ? strlen(carch) : 0) + 34 );

	if ( croot ) {
	    sprintf( buf, "%s/lib/python%d.%d", croot, PY_MAJOR_VERSION, PY_MINOR_VERSION );
	    if ( ! stat( buf, &thestats ) && S_ISDIR(thestats.st_mode) ) {
		if ( path_index > 0 ) paths[path_index++] = strdup(":");
		paths[path_index++] = buf;
	    } else if ( carch ) {
		sprintf( buf, "%s/%s/python%d.%d", croot, carch, PY_MAJOR_VERSION, PY_MINOR_VERSION );
		if ( ! stat( buf, &thestats ) && S_ISDIR(thestats.st_mode) ) {
		    if ( path_index > 0 ) paths[path_index++] = strdup(":");
		    paths[path_index++] = buf;
		} else {
		    sprintf( buf, "%s/%s/python/%d.%d", croot, carch, PY_MAJOR_VERSION, PY_MINOR_VERSION );
		    if ( ! stat( buf, &thestats ) && S_ISDIR(thestats.st_mode) ) {
			if ( path_index > 0 ) paths[path_index++] = strdup(":");
			paths[path_index++] = buf;
		    }
		}
	    }
	}
    }

    if ( arg_path ) {
	if ( path_index > 0 ) paths[path_index++] = strdup(":");
	paths[path_index++] = strdup(arg_path);
    }    

    int path_len = 0;
    for ( int i=0; paths[i] && i < path_max; ++i ) path_len += strlen(paths[i]);
    char *path = (char*) malloc(sizeof(char) * (path_len + 1));
    path[0] = '\0';
    for ( int i=0; paths[i] && i < path_max; ++i ) strcat( path, paths[i] );

    CCM_Python::Py::init( argc, argv_mod, path );
    CCM_Python::Py py;

    // -- move to casapy, maybe...
    // PyRun_SimpleString("import casac");
    py.evalFile( "casapy.py" );
    // py.evalString("IPython.Shell.IPShell(user_ns=__ipython_namespace__).mainloop(1)");

}

