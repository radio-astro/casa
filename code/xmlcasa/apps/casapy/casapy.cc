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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Python.h>
#include <CCM_Python/Py.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <xmlcasa/version.h>
#include <sys/errno.h>

#include <ostream>
#include <casa/System/Aipsrc.h>


//
//  helper function to create ~/.casa/ipython/security
//
/*
 * Not needed as it's done in the python driver script
static int make_it_a_dir( const char *path ) {
    struct stat buf;
    if ( stat( path, &buf ) == 0 ) {
	if ( ! S_ISDIR(buf.st_mode) ) {
	    char *savepath = (char*) malloc(sizeof(char) * (strlen(path) + 12));
	    int count = 0;

	    do {
		count += 1;
		sprintf( savepath, "%s_SAV%03d", path, count );
	    } while ( stat( savepath, &buf ) == 0 );

	    if ( rename( path, savepath ) != 0 ) {
		return 1;
	    }
	    if ( mkdir( path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0 ) {
		return 1;
	    }
	    free(savepath);
	}
    } else {
	if ( mkdir( path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0 ) {
	    return 1;
	}
    }
    return 0;
}
*/

#include <display/QtViewer/QtApp.h>

int main( int argc, char **argv ) {
    casa::QtApp::init(argc, argv);

    static const char file_separator = '/';

    char *argv_mod[argc];

    char *path = 0;
    int path_size = 0;
    int path_len = 0;

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
	oldlog << logfile << "-" << 
		       	last_mod->tm_year+1900 << "-";
       	oldlog.width(2);
	oldlog << last_mod->tm_mon+1 << "-";
       	oldlog.width(2);
	oldlog << last_mod->tm_mday << "T";
       	oldlog.width(2);
	oldlog << last_mod->tm_hour << ":";
       	oldlog.width(2);
	oldlog << last_mod->tm_min << ":" ;
       	oldlog.width(2);
 	oldlog << last_mod->tm_sec;
        if(rename(logfile, oldlog.str().c_str()))
		perror("Oh no...");
    }

#define PLACE_PATH(SOURCE)							\
    int add_len = strlen(SOURCE);						\
    if ( add_len > 0 && SOURCE[add_len-1] == file_separator ) --add_len;	\
    if ( path_len + add_len + 2 > path_size ) {					\
	while ( path_len + add_len > path_size )				\
	    path_size = (path_size ? path_size : 2) * 2;			\
	path = (char *) (path ? realloc(path, path_size) : malloc(path_size));	\
    }										\
										\
    memcpy( &path[path_len], SOURCE, add_len );					\
    path_len += add_len;							\
    path[path_len++] = ':';

#define CHECK_PATHS_BOUNDARY							\
    if ( ipython_paths_len == ipython_paths_max ) {				\
	ipython_paths_max *= 2;							\
	ipython_paths = (const char**)						\
		realloc( ipython_paths, sizeof(char*) * ipython_paths_max );	\
    }


    /**************** needed to make ipython work on linux... huh? ****************/
#ifndef PYTHONROOT
#define PYTHONROOT "/usr/lib/casapy"
#endif
#ifndef PYTHONVER
#define PYTHONVER "2.5"
#endif

#ifndef AIPS_DARWIN

#ifdef CASAPYROOT
    const char *ipython_path = ""CASAPYROOT"/lib/python24.zip:"CASAPYROOT"/lib/python"PYTHONVER":"CASAPYROOT"/lib/python"PYTHONVER"/plat-linux2:"CASAPYROOT"/lib/python"PYTHONVER"/lib-tk:"CASAPYROOT"/lib/python"PYTHONVER"/lib-dynload:"CASAPYROOT"/lib/python"PYTHONVER"/site-packages:"CASAPYROOT"/lib/python"PYTHONVER"/site-packages/Numeric:"CASAPYROOT"/lib/python"PYTHONVER"/site-packages/PyObjC:"PYTHONROOT"/bin:"PYTHONROOT"/lib/python24.zip:"PYTHONROOT"/lib/python"PYTHONVER":"PYTHONROOT"/lib/python"PYTHONVER"/plat-linux2:"PYTHONROOT"/lib/python"PYTHONVER"/lib-tk:"PYTHONROOT"/lib/python"PYTHONVER"/lib-dynload:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages/Numeric:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages/PyObjC";
#else
    const char *ipython_path = ""PYTHONROOT"/bin:"PYTHONROOT"/lib/python24.zip:"PYTHONROOT"/lib/python"PYTHONVER":"PYTHONROOT"/lib/python"PYTHONVER"/plat-linux2:"PYTHONROOT"/lib/python"PYTHONVER"/lib-tk:"PYTHONROOT"/lib/python"PYTHONVER"/lib-dynload:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages/Numeric:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages/PyObjC";
#endif

#else
    const char *ipython_path = ""PYTHONROOT"/bin:"PYTHONROOT"/lib/python24.zip:"PYTHONROOT"/lib/python"PYTHONVER":"PYTHONROOT"/lib/python"PYTHONVER"/plat-mac:"PYTHONROOT"/lib/python"PYTHONVER"/plat-darwin:"PYTHONROOT"/lib/python"PYTHONVER"/lib-tk:"PYTHONROOT"/lib/python"PYTHONVER"/lib-dynload:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages/Numeric:"PYTHONROOT"/lib/python"PYTHONVER"/site-packages/PyObjC";
#endif

    int ipython_paths_max = 10;
    int ipython_paths_len = 0;
    const char **ipython_paths = (const char**) malloc(sizeof(char*) * ipython_paths_max);
    ipython_paths[ipython_paths_len++] = ipython_path;

    // --- --- --- process command line arguments first --- --- ---
    // ------------------------------------------------------------
    // move past executable name
    argv_mod[0] = argv[0];
    argv++;

    while ( argc >= 3 ) {

	if ( ! strcmp( argv[0], "--prepend-path" )) {
	    // specifies load path
	    CHECK_PATHS_BOUNDARY
	    memmove(ipython_paths+1, ipython_paths, sizeof(char*) * ipython_paths_len);
	    ipython_paths[0] = argv[1];
	    ipython_paths_len += 1;
	    argc -= 2;
	    argv += 2;
	} else if ( ! strcmp( argv[0], "--path" )) {
	    // specifies load path
	    CHECK_PATHS_BOUNDARY
	    ipython_paths[ipython_paths_len++] = argv[1];
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

    //
    // first configure the path (above), and
    // then construct the string (here)...
    //
    for ( int x = 0; x < ipython_paths_len; ++x ) {
	PLACE_PATH(ipython_paths[x])
    }

    char *dbg = getenv("CASA_DEBUG");
    if ( dbg && atoi(dbg))
	std::cerr << ipython_path << std::endl;

    // move over the rest of the non-casa parameters
    for ( int i=1; i < argc; ++i ) {
	argv_mod[i] = argv[i-1];
    }

    // --- --- --- process $CASAROOT                    --- --- ---
    // ------------------------------------------------------------
    char *casaroot = getenv( "CASAROOT" );
    if ( casaroot ) {
	char *buf = (char *) malloc( strlen(casaroot) + 11 );
	sprintf( buf, "%s/python/%%V", casaroot );
	PLACE_PATH(buf);
	free(buf);
    }


    // --- --- --- process $AIPSROOT                    --- --- ---
    // ------------------------------------------------------------
    char *aipspath = getenv( "CASAPATH" );
    if ( aipspath ) {

	char *apath = strdup( aipspath );
	const char *atmp = strtok( apath, " \t" );
	char *aptr = atmp ? strdup(atmp) : NULL;
	atmp = strtok( NULL, " \t");
	char *aarch = atmp ? strdup(atmp) : NULL;

	if ( aptr && aarch ) {
	    char *buf = (char*) malloc( strlen(aptr)+strlen(aarch) + 12 );
	    sprintf( buf, "%s/%s/python/%%V", aptr, aarch );
	    PLACE_PATH(buf);
	}

	if (aptr) free( aptr );
	if (aarch) free( aarch );
	free( apath );
    }

    if ( path ) {
	if ( path_len > 0 && path[path_len-1] == ':' )
	    path[path_len-1] = '\0';
	else
	    path[path_len]='\0';
    } else {
	path= strdup("");
    }
    /*
     * Now done in the casapy.py script

    //
    //  setup ~/.casa/ state directory...
    //  ----------------------------------------------------------------------
    char *homepath = getenv( "HOME" );
    if ( ! homepath ) {
	std::cerr << "error: 'HOME' environment variable is not set" << std::endl;
	std::cerr << "       please set it to your home directory." << std::endl;
	exit(1);
    }

    char *ipythonpath = (char *) malloc(sizeof(char) * (strlen(homepath) + 50));
    char *ipythonenv = (char *) malloc(sizeof(char) * (strlen(homepath) + 30));
    if ( ! ipythonpath || ! ipythonenv ) {
	std::cerr << "error: failed allocation (ipythonpath)" << std::endl;
	exit(1);
    }

    sprintf( ipythonenv, "IPYTHONDIR=%s/.casa/ipython", homepath );
    putenv( ipythonenv );
    
    struct stat rcstats;
    sprintf( ipythonpath, "%s/.casa/ipython/security", homepath );
    if ( stat(ipythonpath, &rcstats) != 0 ) {
	if ( stat( homepath, &rcstats ) != 0 ) {
	    std::cerr << "error: your home directory '" << homepath << "'" << std::endl;
	    std::cerr << "       does not exist, please create it." << std::endl;
	    exit(1);
	}
	sprintf( ipythonpath, "%s/.casa", homepath );
	if ( make_it_a_dir( ipythonpath ) != 0 ) {
	    std::cerr << "error: could not create '" << ipythonpath << "'" << std::endl;
	    exit(1);
	}
	sprintf( ipythonpath, "%s/.casa/ipython", homepath );
	if ( make_it_a_dir( ipythonpath ) != 0 ) {
	    std::cerr << "error: could not create '" << ipythonpath << "'" << std::endl;
	    exit(1);
	}
	sprintf( ipythonpath, "%s/.casa/ipython/security", homepath );
	if ( make_it_a_dir( ipythonpath ) != 0 ) {
	    std::cerr << "error: could not create '" << ipythonpath << "'" << std::endl;
	    exit(1);
	}
    } else {
	if ( make_it_a_dir( ipythonpath ) != 0 ) {
	    std::cerr << "error: could not create '" << ipythonpath << "'" << std::endl;
	    exit(1);
	}
    }

    free(ipythonpath);
    //  ----------------------------------------------------------------------
    //
    */

    std::cout << "CASA Version ";
    casa::VersionInfo::report(std::cout);
    std::cout << std::endl << "  Built on: "<< casa::VersionInfo::date() << std::endl;
    std::cout << std::endl;
    CCM_Python::Py::init( argc, argv_mod, path );
    CCM_Python::Py py;

    // -- move to casapy, maybe...
    // PyRun_SimpleString("import casac");
    py.evalFile( "casapy.py" );
    // py.evalString("IPython.Shell.IPShell(user_ns=__ipython_namespace__).mainloop(1)");

}
